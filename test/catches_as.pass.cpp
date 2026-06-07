// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <eggs/test.hpp>

#include <stdexcept>
#include <string_view>

namespace {
struct my_error
{
    int value = 0;
};

template <typename T, typename U>
struct typed_error
{
    T first;
    U second;
};

void throw_typed_error()
{
    throw typed_error<int, int>{1, 2};
}
} // namespace

TEST_CASE(
    check_catches_as_int,
    "CHECK_CATCHES_AS passes and binds int exception as exc"
)
{
    CHECK_CATCHES_AS(int, throw 42)
    {
        CHECK(exc == 42);
    }
}

TEST_CASE(
    require_catches_as_int,
    "REQUIRE_CATCHES_AS passes without stopping the test"
)
{
    REQUIRE_CATCHES_AS(int, throw 42)
    {
        CHECK(exc == 42);
    }
    CHECK(true);
}

TEST_CASE(
    check_catches_as_std_exception,
    "CHECK_CATCHES_AS passes and binds std exception as exc"
)
{
    CHECK_CATCHES_AS(std::runtime_error, throw std::runtime_error{"oops"})
    {
        CHECK(std::string_view(exc.what()) == "oops");
    }
}

TEST_CASE(
    require_catches_as_std_exception,
    "REQUIRE_CATCHES_AS passes without stopping the test"
)
{
    REQUIRE_CATCHES_AS(std::runtime_error, throw std::runtime_error{"oops"})
    {
        CHECK(std::string_view(exc.what()) == "oops");
    }
    CHECK(true);
}

TEST_CASE(
    check_catches_as_derived_std_exception,
    "CHECK_CATCHES_AS passes for derived std type"
)
{
    CHECK_CATCHES_AS(std::exception, throw std::runtime_error{"oops"})
    {
        CHECK(std::string_view(exc.what()) == "oops");
    }
}

TEST_CASE(
    require_catches_as_derived_std_exception,
    "REQUIRE_CATCHES_AS passes without stopping the test"
)
{
    REQUIRE_CATCHES_AS(std::exception, throw std::runtime_error{"oops"})
    {
        CHECK(std::string_view(exc.what()) == "oops");
    }
    CHECK(true);
}

TEST_CASE(
    check_catches_as_custom_exception,
    "CHECK_CATCHES_AS passes for exact custom type"
)
{
    CHECK_CATCHES_AS(my_error, throw my_error{})
    {
        CHECK(exc.value == 0);
    }
}

TEST_CASE(
    require_catches_as_custom_exception,
    "REQUIRE_CATCHES_AS passes without stopping the test"
)
{
    REQUIRE_CATCHES_AS(my_error, throw my_error{})
    {
        CHECK(exc.value == 0);
    }
    CHECK(true);
}

TEST_CASE(
    check_catches_as_empty_body,
    "CHECK_CATCHES_AS with empty body behaves like CHECK_THROWS_AS"
)
{
    CHECK_CATCHES_AS(std::runtime_error, throw std::runtime_error{"x"}) {}
}

TEST_CASE(
    require_catches_as_empty_body,
    "REQUIRE_CATCHES_AS with empty body does not stop the test"
)
{
    REQUIRE_CATCHES_AS(std::runtime_error, throw std::runtime_error{"x"}) {}
    CHECK(true);
}

TEST_CASE(
    check_catches_as_parenthesized,
    "CHECK_CATCHES_AS accepts a parenthesized type argument"
)
{
    CHECK_CATCHES_AS((std::runtime_error), throw std::runtime_error{"oops"})
    {
        CHECK(std::string_view(exc.what()) == "oops");
    }
}

TEST_CASE(
    check_catches_as_template_commas,
    "CHECK_CATCHES_AS handles parenthesized template types with commas"
)
{
    CHECK_CATCHES_AS((typed_error<int, int>), throw_typed_error())
    {
        CHECK(exc.first == 1);
        CHECK(exc.second == 2);
    }
}

TEST_CASE(
    require_catches_as_template_commas,
    "REQUIRE_CATCHES_AS passes without stopping the test"
)
{
    REQUIRE_CATCHES_AS((typed_error<int, int>), throw_typed_error())
    {
        CHECK(exc.first == 1);
        CHECK(exc.second == 2);
    }
    CHECK(true);
}
