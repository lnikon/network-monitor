#include <iostream>
#include <iomanip>
#include <variant>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast.hpp>

using tcp = boost::asio::ip::tcp;

void Log(std::string from, boost::system::error_code ec)
{
    std::cerr << "From: " << from << "\n";
    std::cerr << (ec ? "Error: " : "OK")
              << (ec ? ec.message() : "")
              << std::endl;
}

void OnConnect(boost::system::error_code ec,
               std::string address,
               std::string endpoint,
               boost::beast::websocket::stream<boost::beast::tcp_stream> &ws,
               const boost::asio::const_buffer &wbuffer,
               boost::beast::flat_buffer &rbuffer);

void OnHandshake(boost::system::error_code ec,
                 boost::beast::websocket::stream<boost::beast::tcp_stream> &ws,
                 const boost::asio::const_buffer &wbuffer,
                 boost::beast::flat_buffer &rbuffer);

void OnConnect(boost::system::error_code ec,
               std::string address,
               std::string endpoint,
               boost::beast::websocket::stream<boost::beast::tcp_stream>& ws,
               const boost::asio::const_buffer& wbuffer,
               boost::beast::flat_buffer& rbuffer)
{
    if (ec)
    {
        Log("OnConnect", ec);
        return;
    }

    ws.async_handshake(address, endpoint, [&ws, &wbuffer, &rbuffer](auto ec) {
        OnHandshake(ec, ws, wbuffer, rbuffer);
    });
}

void OnHandshake(boost::system::error_code ec,
                 boost::beast::websocket::stream<boost::beast::tcp_stream> &ws,
                 const boost::asio::const_buffer &wbuffer,
                 boost::beast::flat_buffer &rbuffer)
{
    if (ec)
    {
        Log("OnHandshake", ec);
        return;
    }

    ws.write(wbuffer, ec);
    if (ec)
    {
        Log("OnHandshake", ec);
        return;
    }

    ws.read(rbuffer, ec);
    if (ec)
    {
        Log("OnHandshake", ec);
        return;
    }

    std::cout << boost::beast::make_printable(rbuffer.data());
}

int main()
{
    boost::asio::io_context ioc {};
    boost::system::error_code ec {};

    tcp::resolver resolver{ioc};
    auto address{"echo.websocket.org"};
    auto endpoint{"/"};
    auto port{"80"};
    auto resolveIt{resolver.resolve(address, port, ec)};
    if (ec)
    {
        Log("main", ec);
        return -1;
    }


    tcp::socket socket(ioc);
    boost::beast::websocket::stream<boost::beast::tcp_stream> ws{std::move(socket)};
    std::string message{"Hello, WebSocket!\n"};
    boost::asio::const_buffer wbuffer {message.c_str(), message.size()};
    boost::beast::flat_buffer rbuffer{};
    ws.next_layer().async_connect(*resolveIt, [&address, &endpoint, &ws, &wbuffer, &rbuffer](auto ec) {
        OnConnect(ec, address, endpoint, ws, wbuffer, rbuffer);
    });


    ioc.run();

    return 0;
}
