// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
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

struct bin_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        detail::mpz_raii m1, m2;
        integer n1, n2;
        mpz_bin_ui(&m1.m_mpz, &m2.m_mpz, 0u);
        REQUIRE(&bin_ui(n1, n2, 0) == &n1);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(bin_ui(n2, 0)) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        std::uniform_int_distribution<int> sdist(0, 1);
        std::uniform_int_distribution<int> ndist(-20, 20);
        std::uniform_int_distribution<unsigned> kdist(0, 20);
        for (int i = 0; i < ntries; ++i) {
            // NOLINTNEXTLINE(misc-redundant-expression)
            if (sdist(rng) && sdist(rng) && sdist(rng)) {
                // Reset rop every once in a while.
                n1 = integer{};
            }
            const auto n = ndist(rng);
            const auto k = kdist(rng);
            mpz_set_si(&m2.m_mpz, n);
            n2 = integer(n);
            if (n1.is_static() && sdist(rng)) {
                // Promote sometimes, if possible.
                n1.promote();
            }
            if (n2.is_static() && sdist(rng)) {
                // Promote sometimes, if possible.
                n2.promote();
            }
            mpz_bin_ui(&m1.m_mpz, &m2.m_mpz, k);
            bin_ui(n1, n2, k);
            REQUIRE((lex_cast(n1) == lex_cast(m1)));
            REQUIRE((lex_cast(bin_ui(n2, k)) == lex_cast(m1)));
        }
    }
};

TEST_CASE("bin")
{
    tuple_for_each(sizes{}, bin_tester{});
    // Let's test the are_integer_integral_op_types type trait here.
    REQUIRE((are_integer_integral_op_types<integer<1>, integer<1>>::value));
    REQUIRE((are_integer_integral_op_types<integer<2>, integer<2>>::value));
    REQUIRE((are_integer_integral_op_types<integer<1>, int>::value));
    REQUIRE((are_integer_integral_op_types<char, integer<1>>::value));
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE((are_integer_integral_op_types<__int128_t, integer<1>>::value));
    REQUIRE((are_integer_integral_op_types<__uint128_t, integer<1>>::value));
    REQUIRE((are_integer_integral_op_types<integer<1>, __int128_t>::value));
    REQUIRE((are_integer_integral_op_types<integer<1>, __uint128_t>::value));
#endif
    REQUIRE((!are_integer_integral_op_types<integer<1>, integer<1> &>::value));
    REQUIRE((!are_integer_integral_op_types<const integer<1>, integer<1>>::value));
    REQUIRE((!are_integer_integral_op_types<int, int>::value));
    REQUIRE((!are_integer_integral_op_types<int, void>::value));
    REQUIRE((!are_integer_integral_op_types<void, int>::value));
    REQUIRE((!are_integer_integral_op_types<void, void>::value));
    REQUIRE((!are_integer_integral_op_types<integer<1>, integer<2>>::value));
    REQUIRE((!are_integer_integral_op_types<integer<2>, integer<1>>::value));
}

struct binomial_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = integer<T::value>;
        int_type n;
        REQUIRE(binomial(n, 0) == 1);
        REQUIRE(binomial(n, 1) == 0);
        REQUIRE(binomial(n, false) == 1);
        REQUIRE(binomial(n, true) == 0);
        n = 1;
        REQUIRE(binomial(n, 1) == 1);
        n = 5;
        REQUIRE(binomial(n, 3) == 10);
        n = -5;
        REQUIRE(binomial(n, int_type(4)) == 70);
        // Random tests.
        std::uniform_int_distribution<int> ud(-1000, 1000);
        std::uniform_int_distribution<int> promote_dist(0, 1);
        detail::mpz_raii m;
        for (int i = 0; i < ntries; ++i) {
            auto tmp1 = ud(rng), tmp2 = ud(rng);
            n = tmp1;
            if (promote_dist(rng) && n.is_static()) {
                n.promote();
            }
            if (tmp2 < 0) {
                // NOTE: we cannot check this case with GMP, defer to some tests below.
                CHECK_NOTHROW(binomial(n, tmp2));
                continue;
            }
            mpz_set_si(&m.m_mpz, static_cast<long>(tmp1));
            mpz_bin_ui(&m.m_mpz, &m.m_mpz, static_cast<unsigned long>(tmp2));
            REQUIRE(binomial(n, tmp2).to_string() == lex_cast(m));
        }
        REQUIRE_THROWS_AS(binomial(n, std::numeric_limits<unsigned long>::max() + int_type(1)), std::overflow_error);
        REQUIRE_THROWS_AS(binomial(-int_type{std::numeric_limits<unsigned long>::max()} + 1,
                                   -2 * int_type{std::numeric_limits<unsigned long>::max()}),
                          std::overflow_error);
        // Negative k.
        REQUIRE(binomial(int_type{-3}, -4) == -3);
        REQUIRE(binomial(int_type{-3}, -10) == -36);
        REQUIRE(binomial(int_type{-3}, -1) == 0);
        REQUIRE(binomial(int_type{3}, -1) == 0);
        REQUIRE(binomial(int_type{10}, -1) == 0);
        REQUIRE(binomial(int_type{-3}, -3) == 1);
        REQUIRE(binomial(int_type{-1}, -1) == 1);
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(binomial(int_type{-3}, __int128_t{-4}) == -3);
        REQUIRE(binomial(__int128_t{-5}, int_type{4}) == 70);
        REQUIRE(binomial(int_type{5}, __uint128_t{3}) == 10);
        REQUIRE(binomial(__uint128_t{1}, int_type{1}) == 1);
#endif
    }
};

TEST_CASE("binomial")
{
    tuple_for_each(sizes{}, binomial_tester{});
}
