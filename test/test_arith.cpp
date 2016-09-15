/* Copyright 2009-2016 Francesco Biscani (bluescarni@gmail.com)

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

// TODO inplace tests, particularily for add/mul/etc. when promotion of rop might mean
// promotion of operands.

struct add_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        // Start with all zeroes.
        mpz_raii m1, m2, m3;
        integer n1, n2, n3;
        add(n1, n2, n3);
        ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        REQUIRE(n2.is_static());
        REQUIRE(n3.is_static());
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.negate();
                }
                random_integer(tmp, y, rng);
                ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                n3 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.negate();
                }
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                add(n1, n2, n3);
                ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // Test overflow when second size is larger than the other.
                if (y > x) {
                    random_integer(tmp, x, rng);
                    ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                    n2 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                        n2.negate();
                    }
                    max_integer(tmp, y);
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                        n3.negate();
                    }
                    add(n1, n2, n3);
                    ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                }
                // Test subtraction of equal numbers.
                if (x == y) {
                    random_integer(tmp, x, rng);
                    ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                    n2 = integer(mpz_to_str(&tmp.m_mpz));
                    const bool neg = sdist(rng);
                    if (neg) {
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                        n2.negate();
                    }
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (!neg) {
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                        n3.negate();
                    }
                    add(n1, n2, n3);
                    ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    REQUIRE((lex_cast(n1) == "0"));
                }
                // Test subtraction with equal top limbs.
                if (x == y) {
                    random_integer(tmp, x, rng);
                    ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                    n2 = integer(mpz_to_str(&tmp.m_mpz));
                    const bool neg = sdist(rng);
                    if (neg) {
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                        n2.negate();
                    }
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (!neg) {
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                        n3.negate();
                    }
                    // Add 1 to bump up the lower limb.
                    integer one(1);
                    add(n2, n2, one);
                    ::mpz_add_ui(&m2.m_mpz, &m2.m_mpz, 1u);
                    add(n1, n2, n3);
                    ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    add(n1, n3, n2);
                    ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                }
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

        // Testing specific to the 2-limb optimisation.
        if (S::value == 2u) {
            // Carry only from lo.
            max_integer(m2, 1u);
            ::mpz_set_ui(&m3.m_mpz, 1u);
            n2 = integer(::mp_limb_t(-1) & GMP_NUMB_MAX);
            n3 = integer(1);
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            // Carry only from hi.
            max_integer(m2, 2u);
            ::mpz_set_ui(&m3.m_mpz, 1u);
            ::mpz_mul_2exp(&m3.m_mpz, &m3.m_mpz, GMP_NUMB_BITS);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            // Carry from hi and lo.
            max_integer(m2, 2u);
            ::mpz_set_ui(&m3.m_mpz, 1u);
            ::mpz_mul_2exp(&m3.m_mpz, &m3.m_mpz, GMP_NUMB_BITS);
            ::mpz_add_ui(&m3.m_mpz, &m3.m_mpz, 1u);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            // Subtraction that kills hi.
            max_integer(m2, 2u);
            max_integer(m3, 1u);
            ::mpz_mul_2exp(&m3.m_mpz, &m3.m_mpz, GMP_NUMB_BITS);
            ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 1u));
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 1u));
            ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
            ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 1u));
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 1u));
            // Subtraction that kills lo.
            max_integer(m2, 2u);
            max_integer(m3, 1u);
            ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 2u));
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 2u));
            ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
            ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 2u));
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 2u));
        }
    }
};

TEST_CASE("add")
{
    tuple_for_each(sizes{}, add_tester{});
}

struct mul_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        integer n1;
        mul(n1, integer{1}, integer{2});
        std::cout << n1 << '\n';
        mul(n1, integer{-1}, integer{2});
        std::cout << n1 << '\n';
        mul(n1, integer{2}, integer{-1});
        std::cout << n1 << '\n';
        mul(n1, integer{2}, integer{::mp_limb_t(-1)});
        std::cout << n1 << '\n';
#if 0
        // Start with all zeroes.
        mpz_raii m1, m2, m3;
        integer n1, n2, n3;
        add(n1, n2, n3);
        ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        REQUIRE(n2.is_static());
        REQUIRE(n3.is_static());
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.negate();
                }
                random_integer(tmp, y, rng);
                ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                n3 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.negate();
                }
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                add(n1, n2, n3);
                ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // Test overflow when second size is larger than the other.
                if (y > x) {
                    random_integer(tmp, x, rng);
                    ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                    n2 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                        n2.negate();
                    }
                    max_integer(tmp, y);
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                        n3.negate();
                    }
                    add(n1, n2, n3);
                    ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                }
                // Test subtraction of equal numbers.
                if (x == y) {
                    random_integer(tmp, x, rng);
                    ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                    n2 = integer(mpz_to_str(&tmp.m_mpz));
                    const bool neg = sdist(rng);
                    if (neg) {
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                        n2.negate();
                    }
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (!neg) {
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                        n3.negate();
                    }
                    add(n1, n2, n3);
                    ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    REQUIRE((lex_cast(n1) == "0"));
                }
                // Test subtraction with equal top limbs.
                if (x == y) {
                    random_integer(tmp, x, rng);
                    ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                    n2 = integer(mpz_to_str(&tmp.m_mpz));
                    const bool neg = sdist(rng);
                    if (neg) {
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                        n2.negate();
                    }
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (!neg) {
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                        n3.negate();
                    }
                    // Add 1 to bump up the lower limb.
                    integer one(1);
                    add(n2, n2, one);
                    ::mpz_add_ui(&m2.m_mpz, &m2.m_mpz, 1u);
                    add(n1, n2, n3);
                    ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    add(n1, n3, n2);
                    ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                }
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

        // Testing specific to the 2-limb optimisation.
        if (S::value == 2u) {
            // Carry only from lo.
            max_integer(m2, 1u);
            ::mpz_set_ui(&m3.m_mpz, 1u);
            n2 = integer(::mp_limb_t(-1) & GMP_NUMB_MAX);
            n3 = integer(1);
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            // Carry only from hi.
            max_integer(m2, 2u);
            ::mpz_set_ui(&m3.m_mpz, 1u);
            ::mpz_mul_2exp(&m3.m_mpz, &m3.m_mpz, GMP_NUMB_BITS);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            // Carry from hi and lo.
            max_integer(m2, 2u);
            ::mpz_set_ui(&m3.m_mpz, 1u);
            ::mpz_mul_2exp(&m3.m_mpz, &m3.m_mpz, GMP_NUMB_BITS);
            ::mpz_add_ui(&m3.m_mpz, &m3.m_mpz, 1u);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            // Subtraction that kills hi.
            max_integer(m2, 2u);
            max_integer(m3, 1u);
            ::mpz_mul_2exp(&m3.m_mpz, &m3.m_mpz, GMP_NUMB_BITS);
            ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 1u));
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 1u));
            ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
            ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 1u));
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 1u));
            // Subtraction that kills lo.
            max_integer(m2, 2u);
            max_integer(m3, 1u);
            ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 2u));
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 2u));
            ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
            ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
            n2 = integer(lex_cast(m2));
            n3 = integer(lex_cast(m3));
            ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            add(n1, n2, n3);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 2u));
            ::mpz_add(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            add(n1, n3, n2);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((::mpz_size(&m1.m_mpz) == 2u));
        }
#endif
    }
};

TEST_CASE("mul")
{
    tuple_for_each(sizes{}, mul_tester{});
}
