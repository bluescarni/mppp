// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
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

static inline bool check_cmp(int c1, int c2)
{
    if (c1 < 0) {
        return c2 < 0;
    }
    if (c1 == 0) {
        return c2 == 0;
    }
    return c2 > 0;
}

struct cmp_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        mpz_raii m1, m2;
        integer n1, n2;
        REQUIRE(check_cmp(cmp(n1, n2), ::mpz_cmp(&m1.m_mpz, &m2.m_mpz)));
        REQUIRE(n1.is_static());
        REQUIRE(n2.is_static());
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
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
                random_integer(tmp, y, rng);
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                REQUIRE(check_cmp(cmp(n1, n2), ::mpz_cmp(&m1.m_mpz, &m2.m_mpz)));
                REQUIRE(check_cmp(cmp(n1, n1), ::mpz_cmp(&m1.m_mpz, &m1.m_mpz)));
                REQUIRE(check_cmp(cmp(n2, n2), ::mpz_cmp(&m2.m_mpz, &m2.m_mpz)));
                REQUIRE((n1 == n1));
                REQUIRE((n2 == n2));
                if (::mpz_cmp(&m1.m_mpz, &m2.m_mpz)) {
                    REQUIRE((n1 != n2));
                } else {
                    REQUIRE((n1 == n2));
                }
                n2 = n1;
                ::mpz_set(&m2.m_mpz, &m1.m_mpz);
                if (sdist(rng) && n2.is_static()) {
                    n2.promote();
                }
                REQUIRE(check_cmp(cmp(n1, n2), ::mpz_cmp(&m1.m_mpz, &m2.m_mpz)));
                // Overlap.
                REQUIRE(check_cmp(cmp(n1, n1), ::mpz_cmp(&m1.m_mpz, &m1.m_mpz)));
            }
        };

        random_xy(0, 0);

        random_xy(1, 0);
        random_xy(0, 1);
        random_xy(1, 1);

        random_xy(0, 2);
        random_xy(1, 2);
        random_xy(2, 0);
        random_xy(2, 1);
        random_xy(2, 2);

        random_xy(0, 3);
        random_xy(1, 3);
        random_xy(2, 3);
        random_xy(3, 0);
        random_xy(3, 1);
        random_xy(3, 2);
        random_xy(3, 3);

        random_xy(0, 4);
        random_xy(1, 4);
        random_xy(2, 4);
        random_xy(3, 4);
        random_xy(4, 0);
        random_xy(4, 1);
        random_xy(4, 2);
        random_xy(4, 3);
        random_xy(4, 4);
    }
};

TEST_CASE("cmp")
{
    tuple_for_each(sizes{}, cmp_tester{});
}
