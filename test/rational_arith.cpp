// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <random>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include <gmp.h>

#include <mp++/rational.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

static const int ntries = 300;

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

// NOLINTNEXTLINE(cert-err58-cpp, cert-msc32-c, cert-msc51-cpp, cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937 rng;

struct add_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        // Start with all zeroes.
        detail::mpq_raii m1, m2, m3;
        rational n1, n2, n3;
        REQUIRE(&add(n1, n2, n3) == &n1);
        mpq_add(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(n2 + n3) == lex_cast(m1)));
        REQUIRE(n1.get_num().is_static());
        REQUIRE(n1.get_den().is_static());
        REQUIRE(n2.get_num().is_static());
        REQUIRE(n2.get_den().is_static());
        REQUIRE(n3.get_num().is_static());
        REQUIRE(n3.get_den().is_static());
        detail::mpq_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                random_rational(tmp, x, rng);
                mpq_set(&m2.m_mpq, &tmp.m_mpq);
                n2 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    mpq_neg(&m2.m_mpq, &m2.m_mpq);
                    n2.neg();
                }
                if (n2.get_num().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_num().promote();
                }
                if (n2.get_den().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_den().promote();
                }
                random_rational(tmp, y, rng);
                mpq_set(&m3.m_mpq, &tmp.m_mpq);
                n3 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    mpq_neg(&m3.m_mpq, &m3.m_mpq);
                    n3.neg();
                }
                if (n3.get_num().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3._get_num().promote();
                }
                if (n3.get_den().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3._get_den().promote();
                }
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = rational{};
                }
                add(n1, n2, n3);
                mpq_add(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2 + n3) == lex_cast(m1)));
                // Various variations of in-place.
                auto n1_old(n1);
                auto n2_old(n2);
                add(n1, n1, n2);
                mpq_add(&m1.m_mpq, &m1.m_mpq, &m2.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n1_old + n2) == lex_cast(m1)));
                add(n2, n1, n2);
                mpq_add(&m2.m_mpq, &m1.m_mpq, &m2.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                REQUIRE((lex_cast(n1 + n2_old) == lex_cast(m2)));
                n1_old = n1;
                add(n1, n1, n1);
                mpq_add(&m1.m_mpq, &m1.m_mpq, &m1.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n1_old + n1_old) == lex_cast(m1)));
                // Tests with integral arguments.
                auto n2_copy(n2);
                auto n3_copy(n3);
                detail::mpq_raii m2_copy, m3_copy;
                mpq_set(&m2_copy.m_mpq, &m2.m_mpq);
                mpq_set(&m3_copy.m_mpq, &m3.m_mpq);
                n2_copy._get_den() = 1;
                mpz_set_si(mpq_denref(&m2_copy.m_mpq), 1);
                add(n1, n2_copy, n3_copy);
                mpq_add(&m1.m_mpq, &m2_copy.m_mpq, &m3_copy.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2_copy + n3_copy) == lex_cast(m1)));
                REQUIRE((lex_cast(n2_copy.get_num() + n3_copy) == lex_cast(m1)));
                add(n1, n3_copy, n2_copy);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n3_copy + n2_copy) == lex_cast(m1)));
                REQUIRE((lex_cast(n3_copy + n2_copy.get_num()) == lex_cast(m1)));
                n3_copy._get_den() = 1;
                mpz_set_si(mpq_denref(&m3_copy.m_mpq), 1);
                add(n1, n2_copy, n3_copy);
                mpq_add(&m1.m_mpq, &m2_copy.m_mpq, &m3_copy.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n3_copy + n2_copy) == lex_cast(m1)));
                // Tests with equal dens. This checks that
                // the den of the retval is handled correctly.
                n1 = "3/2";
                n2_copy = n2;
                n3_copy = n3;
                n2_copy._get_num() = n3_copy.get_den() + 1;
                n2_copy._get_den() = n3_copy.get_den();
                add(n1, n2_copy, n3_copy);
                REQUIRE((lex_cast(n1) == lex_cast(rational{n2_copy.get_num() + n3_copy.get_num(), n3_copy.get_den()})));
                REQUIRE((lex_cast(n2_copy + n3_copy)
                         == lex_cast(rational{n2_copy.get_num() + n3_copy.get_num(), n3_copy.get_den()})));
                // Test subtraction of equal numbers.
                if (x == y) {
                    random_rational(tmp, x, rng);
                    mpq_set(&m2.m_mpq, &tmp.m_mpq);
                    n2 = rational(&tmp.m_mpq);
                    const bool neg = sdist(rng) == 1;
                    if (neg) {
                        mpq_neg(&m2.m_mpq, &m2.m_mpq);
                        n2.neg();
                    }
                    mpq_set(&m3.m_mpq, &tmp.m_mpq);
                    n3 = rational(&tmp.m_mpq);
                    if (!neg) {
                        mpq_neg(&m3.m_mpq, &m3.m_mpq);
                        n3.neg();
                    }
                    add(n1, n2, n3);
                    mpq_add(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    REQUIRE((lex_cast(n2 + n3) == lex_cast(m1)));
                    REQUIRE((lex_cast(n1) == "0"));
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

TEST_CASE("add")
{
    tuple_for_each(sizes{}, add_tester{});
}

struct sub_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        // Start with all zeroes.
        detail::mpq_raii m1, m2, m3;
        rational n1, n2, n3;
        REQUIRE(&sub(n1, n2, n3) == &n1);
        mpq_sub(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(n2 - n3) == lex_cast(m1)));
        REQUIRE(n1.get_num().is_static());
        REQUIRE(n1.get_den().is_static());
        REQUIRE(n2.get_num().is_static());
        REQUIRE(n2.get_den().is_static());
        REQUIRE(n3.get_num().is_static());
        REQUIRE(n3.get_den().is_static());
        detail::mpq_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                random_rational(tmp, x, rng);
                mpq_set(&m2.m_mpq, &tmp.m_mpq);
                n2 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    mpq_neg(&m2.m_mpq, &m2.m_mpq);
                    n2.neg();
                }
                if (n2.get_num().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_num().promote();
                }
                if (n2.get_den().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_den().promote();
                }
                random_rational(tmp, y, rng);
                mpq_set(&m3.m_mpq, &tmp.m_mpq);
                n3 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    mpq_neg(&m3.m_mpq, &m3.m_mpq);
                    n3.neg();
                }
                if (n3.get_num().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3._get_num().promote();
                }
                if (n3.get_den().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3._get_den().promote();
                }
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = rational{};
                }
                sub(n1, n2, n3);
                mpq_sub(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2 - n3) == lex_cast(m1)));
                // Various variations of in-place.
                auto old_n1(n1);
                auto old_n2(n2);
                sub(n1, n1, n2);
                mpq_sub(&m1.m_mpq, &m1.m_mpq, &m2.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(old_n1 - n2) == lex_cast(m1)));
                sub(n2, n1, n2);
                mpq_sub(&m2.m_mpq, &m1.m_mpq, &m2.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                REQUIRE((lex_cast(n1 - old_n2) == lex_cast(m2)));
                old_n1 = n1;
                sub(n1, n1, n1);
                mpq_sub(&m1.m_mpq, &m1.m_mpq, &m1.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(old_n1 - old_n1) == lex_cast(m1)));
                // Tests with integral arguments.
                auto n2_copy(n2);
                auto n3_copy(n3);
                detail::mpq_raii m2_copy, m3_copy;
                mpq_set(&m2_copy.m_mpq, &m2.m_mpq);
                mpq_set(&m3_copy.m_mpq, &m3.m_mpq);
                n2_copy._get_den() = 1;
                mpz_set_si(mpq_denref(&m2_copy.m_mpq), 1);
                sub(n1, n2_copy, n3_copy);
                mpq_sub(&m1.m_mpq, &m2_copy.m_mpq, &m3_copy.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2_copy - n3_copy) == lex_cast(m1)));
                REQUIRE((lex_cast(n2_copy.get_num() - n3_copy) == lex_cast(m1)));
                sub(n1, n3_copy, n2_copy);
                n1.neg();
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(-(n3_copy - n2_copy)) == lex_cast(m1)));
                REQUIRE((lex_cast(-(n3_copy - n2_copy.get_num())) == lex_cast(m1)));
                n3_copy._get_den() = 1;
                mpz_set_si(mpq_denref(&m3_copy.m_mpq), 1);
                sub(n1, n2_copy, n3_copy);
                mpq_sub(&m1.m_mpq, &m2_copy.m_mpq, &m3_copy.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2_copy - n3_copy) == lex_cast(m1)));
                // Tests with equal dens. This checks that
                // the den of the retval is handled correctly.
                n1 = "3/2";
                n2_copy = n2;
                n3_copy = n3;
                n2_copy._get_num() = n3_copy.get_den() + 1;
                n2_copy._get_den() = n3_copy.get_den();
                sub(n1, n2_copy, n3_copy);
                REQUIRE((lex_cast(n1) == lex_cast(rational{n2_copy.get_num() - n3_copy.get_num(), n3_copy.get_den()})));
                REQUIRE((lex_cast(n2_copy - n3_copy)
                         == lex_cast(rational{n2_copy.get_num() - n3_copy.get_num(), n3_copy.get_den()})));
                // Test subtraction of equal numbers.
                if (x == y) {
                    random_rational(tmp, x, rng);
                    mpq_set(&m2.m_mpq, &tmp.m_mpq);
                    mpq_neg(&m2.m_mpq, &tmp.m_mpq);
                    n2 = rational(&tmp.m_mpq);
                    n2.neg();
                    const bool neg = sdist(rng) == 1;
                    if (neg) {
                        mpq_neg(&m2.m_mpq, &m2.m_mpq);
                        n2.neg();
                    }
                    mpq_set(&m3.m_mpq, &tmp.m_mpq);
                    n3 = rational(&tmp.m_mpq);
                    if (!neg) {
                        mpq_neg(&m3.m_mpq, &m3.m_mpq);
                        n3.neg();
                    }
                    sub(n1, n2, n3);
                    mpq_sub(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
                    REQUIRE((lex_cast(n2 - n3) == lex_cast(m1)));
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    REQUIRE((lex_cast(n1) == "0"));
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

TEST_CASE("sub")
{
    tuple_for_each(sizes{}, sub_tester{});
}

struct mul_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        // Start with all zeroes.
        detail::mpq_raii m1, m2, m3;
        rational n1, n2, n3;
        REQUIRE(&mul(n1, n2, n3) == &n1);
        mpq_mul(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(n2 * n3) == lex_cast(m1)));
        REQUIRE(n1.get_num().is_static());
        REQUIRE(n1.get_den().is_static());
        REQUIRE(n2.get_num().is_static());
        REQUIRE(n2.get_den().is_static());
        REQUIRE(n3.get_num().is_static());
        REQUIRE(n3.get_den().is_static());
        detail::mpq_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                random_rational(tmp, x, rng);
                mpq_set(&m2.m_mpq, &tmp.m_mpq);
                n2 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    mpq_neg(&m2.m_mpq, &m2.m_mpq);
                    n2.neg();
                }
                if (n2.get_num().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_num().promote();
                }
                if (n2.get_den().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_den().promote();
                }
                random_rational(tmp, y, rng);
                mpq_set(&m3.m_mpq, &tmp.m_mpq);
                n3 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    mpq_neg(&m3.m_mpq, &m3.m_mpq);
                    n3.neg();
                }
                if (n3.get_num().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3._get_num().promote();
                }
                if (n3.get_den().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3._get_den().promote();
                }
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = rational{};
                }
                mul(n1, n2, n3);
                mpq_mul(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2 * n3) == lex_cast(m1)));
                // Various variations of in-place.
                auto n1_old(n1);
                auto n2_old(n2);
                mul(n1, n1, n2);
                mpq_mul(&m1.m_mpq, &m1.m_mpq, &m2.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n1_old * n2) == lex_cast(m1)));
                mul(n2, n1, n2);
                mpq_mul(&m2.m_mpq, &m1.m_mpq, &m2.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                REQUIRE((lex_cast(n2_old * n1) == lex_cast(m2)));
                n1_old = n1;
                mul(n1, n1, n1);
                mpq_mul(&m1.m_mpq, &m1.m_mpq, &m1.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n1_old * n1_old) == lex_cast(m1)));
                // Tests with integral arguments.
                auto n2_copy(n2);
                auto n3_copy(n3);
                detail::mpq_raii m2_copy, m3_copy;
                mpq_set(&m2_copy.m_mpq, &m2.m_mpq);
                mpq_set(&m3_copy.m_mpq, &m3.m_mpq);
                n2_copy._get_den() = 1;
                mpz_set_si(mpq_denref(&m2_copy.m_mpq), 1);
                mul(n1, n2_copy, n3_copy);
                mpq_mul(&m1.m_mpq, &m2_copy.m_mpq, &m3_copy.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2_copy * n3_copy) == lex_cast(m1)));
                REQUIRE((lex_cast(n2_copy.get_num() * n3_copy) == lex_cast(m1)));
                mul(n1, n3_copy, n2_copy);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n3_copy * n2_copy) == lex_cast(m1)));
                REQUIRE((lex_cast(n3_copy * n2.get_num()) == lex_cast(m1)));
                n3_copy._get_den() = 1;
                mpz_set_si(mpq_denref(&m3_copy.m_mpq), 1);
                mul(n1, n2_copy, n3_copy);
                mpq_mul(&m1.m_mpq, &m2_copy.m_mpq, &m3_copy.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2_copy * n3_copy) == lex_cast(m1)));
                // Tests with equal dens. This checks that
                // the den of the retval is handled correctly.
                n1 = "3/2";
                n2_copy = n2;
                n3_copy = n3;
                n2_copy._get_num() = n3_copy.get_den() + 1;
                n2_copy._get_den() = n3_copy.get_den();
                mul(n1, n2_copy, n3_copy);
                REQUIRE((lex_cast(n1)
                         == lex_cast(rational{n3_copy.get_num() * (n3_copy.get_den() + 1),
                                              n3_copy.get_den() * n3_copy.get_den()})));
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

