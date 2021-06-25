// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <tuple>
#include <type_traits>

#include <mp++/rational.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
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
        REQUIRE(!n.is_one());
        REQUIRE(!n.is_negative_one());
        REQUIRE(!is_one(n));
        REQUIRE(!is_negative_one(n));
        n._get_num().promote();
        REQUIRE(n.is_zero());
        n._get_den().promote();
        REQUIRE(n.is_zero());
        REQUIRE(is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(!n.is_negative_one());
        REQUIRE(!is_one(n));
        REQUIRE(!is_negative_one(n));
        n = 1;
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(n.is_one());
        REQUIRE(!n.is_negative_one());
        REQUIRE(is_one(n));
        REQUIRE(!is_negative_one(n));
        n._get_num().promote();
        REQUIRE(!n.is_zero());
        n._get_den().promote();
        REQUIRE(!n.is_zero());
        REQUIRE(n.is_one());
        REQUIRE(!n.is_negative_one());
        REQUIRE(is_one(n));
        REQUIRE(!is_negative_one(n));
        n = -1;
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(n.is_negative_one());
        REQUIRE(!is_one(n));
        REQUIRE(is_negative_one(n));
        n._get_num().promote();
        REQUIRE(!n.is_zero());
        n._get_den().promote();
        REQUIRE(!n.is_zero());
        REQUIRE(!n.is_one());
        REQUIRE(n.is_negative_one());
        REQUIRE(!is_one(n));
        REQUIRE(is_negative_one(n));
        n = 12;
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(!n.is_negative_one());
        REQUIRE(!is_one(n));
        REQUIRE(!is_negative_one(n));
        n._get_num().promote();
        REQUIRE(!n.is_zero());
        n._get_den().promote();
        REQUIRE(!n.is_zero());
        REQUIRE(!n.is_one());
        REQUIRE(!n.is_negative_one());
        REQUIRE(!is_one(n));
        REQUIRE(!is_negative_one(n));
        n = -12;
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_zero());
        REQUIRE(!is_zero(n));
        REQUIRE(!n.is_one());
        REQUIRE(!n.is_negative_one());
        REQUIRE(!is_one(n));
        REQUIRE(!is_negative_one(n));
        n._get_num().promote();
        REQUIRE(!n.is_zero());
        n._get_den().promote();
        REQUIRE(!n.is_zero());
        REQUIRE(!n.is_one());
        REQUIRE(!n.is_negative_one());
        REQUIRE(!is_one(n));
        REQUIRE(!is_negative_one(n));
    }
};

TEST_CASE("is_zero_one")
{
    tuple_for_each(sizes{}, is_zero_one_tester{});
}
