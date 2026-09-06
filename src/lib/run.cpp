// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <eggs/test/detail/print.hpp>
#include <eggs/test/detail/registry.hpp>
#include <eggs/test/detail/run_state.hpp>
#include <eggs/test/detail/unwind.hpp>
#include <eggs/test/run.hpp>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <format>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

eggs::test::detail::run_state*&
eggs::test::detail::run_state::_current_ptr() noexcept
{
    static thread_local run_state* tl_current_state = nullptr;
    return tl_current_state;
}

eggs::test::detail::registry::cases_type& eggs::test::detail::registry::cases()
{
    static registry::cases_type cases_;
    return cases_;
}

namespace eggs::test {
namespace detail {
namespace {

// "<passed> passed (<percent>%)", plus " | <failed> failed (<percent>%)"
// when failed != 0. The two percentages always add up to 100.
std::string format_summary(std::size_t passed, std::size_t failed)
{
    auto const total = passed + failed;
    auto const percent_passed = total == 0 ? 100 : passed * 100 / total;

    if (failed == 0) {
        return std::format("{} passed ({}%)", passed, percent_passed);
    }

    return std::format(
        "{} passed ({}%) | {} failed ({}%)", passed, percent_passed, failed,
        100 - percent_passed
    );
}

int run(std::vector<test_entry const*> const& run, bool verbose)
{
    std::size_t cases_passed = 0;
    std::vector<std::string_view> cases_failed;

    for (test_entry const* e : run) {
        detail::println(
            stdout, "[ RUN  ] {} -- {}  [{}:{}]", e->name, e->desc,
            e->loc.file_name(), e->loc.line()
        );

        run_state state;
        state.verbose = verbose;

        run_state::set_current(&state);
        bool passed = false;
        try {
            e->run(state);
            passed = !state.assertions_failed;
        } catch (detail::unwind const&) {
        } catch (std::exception const& ex) {
            detail::println(stdout, "  EXCEPTION: {}", ex.what());
        } catch (...) {
            detail::println(stdout, "  UNKNOWN EXCEPTION");
        }
        run_state::set_current(nullptr);

        auto const assertions_total =
            state.assertions_passed + state.assertions_failed;
        if (assertions_total == 0) {
            detail::println(
                stdout, "[ {} ] {} -- 0 assertions\n", passed ? "PASS" : "FAIL",
                e->name
            );
        } else {
            detail::println(
                stdout, "[ {} ] {} -- {} assertions: {}\n",
                passed ? "PASS" : "FAIL", e->name, assertions_total,
                detail::format_summary(
                    state.assertions_passed, state.assertions_failed
                )
            );
        }

        if (passed) {
            ++cases_passed;
        } else {
            cases_failed.push_back(e->name);
        }
    }

    // Omit summary if only one test-case.
    auto const cases_total = cases_passed + cases_failed.size();
    if (cases_total != 1) {
        detail::println(
            stdout, "{} test cases: {}{}", cases_total,
            detail::format_summary(cases_passed, cases_failed.size()),
            cases_failed.empty() ? "" : ":"
        );
        for (auto const& e : cases_failed) {
            detail::println(stdout, "- {}", e);
        }
    }

    return cases_failed.empty() ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace
} // namespace detail

int run(run_options opts)
{
    auto const& all_cases = detail::registry::cases();

    std::vector<detail::test_entry const*> selected_cases;
    if (opts.run.empty()) {
        selected_cases.resize(all_cases.size()); // for overwrite
        auto it = selected_cases.begin();
        for (auto const& e : all_cases) {
            *it++ = &e;
        }
    } else {
        selected_cases.reserve(opts.run.size());

        bool any_unknown = false;

        std::unordered_set<std::string_view> seen;
        seen.reserve(opts.run.size());
        for (auto const& name : opts.run) {
            auto const it = all_cases.find(name);
            if (it == all_cases.end()) {
                detail::println(stderr, "error: unknown test case '{}'", name);
                any_unknown = true;

                if (!detail::is_valid_instance_name(name)) {
                    detail::println(
                        stderr, "warning: '{}' is not a valid test case name",
                        name
                    );
                }
            } else if (!seen.insert(name).second) {
                detail::println(
                    stderr, "warning: duplicate test case '{}'", name
                );
            } else {
                selected_cases.push_back(&*it);
            }
        }

        // TODO: consider executing known test cases instead of failing
        if (any_unknown) return EXIT_FAILURE;
    }

    if (opts.list) {
        for (auto const* e : selected_cases) {
            detail::println(stdout, "{}", e->name);
        }
        return EXIT_SUCCESS;
    }

    return detail::run(selected_cases, opts.verbose);
}

} // namespace eggs::test
