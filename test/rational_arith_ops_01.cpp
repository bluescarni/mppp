// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <gmp.h>

#include <mp++/config.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
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
using is_addable = detail::is_detected<add_t, T, U>;

template <typename T, typename U>
using is_addable_inplace = detail::is_detected<inplace_add_t, T, U>;

struct add_tester {
    template <typename S>
    // NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
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
        n1 = 1;
        REQUIRE(std::is_same<std::complex<float>, decltype(n1 + std::complex<float>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<float>, decltype(std::complex<float>{4, 0} + n1)>::value);
        REQUIRE(n1 + std::complex<float>{4, 0} == std::complex<float>{5, 0});
        REQUIRE(std::complex<float>{4, 0} + n1 == std::complex<float>{5, 0});
        if (std::numeric_limits<float>::is_iec559) {
            REQUIRE((n1 / 2) + std::complex<float>{4, 0} == std::complex<float>{4.5f, 0});
            REQUIRE(std::complex<float>{4, 0} + (n1 / 2) == std::complex<float>{4.5f, 0});
        }

        REQUIRE(std::is_same<std::complex<double>, decltype(n1 + std::complex<double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(std::complex<double>{4, 0} + n1)>::value);
        REQUIRE(n1 + std::complex<double>{4, 0} == std::complex<double>{5, 0});
        REQUIRE(std::complex<double>{4, 0} + n1 == std::complex<double>{5, 0});
        if (std::numeric_limits<double>::is_iec559) {
            REQUIRE((n1 / 2) + std::complex<double>{4, 0} == std::complex<double>{4.5, 0});
            REQUIRE(std::complex<double>{4, 0} + (n1 / 2) == std::complex<double>{4.5, 0});
        }

#if defined(MPPP_WITH_MPFR)
        REQUIRE(std::is_same<std::complex<long double>, decltype(n1 + std::complex<long double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<long double>, decltype(std::complex<long double>{4, 0} + n1)>::value);
        REQUIRE(n1 + std::complex<long double>{4, 0} == std::complex<long double>{5, 0});
        REQUIRE(std::complex<long double>{4, 0} + n1 == std::complex<long double>{5, 0});
        if (std::numeric_limits<long double>::is_iec559) {
            REQUIRE((n1 / 2) + std::complex<long double>{4, 0} == std::complex<long double>{4.5l, 0});
            REQUIRE(std::complex<long double>{4, 0} + (n1 / 2) == std::complex<long double>{4.5l, 0});
        }
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
        retval = 12;
        retval += std::complex<float>{1, 0};
        REQUIRE(std::is_same<rational &, decltype(retval += std::complex<float>{1, 0})>::value);
        REQUIRE(retval == 13);
        if (std::numeric_limits<float>::is_iec559) {
            retval += std::complex<float>{1.5f, 0};
            REQUIRE(retval == rational{29, 2});
            retval = 13;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval += std::complex<float>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(1.f);
            });

        retval += std::complex<double>{1, 0};
        REQUIRE(std::is_same<rational &, decltype(retval += std::complex<double>{1, 0})>::value);
        REQUIRE(retval == 14);
        if (std::numeric_limits<double>::is_iec559) {
            retval += std::complex<double>{1.5, 0};
            REQUIRE(retval == rational{31, 2});
            retval = 14;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval += std::complex<double>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(1.);
            });
#if defined(MPPP_WITH_MPFR)
        retval += std::complex<long double>{1, 0};
        REQUIRE(std::is_same<rational &, decltype(retval += std::complex<long double>{1, 0})>::value);
        REQUIRE(retval == 15);
        if (std::numeric_limits<long double>::is_iec559) {
            retval += std::complex<long double>{1.5l, 0};
            REQUIRE(retval == rational{33, 2});
            retval = 15;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval += std::complex<long double>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(1.l);
            });
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
            n = detail::nl_max<int>();
            REQUIRE_THROWS_AS(n += rational{1}, std::overflow_error);
            n = detail::nl_min<int>();
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
                retval = 1;
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
        std::complex<float> cf{1, 2};
        cf += rational{2};
        REQUIRE(std::is_same<std::complex<float> &, decltype(cf += rational{2})>::value);
        REQUIRE(cf == std::complex<float>{3, 2});
        if (std::numeric_limits<float>::is_iec559) {
            cf += rational{1, 2};
            REQUIRE(cf == std::complex<float>{3.5f, 2});
        }

