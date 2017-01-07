/* Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)

This file is part of the mp++ library.

The mp++ library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The mp++ library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the mp++ library.  If not,
see https://www.gnu.org/licenses/. */

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>

#define MPPP_CUSTOM_NAMESPACE my_custom_mppp
#include <mp++.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

static int ntries = 1000;

using namespace my_custom_mppp;

TEST_CASE("custom namespace")
{
    using integer = mp_integer<1>;
    integer n{123};
    REQUIRE(n == 123);
    integer r, m{2};
    sub(r, n, m);
    REQUIRE(r == 121);
    std::cout << std::hash<integer>{}(n) << '\n';
}
