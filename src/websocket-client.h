#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <string>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/system_error.hpp>

namespace NetworkMonitor
{

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
     *  \param The The port of the server.
     *  \param The io_context object. The user takes care of calling ioc.run().
     */
    WebSocketClient(const std::string& url, const std::string& port, boost::asio::io_context& ioc);

    /*! \brief Destructor
     */
    ~WebSocketClient();

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
    std::string m_url;
    std::string m_port;

    tcp::resolver m_resolver;
	boost::beast::websocket::stream<tcp_stream> m_ws;
	boost::beast::flat_buffer m_rBuffer;
};

} // namespace NetworkMonitor

#endif // WEBSOCKET_CLIENT_H
