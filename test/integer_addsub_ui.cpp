// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <gmp.h>
#include <limits>
#include <random>
#include <tuple>
#include <type_traits>

#include <mp++/integer.hpp>

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

struct add_ui_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        mpz_raii m1, m2;
        integer n1, n2;
        REQUIRE(&add_ui(n1, n2, 0u) == &n1);
        ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 0);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        // Ones and zeroes.
        add_ui(n1, n2, 1u);
        ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 1);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        n2 = integer{1};
        ::mpz_set_si(&m2.m_mpz, 1);
        add_ui(n1, n2, 0u);
        ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 0);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        n2 = integer{-1};
        ::mpz_set_si(&m2.m_mpz, -1);
        add_ui(n1, n2, 0u);
        ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 0);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        add_ui(n1, n2, 1u);
        ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 1);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        add_ui(n1, n2, 123u);
        ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 123);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        std::uniform_int_distribution<unsigned long> uldist(0ull, std::numeric_limits<unsigned long>::max());
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
                add_ui(n1, n2, 0u);
                ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 0);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                const auto rul = uldist(rng);
                add_ui(n1, integer{}, rul);
                mpz_raii empty;
                ::mpz_add_ui(&m1.m_mpz, &empty.m_mpz, rul);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                add_ui(n1, n2, rul);
                ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, rul);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // Overlap.
                add_ui(n2, n2, rul);
                ::mpz_add_ui(&m2.m_mpz, &m2.m_mpz, rul);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                REQUIRE((lex_cast(n2) == lex_cast(n1)));
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);

        if (S::value == 1) {
            // Tests specific for 1-limb optimisation.
            n2 = integer{GMP_NUMB_MAX};
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            add_ui(n1, n2, 0u);
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 0);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            add_ui(n1, n2, 1u);
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 1);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            add_ui(n1, n2, 123u);
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 123);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            add_ui(n2, n2, 1u);
            ::mpz_add_ui(&m2.m_mpz, &m2.m_mpz, 1);
            REQUIRE((lex_cast(n2) == lex_cast(m2)));
            n1 = integer{};
        }

        if (S::value == 2) {
            // Tests specific for 2-limb optimisation.
            n2 = integer{GMP_NUMB_MAX};
            mul_2exp(n2, n2, GMP_NUMB_BITS);
            add(n2, n2, integer{GMP_NUMB_MAX});
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            add_ui(n1, n2, 0u);
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 0);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            add_ui(n1, n2, 1u);
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 1);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            add_ui(n1, n2, 123u);
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 123);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            add_ui(n2, n2, 1u);
            ::mpz_add_ui(&m2.m_mpz, &m2.m_mpz, 1);
            REQUIRE((lex_cast(n2) == lex_cast(m2)));
            n1 = integer{};
        }

        if (S::value > 2) {
            // Tests specific for mpn implementation.
            n2 = integer{GMP_NUMB_MAX};
            for (auto i = 1u; i < S::value; ++i) {
                mul_2exp(n2, n2, GMP_NUMB_BITS);
                add(n2, n2, integer{GMP_NUMB_MAX});
            }
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            add_ui(n1, n2, 0u);
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 0);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            add_ui(n1, n2, 1u);
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 1);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            n2 = integer{};
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            add_ui(n1, n2, static_cast<unsigned long>(-1));
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, static_cast<unsigned long>(-1));
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n2 = integer{GMP_NUMB_MAX};
            mul_2exp(n2, n2, GMP_NUMB_BITS);
            add(n2, n2, integer{GMP_NUMB_MAX});
            n2.neg();
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            add_ui(n1, n2, 123u);
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 123);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            n2 = integer{GMP_NUMB_MAX};
            n2.neg();
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            add_ui(n1, n2, 123u);
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 123);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            n2 = integer{1};
            n2.neg();
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            add_ui(n1, n2, 123u);
            ::mpz_add_ui(&m1.m_mpz, &m2.m_mpz, 123);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
        }
    }
};

TEST_CASE("add_ui")
{
    tuple_for_each(sizes{}, add_ui_tester{});
}

