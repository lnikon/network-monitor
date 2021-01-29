#ifndef FILE_DOWNLOADER_H
#define FILE_DOWNLOADER_H

#include <nlohmann/json.hpp>

#include <filesystem>

namespace NetworkMonitor
{

	/*! \brief Parse local JSON file into object.
	 *
	 *  \param source The path to the json file.
	 */
	nlohmann::json ParseJsonFile(const std::filesystem::path& source);

} // namespace NetworkMonitor

#endif // FILE_DOWNLOADER_H
