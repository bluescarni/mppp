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

#include <mp++/detail/gmp.hpp>
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

struct gcd_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with zeroes.
        detail::mpz_raii m1, m2, m3;
        integer n1, n2, n3;
        REQUIRE(gcd(n2, n3) == 0);
        REQUIRE(std::is_same<integer, decltype(gcd(n2, n3))>::value);
        REQUIRE(gcd(n1, n2, n3) == 0);
        REQUIRE(std::is_same<integer &, decltype(gcd(n1, n2, n3))>::value);
        mpz_set_si(&m3.m_mpz, 1);
        n3 = integer(1);
        mpz_gcd(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        REQUIRE(&gcd(n1, n2, n3) == &n1);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(gcd(n2, n3)) == lex_cast(m1)));
        mpz_set_si(&m3.m_mpz, -2);
        n3 = integer(-2);
        mpz_gcd(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        gcd(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(gcd(n2, n3)) == lex_cast(m1)));
        // Simple tests.
        mpz_set_si(&m2.m_mpz, 8);
        n2 = integer(8);
        mpz_set_si(&m3.m_mpz, 2);
        n3 = integer(2);
        mpz_gcd(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        gcd(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(gcd(n2, n3)) == lex_cast(m1)));
        mpz_set_si(&m2.m_mpz, 8);
        n2 = integer(8);
        mpz_set_si(&m3.m_mpz, 0);
        n3 = integer(0);
        mpz_gcd(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        gcd(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(gcd(n2, n3)) == lex_cast(m1)));
        mpz_set_si(&m2.m_mpz, 16);
        n2 = integer(16);
        mpz_set_si(&m3.m_mpz, -2);
        n3 = integer(-2);
        mpz_gcd(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        gcd(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(gcd(n2, n3)) == lex_cast(m1)));
        mpz_set_si(&m2.m_mpz, -32);
        n2 = integer(-32);
        mpz_set_si(&m3.m_mpz, 4);
        n3 = integer(4);
        mpz_gcd(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        gcd(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(gcd(n2, n3)) == lex_cast(m1)));
        mpz_set_si(&m2.m_mpz, -32);
        n2 = integer(-32);
        mpz_set_si(&m3.m_mpz, -4);
        n3 = integer(-4);
        mpz_gcd(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        gcd(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(gcd(n2, n3)) == lex_cast(m1)));
        // Random testing.
        std::uniform_int_distribution<int> sdist(0, 1), mdist(1, 3);
        detail::mpz_raii tmp;
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                random_integer(tmp, x, rng);
                mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(detail::mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                random_integer(tmp, y, rng);
                mpz_set(&m3.m_mpz, &tmp.m_mpz);
                n3 = integer(detail::mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                if (n3.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3.promote();
                }
                gcd(n1, n2, n3);
                mpz_gcd(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(gcd(n2, n3)) == lex_cast(m1)));
                gcd(n1, n3, n2);
                mpz_gcd(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(gcd(n3, n2)) == lex_cast(m1)));
                // Overlapping.
                gcd(n1, n2, n2);
                mpz_gcd(&m1.m_mpz, &m2.m_mpz, &m2.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                gcd(n2, n2, n2);
                mpz_gcd(&m2.m_mpz, &m2.m_mpz, &m2.m_mpz);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
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

TEST_CASE("gcd")
{
    tuple_for_each(sizes{}, gcd_tester{});
}

struct lcm_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with zeroes.
        detail::mpz_raii m1, m2, m3;
        integer n1, n2, n3;
        REQUIRE(lcm(n2, n3) == 0);
        REQUIRE(std::is_same<integer, decltype(lcm(n2, n3))>::value);
        REQUIRE(lcm(n1, n2, n3) == 0);
        REQUIRE(std::is_same<integer &, decltype(lcm(n1, n2, n3))>::value);
        mpz_set_si(&m3.m_mpz, 1);
        n3 = integer(1);
        mpz_lcm(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        REQUIRE(&lcm(n1, n2, n3) == &n1);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(lcm(n2, n3)) == lex_cast(m1)));
        mpz_set_si(&m3.m_mpz, -2);
        n3 = integer(-2);
        mpz_lcm(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        lcm(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(lcm(n2, n3)) == lex_cast(m1)));
        // Simple tests.
        mpz_set_si(&m2.m_mpz, 8);
        n2 = integer(8);
        mpz_set_si(&m3.m_mpz, 2);
        n3 = integer(2);
        mpz_lcm(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        lcm(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(lcm(n2, n3)) == lex_cast(m1)));
        mpz_set_si(&m2.m_mpz, 8);
        n2 = integer(8);
        mpz_set_si(&m3.m_mpz, 0);
        n3 = integer(0);
        mpz_lcm(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        lcm(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(lcm(n2, n3)) == lex_cast(m1)));
        mpz_set_si(&m2.m_mpz, 16);
        n2 = integer(16);
        mpz_set_si(&m3.m_mpz, -2);
        n3 = integer(-2);
        mpz_lcm(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        lcm(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(lcm(n2, n3)) == lex_cast(m1)));
        mpz_set_si(&m2.m_mpz, -32);
        n2 = integer(-32);
        mpz_set_si(&m3.m_mpz, 4);
        n3 = integer(4);
        mpz_lcm(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        lcm(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(lcm(n2, n3)) == lex_cast(m1)));
        mpz_set_si(&m2.m_mpz, -32);
        n2 = integer(-32);
        mpz_set_si(&m3.m_mpz, -4);
        n3 = integer(-4);
        mpz_lcm(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        lcm(n1, n2, n3);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(lcm(n2, n3)) == lex_cast(m1)));
        // Random testing.
        std::uniform_int_distribution<int> sdist(0, 1), mdist(1, 3);
        detail::mpz_raii tmp;
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                random_integer(tmp, x, rng);
                mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(detail::mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                random_integer(tmp, y, rng);
                mpz_set(&m3.m_mpz, &tmp.m_mpz);
                n3 = integer(detail::mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                if (n3.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3.promote();
                }
                lcm(n1, n2, n3);
                mpz_lcm(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(lcm(n2, n3)) == lex_cast(m1)));
                lcm(n1, n3, n2);
                mpz_lcm(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(lcm(n3, n2)) == lex_cast(m1)));
                // Overlapping.
                lcm(n1, n2, n2);
                mpz_lcm(&m1.m_mpz, &m2.m_mpz, &m2.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                lcm(n2, n2, n2);
                mpz_lcm(&m2.m_mpz, &m2.m_mpz, &m2.m_mpz);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
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

TEST_CASE("lcm")
{
    tuple_for_each(sizes{}, lcm_tester{});

    // Try some tests with integer<1> that will result
    // in 2-limb results.
    using int_t = integer<1>;

    int_t n1{::mp_limb_t(-1)}, n2 = n1 - 1, n3;
    int_t m1{n1}, m2{n2}, m3;

    lcm(n3, n1, n2);
    mpz_lcm(m3.get_mpz_t(), m1.get_mpz_view(), m2.get_mpz_view());

    REQUIRE(n3 == m3);
    REQUIRE(lcm(n1, n2) == n3);

    n1.neg();
    m1.neg();

    lcm(n3, n1, n2);
    mpz_lcm(m3.get_mpz_t(), m1.get_mpz_view(), m2.get_mpz_view());

    REQUIRE(n3 == m3);
    REQUIRE(lcm(n1, n2) == n3);

    n2.neg();
    m2.neg();

    lcm(n3, n1, n2);
    mpz_lcm(m3.get_mpz_t(), m1.get_mpz_view(), m2.get_mpz_view());

    REQUIRE(n3 == m3);
    REQUIRE(lcm(n1, n2) == n3);

    n1.neg();
    m1.neg();

    lcm(n3, n1, n2);
    mpz_lcm(m3.get_mpz_t(), m1.get_mpz_view(), m2.get_mpz_view());

    REQUIRE(n3 == m3);
    REQUIRE(lcm(n1, n2) == n3);
}