        std::complex<double> cd{1, 2};
        cd += rational{2};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd += rational{2})>::value);
        REQUIRE(cd == std::complex<double>{3, 2});
        if (std::numeric_limits<double>::is_iec559) {
            cd += rational{1, 2};
            REQUIRE(cd == std::complex<double>{3.5, 2});
        }

#if defined(MPPP_WITH_MPFR)
        std::complex<long double> cld{1, 2};
        cld += rational{2};
        REQUIRE(std::is_same<std::complex<long double> &, decltype(cld += rational{2})>::value);
        REQUIRE(cld == std::complex<long double>{3, 2});
        if (std::numeric_limits<long double>::is_iec559) {
            cld += rational{1, 2};
            REQUIRE(cld == std::complex<long double>{3.5l, 2});
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
using is_subtractable = detail::is_detected<sub_t, T, U>;

template <typename T, typename U>
using is_subtractable_inplace = detail::is_detected<inplace_sub_t, T, U>;

struct sub_tester {
    template <typename S>
    // NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
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
        n1 = 1;
        REQUIRE(std::is_same<std::complex<float>, decltype(n1 - std::complex<float>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<float>, decltype(std::complex<float>{4, 0} - n1)>::value);
        REQUIRE(n1 - std::complex<float>{4, 0} == std::complex<float>{-3, 0});
        REQUIRE(std::complex<float>{4, 0} - n1 == std::complex<float>{3, 0});
        if (std::numeric_limits<float>::is_iec559) {
            REQUIRE((n1 / 2) - std::complex<float>{4, 0} == std::complex<float>{-3.5f, 0});
            REQUIRE(std::complex<float>{4, 0} - (n1 / 2) == std::complex<float>{3.5f, 0});
        }

        REQUIRE(std::is_same<std::complex<double>, decltype(n1 - std::complex<double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(std::complex<double>{4, 0} - n1)>::value);
        REQUIRE(n1 - std::complex<double>{4, 0} == std::complex<double>{-3, 0});
        REQUIRE(std::complex<double>{4, 0} - n1 == std::complex<double>{3, 0});
        if (std::numeric_limits<double>::is_iec559) {
            REQUIRE((n1 / 2) - std::complex<double>{4, 0} == std::complex<double>{-3.5, 0});
            REQUIRE(std::complex<double>{4, 0} - (n1 / 2) == std::complex<double>{3.5, 0});
        }

#if defined(MPPP_WITH_MPFR)
        REQUIRE(std::is_same<std::complex<long double>, decltype(n1 - std::complex<long double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<long double>, decltype(std::complex<long double>{4, 0} - n1)>::value);
        REQUIRE(n1 - std::complex<long double>{4, 0} == std::complex<long double>{-3, 0});
        REQUIRE(std::complex<long double>{4, 0} - n1 == std::complex<long double>{3, 0});
        if (std::numeric_limits<long double>::is_iec559) {
            REQUIRE((n1 / 2) - std::complex<long double>{4, 0} == std::complex<long double>{-3.5l, 0});
            REQUIRE(std::complex<long double>{4, 0} - (n1 / 2) == std::complex<long double>{3.5l, 0});
        }
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
        retval = 12;
        retval -= std::complex<float>{1, 0};
        REQUIRE(std::is_same<rational &, decltype(retval -= std::complex<float>{1, 0})>::value);
        REQUIRE(retval == 11);
        if (std::numeric_limits<float>::is_iec559) {
            retval -= std::complex<float>{1.5f, 0};
            REQUIRE(retval == rational{19, 2});
            retval = 11;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval += std::complex<float>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(1.f);
            });

        retval -= std::complex<double>{1, 0};
        REQUIRE(std::is_same<rational &, decltype(retval += std::complex<double>{1, 0})>::value);
        REQUIRE(retval == 10);
        if (std::numeric_limits<double>::is_iec559) {
            retval -= std::complex<double>{1.5, 0};
            REQUIRE(retval == rational{17, 2});
            retval = 10;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval += std::complex<double>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(1.);
            });
#if defined(MPPP_WITH_MPFR)
        retval -= std::complex<long double>{1, 0};
        REQUIRE(std::is_same<rational &, decltype(retval += std::complex<long double>{1, 0})>::value);
        REQUIRE(retval == 9);
        if (std::numeric_limits<long double>::is_iec559) {
            retval -= std::complex<long double>{1.5l, 0};
            REQUIRE(retval == rational{15, 2});
            retval = 9;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval += std::complex<long double>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(1.l);
            });
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
            n = detail::nl_max<int>();
            REQUIRE_THROWS_AS(n -= rational{-1}, std::overflow_error);
            n = detail::nl_min<int>();
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
                retval = 1;
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
        std::complex<float> cf{1, 2};
        cf -= rational{2};
        REQUIRE(std::is_same<std::complex<float> &, decltype(cf -= rational{2})>::value);
        REQUIRE(cf == std::complex<float>{-1, 2});
        if (std::numeric_limits<float>::is_iec559) {
            cf -= rational{1, 2};
            REQUIRE(cf == std::complex<float>{-1.5f, 2});
        }

        std::complex<double> cd{1, 2};
        cd -= rational{2};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd -= rational{2})>::value);
        REQUIRE(cd == std::complex<double>{-1, 2});
        if (std::numeric_limits<double>::is_iec559) {
            cd -= rational{1, 2};
            REQUIRE(cd == std::complex<double>{-1.5, 2});
        }

