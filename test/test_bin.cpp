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
#include <limits>
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

struct bin_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        // Start with all zeroes.
        mpz_raii m1, m2;
        integer n1, n2;
        ::mpz_bin_ui(&m1.m_mpz, &m2.m_mpz, 0u);
        bin_ui(n1, n2, 0);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(bin_ui(n2, 0)) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        std::uniform_int_distribution<int> sdist(0, 1);
        std::uniform_int_distribution<int> ndist(-20, 20);
        std::uniform_int_distribution<unsigned> kdist(0, 20);
        for (int i = 0; i < ntries; ++i) {
            if (sdist(rng) && sdist(rng) && sdist(rng)) {
                // Reset rop every once in a while.
                n1 = integer{};
            }
            const auto n = ndist(rng);
            const auto k = kdist(rng);
            ::mpz_set_si(&m2.m_mpz, n);
            n2 = integer(n);
            if (n1.is_static() && sdist(rng)) {
                // Promote sometimes, if possible.
                n1.promote();
            }
            if (n2.is_static() && sdist(rng)) {
                // Promote sometimes, if possible.
                n2.promote();
            }
            ::mpz_bin_ui(&m1.m_mpz, &m2.m_mpz, k);
            bin_ui(n1, n2, k);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((lex_cast(bin_ui(n2, k)) == lex_cast(m1)));
        }
    }
};

TEST_CASE("bin")
{
    tuple_for_each(sizes{}, bin_tester{});
}

struct binomial_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mp_integer<T::value>;
        int_type n;
        REQUIRE(binomial(n, 0) == 1);
        REQUIRE(binomial(n, 1) == 0);
        n = 1;
        REQUIRE(binomial(n, 1) == 1);
        n = 5;
        REQUIRE(binomial(n, 3) == 10);
        n = -5;
        REQUIRE(binomial(n, int_type(4)) == 70);
        // Random tests.
        std::uniform_int_distribution<int> ud(-1000, 1000);
        std::uniform_int_distribution<int> promote_dist(0, 1);
        mpz_raii m;
        for (int i = 0; i < ntries; ++i) {
            auto tmp1 = ud(rng), tmp2 = ud(rng);
            n = tmp1;
            if (promote_dist(rng) && n.is_static()) {
                n.promote();
            }
            if (tmp2 < 0) {
                // NOTE: we cannot check this case with GMP, defer to some tests below.
                CHECK_NOTHROW(binomial(n, tmp2));
                continue;
            }
            ::mpz_set_si(&m.m_mpz, static_cast<long>(tmp1));
            ::mpz_bin_ui(&m.m_mpz, &m.m_mpz, static_cast<unsigned long>(tmp2));
            REQUIRE(binomial(n, tmp2).to_string() == lex_cast(m));
        }
        REQUIRE_THROWS_AS(binomial(n, std::numeric_limits<unsigned long>::max() + int_type(1)), std::overflow_error);
        REQUIRE_THROWS_AS(binomial(-int_type{std::numeric_limits<unsigned long>::max()} + 1,
                                   -2 * int_type{std::numeric_limits<unsigned long>::max()}),
                          std::overflow_error);
        // Negative k.
        REQUIRE(binomial(int_type{-3}, -4) == -3);
        REQUIRE(binomial(int_type{-3}, -10) == -36);
        REQUIRE(binomial(int_type{-3}, -1) == 0);
        REQUIRE(binomial(int_type{3}, -1) == 0);
        REQUIRE(binomial(int_type{10}, -1) == 0);
        REQUIRE(binomial(int_type{-3}, -3) == 1);
        REQUIRE(binomial(int_type{-1}, -1) == 1);
    }
};

TEST_CASE("binomial")
{
    tuple_for_each(sizes{}, binomial_tester{});
}
