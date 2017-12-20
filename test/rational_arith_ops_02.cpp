// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cmath>
#include <cstddef>
#include <gmp.h>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <mp++/config.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

template <typename T, typename U>
using divvv_t = decltype(std::declval<const T &>() / std::declval<const U &>());

template <typename T, typename U>
using inplace_divvv_t = decltype(std::declval<T &>() /= std::declval<const U &>());

template <typename T, typename U>
using is_divisible = is_detected<divvv_t, T, U>;

template <typename T, typename U>
using is_divisible_inplace = is_detected<inplace_divvv_t, T, U>;

struct div_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = typename rational::int_t;
        // Binary div.
        rational n1{1, 2}, n2{2, -3};
        REQUIRE((lex_cast(n1 / n2) == "-3/4"));
        REQUIRE((std::is_same<rational, decltype(n1 / n2)>::value));
        REQUIRE_THROWS_PREDICATE(n1 / rational{0}, zero_division_error, [](const zero_division_error &zde) {
            return std::string(zde.what()) == "Zero divisor in rational division";
        });
        REQUIRE((lex_cast(rational{3} / integer{4}) == "3/4"));
        REQUIRE_THROWS_PREDICATE(n1 / integer{0}, zero_division_error, [](const zero_division_error &zde) {
            return std::string(zde.what()) == "Zero divisor in rational division";
        });
        REQUIRE((lex_cast(rational{16} / integer{-4}) == "-4"));
        REQUIRE((lex_cast(integer{16} / rational{-4}) == "-4"));
        REQUIRE((lex_cast(rational{16, 11} / integer{-4}) == "-4/11"));
        REQUIRE((lex_cast(integer{16} / rational{-4, 3}) == "-12"));
        REQUIRE((lex_cast(integer{4} / rational{3}) == "4/3"));
        REQUIRE_THROWS_PREDICATE(integer{4} / rational{0}, zero_division_error, [](const zero_division_error &zde) {
            return std::string(zde.what()) == "Zero divisor in rational division";
        });
        REQUIRE((std::is_same<rational, decltype(integer{4} / rational{3})>::value));
        REQUIRE((std::is_same<rational, decltype(rational{3} / integer{4})>::value));
        REQUIRE((lex_cast(rational{-3, 2} / integer{4}) == "-3/8"));
        REQUIRE((lex_cast(integer{4} / rational{-3, 2}) == "-8/3"));
        REQUIRE((lex_cast(rational{3} / 4) == "3/4"));
        REQUIRE((lex_cast(rational{3} / wchar_t{4}) == "3/4"));
        REQUIRE((lex_cast(wchar_t{3} / rational{4}) == "3/4"));
        REQUIRE_THROWS_PREDICATE(rational{3} / 0, zero_division_error, [](const zero_division_error &zde) {
            return std::string(zde.what()) == "Zero divisor in rational division";
        });
        REQUIRE((lex_cast(4ul / rational{3}) == "4/3"));
        REQUIRE_THROWS_PREDICATE(4ul / rational{}, zero_division_error, [](const zero_division_error &zde) {
            return std::string(zde.what()) == "Zero divisor in rational division";
        });
        REQUIRE((std::is_same<rational, decltype(3 / rational{3})>::value));
        REQUIRE((std::is_same<rational, decltype(rational{3} / 3)>::value));
        REQUIRE((lex_cast(rational{-3, 2} / static_cast<signed char>(4)) == "-3/8"));
        REQUIRE((lex_cast(4ll / rational{-3, 2}) == "-8/3"));
        REQUIRE((rational{3} / 4.f == 3.f / 4));
        REQUIRE((4.f / rational{3} == 4.f / 3));
        REQUIRE((rational{3} / 4. == 3. / 4));
        REQUIRE((std::abs(4. / rational{3} - 4. / 3) < 1E-8));
        REQUIRE((std::is_same<double, decltype(integer{4} / 3.)>::value));
        REQUIRE((std::is_same<float, decltype(3.f / integer{4})>::value));
        if (std::numeric_limits<double>::is_iec559) {
            REQUIRE((rational{3} / 0. == std::numeric_limits<double>::infinity()));
            REQUIRE((-1. / rational{} == -std::numeric_limits<double>::infinity()));
        }
