// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <gmp.h>
#include <random>
#include <stdexcept>
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

struct add_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        // Start with all zeroes.
        mpq_raii m1, m2, m3;
        rational n1, n2, n3;
        add(n1, n2, n3);
        ::mpq_add(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE(n1.get_num().is_static());
        REQUIRE(n1.get_den().is_static());
        REQUIRE(n2.get_num().is_static());
        REQUIRE(n2.get_den().is_static());
        REQUIRE(n3.get_num().is_static());
        REQUIRE(n3.get_den().is_static());
        mpq_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                random_rational(tmp, x, rng);
                ::mpq_set(&m2.m_mpq, &tmp.m_mpq);
                n2 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    ::mpq_neg(&m2.m_mpq, &m2.m_mpq);
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
                ::mpq_set(&m3.m_mpq, &tmp.m_mpq);
                n3 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    ::mpq_neg(&m3.m_mpq, &m3.m_mpq);
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
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = rational{};
                }
                add(n1, n2, n3);
                ::mpq_add(&m1.m_mpq, &m2.m_mpq, &m3.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // Various variations of in-place.
                add(n1, n1, n2);
                ::mpq_add(&m1.m_mpq, &m1.m_mpq, &m2.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                add(n2, n1, n2);
                ::mpq_add(&m2.m_mpq, &m1.m_mpq, &m2.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
                add(n1, n1, n1);
                ::mpq_add(&m1.m_mpq, &m1.m_mpq, &m1.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // Tests with integral arguments.
                auto n2_copy(n2);
                auto n3_copy(n3);
                mpq_raii m2_copy, m3_copy;
                ::mpq_set(&m2_copy.m_mpq, &m2.m_mpq);
                ::mpq_set(&m3_copy.m_mpq, &m3.m_mpq);
                n2_copy._get_den() = 1;
                ::mpz_set_si(mpq_denref(&m2_copy.m_mpq), 1);
                add(n1, n2_copy, n3_copy);
                ::mpq_add(&m1.m_mpq, &m2_copy.m_mpq, &m3_copy.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                add(n1, n3_copy, n2_copy);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                n3_copy._get_den() = 1;
                ::mpz_set_si(mpq_denref(&m3_copy.m_mpq), 1);
                add(n1, n2_copy, n3_copy);
                ::mpq_add(&m1.m_mpq, &m2_copy.m_mpq, &m3_copy.m_mpq);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                // Tests with equal dens. This checks that
                // the den of the retval is handled correctly.
                n1 = "3/2";
                n2_copy = n2;
                n3_copy = n3;
                n2_copy._get_num() = n3_copy.get_den() + 1;
                n2_copy._get_den() = n3_copy.get_den();
                add(n1, n2_copy, n3_copy);
                REQUIRE((lex_cast(n1) == lex_cast(rational{n2_copy.get_num() + n3_copy.get_num(), n3_copy.get_den()})));
#if 0
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
#endif
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
