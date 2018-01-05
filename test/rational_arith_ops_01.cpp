// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
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

// Type traits to detect the availability of operators.
template <typename T, typename U>
using add_t = decltype(std::declval<const T &>() + std::declval<const U &>());

template <typename T, typename U>
using inplace_add_t = decltype(std::declval<T &>() += std::declval<const U &>());

template <typename T, typename U>
using is_addable = is_detected<add_t, T, U>;

template <typename T, typename U>
using is_addable_inplace = is_detected<inplace_add_t, T, U>;

struct add_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = typename rational::int_t;
        // Binary add.
        rational n1{1, 2}, n2{2, -3};
        REQUIRE((lex_cast(+n2) == "-2/3"));
        REQUIRE((lex_cast(n1 + n2) == "-1/6"));
        REQUIRE((std::is_same<rational, decltype(n1 + n2)>::value));
        REQUIRE((lex_cast(rational{3} + integer{4}) == "7"));
        REQUIRE((lex_cast(integer{4} + rational{3}) == "7"));
        REQUIRE((std::is_same<rational, decltype(integer{4} + rational{3})>::value));
        REQUIRE((std::is_same<rational, decltype(rational{3} + integer{4})>::value));
        REQUIRE((lex_cast(rational{-3, 2} + integer{4}) == "5/2"));
        REQUIRE((lex_cast(integer{4} + rational{-3, 2}) == "5/2"));
        REQUIRE((lex_cast(rational{3} + 4) == "7"));
        REQUIRE((lex_cast(4ul + rational{3}) == "7"));
        REQUIRE((lex_cast(rational{3} + wchar_t{4}) == "7"));
        REQUIRE((lex_cast(wchar_t{4} + rational{3}) == "7"));
        REQUIRE((std::is_same<rational, decltype(3 + rational{3})>::value));
        REQUIRE((std::is_same<rational, decltype(rational{3} + 3)>::value));
        REQUIRE((lex_cast(rational{-3, 2} + static_cast<signed char>(4)) == "5/2"));
        REQUIRE((lex_cast(4ll + rational{-3, 2}) == "5/2"));
        REQUIRE((rational{3} + 4.f == 7.f));
        REQUIRE((4.f + rational{3} == 7.f));
        REQUIRE((rational{3} + 4. == 7.));
        REQUIRE((4. + rational{3} == 7.));
        REQUIRE((std::is_same<double, decltype(integer{4} + 3.)>::value));
        REQUIRE((std::is_same<float, decltype(3.f + integer{4})>::value));
#if defined(MPPP_WITH_MPFR)
        REQUIRE((rational{3} + 4.l == 7.l));
        REQUIRE((4.l + rational{3} == 7.l));
        REQUIRE((std::is_same<long double, decltype(integer{4} + 3.l)>::value));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((rational{3} + __int128_t{4} == 7));
        REQUIRE((__int128_t{4} + rational{3} == 7));
        REQUIRE((rational{3} + __uint128_t{4} == 7));
        REQUIRE((__uint128_t{4} + rational{3} == 7));
#endif
        REQUIRE((!is_addable<rational, std::string>::value));
        REQUIRE((!is_addable<std::string, rational>::value));

        // In-place add.
        rational retval{1, 2};
        retval += rational{-2, 3};
        REQUIRE((std::is_same<rational &, decltype(retval += rational{-2, 3})>::value));
        REQUIRE((lex_cast(retval) == "-1/6"));
        retval += integer{1};
        REQUIRE((std::is_same<rational &, decltype(retval += integer{1})>::value));
        REQUIRE((lex_cast(retval) == "5/6"));
        retval = 5;
        retval += integer{-1};
        REQUIRE((lex_cast(retval) == "4"));
        retval = "1/2";
        retval += 1;
        REQUIRE((std::is_same<rational &, decltype(retval += 1)>::value));
        REQUIRE((lex_cast(retval) == "3/2"));
        retval += 1ull;
        REQUIRE((lex_cast(retval) == "5/2"));
        retval += static_cast<short>(-1);
        REQUIRE((lex_cast(retval) == "3/2"));
        retval += 2.f;
        REQUIRE((std::is_same<rational &, decltype(retval += 1.)>::value));
        REQUIRE((lex_cast(retval) == "7/2"));
        retval += 2.;
        REQUIRE((lex_cast(retval) == "11/2"));
