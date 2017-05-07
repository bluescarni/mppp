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

#include <cmath>
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

struct pow_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        mpz_raii m1, m2;
        integer n1, n2;
        ::mpz_pow_ui(&m1.m_mpz, &m2.m_mpz, 0u);
        pow_ui(n1, n2, 0);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(pow_ui(n2, 0)) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        std::uniform_int_distribution<unsigned> edist(0, 20);
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
                if (sdist(rng)) {
                    ::mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                const unsigned ex = edist(rng);
                ::mpz_pow_ui(&m1.m_mpz, &m2.m_mpz, ex);
                pow_ui(n1, n2, ex);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(pow_ui(n2, ex)) == lex_cast(m1)));
                // Overlap.
                ::mpz_pow_ui(&m2.m_mpz, &m2.m_mpz, ex);
                pow_ui(n2, n2, ex);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);

        // Tests for the convenience pow() overloads.
        REQUIRE(pow(integer{0}, 0) == 1);
        REQUIRE(pow(0, integer{0}) == 1);
        REQUIRE((std::is_same<integer, decltype(pow(integer{0}, 0))>::value));
        REQUIRE((std::is_same<integer, decltype(pow(0, integer{0}))>::value));
        REQUIRE(pow(integer{4}, 2) == 16);
        REQUIRE(pow(2, integer{4}) == 16);
        REQUIRE(pow(integer{4}, char(0)) == 1);
        REQUIRE(pow(char(4), integer{0}) == 1);
        REQUIRE((std::is_same<integer, decltype(pow(integer{0}, char(0)))>::value));
        REQUIRE((std::is_same<integer, decltype(pow(char(0), integer{0}))>::value));
        REQUIRE(pow(integer{4}, 3ull) == 64);
        REQUIRE(pow(4ull, integer{3}) == 64);
        REQUIRE((std::is_same<integer, decltype(pow(integer{0}, 0ull))>::value));
        REQUIRE((std::is_same<integer, decltype(pow(0ull, integer{0}))>::value));
        REQUIRE(pow(integer{4}, integer{4}) == 256);
        REQUIRE((std::is_same<integer, decltype(pow(integer{0}, integer{0}))>::value));
        REQUIRE(pow(integer{-4}, 2) == 16);
        REQUIRE(pow(-4, integer{2}) == 16);
        REQUIRE(pow(integer{-4}, char(0)) == 1);
        REQUIRE(pow((signed char)-4, integer{0}) == 1);
        REQUIRE(pow(integer{-4}, 3ull) == -64);
        REQUIRE(pow(integer{-4}, integer{4}) == 256);
        if (std::numeric_limits<unsigned long long>::max() > std::numeric_limits<unsigned long>::max()) {
            REQUIRE_THROWS_PREDICATE(pow(integer{-4}, std::numeric_limits<unsigned long long>::max()),
                                     std::overflow_error, [](const std::overflow_error &oe) {
                                         return oe.what()
                                                == "Cannot convert the integral value "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + " to unsigned long: the value is too large.";
                                     });
            REQUIRE_THROWS_PREDICATE(pow(integer{-4}, integer{std::numeric_limits<unsigned long long>::max()}),
                                     std::overflow_error, [](const std::overflow_error &oe) {
                                         return oe.what()
                                                == "Cannot convert the integral value "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + " to unsigned long: the value is too large.";
                                     });
            REQUIRE_THROWS_PREDICATE(pow(-4, integer{std::numeric_limits<unsigned long long>::max()}),
                                     std::overflow_error, [](const std::overflow_error &oe) {
                                         return oe.what()
                                                == "Cannot convert the integral value "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + " to unsigned long: the value is too large.";
                                     });
        }
        REQUIRE_THROWS_PREDICATE(pow(integer{0}, -1), zero_division_error, [](const zero_division_error &zde) {
            return zde.what() == std::string("cannot raise zero to the negative power -1");
        });
        REQUIRE_THROWS_PREDICATE(pow(integer{0}, -2ll), zero_division_error, [](const zero_division_error &zde) {
            return zde.what() == std::string("cannot raise zero to the negative power -2");
        });
        REQUIRE_THROWS_PREDICATE(pow(integer{0}, integer{-25}), zero_division_error,
                                 [](const zero_division_error &zde) {
                                     return zde.what() == std::string("cannot raise zero to the negative power -25");
                                 });
        REQUIRE_THROWS_PREDICATE(pow(0, integer{-1}), zero_division_error, [](const zero_division_error &zde) {
            return zde.what() == std::string("cannot raise zero to the negative power -1");
        });
        REQUIRE_THROWS_PREDICATE(pow(0ll, integer{-2ll}), zero_division_error, [](const zero_division_error &zde) {
            return zde.what() == std::string("cannot raise zero to the negative power -2");
        });
        // 1 to negative exp.
        REQUIRE(pow(integer{1}, -1) == 1);
        REQUIRE(pow(1, integer{-1}) == 1);
        REQUIRE(pow(integer{1}, (signed char)(-2)) == 1);
        REQUIRE(pow(char(1), integer{-2}) == 1);
        REQUIRE(pow(integer{1}, -3ll) == 1);
        REQUIRE(pow(1ll, integer{-3ll}) == 1);
        REQUIRE(pow(integer{1}, integer{-4ll}) == 1);
        // -1 to negative exp.
        REQUIRE(pow(integer{-1}, -1) == -1);
        REQUIRE(pow(integer{-1}, (signed char)(-2)) == 1);
        REQUIRE(pow(integer{-1}, -3ll) == -1);
        REQUIRE(pow(-1, integer{-1}) == -1);
        REQUIRE(pow(-1, integer{-2}) == 1);
        REQUIRE(pow(-1, integer{-3ll}) == -1);
        REQUIRE(pow(integer{-1}, integer{-4ll}) == 1);
        // n to negative exp.
        REQUIRE(pow(integer{2}, -1) == 0);
        REQUIRE(pow(integer{-3}, (signed char)(-2)) == 0);
        REQUIRE(pow(integer{4}, -3ll) == 0);
        REQUIRE(pow(2, integer{-1}) == 0);
        REQUIRE(pow((signed char)-3, integer{-2}) == 0);
        REQUIRE(pow(4, integer{-3ll}) == 0);
        REQUIRE(pow(integer{-5}, integer{-4}) == 0);
        // FP testing.
        REQUIRE((std::is_same<float, decltype(pow(integer{}, 0.f))>::value));
        REQUIRE((std::is_same<float, decltype(pow(0.f, integer{}))>::value));
        REQUIRE((std::is_same<double, decltype(pow(integer{}, 0.))>::value));
        REQUIRE((std::is_same<double, decltype(pow(0., integer{}))>::value));
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE((std::is_same<long double, decltype(pow(integer{}, 0.l))>::value));
        REQUIRE((std::is_same<long double, decltype(pow(0.l, integer{}))>::value));
#endif
        REQUIRE(pow(integer{2}, 4.5f) == std::pow(2.f, 4.5f));
        REQUIRE(pow(4.5f, integer{-2}) == std::pow(4.5f, -2.f));
        REQUIRE(pow(integer{2}, 4.5) == std::pow(2., 4.5));
        REQUIRE(pow(4.5, integer{-2}) == std::pow(4.5, -2.));
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE(pow(integer{2}, 4.5l) == std::pow(2.l, 4.5l));
        REQUIRE(pow(4.5l, integer{-2}) == std::pow(4.5l, -2.l));
#endif
    }
};

TEST_CASE("pow")
{
    tuple_for_each(sizes{}, pow_tester{});
}