#if defined(MPPP_WITH_MPFR)
        std::complex<long double> cld{1, 2};
        cld -= rational{2};
        REQUIRE(std::is_same<std::complex<long double> &, decltype(cld -= rational{2})>::value);
        REQUIRE(cld == std::complex<long double>{-1, 2});
        if (std::numeric_limits<long double>::is_iec559) {
            cld -= rational{1, 2};
            REQUIRE(cld == std::complex<long double>{-1.5l, 2});
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
        retval -= *&retval;
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
using is_multipliable = detail::is_detected<mul_t, T, U>;

template <typename T, typename U>
using is_multipliable_inplace = detail::is_detected<inplace_mul_t, T, U>;

struct mul_tester {
    template <typename S>
    // NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
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
        n1 = 2;
        REQUIRE(std::is_same<std::complex<float>, decltype(n1 * std::complex<float>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<float>, decltype(std::complex<float>{4, 0} * n1)>::value);
        REQUIRE(n1 * std::complex<float>{4, 0} == std::complex<float>{8, 0});
        REQUIRE(std::complex<float>{4, 0} * n1 == std::complex<float>{8, 0});
        if (std::numeric_limits<float>::is_iec559) {
            REQUIRE((n1 / 2) * std::complex<float>{4, 0} == std::complex<float>{4.f, 0});
            REQUIRE(std::complex<float>{4, 0} * (n1 / 2) == std::complex<float>{4.f, 0});
        }

        REQUIRE(std::is_same<std::complex<double>, decltype(n1 * std::complex<double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(std::complex<double>{4, 0} * n1)>::value);
        REQUIRE(n1 * std::complex<double>{4, 0} == std::complex<double>{8, 0});
        REQUIRE(std::complex<double>{4, 0} * n1 == std::complex<double>{8, 0});
        if (std::numeric_limits<double>::is_iec559) {
            REQUIRE((n1 / 2) * std::complex<double>{4, 0} == std::complex<double>{4., 0});
            REQUIRE(std::complex<double>{4, 0} * (n1 / 2) == std::complex<double>{4., 0});
        }

#if defined(MPPP_WITH_MPFR)
        REQUIRE(std::is_same<std::complex<long double>, decltype(n1 * std::complex<long double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<long double>, decltype(std::complex<long double>{4, 0} * n1)>::value);
        REQUIRE(n1 * std::complex<long double>{4, 0} == std::complex<long double>{8, 0});
        REQUIRE(std::complex<long double>{4, 0} * n1 == std::complex<long double>{8, 0});
        if (std::numeric_limits<long double>::is_iec559) {
            REQUIRE((n1 / 2) * std::complex<long double>{4, 0} == std::complex<long double>{4.l, 0});
            REQUIRE(std::complex<long double>{4, 0} * (n1 / 2) == std::complex<long double>{4.l, 0});
        }
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
        retval = 3;
        retval *= std::complex<float>{2, 0};
        REQUIRE(std::is_same<rational &, decltype(retval *= std::complex<float>{2, 0})>::value);
        REQUIRE(retval == 6);
        if (std::numeric_limits<float>::is_iec559) {
            retval *= std::complex<float>{.25f, 0};
            REQUIRE(retval == rational{3, 2});
            retval = 3;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval *= std::complex<float>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(3.f);
            });

        retval *= std::complex<double>{2, 0};
        REQUIRE(std::is_same<rational &, decltype(retval *= std::complex<double>{2, 0})>::value);
        REQUIRE(retval == 6);
        if (std::numeric_limits<double>::is_iec559) {
            retval *= std::complex<double>{.25, 0};
            REQUIRE(retval == rational{3, 2});
            retval = 3;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval *= std::complex<double>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(3.);
            });
#if defined(MPPP_WITH_MPFR)
        retval *= std::complex<long double>{2, 0};
        REQUIRE(std::is_same<rational &, decltype(retval *= std::complex<long double>{2, 0})>::value);
        REQUIRE(retval == 6);
        if (std::numeric_limits<double>::is_iec559) {
            retval *= std::complex<double>{.25l, 0};
            REQUIRE(retval == rational{3, 2});
            retval = 3;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval *= std::complex<long double>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(3.l);
            });
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
            n = detail::nl_max<int>();
            REQUIRE_THROWS_AS(n *= rational{2}, std::overflow_error);
            n = detail::nl_min<int>();
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
                retval = 1;
                REQUIRE_THROWS_PREDICATE(
                    retval *= std::numeric_limits<double>::infinity(), std::domain_error,
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
        std::complex<float> cf{1, 2};
        cf *= rational{2};
        REQUIRE(std::is_same<std::complex<float> &, decltype(cf *= rational{2})>::value);
        REQUIRE(cf == std::complex<float>{2, 4});
        if (std::numeric_limits<float>::is_iec559) {
            cf *= rational{1, 4};
            REQUIRE(cf == std::complex<float>{.5f, 1});
        }

        std::complex<double> cd{1, 2};
        cd *= rational{2};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd *= rational{2})>::value);
        REQUIRE(cd == std::complex<double>{2, 4});
        if (std::numeric_limits<double>::is_iec559) {
            cd *= rational{1, 4};
            REQUIRE(cd == std::complex<double>{.5, 1});
        }

#if defined(MPPP_WITH_MPFR)
        std::complex<long double> cld{1, 2};
        cld *= rational{2};
        REQUIRE(std::is_same<std::complex<long double> &, decltype(cld *= rational{2})>::value);
        REQUIRE(cld == std::complex<long double>{2, 4});
        if (std::numeric_limits<long double>::is_iec559) {
            cld *= rational{1, 4};
            REQUIRE(cld == std::complex<long double>{.5l, 1});
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
