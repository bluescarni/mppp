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

struct hash_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        const std::hash<integer> hasher{};
        integer n1, n2;
        REQUIRE((hash(n1) == 0u));
        REQUIRE((hasher(n1) == 0u));
        n1.promote();
        REQUIRE((hash(n1) == 0u));
        REQUIRE((hasher(n1) == 0u));
        n1 = integer{12};
        n2 = n1;
        REQUIRE(n2.is_static());
        n1.promote();
        REQUIRE(n1.is_dynamic());
        REQUIRE((hash(n1) == hash(n2)));
        REQUIRE((hasher(n1) == hash(n2)));
        n1 = integer{-12};
        n2 = n1;
        REQUIRE(n2.is_static());
        n1.promote();
        REQUIRE(n1.is_dynamic());
        REQUIRE((hash(n1) == hash(n2)));
        REQUIRE((hash(n1) == hasher(n2)));
        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x number of limbs.
        auto random_xy = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                n1 = integer(detail::mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    n1.neg();
                }
                n2 = n1;
                if (n2.is_static()) {
                    n1.promote();
                }
                REQUIRE((hash(n1) == hash(n2)));
                REQUIRE((hasher(n1) == hash(n2)));
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
