#ifndef NETWORK_MONITOR_STOMP_CLIENT_H
#define NETWORK_MONITOR_STOMP_CLIENT_H

#include <network-monitor/stomp-frame.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <spdlog/spdlog.h>

#include <sstream>
#include <unordered_map>

namespace NetworkMonitor
{

/*! \brief Error codes for the STOMP client.
 */
enum class StompClientError
{
    OK = 0,
    UNDEFINED_ERROR,
    COULD_NOT_CLOSE_WEB_SOCKETS_CONNECTION,
    COULD_NOT_CONNECT_TO_WEB_SOCKETS_SERVER,
    COULD_NOT_SEND_STOMP_FRAME,
    COULD_NOT_SUBSCRIBE_FRAME,
    COULD_NOT_CREATE_VALID_FRAME,
    UNEXPECTED_MESSAGE_CONTENT_TYPE,
    UNEXPECTED_SUBSCRIPTION_MISMATCH,
    WEB_SOCKETS_SERVER_DISCONNECTED,
    COULD_NOT_SEND_SUBSCRIBE_FRAME,

    SIZE_OF_ENUM
};

/*! \brief Convert 'StompClientError' into string.
 */
std::string ToString(const StompClientError error);

/*! \brief Print operator for StompClientError class
 */
std::ostream& operator<<(std::ostream& os, const StompClientError error);

/*! \brief STOMP client implementing the subset of commands needed by the network-events service.
 */
template <typename WsClient> class StompClient
{
private:
    struct Subscription
    {
        std::string destination;
        std::function<void(StompClientError, std::string&&)> onSubscribe;
        std::function<void(StompClientError, std::string&&)> onMessage;
    };

public:
    /*! \brief Construct a STOMP client connecting to a remote URL/port through
     *  a secure WebSockets connection.
     */
    StompClient(const std::string& url,
                const std::string& endpoint,
                const std::string& port,
                boost::asio::io_context& ioc,
                boost::asio::ssl::context& ctx)
        : m_ws{url, port, endpoint, ioc, ctx}
        , m_url{url}
        , m_context{boost::asio::make_strand(ioc)}
    {
        spdlog::info("StompClient: Create STOMP client for {}{}:{}", url, endpoint, port);
    }

    /*! \brief The copy constructor is deleted.
     */
    StompClient(const StompClient&) = delete;

    /*! \brief Move constructor.
     */
    StompClient(StompClient&&) = default;

    /*! \brief Copy assignment operator.
     */
    StompClient& operator=(const StompClient&) = delete;

    /*! \brief Move assignment operator.
     */
    StompClient& operator=(StompClient&&) = default;

    void Connect(const std::string& username,
                 const std::string& password,
                 std::function<void(StompClientError)> onConnect = nullptr,
                 std::function<void(StompClientError)> onDisconnect = nullptr)
    {
        m_username = username;
        m_password = password;
        m_onConnect = onConnect;
        m_onDisconnect = onDisconnect;

        spdlog::info("StompClient: Attempting to connect");
        m_ws.Connect([this](auto ec) { onWsConnect(ec); },
                     [this](auto ec, auto&& msg) { onWsMessage(ec, std::move(msg)); },
                     [this](auto ec) { onWsDisconnect(ec); });
    }

    std::string Subscribe(const std::string& destination,
                          std::function<void(StompClientError, std::string&&)> onSubscribe,
                          std::function<void(StompClientError, std::string&&)> onMessage)
    {
        auto subscriptionId{generateId()};
        Subscription subscription{destination, onSubscribe, onMessage};

        std::vector<StompFrame::HeaderCopy> headers{{{StompHeaders::ID, subscriptionId},
                                                     {StompHeaders::DESTINATION, destination},
                                                     {StompHeaders::ACK, "auto"},
                                                     {StompHeaders::RECEIPT, subscriptionId}}};

        StompError err{StompError::OK};
        StompFrame frame{err, StompCommand::SUBSCRIBE, headers, std::string{}};

        if (err != StompError::OK)
        {
            return std::string{};
        }

        spdlog::info("StompClient: Sending subscription message:\n{}", frame.String());

        m_ws.Send(frame.String(),
                  [this, subscriptionId = subscriptionId, subscription = std::move(subscription)](
                      auto ec) mutable {
                      onWsSendSubscribe(ec, std::move(subscriptionId), std::move(subscription));
                  });
        return subscriptionId;
    }

    void Close(std::function<void(StompClientError)> onClose = nullptr)
    {
        if (onClose)
        {
            m_ws.Close([this, onClose](auto ec) { onWsClose(ec, onClose); });
        }
    }

private:
    void onWsConnect(boost::system::error_code ec)
    {
        using Header = StompFrame::Header;
        using Error = StompClientError;

        if (ec)
        {
            spdlog::warn("StompClient: Unable to connect to STOMP server");
            if (m_onConnect)
            {
                spdlog::warn("StompClient: Calling user provided handler for STOMP connection");
                boost::asio::post(m_context, [onConnect = m_onConnect]() {
                    onConnect(Error::COULD_NOT_CONNECT_TO_WEB_SOCKETS_SERVER);
                });
            }
            return;
        }

        const StompCommand cmd{StompCommand::STOMP};
        const std::vector<StompFrame::HeaderCopy> headers{{{StompHeaders::ACCEPT_VERSION, "1.2"},
                                                           {StompHeaders::HOST, m_url},
                                                           {StompHeaders::LOGIN, m_username},
                                                           {StompHeaders::PASSCODE, m_password}}};

        StompError err{StompError::OK};
        StompFrame frame(err, cmd, headers, std::string{});
        if (err != StompError::OK)
        {
            spdlog::warn("StompClient: Unable to create STOMP frame for to initiate connection");
            if (m_onConnect)
            {
                spdlog::warn("StompClient: Calling user provided handler for STOMP connection");
                boost::asio::post(m_context, [onConnect = m_onConnect]() {
                    onConnect(Error::COULD_NOT_CREATE_VALID_FRAME);
                });
            }
            return;
        }

        spdlog::info("StompClient: Sending STOMP frame to connect. Frame is:\n{}", frame.String());
        m_ws.Send(frame.String(), [this](auto ec) { onWsSendStomp(ec); });
    }

    void onWsMessage(boost::system::error_code ec, std::string msg)
    {
        using Error = StompClientError;
        StompError err;
        StompFrame frame{err, std::move(msg)};
        if (err != StompError::OK)
        {
            spdlog::warn("StompClient: Can not parse received STOMP messsage. Message is: {}", msg);
            if (m_onConnect)
            {
                spdlog::info(
                    "StompClient: Calling user provided handler for STOMP message handling");
                boost::asio::post(m_context, [onConnect = m_onConnect] {
                    onConnect(Error::COULD_NOT_CREATE_VALID_FRAME);
                });
            }
            return;
        }

        spdlog::info("StompClient: Successfully parsed received STOMP message. Message is:\n{}",
                     frame.String());

        switch (frame.GetCommand())
        {
        case StompCommand::CONNECTED:
            handleConnected(std::move(frame));
            break;
        case StompCommand::RECEIPT:
            handleSubscriptionReceipt(std::move(frame));
            break;
        case StompCommand::MESSAGE:
            handleSubscriptionMessage(std::move(frame));
            break;
        default:
            break;
        }
    }

    void onWsDisconnect(boost::system::error_code ec)
    {
        spdlog::warn("StompClient: STOMP connection disconnected");
        if (m_onDisconnect)
        {
            spdlog::warn(
                "StompClient: Calling user provided handler for disconnected STOMP connection");
            auto error{ec ? StompClientError::WEB_SOCKETS_SERVER_DISCONNECTED
                          : StompClientError::OK};
            boost::asio::post(m_context,
                              [onDisconnect = m_onDisconnect, error]() { onDisconnect(error); });
        }
    }

    void onWsSendStomp(boost::system::error_code ec)
    {
        if (ec)
        {
            if (m_onConnect)
            {
                spdlog::warn(
                    "StompClient: Calling user provided handler for disconnected STOMP connection");
                boost::asio::post(m_context, [onConnect = m_onConnect]() {
                    onConnect(StompClientError::COULD_NOT_SEND_STOMP_FRAME);
                });
            }
        }
        else
        {
            spdlog::info("StompClient: Successfully sent STOMP connection frame");
        }
    }

    void onWsSendSubscribe(boost::system::error_code ec,
                           std::string&& subscriptionId,
                           Subscription&& subscription)
    {
        if (!ec)
        {
            m_subscriptions.emplace(subscriptionId, std::move(subscription));
        }
        else
        {
            if (subscription.onSubscribe)
            {
                boost::asio::post(m_context, [onSubscribe = subscription.onSubscribe]() {
                    onSubscribe(StompClientError::COULD_NOT_SEND_SUBSCRIBE_FRAME, "");
                });
            }
        }
    }

    void handleConnected(StompFrame&& /* frame */)
    {
        using Error = StompClientError;
        if (m_onConnect)
        {
            spdlog::info("StompClient: Calling user provided callback from connection handler");
            boost::asio::post(m_context, [onConnect = m_onConnect] { onConnect(Error::OK); });
        }
    }

    void handleSubscriptionReceipt(StompFrame&& frame)
    {
        using Error = StompClientError;
        auto subscriptionId{frame.GetHeaderValue(StompHeaders::RECEIPT_ID)};
        auto subscriptionIt{m_subscriptions.find(subscriptionId)};
        if (m_subscriptions.end() == subscriptionIt)
        {
            spdlog::warn("StompClient: No subscription found for {} receipt id", subscriptionId);
            return;
        }

        spdlog::info("StompClient: Subscription found for {} receipt id", subscriptionId);
        const auto& subscription{subscriptionIt->second};
        if (subscription.onSubscribe)
        {
            spdlog::info("StompClient: Calling user provided handler for subscription handling");
            boost::asio::post(m_context,
                              [onSubscribe = subscription.onSubscribe,
                               subscriptionId = std::string(subscriptionId)]() mutable {
                                  onSubscribe(Error::OK, std::move(subscriptionId));
                              });
        }
    }

    void handleSubscriptionMessage(StompFrame&& frame)
    {
        auto subscriptionId{frame.GetHeaderValue(StompHeaders::RECEIPT_ID)};
        auto subscriptionIt{m_subscriptions.find(subscriptionId)};
        if (m_subscriptions.end() == subscriptionIt)
        {
            spdlog::warn("StompClient: No subscription found for {} receipt id", subscriptionId);
            return;
        }

        const auto& subscription = subscriptionIt->second;
        spdlog::info("StompClient: Subscription found for {} receipt id", subscriptionId);
        if (subscription.destination != frame.GetHeaderValue(StompHeaders::DESTINATION))
        {
            if (subscription.onSubscribe)
            {
                spdlog::warn(
                    "StompClient: Mismatched subscription destinations. Stored: {}, Received: {}",
                    subscription.destination,
                    frame.GetHeaderValue(StompHeaders::DESTINATION));
                spdlog::info("StompClient: Subscription found for {} receipt id", subscriptionId);
                boost::asio::post(m_context, [onMessage = subscription.onMessage]() mutable {
                    onMessage(StompClientError::UNEXPECTED_SUBSCRIPTION_MISMATCH, {});
                });
            }
            return;
        }

        if (subscription.onMessage)
        {
            spdlog::info(
                "StompClient: Calling user provided handler for subscription message handling");
            boost::asio::post(
                m_context,
                [message = frame.GetBody(), onMessage = subscription.onMessage]() mutable {
                    onMessage(StompClientError::OK, std::move(message));
                });
        }
    }

    void onWsClose(boost::system::error_code ec,
                   std::function<void(StompClientError)> onClose = nullptr)
    {
        spdlog::info("StompClient: Closing STOMP connection");
        if (onClose)
        {
            spdlog::info("StompClient: Calling user provided handler for STOMP closing");
            auto error{ec ? StompClientError::COULD_NOT_CLOSE_WEB_SOCKETS_CONNECTION
                          : StompClientError::OK};
            boost::asio::post(m_context, [onClose, error]() { onClose(error); });
        }
    }

    std::string generateId() const
    {
        std::stringstream ss{};
        ss << boost::uuids::random_generator()();
        return ss.str();
    }

private:
    WsClient m_ws;
    std::string m_url;
    boost::asio::strand<boost::asio::io_context::executor_type> m_context;
    std::unordered_map<std::string, Subscription> m_subscriptions;

    std::string m_username;
    std::string m_password;
    std::function<void(StompClientError)> m_onConnect;
    std::function<void(StompClientError)> m_onDisconnect;
};

}; // namespace NetworkMonitor

#endif // NETWORK_MONITOR_STOMP_CLIENT_H