#if defined(MPPP_WITH_MPFR)
        REQUIRE((rational{3} / 4.l == 3 / 4.l));
        REQUIRE((std::abs(4.l / rational{3} - 4.l / 3) < 1E-8));
        REQUIRE((std::is_same<long double, decltype(integer{4} / 3.l)>::value));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((rational{3} / __int128_t{4} == rational{3, 4}));
        REQUIRE((__int128_t{4} / rational{3} == rational{4, 3}));
        REQUIRE((rational{3} / __uint128_t{4} == rational{3, 4}));
        REQUIRE((__uint128_t{4} / rational{3} == rational{4, 3}));
#endif
        REQUIRE((!is_divisible<rational, std::string>::value));
        REQUIRE((!is_divisible<std::string, rational>::value));

        // In-place div.
        rational retval{1, 2};
        retval /= rational{-2, 3};
        REQUIRE((std::is_same<rational &, decltype(retval /= rational{-2, 3})>::value));
        REQUIRE((lex_cast(retval) == "-3/4"));
        retval /= integer{2};
        REQUIRE((std::is_same<rational &, decltype(retval /= integer{1})>::value));
        REQUIRE((lex_cast(retval) == "-3/8"));
        retval /= integer{-3};
        REQUIRE((lex_cast(retval) == "1/8"));
        retval /= integer{-5};
        REQUIRE((lex_cast(retval) == "-1/40"));
        REQUIRE_THROWS_PREDICATE(retval /= integer{0}, zero_division_error, [](const zero_division_error &zde) {
            return std::string(zde.what()) == "Zero divisor in rational division";
        });
        retval *= 80;
        retval /= 2;
        REQUIRE((lex_cast(retval) == "-1"));
        retval /= integer{-3};
        REQUIRE((lex_cast(retval) == "1/3"));
        retval = 5;
        retval /= integer{-1};
        REQUIRE((lex_cast(retval) == "-5"));
        retval = "1/2";
        retval /= 3;
        REQUIRE((std::is_same<rational &, decltype(retval /= 3)>::value));
        REQUIRE((lex_cast(retval) == "1/6"));
        retval /= 4ull;
        REQUIRE((lex_cast(retval) == "1/24"));
        retval /= static_cast<short>(-1);
        REQUIRE((lex_cast(retval) == "-1/24"));
        retval = 12;
        retval /= 2.f;
        REQUIRE((std::is_same<rational &, decltype(retval /= 1.)>::value));
        REQUIRE((lex_cast(retval) == "6"));
        retval /= 2.;
        REQUIRE((lex_cast(retval) == "3"));
#if defined(MPPP_WITH_MPFR)
        retval /= -1.l;
        REQUIRE((lex_cast(retval) == "-3"));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        retval = 1;
        retval /= __int128_t{-5};
        REQUIRE(retval == rational{-1, 5});
        retval /= __uint128_t{3};
        REQUIRE(retval == rational{1} / -15);
#endif
        retval = -3;

        // Interop on the left.
        {
            integer n{5};
            n /= rational{-4, 3};
            REQUIRE((std::is_same<integer &, decltype(n /= rational{-4})>::value));
            REQUIRE((lex_cast(n) == "-3"));
            n /= rational{-5, 2};
            REQUIRE((lex_cast(n) == "1"));
        }
        {
            int n = 5;
            n /= rational{-4, 3};
            REQUIRE((lex_cast(n) == "-3"));
            REQUIRE((std::is_same<int &, decltype(n /= rational{-4})>::value));
            n /= rational{-5, 2};
            REQUIRE((lex_cast(n) == "1"));
            n = nl_max<int>();
            REQUIRE_THROWS_AS(n /= (rational{1, 2}), std::overflow_error);
            n = nl_min<int>();
            REQUIRE_THROWS_AS(n /= (rational{1, 2}), std::overflow_error);
        }
        {
            if (std::numeric_limits<double>::is_iec559) {
                double x = 5;
                x /= rational{-5, 2};
                REQUIRE((std::is_same<double &, decltype(x /= rational{-4})>::value));
                REQUIRE((std::abs(-2. - x) < 1E-8));
                x /= rational{-5, 2};
                REQUIRE((std::abs(4. / 5 - x) < 1E-8));
                REQUIRE_THROWS_PREDICATE(retval /= 0., std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot construct a rational from the non-finite floating-point value "
                                  + std::to_string(-std::numeric_limits<double>::infinity());
                });
            }
        }
