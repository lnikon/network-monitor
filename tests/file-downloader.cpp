#include <boost/test/unit_test.hpp>

#include <network-monitor/websocket-client.h>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <filesystem>
#include <fstream>
#include <string>

using NetworkMonitor::WebSocketClient;

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(file_downloader)
{
    // Download network layout file
    const auto network{std::string{"https://ltnm.learncppthroughprojects.com/network-layout.json"}};
    const auto destination{std::filesystem::temp_directory_path() / "network-layout.json"};
    bool downloaded{false};
    downloaded =
        NetworkMonitor::DownloadFile(network, destination, std::filesystem::path(TESTS_CACERT_PEM));

    BOOST_CHECK(downloaded);
    BOOST_CHECK(std::filesystem::exists(destination));

    const std::string expectedString = {"\"stations\": ["};
    std::ifstream file{destination};
    std::string line{};
    bool foundExpectedString{false};
    while (std::getline(file, line))
    {
        if (foundExpectedString = (line.find(expectedString) != std::string::npos);
            foundExpectedString)
        {
            break;
        }
    }

    BOOST_CHECK(foundExpectedString);
    std::filesystem::remove(destination);
}

BOOST_AUTO_TEST_SUITE_END();
