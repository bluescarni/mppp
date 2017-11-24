// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
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

struct ior_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        mpz_raii m1, m2, m3;
        integer n1, n2, n3;
        ::mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        bitwise_ior(n1, n2, n3);
        REQUIRE(n1 == integer{&m1.m_mpz});
        REQUIRE(n1 == (n2 | n3));
        mpz_raii tmp1, tmp2;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                random_integer(tmp1, x, rng);
                ::mpz_set(&m2.m_mpz, &tmp1.m_mpz);
                random_integer(tmp2, y, rng);
                ::mpz_set(&m3.m_mpz, &tmp2.m_mpz);
                n2 = integer(&tmp1.m_mpz);
                n3 = integer(&tmp2.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                if (sdist(rng)) {
                    ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                if (n3.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3.promote();
                }
                ::mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                bitwise_ior(n1, n2, n3);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (n2 | n3));
                // Overlapping arguments.
                integer old_n1{n1};
                ::mpz_ior(&m1.m_mpz, &m1.m_mpz, &m3.m_mpz);
                bitwise_ior(n1, n1, n3);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (old_n1 | n3));
                old_n1 |= n3;
                REQUIRE(n1 == old_n1);
                integer old_n2{n2};
                ::mpz_ior(&m2.m_mpz, &m1.m_mpz, &m2.m_mpz);
                bitwise_ior(n2, n1, n2);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (n1 | old_n2));
                old_n1 = n1;
                ::mpz_ior(&m1.m_mpz, &m1.m_mpz, &m1.m_mpz);
                bitwise_ior(n1, n1, n1);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (old_n1 | old_n1));
                old_n1 |= old_n1;
                REQUIRE(n1 == old_n1);
            }
        };

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

        // Size-specific testing.
        if (S::value == 1u) {
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            ::mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            ::mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            ::mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = 0;
            ::mpz_set_si(&m2.m_mpz, 0);
            n3.neg();
            ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_ior(n1, n2, n3);
            ::mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
        }

        if (S::value == 2u) {
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            ::mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            ::mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            ::mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = 0;
            ::mpz_set_si(&m2.m_mpz, 0);
            n3.neg();
            ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_ior(n1, n2, n3);
            ::mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // Fill the high limbs too.
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            n2 <<= GMP_NUMB_BITS;
            n3 <<= GMP_NUMB_BITS;
            n2 += GMP_NUMB_MAX;
            n3 += GMP_NUMB_MAX;
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            ::mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            ::mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            ::mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 >>= GMP_NUMB_BITS;
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            ::mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            ::mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
        }
    }
};

TEST_CASE("integer ior")
{
    tuple_for_each(sizes{}, ior_tester{});
}