struct sub_ui_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        mpz_raii m1, m2;
        integer n1, n2;
        REQUIRE(&sub_ui(n1, n2, 0u) == &n1);
        ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 0);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        // Ones and zeroes.
        sub_ui(n1, n2, 1u);
        ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 1);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        n2 = integer{1};
        ::mpz_set_si(&m2.m_mpz, 1);
        sub_ui(n1, n2, 0u);
        ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 0);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        n2 = integer{-1};
        ::mpz_set_si(&m2.m_mpz, -1);
        sub_ui(n1, n2, 0u);
        ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 0);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        sub_ui(n1, n2, 1u);
        ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 1);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        sub_ui(n1, n2, 123u);
        ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 123);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        std::uniform_int_distribution<unsigned long> uldist(0ull, std::numeric_limits<unsigned long>::max());
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
                sub_ui(n1, n2, 0u);
                ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 0);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                const auto rul = uldist(rng);
                sub_ui(n1, integer{}, rul);
                mpz_raii empty;
                ::mpz_sub_ui(&m1.m_mpz, &empty.m_mpz, rul);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                sub_ui(n1, n2, rul);
                ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, rul);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // Overlap.
                sub_ui(n2, n2, rul);
                ::mpz_sub_ui(&m2.m_mpz, &m2.m_mpz, rul);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                REQUIRE((lex_cast(n2) == lex_cast(n1)));
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);

        if (S::value == 1) {
            // Tests specific for 1-limb optimisation.
            n2 = integer{GMP_NUMB_MAX};
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            sub_ui(n1, n2, 0u);
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 0);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            sub_ui(n1, n2, 1u);
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 1);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            sub_ui(n1, n2, 123u);
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 123);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            sub_ui(n2, n2, 1u);
            ::mpz_sub_ui(&m2.m_mpz, &m2.m_mpz, 1);
            REQUIRE((lex_cast(n2) == lex_cast(m2)));
            n1 = integer{};
        }

        if (S::value == 2) {
            // Tests specific for 2-limb optimisation.
            n2 = integer{GMP_NUMB_MAX};
            mul_2exp(n2, n2, GMP_NUMB_BITS);
            add(n2, n2, integer{GMP_NUMB_MAX});
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            sub_ui(n1, n2, 0u);
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 0);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            sub_ui(n1, n2, 1u);
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 1);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            sub_ui(n1, n2, 123u);
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 123);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            sub_ui(n2, n2, 1u);
            ::mpz_sub_ui(&m2.m_mpz, &m2.m_mpz, 1);
            REQUIRE((lex_cast(n2) == lex_cast(m2)));
            n1 = integer{};
        }

        if (S::value > 2) {
            // Tests specific for mpn implementation.
            n2 = integer{GMP_NUMB_MAX};
            for (auto i = 1u; i < S::value; ++i) {
                mul_2exp(n2, n2, GMP_NUMB_BITS);
                add(n2, n2, integer{GMP_NUMB_MAX});
            }
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            sub_ui(n1, n2, 0u);
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 0);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            sub_ui(n1, n2, 1u);
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 1);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            n2 = integer{};
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            sub_ui(n1, n2, static_cast<unsigned long>(-1));
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, static_cast<unsigned long>(-1));
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n2 = integer{GMP_NUMB_MAX};
            mul_2exp(n2, n2, GMP_NUMB_BITS);
            add(n2, n2, integer{GMP_NUMB_MAX});
            n2.neg();
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            sub_ui(n1, n2, 123u);
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 123);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            n2 = integer{GMP_NUMB_MAX};
            n2.neg();
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            sub_ui(n1, n2, 123u);
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 123);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
            n2 = integer{1};
            n2.neg();
            ::mpz_set(&m2.m_mpz, n2.get_mpz_view());
            sub_ui(n1, n2, 123u);
            ::mpz_sub_ui(&m1.m_mpz, &m2.m_mpz, 123);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            n1 = integer{};
        }
    }
};

TEST_CASE("sub_ui")
{
    tuple_for_each(sizes{}, sub_ui_tester{});
}
