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
#include <gmp.h>
#include <random>
#include <tuple>
#include <type_traits>

#include <mp++.hpp>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

static int ntries = 1000;

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

static std::mt19937 rng;

struct even_odd_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        mpz_raii m1;
        integer n1;
        REQUIRE(even_p(n1));
        REQUIRE(n1.even_p());
        REQUIRE(!odd_p(n1));
        REQUIRE(!n1.odd_p());
        // Simple tests.
        n1 = integer{1};
        REQUIRE(!even_p(n1));
        REQUIRE(!n1.even_p());
        REQUIRE(odd_p(n1));
        REQUIRE(n1.odd_p());
        n1 = integer{-1};
        REQUIRE(!even_p(n1));
        REQUIRE(!n1.even_p());
        REQUIRE(odd_p(n1));
        REQUIRE(n1.odd_p());
        n1 = integer{3};
        REQUIRE(!even_p(n1));
        REQUIRE(!n1.even_p());
        REQUIRE(odd_p(n1));
        REQUIRE(n1.odd_p());
        n1 = integer{-3};
        REQUIRE(!even_p(n1));
        REQUIRE(!n1.even_p());
        REQUIRE(odd_p(n1));
        REQUIRE(n1.odd_p());
        n1 = integer{4};
        REQUIRE(even_p(n1));
        REQUIRE(n1.even_p());
        REQUIRE(!odd_p(n1));
        REQUIRE(!n1.odd_p());
        n1 = integer{-4};
        REQUIRE(even_p(n1));
        REQUIRE(n1.even_p());
        REQUIRE(!odd_p(n1));
        REQUIRE(!n1.odd_p());
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Random tests with x number of limbs.
        auto random_xy = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                ::mpz_set(&m1.m_mpz, &tmp.m_mpz);
                n1 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m1.m_mpz, &m1.m_mpz);
                    n1.neg();
                }
                if (n1.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n1.promote();
                }
                REQUIRE((even_p(n1) == mpz_even_p(&m1.m_mpz)));
                REQUIRE((n1.even_p() == mpz_even_p(&m1.m_mpz)));
                REQUIRE((odd_p(n1) == mpz_odd_p(&m1.m_mpz)));
                REQUIRE((n1.odd_p() == mpz_odd_p(&m1.m_mpz)));
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);
    }
};

TEST_CASE("even_odd")
{
    tuple_for_each(sizes{}, even_odd_tester{});
}
