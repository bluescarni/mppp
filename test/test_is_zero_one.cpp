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
#include <tuple>
#include <type_traits>

#include <mp++.hpp>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;
using namespace mppp::mppp_impl;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

struct is_zero_one_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        integer n;
        REQUIRE(n.is_zero());
        REQUIRE(is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(!is_one(n));
        n.promote();
        REQUIRE(n.is_zero());
        REQUIRE(is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(!is_one(n));
        n = 1;
        REQUIRE(n.is_static());
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(n.is_one());
        REQUIRE(is_one(n));
        n.promote();
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(n.is_one());
        REQUIRE(is_one(n));
        n = -1;
        REQUIRE(n.is_static());
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(!is_one(n));
        n.promote();
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(!is_one(n));
        n = 12;
        REQUIRE(n.is_static());
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(!is_one(n));
        n.promote();
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(!is_one(n));
        n = -12;
        REQUIRE(n.is_static());
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(!is_one(n));
        n.promote();
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(!is_one(n));
    }
};

TEST_CASE("is_zero_one")
{
    tuple_for_each(sizes{}, is_zero_one_tester{});
}