struct div_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        detail::mpq_raii m1, m2, m3;
        rational n1, n2, n3;
        // Some zero testing.
        REQUIRE_THROWS_PREDICATE(div(n1, n2, n3), zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Zero divisor in rational division";
        });
        REQUIRE_THROWS_PREDICATE(n2 / n3, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Zero divisor in rational division";
        });
        n2 = 10;
        REQUIRE_THROWS_PREDICATE(div(n1, n2, n3), zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Zero divisor in rational division";
        });
        REQUIRE_THROWS_PREDICATE(n2 / n3, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Zero divisor in rational division";
        });
        n2 = 0;
        n3 = -10;
        mpq_set_si(&m3.m_mpq, -10, 1);
        REQUIRE(&div(n1, n2, n3) == &n1);
        mpq_div(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(n2 / n3) == lex_cast(m1)));
        REQUIRE(n1.get_num().is_static());
        REQUIRE(n1.get_den().is_static());
        REQUIRE(n2.get_num().is_static());
        REQUIRE(n2.get_den().is_static());
        REQUIRE(n3.get_num().is_static());
        REQUIRE(n3.get_den().is_static());
        detail::mpq_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                random_rational(tmp, x, rng);
                mpq_set(&m2.m_mpq, &tmp.m_mpq);
                n2 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    mpq_neg(&m2.m_mpq, &m2.m_mpq);
                    n2.neg();
                }
                if (n2.get_num().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_num().promote();
                }
                if (n2.get_den().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_den().promote();
                }
                random_rational(tmp, y, rng);
                mpq_set(&m3.m_mpq, &tmp.m_mpq);
                n3 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    mpq_neg(&m3.m_mpq, &m3.m_mpq);
                    n3.neg();
                }
                if (n3.get_num().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3._get_num().promote();
                }
                if (n3.get_den().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3._get_den().promote();
                }
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = rational{};
                }
                // Handle zero divisor.
                if (mpq_sgn(&m3.m_mpq) == 0) {
                    REQUIRE_THROWS_PREDICATE(div(n1, n2, n3), zero_division_error, [](const zero_division_error &ex) {
                        return std::string(ex.what()) == "Zero divisor in rational division";
                    });
                    REQUIRE_THROWS_PREDICATE(n2 / n3, zero_division_error, [](const zero_division_error &ex) {
                        return std::string(ex.what()) == "Zero divisor in rational division";
                    });
                    continue;
                }
                div(n1, n2, n3);
                mpq_div(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2 / n3) == lex_cast(m1)));
                // Various variations of in-place.
                auto n1_old(n1);
                auto n2_old(n2);
                if (n2.sgn() != 0) {
                    div(n1, n1, n2);
                    mpq_div(&m1.m_mpq, &m1.m_mpq, &m2.m_mpq);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    REQUIRE((lex_cast(n1_old / n2) == lex_cast(m1)));
                    div(n2, n1, n2);
                    mpq_div(&m2.m_mpq, &m1.m_mpq, &m2.m_mpq);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    REQUIRE((lex_cast(n2) == lex_cast(m2)));
                    REQUIRE((lex_cast(n1 / n2_old) == lex_cast(m2)));
                }
                n1_old = n1;
                if (n1.sgn() != 0) {
                    div(n1, n1, n1);
                    mpq_div(&m1.m_mpq, &m1.m_mpq, &m1.m_mpq);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    REQUIRE((lex_cast(n1 / n1) == lex_cast(m1)));
                }
                // Tests with integral arguments.
                auto n2_copy(n2);
                auto n3_copy(n3);
                detail::mpq_raii m2_copy, m3_copy;
                mpq_set(&m2_copy.m_mpq, &m2.m_mpq);
                mpq_set(&m3_copy.m_mpq, &m3.m_mpq);
                n2_copy._get_den() = 1;
                mpz_set_si(mpq_denref(&m2_copy.m_mpq), 1);
                if (n3_copy.sgn() != 0) {
                    div(n1, n2_copy, n3_copy);
                    mpq_div(&m1.m_mpq, &m2_copy.m_mpq, &m3_copy.m_mpq);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    REQUIRE((lex_cast(n2_copy.get_num() / n3_copy) == lex_cast(m1)));
                }
                if (n2_copy.sgn() != 0) {
                    div(n1, n3_copy, n2_copy);
                    mpq_div(&m1.m_mpq, &m3_copy.m_mpq, &m2_copy.m_mpq);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    REQUIRE((lex_cast(n3_copy / n2_copy.get_num()) == lex_cast(m1)));
                }
                n3_copy._get_den() = 1;
                mpz_set_si(mpq_denref(&m3_copy.m_mpq), 1);
                if (n3_copy.sgn() != 0) {
                    div(n1, n2_copy, n3_copy);
                    mpq_div(&m1.m_mpq, &m2_copy.m_mpq, &m3_copy.m_mpq);
                    REQUIRE((lex_cast(n1) == lex_cast(m1)));
                    REQUIRE((lex_cast(n2_copy / n3_copy.get_num()) == lex_cast(m1)));
                    REQUIRE((lex_cast(n2_copy.get_num() / n3_copy) == lex_cast(m1)));
                    REQUIRE_THROWS_PREDICATE(n2_copy.get_num() / rational{}, zero_division_error,
                                             [](const zero_division_error &ex) {
                                                 return std::string(ex.what()) == "Zero divisor in rational division";
                                             });
                    REQUIRE_THROWS_PREDICATE(n2_copy / rational{}.get_num(), zero_division_error,
                                             [](const zero_division_error &ex) {
                                                 return std::string(ex.what()) == "Zero divisor in rational division";
                                             });
                }
                // Tests with equal dens. This checks that
                // the den of the retval is handled correctly.
                n1 = "3/2";
                n2_copy = n2;
                n3_copy = n3;
                n2_copy._get_num() = n3_copy.get_den() + 1;
                n2_copy._get_den() = n3_copy.get_den();
                if (n3_copy.sgn() != 0) {
                    div(n1, n2_copy, n3_copy);
                    REQUIRE((lex_cast(n1) == lex_cast(rational{n3_copy.get_den() + 1, n3_copy.get_num()})));
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

TEST_CASE("div")
{
    tuple_for_each(sizes{}, div_tester{});
}
