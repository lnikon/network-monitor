#include <boost/test/unit_test.hpp>

#include <network-monitor/stomp-frame.h>
#include <network-monitor/stomp-parser.h>

using NetworkMonitor::StompCommand;
using NetworkMonitor::StompError;
using NetworkMonitor::StompFrame;
using NetworkMonitor::StompParser;

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_SUITE(class_StompParser);

using namespace std::string_literals;

BOOST_AUTO_TEST_CASE(parse_well_formed)
{

    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0"s
    };

    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::OK);
    BOOST_CHECK_EQUAL(frame.GetCommand(), StompCommand::CONNECT);
    BOOST_CHECK_EQUAL(frame.GetHeaderValue(NetworkMonitor::StompHeaders::ACCEPT_VERSION), std::string{"42"});
    BOOST_CHECK_EQUAL(frame.GetHeaderValue(NetworkMonitor::StompHeaders::HOST), std::string{"host.com"});
    BOOST_CHECK_EQUAL(frame.GetBody(), std::string{"Frame body"});
}

BOOST_AUTO_TEST_CASE(parse_valid_command)
{
    std::string_view command{"STOMP"};
    StompParser parser{command};
    StompError ec;

    auto cmd = parser.parseCommand(ec);
    BOOST_CHECK(StompError::OK == ec);
    BOOST_CHECK(StompCommand::STOMP == cmd);
}

BOOST_AUTO_TEST_CASE(parse_invalid_command)
{
    std::string_view command{"ERRORRRR"};
    StompParser parser{command};
    StompError ec;

    parser.parseCommand(ec);
    BOOST_CHECK(StompError::UNDEFINED_COMMAND == ec);
}

BOOST_AUTO_TEST_CASE(parse_two_valid_commands)
{
    std::string_view command{"STOMP\nERROR\n"};
    StompParser parser{command};
    StompError ec;

    auto cmd = parser.parseCommand(ec);
    BOOST_CHECK(StompError::OK == ec);
    BOOST_CHECK(StompCommand::STOMP == cmd);

    cmd = parser.parseCommand(ec);
    BOOST_CHECK(StompError::OK == ec);
    BOOST_CHECK(StompCommand::ERROR == cmd);
}

BOOST_AUTO_TEST_CASE(parse_valid_then_invalid_commands)
{
    std::string_view command{"SUBSCRIBE\nSTOMPPP\n"};
    StompParser parser{command};
    StompError ec;

    auto cmd = parser.parseCommand(ec);
    BOOST_CHECK(StompError::OK == ec);
    BOOST_CHECK(StompCommand::SUBSCRIBE == cmd);

    parser.parseCommand(ec);
    BOOST_CHECK(StompError::UNDEFINED_COMMAND == ec);
}

BOOST_AUTO_TEST_CASE(parse_empty_header)
{
    std::string_view header{"\n"};
    StompParser parser{header};
    StompError ec;

    parser.parseHeader(ec);
    BOOST_CHECK(StompError::EMPTY_HEADER == ec);
}

BOOST_AUTO_TEST_CASE(parse_well_formed_content_length)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:10\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_empty_body)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_empty_body_content_length)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:0\n"
        "\n"
        "\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_empty_headers)
{
    std::string plain {
        "DISCONNECT\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_only_command)
{
    std::string plain {
        "DISCONNECT\n"
        "\n"
        "\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_bad_command)
{
    std::string plain {
        "CONNECTX\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_bad_header)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "login\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);
    BOOST_CHECK(error == StompError::BAD_HEADER);
}

BOOST_AUTO_TEST_CASE(parse_missing_body_newline)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);
    BOOST_CHECK(error == StompError::MISSING_BODY_NEWLINE);
}

BOOST_AUTO_TEST_CASE(parse_missing_last_header_newline)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com"
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_unrecognized_header)
{
    std::string plain {
        "CONNECT\n"
        "bad_header:42\n"
        "host:host.com\n"
        "\n"
        "\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_empty_header_value)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:\n"
        "host:host.com\n"
        "\n"
        "\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);
    BOOST_CHECK(error == StompError::EMPTY_HEADER_VALUE);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_just_command)
{
    std::string plain {
        "CONNECT"
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_newline_after_command)
{
    std::string plain {
        "DISCONNECT\n"
        "\n"
        "version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_double_colon_in_header_line)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42:43\n"
        "host:host.com\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_repeated_headers)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "accept-version:43\n"
        "host:host.com\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_repeated_headers_error_in_second)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "accept-version:\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_unterminated_body)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);
    BOOST_CHECK(error == StompError::UNTERMINATED_BODY);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_unterminated_body_content_length)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:10\n"
        "\n"
        "Frame body"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_junk_after_body)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0\n\njunk\n"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_junk_after_body_content_length)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:10\n"
        "\n"
        "Frame body\0\n\njunk\n"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);
    BOOST_CHECK(error == StompError::JUNK_AFTER_BODY);
}

BOOST_AUTO_TEST_CASE(parse_newlines_after_body)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0\n\n\n"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_newlines_after_body_content_length)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:10\n"
        "\n"
        "Frame body\0\n\n\n"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_content_length_wrong_number)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:9\n" // This is one byte off
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);
}

BOOST_AUTO_TEST_CASE(parse_content_length_exceeding)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:15\n" // Way above the actual body length
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK(error != StompError::OK);

    // Add more checks here.
    // ...
}

BOOST_AUTO_TEST_CASE(parse_required_headers)
{
    StompError error;
    {
        std::string plain {
            "CONNECT\n"
            "\n"
            "\0"s
        };
        StompFrame frame {error, std::move(plain)};
        BOOST_CHECK(error != StompError::OK);

    // Add more checks here.
    // ...
    }

    {
        std::string plain {
            "CONNECT\n"
            "accept-version:42\n"
            "\n"
            "\0"s
        };
        StompFrame frame {error, std::move(plain)};
        BOOST_CHECK(error != StompError::OK);

    // Add more checks here.
    // ...
    }

    {
        std::string plain {
            "CONNECT\n"
            "accept-version:42\n"
            "host:host.com\n"
            "\n"
            "\0"s
        };
        StompFrame frame {error, std::move(plain)};

    // Add more checks here.
    // ...
    }
}

BOOST_AUTO_TEST_SUITE_END(); // class_StompParser

BOOST_AUTO_TEST_SUITE_END(); // network_monitor
