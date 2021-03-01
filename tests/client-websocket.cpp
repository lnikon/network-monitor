#include <network-monitor/websocket-client.h>

#include <openssl/ssl.h>

#include <boost/test/unit_test.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/system/system_error.hpp>

#include <iomanip>
#include <iostream>
#include <thread>

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(websocket_client)
{
    const auto url{std::string{"echo.websocket.org"}};
    const auto port{std::string{"80"}};
    const auto endpoint{std::string{"/network-events"}};
    const auto message{std::string{"Hello WebSocket"}};

    boost::asio::io_context ioc{};
    boost::asio::ssl::context sslCtx{boost::asio::ssl::context::tlsv12_client};
    NetworkMonitor::BoostWebSocketClient client(url, port, endpoint, ioc, sslCtx);

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
    }
    else
    {
        std::cerr << "Test Failed\n";
    }
}

BOOST_AUTO_TEST_SUITE_END(); // network_monitor
