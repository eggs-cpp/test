# Eggs.Test
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2026
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# ── Verify interface header sets ────────────────────────────────────────────

add_test(
    NAME eggs.test.verify_headers
    COMMAND
        ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target
        all_verify_interface_header_sets
)

# Helper — creates a test executable and registers it with CTest.
# PASS_REGEX: if given, the test passes iff its output matches the regex
#             (exit code is ignored).  Use this for tests that are expected
#             to fail or crash and whose output must contain a specific pattern.
function(add_lib_test name)
    cmake_parse_arguments(
        ARG
        "WILL_FAIL"
        "FAIL_REGEX;PASS_REGEX"
        "ARGS;SOURCES"
        ${ARGN}
    )

    add_executable(_eggs_test.${name} ${ARG_SOURCES})
    target_link_libraries(_eggs_test.${name} PRIVATE Eggs::Test Eggs::TestMain)

    add_test(NAME eggs.test.${name} COMMAND _eggs_test.${name} ${ARG_ARGS})

    set_tests_properties(
        eggs.test.${name}
        PROPERTIES FOLDER "test" LABELS "unit"
    )
    if(ARG_FAIL_REGEX)
        set_tests_properties(
            eggs.test.${name}
            PROPERTIES FAIL_REGULAR_EXPRESSION "${ARG_FAIL_REGEX}"
        )
    endif()
    if(ARG_PASS_REGEX)
        set_tests_properties(
            eggs.test.${name}
            PROPERTIES PASS_REGULAR_EXPRESSION "${ARG_PASS_REGEX}"
        )
    endif()
    if(ARG_PASS_REGEX OR ARG_FAIL_REGEX)
        # PASS/FAIL_REGEX ignore exit code; add a companion test that only
        # checks the exit code.
        add_test(
            NAME eggs.test.${name}.exit
            COMMAND _eggs_test.${name} ${ARG_ARGS}
        )
        set_tests_properties(
            eggs.test.${name}.exit
            PROPERTIES FOLDER "test" LABELS "unit" WILL_FAIL "${ARG_WILL_FAIL}"
        )
    elseif(ARG_WILL_FAIL)
        set_tests_properties(eggs.test.${name} PROPERTIES WILL_FAIL TRUE)
    endif()
endfunction()

# --- registration tests ------------------------------------------------------

# Multiple TEST_CASEs in one TU all run.
add_lib_test(
    registration
    SOURCES registration.cpp
    PASS_REGEX "3 of 3 test cases passed"
)

# Test cases defined across two translation units both register and run.
add_lib_test(
    registration.multi_tu
    SOURCES registration.multi_tu.a.cpp registration.multi_tu.b.cpp
    PASS_REGEX "2 of 2 test cases passed"
)

# --- passing tests -----------------------------------------------------------

# CHECK with true expressions must exit 0.
add_lib_test(check.pass SOURCES check.pass.cpp)

# REQUIRE with true expressions must exit 0 and not stop execution.
add_lib_test(require.pass SOURCES require.pass.cpp)

# CHECK/REQUIRE called from a helper function must compile and run correctly.
# The .pass test exercises the non-failure path (no stacktrace printed).
# The .check.fail/.require.fail tests exercise the failure path, verifying that
# the specific helper function name appears in the printed stacktrace frames.
add_lib_test(stacktrace.pass SOURCES stacktrace.pass.cpp)
if(EGGS_HAS_CXX23_STACKTRACE)
    add_lib_test(
        stacktrace.check.fail
        SOURCES stacktrace.check.fail.cpp
        PASS_REGEX "$<IF:$<CONFIG:Debug>,failing_check_helper,FAILED:>"
        WILL_FAIL
    )
    add_lib_test(
        stacktrace.require.fail
        SOURCES stacktrace.require.fail.cpp
        PASS_REGEX "$<IF:$<CONFIG:Debug>,failing_require_helper,FAILED:>"
        WILL_FAIL
    )
