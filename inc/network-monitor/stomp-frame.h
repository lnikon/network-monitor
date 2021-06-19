#ifndef STOMP_FRAME_H
#define STOMP_FRAME_H

#include <cstdint>
#include <ostream>
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

std::string ToString(const StompCommand cmd);

std::ostream& operator<<(std::ostream& ost, const StompCommand cmd);

/*! \brief Available STOMP header from STOMP protocol v1.2.
 */
enum class StompHeaders : uint8_t
{
    CONTENT_LENGTH,
    CONTENT_TYPE,
    RECEIPT,
    ACCEPT_VERSION,
    HOST,
    LOGIN,
    PASSCODE,
    ID,
    DESTINATION,
    ACK,
    VERSION,
    SESSION,
    RECEIPT_ID,

    SIZE_OF_ENUM
};

std::string ToString(const StompHeaders cmd);

std::ostream& operator<<(std::ostream& os, const StompHeaders cmd);

/*! \brief Error codes for the STOMP protocol.
 */
enum class StompError : uint8_t
{
    OK,
    UNDEFINED_COMMAND,
    EMPTY_HEADER,
    BAD_HEADER,
    DEV_ERROR,
    MISSING_BODY_NEWLINE,
    WRONG_CONTENT_LENGTH,
    MISSING_ACCEPT_VERSION,
    MISSING_HOST,
    JUNK_AFTER_BODY,
    UNTERMINATED_BODY,
    EMPTY_HEADER_VALUE,

    SIZE_OF_ENUM
};

std::string ToString(const StompError err);

std::ostream& operator<<(std::ostream& ost, const StompError err);

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
        StompHeaders key{StompHeaders::SIZE_OF_ENUM};
        std::string_view value;
    };

    struct HeaderCopy
    {
        StompHeaders key;
        std::string value;

        HeaderCopy(StompHeaders key, std::string value)
            : key(key)
            , value(value)
        {
        }
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

    /*! \brief Construct STOMP frame from frame parts.
     *
     * The result of the operation is stored in the error code.
     */
    StompFrame(StompError& ec,
               const StompCommand cmd,
               const std::vector<HeaderCopy>& headers,
               const Body& body);

    /*! \brief Construct STOMP frame from a string. The string is moved.
     *
     * The result of the operation is stored in the error code.
     */
    StompFrame(StompError& ec, std::string&& frame);

    /*! \brief The copy constructor.
     */
    StompFrame(const StompFrame& frame) = default;

    /*! \brief The copy assingment operator.
     */
    StompFrame& operator=(const StompFrame& frame) = default;

    /*! \brief The move constructor.
     */
    StompFrame(StompFrame&& frame) = default;

    /*! \brief The move assingment operator.
     */
    StompFrame& operator=(StompFrame&& frame) = default;

    StompCommand GetCommand() const;

    HeaderCopy GetHeader(const StompHeaders hdr);

    std::string GetHeaderValue(const StompHeaders hdr);

    std::vector<HeaderCopy> GetHeaders();

    std::string GetBody() const;

    std::string String() const;

private:
    void initialize(StompError& ec);

private:
    std::string m_frame;
    StompCommand m_command;
    std::vector<Header> m_headers;
    Body m_body;
};

} // namespace NetworkMonitor

#endif // STOMP_FRAME_H
