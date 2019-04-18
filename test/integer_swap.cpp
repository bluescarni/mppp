// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
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

#include <mp++/detail/gmp.hpp>
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

struct swap_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;

        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);

        auto random_x = [&](unsigned x) {
            integer n1, n2;

            for (int i = 0; i < ntries; ++i) {
                // Reset ops every once in a while.
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                }
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n2 = integer{};
                }

                // Create randoms.
                random_integer(tmp, x, rng);
                n1 = &tmp.m_mpz;
                random_integer(tmp, x, rng);
                n2 = &tmp.m_mpz;

                // Promote sometimes, if possible.
                if (sdist(rng) && n1.is_static()) {
                    n1.promote();
                }
                if (sdist(rng) && n2.is_static()) {
                    n2.promote();
                }

                // Check staticness.
                const auto s1 = n1.is_static();
                const auto s2 = n2.is_static();

                // Create copies.
                const auto n1_copy(n1), n2_copy(n2);

                // Do the swap.
                swap(n1, n2);

                REQUIRE(n2 == n1_copy);
                REQUIRE(n1 == n2_copy);

                // Check staticness is preserved.
                REQUIRE(s1 == n2.is_static());
                REQUIRE(s2 == n1.is_static());
            };
        };

        random_x(0);
        random_x(1);
        random_x(2);
        random_x(3);
        random_x(4);
        random_x(5);
        random_x(6);
    }
};

TEST_CASE("swap")
{
    tuple_for_each(sizes{}, swap_tester{});
}