endif()

# --- failing tests -----------------------------------------------------------

# CHECK with a false expression must cause the test case to be reported as
# failed, and the stringified expression must appear in the output.
add_lib_test(
    check.fail
    SOURCES check.fail.cpp
    PASS_REGEX "FAILED: 1 \\+ 2 == 4"
    WILL_FAIL
)

# REQUIRE with a false expression must fail the test case and stop executing
# the rest of it; the stringified expression must appear in the output.
add_lib_test(
    require.fail
    SOURCES require.fail.cpp
    PASS_REGEX "FAILED: 1 \\+ 2 == 4"
    WILL_FAIL
)

# An exception leaked from a test case must be caught and reported as a failure.
add_lib_test(
    exceptions.std
    SOURCES exceptions.fail.cpp
    ARGS "--run=leaked_std_exception"
    PASS_REGEX "EXCEPTION: deliberate failure"
    WILL_FAIL
)
add_lib_test(
    exceptions.unknown
    SOURCES exceptions.fail.cpp
    ARGS "--run=leaked_unknown_exception"
    PASS_REGEX "UNKNOWN EXCEPTION"
    WILL_FAIL
)

# --- cli tests -----------------------------------------------------------

# --help must print usage.
add_lib_test(
    cli.help.pass
    SOURCES check.pass.cpp
    ARGS "--help"
    PASS_REGEX "Usage: <test-executable> \\[options\\]"
)

# -h must print usage.
add_lib_test(
    cli.help_alias.pass
    SOURCES check.pass.cpp
    ARGS "-h"
    PASS_REGEX "Usage: <test-executable> \\[options\\]"
)

# --unknown must print unknown argument.
add_lib_test(
    cli.unknown.fail
    SOURCES check.pass.cpp
    ARGS "--unknown"
    PASS_REGEX "unknown argument '--unknown'"
    WILL_FAIL
)

# -u must print unknown argument.
add_lib_test(
    cli.unknown_alias.fail
    SOURCES check.pass.cpp
    ARGS "-u"
    PASS_REGEX "unknown argument '-u'"
    WILL_FAIL
)

# --list must print exactly the registered test case names, one per line.
string(
    CONCAT _list_regex
    "^"
    "registration_(one|two|three)\n"
    "registration_(one|two|three)\n"
    "registration_(one|two|three)\n?$"
)
add_lib_test(
    cli.list.pass
    SOURCES registration.cpp
    ARGS "--list"
    PASS_REGEX "${_list_regex}"
)

# --list --run=<name> must list only the matching test case.
add_lib_test(
    cli.list.run.pass
    SOURCES registration.cpp
    ARGS "--list" "--run=registration_one"
    PASS_REGEX "^registration_one\n?$"
    FAIL_REGEX "registration_two;registration_three"
)

# --list --run=<a> --run=<b> must list names in --run order.
string(
    CONCAT _list_run_order_regex
    "^"
    "registration_three\n"
    "registration_one\n?$"
)
add_lib_test(
    cli.list.run.order.pass
    SOURCES registration.cpp
    ARGS "--list" "--run=registration_three" "--run=registration_one"
    PASS_REGEX "${_list_run_order_regex}"
    FAIL_REGEX "registration_two"
)

# --run=<name> must not run other test cases.
add_lib_test(
    cli.run.pass
    SOURCES registration.cpp
    ARGS "--run=registration_one"
    PASS_REGEX "registration_one"
    FAIL_REGEX "registration_two;registration_three"
)

# --run=<name> --run=<name> must warn and run the case only once.
string(
    CONCAT _run_dedup_regex
    "warning: duplicate test case 'registration_one'"
    "(\n[^\n]*)*\n"
    "1 of 1 test cases passed"
)
add_lib_test(
    cli.run.dedup.pass
    SOURCES registration.cpp
    ARGS "--run=registration_one" "--run=registration_one"
    PASS_REGEX "${_run_dedup_regex}"
)

