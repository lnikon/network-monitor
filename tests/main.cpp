#define BOOST_TEST_MODULE network - monitor
#include <boost/test/unit_test.hpp>

#include <network-monitor/websocket-client.h>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <filesystem>

using NetworkMonitor::WebSocketClient;

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(cacert_perm)
{
	BOOST_CHECK(std::filesystem::exists(TESTS_CACERT_PEM));
}

BOOST_AUTO_TEST_CASE(class_WebSocketClient)
{
    const auto url{std::string{"echo.websocket.org"}};
    const auto port{std::string{"80"}};
    const auto message{std::string{"Hello WebSocket"}};
	auto receivedMsg{std::string{}};

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
        [&client, &onClose, &messageReceived, &messageMatches, &message, &receivedMsg](auto ec, auto received) {
            messageReceived = !ec;
            messageMatches = message == received;
			receivedMsg = received;
            client.Close(onClose);
        }};

    client.Connect(onConnect, onReceive);
    ioc.run();

	BOOST_CHECK(connected);
	BOOST_CHECK(messageSent);
	BOOST_CHECK(messageReceived);
	BOOST_CHECK_EQUAL(message, receivedMsg);
	BOOST_CHECK(disconnected);
}

BOOST_AUTO_TEST_SUITE_END();
