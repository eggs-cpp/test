// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "cli.hpp"

#include <eggs/test/detail/print.hpp>

#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <optional>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

namespace eggs::test {
namespace {

// ── Option table ─────────────────────────────────────────────────────────────
//
// flag    — the flag text
// value   - the flag value
// also    — extra form shown alongside in --help:
//             short alias (-x)   → listed before the flag: "-v, --verbose"
// desc    — --help right-column text; empty = hidden from --help (alias or internal flag)

struct opt_spec
{
    std::string_view flag;
    std::string_view value;
    std::string_view also;
    std::span<std::string_view const> desc;
};

#define EGGS_TEST_MAKE_STRING_VIEW_SPAN(...)                     \
    []() -> std::span<std::string_view const> {                  \
        static constexpr std::string_view arr[] = {__VA_ARGS__}; \
        return arr;                                              \
    }()

constexpr opt_spec k_opts[] = {
    // {flag, value, also, desc}
    {"--list",
     {},
     {},
     EGGS_TEST_MAKE_STRING_VIEW_SPAN(
         "list selected test case names and exit;", "respects --run filters"
     )},
    {"--run=",
     "<test_case>",
     {},
     EGGS_TEST_MAKE_STRING_VIEW_SPAN(
         "run only the specified test cases (repeatable).",
         "all test cases will run if omitted"
     )},

    {"--randomize",
     "",
     {},
     EGGS_TEST_MAKE_STRING_VIEW_SPAN(
         "shuffle the run list using a random seed"
     )},
    {"--randomize",
     "<seed>",
     {},
     EGGS_TEST_MAKE_STRING_VIEW_SPAN(
         "shuffle the run list using the given seed"
     )},
};

#undef EGGS_TEST_MAKE_SPAN

// ── Help printing ────────────────────────────────────────────────────────────

// description starts at this column (0-based)
static constexpr std::size_t k_desc_col = 29u;

void print_help()
{
    detail::print(
        stdout, "Usage: <test-executable> [options]\n"
                "\n"
                "Options:\n"
    );

    for (opt_spec const& opt : k_opts) {
        if (opt.desc.empty()) continue;

        // Build left-column display string from flag + value + also.
        auto disp = std::string(opt.flag) += opt.value;
        if (!opt.also.empty()) {
            // "-h, --help"
            disp = std::format("{}, {}", opt.also, disp);
        }

        bool first = true;
        for (auto const& line : opt.desc) {
            if (first) {
                // Print left column + description.
                if (2 + disp.size() <= k_desc_col) {
                    // Fits on one line. Pad left column to (k_desc_col - 2)
                    detail::println(
                        stdout, "  {:<{}} {}", disp, k_desc_col - 2, line
                    );
                } else {
                    // Too long. Print option name, wrap to next line, pad description
                    detail::println(stdout, "  {}", disp);
                    detail::println(stdout, "{:>{}} {}", "", k_desc_col, line);
                }
                first = false;
            } else {
                // Subsequent lines are always padded to match the description column
                detail::println(stdout, "{:>{}} {}", "", k_desc_col, line);
            }
        }
    }
}

std::optional<std::uint32_t> parse_seed(std::string_view value)
{
#ifdef __cpp_lib_to_chars
    std::uint32_t seed = 0;
    auto const [ptr, ec] =
        std::from_chars(value.data(), value.data() + value.size(), seed);
    if (ec != std::errc{} || ptr != value.data() + value.size())
        return std::nullopt;
    return seed;
#else
    // value.data() is null-terminated (argv entry).
    // isdigit guard rejects leading '+'/'-'/whitespace that strtoul accepts.
    if (value.empty() || !std::isdigit(static_cast<unsigned char>(value[0])))
        return std::nullopt;
    char* end = nullptr;
    errno = 0;
    unsigned long const ul = std::strtoul(value.data(), &end, 10);
    if (end != value.data() + value.size() || errno != 0 || ul > 0xFFFF'FFFFul)
        return std::nullopt;
    return static_cast<std::uint32_t>(ul);
#endif
}

} // namespace

// ── Public interface ─────────────────────────────────────────────────────────

parse_result parse_args(int argc, char const* const argv[])
{
    parse_result pr;
    pr.action = parse_action::run;

    for (int i = 1; i < argc; ++i) {
        std::string_view const arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            print_help();

            pr.action = parse_action::exit_success;
            return pr;
        } else if (arg == "--list") {
            pr.opts.list = true;
        } else if (arg == "--randomize") {
            pr.opts.seed = static_cast<std::uint32_t>(std::random_device{}());
        } else if (arg.starts_with("--randomize=")) {
            auto const value = arg.substr(12);

            pr.opts.seed = parse_seed(value);
            if (!pr.opts.seed.has_value()) {
                std::fprintf(
                    stderr, "error: invalid seed '%.*s'\n",
                    static_cast<int>(value.size()), value.data()
                );

                pr.action = parse_action::exit_failure;
                return pr;
            }
        } else if (arg.starts_with("--run=")) {
            auto const name = arg.substr(6);

            pr.opts.run.push_back(name);
        } else {
            detail::println(stderr, "error: unknown argument '{}'", arg);

            pr.action = parse_action::exit_failure;
            return pr;
        }
    }

    if (pr.opts.seed.has_value()) {
        std::fprintf(
            stderr, "Randomized with seed: %u\n",
            static_cast<unsigned>(*pr.opts.seed)
        );
    }

    return pr;
}

} // namespace eggs::test
