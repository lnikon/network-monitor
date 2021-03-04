#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <filesystem>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/system/system_error.hpp>

namespace NetworkMonitor
{

void Log(const std::string& where, boost::system::error_code ec);

/*! \brief Download a file from a remote HTTPS URL.
 *
 *	\param fileUrl URL of the remote file.
 *	\param destination Path to save the downloaded file.
 *	\param caCertFile Path to the certificate.
 */
bool DownloadFile(const std::string& fileUrl,
                  const std::filesystem::path& destination,
                  const std::filesystem::path& caCertFile = {});

/*! \brief Client to connect to a WebSocket server over plain TCP
 */
template <typename Resolver, typename WebSocketStream> class WebSocketClient
{
public:
    using tcp = boost::asio::ip::tcp;
    using tcp_stream = boost::beast::tcp_stream;

    /*! \brief Construct a WebSocket client.
     *
     *  \note This constructor does not initiate a connection.
     *
     *  \param url The URL of the server.
     *  \param port The The port of the server.
     *  \param endpoint The endpoint of the host.
     *  \param ioc The io_context object. The user takes care of calling ioc.run().
     *  \param ctx The ssl::context object. Used to provide support for tls.
     */
    WebSocketClient(const std::string& url,
                    const std::string& port,
                    const std::string& endpoint,
                    boost::asio::io_context& ioc,
                    boost::asio::ssl::context& ctx);

    /*! \brief Destructor
     */
    ~WebSocketClient() = default;

    /*! \brief Connect to the server.
     *
     *  \param onConnect    Called when the connection fails or succeeds.
     *  \param onMessage 	Called only when a message is successfully
     *					 	received. The message is an rvalue reference;
     *					 	ownership is passter to the receiver.
     *	\param onDisconnect Called when the connection is closed by the server
     *						or due to a connection error.
     */
    void Connect(std::function<void(boost::system::error_code)> onConnect = nullptr,
                 std::function<void(boost::system::error_code, std::string&&)> onMessage = nullptr,
                 std::function<void(boost::system::error_code)> onDisconnect = nullptr);

    /*! \brief Send a text message to the WebSocket server.
     *
     *  \param message The message to send.
     *  \param onSend  Called when a message is sent successfully
     *				   or if it failed to send.
     */
    void Send(const std::string& message,
              std::function<void(boost::system::error_code)> onSend = nullptr);

    /*! \brief Close the WebSocket connection.
     *
     *  \param onClose Called when the connection is closed, successfully or not.
     */
    void Close(std::function<void(boost::system::error_code)> onClose = nullptr);

private:
    void onResolve(const boost::system::error_code& ec, tcp::resolver::iterator endpoint);
    void onConnect(const boost::system::error_code& ec);
    void onHandshake(const boost::system::error_code& ec);
    void onTlsHandshake(const boost::system::error_code& ec);
    void listenToIncomingMessage(const boost::system::error_code& ec);
    void onRead(const boost::system::error_code& ec, std::size_t nBytes);

private:
    std::string m_url;
    std::string m_port;
    std::string m_endpoint;

    Resolver m_resolver;
    WebSocketStream m_ws;
    boost::beast::flat_buffer m_rBuffer;

    std::function<void(boost::system::error_code)> m_onConnect;
    std::function<void(boost::system::error_code, std::string&&)> m_onMessage;
    std::function<void(boost::system::error_code)> m_onDisconnect;
};

template <typename Resolver, typename WebSocketStream>
WebSocketClient<Resolver, WebSocketStream>::WebSocketClient(const std::string& url,
                                                            const std::string& port,
                                                            const std::string& endpoint,
                                                            boost::asio::io_context& ioc,
                                                            boost::asio::ssl::context& ctx)
    : m_url(url)
    , m_port(port)
    , m_endpoint(endpoint)
    , m_resolver(boost::asio::make_strand(ioc))
    , m_ws(boost::asio::make_strand(ioc), ctx)
{
}

template <typename Resolver, typename WebSocketStream>
void WebSocketClient<Resolver, WebSocketStream>::Connect(
    std::function<void(boost::system::error_code)> onConnect,
    std::function<void(boost::system::error_code, std::string&&)> onMessage,
    std::function<void(boost::system::error_code)> onDisconnect)
{
    m_onConnect = onConnect;
    m_onMessage = onMessage;
    m_onDisconnect = onDisconnect;

    // Start the chain of async callbacks
    m_resolver.async_resolve(
        m_url, m_port, [this](auto ec, auto endpoint) { onResolve(ec, endpoint); });
}

template <typename Resolver, typename WebSocketStream>
void WebSocketClient<Resolver, WebSocketStream>::Send(
    const std::string& message,
    std::function<void(boost::system::error_code)> onSend)
{
    m_ws.async_write(boost::asio::buffer(std::move(message)), [this, onSend](auto ec, auto) {
        if (onSend)
        {
            onSend(ec);
        }
    });
}

template <typename Resolver, typename WebSocketStream>
void WebSocketClient<Resolver, WebSocketStream>::Close(
    std::function<void(boost::system::error_code)> onClose)
{
    m_ws.async_close(boost::beast::websocket::close_code::none, [this, onClose](auto ec) {
        if (onClose)
        {
            onClose(ec);
        }
    });
}

template <typename Resolver, typename WebSocketStream>
void WebSocketClient<Resolver, WebSocketStream>::onResolve(const boost::system::error_code& ec,
                                                           tcp::resolver::iterator endpoint)
{
    if (ec)
    {
        Log("OnResolve", ec);
        if (m_onConnect)
        {
            m_onConnect(ec);
        }
        return;
    }

    boost::beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(5));
    boost::beast::get_lowest_layer(m_ws).async_connect(*endpoint,
                                                       [this](auto ec) { onConnect(ec); });
}

