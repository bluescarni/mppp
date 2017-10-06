// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <mp++/detail/mpfr.hpp>
#include <mp++/integer.hpp>
#include <mp++/real.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

using int_t = integer<1>;
using rat_t = rational<1>;

TEST_CASE("real plus")
{
    real r0{123};
    REQUIRE(::mpfr_cmp_ui((+r0).get_mpfr_t(), 123ul) == 0);
    REQUIRE(::mpfr_cmp_ui((+real{123}).get_mpfr_t(), 123ul) == 0);
    std::cout << (real{123} + real{4}) << '\n';
    std::cout << (real{123} + int_t{4}) << '\n';
    std::cout << (int_t{4} + real{123}) << '\n';
    std::cout << (real{123} + rat_t{4}) << '\n';
    std::cout << (rat_t{4} + real{123}) << '\n';
    std::cout << (real{123} + 34u) << '\n';
    std::cout << (36u + real{123}) << '\n';
    std::cout << (real{123} + -34) << '\n';
    std::cout << (-36 + real{123}) << '\n';
    std::cout << (real{123} + true) << '\n';
    std::cout << (false + real{123}) << '\n';
}
