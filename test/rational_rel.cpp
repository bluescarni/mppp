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

static inline bool check_cmp(int c1, int c2)
{
    if (c1 < 0) {
        return c2 < 0;
    }
    if (c1 == 0) {
        return c2 == 0;
    }
    return c2 > 0;
}

struct cmp_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        // Start with all zeroes.
        detail::mpq_raii m1, m2;
        rational n1, n2;
        REQUIRE(check_cmp(cmp(n1, n2), mpq_cmp(&m1.m_mpq, &m2.m_mpq)));
        detail::mpq_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                random_rational(tmp, x, rng);
                mpq_set(&m1.m_mpq, &tmp.m_mpq);
                n1 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    mpq_neg(&m1.m_mpq, &m1.m_mpq);
                    n1.neg();
                }
                if (n1.get_num().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n1._get_num().promote();
                }
                if (n1.get_den().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n1._get_den().promote();
                }
                random_rational(tmp, y, rng);
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
                REQUIRE(check_cmp(cmp(n1, n2), mpq_cmp(&m1.m_mpq, &m2.m_mpq)));
                REQUIRE(check_cmp(cmp(n1, n1), mpq_cmp(&m1.m_mpq, &m1.m_mpq)));
                REQUIRE(check_cmp(cmp(n2, n2), mpq_cmp(&m2.m_mpq, &m2.m_mpq)));
                REQUIRE((n1 == n1));
                REQUIRE((n2 == n2));
                if (mpq_cmp(&m1.m_mpq, &m2.m_mpq)) {
                    REQUIRE((n1 != n2));
                } else {
                    REQUIRE((n1 == n2));
                }
                // Test the integer versions as well.
                REQUIRE(check_cmp(cmp(n1, n2.get_num()), mpq_cmp_z(&m1.m_mpq, mpq_numref(&m2.m_mpq))));
                REQUIRE(check_cmp(cmp(n2.get_num(), n1), -mpq_cmp_z(&m1.m_mpq, mpq_numref(&m2.m_mpq))));
                n2._get_den() = 1;
                REQUIRE(cmp(n2, n2.get_num()) == 0);
                REQUIRE(cmp(n2.get_num(), n2) == 0);
                n2 = n1;
                mpq_set(&m2.m_mpq, &m1.m_mpq);
                if (n2.get_num().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_num().promote();
                }
                if (n2.get_den().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_den().promote();
                }
                REQUIRE(check_cmp(cmp(n1, n2), mpq_cmp(&m1.m_mpq, &m2.m_mpq)));
                // Overlap.
                REQUIRE(check_cmp(cmp(n1, n1), mpq_cmp(&m1.m_mpq, &m1.m_mpq)));
            }
        };

        random_xy(0, 0);

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

TEST_CASE("cmp")
{
    tuple_for_each(sizes{}, cmp_tester{});
}
