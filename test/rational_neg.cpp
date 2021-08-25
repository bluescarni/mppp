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

#include <mp++/rational.hpp>

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

struct neg_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        // Start with all zeroes.
        detail::mpq_raii m1, m2;
        rational n1, n2;
        mpq_neg(&m1.m_mpq, &m2.m_mpq);
        REQUIRE(&neg(n1, n2) == &n1);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        // Test the other variants.
        n1.neg();
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(neg(n1)) == lex_cast(m1)));
        detail::mpq_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x number of limbs.
        auto random_xy = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = rational{};
                }
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
                mpq_neg(&m1.m_mpq, &m2.m_mpq);
                neg(n1, n2);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(n1) == lex_cast(neg(n2))));
                n2.neg();
                REQUIRE((lex_cast(n1) == lex_cast(n2)));
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);
    }
};

TEST_CASE("neg")
{
    tuple_for_each(sizes{}, neg_tester{});
}
