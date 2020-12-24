#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <iomanip>
#include <iostream>
#include <thread>

namespace net = boost::asio;
namespace beast = boost::beast;
using namespace boost::beast;
using namespace boost::beast::websocket;
using tcp = boost::asio::ip::tcp;

void Log(const std::string& msg, boost::system::error_code ec)
{
    std::cerr << "[" << std::setw(14) << std::this_thread::get_id() << "] (" << msg << ") "
              << (ec ? "Error: " : "OK") << (ec ? ec.message() : "") << std::endl;
}

void onHandshake(
    // <-- Start of shared data
    websocket::stream<tcp_stream>& ws, const boost::asio::const_buffer& wBuffer,
    const boost::asio::const_buffer& rBuffer,
    // <-- End of shared data
    const boost::system::error_code& ec)
{
    if (ec)
    {
        Log("onHandshake", ec);
        return;
    }
}

void onConnect(
    // <-- Start of shared data
    websocket::stream<tcp_stream>& ws, const std::string& url,
    const boost::asio::const_buffer& wBuffer, const boost::asio::const_buffer& rBuffer,
    // <-- End of shared data
    const boost::system::error_code& ec)
{
    if (ec)
    {
        Log("onConnect", ec);
        return;
    }

    ws.async_handshake(
        url, "/", [&ws, &wBuffer, &rBuffer](auto ec) { onHandshake(ws, wBuffer, rBuffer, ec); });
}

int main()
{
    std::cerr << "[" << std::setw(14) << std::this_thread::get_id() << "] main" << std::endl;

    boost::asio::io_context ioc;
    boost::system::error_code ec;

    const auto url{"echo.websocket.org"};
    const auto port{"80"};
    tcp::resolver resolver(ioc);
    auto endpoint{resolver.resolve(url, port, ec)};
    if (ec)
    {
        Log("main", ec);
        return -1;
    }

    beast::websocket::stream<tcp_stream> ws;

    const auto msg{std::string{"GET / HTTP/1.1"
                               "Host: echo.websocket.org"
                               "Upgrade: websocket"
                               "Connection: Upgrade"}};

    boost::asio::const_buffer wBuffer{msg.data(), msg.size()};
    boost::asio::const_buffer rBuffer{};
    ws.next_layer().async_connect(*endpoint, [&ws, &url, &wBuffer, &rBuffer](auto ec) {
        onConnect(ws, url, wBuffer, rBuffer, ec);
    });

    return 0;
}
