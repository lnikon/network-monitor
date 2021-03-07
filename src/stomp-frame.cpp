#include <network-monitor/stomp-frame.h>
#include <network-monitor/stomp-parser.h>

#include <iostream>

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

StompFrame::StompFrame(StompError& ec, std::string&& frame)
    : m_frame{std::move(frame)}
{
	initialize(ec);
}

StompFrame::StompFrame(const StompFrame& frame)
{
}

StompFrame& StompFrame::operator=(const StompFrame& frame)
{
    if (&frame == this)
    {
        return *this;
    }

    return *this;
}

StompFrame::StompFrame(StompFrame&& frame)
{
}

StompFrame& StompFrame::operator=(StompFrame&& frame)
{
}

StompCommand StompFrame::GetCommand() const
{
    return m_command;
}

std::vector<StompFrame::HeaderCopy> StompFrame::GetHeaders()
{
    std::vector<HeaderCopy> results;
    for (auto& header : m_headers)
    {
        results.emplace_back(HeaderCopy{std::string{header.key.data(), header.key.size()},
                                        std::string{header.value.data(), header.value.size()}});
    }

    return results;
}

std::string StompFrame::GetBody() const
{
    return std::string{m_body.data(), m_body.size()};
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

        headers.push_back(hdr);
    } while (hdr.key.size() != 0);

    Body body = parser.parseBody(ec);
    if (StompError::OK != ec)
    {
        return;
    }

    m_command = cmd;
    m_headers = std::move(headers);
    m_body = body;
}

}; // namespace NetworkMonitor
