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

#include <mp++/detail/gmp.hpp>
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

struct swap_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;

        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);

        auto random_xy = [&](unsigned x, unsigned y) {
            integer n1, n2, n1_copy, n2_copy;

            for (int i = 0; i < ntries; ++i) {
                // Reset ops every once in a while.
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n1 = integer{};
                }
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    n2 = integer{};
                }

                // Create randoms.
                random_integer(tmp, x, rng);
                n1 = &tmp.m_mpz;
                random_integer(tmp, y, rng);
                n2 = &tmp.m_mpz;

                // Promote sometimes, if possible.
                if (sdist(rng) && n1.is_static()) {
                    n1.promote();
                }
                if (sdist(rng) && n2.is_static()) {
                    n2.promote();
                }

                // Create copies.
                n1_copy = n1;
                n2_copy = n2;

                // Self swap first.
                swap(n1, n1);
                REQUIRE(n1 == n1_copy);

                // n1-n2 swap.
                swap(n1, n2);

                REQUIRE(n2 == n1_copy);
                REQUIRE(n1 == n2_copy);

                // Check staticness is preserved.
                REQUIRE(n1_copy.is_static() == n2.is_static());
                REQUIRE(n2_copy.is_static() == n1.is_static());
            };
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

        integer n1, n2;
        REQUIRE(noexcept(swap(n1, n2)));
    }
};

TEST_CASE("swap")
{
    tuple_for_each(sizes{}, swap_tester{});
}
