#include <network-monitor/stomp-frame.h>
#include <network-monitor/stomp-parser.h>

#include <algorithm>
#include <iostream>
#include <iterator>

namespace NetworkMonitor
{

StompFrame::StompFrame()
{
}

StompFrame::StompFrame(StompError& ec, const std::string& frame)
    : m_frame{frame}
{
    initialize(ec);
}

StompFrame::StompFrame(StompError& ec,
                       const StompCommand cmd,
                       const std::vector<StompFrame::HeaderCopy>& headers,
                       const StompFrame::Body& body)
{
    using namespace std::string_literals;

    std::string plain{""};
    plain += ToString(cmd);
    plain += "\n";
    for (const auto& hdr : headers)
    {
        plain += ToString(hdr.key);
        plain += ":";
        plain += hdr.value;
        plain += "\n";
    }
    plain += "\n";
    plain += body;
    plain += "\0"s;
    m_frame = std::move(plain);
}

StompFrame::StompFrame(StompError& ec, std::string&& frame)
    : m_frame{std::move(frame)}
{
    initialize(ec);
}

StompCommand StompFrame::GetCommand() const
{
    return m_command;
}

StompFrame::HeaderCopy StompFrame::GetHeader(const StompHeaders headerKey)
{
    auto it = std::find_if(std::begin(m_headers),
                           std::end(m_headers),
                           [&headerKey](const auto header) { return header.key == headerKey; });

    return (it != m_headers.end()
                ? HeaderCopy{(*it).key, std::string{(*it).value.data(), (*it).value.size()}}
                : HeaderCopy{StompHeaders::SIZE_OF_ENUM, ""});
}

std::string StompFrame::GetHeaderValue(const StompHeaders hdr)
{
    return GetHeader(hdr).value;
}

std::vector<StompFrame::HeaderCopy> StompFrame::GetHeaders()
{
    std::vector<HeaderCopy> results;
    for (auto& header : m_headers)
    {
        results.emplace_back(
            HeaderCopy{header.key, std::string{header.value.data(), header.value.size()}});
    }

    return results;
}

std::string StompFrame::GetBody() const
{
    return std::string{m_body.data(), m_body.size()};
}

std::string StompFrame::String() const
{
    return m_frame;
}

void StompFrame::initialize(StompError& ec)
{
    StompParser parser{m_frame};
    auto cmd = parser.parseCommand(ec);
    if (StompError::OK != ec)
    {
        return;
    }

    std::vector<Header> headers;
    Header hdr;
    do
    {
        hdr = parser.parseHeader(ec);
        if (StompError::OK != ec)
        {
            if (StompError::EMPTY_HEADER != ec)
            {
                return;
            }
        }

        if (StompHeaders::SIZE_OF_ENUM != hdr.key)
        {
            headers.push_back(hdr);
        }
    } while (StompHeaders::SIZE_OF_ENUM != hdr.key);

    std::size_t contentLength{std::string::npos};
    if (std::string contentLengthStr = GetHeaderValue(StompHeaders::CONTENT_LENGTH);
        !contentLengthStr.empty())
    {
        contentLength = std::stoull(contentLengthStr);
    }

    Body body = parser.parseBody(ec, contentLength);
    if (StompError::OK != ec)
    {
        return;
    }

    size_t contentLengths{0};
    for (auto& hdr : headers)
    {
        if (hdr.key == StompHeaders::CONTENT_LENGTH)
        {
            contentLengths = std::stoul(std::string{hdr.value.data(), hdr.value.size()});
            break;
        }
    }

    if (contentLengths != 0 && body.size() != contentLengths)
    {
        ec = StompError::WRONG_CONTENT_LENGTH;
        return;
    }

    if (cmd == StompCommand::CONNECT)
    {
        const auto containsAcceptVersion{
            std::find_if(std::begin(headers), std::end(headers), [](auto hdr) {
                return hdr.key == StompHeaders::ACCEPT_VERSION;
            }) != headers.end()};

        const auto containsHost{std::find_if(std::begin(headers), std::end(headers), [](auto hdr) {
                                    return hdr.key == StompHeaders::HOST;
                                }) != headers.end()};

        if (!containsAcceptVersion)
        {
            ec = StompError::MISSING_ACCEPT_VERSION;
            return;
        }

        if (!containsHost)
        {
            ec = StompError::MISSING_HOST;
            return;
        }
    }

    m_command = cmd;
    m_headers = std::move(headers);
    m_body = body;
}

std::ostream& operator<<(std::ostream& ost, const StompError err)
{
    ost << ToString(err);
    return ost;
}

std::string ToString(const StompError err)
{
    switch (err)
    {
    case NetworkMonitor::StompError::MISSING_ACCEPT_VERSION:
        return std::string{"MISSING_ACCEPT_VERSION"};
        break;
    case NetworkMonitor::StompError::MISSING_HOST:
        return std::string{"MISSING_HOST"};
        break;
    case NetworkMonitor::StompError::WRONG_CONTENT_LENGTH:
        return std::string{"WRONG_CONTENT_LENGTH"};
        break;
    case NetworkMonitor::StompError::JUNK_AFTER_BODY:
        return std::string{"JUNK_AFTER_BODY"};
        break;
    case NetworkMonitor::StompError::UNTERMINATED_BODY:
        return std::string{"UNTERMINATED_BODY"};
        break;
    case NetworkMonitor::StompError::EMPTY_HEADER_VALUE:
        return std::string{"EMPTY_HEADER_VALUE"};
        break;
    case NetworkMonitor::StompError::BAD_HEADER:
        return std::string{"BAD_HEADER"};
        break;
    case NetworkMonitor::StompError::MISSING_BODY_NEWLINE:
        return std::string{"MISSING_BODY_NEWLINE"};
        break;
    case NetworkMonitor::StompError::OK:
        return std::string{"OK"};
        break;
    case NetworkMonitor::StompError::UNDEFINED_COMMAND:
        return std::string{"UNDEFINED_COMMAND"};
        break;
    case NetworkMonitor::StompError::EMPTY_HEADER:
        return std::string{"EMPTY_HEADER"};
        break;
    case NetworkMonitor::StompError::DEV_ERROR:
        return std::string{"DEV_ERROR"};
        break;
    case NetworkMonitor::StompError::SIZE_OF_ENUM:
    default:
        return std::string{""};
        break;
    }
}

std::ostream& operator<<(std::ostream& ost, const StompCommand cmd)
{
    ost << ToString(cmd);
    return ost;
}

std::string ToString(const StompCommand cmd)
{
    switch (cmd)
    {
    case NetworkMonitor::StompCommand::SUBSCRIBE:
        return std::string{"SUBSCRIBE"};
        break;
    case NetworkMonitor::StompCommand::UNSUBSCRIBE:
        return std::string{"UNSUBSCRIBE"};
        break;
    case NetworkMonitor::StompCommand::BEGIN:
        return std::string{"BEGIN"};
        break;
    case NetworkMonitor::StompCommand::COMMENT:
        return std::string{"COMMENT"};
        break;
    case NetworkMonitor::StompCommand::ABORT:
        return std::string{"ABORT"};
        break;
    case NetworkMonitor::StompCommand::ACK:
        return std::string{"ACK"};
        break;
    case NetworkMonitor::StompCommand::NACK:
        return std::string{"NACK"};
        break;
    case NetworkMonitor::StompCommand::DISCONNECT:
        return std::string{"DISCONNECT"};
        break;
    case NetworkMonitor::StompCommand::CONNECT:
        return std::string{"CONNECT"};
        break;
    case NetworkMonitor::StompCommand::STOMP:
        return std::string{"STOMP"};
        break;
    case NetworkMonitor::StompCommand::CONNECTED:
        return std::string{"CONNECTED"};
        break;
    case NetworkMonitor::StompCommand::MESSAGE:
        return std::string{"MESSAGE"};
        break;
    case NetworkMonitor::StompCommand::RECEIPT:
        return std::string{"RECEIPT"};
        break;
    case NetworkMonitor::StompCommand::ERROR:
        return std::string{"ERROR"};
        break;
    case StompCommand::SEND:
        return std::string{"SEND"};
        break;
    case NetworkMonitor::StompCommand::SIZE_OF_ENUM:
    default:
        return std::string{""};
        break;
    }
}

std::string ToString(const StompHeaders cmd)
{
    switch (cmd)
    {
    case NetworkMonitor::StompHeaders::CONTENT_LENGTH:
        return std::string{"content-length"};
        break;
    case NetworkMonitor::StompHeaders::CONTENT_TYPE:
        return std::string{"content-type"};
        break;
    case NetworkMonitor::StompHeaders::RECEIPT:
        return std::string{"receipt"};
        break;
    case NetworkMonitor::StompHeaders::ACCEPT_VERSION:
        return std::string{"accept-version"};
        break;
    case NetworkMonitor::StompHeaders::HOST:
        return std::string{"host"};
        break;
    case NetworkMonitor::StompHeaders::LOGIN:
        return std::string{"login"};
        break;
    case NetworkMonitor::StompHeaders::PASSCODE:
        return std::string{"passcode"};
        break;
    case NetworkMonitor::StompHeaders::ID:
        return std::string{"id"};
        break;
    case NetworkMonitor::StompHeaders::DESTINATION:
        return std::string{"destination"};
        break;
    case NetworkMonitor::StompHeaders::ACK:
        return std::string{"ack"};
        break;
    case NetworkMonitor::StompHeaders::SESSION:
        return std::string{"session"};
        break;
    case NetworkMonitor::StompHeaders::RECEIPT_ID:
        return std::string{"receipt-id"};
        break;
    case NetworkMonitor::StompHeaders::SIZE_OF_ENUM:
    default:
        return std::string{"SIZE_OF_ENUM"};
        break;
    }
}

std::ostream& operator<<(std::ostream& os, const StompHeaders cmd)
{
    os << ToString(cmd);
    return os;
}

}; // namespace NetworkMonitor
