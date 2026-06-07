// Eggs.Test
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <eggs/test.hpp>

// CHECK_CATCHES_AS with a reference ExcType must fail to compile:
// the requires std::is_object_v<ExcType> constraint is not satisfied.
void f()
{
    CHECK_CATCHES_AS(int&, throw 0) {}
}
