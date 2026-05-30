// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <eggs/test/detail/decompose.hpp>
#include <eggs/test/detail/print.hpp>
#include <eggs/test/detail/registry.hpp>
#include <eggs/test/detail/run_state.hpp>
#include <eggs/test/detail/stacktrace.hpp>

#include <cstddef>
#include <source_location>

namespace eggs::test::detail {

template <typename Expr>
inline void print_failure(
    Expr const& expr, char const* text, std::source_location const& loc,
    detail::stacktrace const& st
)
{
    detail::println(
        stdout, "  FAILED: {}  [{}:{}]", text, loc.file_name(), loc.line()
    );

    if constexpr (decomposed_expr<Expr>) {
        using Lhs = decltype(expr.lhs);
        using Rhs = decltype(expr.rhs);
        if constexpr (formattable_operand<Lhs> || formattable_operand<Rhs>)
            detail::println(
                stdout, "    expanded: {} {} {}", format_operand(expr.lhs),
                expr.op, format_operand(expr.rhs)
            );
    }

#ifdef __cpp_lib_stacktrace
    auto const& s = run_state::current();
    if (st.size() > s.entry_depth + 1) {
        detail::println(stdout, "  Stacktrace (innermost first):");
        std::size_t const user_frames = st.size() - s.entry_depth - 1;
        for (std::size_t i = 0; i < user_frames; ++i)
            detail::println(stdout, "    #{} {}", i, st[i].description());
    }
#else
    (void)st;
#endif
}

template <typename Expr>
inline void do_check(
    Expr&& expr, char const* text,
    std::source_location loc = std::source_location::current()
)
{
    auto& s = run_state::current();
    if (static_cast<bool>(expr)) {
        ++s.assertions_passed;
        return;
    }
    ++s.assertions_failed;
    print_failure(expr, text, loc, detail::stacktrace::current(1));
}

template <typename Expr>
inline void do_require(
    Expr&& expr, char const* text,
    std::source_location loc = std::source_location::current()
)
{
    auto& s = run_state::current();
    if (static_cast<bool>(expr)) {
        ++s.assertions_passed;
        return;
    }
    ++s.assertions_failed;
    print_failure(expr, text, loc, detail::stacktrace::current(1));
    throw require_failed{};
}

} // namespace eggs::test::detail
