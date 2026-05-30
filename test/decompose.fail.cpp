#include <eggs/test.hpp>

#include <string>

// This executable exits 1.  CTest marks it WILL_FAIL.
// Each test case below produces specific output that the per-case CTest
// entries verify via PASS_REGEX / FAIL_REGEX.

// --- integers: shows numeric values ---

TEST_CASE(decompose_equal_fail, "failed == check prints both operand values")
{
    int a = 42, b = 0;
    CHECK(a == b); // expanded: 42 == 0
}

TEST_CASE(decompose_less_fail, "failed < check prints both operand values")
{
    CHECK(2 < 1); // expanded: 2 < 1
}

// --- strings: quoted in output ---

TEST_CASE(
    decompose_string_fail,
    "failed std::string == check prints both values in quotes"
)
{
    std::string s1 = "hello", s2 = "world";
    CHECK(s1 == s2); // expanded: "hello" == "world"
}

// --- arithmetic in operand: shows evaluated value, not source text ---

TEST_CASE(
    decompose_arithmetic_fail,
    "arithmetic in operand shows the evaluated value on failure"
)
{
    int x = 2;
    CHECK(x + 1 == 5); // expanded: 3 == 5
}

// --- REQUIRE: same decomposition path, stops the test case on failure ---

TEST_CASE(
    require_decompose_fail,
    "failed REQUIRE prints the expanded line and then stops the test case"
)
{
    int a = 42, b = 0;
    REQUIRE(a == b); // expanded: 42 == 0, then throws require_failed
    CHECK(false);    // must not be reached
}

// --- non-formattable types: expanded line is suppressed entirely ---

namespace {

struct opaque
{
    int val;
    bool operator==(opaque const&) const noexcept = default;
};

} // namespace

TEST_CASE(
    decompose_unformattable_fail,
    "non-formattable type skips the expanded line; shows only FAILED: text"
)
{
    opaque a{1}, b{2};
    CHECK(a == b); // no expanded line — both sides are "?"
}