#if defined(MPPP_WITH_MPFR)
        retval += 2.l;
        REQUIRE((lex_cast(retval) == "15/2"));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        retval = 1;
        retval += __int128_t{-5};
        REQUIRE(retval == -4);
        retval += __uint128_t{3};
        REQUIRE(retval == -1);
#endif

        // Interop on the left.
        {
            integer n{5};
            n += rational{-4};
            REQUIRE((std::is_same<integer &, decltype(n += rational{-4})>::value));
            REQUIRE((lex_cast(n) == "1"));
            n += rational{-5, 2};
            REQUIRE((lex_cast(n) == "-1"));
        }
        {
            int n = 5;
            n += rational{-4};
            REQUIRE((lex_cast(n) == "1"));
            REQUIRE((std::is_same<int &, decltype(n += rational{-4})>::value));
            n += rational{-5, 2};
            REQUIRE((lex_cast(n) == "-1"));
            n = nl_max<int>();
            REQUIRE_THROWS_AS(n += rational{1}, std::overflow_error);
            n = nl_min<int>();
            REQUIRE_THROWS_AS(n += rational{-1}, std::overflow_error);
        }
        {
            if (std::numeric_limits<double>::is_iec559) {
                double x = 5;
                x += rational{-4};
                REQUIRE((std::is_same<double &, decltype(x += rational{-4})>::value));
                REQUIRE((lex_cast(x) == "1"));
                x += rational{-5, 2};
                REQUIRE((std::abs(-1.5 - x) < 1E-8));
                REQUIRE_THROWS_PREDICATE(
                    retval += std::numeric_limits<double>::infinity(), std::domain_error,
                    [](const std::domain_error &ex) {
                        return std::string(ex.what())
                               == "Cannot construct a rational from the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<double>::infinity());
                    });
            }
        }
#if defined(MPPP_WITH_MPFR)
        {
            if (std::numeric_limits<long double>::is_iec559) {
                long double x = 5;
                x += rational{-4};
                REQUIRE((lex_cast(x) == "1"));
                x += rational{-5, 2};
                REQUIRE((std::abs(-1.5l - x) < 1E-8l));
            }
        }
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        {
            __int128_t n128 = -6;
            n128 += rational{-5};
            REQUIRE(n128 == -11);
            __uint128_t un128 = 6;
            un128 += rational{1};
            REQUIRE(un128 == 7);
        }
#endif
        REQUIRE((!is_addable_inplace<rational, std::string>::value));
        REQUIRE((!is_addable_inplace<std::string, rational>::value));

        // In-place add with self.
        retval = "3/4";
        retval += retval;
        REQUIRE(retval == rational(3, 2));
    }
};

TEST_CASE("add")
{
    tuple_for_each(sizes{}, add_tester{});
}

template <typename T, typename U>
using sub_t = decltype(std::declval<const T &>() - std::declval<const U &>());

template <typename T, typename U>
using inplace_sub_t = decltype(std::declval<T &>() -= std::declval<const U &>());

template <typename T, typename U>
using is_subtractable = is_detected<sub_t, T, U>;

template <typename T, typename U>
using is_subtractable_inplace = is_detected<inplace_sub_t, T, U>;

