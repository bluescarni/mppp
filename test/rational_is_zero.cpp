// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <tuple>
#include <type_traits>

#include <mp++/mp++.hpp>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

struct is_zero_one_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        rational n;
        REQUIRE(n.is_zero());
        REQUIRE(is_zero(n));
        n._get_num().promote();
        REQUIRE(n.is_zero());
        n._get_den().promote();
        REQUIRE(n.is_zero());
        REQUIRE(is_zero(n));
        n = 1;
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        n._get_num().promote();
        REQUIRE(!n.is_zero());
        n._get_den().promote();
        REQUIRE(!n.is_zero());
        n = -1;
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        n._get_num().promote();
        REQUIRE(!n.is_zero());
        n._get_den().promote();
        REQUIRE(!n.is_zero());
        n = 12;
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        n._get_num().promote();
        REQUIRE(!n.is_zero());
        n._get_den().promote();
        REQUIRE(!n.is_zero());
        n = -12;
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        n._get_num().promote();
        REQUIRE(!n.is_zero());
        n._get_den().promote();
        REQUIRE(!n.is_zero());
    }
};

TEST_CASE("is_zero_one")
{
    tuple_for_each(sizes{}, is_zero_one_tester{});
}
