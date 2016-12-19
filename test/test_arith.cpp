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
#include <stdexcept>
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
                    n2.neg();
                }
                random_integer(tmp, y, rng);
                ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                n3 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                add(n1, n2, n3);
                ::mpz_add(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // Various variations if in-place.
                add(n1, n1, n2);
                ::mpz_add(&m1.m_mpz, &m1.m_mpz, &m2.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                add(n2, n1, n2);
                ::mpz_add(&m2.m_mpz, &m1.m_mpz, &m2.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                add(n1, n1, n1);
                ::mpz_add(&m1.m_mpz, &m1.m_mpz, &m1.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // Test overflow when second size is larger than the other.
                if (y > x) {
                    random_integer(tmp, x, rng);
                    ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                    n2 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                        n2.neg();
                    }
                    max_integer(tmp, y);
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                        n3.neg();
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
                    const bool neg = sdist(rng) == 1;
                    if (neg) {
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                        n2.neg();
                    }
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (!neg) {
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                        n3.neg();
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
                    const bool neg = sdist(rng) == 1;
                    if (neg) {
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                        n2.neg();
                    }
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (!neg) {
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                        n3.neg();
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
        // Start with zeroes.
        mpz_raii m1, m2, m3;
        integer n1, n2, n3;
        mul(n1, n2, n3);
        ::mpz_mul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        REQUIRE(n2.is_static());
        REQUIRE(n3.is_static());
        n1 = integer(12);
        ::mpz_set_ui(&m1.m_mpz, 12);
        mul(n1, n2, n3);
        ::mpz_mul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        REQUIRE(n2.is_static());
        REQUIRE(n3.is_static());
        mul(n1, n3, n2);
        ::mpz_mul(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
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
                    n2.neg();
                }
                random_integer(tmp, y, rng);
                ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                n3 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                mul(n1, n2, n3);
                ::mpz_mul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // In-place variations.
                random_integer(tmp, x, rng);
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                random_integer(tmp, y, rng);
                ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                n3 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                mul(n2, n2, n3);
                ::mpz_mul(&m2.m_mpz, &m2.m_mpz, &m3.m_mpz);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                random_integer(tmp, x, rng);
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                random_integer(tmp, y, rng);
                ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                n3 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                mul(n2, n3, n2);
                ::mpz_mul(&m2.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                random_integer(tmp, x, rng);
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                mul(n2, n2, n2);
                ::mpz_mul(&m2.m_mpz, &m2.m_mpz, &m2.m_mpz);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // Specific test for single-limb optimization.
                if (S::value == 1u && x == 1u && y == 1u) {
                    n1 = integer{};
                    random_integer(tmp, 1, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
                    ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                    n2 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        n2.neg();
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    }
                    random_integer(tmp, 1, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        n3.neg();
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    }
                    mul(n1, n2, n3);
                    ::mpz_mul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                }
                // Make sure we test 2 x 1 when it succeeds.
                if (S::value == 2u && x == 1u && y == 2u) {
                    n1 = integer{};
                    ::mpz_set_ui(&m2.m_mpz, 1);
                    n2 = integer(1);
                    if (sdist(rng)) {
                        n2.neg();
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    }
                    random_integer(tmp, y, rng);
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        n3.neg();
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    }
                    mul(n1, n2, n3);
                    ::mpz_mul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                }
                // When using mpn, test a case in which we can write directly to the output operand, after
                // verifying that the size fits.
                if (S::value == 3u && x == 1u && y == 3u) {
                    n1 = integer{};
                    ::mpz_set_ui(&m2.m_mpz, 1);
                    n2 = integer(1);
                    if (sdist(rng)) {
                        n2.neg();
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    }
                    random_integer(tmp, y, rng);
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        n3.neg();
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    }
                    mul(n1, n2, n3);
                    ::mpz_mul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
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
    }
};

TEST_CASE("mul")
{
    tuple_for_each(sizes{}, mul_tester{});
}

struct addmul_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        // Start with zeroes.
        mpz_raii m1, m2, m3;
        integer n1, n2, n3;
        addmul(n1, n2, n3);
        ::mpz_addmul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        REQUIRE(n2.is_static());
        REQUIRE(n3.is_static());
        n1 = integer(12);
        ::mpz_set_ui(&m1.m_mpz, 12);
        addmul(n1, n2, n3);
        ::mpz_addmul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        REQUIRE(n2.is_static());
        REQUIRE(n3.is_static());
        addmul(n1, n3, n2);
        ::mpz_addmul(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
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
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                random_integer(tmp, y, rng);
                ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                n3 = integer(mpz_to_str(&tmp.m_mpz));
                if (n3.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3.promote();
                }
                if (sdist(rng)) {
                    ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                addmul(n1, n2, n3);
                ::mpz_addmul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // In-place variations.
                random_integer(tmp, x, rng);
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                random_integer(tmp, y, rng);
                ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                n3 = integer(mpz_to_str(&tmp.m_mpz));
                if (n3.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3.promote();
                }
                if (sdist(rng)) {
                    ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                addmul(n2, n2, n3);
                ::mpz_addmul(&m2.m_mpz, &m2.m_mpz, &m3.m_mpz);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                random_integer(tmp, x, rng);
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                random_integer(tmp, y, rng);
                ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                n3 = integer(mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                if (n3.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3.promote();
                }
                addmul(n2, n3, n2);
                ::mpz_addmul(&m2.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                random_integer(tmp, x, rng);
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                addmul(n2, n2, n2);
                ::mpz_addmul(&m2.m_mpz, &m2.m_mpz, &m2.m_mpz);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // Specific test for single-limb optimization.
                if (S::value == 1u && x == 1u && y == 1u) {
                    // Check when product succeeds but add fails.
                    max_integer(tmp, 1);
                    ::mpz_set(&m1.m_mpz, &tmp.m_mpz);
                    n1 = integer(mpz_to_str(&tmp.m_mpz));
                    ::mpz_set_ui(&m2.m_mpz, 2);
                    n2 = integer(2);
                    ::mpz_set_ui(&m3.m_mpz, 2);
                    n3 = integer(2);
                    addmul(n1, n2, n3);
                    ::mpz_addmul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    // Prod cancels rop.
                    std::uniform_int_distribution<int> idist(1, 40);
                    int i2 = -idist(rng), i3 = idist(rng), i1 = -i2 * i3;
                    n1 = integer(i1);
                    n2 = integer(i2);
                    n3 = integer(i3);
                    addmul(n1, n2, n3);
                    ::mpz_set_si(&m1.m_mpz, i1);
                    ::mpz_set_si(&m2.m_mpz, i2);
                    ::mpz_set_si(&m3.m_mpz, i3);
                    ::mpz_addmul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    // Prod different sign from rop and larger in abs.
                    i2 = -idist(rng), i3 = idist(rng), i1 = -i2 * i3 - 1;
                    n1 = integer(i1);
                    n2 = integer(i2);
                    n3 = integer(i3);
                    addmul(n1, n2, n3);
                    ::mpz_set_si(&m1.m_mpz, i1);
                    ::mpz_set_si(&m2.m_mpz, i2);
                    ::mpz_set_si(&m3.m_mpz, i3);
                    ::mpz_addmul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                }
                // Make sure we test 2 x 1 when it succeeds.
                if (S::value == 2u && x == 1u && y == 2u) {
                    n1 = integer{1};
                    ::mpz_set_ui(&m1.m_mpz, 1);
                    ::mpz_set_ui(&m2.m_mpz, 1);
                    n2 = integer(1);
                    if (sdist(rng)) {
                        n2.neg();
                        ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    }
                    random_integer(tmp, y, rng);
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        n3.neg();
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    }
                    addmul(n1, n2, n3);
                    ::mpz_addmul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                }
                // SSize 2, diff signs, abs(rop) >= abs(prod), result size 1.
                if (S::value == 2) {
                    random_integer(tmp, 1, rng);
                    ::mpz_set(&m1.m_mpz, &tmp.m_mpz);
                    n1 = integer(mpz_to_str(&tmp.m_mpz));
                    ::mpz_set_si(&m2.m_mpz, -1);
                    n2 = integer(-1);
                    std::uniform_int_distribution<int> idist(1, 40);
                    auto i1 = idist(rng);
                    ::mpz_set_si(&m3.m_mpz, i1);
                    n3 = integer(i1);
                    addmul(n1, n2, n3);
                    ::mpz_addmul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                }
                // Overflow in the addition.
                if (S::value == 2) {
                    max_integer(tmp, 2);
                    ::mpz_set(&m1.m_mpz, &tmp.m_mpz);
                    n1 = integer(mpz_to_str(&tmp.m_mpz));
                    std::uniform_int_distribution<int> idist(1, 40);
                    auto i1 = idist(rng);
                    ::mpz_set_si(&m2.m_mpz, i1);
                    n2 = integer(i1);
                    i1 = idist(rng);
                    ::mpz_set_si(&m3.m_mpz, i1);
                    n3 = integer(i1);
                    addmul(n1, n2, n3);
                    ::mpz_addmul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                }
                // SSize 2, diff signs, abs(rop) >= abs(prod), result size 2.
                if (S::value == 2) {
                    random_integer(tmp, 2, rng);
                    ::mpz_set(&m1.m_mpz, &tmp.m_mpz);
                    n1 = integer(mpz_to_str(&tmp.m_mpz));
                    ::mpz_set_si(&m2.m_mpz, -1);
                    n2 = integer(-1);
                    std::uniform_int_distribution<int> idist(1, 40);
                    auto i1 = idist(rng);
                    ::mpz_set_si(&m3.m_mpz, i1);
                    n3 = integer(i1);
                    addmul(n1, n2, n3);
                    ::mpz_addmul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                }
                // SSize 2, diff signs, final result is zero.
                if (S::value == 2) {
                    std::uniform_int_distribution<int> idist(1, 40);
                    auto i1 = idist(rng), i2 = idist(rng);
                    ::mpz_set_si(&m1.m_mpz, i1 * i2);
                    n1 = integer(i1 * i2);
                    ::mpz_set_si(&m2.m_mpz, i1);
                    n2 = integer(i1);
                    ::mpz_set_si(&m3.m_mpz, -i2);
                    n3 = integer(-i2);
                    addmul(n1, n2, n3);
                    ::mpz_addmul(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
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
    }
};

TEST_CASE("addmul")
{
    tuple_for_each(sizes{}, addmul_tester{});
}

struct div_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        mpz_raii m1, m2, m3, m4;
        integer n1, n2, n3, n4;
        // A few simple tests to start.
        n3 = integer(12);
        n4 = integer(5);
        ::mpz_set_ui(&m3.m_mpz, 12);
        ::mpz_set_ui(&m4.m_mpz, 5);
        div(n1, n2, n3, n4);
        ::mpz_tdiv_qr(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz, &m4.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(n2) == lex_cast(m2)));
        n3 = integer(-12);
        ::mpz_set_si(&m3.m_mpz, -12);
        div(n1, n2, n3, n4);
        ::mpz_tdiv_qr(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz, &m4.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(n2) == lex_cast(m2)));
        n4 = integer(-5);
        ::mpz_set_si(&m4.m_mpz, -5);
        div(n1, n2, n3, n4);
        ::mpz_tdiv_qr(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz, &m4.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(n2) == lex_cast(m2)));
        n3 = integer(12);
        ::mpz_set_ui(&m3.m_mpz, 12);
        div(n1, n2, n3, n4);
        ::mpz_tdiv_qr(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz, &m4.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(n2) == lex_cast(m2)));
        // Random testing.
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                // Helper to generate randomly dividend and divisor.
                auto random_34 = [&]() {
                    random_integer(tmp, x, rng);
                    ::mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = integer(mpz_to_str(&tmp.m_mpz));
                    if (sdist(rng)) {
                        ::mpz_neg(&m3.m_mpz, &m3.m_mpz);
                        n3.neg();
                    }
                    if (n3.is_static() && sdist(rng)) {
                        // Promote sometimes, if possible.
                        n3.promote();
                    }
                    // Make sure divisor is not zero.
                    do {
                        random_integer(tmp, y, rng);
                        ::mpz_set(&m4.m_mpz, &tmp.m_mpz);
                        n4 = integer(mpz_to_str(&tmp.m_mpz));
                        if (sdist(rng)) {
                            ::mpz_neg(&m4.m_mpz, &m4.m_mpz);
                            n4.neg();
                        }
                        if (n4.is_static() && sdist(rng)) {
                            // Promote sometimes, if possible.
                            n4.promote();
                        }
                    } while (n4.sign() == 0);
                };
                random_34();
                // Reset rops every once in a while.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n2 = integer{};
                    ::mpz_set_ui(&m2.m_mpz, 0);
                }
                div(n1, n2, n3, n4);
                ::mpz_tdiv_qr(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // In-place variations.
                random_34();
                div(n1, n3, n3, n4);
                ::mpz_tdiv_qr(&m1.m_mpz, &m3.m_mpz, &m3.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n3) == lex_cast(m3)));
                random_34();
                div(n1, n4, n3, n4);
                ::mpz_tdiv_qr(&m1.m_mpz, &m4.m_mpz, &m3.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n4) == lex_cast(m4)));
                random_34();
                div(n1, n2, n4, n4);
                ::mpz_tdiv_qr(&m1.m_mpz, &m2.m_mpz, &m4.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                random_34();
                div(n1, n4, n4, n4);
                ::mpz_tdiv_qr(&m1.m_mpz, &m4.m_mpz, &m4.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n4) == lex_cast(m4)));
                random_34();
                div(n4, n2, n4, n4);
                ::mpz_tdiv_qr(&m4.m_mpz, &m2.m_mpz, &m4.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n4) == lex_cast(m4)));
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
            }
            // Error handling.
            n3 = integer(12);
            n4 = integer(0);
            REQUIRE_THROWS_PREDICATE(div(n1, n2, n3, n4), zero_division_error, [](const zero_division_error &ex) {
                return std::string(ex.what()) == "Integer division by zero";
            });
            REQUIRE_THROWS_PREDICATE(div(n1, n1, n3, n3), std::invalid_argument, [](const std::invalid_argument &ex) {
                return std::string(ex.what()) == "When performing a division with remainder, the quotient 'q' and the "
                                                 "remainder 'r' must be distinct objects";
            });
        };

        random_xy(0, 1);
        random_xy(1, 1);

        random_xy(0, 2);
        random_xy(1, 2);
        random_xy(2, 1);
        random_xy(2, 2);

        random_xy(0, 3);
        random_xy(1, 3);
        random_xy(2, 3);
        random_xy(3, 1);
        random_xy(3, 2);
        random_xy(3, 3);

        random_xy(0, 4);
        random_xy(1, 4);
        random_xy(2, 4);
        random_xy(3, 4);
        random_xy(4, 1);
        random_xy(4, 2);
        random_xy(4, 3);
        random_xy(4, 4);
    }
};

