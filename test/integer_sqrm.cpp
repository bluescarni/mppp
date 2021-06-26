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
#include <string>
#include <tuple>
#include <type_traits>

#include <mp++/detail/gmp.hpp>
#include <mp++/exceptions.hpp>
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

struct sqrm_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer ret;

        // A few simple tests.
        sqrm(ret, integer{0}, integer{1});
        REQUIRE(ret == 0);
        REQUIRE(sqrm(integer{0}, integer{1}) == 0);

        sqrm(ret, integer{0}, integer{-1});
        REQUIRE(ret == 0);
        REQUIRE(sqrm(integer{0}, integer{-1}) == 0);

        sqrm(ret, integer{1}, integer{1});
        REQUIRE(ret == 0);
        REQUIRE(sqrm(integer{1}, integer{1}) == 0);

        sqrm(ret, integer{1}, integer{-1});
        REQUIRE(ret == 0);
        REQUIRE(sqrm(integer{1}, integer{-1}) == 0);

        sqrm(ret, integer{-1}, integer{-1});
        REQUIRE(ret == 0);
        REQUIRE(sqrm(integer{-1}, integer{-1}) == 0);

        sqrm(ret, integer{-1}, integer{1});
        REQUIRE(ret == 0);
        REQUIRE(sqrm(integer{-1}, integer{1}) == 0);

        sqrm(ret, integer{2}, integer{3});
        REQUIRE(ret == 1);
        REQUIRE(sqrm(integer{2}, integer{3}) == 1);

        sqrm(ret, integer{2}, integer{-3});
        REQUIRE(ret == 1);
        REQUIRE(sqrm(integer{2}, integer{-3}) == 1);

        sqrm(ret, integer{-2}, integer{-3});
        REQUIRE(ret == 1);
        REQUIRE(sqrm(integer{-2}, integer{-3}) == 1);

        sqrm(ret, integer{-2}, integer{3});
        REQUIRE(ret == 1);
        REQUIRE(sqrm(integer{-2}, integer{3}) == 1);

        sqrm(ret, integer{2}, integer{7});
        REQUIRE(ret == 4);
        REQUIRE(sqrm(integer{2}, integer{7}) == 4);

        sqrm(ret, integer{2}, integer{-7});
        REQUIRE(ret == 4);
        REQUIRE(sqrm(integer{2}, integer{-7}) == 4);

        sqrm(ret, integer{-2}, integer{-7});
        REQUIRE(ret == 4);
        REQUIRE(sqrm(integer{-2}, integer{-7}) == 4);

        sqrm(ret, integer{-2}, integer{7});
        REQUIRE(ret == 4);
        REQUIRE(sqrm(integer{-2}, integer{7}) == 4);

        REQUIRE_THROWS_PREDICATE(
            sqrm(ret, integer{-2}, integer{0}), zero_division_error,
            [](const zero_division_error &ex) { return std::string(ex.what()) == "Integer division by zero"; });

        REQUIRE_THROWS_PREDICATE(sqrm(integer{-2}, integer{0}), zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });

        // Random testing.
        integer n1, n2, n3;
        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
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

                random_integer(tmp, y, rng);
                n3 = &tmp.m_mpz;
                if (sdist(rng)) {
                    n3.neg();
                }
                if (n3.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3.promote();
                }
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                sqrm(n1, n2, n3);
                REQUIRE(n1 == (n2 * n2) % n3);

                // The binary variant.
                REQUIRE(sqrm(n2, n3) == n1);

                // In-place variants.
                auto n2_old(n2);
                sqrm(n2, n2, n3);
                REQUIRE(n2 == (n2_old * n2_old) % n3);
                n2 = n2_old;

                auto n3_old(n3);
                sqrm(n3, n2, n3);
                REQUIRE(n3 == (n2 * n2) % n3_old);
                n3 = n3_old;

                if (n2 != 0) {
                    sqrm(n1, n2, n2);
                    REQUIRE(n1 == (n2 * n2) % n2);

                    sqrm(n2, n2, n2);
                    REQUIRE(n1 == (n2_old * n2_old) % n2_old);
                }
            }
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

TEST_CASE("sqrm")
{
    tuple_for_each(sizes{}, sqrm_tester{});
}

#if defined(_MSC_VER)

#pragma warning(pop)

#endif
