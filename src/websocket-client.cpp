#include "network-monitor/websocket-client.h"

#include <iomanip>
#include <iostream>

#include <curl/curl.h>

namespace NetworkMonitor
{

static void Log(const std::string& where, boost::system::error_code ec)
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

            // This call will block
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

WebSocketClient::WebSocketClient(const std::string& url,
                                 const std::string& port,
                                 const std::string& endpoint,
                                 boost::asio::io_context& ioc,
                                 boost::asio::ssl::context& ctx)
    : m_url(url)
    , m_port(port)
    , m_endpoint(endpoint)
    , m_resolver(boost::asio::make_strand(ioc))
    , m_ws(boost::asio::make_strand(ioc), ctx)
{
}

void WebSocketClient::Connect(
    std::function<void(boost::system::error_code)> onConnect,
    std::function<void(boost::system::error_code, std::string&&)> onMessage,
    std::function<void(boost::system::error_code)> onDisconnect)
{
    m_onConnect = onConnect;
    m_onMessage = onMessage;
    m_onDisconnect = onDisconnect;

    // Start the chain of async callbacks
    m_resolver.async_resolve(
        m_url, m_port, [this](auto ec, auto endpoint) { onResolve(ec, endpoint); });
}

void WebSocketClient::Send(const std::string& message,
                           std::function<void(boost::system::error_code)> onSend)
{
    m_ws.async_write(boost::asio::buffer(std::move(message)), [this, onSend](auto ec, auto) {
        if (onSend)
        {
            onSend(ec);
        }
    });
}

void WebSocketClient::Close(std::function<void(boost::system::error_code)> onClose)
{
    m_ws.async_close(boost::beast::websocket::close_code::none, [this, onClose](auto ec) {
        if (onClose)
        {
            onClose(ec);
        }
    });
}

void WebSocketClient::onResolve(const boost::system::error_code& ec,
                                tcp::resolver::iterator endpoint)
{
    if (ec)
    {
        Log("OnResolve", ec);
        if (m_onConnect)
        {
            m_onConnect(ec);
        }
        return;
    }

    boost::beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(5));
    boost::beast::get_lowest_layer(m_ws).async_connect(*endpoint,
                                                       [this](auto ec) { onConnect(ec); });
}

void WebSocketClient::onConnect(const boost::system::error_code& ec)
{
    if (ec)
    {
        Log("OnConnect", ec);
        if (m_onConnect)
        {
            m_onConnect(ec);
        }
        return;
    }

    boost::beast::get_lowest_layer(m_ws).expires_never();
    m_ws.set_option(
        boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));

    m_ws.next_layer().async_handshake(boost::asio::ssl::stream_base::client,
                                      [this](auto ec) { onTlsHandshake(ec); });
}

void WebSocketClient::onHandshake(const boost::system::error_code& ec)
{
    if (ec)
    {
        Log("OnHandshake", ec);
        if (m_onConnect)
        {
            m_onConnect(ec);
        }
        return;
    }

    m_ws.text(true);

    listenToIncomingMessage(ec);

    if (m_onConnect)
    {
        m_onConnect(ec);
    }
}

void WebSocketClient::onTlsHandshake(const boost::system::error_code& ec)
{
    if (ec)
    {
        Log("OnTlsHandshake", ec);
        if (m_onConnect)
        {
            m_onConnect(ec);
        }
        return;
    }

    m_ws.async_handshake(m_url, m_endpoint, [this](auto ec) { onHandshake(ec); });
}

void WebSocketClient::listenToIncomingMessage(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted)
    {
        if (m_onDisconnect)
        {
            m_onDisconnect(ec);
        }
        return;
    }

    m_ws.async_read(m_rBuffer, [this](auto ec, auto nBytes) {
        onRead(ec, nBytes);
        listenToIncomingMessage(ec);
    });
}

void WebSocketClient::onRead(const boost::system::error_code& ec, std::size_t nBytes)
{
    if (ec)
    {
        return;
    }

    std::string message{boost::beast::buffers_to_string(m_rBuffer.data())};
    m_rBuffer.consume(nBytes);
    if (m_onMessage)
    {
        m_onMessage(ec, std::move(message));
    }
}

} // namespace NetworkMonitor