TEST_CASE("div")
{
    tuple_for_each(sizes{}, div_tester{});
}

struct lshift_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        // A few zero tests to start.
        mpz_raii m1, m2;
        integer n1, n2;
        mul_2exp(n1, n2, 0u);
        ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, 0u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        n2 = integer(4);
        ::mpz_set_ui(&m2.m_mpz, 4);
        mul_2exp(n1, n2, 0u);
        ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, 0u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        n2 = integer(-4);
        ::mpz_set_si(&m2.m_mpz, -4);
        mul_2exp(n1, n2, 0u);
        ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, 0u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        n2 = integer(0);
        ::mpz_set_ui(&m2.m_mpz, 0);
        mul_2exp(n1, n2, 4u);
        ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, 0u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        // Simple tests.
        n2 = integer(12);
        ::mpz_set_ui(&m2.m_mpz, 12);
        mul_2exp(n1, n2, 2u);
        ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, 2u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        n2 = integer(-12);
        ::mpz_set_si(&m2.m_mpz, -12);
        mul_2exp(n1, n2, 2u);
        ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, 2u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        // Random testing.
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        auto random_x = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                // Half limb shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bdh(0u, GMP_NUMB_BITS / 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                auto rbs = bdh(rng);
                mul_2exp(n1, n2, rbs);
                ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // Try in-place as well.
                mul_2exp(n2, n2, rbs);
                ::mpz_mul_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 1 limb shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd1(0u, GMP_NUMB_BITS);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd1(rng);
                mul_2exp(n1, n2, rbs);
                ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                mul_2exp(n2, n2, rbs);
                ::mpz_mul_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 1 and half limb shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd1h(0u, GMP_NUMB_BITS + GMP_NUMB_BITS / 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd1h(rng);
                mul_2exp(n1, n2, rbs);
                ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                mul_2exp(n2, n2, rbs);
                ::mpz_mul_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 2 limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd2(0u, GMP_NUMB_BITS * 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd2(rng);
                mul_2exp(n1, n2, rbs);
                ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                mul_2exp(n2, n2, rbs);
                ::mpz_mul_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 2 and half limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd2h(0u, GMP_NUMB_BITS * 2u + GMP_NUMB_BITS / 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd2h(rng);
                mul_2exp(n1, n2, rbs);
                ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                mul_2exp(n2, n2, rbs);
                ::mpz_mul_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 3 limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd3(0u, GMP_NUMB_BITS * 3u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd3(rng);
                mul_2exp(n1, n2, rbs);
                ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                mul_2exp(n2, n2, rbs);
                ::mpz_mul_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 3 and half limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd3h(0u, GMP_NUMB_BITS * 3u + GMP_NUMB_BITS / 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd3h(rng);
                mul_2exp(n1, n2, rbs);
                ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                mul_2exp(n2, n2, rbs);
                ::mpz_mul_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 4 limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd4(0u, GMP_NUMB_BITS * 4u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd4(rng);
                mul_2exp(n1, n2, rbs);
                ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                mul_2exp(n2, n2, rbs);
                ::mpz_mul_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 4 limbs and half shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd4h(0u, GMP_NUMB_BITS * 4u + GMP_NUMB_BITS / 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd4h(rng);
                mul_2exp(n1, n2, rbs);
                ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                mul_2exp(n2, n2, rbs);
                ::mpz_mul_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 5 limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd5(0u, GMP_NUMB_BITS * 5u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd5(rng);
                mul_2exp(n1, n2, rbs);
                ::mpz_mul_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                mul_2exp(n2, n2, rbs);
                ::mpz_mul_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
            }
        };

        random_x(0);
        random_x(1);
        random_x(2);
        random_x(3);
        random_x(4);
    }
};

TEST_CASE("lshift")
{
    tuple_for_each(sizes{}, lshift_tester{});
}

struct rshift_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        // A few zero tests to start.
        mpz_raii m1, m2;
        integer n1, n2;
        tdiv_q_2exp(n1, n2, 0u);
        ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, 0u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        n2 = integer(4);
        ::mpz_set_ui(&m2.m_mpz, 4);
        tdiv_q_2exp(n1, n2, 0u);
        ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, 0u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        n2 = integer(-4);
        ::mpz_set_si(&m2.m_mpz, -4);
        tdiv_q_2exp(n1, n2, 0u);
        ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, 0u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        n2 = integer(0);
        ::mpz_set_ui(&m2.m_mpz, 0);
        tdiv_q_2exp(n1, n2, 4u);
        ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, 0u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        // Simple tests.
        n2 = integer(12);
        ::mpz_set_ui(&m2.m_mpz, 12);
        tdiv_q_2exp(n1, n2, 2u);
        ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, 2u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        n2 = integer(-12);
        ::mpz_set_si(&m2.m_mpz, -12);
        tdiv_q_2exp(n1, n2, 2u);
        ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, 2u);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        // Random testing.
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        auto random_x = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                // Half limb shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bdh(0u, GMP_NUMB_BITS / 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                auto rbs = bdh(rng);
                tdiv_q_2exp(n1, n2, rbs);
                ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // Try in-place as well.
                tdiv_q_2exp(n2, n2, rbs);
                ::mpz_tdiv_q_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 1 limb shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd1(0u, GMP_NUMB_BITS);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd1(rng);
                tdiv_q_2exp(n1, n2, rbs);
                ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                tdiv_q_2exp(n2, n2, rbs);
                ::mpz_tdiv_q_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 1 and half limb shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd1h(0u, GMP_NUMB_BITS + GMP_NUMB_BITS / 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd1h(rng);
                tdiv_q_2exp(n1, n2, rbs);
                ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                tdiv_q_2exp(n2, n2, rbs);
                ::mpz_tdiv_q_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 2 limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd2(0u, GMP_NUMB_BITS * 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd2(rng);
                tdiv_q_2exp(n1, n2, rbs);
                ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                tdiv_q_2exp(n2, n2, rbs);
                ::mpz_tdiv_q_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 2 and half limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd2h(0u, GMP_NUMB_BITS * 2u + GMP_NUMB_BITS / 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd2h(rng);
                tdiv_q_2exp(n1, n2, rbs);
                ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                tdiv_q_2exp(n2, n2, rbs);
                ::mpz_tdiv_q_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 3 limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd3(0u, GMP_NUMB_BITS * 3u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd3(rng);
                tdiv_q_2exp(n1, n2, rbs);
                ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                tdiv_q_2exp(n2, n2, rbs);
                ::mpz_tdiv_q_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 3 and half limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd3h(0u, GMP_NUMB_BITS * 3u + GMP_NUMB_BITS / 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd3h(rng);
                tdiv_q_2exp(n1, n2, rbs);
                ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                tdiv_q_2exp(n2, n2, rbs);
                ::mpz_tdiv_q_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 4 limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd4(0u, GMP_NUMB_BITS * 4u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd4(rng);
                tdiv_q_2exp(n1, n2, rbs);
                ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                tdiv_q_2exp(n2, n2, rbs);
                ::mpz_tdiv_q_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 4 limbs and half shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd4h(0u, GMP_NUMB_BITS * 4u + GMP_NUMB_BITS / 2u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd4h(rng);
                tdiv_q_2exp(n1, n2, rbs);
                ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                tdiv_q_2exp(n2, n2, rbs);
                ::mpz_tdiv_q_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                // 5 limbs shift.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    ::mpz_set_ui(&m1.m_mpz, 0);
                }
                random_integer(tmp, x, rng);
                std::uniform_int_distribution<unsigned> bd5(0u, GMP_NUMB_BITS * 5u);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                rbs = bd5(rng);
                tdiv_q_2exp(n1, n2, rbs);
                ::mpz_tdiv_q_2exp(&m1.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                tdiv_q_2exp(n2, n2, rbs);
                ::mpz_tdiv_q_2exp(&m2.m_mpz, &m2.m_mpz, rbs);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
            }
        };

        random_x(0);
        random_x(1);
        random_x(2);
        random_x(3);
        random_x(4);
    }
};

TEST_CASE("rshift")
{
    tuple_for_each(sizes{}, rshift_tester{});
}