#if defined(MPPP_WITH_MPFR)
        {
            if (std::numeric_limits<long double>::is_iec559) {
                long double x = 5;
                x /= rational{-5, 2};
                REQUIRE((std::abs(-2.l - x) < 1E-8));
                x /= rational{-5, 2};
                REQUIRE((std::abs(4.l / 5 - x) < 1E-8));
            }
        }
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        {
            __int128_t n128 = -6;
            n128 /= rational{-5};
            REQUIRE(n128 == 1);
            __uint128_t un128 = 6;
            un128 /= rational{2};
            REQUIRE(un128 == 3);
        }
#endif
        REQUIRE((!is_divisible_inplace<rational, std::string>::value));
        REQUIRE((!is_divisible_inplace<std::string, rational>::value));

        // In-place div with self.
        retval = "-3/4";
        retval /= retval;
        REQUIRE(retval == rational(1));
    }
};

TEST_CASE("div")
{
    tuple_for_each(sizes{}, div_tester{});
}

struct rel_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = typename rational::int_t;
        rational n1{4}, n2{-2};

        REQUIRE(n1 != n2);
        REQUIRE(n1 == n1);
        REQUIRE(rational{} == rational{});
        REQUIRE(rational{} == 0);
        REQUIRE(0 == rational{});
        REQUIRE(wchar_t{0} == rational{});
        REQUIRE(n1 == 4);
        REQUIRE(n1 == wchar_t{4});
        REQUIRE(n1 == integer{4});
        REQUIRE(integer{4} == n1);
        REQUIRE(4u == n1);
        REQUIRE(n1 != 3);
        REQUIRE(n1 != wchar_t{3});
        REQUIRE(wchar_t{3} != n1);
        REQUIRE(static_cast<signed char>(-3) != n1);
        REQUIRE(4ull == n1);
        REQUIRE(-2 == n2);
        REQUIRE(n2 == short(-2));
        REQUIRE(-2.f == n2);
        REQUIRE(n2 == -2.f);
        REQUIRE(-3.f != n2);
        REQUIRE(n2 != -3.f);
        REQUIRE(-2. == n2);
        REQUIRE(n2 == -2.);
        REQUIRE(-3. != n2);
        REQUIRE(n2 != -3.);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(-2.l == n2);
        REQUIRE(n2 == -2.l);
        REQUIRE(-3.l != n2);
        REQUIRE(n2 != -3.l);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(__int128_t{2} == rational{2});
        REQUIRE(rational{2} == __int128_t{2});
        REQUIRE(__uint128_t{2} == rational{2});
        REQUIRE(rational{2} == __uint128_t{2});
        REQUIRE(__int128_t{3} != rational{2});
        REQUIRE(rational{3} != __int128_t{2});
        REQUIRE(__uint128_t{3} != rational{2});
        REQUIRE(rational{3} != __uint128_t{2});
#endif

        REQUIRE(n2 < n1);
        REQUIRE(n2 < 0);
        REQUIRE(n2 < wchar_t{0});
        REQUIRE(wchar_t{0} < n1);
        REQUIRE(n2 < integer{0});
        REQUIRE(integer{-100} < n2);
        REQUIRE(-3 < n2);
        REQUIRE(n2 < 0u);
        REQUIRE(-3ll < n2);
        REQUIRE(n2 < 0.f);
        REQUIRE(-3.f < n2);
        REQUIRE(n2 < 0.);
        REQUIRE(-3. < n2);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(n2 < 0.l);
        REQUIRE(-3.l < n2);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(__int128_t{2} < rational{4});
        REQUIRE(rational{2} < __int128_t{3});
        REQUIRE(__uint128_t{2} < rational{4});
        REQUIRE(rational{2} < __uint128_t{3});
#endif

        REQUIRE(n1 > n2);
        REQUIRE(0 > n2);
        REQUIRE(wchar_t{0} > n2);
        REQUIRE(n1 > wchar_t{0});
        REQUIRE(integer{0} > n2);
        REQUIRE(n2 > integer{-150});
        REQUIRE(n2 > -3);
        REQUIRE(0u > n2);
        REQUIRE(n2 > -3ll);
        REQUIRE(0.f > n2);
        REQUIRE(n2 > -3.f);
        REQUIRE(0. > n2);
        REQUIRE(n2 > -3.);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(0.l > n2);
        REQUIRE(n2 > -3.l);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(__int128_t{6} > rational{4});
        REQUIRE(rational{7} > __int128_t{3});
        REQUIRE(__uint128_t{5} > rational{4});
        REQUIRE(rational{34} > __uint128_t{3});