# --run=<a> --run=<b> must run cases in the given order.
string(
    CONCAT _run_order_regex
    "\\[ RUN  \\] registration_three[^\n]*"
    "(\n[^\n]+)+\n"
    "\\[ RUN  \\] registration_one"
)
add_lib_test(
    cli.run.order.pass
    SOURCES registration.cpp
    ARGS "--run=registration_three" "--run=registration_one"
    PASS_REGEX "${_run_order_regex}"
    FAIL_REGEX "registration_two"
)

# --run=<unknown> must print an error.
add_lib_test(
    cli.run.unknown.fail
    SOURCES registration.cpp
    ARGS "--run=unknown"
    PASS_REGEX "error: unknown test case 'unknown'"
    FAIL_REGEX "\\[ RUN  \\]"
    WILL_FAIL
)

# --run=<unknown_a> --run=<unknown_b> must report all invalid names.
string(
    CONCAT _run_unknown_multi_regex
    "^"
    "error: unknown test case 'unknown_a'\n"
    "error: unknown test case 'unknown_b'\n?$"
)
add_lib_test(
    cli.run.unknown_multi.fail
    SOURCES registration.cpp
    ARGS "--run=unknown_a" "--run=unknown_b"
    PASS_REGEX "${_run_unknown_multi_regex}"
    FAIL_REGEX "\\[ RUN  \\]"
    WILL_FAIL
)

# Randomized order: all passing cases must still pass.
add_test(
    NAME eggs.test.randomize.pass
    COMMAND _eggs_test.check.pass --randomize=0
)
set_tests_properties(
    eggs.test.randomize.pass
    PROPERTIES FOLDER "test" LABELS "unit"
)

# Randomized order: failure is detected regardless of execution order.
add_test(
    NAME eggs.test.randomize.fail
    COMMAND _eggs_test.check.fail --randomize=0
)
set_tests_properties(
    eggs.test.randomize.fail
    PROPERTIES
        FOLDER "test"
        LABELS "unit"
        PASS_REGULAR_EXPRESSION "FAILED: 1 \\+ 2 == 4"
)

# --- cli --randomize tests ---------------------------------------------------

# --randomize (no =) generates a random seed and prints it to stderr.
add_lib_test(
    cli.randomize.seed_printed.pass
    SOURCES check.pass.cpp
    ARGS "--randomize"
    PASS_REGEX "Randomized with seed: [0-9]+"
)

# --randomize=42 prints the exact given seed.
add_lib_test(
    cli.randomize.explicit_seed.pass
    SOURCES check.pass.cpp
    ARGS "--randomize=42"
    PASS_REGEX "Randomized with seed: 42"
)

# seed=0 is valid (uint32 min boundary).
add_lib_test(
    cli.randomize.seed_zero.pass
    SOURCES check.pass.cpp
    ARGS "--randomize=0"
    PASS_REGEX "Randomized with seed: 0"
)

# seed=4294967295 is valid (uint32 max boundary).
add_lib_test(
    cli.randomize.seed_max.pass
    SOURCES check.pass.cpp
    ARGS "--randomize=4294967295"
    PASS_REGEX "Randomized with seed: 4294967295"
)

# --randomize= (empty after =) must be rejected.
add_lib_test(
    cli.randomize.invalid_empty.fail
    SOURCES check.pass.cpp
    ARGS "--randomize="
    PASS_REGEX "error: invalid seed ''"
    WILL_FAIL
)

# --randomize=-1 (negative) must be rejected; from_chars rejects leading '-'
# for unsigned parsing.
add_lib_test(
    cli.randomize.invalid_negative.fail
    SOURCES check.pass.cpp
    ARGS "--randomize=-1"
    PASS_REGEX "error: invalid seed '-1'"
    WILL_FAIL
)

