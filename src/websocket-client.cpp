#include "websocket-client.h"

namespace NetworkMonitor
{

WebSocketClient::WebSocketClient(const std::string& url,
                                 const std::string& port,
                                 boost::asio::io_context& ioc)
    : m_url(url)
    , m_port(port)
    , m_resolver(boost::asio::make_strand(ioc))
	, m_ws(boost::asio::make_strand(ioc))
{
}

void Connect(std::function<void(boost::system::error_code)> onConnect,
             std::function<void(boost::system::error_code, std::string&&)> onMessage,
             std::function<void(boost::system::error_code)> onDisconnect)
{
}

} // namespace NetworkMonitor
