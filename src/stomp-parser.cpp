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
                    {"CONNECT", StompCommand::COMMENT},
                    {"STOMP", StompCommand::STOMP},
                    {"CONNECTED", StompCommand::CONNECTED},
                    {"MESSAGE", StompCommand::MESSAGE},
                    {"RECEIPT", StompCommand::RECEIPT},
                    {"ERROR", StompCommand::ERROR}}
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
    size_t newPos = m_frame.find_first_of('\n', m_pos);

    std::string_view header = m_frame.substr(currentPos, newPos - currentPos);
    if (header.size() == 0)
    {
		m_pos = newPos + 1;
        ec = StompError::EMPTY_HEADER;
        return StompFrame::Header{};
    }

    size_t headerDelimPos = header.find_first_of(':');
    StompFrame::Header result{header.substr(0, headerDelimPos), header.substr(headerDelimPos + 1)};
    m_pos = newPos + 1;
    ec = StompError::OK;
    return result;
}

StompFrame::Body StompParser::parseBody(StompError& ec)
{
    size_t currentPos = m_pos;
    size_t newPos = m_frame.find_first_of('\0', m_pos);
    std::string_view body{m_frame.substr(currentPos, newPos - currentPos)};
	ec = StompError::OK;
	m_pos = newPos + 1;
	return body;
}

} // namespace NetworkMonitor