# --randomize=abc (non-numeric) must be rejected.
add_lib_test(
    cli.randomize.invalid_alpha.fail
    SOURCES check.pass.cpp
    ARGS "--randomize=abc"
    PASS_REGEX "error: invalid seed 'abc'"
    WILL_FAIL
)

# --randomize=4294967296 (> uint32 max) must be rejected via
# std::errc::result_out_of_range.
add_lib_test(
    cli.randomize.invalid_overflow.fail
    SOURCES check.pass.cpp
    ARGS "--randomize=4294967296"
    PASS_REGEX "error: invalid seed '4294967296'"
    WILL_FAIL
)

# --randomize=1.5 (decimal) must be rejected via the partial-parse guard
# (ptr != end after from_chars stops at '.').
add_lib_test(
    cli.randomize.invalid_decimal.fail
    SOURCES check.pass.cpp
    ARGS "--randomize=1.5"
    PASS_REGEX "error: invalid seed '1.5'"
    WILL_FAIL
)

# --list --randomize=42 prints the seed even in list mode.
add_lib_test(
    cli.list.randomize.seed_printed.pass
    SOURCES registration.cpp
    ARGS "--list" "--randomize=42"
    PASS_REGEX "Randomized with seed: 42"
)

# --list --randomize=<seed> (no --run) lists ALL cases sorted then shuffled.
# Sorted input: registration_one, registration_three, registration_two.
# With seed=0 the shuffle produces: one, two, three.
string(
    CONCAT _list_randomize_all_regex
    "registration_one\n"
    "registration_three\n"
    "registration_two"
)
add_lib_test(
    cli.list.randomize.pass
    SOURCES registration.cpp
    ARGS "--list" "--randomize=0"
    PASS_REGEX "${_list_randomize_all_regex}"
)

# --list --randomize=<seed> --run=... lists selected cases in shuffled order.
# --run order [one, two, three] shuffled with seed=0 produces: one, three, two.
string(
    CONCAT _list_randomize_run_regex
    "registration_one\n"
    "registration_three\n"
    "registration_two"
)
add_lib_test(
    cli.list.randomize.run.order.pass
    SOURCES registration.cpp
    ARGS
        "--list"
        "--randomize=0"
        "--run=registration_one"
        "--run=registration_two"
        "--run=registration_three"
    PASS_REGEX "${_list_randomize_run_regex}"
    FAIL_REGEX "registration_one\nregistration_two\nregistration_three"
)

# --randomize=<seed> (no --run) runs all cases in the same shuffled order as
# --list predicted.  seed=0: one, two, three.
string(
    CONCAT _run_randomize_all_regex
    "\\[ RUN  \\] registration_one[^\n]*(\n[^\n]+)+\n"
    "\\[ RUN  \\] registration_three[^\n]*(\n[^\n]+)+\n"
    "\\[ RUN  \\] registration_two"
)
add_lib_test(
    cli.run.randomize.pass
    SOURCES registration.cpp
    ARGS "--randomize=0"
    PASS_REGEX "${_run_randomize_all_regex}"
)

# --randomize=<seed> --run=... runs in the shuffled order.
# seed=0: one, three, two — differs from the given --run order one, two, three.
string(
    CONCAT _run_randomize_order_regex
    "\\[ RUN  \\] registration_one[^\n]*(\n[^\n]+)+\n"
    "\\[ RUN  \\] registration_three[^\n]*(\n[^\n]+)+\n"
    "\\[ RUN  \\] registration_two"
)
add_lib_test(
    cli.run.randomize.order.pass
    SOURCES registration.cpp
    ARGS
        "--randomize=0"
        "--run=registration_one"
        "--run=registration_two"
        "--run=registration_three"
    PASS_REGEX "${_run_randomize_order_regex}"
    FAIL_REGEX
        "\\[ RUN  \\] registration_one[^\n]*\n[^\n]*\n\\[ RUN  \\] registration_two"
)
