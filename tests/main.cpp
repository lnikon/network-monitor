#define BOOST_TEST_MODULE network-monitor
#include <boost/test/unit_test.hpp>

// #include <network-monitor/websocket-client.h>
// 
// #include <boost/asio.hpp>
// #include <boost/test/unit_test.hpp>
// 
// #include <filesystem>
// #include <fstream>
// #include <string>
// 
// using NetworkMonitor::WebSocketClient;
// 
// BOOST_AUTO_TEST_SUITE(network_monitor);
// 
// BOOST_AUTO_TEST_CASE(cacert_perm)
// {
//     BOOST_CHECK(std::filesystem::exists(TESTS_CACERT_PEM));
// }
// 
// BOOST_AUTO_TEST_CASE(class_WebSocketClient)
// {
//     // Setup network feed staff
//     const auto url{std::string{"ltnm.learncppthroughprojects.com"}};
//     const auto port{std::string{"443"}};
//     const auto endpoint{std::string{"/network-events"}};
//     auto receivedMsg{std::string{}};
// 
//     // Username and password for auth
//     const auto username{std::string{"fake_username"}};
//     const auto password{std::string{"fake_password"}};
// 
//     // Test frame
//     std::stringstream ss{};
//     ss << "STOMP" << std::endl
//        << "accept-version:1.2" << std::endl
//        << "host:transportforlondon.com" << std::endl
//        << "login:" << username << std::endl
//        << "passcode:" << password << std::endl
//        << std::endl
//        << '\0';
// 
//     const auto message{std::string{ss.str()}};
// 
//     // Boost.Asio staff
//     boost::asio::io_context ioc{};
//     boost::asio::ssl::context sslCtx{boost::asio::ssl::context::tlsv12_client};
//     sslCtx.load_verify_file(TESTS_CACERT_PEM);
//     boost::system::error_code ec;
// 
//     NetworkMonitor::WebSocketClient client(url, port, endpoint, ioc, sslCtx);
// 
//     bool connected{false};
//     bool messageSent{false};
//     bool messageReceived{false};
//     bool messageMatches{false};
//     bool disconnected{false};
// 
//     auto onSend{[&messageSent](auto ec) { messageSent = !ec; }};
// 
//     auto onConnect{[&client, &connected, &onSend, &message](auto ec) {
//         connected = !ec;
//         if (!ec)
//         {
//             client.Send(message, onSend);
//         }
//     }};
// 
//     auto onClose{[&disconnected](auto ec) { disconnected = !ec; }};
// 
//     auto onReceive{[&client, &onClose, &messageReceived, &messageMatches, &message, &receivedMsg](
//                        auto ec, auto received) {
//         auto checkResponse = [](const auto resp) {
//             bool ok{true};
//             ok &= resp.find("ERROR") != std::string::npos;
//             ok &= resp.find("ValidationInvalidAuth") != std::string::npos;
//             return ok;
//         };
// 
//         messageReceived = !ec;
//         messageMatches = checkResponse(received);
//         receivedMsg = received;
//         client.Close(onClose);
//     }};
// 
//     client.Connect(onConnect, onReceive);
//     ioc.run();
// 
//     BOOST_CHECK(connected);
//     BOOST_CHECK(messageSent);
//     BOOST_CHECK(messageReceived);
//     BOOST_CHECK(messageMatches);
//     std::cerr << receivedMsg << "\n";
//     // BOOST_CHECK_EQUAL(message, receivedMsg);
//     BOOST_CHECK(disconnected);
// }
// 
// BOOST_AUTO_TEST_SUITE_END();
