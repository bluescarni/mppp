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

#include <gmp.h>

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

struct tdiv_q_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        detail::mpz_raii m1, m3, m4;
        integer n1, n3, n4;
        // A few simple tests to start.
        n3 = integer(12);
        n4 = integer(5);
        mpz_set_ui(&m3.m_mpz, 12);
        mpz_set_ui(&m4.m_mpz, 5);
        tdiv_q(n1, n3, n4);
        REQUIRE(std::is_same<integer &, decltype(tdiv_q(n1, n3, n4))>::value);
        mpz_tdiv_q(&m1.m_mpz, &m3.m_mpz, &m4.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        n3 = integer(-12);
        mpz_set_si(&m3.m_mpz, -12);
        tdiv_q(n1, n3, n4);
        mpz_tdiv_q(&m1.m_mpz, &m3.m_mpz, &m4.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        n4 = integer(-5);
        mpz_set_si(&m4.m_mpz, -5);
        tdiv_q(n1, n3, n4);
        mpz_tdiv_q(&m1.m_mpz, &m3.m_mpz, &m4.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        n3 = integer(12);
        mpz_set_ui(&m3.m_mpz, 12);
        tdiv_q(n1, n3, n4);
        mpz_tdiv_q(&m1.m_mpz, &m3.m_mpz, &m4.m_mpz);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        // Random testing.
        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                // Helper to generate randomly dividend and divisor.
                auto random_34 = [&]() {
                    random_integer(tmp, x, rng);
                    mpz_set(&m3.m_mpz, &tmp.m_mpz);
                    n3 = &tmp.m_mpz;
                    if (sdist(rng)) {
                        mpz_neg(&m3.m_mpz, &m3.m_mpz);
                        n3.neg();
                    }
                    if (n3.is_static() && sdist(rng)) {
                        // Promote sometimes, if possible.
                        n3.promote();
                    }
                    // Make sure divisor is not zero.
                    do {
                        random_integer(tmp, y, rng);
                        mpz_set(&m4.m_mpz, &tmp.m_mpz);
                        n4 = &tmp.m_mpz;
                        if (sdist(rng)) {
                            mpz_neg(&m4.m_mpz, &m4.m_mpz);
                            n4.neg();
                        }
                        if (n4.is_static() && sdist(rng)) {
                            // Promote sometimes, if possible.
                            n4.promote();
                        }
                    } while (n4.sgn() == 0);
                };
                random_34();
                // Reset rops every once in a while.
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                    mpz_set_ui(&m1.m_mpz, 0);
                }
                tdiv_q(n1, n3, n4);
                mpz_tdiv_q(&m1.m_mpz, &m3.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // In-place variations.
                random_34();
                tdiv_q(n1, n3, n4);
                mpz_tdiv_q(&m1.m_mpz, &m3.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                random_34();
                tdiv_q(n1, n3, n4);
                mpz_tdiv_q(&m1.m_mpz, &m3.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                random_34();
                tdiv_q(n1, n4, n4);
                mpz_tdiv_q(&m1.m_mpz, &m4.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                random_34();
                tdiv_q(n1, n4, n4);
                mpz_tdiv_q(&m1.m_mpz, &m4.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                random_34();
                tdiv_q(n4, n4, n4);
                mpz_tdiv_q(&m4.m_mpz, &m4.m_mpz, &m4.m_mpz);
                REQUIRE((lex_cast(n4) == lex_cast(m4)));
            }
            // Error handling.
            n3 = integer(12);
            n4 = integer(0);
            REQUIRE_THROWS_PREDICATE(tdiv_q(n1, n3, n4), zero_division_error, [](const zero_division_error &ex) {
                return std::string(ex.what()) == "Integer division by zero";
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

TEST_CASE("tdiv_q")
{
    tuple_for_each(sizes{}, tdiv_q_tester{});
}
