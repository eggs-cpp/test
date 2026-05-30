# Eggs.Test
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2026
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.24)
project(Eggs.Test DESCRIPTION "A C++23 test library" LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)

# Options (two spellings: short when top-level, prefixed when a subdirectory)
if(PROJECT_IS_TOP_LEVEL)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

    include(CTest) # option(BUILD_TESTING ...)
    option(ENABLE_INSTALL "Generate install rules" ON)
else()
    option(EGGS_TEST_BUILD_TESTING "Build the test suite" OFF)
    option(EGGS_TEST_ENABLE_INSTALL "Generate install rules" OFF)

    # Map prefixed options back to the canonical names used below
    set(BUILD_TESTING ${EGGS_TEST_BUILD_TESTING})
    set(ENABLE_INSTALL ${EGGS_TEST_ENABLE_INSTALL})
endif()

#
# Library target — header-only INTERFACE library.
# Internal name _eggs_test; downstream CMake code uses the alias Eggs::Test.
#
add_library(_eggs_test INTERFACE)
target_sources(
    _eggs_test
    INTERFACE
        FILE_SET HEADERS
            BASE_DIRS include
            FILES
                include/eggs/test.hpp
                include/eggs/test/detail/decompose.hpp
                include/eggs/test/detail/registry.hpp
                include/eggs/test/detail/checks.hpp
                include/eggs/test/detail/print.hpp
                include/eggs/test/detail/run_state.hpp
                include/eggs/test/detail/runner.hpp
                include/eggs/test/detail/stacktrace.hpp
)
set_target_properties(
    _eggs_test
    PROPERTIES
        EXPORT_NAME Eggs::Test
        VERIFY_INTERFACE_HEADER_SETS ${BUILD_TESTING}
)
target_compile_features(_eggs_test INTERFACE cxx_std_23)

include(Cxx23Stacktrace)
target_link_libraries(_eggs_test INTERFACE ${EGGS_CXX23_STACKTRACE_LIB})

add_library(Eggs::Test ALIAS _eggs_test)

#
# Eggs::TestMain — object library that provides main().
# Link it into any test executable instead of writing your own main.
#
add_library(_eggs_test_main STATIC src/test_main.cpp src/cli.cpp src/cli.hpp)
target_link_libraries(_eggs_test_main PUBLIC _eggs_test)
set_target_properties(_eggs_test_main PROPERTIES EXPORT_NAME TestMain)

add_library(Eggs::TestMain ALIAS _eggs_test_main)

#
# Test suite
#
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

#
# Install rules
#
if(ENABLE_INSTALL)
    include(CMakePackageConfigHelpers)

    install(
        TARGETS _eggs_test _eggs_test_main
        EXPORT EggsTestTargets
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(
        EXPORT EggsTestTargets
        NAMESPACE Eggs::
        FILE eggs.test-targets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/eggs.test
    )

    configure_package_config_file(
        cmake/eggs.test-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/eggs.test-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/eggs.test
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/eggs.test-config-version.cmake
        VERSION 1.0.0
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/eggs.test-config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/eggs.test-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/eggs.test
    )
endif()
