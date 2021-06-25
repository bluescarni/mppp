// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#if defined(_MSC_VER)

// Disable some warnings for MSVC. These arise only in release mode apparently.
#pragma warning(push)
#pragma warning(disable : 4723)

#endif

#include <cstddef>
#include <random>
#include <tuple>
#include <type_traits>

#include <mp++/detail/gmp.hpp>
#include <mp++/integer.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

static const int ntries = 1000;

// NOLINTNEXTLINE(cert-err58-cpp, cert-msc32-c, cert-msc51-cpp, cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937 rng;

struct sqr_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer ret;

        // A few simple tests.
        sqr(ret, integer{0});
        REQUIRE(ret == 0);

        sqr(ret, integer{1});
        REQUIRE(ret == 1);

        sqr(ret, integer{0});
        REQUIRE(ret == 0);

        sqr(ret, integer{2});
        REQUIRE(ret == 4);

        sqr(ret, integer{-1});
        REQUIRE(ret == 1);

        sqr(ret, integer{-2});
        REQUIRE(ret == 4);

        sqr(ret, integer{20883l});
        REQUIRE(ret == 436099689l);

        sqr(ret, integer{-8070l});
        REQUIRE(ret == 65124900l);

        // Random testing.
        integer n1, n2;
        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operand with x number of limbs.
        auto random_x = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                n2 = &tmp.m_mpz;
                if (sdist(rng)) {
                    n2.neg();
                }
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                sqr(n1, n2);
                REQUIRE(n1 == n2 * n2);

                // The unary variant.
                REQUIRE(sqr(n2) == n1);

                // In-place variant.
                auto n2_old(n2);
                sqr(n2, n2);
                REQUIRE(n2 == n2_old * n2_old);

                // The member function.
                n2 = n2_old;
                REQUIRE(n2.sqr() == n2_old * n2_old);
            }
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

TEST_CASE("sqr")
{
    tuple_for_each(sizes{}, sqr_tester{});
}

#if defined(_MSC_VER)

#pragma warning(pop)

#endif
