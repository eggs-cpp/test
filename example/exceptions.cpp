// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <eggs/test.hpp>

#include <stdexcept>
#include <string>
#include <string_view>

namespace {

int parse_int(std::string_view s)
{
    std::size_t pos = 0;
    int const result = std::stoi(std::string(s), &pos);
    if (pos != s.size()) throw std::invalid_argument{"trailing characters"};
    return result;
}

// A template exception whose two type parameters introduce a comma.
// CHECK_THROWS_AS(parse_error<int, int>, ...) would confuse the preprocessor;
// wrapping in parentheses — (parse_error<int, int>) - hides the comma.
template <typename Input, typename Code>
struct parse_error
{
    Code code;
};

void throw_parse_error()
{
    throw parse_error<int, int>{42};
}

} // namespace

TEST_CASE(parse_throws, "throwing and non-throwing expressions")
{
    CHECK_THROWS(parse_int("abc"));
    CHECK_THROWS(parse_int("1abc"));
    CHECK_NOTHROW(parse_int("42"));
}

TEST_CASE(parse_throws_as, "matching the exception type")
{
    CHECK_THROWS_AS(std::invalid_argument, parse_int("abc"));
    CHECK_THROWS_AS(std::exception, parse_int("abc")); // base class matches too

    CHECK_THROWS_AS(std::out_of_range, parse_int("99999999999999999999"));
}

TEST_CASE(parse_catches_as, "inspecting the caught exception")
{
    CHECK_CATCHES_AS(std::invalid_argument, parse_int("1abc"))
    {
        CHECK(std::string_view(exc.what()) == "trailing characters");
    }
}

TEST_CASE(
    parse_template_error, "parenthesized template exception types with commas"
)
{
    // Wrapping the type in parentheses hides the comma from the preprocessor.
    CHECK_THROWS_AS((parse_error<int, int>), throw_parse_error());
    CHECK_CATCHES_AS((parse_error<int, int>), throw_parse_error())
    {
        CHECK(exc.code == 42);
    }
}
