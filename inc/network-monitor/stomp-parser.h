#ifndef STOMP_PARSER_H
#define STOMP_PARSER_H

#include <network-monitor/stomp-frame.h>
#include <unordered_map>

namespace NetworkMonitor
{

/*! \brief STOMP frame parser, supporting STOMP v1.2.
 */
class StompParser
{
public:
    /*! \brief Constructor. Stores view into STOMP frame.
     */
    explicit StompParser(std::string_view frame);

    /*! \brief Parse is neither copyable non movable.
     */
    StompParser(const StompParser&) = delete;
    StompParser(StompParser&&) = delete;
    StompParser& operator=(const StompParser&) = delete;
    StompParser& operator=(StompParser&&) = delete;

    /*! \brief Parses command from a STOMP frame.
     *  Stores operation result in a error code.
     *
     *  If command is successfully parsed and recognized command enum value is returned,
     *  otherwise return value is undefined.
     */
    StompCommand parseCommand(StompError& ec);

    /*! \brief Parses header from a STOMP frame.
     *  Stores operation result in a error code.
     */
    StompFrame::Header parseHeader(StompError& ec);

    /*! \brief Parses body from a STOMP frame.
     *  Stores operation result in a error code.
     */
    StompFrame::Body parseBody(StompError& ec);

private:
    std::unordered_map<std::string_view, StompCommand> m_commandsMap;
    std::string_view m_frame;
    size_t m_pos{0};
};

} // namespace NetworkMonitor

#endif // STOMP_PARSER_H
