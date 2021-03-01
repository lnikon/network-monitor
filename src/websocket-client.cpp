#include "network-monitor/websocket-client.h"

#include <iomanip>
#include <iostream>

#include <curl/curl.h>

namespace NetworkMonitor
{

void Log(const std::string& where, boost::system::error_code ec)
{
    std::cerr << "[" << std::setw(20) << where << "]" << (ec ? "Error: " : "OK")
              << (ec ? ec.message() : "") << std::endl;
}

static std::size_t WriteData(void* ptr, std::size_t size, std::size_t nmemb, void* stream)
{
    std::size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;
}

bool DownloadFile(const std::string& fileUrl,
                  const std::filesystem::path& destination,
                  const std::filesystem::path& caCertFile)
{
    BOOST_ASSERT(!fileUrl.empty());
    CURL* curlHandle{curl_easy_init()};
    if (curlHandle)
    {
        curl_easy_setopt(curlHandle, CURLOPT_URL, fileUrl.data());
        curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curlHandle, CURLOPT_CAINFO, caCertFile.c_str());
        curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYHOST, 2L);

        FILE* networkFile = fopen(destination.string().data(), "wb");
        if (networkFile)
        {
            curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, networkFile);

            // Note: curl_easy_perform is a blocking call.
            if (CURLE_OK != curl_easy_perform(curlHandle))
            {
                fclose(networkFile);
                return false;
            }

            fclose(networkFile);
        }

        curl_easy_cleanup(curlHandle);
        curl_global_cleanup();
        return true;
    }

    return false;
}

} // namespace NetworkMonitor
