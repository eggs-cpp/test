// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <eggs/test/detail/checks.hpp>
#include <eggs/test/detail/decompose.hpp>
#include <eggs/test/detail/registry.hpp>
#include <eggs/test/detail/run_state.hpp>
#include <eggs/test/detail/runner.hpp>
#include <eggs/test/detail/stacktrace.hpp>

#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <string_view>
#include <unordered_set>
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
// Uses #__VA_ARGS__ so that expressions containing commas (e.g. template
// arguments) stringify correctly.
#define CHECK(...)                                                      \
    ::eggs::test::detail::do_check(                                     \
        ::eggs::test::detail::Decomposer{} <= __VA_ARGS__, #__VA_ARGS__ \
    )

// REQUIRE(expr)
//
// Identical to CHECK but also throws eggs::test::detail::require_failed on
// failure, which is caught by the runner to stop execution of the current test
// case while allowing subsequent cases to continue.
#define REQUIRE(...)                                                    \
    ::eggs::test::detail::do_require(                                   \
        ::eggs::test::detail::Decomposer{} <= __VA_ARGS__, #__VA_ARGS__ \
    )

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
inline int run(run_options opts = {})
{
    auto const& all_cases = detail::registry::cases();

    std::vector<detail::test_entry> selected_cases;
    selected_cases.reserve(opts.run.size());
    if (opts.run.empty()) {
        selected_cases.assign(all_cases.begin(), all_cases.end());
    } else {
        bool any_unknown = false;

        std::unordered_set<std::string_view> seen;
        seen.reserve(opts.run.size());
        for (auto const& name : opts.run) {
            auto const it = all_cases.find(name);
            if (it == all_cases.end()) {
                detail::println(stderr, "error: unknown test case '{}'", name);
                any_unknown = true;
            } else if (!seen.insert(name).second) {
                detail::println(
                    stderr, "warning: duplicate test case '{}'", name
                );
            } else {
                selected_cases.push_back(*it);
            }
        }

        // TODO: consider executing known test cases instead of failing
        if (any_unknown) return EXIT_FAILURE;
    }

    if (opts.list) {
        for (auto const& e : selected_cases) {
            detail::println(stdout, "{}", e.name);
        }
        return EXIT_SUCCESS;
    }

    return detail::registry::run(selected_cases);
}

} // namespace eggs::test
