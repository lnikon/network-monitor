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

/*! \brief Download a file from a remote HTTPS URL.
 *
 *	\param fileUrl URL of the remote file.
 *	\param destination Path to save the downloaded file.
 *	\param destination Path to the certificate.
 */
bool DownloadFile(const std::string& fileUrl,
                  const std::filesystem::path& destination,
                  const std::filesystem::path& caCertFile = {});

/*! \brief Client to connect to a WebSocket server over plain TCP
 */
class WebSocketClient
{
public:
    using tcp = boost::asio::ip::tcp;
    using tcp_stream = boost::beast::tcp_stream;

    /*! \brief Construct a WebSocket client.
     *
     *  \note This constructor does not initiate a connection.
     *
     *  \param url The URL of the server.
     *  \param prt The The port of the server.
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

    tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> m_ws;
    boost::beast::flat_buffer m_rBuffer;

    std::function<void(boost::system::error_code)> m_onConnect;
    std::function<void(boost::system::error_code, std::string&&)> m_onMessage;
    std::function<void(boost::system::error_code)> m_onDisconnect;
};

} // namespace NetworkMonitor

#endif // WEBSOCKET_CLIENT_H
