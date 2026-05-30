// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <eggs/test/detail/decompose.hpp>

#include <format>

// Three user-defined types that exercise the three branches of format_operand:
//
//   udt::unformattable     — no std::formatter; format_operand returns "?"
//   udt::formattable       — std::formatter with {}, no set_debug_format()
//   udt::debug_formattable — std::formatter with both {} and {:?}
//
// All three have operator== and operator< so they can be used with every
// relational operator in CHECK / REQUIRE.

namespace udt {

struct unformattable
{
    int val;
    bool operator==(unformattable const&) const noexcept = default;

    bool operator<(unformattable const& o) const noexcept
    {
        return val < o.val;
    }
};

struct formattable
{
    int val;
    bool operator==(formattable const&) const noexcept = default;

    bool operator<(formattable const& o) const noexcept { return val < o.val; }
};

struct debug_formattable
{
    int val;
    bool operator==(debug_formattable const&) const noexcept = default;

    bool operator<(debug_formattable const& o) const noexcept
    {
        return val < o.val;
    }
};

} // namespace udt

// std::formatter for udt::formattable.
// Deliberately does NOT inherit from std::formatter<std::string> (which carries
// set_debug_format()) so that this type satisfies formattable but NOT
// debug_formattable.
template <>
struct std::formatter<udt::formattable>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(udt::formattable const& v, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "formattable({})", v.val);
    }
};

// std::formatter for udt::debug_formattable.
// Supports both {} and {:?}.  The {:?} path (set via set_debug_format()) wraps
// the output in quotes so format_operand can distinguish the two paths.
template <>
struct std::formatter<udt::debug_formattable>
{
    bool debug_ = false;

    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == '?') {
            debug_ = true;
            ++it;
        }
        return it;
    }

    template <typename FormatContext>
    auto format(udt::debug_formattable const& v, FormatContext& ctx) const
    {
        if (debug_)
            return std::format_to(
                ctx.out(), "\"debug_formattable({})\"", v.val
            );
        return std::format_to(ctx.out(), "debug_formattable({})", v.val);
    }

    constexpr void set_debug_format() noexcept { debug_ = true; }
};

// Verify that each type sits in exactly the right category.
// The key invariant is that udt::formattable must NOT accidentally satisfy
// debug_formattable (e.g. by inheriting from std::formatter<std::string>).
static_assert(!std::formattable<udt::unformattable, char>);
static_assert(!eggs::test::detail::debug_formattable<udt::unformattable>);

static_assert(std::formattable<udt::formattable, char>);
static_assert(!eggs::test::detail::debug_formattable<udt::formattable>);

static_assert(std::formattable<udt::debug_formattable, char>);
static_assert(eggs::test::detail::debug_formattable<udt::debug_formattable>);
