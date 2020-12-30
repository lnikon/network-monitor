#include "network-monitor/websocket-client.h"

#include <openssl/ssl.h>

#include <iomanip>
#include <iostream>
#include <thread>

int main()
{
    const auto url{std::string{"echo.websocket.org"}};
    const auto port{std::string{"80"}};
    const auto message{std::string{"Hello WebSocket"}};

    boost::asio::io_context ioc{};
    NetworkMonitor::WebSocketClient client(url, port, ioc);

    bool connected{false};
    bool messageSent{false};
    bool messageReceived{false};
    bool messageMatches{false};
    bool disconnected{false};

    auto onSend{[&messageSent](auto ec) { messageSent = !ec; }};

    auto onConnect{[&client, &connected, &onSend, &message](auto ec) {
        connected = !ec;
        if (!ec)
        {
            client.Send(message, onSend);
        }
    }};

    auto onClose{[&disconnected](auto ec) { disconnected = !ec; }};

    auto onReceive{
        [&client, &onClose, &messageReceived, &messageMatches, &message](auto ec, auto received) {
            messageReceived = !ec;
            messageMatches = message == received;
            client.Close(onClose);
        }};

    client.Connect(onConnect, onReceive);
    ioc.run();

    bool ok{connected && messageSent && messageReceived && messageMatches && disconnected};

    if (ok)
    {
        std::cout << "OK\n";
        return 0;
    }
    else
    {
        std::cerr << "Test Failed\n";
        return 1;
    }

    return 0;
}
