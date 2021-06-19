#include <network-monitor/stomp-client.h>

namespace NetworkMonitor
{

std::string ToString(const StompClientError error)
{
    switch(error)
    {
    case NetworkMonitor::StompClientError::OK:
        return std::string{"OK"};
        break;
    case NetworkMonitor::StompClientError::UNDEFINED_ERROR:
        return std::string{"UNDEFINED_ERROR"};
        break;
    case NetworkMonitor::StompClientError::COULD_NOT_CLOSE_WEB_SOCKETS_CONNECTION:
        return std::string{"COULD_NOT_CLOSE_WEB_SOCKETS_CONNECTION"};
        break;
    case NetworkMonitor::StompClientError::COULD_NOT_CONNECT_TO_WEB_SOCKETS_SERVER:
        return std::string{"COULD_NOT_CONNECT_TO_WEB_SOCKETS_SERVER"};
        break;
    case NetworkMonitor::StompClientError::COULD_NOT_SEND_STOMP_FRAME:
        return std::string{"COULD_NOT_SEND_STOMP_FRAME"};
        break;
    case NetworkMonitor::StompClientError::COULD_NOT_SUBSCRIBE_FRAME:
        return std::string{"COULD_NOT_SUBSCRIBE_FRAME"};
        break;
    case NetworkMonitor::StompClientError::COULD_NOT_CREATE_VALID_FRAME:
        return std::string{"COULD_NOT_CREATE_VALID_FRAME"};
        break;
    case NetworkMonitor::StompClientError::UNEXPECTED_MESSAGE_CONTENT_TYPE:
        return std::string{"UNEXPECTED_MESSAGE_CONTENT_TYPE"};
        break;
    case NetworkMonitor::StompClientError::UNEXPECTED_SUBSCRIPTION_MISMATCH:
        return std::string{"UNEXPECTED_SUBSCRIPTION_MISMATCH"};
        break;
    case NetworkMonitor::StompClientError::WEB_SOCKETS_SERVER_DISCONNECTED:
        return std::string{"WEB_SOCKETS_SERVER_DISCONNECTED"};
        break;
    }

	return std::string{};
}

std::ostream& operator<<(std::ostream& os, const StompClientError error)
{
    os << ToString(error);
    return os;
}

}; // NetworkMonitor