struct sub_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = typename rational::int_t;
        // Binary sub.
        rational n1{1, 2}, n2{2, -3};
        REQUIRE((lex_cast(-n2) == "2/3"));
        REQUIRE((lex_cast(n1 - n2) == "7/6"));
        REQUIRE((std::is_same<rational, decltype(n1 - n2)>::value));
        REQUIRE((lex_cast(rational{3} - integer{4}) == "-1"));
        REQUIRE((lex_cast(integer{4} - rational{3}) == "1"));
        REQUIRE((std::is_same<rational, decltype(integer{4} - rational{3})>::value));
        REQUIRE((std::is_same<rational, decltype(rational{3} - integer{4})>::value));
        REQUIRE((lex_cast(rational{-3, 2} - integer{4}) == "-11/2"));
        REQUIRE((lex_cast(integer{4} - rational{-3, 2}) == "11/2"));
        REQUIRE((lex_cast(rational{3} - 4) == "-1"));
        REQUIRE((lex_cast(4ul - rational{3}) == "1"));
        REQUIRE((lex_cast(rational{3} - wchar_t{4}) == "-1"));
        REQUIRE((lex_cast(wchar_t{4} - rational{3}) == "1"));
        REQUIRE((std::is_same<rational, decltype(3 - rational{3})>::value));
        REQUIRE((std::is_same<rational, decltype(rational{3} - 3)>::value));
        REQUIRE((lex_cast(rational{-3, 2} - static_cast<signed char>(4)) == "-11/2"));
        REQUIRE((lex_cast(4ll - rational{-3, 2}) == "11/2"));
        REQUIRE((rational{3} - 4.f == -1.f));
        REQUIRE((4.f - rational{3} == 1.f));
        REQUIRE((rational{3} - 4. == -1.));
        REQUIRE((4. - rational{3} == 1.));
        REQUIRE((std::is_same<double, decltype(integer{4} - 3.)>::value));
        REQUIRE((std::is_same<float, decltype(3.f - integer{4})>::value));
#if defined(MPPP_WITH_MPFR)
        REQUIRE((rational{3} - 4.l == -1.l));
        REQUIRE((4.l - rational{3} == 1.l));
        REQUIRE((std::is_same<long double, decltype(integer{4} - 3.l)>::value));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((rational{3} - __int128_t{4} == -1));
        REQUIRE((__int128_t{4} - rational{3} == 1));
        REQUIRE((rational{3} - __uint128_t{4} == -1));
        REQUIRE((__uint128_t{4} - rational{3} == 1));
#endif
        REQUIRE((!is_subtractable<rational, std::string>::value));
        REQUIRE((!is_subtractable<std::string, rational>::value));

        // In-place sub.
        rational retval{1, 2};
        retval -= rational{-2, 3};
        REQUIRE((std::is_same<rational &, decltype(retval -= rational{-2, 3})>::value));
        REQUIRE((lex_cast(retval) == "7/6"));
        retval -= integer{1};
        REQUIRE((std::is_same<rational &, decltype(retval -= integer{1})>::value));
        REQUIRE((lex_cast(retval) == "1/6"));
        retval = 5;
        retval -= integer{-1};
        REQUIRE((lex_cast(retval) == "6"));
        retval = "1/2";
        retval -= 1;
        REQUIRE((std::is_same<rational &, decltype(retval -= 1)>::value));
        REQUIRE((lex_cast(retval) == "-1/2"));
        retval -= 1ull;
        REQUIRE((lex_cast(retval) == "-3/2"));
        retval -= static_cast<short>(-1);
        REQUIRE((lex_cast(retval) == "-1/2"));
        retval -= 2.f;
        REQUIRE((std::is_same<rational &, decltype(retval -= 1.)>::value));
        REQUIRE((lex_cast(retval) == "-5/2"));
        retval -= 2.;
        REQUIRE((lex_cast(retval) == "-9/2"));
#if defined(MPPP_WITH_MPFR)
        retval -= 2.l;
        REQUIRE((lex_cast(retval) == "-13/2"));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        retval = 1;
        retval -= __int128_t{-5};
        REQUIRE(retval == 6);
        retval -= __uint128_t{3};
        REQUIRE(retval == 3);
