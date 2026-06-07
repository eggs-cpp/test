# Eggs.Test
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2026
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Helper — creates an example executable and, when BUILD_TESTING is also
# enabled, registers it as a CTest test case.
#
# CASES: ordered list of test case names passed as --run=<name> arguments to
#        keep execution order deterministic.
function(add_example name)
    cmake_parse_arguments(ARG "" "" "CASES" ${ARGN})

    add_executable(eggs.test.example.${name} ${name}.cpp)
    target_link_libraries(
        eggs.test.example.${name}
        PRIVATE Eggs::Test Eggs::TestMain
    )
    set_target_properties(eggs.test.example.${name} PROPERTIES FOLDER "example")

    if(BUILD_TESTING)
        set(_run_args "")
        foreach(_case IN LISTS ARG_CASES)
            list(APPEND _run_args "--run=${_case}")
        endforeach()

        add_test(
            NAME eggs.test.example.${name}
            COMMAND eggs.test.example.${name} ${_run_args}
        )
        set_tests_properties(
            eggs.test.example.${name}
            PROPERTIES LABELS "example"
        )
    endif()
endfunction()

add_example(basic CASES arithmetic string_view containers)
add_example(
    exceptions
    CASES parse_throws parse_throws_as parse_catches_as parse_template_error
)
add_example(helpers CASES helper_check helper_require)
