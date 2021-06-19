#include <boost/test/unit_test.hpp>
#include <boost/spirit/include/qi.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

BOOST_AUTO_TEST_SUITE(boost_spirit_parser_test);

BOOST_AUTO_TEST_CASE(basic)
{
    using namespace boost::spirit;
    std::string s{"1231"};
    auto it = s.begin();
    bool match = qi::parse(it, s.end(), ascii::digit);
    std::cout << std::boolalpha << match << '\n';
    if (it != s.end()) {
        auto ss = std::string{it, s.end()};
        std::cout << ss << '\n';
    }
}

BOOST_AUTO_TEST_SUITE_END(); // boost_spirit_parser_test
