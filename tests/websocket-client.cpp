#include "network-monitor/websocket-client.h"

#include <iomanip>
#include <iostream>
#include <thread>

int main()
{
	const auto url{std::string{"echo.websocket.org"}};
	const auto port{std::string{"80"}};

	boost::asio::io_context ioc;
	NetworkMonitor::WebSocketClient client(url, port, ioc);
	client.Connect();

	ioc.run();

    return 0;
}
