#include <network-monitor/stomp-parser.h>

#include <iostream>

namespace NetworkMonitor
{

StompParser::StompParser(std::string_view frame)
    : m_commandsMap{{"SEND", StompCommand::SEND},
                    {"SUBSCRIBE", StompCommand::SUBSCRIBE},
                    {"UNSUBSCRIBE", StompCommand::UNSUBSCRIBE},
                    {"BEGIN", StompCommand::BEGIN},
                    {"COMMENT", StompCommand::COMMENT},
                    {"ABORT", StompCommand::ABORT},
                    {"ACK", StompCommand::ACK},
                    {"NACK", StompCommand::NACK},
                    {"DISCONNECT", StompCommand::DISCONNECT},
                    {"CONNECT", StompCommand::CONNECT},
                    {"STOMP", StompCommand::STOMP},
                    {"CONNECTED", StompCommand::CONNECTED},
                    {"MESSAGE", StompCommand::MESSAGE},
                    {"RECEIPT", StompCommand::RECEIPT},
                    {"ERROR", StompCommand::ERROR}}
    , m_headersMap{{"content-length", StompHeaders::CONTENT_LENGTH},
                   {"content-type", StompHeaders::CONTENT_TYPE},
                   {"receipt", StompHeaders::RECEIPT},
                   {"accept-version", StompHeaders::ACCEPT_VERSION},
                   {"host", StompHeaders::HOST},
                   {"version", StompHeaders::VERSION},
                   {"session", StompHeaders::SESSION},
                   {"receipt-id", StompHeaders::RECEIPT_ID}}
    , m_frame{frame}
{
}

StompCommand StompParser::parseCommand(StompError& ec)
{
    size_t currentPos = m_pos;
    size_t newPos = m_frame.find_first_of('\n', m_pos);

    std::string_view command{m_frame.substr(currentPos, newPos - currentPos)};
    if (auto cmdIt = m_commandsMap.find(command); cmdIt != m_commandsMap.end())
    {
        m_pos = newPos + 1;
        ec = StompError::OK;
        return cmdIt->second;
    }

    ec = StompError::UNDEFINED_COMMAND;
    return StompCommand::SIZE_OF_ENUM;
}

StompFrame::Header StompParser::parseHeader(StompError& ec)
{
    size_t currentPos = m_pos;
    size_t newPos = m_frame.find_first_of('\n', currentPos);

    std::string_view header = m_frame.substr(currentPos, newPos - currentPos);
    if (header.size() == 0)
    {
        ec = StompError::EMPTY_HEADER;
        return StompFrame::Header{StompHeaders::SIZE_OF_ENUM, ""};
    }

    // Parse and detect header key.
    size_t headerDelimPos = header.find_first_of(':');
    if (std::string::npos == headerDelimPos)
    {
        ec = StompError::BAD_HEADER;
        return StompFrame::Header{StompHeaders::SIZE_OF_ENUM, ""};
    }

    std::string_view headerKeyView = header.substr(0, headerDelimPos);
    StompHeaders headerKey{StompHeaders::SIZE_OF_ENUM};
    if (auto it = m_headersMap.find(headerKeyView); it != m_headersMap.end())
    {
        headerKey = it->second;
    }

    if (headerKey == StompHeaders::SIZE_OF_ENUM)
    {
        ec = StompError::EMPTY_HEADER;
        return StompFrame::Header{StompHeaders::SIZE_OF_ENUM, ""};
    }

    std::string_view headerValue{header.substr(headerDelimPos + 1)};
    if (headerValue.size() == 0)
    {
        ec = StompError::EMPTY_HEADER_VALUE;
        return StompFrame::Header{StompHeaders::SIZE_OF_ENUM, ""};
    }

    StompFrame::Header result{headerKey, headerValue};
    m_pos = newPos + 1;
    ec = StompError::OK;
    return result;
}

StompFrame::Body StompParser::parseBody(StompError& ec, std::size_t contentLength)
{
    size_t currentPos = m_pos;
    size_t bodyNewlinePos = m_frame.find_first_of('\n', m_pos);
    if (currentPos != bodyNewlinePos)
    {
        ec = StompError::MISSING_BODY_NEWLINE;
        return std::string_view{};
    }

    // Skip new line character.
    currentPos += 1;

    size_t newPos = contentLength;
    if (newPos >= m_frame.size() || std::string::npos == newPos)
    {
        newPos = m_frame.find_first_of('\0', currentPos);
    }

    if (std::string::npos == newPos)
    {
        ec = StompError::UNTERMINATED_BODY;
        return std::string_view{};
    }

    std::string_view body{m_frame.substr(currentPos, newPos - currentPos)};
    ec = StompError::OK;
    m_pos = newPos + 1;

    if (auto afterBody = m_frame.substr(m_pos); afterBody.size() != 0)
    {
        bool allNewline{true};
        for (auto ch : afterBody)
        {
            if (ch != '\n')
            {
                allNewline = false;
                break;
            }
        }

        if (!allNewline)
        {
            ec = StompError::JUNK_AFTER_BODY;
            return std::string_view{};
        }
    }

    return body;
}

} // namespace NetworkMonitor
