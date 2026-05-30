// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <eggs/test.hpp>

#include "support/udt.hpp"

// --- udt::unformattable (no std::formatter) ---

TEST_CASE(udt_unformattable_equal_pass, "unformattable == check passes")
{
    udt::unformattable a{1}, b{1};
    CHECK(a == b);
}

TEST_CASE(udt_unformattable_less_pass, "unformattable < check passes")
{
    udt::unformattable a{1}, b{2};
    CHECK(a < b);
}

// --- udt::formattable (std::formatter with {}, no set_debug_format()) ---

TEST_CASE(udt_formattable_equal_pass, "formattable == check passes")
{
    udt::formattable a{1}, b{1};
    CHECK(a == b);
}

TEST_CASE(udt_formattable_less_pass, "formattable < check passes")
{
    udt::formattable a{1}, b{2};
    CHECK(a < b);
}

// --- udt::debug_formattable (std::formatter with {} and {:?}) ---

TEST_CASE(udt_debug_formattable_equal_pass, "debug_formattable == check passes")
{
    udt::debug_formattable a{1}, b{1};
    CHECK(a == b);
}

TEST_CASE(udt_debug_formattable_less_pass, "debug_formattable < check passes")
{
    udt::debug_formattable a{1}, b{2};
    CHECK(a < b);
}
