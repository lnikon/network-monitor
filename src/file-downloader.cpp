#include "network-monitor/file-downloader.h"

#include <fstream>
#include <iostream>

#include <boost/assert.hpp>

namespace NetworkMonitor
{

nlohmann::json ParseJsonFile(const std::filesystem::path& source)
{
    BOOST_ASSERT(!source.empty());
    nlohmann::json json{};
    if (!std::filesystem::exists(source))
    {
        return json;
    }

    try
    {
        std::ifstream file{source};
        file >> json;
    }
    catch (...)
    {
        std::cerr << "Unable to parse " << source << "\n";
    }

    return json;
}

} // namespace NetworkMonitor
