#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/assert.hpp>
#include <boost/beast.hpp>

#include <iomanip>
#include <iostream>
#include <thread>

namespace net = boost::asio;
namespace beast = boost::beast;
using namespace boost::beast;
using namespace boost::beast::websocket;
using tcp = boost::asio::ip::tcp;

void Log(boost::system::error_code ec)
{
    std::cerr << "[" << std::setw(14) << std::this_thread::get_id() << "] "
              << (ec ? "Error: " : "OK") << (ec ? ec.message() : "") << std::endl;
}

void OnConnect(boost::system::error_code ec)
{
    Log(ec);
}

int main()
{
    std::cerr << "[" << std::setw(14) << std::this_thread::get_id() << "] main" << std::endl;

    boost::asio::io_context ioc;
    tcp::socket socket(ioc);

    boost::system::error_code ec;

	tcp::resolver resolver(ioc);
	const std::string url("echo.websocket.org");
	const std::string port("80");
	tcp::resolver::results_type results = resolver.resolve(url, port, ec);
	BOOST_ASSERT_MSG(results.begin() != results.end(), "empty endpoints for echo.websocket.org:80");
	auto endpoint{*results.begin()};
    if (ec)
    {
        Log(ec);
        return -1;
    }

	socket.connect(endpoint, ec);
    if (ec)
    {
        Log(ec);
        return -2;
    }

	stream<tcp_stream> ws(std::move(socket));
	ws.handshake(url, "/", ec);
    if (ec)
    {
        Log(ec);
        return -3;
    }

	ws.text(true);

	std::string handshakeStr = "GET / HTTP/1.1"
							"Host: echo.websocket.org"
							"Upgrade: websocket"
							"Connection: Upgrade";
	boost::asio::const_buffer wbuffer(handshakeStr.data(), handshakeStr.size());
	ws.write(wbuffer, ec);
    if (ec)
    {
        Log(ec);
        return -3;
    }

	boost::beast::flat_buffer rbuffer;
	ws.read(rbuffer, ec);

	std::cout << "ECHO: "
			<< boost::beast::make_printable(rbuffer.data())
			<< std::endl;

	// ioc.run(ec);
    // if (ec)
    // {
    //     Log(ec);
    //     return -3;
    // }

    return 0;
}
