// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <functional>
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

struct hash_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        const std::hash<rational> hasher{};
        rational n1;
        const auto orig_h = hash(n1);
        REQUIRE((hash(n1) == hash(n1.get_num()) + hash(n1.get_den())));
        REQUIRE((hasher(n1) == hash(n1)));
        n1._get_num().promote();
        REQUIRE((hash(n1) == orig_h));
        REQUIRE((hasher(n1) == orig_h));
        n1._get_den().promote();
        REQUIRE((hash(n1) == orig_h));
        REQUIRE((hasher(n1) == orig_h));
        n1._get_num().demote();
        REQUIRE((hash(n1) == orig_h));
        REQUIRE((hasher(n1) == orig_h));
        detail::mpq_raii tmp;
        detail::mpz_raii num, den;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x number of limbs.
        auto random_xy = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = rational{};
                }
                random_integer(num, x, rng);
                random_integer(den, x, rng);
                mpz_set(mpq_numref(&tmp.m_mpq), &num.m_mpz);
                mpz_set(mpq_denref(&tmp.m_mpq), &den.m_mpz);
                if (mpz_sgn(mpq_denref(&tmp.m_mpq)) == 0) {
                    mpz_set_ui(mpq_denref(&tmp.m_mpq), 1u);
                }
                mpq_canonicalize(&tmp.m_mpq);
                n1 = &tmp.m_mpq;
                REQUIRE((hash(n1) == hash(n1.get_num()) + hash(n1.get_den())));
                REQUIRE((hasher(n1) == hash(n1)));
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);
    }
};

TEST_CASE("hash")
{
    tuple_for_each(sizes{}, hash_tester{});
}
