// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
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

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

struct binomial_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = typename rational::int_t;
        // A couple of tests with both integer values.
        REQUIRE(binomial(rational{}, integer{}) == 1);
        REQUIRE(binomial(rational{5}, integer{2}) == 10);
        REQUIRE(binomial(rational{5}, integer{-2}) == 0);
        REQUIRE(binomial(rational{-5}, integer{-2}) == 0);
        REQUIRE(binomial(rational{-5}, integer{2}) == 15);
        REQUIRE(binomial(rational{}, 0) == 1);
        REQUIRE(binomial(rational{5}, 2u) == 10);
        REQUIRE(binomial(rational{5}, static_cast<signed char>(-2)) == 0);
        REQUIRE(binomial(rational{-5}, -2ll) == 0);
        REQUIRE(binomial(rational{-5}, 2ul) == 15);
        // First special case: choose(rational, neg int) is always zero.
        REQUIRE(binomial(rational{5, 2}, integer{-2}) == 0);
        REQUIRE(binomial(rational{-5, 2}, -2l) == 0);
        REQUIRE(binomial(rational{5, 2}, static_cast<short>(-2)) == 0);
        // Second special case: choose(rational, 0) is always one.
        REQUIRE(binomial(rational{5, 2}, integer{0}) == 1);
        REQUIRE(binomial(rational{-5, 2}, 0l) == 1);
        REQUIRE(binomial(rational{5, 2}, static_cast<unsigned short>(0)) == 1);
        // Main case.
        REQUIRE(binomial(rational{5, 2}, integer{2}) == rational{15, 8});
        REQUIRE(binomial(rational{-5, 2}, 2) == rational{35, 8});
        REQUIRE(binomial(rational{3, 4}, 2) == rational{-3, 32});
        REQUIRE(binomial(rational{3, 4}, 10) == -rational{1057485, 268435456ll});
        REQUIRE(binomial(rational{3, 4}, 0) == 1);
        REQUIRE(binomial(rational{3, 4}, -1) == 0);
        REQUIRE(binomial(rational{3, 4}, -2) == 0);
        REQUIRE(binomial(rational{3, 4}, -10ll) == 0);
        REQUIRE(binomial(rational{3, -4}, 0) == 1);
        REQUIRE(binomial(rational{3, -4}, 1) == -rational{3, 4});
        REQUIRE(binomial(rational{3, -4}, 5) == -rational{4389, 8192});
        REQUIRE(binomial(rational{3, -4}, -1) == 0);
        REQUIRE(binomial(rational{3, -4}, -5) == 0);
    }
};

TEST_CASE("binomial")
{
    tuple_for_each(sizes{}, binomial_tester{});
}
