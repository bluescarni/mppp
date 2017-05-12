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

#include <mp++/mp++.hpp>

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

struct sqrt_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        mpz_raii m1, m2;
        integer n1, n2;
        ::mpz_sqrt(&m1.m_mpz, &m2.m_mpz);
        sqrt(n1, n2);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        REQUIRE((lex_cast(sqrt(n2)) == lex_cast(m1)));
        REQUIRE(sqrt(n2).is_static());
        n2.sqrt();
        REQUIRE((lex_cast(n2) == lex_cast(m1)));
        REQUIRE(n2.is_static());
        // Try one.
        n2 = integer{1};
        ::mpz_set_ui(&m2.m_mpz, 1u);
        ::mpz_sqrt(&m1.m_mpz, &m2.m_mpz);
        sqrt(n1, n2);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        REQUIRE((lex_cast(sqrt(n2)) == lex_cast(m1)));
        REQUIRE(sqrt(n2).is_static());
        n2.sqrt();
        REQUIRE((lex_cast(n2) == lex_cast(m1)));
        REQUIRE(n2.is_static());
        // Two.
        n2 = integer{2};
        ::mpz_set_ui(&m2.m_mpz, 2u);
        ::mpz_sqrt(&m1.m_mpz, &m2.m_mpz);
        sqrt(n1, n2);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        REQUIRE((lex_cast(sqrt(n2)) == lex_cast(m1)));
        REQUIRE(sqrt(n2).is_static());
        n2.sqrt();
        REQUIRE((lex_cast(n2) == lex_cast(m1)));
        REQUIRE(n2.is_static());
        // Four.
        n2 = integer{4};
        ::mpz_set_ui(&m2.m_mpz, 4u);
        ::mpz_sqrt(&m1.m_mpz, &m2.m_mpz);
        sqrt(n1, n2);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        REQUIRE((lex_cast(sqrt(n2)) == lex_cast(m1)));
        REQUIRE(sqrt(n2).is_static());
        n2.sqrt();
        REQUIRE((lex_cast(n2) == lex_cast(m1)));
        REQUIRE(n2.is_static());
        // Ten.
        n2 = integer{10};
        ::mpz_set_ui(&m2.m_mpz, 10u);
        ::mpz_sqrt(&m1.m_mpz, &m2.m_mpz);
        sqrt(n1, n2);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        REQUIRE((lex_cast(sqrt(n2)) == lex_cast(m1)));
        REQUIRE(sqrt(n2).is_static());
        n2.sqrt();
        REQUIRE((lex_cast(n2) == lex_cast(m1)));
        REQUIRE(n2.is_static());
        // Error testing.
        n2 = integer{-1};
        REQUIRE_THROWS_PREDICATE(sqrt(n1, n2), std::domain_error, [](const std::domain_error &ex) {
            return std::string(ex.what()) == "Cannot compute the square root of the negative number -1";
        });
        REQUIRE_THROWS_PREDICATE(sqrt(integer{-2}), std::domain_error, [](const std::domain_error &ex) {
            return std::string(ex.what()) == "Cannot compute the square root of the negative number -2";
        });
        n2 = integer{-3};
        REQUIRE_THROWS_PREDICATE(n2.sqrt(), std::domain_error, [](const std::domain_error &ex) {
            return std::string(ex.what()) == "Cannot compute the square root of the negative number -3";
        });
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x number of limbs.
        auto random_xy = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                random_integer(tmp, x, rng);
                ::mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(mpz_to_str(&tmp.m_mpz));
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                ::mpz_sqrt(&m1.m_mpz, &m2.m_mpz);
                sqrt(n1, n2);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(sqrt(n2)) == lex_cast(m1)));
                n2.sqrt();
                REQUIRE((lex_cast(n2) == lex_cast(m1)));
                // Overlap.
                n2 = integer(mpz_to_str(&m2.m_mpz));
                ::mpz_sqrt(&m2.m_mpz, &m2.m_mpz);
                sqrt(n2, n2);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);
    }
};

TEST_CASE("sqrt")
{
    tuple_for_each(sizes{}, sqrt_tester{});
}
