// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <eggs/test/detail/noinline.hpp>
#include <eggs/test/detail/registry.hpp>
#include <eggs/test/detail/run_state.hpp>

#include <cstddef>
#include <exception>
#include <source_location>
#include <type_traits>

namespace eggs::test::detail {

EGGS_TEST_NOINLINE void check_failed(
    const char* expr, std::source_location const& loc, std::size_t entry_depth
);

EGGS_TEST_NOINLINE void throws_failed(
    const char* expr, std::source_location const& loc, std::size_t entry_depth
);

EGGS_TEST_NOINLINE void throws_as_failed(
    const char* expr, const char* exc_type, std::exception_ptr threw,
    std::source_location const& loc, std::size_t entry_depth
);

EGGS_TEST_NOINLINE void nothrow_failed(
    const char* expr, std::exception_ptr threw, std::source_location const& loc,
    std::size_t entry_depth
);

template <typename Fn>
EGGS_TEST_NOINLINE inline bool check_throws(
    Fn fn, const char* expr, run_state& s, std::source_location const& loc
)
{
    try {
        fn();
    } catch (require_failed const&) {
        throw;
    } catch (...) {
        ++s.assertions_passed;
        return true;
    }

    ++s.assertions_failed;
    throws_failed(expr, loc, s.entry_depth);
    return false;
}

template <typename ExcType, typename Fn>
    requires std::is_object_v<ExcType>
EGGS_TEST_NOINLINE inline std::exception_ptr check_throws_as(
    Fn fn, const char* expr, run_state& s, const char* exc_type,
    std::source_location const& loc
)
{
    std::exception_ptr threw = nullptr;
    try {
        fn();
    } catch (require_failed const&) {
        throw;
    } catch (ExcType const&) {
        ++s.assertions_passed;
        return std::current_exception();
    } catch (...) {
        threw = std::current_exception();
    }

    ++s.assertions_failed;
    if (threw)
        throws_as_failed(expr, exc_type, threw, loc, s.entry_depth);
    else
        throws_failed(expr, loc, s.entry_depth);
    return nullptr;
}

template <typename Fn>
EGGS_TEST_NOINLINE inline bool check_nothrow(
    Fn fn, const char* expr, run_state& s, std::source_location const& loc
)
{
    std::exception_ptr threw = nullptr;
    try {
        fn();

        ++s.assertions_passed;
        return true;
    } catch (require_failed const&) {
        throw;
    } catch (...) {
        threw = std::current_exception();
    }

    ++s.assertions_failed;
    nothrow_failed(expr, threw, loc, s.entry_depth);
    return false;
}

} // namespace eggs::test::detail
