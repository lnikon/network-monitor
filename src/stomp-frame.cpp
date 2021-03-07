#include <network-monitor/stomp-frame.h>

namespace NetworkMonitor
{

StompFrame::StompFrame()
{
}

StompFrame::StompFrame(StompError& ec, const std::string& frame)
	: m_frame{frame}
{
	ec = StompError::DEV_ERROR;
}

StompFrame::StompFrame(StompError& ec, std::string&& frame)
	: m_frame{std::move(frame)}
{
	ec = StompError::DEV_ERROR;
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

}; // namespace NetworkMonitor
