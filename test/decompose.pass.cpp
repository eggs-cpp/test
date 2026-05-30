#include <eggs/test.hpp>

#include <string>

// --- all six relational operators ---

TEST_CASE(decompose_equal_pass, "decomposed == check passes")
{
    int a = 1, b = 1;
    CHECK(a == b);
}

TEST_CASE(decompose_not_equal_pass, "decomposed != check passes")
{
    CHECK(1 != 2);
}

TEST_CASE(decompose_less_pass, "decomposed < check passes")
{
    CHECK(1 < 2);
}

TEST_CASE(decompose_less_equal_pass, "decomposed <= check passes")
{
    CHECK(1 <= 1);
}

TEST_CASE(decompose_greater_pass, "decomposed > check passes")
{
    CHECK(2 > 1);
}

TEST_CASE(decompose_greater_equal_pass, "decomposed >= check passes")
{
    CHECK(2 >= 2);
}

// --- plain bool (no decomposition) ---

TEST_CASE(decompose_plain_bool, "plain bool expression still works")
{
    CHECK(true);
}

// --- pointer comparisons ---

TEST_CASE(decompose_nullptr_equal, "pointer compared to nullptr passes")
{
    int* p = nullptr;
    CHECK(p == nullptr);
}

TEST_CASE(decompose_pointer_not_null, "non-null pointer != nullptr passes")
{
    int x = 0;
    CHECK(&x != nullptr);
}

// --- string comparisons (quoted in output: "hello" == "world") ---

TEST_CASE(decompose_string_equal_pass, "std::string == check passes")
{
    std::string a = "hello", b = "hello";
    CHECK(a == b);
}

TEST_CASE(decompose_string_less_pass, "std::string < check passes")
{
    std::string a = "alpha", b = "beta";
    CHECK(a < b);
}

// --- non-formattable types: still compiles and passes ---

namespace {

struct opaque
{
    int val;
    bool operator==(opaque const&) const noexcept = default;

    bool operator<(opaque const& o) const noexcept { return val < o.val; }
};

} // namespace

TEST_CASE(
    decompose_unformattable_pass,
    "non-formattable type compiles and passes (no expanded line on failure)"
)
{
    opaque a{1}, b{1};
    CHECK(a == b);
}

// --- arithmetic in operands: shows the evaluated value, not the source text ---

TEST_CASE(
    decompose_arithmetic_lhs,
    "arithmetic expression in lhs shows the evaluated result"
)
{
    int x = 2;
    CHECK(x + 1 == 3); // lhs is captured as 3, not the text "x + 1"
}

// --- compound bool expressions degrade gracefully (no expanded line) ---

TEST_CASE(
    decompose_compound_bool, "compound && expression is treated as a plain bool"
)
{
    int x = 5;
    CHECK(x > 0 && x < 10);
}