#endif

        // Interop on the left.
        {
            integer n{5};
            n -= rational{-4};
            REQUIRE((std::is_same<integer &, decltype(n -= rational{-4})>::value));
            REQUIRE((lex_cast(n) == "9"));
            n -= rational{-5, 2};
            REQUIRE((lex_cast(n) == "11"));
        }
        {
            int n = 5;
            n -= rational{-4};
            REQUIRE((lex_cast(n) == "9"));
            REQUIRE((std::is_same<int &, decltype(n -= rational{-4})>::value));
            n -= rational{-5, 2};
            REQUIRE((lex_cast(n) == "11"));
            n = nl_max<int>();
            REQUIRE_THROWS_AS(n -= rational{-1}, std::overflow_error);
            n = nl_min<int>();
            REQUIRE_THROWS_AS(n -= rational{1}, std::overflow_error);
        }
        {
            if (std::numeric_limits<double>::is_iec559) {
                double x = 5;
                x -= rational{-4};
                REQUIRE((std::is_same<double &, decltype(x -= rational{-4})>::value));
                REQUIRE((lex_cast(x) == "9"));
                x -= rational{-5, 2};
                REQUIRE((std::abs(23. / 2 - x) < 1E-8));
                REQUIRE_THROWS_PREDICATE(
                    retval -= std::numeric_limits<double>::infinity(), std::domain_error,
                    [](const std::domain_error &ex) {
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
                x -= rational{-4};
                REQUIRE((lex_cast(x) == "9"));
                x -= rational{-5, 2};
                REQUIRE((std::abs(23.l / 2 - x) < 1E-8l));
            }
        }
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        {
            __int128_t n128 = -6;
            n128 -= rational{-5};
            REQUIRE(n128 == -1);
            __uint128_t un128 = 6;
            un128 -= rational{1};
            REQUIRE(un128 == 5);
        }
#endif
        REQUIRE((!is_subtractable_inplace<rational, std::string>::value));
        REQUIRE((!is_subtractable_inplace<std::string, rational>::value));

        // In-place sub with self.
        retval = "3/4";
        retval -= retval;
        REQUIRE(retval == rational{});
    }
};

TEST_CASE("sub")
{
    tuple_for_each(sizes{}, sub_tester{});
}

template <typename T, typename U>
using mul_t = decltype(std::declval<const T &>() * std::declval<const U &>());

template <typename T, typename U>
using inplace_mul_t = decltype(std::declval<T &>() *= std::declval<const U &>());

template <typename T, typename U>
using is_multipliable = is_detected<mul_t, T, U>;

template <typename T, typename U>
using is_multipliable_inplace = is_detected<inplace_mul_t, T, U>;

struct mul_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = typename rational::int_t;
        // Binary mul.
        rational n1{1, 2}, n2{2, -3};
        REQUIRE((lex_cast(n1 * n2) == "-1/3"));
        REQUIRE((std::is_same<rational, decltype(n1 * n2)>::value));
        REQUIRE((lex_cast(rational{3} * integer{4}) == "12"));
        REQUIRE((lex_cast(integer{4} * rational{3}) == "12"));
        REQUIRE((std::is_same<rational, decltype(integer{4} * rational{3})>::value));
        REQUIRE((std::is_same<rational, decltype(rational{3} * integer{4})>::value));
        REQUIRE((lex_cast(rational{-3, 2} * integer{4}) == "-6"));
        REQUIRE((lex_cast(integer{4} * rational{-3, 2}) == "-6"));
        REQUIRE((lex_cast(rational{3} * 4) == "12"));
        REQUIRE((lex_cast(4ul * rational{3}) == "12"));
        REQUIRE((lex_cast(rational{3} * wchar_t{4}) == "12"));
        REQUIRE((lex_cast(wchar_t{4} * rational{3}) == "12"));
        REQUIRE((std::is_same<rational, decltype(3 * rational{3})>::value));
        REQUIRE((std::is_same<rational, decltype(rational{3} * 3)>::value));
        REQUIRE((lex_cast(rational{-3, 2} * static_cast<signed char>(4)) == "-6"));
        REQUIRE((lex_cast(4ll * rational{-3, 2}) == "-6"));
        REQUIRE((rational{3} * 4.f == 12.f));
        REQUIRE((4.f * rational{3} == 12.f));
        REQUIRE((rational{3} * 4. == 12.));
        REQUIRE((4. * rational{3} == 12.));
        REQUIRE((std::is_same<double, decltype(integer{4} * 3.)>::value));
        REQUIRE((std::is_same<float, decltype(3.f * integer{4})>::value));
#if defined(MPPP_WITH_MPFR)
        REQUIRE((rational{3} * 4.l == 12.l));
        REQUIRE((4.l * rational{3} == 12.l));
        REQUIRE((std::is_same<long double, decltype(integer{4} * 3.l)>::value));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((rational{3} * __int128_t{4} == 12));
        REQUIRE((__int128_t{4} * rational{3} == 12));
        REQUIRE((rational{3} * __uint128_t{4} == 12));
        REQUIRE((__uint128_t{4} * rational{3} == 12));
