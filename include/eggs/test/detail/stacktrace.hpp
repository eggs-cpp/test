// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>
#include <version>

#ifdef __cpp_lib_stacktrace
#    include <stacktrace>
#endif

namespace eggs::test::detail {

#ifdef __cpp_lib_stacktrace
using stacktrace = std::stacktrace;
#else
struct stacktrace
{
    static stacktrace current([[maybe_unused]] std::size_t skip = 0)
    {
        return {};
    }

    std::size_t size() const noexcept { return 0; }
};
#endif

} // namespace eggs::test::detail
