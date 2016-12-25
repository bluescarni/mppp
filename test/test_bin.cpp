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
using namespace mppp::mppp_impl;
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
