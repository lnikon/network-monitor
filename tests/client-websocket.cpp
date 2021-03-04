#include <boost/test/unit_test.hpp>

#include <network-monitor/websocket-client.h>
#include "boost-mock.h"

#include <openssl/ssl.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/system/system_error.hpp>

#include <iomanip>
#include <iostream>
#include <thread>

using NetworkMonitor::MockResolver;
using NetworkMonitor::MockWebSocketClient;

struct WebSocketClientTestFixture {
	WebSocketClientTestFixture()
	{
		MockResolver::resolve_ec = {};
	}
};

static boost::unit_test::timeout gTimeout {3};

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_SUITE(class_WebsocketClient);

BOOST_FIXTURE_TEST_SUITE(Connect, WebSocketClientTestFixture);

BOOST_AUTO_TEST_CASE(fail_resolve, *gTimeout)
{
    const auto url{std::string{"echo.websocket.org"}};
    const auto port{std::string{"443"}};
    const auto endpoint{std::string{"/"}};

    boost::asio::ssl::context sslCtx{boost::asio::ssl::context::tlsv12_client};
	sslCtx.load_verify_file(TESTS_CACERT_PEM);
    boost::asio::io_context ioc{};

	MockResolver::resolve_ec = boost::asio::error::host_not_found;

	NetworkMonitor::MockWebSocketClient client {url, endpoint, port, ioc, sslCtx};
	bool calledOnConnect {false};
	auto onConnect {[&calledOnConnect](auto ec) {
		calledOnConnect = true;
		BOOST_CHECK_EQUAL(ec, boost::asio::error::host_not_found);
	}};

	client.Connect(onConnect);
	ioc.run();

	BOOST_CHECK(calledOnConnect);
}

BOOST_AUTO_TEST_SUITE_END(); // Connect 

BOOST_AUTO_TEST_SUITE_END(); // class_WebsocketClient

BOOST_AUTO_TEST_SUITE_END(); // network_monitor