#endif

        REQUIRE(n2 <= n1);
        REQUIRE(n1 <= n1);
        REQUIRE(rational{} <= rational{});
        REQUIRE(rational{} <= 0);
        REQUIRE(rational{} <= wchar_t{0});
        REQUIRE(wchar_t{0} <= rational{});
        REQUIRE(0 <= rational{});
        REQUIRE(rational{} <= integer{0});
        REQUIRE(integer{0} <= rational{});
        REQUIRE(-2 <= n2);
        REQUIRE(n2 <= -2);
        REQUIRE(n2 <= 0);
        REQUIRE(-3 <= n2);
        REQUIRE(n2 <= 0u);
        REQUIRE(-3ll <= n2);
        REQUIRE(n2 <= 0.f);
        REQUIRE(-3.f <= n2);
        REQUIRE(-2.f <= n2);
        REQUIRE(n2 <= -2.f);
        REQUIRE(n2 <= 0.);
        REQUIRE(-3. <= n2);
        REQUIRE(-2. <= n2);
        REQUIRE(n2 <= -2.);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(n2 <= 0.l);
        REQUIRE(-3.l <= n2);
        REQUIRE(-2.l <= n2);
        REQUIRE(n2 <= -2.l);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(__int128_t{2} <= rational{4});
        REQUIRE(rational{2} <= __int128_t{2});
        REQUIRE(__uint128_t{2} <= rational{4});
        REQUIRE(rational{2} <= __uint128_t{2});
#endif

        REQUIRE(n1 >= n2);
        REQUIRE(n1 >= n1);
        REQUIRE(rational{} >= rational{});
        REQUIRE(rational{} >= 0);
        REQUIRE(rational{} >= wchar_t{0});
        REQUIRE(wchar_t{0} >= rational{});
        REQUIRE(0 >= rational{});
        REQUIRE(rational{} >= integer{0});
        REQUIRE(integer{0} >= rational{});
        REQUIRE(-2 >= n2);
        REQUIRE(n2 >= -2);
        REQUIRE(0 >= n2);
        REQUIRE(n2 >= -3);
        REQUIRE(0u >= n2);
        REQUIRE(n2 >= -3ll);
        REQUIRE(0.f >= n2);
        REQUIRE(n2 >= -3.f);
        REQUIRE(-2.f >= n2);
        REQUIRE(n2 >= -2.f);
        REQUIRE(0. >= n2);
        REQUIRE(n2 >= -3.);
        REQUIRE(-2. >= n2);
        REQUIRE(n2 >= -2.);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(0.l >= n2);
        REQUIRE(n2 >= -3.l);
        REQUIRE(-2.l >= n2);
        REQUIRE(n2 >= -2.l);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(__int128_t{5} >= rational{4});
        REQUIRE(rational{2} >= __int128_t{2});
        REQUIRE(__uint128_t{8} >= rational{4});
        REQUIRE(rational{2} >= __uint128_t{2});
#endif
    }
};

TEST_CASE("rel")
{
    tuple_for_each(sizes{}, rel_tester{});
}

struct incdec_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        rational q;
        REQUIRE(++q == 1);
        REQUIRE(q++ == 1);
        REQUIRE(q == 2);
        REQUIRE(--q == 1);
        REQUIRE(q-- == 1);
        REQUIRE(q == 0);
        REQUIRE(--q == -1);
        q = rational{-23, 7};
        REQUIRE(++q == rational{-16, 7});
        REQUIRE(q++ == rational{-16, 7});
        REQUIRE(++q == rational{-2, 7});
        REQUIRE(++q == rational{5, 7});
        REQUIRE(--q == rational{-2, 7});
        REQUIRE(--q == rational{-9, 7});
        REQUIRE(--q == rational{-16, 7});
        REQUIRE(q-- == rational{-16, 7});
        REQUIRE(q == rational{-23, 7});
    }
};

TEST_CASE("incdec")
{
    tuple_for_each(sizes{}, incdec_tester{});
}
