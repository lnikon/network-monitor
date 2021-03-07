#ifndef STOMP_FRAME_H
#define STOMP_FRAME_H

#include <cstdint>
#include <string>
#include <vector>

namespace NetworkMonitor
{

/*! \brief Available STOMP commands from STOMP protocol v1.2.
 */
enum class StompCommand : uint8_t
{
    SEND,
    SUBSCRIBE,
    UNSUBSCRIBE,
    BEGIN,
    COMMENT,
    ABORT,
    ACK,
    NACK,
    DISCONNECT,
    CONNECT,
    STOMP,
    CONNECTED,
    MESSAGE,
    RECEIPT,
    ERROR,

    SIZE_OF_ENUM
};

/*! \brief Available STOMP header from STOMP protocol v1.2.
 */
enum class StompHeaders : uint8_t
{
    HEADER_LENGTH,
    CONTENT_TYPE,
    RECEIPT,

    SIZE_OF_ENUM
};

/*! \brief Error codes for the STOMP protocol.
 */
enum class StompError : uint8_t
{
    OK,
	UNDEFINED_COMMAND,
	EMPTY_HEADER,
    DEV_ERROR,

    SIZE_OF_ENUM
};

/*! \brief STOMP frame representation, supporting STOMP v1.2.
 */
class StompFrame
{
public:
    /*! \brief Stomp header. Stores key and value.
     *  Value MAY be empty.
     */
    struct Header
    {
        std::string_view key;
        std::string_view value;
    };

	using Body = std::string_view;

    /*! \brief Default constructor. Corresponds to an empty, invalid STOMP frame.
     */
    explicit StompFrame();

    /*! \brief Construct STOMP frame from a string. The string is copied.
     *
     * The result of the operation is stored in the error code.
     */
    StompFrame(StompError& ec, const std::string& frame);

    /*! \brief Construct STOMP frame from a string. The string is moved.
     *
     * The result of the operation is stored in the error code.
     */
    StompFrame(StompError& ec, std::string&& frame);

    /*! \brief The copy constructor.
     */
    StompFrame(const StompFrame& frame);

    /*! \brief The copy assingment operator.
     */
    StompFrame& operator=(const StompFrame& frame);

    /*! \brief The move constructor.
     */
    StompFrame(StompFrame&& frame);

    /*! \brief The move assingment operator.
     */
    StompFrame& operator=(StompFrame&& frame);

private:
    std::string m_frame;
	StompCommand m_command;
	std::vector<Header> m_headers;
};

} // namespace NetworkMonitor

#endif // STOMP_FRAME_H
