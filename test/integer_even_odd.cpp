// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <gmp.h>
#include <random>
#include <tuple>
#include <type_traits>

#include <mp++/integer.hpp>

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
