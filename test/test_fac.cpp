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
#include <random>
#include <stdexcept>
#include <string>
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

struct fac_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        // Start with all zeroes.
        mpz_raii m1;
        integer n1;
        ::mpz_fac_ui(&m1.m_mpz, 0);
        fac_ui(n1, 0);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        // Try one.
        ::mpz_fac_ui(&m1.m_mpz, 1);
        fac_ui(n1, 1);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        // Try two.
        ::mpz_fac_ui(&m1.m_mpz, 2);
        fac_ui(n1, 2);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        // Try 4.
        ::mpz_fac_ui(&m1.m_mpz, 4);
        fac_ui(n1, 4);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        // Try 10.
        ::mpz_fac_ui(&m1.m_mpz, 10);
        fac_ui(n1, 10);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        // Try the limit.
        ::mpz_fac_ui(&m1.m_mpz, 1000000ul);
        fac_ui(n1, 1000000ul);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        // Error testing.
        REQUIRE_THROWS_PREDICATE(fac_ui(n1, 1000001ul), std::invalid_argument, [](const std::invalid_argument &ex) {
            return std::string(ex.what()) == "The value 1000001 is too large to be used as input for the factorial "
                                             "function (the maximum allowed value is 1000000)";
        });
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        std::uniform_int_distribution<unsigned long> udist(0u, 100u);
        for (int i = 0; i < ntries; ++i) {
            if (sdist(rng) && sdist(rng) && sdist(rng)) {
                // Reset rop every once in a while.
                n1 = integer{};
            }
            if (n1.is_static() && sdist(rng)) {
                // Promote sometimes, if possible.
                n1.promote();
            }
            const auto x = udist(rng);
            ::mpz_fac_ui(&m1.m_mpz, x);
            fac_ui(n1, x);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
        }
    }
};

TEST_CASE("fac")
{
    tuple_for_each(sizes{}, fac_tester{});
}
