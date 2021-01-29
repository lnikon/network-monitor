#include <boost/test/unit_test.hpp>

#include <network-monitor/file-downloader.h>
#include <network-monitor/websocket-client.h>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <nlohmann/json.hpp>

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

BOOST_AUTO_TEST_CASE(file_parser)
{
    // Download network layout file
	const std::filesystem::path sourceFile{TESTS_NETWORK_LAYOUT_JSON};
    auto json = NetworkMonitor::ParseJsonFile(sourceFile);
    BOOST_CHECK(json.is_object());
    BOOST_CHECK(json.contains("lines"));
    BOOST_CHECK(json.at("lines").size() > 0);
    BOOST_CHECK(json.contains("stations"));
    BOOST_CHECK(json.at("stations").size() > 0);
    BOOST_CHECK(json.contains("travel_times"));
    BOOST_CHECK(json.at("travel_times").size() > 0);
}

BOOST_AUTO_TEST_SUITE_END();
