#include <boost/test/unit_test.hpp>

#include <network-monitor/stomp-frame.h>
#include <network-monitor/stomp-parser.h>

using NetworkMonitor::StompFrame;
using NetworkMonitor::StompParser;
using NetworkMonitor::StompCommand;
using NetworkMonitor::StompError;

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_SUITE(class_StompParser);

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

BOOST_AUTO_TEST_CASE(parse_empty_headers)
{
	std::string_view header{"\n\n\n\n\n\n"};
	StompParser parser{header};
	StompError ec;

	parser.parseHeader(ec);	
	BOOST_CHECK(StompError::EMPTY_HEADER == ec);
}

BOOST_AUTO_TEST_CASE(parse_valid_header)
{
	std::string_view header{"key1:value2\n"};
	StompParser parser{header};
	StompError ec;

	auto hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK("key1" == hdr.key);
	BOOST_CHECK("value2" == hdr.value);
}

BOOST_AUTO_TEST_CASE(parse_valid_headers)
{
	std::string_view header{"key1:value2\nkey2:value3\nkey3:value4\n\n"};
	StompParser parser{header};
	StompError ec;

	auto hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK("key1" == hdr.key);
	BOOST_CHECK("value2" == hdr.value);

	hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK("key2" == hdr.key);
	BOOST_CHECK("value3" == hdr.value);

	hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK("key3" == hdr.key);
	BOOST_CHECK("value4" == hdr.value);

	hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::EMPTY_HEADER == ec);
}

BOOST_AUTO_TEST_CASE(parse_command_and_headers)
{
	std::string_view header{"MESSAGE\nkey1:value2\nkey2:value3\nkey3:value4\n\n"};
	StompParser parser{header};
	StompError ec;

	auto cmd = parser.parseCommand(ec);
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK(StompCommand::MESSAGE == cmd);

	auto hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK("key1" == hdr.key);
	BOOST_CHECK("value2" == hdr.value);

	hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK("key2" == hdr.key);
	BOOST_CHECK("value3" == hdr.value);

	hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK("key3" == hdr.key);
	BOOST_CHECK("value4" == hdr.value);

	hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::EMPTY_HEADER == ec);
}

BOOST_AUTO_TEST_CASE(parse_valid_body)
{
	std::string_view body{"Any string can be in body!\0"};
	StompParser parser{body};
	StompError ec;
	
	auto bdy = parser.parseBody(ec);
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK(bdy == body);
}

BOOST_AUTO_TEST_CASE(parse_command_and_headers_and_body)
{
	std::string_view header{"MESSAGE\nkey1:value2\nkey2:value3\nkey3:value4\n\nAny string can be in body!\0"};
	StompParser parser{header};
	StompError ec;

	auto cmd = parser.parseCommand(ec);
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK(StompCommand::MESSAGE == cmd);

	auto hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK("key1" == hdr.key);
	BOOST_CHECK("value2" == hdr.value);

	hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK("key2" == hdr.key);
	BOOST_CHECK("value3" == hdr.value);

	hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK("key3" == hdr.key);
	BOOST_CHECK("value4" == hdr.value);

	hdr = parser.parseHeader(ec);	
	BOOST_CHECK(StompError::EMPTY_HEADER == ec);

	std::string_view body{"Any string can be in body!\0"};
	auto bdy = parser.parseBody(ec);
	BOOST_CHECK(StompError::OK == ec);
	BOOST_CHECK(bdy == body);
}

BOOST_AUTO_TEST_SUITE_END(); // class_StompParser 

BOOST_AUTO_TEST_SUITE_END(); // network_monitor

