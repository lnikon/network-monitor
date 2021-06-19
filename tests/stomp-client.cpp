#include <boost/test/unit_test.hpp>

#include <network-monitor/stomp-client.h>
#include <network-monitor/websocket-client.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/system/error_code.hpp>

using namespace std::string_literals;

using timeout = boost::unit_test::timeout;

static std::string GetEnvVar(const std::string& envVar, const std::string& defaultValue = "")
{
    const char* value{std::getenv(envVar.c_str())};
    if (defaultValue == "")
    {
        BOOST_REQUIRE(value != nullptr);
    }
    return value != nullptr ? value : defaultValue;
}

using NetworkMonitor::BoostWebSocketClient;
// using NetworkMonitor::MockWebSocketClientForStomp;
using NetworkMonitor::StompClient;
using NetworkMonitor::StompClientError;
using NetworkMonitor::StompCommand;
using NetworkMonitor::StompError;
using NetworkMonitor::StompFrame;

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_SUITE(stomp_client);

BOOST_AUTO_TEST_SUITE(enum_class_StompClientError);

BOOST_AUTO_TEST_CASE(live)
{
    const std::string url{GetEnvVar("LTNM_SERVER_URL", "ltnm.learncppthroughprojects.com")};
    const std::string endpoint{"/network-events"};
    const std::string port{GetEnvVar("LTNM_SERVER_PORT", "443")};
    boost::asio::io_context ioc{};
    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TESTS_CACERT_PEM);
    const std::string username{GetEnvVar("LTNM_USERNAME", "bejanyan.vahag@protonmail.com")};
    const std::string password{
        GetEnvVar("LTNM_PASSWORD", "bedcdc0f5770e67127526a5cfc1c36b8635336d7")};

    StompClient<BoostWebSocketClient> client{url, endpoint, port, ioc, ctx};

    bool calledOnClose{false};
    auto onClose{[&calledOnClose](auto ec) {
        calledOnClose = true;
        BOOST_REQUIRE_EQUAL(ec, StompClientError::OK);
    }};

    // We cannot guarantee that we will get a message, so we close the
    // connection on a successful subscription.
    bool calledOnSubscribe{false};
    auto onSubscribe{[&calledOnSubscribe, &client, &onClose](auto ec, auto&& id) {
        calledOnSubscribe = true;
        BOOST_REQUIRE_EQUAL(ec, StompClientError::OK);
        BOOST_REQUIRE(id != "");
        client.Close(onClose);
    }};

    // Receiving messages from the live service is not guaranteed, as it depends
    // on the time of the day. If we do receive a message, we check that it is
    // valid.
    auto onMessage{[](auto ec, auto&& msg) { BOOST_CHECK_EQUAL(ec, StompClientError::OK); }};

    bool calledOnConnect{false};
    auto onConnect{[&calledOnConnect, &client, &onSubscribe, &onMessage](auto ec) {
        calledOnConnect = true;
        BOOST_REQUIRE_EQUAL(ec, StompClientError::OK);
        auto id{client.Subscribe("/passengers", onSubscribe, onMessage)};
        BOOST_REQUIRE(id != "");
    }};

    client.Connect(username, password, onConnect);

    ioc.run();

    BOOST_CHECK(calledOnConnect);
    BOOST_CHECK(calledOnSubscribe);
    BOOST_CHECK(calledOnClose);
}

BOOST_AUTO_TEST_SUITE_END(); // class_StompClient

BOOST_AUTO_TEST_SUITE_END(); // stomp_client

BOOST_AUTO_TEST_SUITE_END(); // network_monitor
