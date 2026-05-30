#pragma once

#include <format>
#include <string>
#include <type_traits>

namespace eggs::test::detail {

// True for types whose std::formatter supports the {:?} debug specifier
// (std::string, std::string_view, char, const char*, etc. in C++23).
template <typename T>
concept debug_formattable =
    std::formattable<T, char> &&
    requires(std::formatter<std::remove_cvref_t<T>, char> f) {
        f.set_debug_format();
    };

// Formats a single operand value for the "expanded" diagnostic line.
// String-like types are quoted via {:?} so they read as "hello" == "world".
// Other formattable types use {}.  nullptr_t shows as "nullptr".
// Everything else falls back to "?" (no std::formatter specialisation).
template <typename T>
std::string format_operand(T const& val)
{
    if constexpr (std::is_null_pointer_v<T>)
        return "nullptr";
    else if constexpr (debug_formattable<T>)
        return std::format("{:?}", val);
    else if constexpr (std::formattable<T, char>)
        return std::format("{}", val);
    else
        return "?";
}

// True when format_operand produces something other than "?".
template <typename T>
concept formattable_operand = std::is_null_pointer_v<std::remove_cvref_t<T>> ||
                              std::formattable<std::remove_cvref_t<T>, char>;

// Holds both operands, the operator name, and the boolean result of a
// binary comparison captured via ExprLhs.
// Stored as const references — valid for the lifetime of the enclosing
// CHECK(...) expression, where all temporaries live until the semicolon.
template <typename L, typename R>
struct ExprResult
{
    L const& lhs;
    R const& rhs;
    const char* op;
    bool result;

    explicit operator bool() const noexcept { return result; }
};

// Concept: satisfied iff T is an ExprResult<L,R> (has decomposed operands).
template <typename T>
concept decomposed_expr = requires(T const& e) {
    e.lhs;
    e.rhs;
    e.op;
    { static_cast<bool>(e) } -> std::same_as<bool>;
};

// Captures the left-hand side of a binary expression.
// Each comparison operator returns an ExprResult<L,R>; operator bool() handles
// plain bool-like expressions that were never followed by a comparison.
// Stores lhs as a const reference to avoid a copy on the common passing path.
template <typename L>
struct ExprLhs
{
    L const& lhs;

    explicit operator bool() const noexcept { return static_cast<bool>(lhs); }

// Define all six comparison operators uniformly then clean up the helper macro.
#define EGGS_TEST_DETAIL_BINOP(op, sym)                    \
    template <typename R>                                  \
    auto operator op(R const& rhs) const->ExprResult<L, R> \
    {                                                      \
        return {lhs, rhs, sym, lhs op rhs};                \
    }

    EGGS_TEST_DETAIL_BINOP(==, "==")
    EGGS_TEST_DETAIL_BINOP(!=, "!=")
    EGGS_TEST_DETAIL_BINOP(<, "<")
    EGGS_TEST_DETAIL_BINOP(<=, "<=")
    EGGS_TEST_DETAIL_BINOP(>, ">")
    EGGS_TEST_DETAIL_BINOP(>=, ">=")

#undef EGGS_TEST_DETAIL_BINOP
};

// Entry point used by the CHECK / REQUIRE macros:
//
//   Decomposer{} <= __VA_ARGS__
//
// Operator precedence ensures the <= fires before any equality/comparison
// operator in the user's expression:
//
//   <= is relational (prec. 6);  ==, != are equality (prec. 7, lower);
//   <, <=, >, >= all share prec. 6 with left-to-right associativity.
//
// So `Decomposer{} <= a == b` parses as `(Decomposer{} <= a) == b`,
// yielding ExprLhs<A>{a} and then ExprResult<A,B> via ExprLhs::operator==.
struct Decomposer
{
    template <typename L>
    auto operator<=(L const& lhs) const noexcept -> ExprLhs<L>
    {
        return {lhs};
    }
};

} // namespace eggs::test::detail
