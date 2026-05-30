// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <eggs/test.hpp>

#include "support/udt.hpp"

// This executable exits 1.  CTest marks it WILL_FAIL.
// Each test case produces specific output verified via PASS_REGEX / FAIL_REGEX.

// --- udt::unformattable: both operands are "?", expanded line suppressed ---

TEST_CASE(
    udt_unformattable_fail, "unformattable failure suppresses the expanded line"
)
{
    udt::unformattable a{1}, b{2};
    CHECK(a == b); // no expanded line — both sides are "?"
}

// --- udt::formattable: operands shown via {} ---

TEST_CASE(
    udt_formattable_fail, "formattable failure shows operands formatted with {}"
)
{
    udt::formattable a{1}, b{2};
    CHECK(a == b); // expanded: formattable(1) == formattable(2)
}

// --- udt::debug_formattable: operands shown via {:?} (quoted) ---

TEST_CASE(
    udt_debug_formattable_fail,
    "debug_formattable failure shows operands formatted with {:?}"
)
{
    udt::debug_formattable a{1}, b{2};
    CHECK(a == b); // expanded: "debug_formattable(1)" == "debug_formattable(2)"
}