#endif
        REQUIRE((!is_multipliable<rational, std::string>::value));
        REQUIRE((!is_multipliable<std::string, rational>::value));

        // In-place mul.
        rational retval{1, 2};
        retval *= rational{-2, 3};
        REQUIRE((std::is_same<rational &, decltype(retval *= rational{-2, 3})>::value));
        REQUIRE((lex_cast(retval) == "-1/3"));
        retval *= integer{2};
        REQUIRE((std::is_same<rational &, decltype(retval *= integer{1})>::value));
        REQUIRE((lex_cast(retval) == "-2/3"));
        retval *= integer{-3};
        REQUIRE((lex_cast(retval) == "2"));
        retval *= integer{-5};
        REQUIRE((lex_cast(retval) == "-10"));
        retval = 5;
        retval *= integer{-1};
        REQUIRE((lex_cast(retval) == "-5"));
        retval = "1/2";
        retval *= 3;
        REQUIRE((std::is_same<rational &, decltype(retval *= 3)>::value));
        REQUIRE((lex_cast(retval) == "3/2"));
        retval *= 4ull;
        REQUIRE((lex_cast(retval) == "6"));
        retval *= static_cast<short>(-1);
        REQUIRE((lex_cast(retval) == "-6"));
        retval *= 2.f;
        REQUIRE((std::is_same<rational &, decltype(retval *= 1.)>::value));
        REQUIRE((lex_cast(retval) == "-12"));
        retval *= 2.;
        REQUIRE((lex_cast(retval) == "-24"));
#if defined(MPPP_WITH_MPFR)
        retval *= 2.l;
        REQUIRE((lex_cast(retval) == "-48"));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        retval = 1;
        retval *= __int128_t{-5};
        REQUIRE(retval == -5);
        retval *= __uint128_t{3};
        REQUIRE(retval == -15);
#endif

        // Interop on the left.
        {
            integer n{5};
            n *= rational{-4, 3};
            REQUIRE((std::is_same<integer &, decltype(n *= rational{-4})>::value));
            REQUIRE((lex_cast(n) == "-6"));
            n *= rational{-5, 2};
            REQUIRE((lex_cast(n) == "15"));
        }
        {
            int n = 5;
            n *= rational{-4, 3};
            REQUIRE((lex_cast(n) == "-6"));
            REQUIRE((std::is_same<int &, decltype(n *= rational{-4})>::value));
            n *= rational{-5, 2};
            REQUIRE((lex_cast(n) == "15"));
            n = nl_max<int>();
            REQUIRE_THROWS_AS(n *= rational{2}, std::overflow_error);
            n = nl_min<int>();
            REQUIRE_THROWS_AS(n *= rational{2}, std::overflow_error);
        }
        {
            if (std::numeric_limits<double>::is_iec559) {
                double x = 5;
                x *= rational{-5, 2};
                REQUIRE((std::is_same<double &, decltype(x *= rational{-4})>::value));
                REQUIRE((std::abs(-25. / 2 - x) < 1E-8));
                x *= rational{-5, 2};
                REQUIRE((std::abs(125. / 4 - x) < 1E-8));
                REQUIRE_THROWS_PREDICATE(
                    retval *= -std::numeric_limits<double>::infinity(), std::domain_error,
                    [](const std::domain_error &ex) {
                        return std::string(ex.what())
                               == "Cannot construct a rational from the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<double>::infinity());
                    });
            }
        }
#if defined(MPPP_WITH_MPFR)
        {
            if (std::numeric_limits<long double>::is_iec559) {
                long double x = 5;
                x *= rational{-5, 2};
                REQUIRE((std::abs(-25. / 2 - x) < 1E-8));
                x *= rational{-5, 2};
                REQUIRE((std::abs(125. / 4 - x) < 1E-8));
            }
        }
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        {
            __int128_t n128 = -6;
            n128 *= rational{-5};
            REQUIRE(n128 == 30);
            __uint128_t un128 = 6;
            un128 *= rational{2};
            REQUIRE(un128 == 12);
        }
#endif
        REQUIRE((!is_multipliable_inplace<rational, std::string>::value));
        REQUIRE((!is_multipliable_inplace<std::string, rational>::value));

        // In-place mul with self.
        retval = "-3/4";
        retval *= retval;
        REQUIRE(retval == rational(9, 16));
    }
};

TEST_CASE("mul")
{
    tuple_for_each(sizes{}, mul_tester{});
}
