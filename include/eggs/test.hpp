// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
#    if defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL
#        error "MSVC preprocessor conformance mode required: /Zc:preprocessor"
#    endif
#endif

#include <eggs/test/detail/checks.hpp>
#include <eggs/test/detail/registry.hpp>
#include <eggs/test/detail/run_state.hpp>

#include <exception>
#include <source_location>
#include <string_view>
#include <vector>

// TEST_CASE(name, "description")
//
// Defines a struct named `name` with a static run() member and an inline
// static data member whose initialiser registers the test case before main().
// Follow the macro with a function body:
//
//   TEST_CASE(my_test, "adds two integers") {
//       CHECK(1 + 1 == 2);
//   }
//
// All names are fully qualified (::eggs::test::detail::) so the macro is safe
// inside any user namespace.
#define TEST_CASE(name_, desc_)                                                \
    struct name_                                                               \
    {                                                                          \
        static void run();                                                     \
        inline static bool const registered_ = [] {                            \
            ::eggs::test::detail::registry::add({#name_, desc_, &name_::run}); \
            return true;                                                       \
        }();                                                                   \
    };                                                                         \
    void name_::run()

// CHECK(expr)
//
// Evaluates expr (contextually converted to bool).  On failure prints a
// diagnostic with the source location and the textual expression.  When the
// check is inside a helper function (not directly in the TEST_CASE body) the
// diagnostic also includes a stacktrace of the user frames above it.
//
// Returns bool: true if the check passed, false if it failed.
//
// Uses #__VA_ARGS__ so that expressions containing commas (e.g. template
// arguments) stringify correctly.
#define CHECK(...)                                                          \
    (static_cast<bool>(__VA_ARGS__)                                         \
         ? (++::eggs::test::detail::run_state::current().assertions_passed, \
            true)                                                           \
         : (++::eggs::test::detail::run_state::current().assertions_failed, \
            ::eggs::test::detail::check_failed(                             \
                #__VA_ARGS__, ::std::source_location::current(),            \
                ::eggs::test::detail::run_state::current().entry_depth      \
            ),                                                              \
            false))

// REQUIRE(expr)
//
// Identical to CHECK but stops execution of the current test case on failure.
#define REQUIRE(...)                                                           \
    do {                                                                       \
        if (!CHECK(__VA_ARGS__)) throw ::eggs::test::detail::require_failed{}; \
    } while (false)

// CHECK_THROWS(expr)
//
// Evaluates expr and passes if it throws any exception.  Fails with a
// diagnostic if expr completes without throwing.  Returns bool.
#define CHECK_THROWS(...)                             \
    ::eggs::test::detail::check_throws(               \
        [&]() { (void)(__VA_ARGS__); }, #__VA_ARGS__, \
        ::eggs::test::detail::run_state::current(),   \
        ::std::source_location::current()             \
    )

// REQUIRE_THROWS(expr)
//
// Identical to CHECK_THROWS but stops execution of the current test case on failure.
#define REQUIRE_THROWS(...)                               \
    do {                                                  \
        if (!CHECK_THROWS(__VA_ARGS__))                   \
            throw ::eggs::test::detail::require_failed{}; \
    } while (false)

// CHECK_THROWS_AS(ExcType, expr)
//
// Evaluates expr and passes if it throws an exception of type ExcType (or a
// type derived from it).  Fails if nothing is thrown or the thrown type does
// not match.  Returns std::exception_ptr.
//
// ExcType is a single argument; template types containing commas require a
// using-alias.
#define CHECK_THROWS_AS(ExcType_, ...)                         \
    ::eggs::test::detail::check_throws_as<ExcType_>(           \
        [&]() { (void)(__VA_ARGS__); }, #__VA_ARGS__,          \
        ::eggs::test::detail::run_state::current(), #ExcType_, \
        ::std::source_location::current()                      \
    )

// REQUIRE_THROWS_AS(ExcType, expr)
//
// Identical to CHECK_THROWS_AS but stops execution of the current test case
// on failure.
#define REQUIRE_THROWS_AS(ExcType_, ...)                  \
    do {                                                  \
        if (!CHECK_THROWS_AS(ExcType_, __VA_ARGS__))      \
            throw ::eggs::test::detail::require_failed{}; \
    } while (false)

// CHECK_CATCHES_AS(ExcType, expr)
//
// Evaluates expr and passes if it throws an exception of type ExcType (or a
// type derived from it).  Fails if nothing is thrown or the type does not
// match.  The macro is followed by a catch body in which the caught exception
// is accessible as `exc`:
//
//   CHECK_CATCHES_AS(std::runtime_error, f()) {
//       CHECK(std::string_view(exc.what()) == "expected message");
//   }
//
// ExcType is a single argument; template types containing commas require a
// using-alias.
#define CHECK_CATCHES_AS(ExcType_, ...)                      \
    try {                                                    \
        if (auto e = CHECK_THROWS_AS(ExcType_, __VA_ARGS__)) \
            ::std::rethrow_exception(e);                     \
    } catch (::eggs::test::detail::require_failed const&) {  \
        throw;                                               \
    } catch ([[maybe_unused]] ExcType_ const& exc)

// REQUIRE_CATCHES_AS(ExcType, expr)
//
// Identical to CHECK_CATCHES_AS but stops execution of the current test case
// on failure.
#define REQUIRE_CATCHES_AS(ExcType_, ...)                    \
    try {                                                    \
        if (auto e = CHECK_THROWS_AS(ExcType_, __VA_ARGS__)) \
            ::std::rethrow_exception(e);                     \
        throw ::eggs::test::detail::require_failed{};        \
    } catch (::eggs::test::detail::require_failed const&) {  \
        throw;                                               \
    } catch ([[maybe_unused]] ExcType_ const& exc)

// CHECK_NOTHROW(expr)
//
// Evaluates expr and passes if it does not throw.  Fails with a diagnostic if
// any exception escapes.  Returns bool.
#define CHECK_NOTHROW(...)                            \
    ::eggs::test::detail::check_nothrow(              \
        [&]() { (void)(__VA_ARGS__); }, #__VA_ARGS__, \
        ::eggs::test::detail::run_state::current(),   \
        ::std::source_location::current()             \
    )

// REQUIRE_NOTHROW(expr)
//
// Identical to CHECK_NOTHROW but stops execution of the current test case on
// failure.
#define REQUIRE_NOTHROW(...)                              \
    do {                                                  \
        if (!CHECK_NOTHROW(__VA_ARGS__))                  \
            throw ::eggs::test::detail::require_failed{}; \
    } while (false)

namespace eggs::test {

// Options passed to run()
struct run_options
{
    // ordered test case names; empty = run all
    std::vector<std::string_view> run;
    // list matching test case names instead of running them
    bool list = false;
};

// Public entry point — call this from main().
int run(run_options opts = {});

// Full default main: parse_cli + --help handling + run.
// Link Eggs::TestMain to get this wired up automatically, or
// call it from your own main() to add setup before tests run.
int main(int argc, char const* argv[]);

inline int main(int argc, char* argv[])
{
    return main(argc, const_cast<char const**>(argv));
}

} // namespace eggs::test
