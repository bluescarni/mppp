// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <array>
#include <cstddef>
#include <initializer_list>
#include <random>
#include <tuple>
#include <type_traits>

#include <gmp.h>

#include <mp++/config.hpp>
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

struct ior_tester {
    template <typename S>
    // NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        detail::mpz_raii m1, m2, m3;
        integer n1, n2, n3;
        mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        bitwise_ior(n1, n2, n3);
        REQUIRE(n1 == integer{&m1.m_mpz});
        REQUIRE(n1 == (n2 | n3));
        detail::mpz_raii tmp1, tmp2;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                random_integer(tmp1, x, rng);
                mpz_set(&m2.m_mpz, &tmp1.m_mpz);
                random_integer(tmp2, y, rng);
                mpz_set(&m3.m_mpz, &tmp2.m_mpz);
                n2 = integer(&tmp1.m_mpz);
                n3 = integer(&tmp2.m_mpz);
                if (sdist(rng)) {
                    mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                if (sdist(rng)) {
                    mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                if (n3.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3.promote();
                }
                mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                bitwise_ior(n1, n2, n3);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (n2 | n3));
                bitwise_ior(n1, n3, n2);
                mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                // Overlapping arguments.
                integer old_n1{n1};
                mpz_ior(&m1.m_mpz, &m1.m_mpz, &m3.m_mpz);
                bitwise_ior(n1, n1, n3);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (old_n1 | n3));
                old_n1 |= n3;
                REQUIRE(n1 == old_n1);
                bitwise_ior(n1, n3, n2);
                mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                integer old_n2{n2};
                mpz_ior(&m2.m_mpz, &m1.m_mpz, &m2.m_mpz);
                bitwise_ior(n2, n1, n2);
                REQUIRE(n2 == integer{&m2.m_mpz});
                REQUIRE(n2 == (n1 | old_n2));
                bitwise_ior(n1, n3, n2);
                mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                old_n1 = n1;
                mpz_ior(&m1.m_mpz, &m1.m_mpz, &m1.m_mpz);
                bitwise_ior(n1, n1, n1);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (old_n1 | old_n1));
                bitwise_ior(n1, n3, n2);
                mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                old_n1 |= *&old_n1;
                REQUIRE(n1 == old_n1);
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

        // Size-specific testing.
        if (S::value == 1u) {
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = 0;
            mpz_set_si(&m2.m_mpz, 0);
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
        }

        if (S::value == 2u) {
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = 0;
            mpz_set_si(&m2.m_mpz, 0);
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // Fill the high limbs too.
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            n2 <<= GMP_NUMB_BITS;
            n3 <<= GMP_NUMB_BITS;
            n2 += GMP_NUMB_MAX;
            n3 += GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 >>= GMP_NUMB_BITS;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
            std::array<::mp_limb_t, 2> arr1;
            arr1 = {{0u, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
        }

        if (S::value >= 3u) {
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = 0;
            mpz_set_si(&m2.m_mpz, 0);
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // Fill the high limbs too.
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            n2 <<= GMP_NUMB_BITS;
            n3 <<= GMP_NUMB_BITS;
            n2 += GMP_NUMB_MAX;
            n3 += GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 >>= GMP_NUMB_BITS;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // 3 limbs.
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
            std::array<::mp_limb_t, 3> arr1;
            arr1 = {{GMP_NUMB_MAX, GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            n3 = n2;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{GMP_NUMB_MAX, 0u, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{0u, GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{0u, 0u, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{0u, 0u, ::mp_limb_t{1} << (GMP_NUMB_BITS - 1)}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_ior(n1, n2, n3);
            mpz_ior(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_ior(n1, n3, n2);
            mpz_ior(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
        }
        // A couple of tests for the operators.
        REQUIRE((integer{} | 0) == 0);
        REQUIRE((0 | integer{}) == 0);
        REQUIRE((integer{25} | -5) == -5);
        REQUIRE((-5ll | integer{25}) == -5);
        REQUIRE((std::is_same<integer, decltype(-5ll | integer{25})>::value));
        n1 = 25;
        n1 |= -5;
        REQUIRE(n1 == -5);
        int tmp_int = 25;
        tmp_int |= integer{-5};
        REQUIRE(tmp_int == -5);
        REQUIRE((std::is_same<integer &, decltype(n1 |= -5)>::value));
        REQUIRE((std::is_same<int &, decltype(tmp_int |= integer{-5})>::value));
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((integer{25} | __int128_t{-5}) == -5);
        REQUIRE((__int128_t{25} | integer{-5}) == -5);
        REQUIRE((integer{} | __uint128_t{0}) == 0);
        REQUIRE((__uint128_t{0} | integer{}) == 0);
        n1 = 25;
        n1 |= __int128_t{-5};
        REQUIRE(n1 == -5);
        n1 |= __uint128_t{6};
        REQUIRE(n1 == -1);
        __int128_t n128{25};
        n128 |= integer{-5};
        REQUIRE(n128 == -5);
        __uint128_t un128{25};
        un128 |= 5;
        REQUIRE(un128 == 29);
#endif
    }
};

TEST_CASE("integer ior")
{
    tuple_for_each(sizes{}, ior_tester{});
}

struct not_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        detail::mpz_raii m1, m2;
        integer n1, n2;
        mpz_com(&m1.m_mpz, &m2.m_mpz);
        bitwise_not(n1, n2);
        REQUIRE(n1 == integer{&m1.m_mpz});
        REQUIRE(n1 == ~n2);
        // Try 1/-1;
        n2 = 1;
        mpz_set(&m2.m_mpz, n2.get_mpz_view());
        mpz_com(&m1.m_mpz, &m2.m_mpz);
        bitwise_not(n1, n2);
        REQUIRE(n1 == integer{&m1.m_mpz});
        REQUIRE(n1 == ~n2);
        n2 = -1;
        mpz_set(&m2.m_mpz, n2.get_mpz_view());
        mpz_com(&m1.m_mpz, &m2.m_mpz);
        bitwise_not(n1, n2);
        REQUIRE(n1 == integer{&m1.m_mpz});
        REQUIRE(n1 == ~n2);
        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x number of limbs.
        auto random_xy = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                random_integer(tmp, x, rng);
                mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer{&tmp.m_mpz};
                if (sdist(rng)) {
                    mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                mpz_com(&m1.m_mpz, &m2.m_mpz);
                bitwise_not(n1, n2);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == ~n2);
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);

        // Size-specific testing.
        if (S::value == 1u) {
            n2 = integer{GMP_NUMB_MAX};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            n2 = -integer{GMP_NUMB_MAX};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
        }

        if (S::value == 2u) {
            n2 = integer{GMP_NUMB_MAX};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            n2 = -integer{GMP_NUMB_MAX};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
            std::array<::mp_limb_t, 2> tmp_arr;
            tmp_arr = {{GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr = {{GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = -integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr = {{0, GMP_NUMB_MAX}};
            n2 = integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr = {{0, GMP_NUMB_MAX}};
            n2 = -integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr = {{0, 1}};
            n2 = integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr = {{0, 1}};
            n2 = -integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr = {{GMP_NUMB_MAX, 1}};
            n2 = integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr = {{GMP_NUMB_MAX, 1}};
            n2 = -integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
        }

        if (S::value == 3u) {
            n2 = integer{GMP_NUMB_MAX};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            n2 = -integer{GMP_NUMB_MAX};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
            std::array<::mp_limb_t, 2> tmp_arr;
            tmp_arr = {{GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr = {{GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = -integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr = {{0, GMP_NUMB_MAX}};
            n2 = integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr = {{0, GMP_NUMB_MAX}};
            n2 = -integer{tmp_arr.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
            std::array<::mp_limb_t, 3> tmp_arr2;
            tmp_arr2 = {{GMP_NUMB_MAX, GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = integer{tmp_arr2.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{GMP_NUMB_MAX, GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = -integer{tmp_arr2.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{0, GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = integer{tmp_arr2.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{0, GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = -integer{tmp_arr2.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{0, 0, GMP_NUMB_MAX}};
            n2 = integer{tmp_arr2.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{0, 0, GMP_NUMB_MAX}};
            n2 = -integer{tmp_arr2.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{GMP_NUMB_MAX, GMP_NUMB_MAX, 1}};
            n2 = integer{tmp_arr2.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{GMP_NUMB_MAX, GMP_NUMB_MAX, 1}};
            n2 = -integer{tmp_arr2.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{GMP_NUMB_MAX, 1, 0}};
            n2 = integer{tmp_arr2.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{GMP_NUMB_MAX, 1, 0}};
            n2 = -integer{tmp_arr2.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{0, 1, 0}};
            n2 = integer{tmp_arr2.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{0, 1, 0}};
            n2 = -integer{tmp_arr2.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{GMP_NUMB_MAX, 0, 1}};
            n2 = integer{tmp_arr2.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
            tmp_arr2 = {{GMP_NUMB_MAX, 0, 1}};
            n2 = -integer{tmp_arr2.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_com(&m1.m_mpz, &m2.m_mpz);
            bitwise_not(n1, n2);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == ~n2);
        }
    }
};

TEST_CASE("integer not")
{
    tuple_for_each(sizes{}, not_tester{});
}

struct and_tester {
    template <typename S>
    // NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        detail::mpz_raii m1, m2, m3;
        integer n1, n2, n3;
        mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        bitwise_and(n1, n2, n3);
        REQUIRE(n1 == integer{&m1.m_mpz});
        REQUIRE(n1 == (n2 & n3));
        detail::mpz_raii tmp1, tmp2;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                random_integer(tmp1, x, rng);
                mpz_set(&m2.m_mpz, &tmp1.m_mpz);
                random_integer(tmp2, y, rng);
                mpz_set(&m3.m_mpz, &tmp2.m_mpz);
                n2 = integer(&tmp1.m_mpz);
                n3 = integer(&tmp2.m_mpz);
                if (sdist(rng)) {
                    mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                if (sdist(rng)) {
                    mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                if (n3.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3.promote();
                }
                mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                bitwise_and(n1, n2, n3);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (n2 & n3));
                bitwise_and(n1, n3, n2);
                mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                // Overlapping arguments.
                integer old_n1{n1};
                mpz_and(&m1.m_mpz, &m1.m_mpz, &m3.m_mpz);
                bitwise_and(n1, n1, n3);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (old_n1 & n3));
                old_n1 &= n3;
                REQUIRE(n1 == old_n1);
                bitwise_and(n1, n3, n2);
                mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                integer old_n2{n2};
                mpz_and(&m2.m_mpz, &m1.m_mpz, &m2.m_mpz);
                bitwise_and(n2, n1, n2);
                REQUIRE(n2 == integer{&m2.m_mpz});
                REQUIRE(n2 == (n1 & old_n2));
                bitwise_and(n1, n3, n2);
                mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                old_n1 = n1;
                mpz_and(&m1.m_mpz, &m1.m_mpz, &m1.m_mpz);
                bitwise_and(n1, n1, n1);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (old_n1 & old_n1));
                bitwise_and(n1, n3, n2);
                mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                old_n1 &= *&old_n1;
                REQUIRE(n1 == old_n1);
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

        // Size-specific testing.
        if (S::value == 1u) {
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = 0;
            mpz_set_si(&m2.m_mpz, 0);
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 1);
            n3 = -3;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 3);
            n3 = -7;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
        }

        if (S::value == 2u) {
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = 0;
            mpz_set_si(&m2.m_mpz, 0);
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // Fill the high limbs too.
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            n2 <<= GMP_NUMB_BITS;
            n3 <<= GMP_NUMB_BITS;
            n2 += GMP_NUMB_MAX;
            n3 += GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 >>= GMP_NUMB_BITS;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
            std::array<::mp_limb_t, 2> arr1;
            arr1 = {{0u, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 1) - (integer{GMP_NUMB_MAX} << GMP_NUMB_BITS);
            n3 = -3;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 3) - (integer{GMP_NUMB_MAX} << GMP_NUMB_BITS);
            n3 = -7;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
        }

        if (S::value >= 3u) {
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = 0;
            mpz_set_si(&m2.m_mpz, 0);
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // Fill the high limbs too.
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            n2 <<= GMP_NUMB_BITS;
            n3 <<= GMP_NUMB_BITS;
            n2 += GMP_NUMB_MAX;
            n3 += GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 >>= GMP_NUMB_BITS;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // 3 limbs.
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
            std::array<::mp_limb_t, 3> arr1;
            arr1 = {{GMP_NUMB_MAX, GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            n3 = n2;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{GMP_NUMB_MAX, 0u, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{0u, GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{0u, 0u, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{0u, 0u, ::mp_limb_t{1} << (GMP_NUMB_BITS - 1)}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_and(n1, n3, n2);
            mpz_and(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 1) - (integer{GMP_NUMB_MAX} << GMP_NUMB_BITS);
            n3 = -3;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 3) - (integer{GMP_NUMB_MAX} << GMP_NUMB_BITS);
            n3 = -7;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_and(n1, n2, n3);
            mpz_and(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
        }
        // A couple of tests for the operators.
        REQUIRE((integer{} & 0) == 0);
        REQUIRE((0 & integer{}) == 0);
        REQUIRE((integer{25} & -6) == 24);
        REQUIRE((-6ll & integer{25}) == 24);
        REQUIRE((std::is_same<integer, decltype(-5ll & integer{25})>::value));
        n1 = 25;
        n1 &= -6;
        REQUIRE(n1 == 24);
        int tmp_int = 25;
        tmp_int &= integer{-6};
        REQUIRE(tmp_int == 24);
        REQUIRE((std::is_same<integer &, decltype(n1 &= -5)>::value));
        REQUIRE((std::is_same<int &, decltype(tmp_int &= integer{-5})>::value));
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((integer{25} & __int128_t{-5}) == 25);
        REQUIRE((__int128_t{25} & integer{-5}) == 25);
        REQUIRE((integer{} & __uint128_t{0}) == 0);
        REQUIRE((__uint128_t{0} & integer{}) == 0);
        n1 = 25;
        n1 &= __int128_t{-5};
        REQUIRE(n1 == 25);
        n1 &= __uint128_t{6};
        REQUIRE(n1 == 0);
        __int128_t n128{25};
        n128 &= integer{-5};
        REQUIRE(n128 == 25);
        __uint128_t un128{25};
        un128 &= 5;
        REQUIRE(un128 == 1);
#endif
    }
};

TEST_CASE("integer and")
{
    tuple_for_each(sizes{}, and_tester{});
}

struct xor_tester {
    template <typename S>
    // NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        detail::mpz_raii m1, m2, m3;
        integer n1, n2, n3;
        mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
        bitwise_xor(n1, n2, n3);
        REQUIRE(n1 == integer{&m1.m_mpz});
        REQUIRE(n1 == (n2 ^ n3));
        detail::mpz_raii tmp1, tmp2;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                random_integer(tmp1, x, rng);
                mpz_set(&m2.m_mpz, &tmp1.m_mpz);
                random_integer(tmp2, y, rng);
                mpz_set(&m3.m_mpz, &tmp2.m_mpz);
                n2 = integer(&tmp1.m_mpz);
                n3 = integer(&tmp2.m_mpz);
                if (sdist(rng)) {
                    mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                if (sdist(rng)) {
                    mpz_neg(&m3.m_mpz, &m3.m_mpz);
                    n3.neg();
                }
                if (n3.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n3.promote();
                }
                REQUIRE((n2 ^ n2).is_zero());
                REQUIRE((n3 ^ n3).is_zero());
                mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
                bitwise_xor(n1, n2, n3);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (n2 ^ n3));
                bitwise_xor(n1, n3, n2);
                mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                // Overlapping arguments.
                integer old_n1{n1};
                mpz_xor(&m1.m_mpz, &m1.m_mpz, &m3.m_mpz);
                bitwise_xor(n1, n1, n3);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (old_n1 ^ n3));
                old_n1 ^= n3;
                REQUIRE(n1 == old_n1);
                bitwise_xor(n1, n3, n2);
                mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                integer old_n2{n2};
                mpz_xor(&m2.m_mpz, &m1.m_mpz, &m2.m_mpz);
                bitwise_xor(n2, n1, n2);
                REQUIRE(n2 == integer{&m2.m_mpz});
                REQUIRE(n2 == (n1 ^ old_n2));
                bitwise_xor(n1, n3, n2);
                mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                old_n1 = n1;
                mpz_xor(&m1.m_mpz, &m1.m_mpz, &m1.m_mpz);
                bitwise_xor(n1, n1, n1);
                REQUIRE(n1 == integer{&m1.m_mpz});
                REQUIRE(n1 == (old_n1 ^ old_n1));
                bitwise_xor(n1, n3, n2);
                mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
                REQUIRE(n1 == integer{&m1.m_mpz});
                old_n1 ^= *&old_n1;
                REQUIRE(n1 == old_n1);
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

        // Size-specific testing.
        if (S::value == 1u) {
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = 0;
            mpz_set_si(&m2.m_mpz, 0);
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 1);
            n3 = -3;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 3);
            n3 = -7;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{1} << (GMP_NUMB_BITS - 1));
            n3 = -n2;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == (n3 ^ n2));
            n2 = (~::mp_limb_t(2338848) + 1u) & GMP_NUMB_MASK;
            n3 = -2338848ll;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == (n3 ^ n2));
        }

        if (S::value == 2u) {
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = 0;
            mpz_set_si(&m2.m_mpz, 0);
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // Fill the high limbs too.
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            n2 <<= GMP_NUMB_BITS;
            n3 <<= GMP_NUMB_BITS;
            n2 += GMP_NUMB_MAX;
            n3 += GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 >>= GMP_NUMB_BITS;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
            std::array<::mp_limb_t, 2> arr1;
            arr1 = {{0u, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 2};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 1) - (integer{GMP_NUMB_MAX} << GMP_NUMB_BITS);
            n3 = -3;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 3) - (integer{GMP_NUMB_MAX} << GMP_NUMB_BITS);
            n3 = -7;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{1} << (GMP_NUMB_BITS * 2 - 1));
            n3 = -n2;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == (n3 ^ n2));
            n2 = (~::mp_limb_t(2338848) + (integer{~::mp_limb_t(2338848)} << GMP_NUMB_BITS) + 1u);
            n3 = -(::mp_limb_t(2338848) + (integer{::mp_limb_t(2338848)} << GMP_NUMB_BITS));
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == (n3 ^ n2));
        }

        if (S::value >= 3u) {
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = 0;
            mpz_set_si(&m2.m_mpz, 0);
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // Fill the high limbs too.
            n2 = GMP_NUMB_MAX;
            n3 = GMP_NUMB_MAX;
            n2 <<= GMP_NUMB_BITS;
            n3 <<= GMP_NUMB_BITS;
            n2 += GMP_NUMB_MAX;
            n3 += GMP_NUMB_MAX;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 >>= GMP_NUMB_BITS;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            // 3 limbs.
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
            std::array<::mp_limb_t, 3> arr1;
            arr1 = {{GMP_NUMB_MAX, GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            n3 = n2;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{GMP_NUMB_MAX, 0u, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{0u, GMP_NUMB_MAX, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{0u, 0u, GMP_NUMB_MAX}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            arr1 = {{0u, 0u, ::mp_limb_t{1} << (GMP_NUMB_BITS - 1)}};
            n2 = integer{arr1.data(), 3};
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2.neg();
            mpz_neg(&m2.m_mpz, &m2.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n3.neg();
            mpz_neg(&m3.m_mpz, &m3.m_mpz);
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            bitwise_xor(n1, n3, n2);
            mpz_xor(&m1.m_mpz, &m3.m_mpz, &m2.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 1) - (integer{GMP_NUMB_MAX} << GMP_NUMB_BITS);
            n3 = -3;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{GMP_NUMB_MAX} - 3) - (integer{GMP_NUMB_MAX} << GMP_NUMB_BITS);
            n3 = -7;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            n2 = -(integer{1} << (GMP_NUMB_BITS * S::value - 1));
            n3 = -n2;
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == (n3 ^ n2));
            n2 = (~::mp_limb_t(2338845) + (integer{~::mp_limb_t(2338848)} << GMP_NUMB_BITS)
                  + (integer{~::mp_limb_t(23)} << (GMP_NUMB_BITS * 2)) + 1u);
            n3 = -(::mp_limb_t(2338845) + (integer{::mp_limb_t(2338848)} << GMP_NUMB_BITS)
                   + (integer{::mp_limb_t(23)} << (GMP_NUMB_BITS * 2)));
            mpz_set(&m2.m_mpz, n2.get_mpz_view());
            mpz_set(&m3.m_mpz, n3.get_mpz_view());
            bitwise_xor(n1, n2, n3);
            mpz_xor(&m1.m_mpz, &m2.m_mpz, &m3.m_mpz);
            REQUIRE(n1 == integer{&m1.m_mpz});
            REQUIRE(n1 == (n3 ^ n2));
        }
        // A couple of tests for the operators.
        REQUIRE((integer{} ^ 0) == 0);
        REQUIRE((0 ^ integer{}) == 0);
        REQUIRE((integer{25} ^ -6) == -29);
        REQUIRE((-6ll ^ integer{25}) == -29);
        REQUIRE((std::is_same<integer, decltype(-5ll ^ integer{25})>::value));
        n1 = 25;
        n1 ^= -6;
        REQUIRE(n1 == -29);
        int tmp_int = 25;
        tmp_int ^= integer{-6};
        REQUIRE(tmp_int == -29);
        REQUIRE((std::is_same<integer &, decltype(n1 ^= -5)>::value));
        REQUIRE((std::is_same<int &, decltype(tmp_int ^= integer{-5})>::value));
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((integer{25} ^ __int128_t{-5}) == -30);
        REQUIRE((__int128_t{25} ^ integer{-5}) == -30);
        REQUIRE((integer{} ^ __uint128_t{0}) == 0);
        REQUIRE((__uint128_t{0} ^ integer{}) == 0);
        n1 = 25;
        n1 ^= __int128_t{-5};
        REQUIRE(n1 == -30);
        n1 ^= __uint128_t{6};
        REQUIRE(n1 == -28);
        __int128_t n128{25};
        n128 ^= integer{-5};
        REQUIRE(n128 == -30);
        __uint128_t un128{25};
        un128 ^= 5;
        REQUIRE(un128 == 28);
#endif
    }
};

TEST_CASE("integer xor")
{
    tuple_for_each(sizes{}, xor_tester{});
}