template <typename Resolver, typename WebSocketStream>
void WebSocketClient<Resolver, WebSocketStream>::onConnect(const boost::system::error_code& ec)
{
    if (ec)
    {
        Log("OnConnect", ec);
        if (m_onConnect)
        {
            m_onConnect(ec);
        }
        return;
    }

    boost::beast::get_lowest_layer(m_ws).expires_never();
    m_ws.set_option(
        boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));

    m_ws.next_layer().async_handshake(boost::asio::ssl::stream_base::client,
                                      [this](auto ec) { onTlsHandshake(ec); });
}

template <typename Resolver, typename WebSocketStream>
void WebSocketClient<Resolver, WebSocketStream>::onHandshake(const boost::system::error_code& ec)
{
    if (ec)
    {
        Log("OnHandshake", ec);
        if (m_onConnect)
        {
            m_onConnect(ec);
        }
        return;
    }

    m_ws.text(true);

    listenToIncomingMessage(ec);

    if (m_onConnect)
    {
        m_onConnect(ec);
    }
}

template <typename Resolver, typename WebSocketStream>
void WebSocketClient<Resolver, WebSocketStream>::onTlsHandshake(const boost::system::error_code& ec)
{
    if (ec)
    {
        Log("OnTlsHandshake", ec);
        if (m_onConnect)
        {
            m_onConnect(ec);
        }
        return;
    }

    m_ws.async_handshake(m_url, m_endpoint, [this](auto ec) { onHandshake(ec); });
}

template <typename Resolver, typename WebSocketStream>
void WebSocketClient<Resolver, WebSocketStream>::listenToIncomingMessage(
    const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted)
    {
        if (m_onDisconnect)
        {
            m_onDisconnect(ec);
        }
        return;
    }

    m_ws.async_read(m_rBuffer, [this](auto ec, auto nBytes) {
        onRead(ec, nBytes);
        listenToIncomingMessage(ec);
    });
}

template <typename Resolver, typename WebSocketStream>
void WebSocketClient<Resolver, WebSocketStream>::onRead(const boost::system::error_code& ec,
                                                        std::size_t nBytes)
{
    if (ec)
    {
        return;
    }

    std::string message{boost::beast::buffers_to_string(m_rBuffer.data())};
    m_rBuffer.consume(nBytes);
    if (m_onMessage)
    {
        m_onMessage(ec, std::move(message));
    }
}

using BoostWebSocketClient = WebSocketClient<
    boost::asio::ip::tcp::resolver,
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>>;

} // namespace NetworkMonitor

#endif // WEBSOCKET_CLIENT_H
