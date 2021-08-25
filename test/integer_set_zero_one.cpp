// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <random>
#include <tuple>
#include <type_traits>

#include <gmp.h>

#include <mp++/integer.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

static const int ntries = 1000;

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

// NOLINTNEXTLINE(cert-err58-cpp, cert-msc32-c, cert-msc51-cpp, cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937 rng;

struct set_zero_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with zero.
        integer n1;
        detail::mpz_raii m1;
        REQUIRE((std::is_same<integer &, decltype(n1.set_zero())>::value));
        n1.set_zero().set_zero();
        REQUIRE(n1.is_zero());
        REQUIRE(n1.is_static());
        n1 = 123;
        n1.set_zero().set_zero();
        REQUIRE(n1.is_zero());
        REQUIRE(n1.is_static());
        n1 = -123;
        n1.set_zero().set_zero();
        REQUIRE(n1.is_zero());
        REQUIRE(n1.is_static());
        n1 = -123;
        n1.promote();
        n1.set_zero().set_zero();
        REQUIRE(n1.is_zero());
        REQUIRE(n1.is_static());
        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x number of limbs.
        auto random_xy = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                mpz_set(&m1.m_mpz, &tmp.m_mpz);
                n1 = integer(&tmp.m_mpz);
                if (sdist(rng)) {
                    mpz_neg(&m1.m_mpz, &m1.m_mpz);
                    n1.neg();
                }
                if (n1.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n1.promote();
                }
                REQUIRE(&set_zero(n1) == &n1);
                REQUIRE(n1.is_zero());
                REQUIRE(n1.is_static());
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);
    }
};

TEST_CASE("set_zero")
{
    tuple_for_each(sizes{}, set_zero_tester{});
}

struct set_one_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with zero.
        integer n1;
        detail::mpz_raii m1;
        REQUIRE((std::is_same<integer &, decltype(n1.set_one())>::value));
        REQUIRE((std::is_same<integer &, decltype(n1.set_negative_one())>::value));
        n1.set_one().set_one();
        REQUIRE(n1.is_one());
        REQUIRE(n1.is_static());
        n1 = 123;
        n1.set_one().set_one();
        REQUIRE(n1.is_one());
        REQUIRE(n1.is_static());
        n1 = -123;
        n1.set_one().set_one();
        REQUIRE(n1.is_one());
        REQUIRE(n1.is_static());
        n1 = -123;
        n1.set_negative_one().set_negative_one();
        REQUIRE(n1.is_negative_one());
        REQUIRE(n1.is_static());
        n1 = -123;
        n1.promote();
        n1.set_one().set_one();
        REQUIRE(n1.is_one());
        REQUIRE(n1.is_static());
        n1 = -123;
        n1.promote();
        n1.set_negative_one().set_negative_one();
        REQUIRE(n1.is_negative_one());
        REQUIRE(n1.is_static());
        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x number of limbs.
        auto random_xy = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                mpz_set(&m1.m_mpz, &tmp.m_mpz);
                n1 = integer(&tmp.m_mpz);
                if (sdist(rng)) {
                    mpz_neg(&m1.m_mpz, &m1.m_mpz);
                    n1.neg();
                }
                if (n1.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n1.promote();
                }
                if (sdist(rng)) {
                    REQUIRE(&set_one(n1) == &n1);
                    REQUIRE(n1.is_one());
                    REQUIRE(n1.is_static());
                } else {
                    REQUIRE(&set_negative_one(n1) == &n1);
                    REQUIRE(n1.is_negative_one());
                    REQUIRE(n1.is_static());
                }
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);
    }
};

TEST_CASE("set_one")
{
    tuple_for_each(sizes{}, set_one_tester{});
}
