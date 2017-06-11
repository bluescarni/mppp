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

#include <mp++/mp++.hpp>

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
        REQUIRE((std::is_same<rational, decltype(3 + rational{3})>::value));
        REQUIRE((std::is_same<rational, decltype(rational{3} + 3)>::value));
        REQUIRE((lex_cast(rational{-3, 2} + (signed char)4) == "5/2"));
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
        retval += (short)-1;
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
            n = std::numeric_limits<int>::max();
            REQUIRE_THROWS_AS(n += rational{1}, std::overflow_error);
            n = std::numeric_limits<int>::min();
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
        REQUIRE((!is_addable_inplace<rational, std::string>::value));
        REQUIRE((!is_addable_inplace<std::string, rational>::value));
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
        REQUIRE((std::is_same<rational, decltype(3 - rational{3})>::value));
        REQUIRE((std::is_same<rational, decltype(rational{3} - 3)>::value));
        REQUIRE((lex_cast(rational{-3, 2} - (signed char)4) == "-11/2"));
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
        retval -= (short)-1;
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
            n = std::numeric_limits<int>::max();
            REQUIRE_THROWS_AS(n -= rational{-1}, std::overflow_error);
            n = std::numeric_limits<int>::min();
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
        REQUIRE((!is_subtractable_inplace<rational, std::string>::value));
        REQUIRE((!is_subtractable_inplace<std::string, rational>::value));
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
        REQUIRE((std::is_same<rational, decltype(3 * rational{3})>::value));
        REQUIRE((std::is_same<rational, decltype(rational{3} * 3)>::value));
        REQUIRE((lex_cast(rational{-3, 2} * (signed char)4) == "-6"));
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
        retval *= (short)-1;
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
            n = std::numeric_limits<int>::max();
            REQUIRE_THROWS_AS(n *= rational{2}, std::overflow_error);
            n = std::numeric_limits<int>::min();
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
        REQUIRE((!is_multipliable_inplace<rational, std::string>::value));
        REQUIRE((!is_multipliable_inplace<std::string, rational>::value));
    }
};

TEST_CASE("mul")
{
    tuple_for_each(sizes{}, mul_tester{});
}
#if 0
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
        using integer = integer<S::value>;
        integer n1{4}, n2{-2};
        REQUIRE((lex_cast(n1 / n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 / n2), integer>::value));
        REQUIRE((lex_cast(n1 / char(4)) == "1"));
        REQUIRE((lex_cast(char(4) / n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 / char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) / n2), integer>::value));
        REQUIRE((lex_cast(n1 / (unsigned char)(4)) == "1"));
        REQUIRE((lex_cast((unsigned char)(4) / n2) == "-2"));
        REQUIRE((lex_cast(n1 / short(4)) == "1"));
        REQUIRE((lex_cast(short(4) / n2) == "-2"));
        REQUIRE((lex_cast(n1 / 4) == "1"));
        REQUIRE((lex_cast(4 / n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 / 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 / n2), integer>::value));
        REQUIRE((lex_cast(n1 / 4u) == "1"));
        REQUIRE((lex_cast(4u / n2) == "-2"));
        REQUIRE((n1 / 4.f == 1.f));
        REQUIRE((4.f / n2 == -2.f));
        REQUIRE((std::is_same<decltype(n1 / 4.f), float>::value));
        REQUIRE((std::is_same<decltype(4.f / n2), float>::value));
        REQUIRE((n1 / 4. == 1.));
        REQUIRE((4. / n2 == -2.));
        REQUIRE((std::is_same<decltype(n1 / 4.), double>::value));
        REQUIRE((std::is_same<decltype(4. / n2), double>::value));
#if defined(MPPP_WITH_MPFR)
        REQUIRE((n1 / 4.l == 1.l));
        REQUIRE((4.l / n2 == -2.l));
        REQUIRE((std::is_same<decltype(n1 / 4.l), long double>::value));
        REQUIRE((std::is_same<decltype(4.l / n2), long double>::value));
#endif
        // In-place div.
        integer retval{2};
        retval /= n1;
        REQUIRE((lex_cast(retval) == "0"));
        retval = 2;
        retval /= 1;
        REQUIRE((lex_cast(retval) == "2"));
        retval /= short(-1);
        REQUIRE((lex_cast(retval) == "-2"));
        retval /= (signed char)(-1);
        REQUIRE((lex_cast(retval) == "2"));
        retval /= (long long)(-5);
        REQUIRE((lex_cast(retval) == "0"));
        retval = -20;
        retval /= (unsigned long long)(20);
        REQUIRE((lex_cast(retval) == "-1"));
        retval /= 2.5f;
        REQUIRE((lex_cast(retval) == "0"));
        retval = 10;
        retval /= -3.5;
        REQUIRE((lex_cast(retval) == lex_cast(integer{10. / -3.5})));
#if defined(MPPP_WITH_MPFR)
        retval /= -1.5l;
        REQUIRE((lex_cast(retval) == lex_cast(integer{10. / -3.5 / -1.5l})));
#endif
        // In-place with interop on the lhs.
        short nl = 12;
        nl /= integer{3};
        REQUIRE((std::is_same<short &, decltype(nl /= integer{1})>::value));
        REQUIRE(nl == 4);
        nl /= integer{-2};
        REQUIRE(nl == -2);
        REQUIRE_THROWS_AS(nl /= integer{}, zero_division_error);
        unsigned long long unl = 24;
        unl /= integer{2};
        REQUIRE(unl == 12);
        REQUIRE_THROWS_AS(unl /= integer{-1}, std::overflow_error);
        double dl = 1.2;
        dl /= integer{2};
        REQUIRE(dl == 1.2 / 2.);
        REQUIRE((std::is_same<double &, decltype(dl /= integer{1})>::value));
        if (std::numeric_limits<double>::is_iec559) {
            dl = std::numeric_limits<double>::infinity();
            dl /= integer{2};
            REQUIRE(dl == std::numeric_limits<double>::infinity());
        }
        // Error checking.
        REQUIRE_THROWS_PREDICATE(integer{1} / integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(integer{1} / 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(1 / integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval /= integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval /= 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        if (std::numeric_limits<double>::is_iec559) {
            REQUIRE((integer{4} / 0. == std::numeric_limits<double>::infinity()));
            REQUIRE((integer{-4} / 0. == -std::numeric_limits<double>::infinity()));
            REQUIRE_THROWS_PREDICATE(retval /= 0., std::domain_error, [&retval](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct an integer from the non-finite floating-point value "
                              + (retval.sgn() > 0 ? std::to_string(std::numeric_limits<double>::infinity())
                                                  : std::to_string(-std::numeric_limits<double>::infinity()));
            });
        }
        // Type traits.
        REQUIRE((!is_divisible<integer, std::string>::value));
        REQUIRE((!is_divisible<std::string, integer>::value));
        REQUIRE((!is_divisible_inplace<integer, std::string>::value));
        REQUIRE((!is_divisible_inplace<const integer, int>::value));
        REQUIRE((!is_divisible_inplace<std::string, integer>::value));
        REQUIRE((!is_divisible_inplace<const int, integer>::value));
    }
};

TEST_CASE("div")
{
    tuple_for_each(sizes{}, div_tester{});
}

template <typename T, typename U>
using lshift_t = decltype(std::declval<const T &>() << std::declval<const U &>());

template <typename T, typename U>
using inplace_lshift_t = decltype(std::declval<T &>() <<= std::declval<const U &>());

template <typename T, typename U>
using is_lshiftable = is_detected<lshift_t, T, U>;

template <typename T, typename U>
using is_lshiftable_inplace = is_detected<inplace_lshift_t, T, U>;

template <typename T, typename U>
using rshift_t = decltype(std::declval<const T &>() >> std::declval<const U &>());

template <typename T, typename U>
using inplace_rshift_t = decltype(std::declval<T &>() >>= std::declval<const U &>());

template <typename T, typename U>
using is_rshiftable = is_detected<rshift_t, T, U>;

template <typename T, typename U>
using is_rshiftable_inplace = is_detected<inplace_rshift_t, T, U>;

struct shift_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer ret;
        REQUIRE((lex_cast(ret << 0) == "0"));
        REQUIRE((lex_cast(ret << 1u) == "0"));
        REQUIRE((lex_cast(ret << short(2)) == "0"));
        ret = 1;
        REQUIRE((lex_cast(ret << 1) == "2"));
        REQUIRE((lex_cast(ret << 2ll) == "4"));
        ret.neg();
        REQUIRE((lex_cast(ret << 3ull) == "-8"));
        REQUIRE((lex_cast(ret <<= 3ull) == "-8"));
        REQUIRE((lex_cast(ret <<= char(1)) == "-16"));
        REQUIRE((lex_cast(ret <<= (signed char)(0)) == "-16"));
        REQUIRE((lex_cast(ret >> 0) == "-16"));
        REQUIRE((lex_cast(ret >> 1) == "-8"));
        REQUIRE((lex_cast(ret >>= 1ul) == "-8"));
        REQUIRE((lex_cast(ret >>= short(1)) == "-4"));
        REQUIRE((lex_cast(ret >> 128) == "0"));
        // Error handling.
        REQUIRE_THROWS_PREDICATE(ret << -1, std::domain_error, [](const std::domain_error &ex) {
            return std::string(ex.what()) == "Cannot bit shift by -1: negative values are not supported";
        });
        REQUIRE_THROWS_PREDICATE(ret <<= -2, std::domain_error, [](const std::domain_error &ex) {
            return std::string(ex.what()) == "Cannot bit shift by -2: negative values are not supported";
        });
        REQUIRE_THROWS_PREDICATE(ret >> -1, std::domain_error, [](const std::domain_error &ex) {
            return std::string(ex.what()) == "Cannot bit shift by -1: negative values are not supported";
        });
        REQUIRE_THROWS_PREDICATE(ret >>= -2, std::domain_error, [](const std::domain_error &ex) {
            return std::string(ex.what()) == "Cannot bit shift by -2: negative values are not supported";
        });
        if (std::numeric_limits<unsigned long long>::max() > std::numeric_limits<::mp_bitcnt_t>::max()) {
            REQUIRE_THROWS_PREDICATE(ret << std::numeric_limits<unsigned long long>::max(), std::domain_error,
                                     [](const std::domain_error &ex) {
                                         return std::string(ex.what())
                                                == "Cannot bit shift by "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + ": the value is too large";
                                     });
            REQUIRE_THROWS_PREDICATE(ret <<= std::numeric_limits<unsigned long long>::max(), std::domain_error,
                                     [](const std::domain_error &ex) {
                                         return std::string(ex.what())
                                                == "Cannot bit shift by "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + ": the value is too large";
                                     });
            REQUIRE_THROWS_PREDICATE(ret >> std::numeric_limits<unsigned long long>::max(), std::domain_error,
                                     [](const std::domain_error &ex) {
                                         return std::string(ex.what())
                                                == "Cannot bit shift by "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + ": the value is too large";
                                     });
            REQUIRE_THROWS_PREDICATE(ret >>= std::numeric_limits<unsigned long long>::max(), std::domain_error,
                                     [](const std::domain_error &ex) {
                                         return std::string(ex.what())
                                                == "Cannot bit shift by "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + ": the value is too large";
                                     });
        }
        if ((unsigned long long)std::numeric_limits<long long>::max() > std::numeric_limits<::mp_bitcnt_t>::max()) {
            REQUIRE_THROWS_PREDICATE(
                ret << std::numeric_limits<long long>::max(), std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot bit shift by " + std::to_string(std::numeric_limits<long long>::max())
                                  + ": the value is too large";
                });
            REQUIRE_THROWS_PREDICATE(
                ret <<= std::numeric_limits<long long>::max(), std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot bit shift by " + std::to_string(std::numeric_limits<long long>::max())
                                  + ": the value is too large";
                });
            REQUIRE_THROWS_PREDICATE(
                ret >> std::numeric_limits<long long>::max(), std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot bit shift by " + std::to_string(std::numeric_limits<long long>::max())
                                  + ": the value is too large";
                });
            REQUIRE_THROWS_PREDICATE(
                ret >>= std::numeric_limits<long long>::max(), std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot bit shift by " + std::to_string(std::numeric_limits<long long>::max())
                                  + ": the value is too large";
                });
        }
        // Type traits.
        REQUIRE((!is_lshiftable<integer, double>::value));
        REQUIRE((!is_lshiftable<integer, integer>::value));
        REQUIRE((!is_lshiftable<integer, std::string>::value));
        REQUIRE((!is_lshiftable<std::string, integer>::value));
        REQUIRE((!is_lshiftable_inplace<integer, std::string>::value));
        REQUIRE((!is_lshiftable_inplace<const integer, int>::value));
        REQUIRE((!is_lshiftable_inplace<std::string, integer>::value));
        REQUIRE((!is_lshiftable_inplace<const int, integer>::value));
        REQUIRE((!is_lshiftable<int, integer>::value));
        REQUIRE((!is_lshiftable_inplace<int, integer>::value));
        REQUIRE((!is_lshiftable_inplace<integer, double>::value));
        REQUIRE((!is_lshiftable_inplace<integer, integer>::value));
        REQUIRE((!is_rshiftable<integer, double>::value));
        REQUIRE((!is_rshiftable<integer, integer>::value));
        REQUIRE((!is_rshiftable<integer, std::string>::value));
        REQUIRE((!is_rshiftable<std::string, integer>::value));
        REQUIRE((!is_rshiftable_inplace<integer, std::string>::value));
        REQUIRE((!is_rshiftable_inplace<const integer, int>::value));
        REQUIRE((!is_rshiftable_inplace<std::string, integer>::value));
        REQUIRE((!is_rshiftable_inplace<const int, integer>::value));
        REQUIRE((!is_rshiftable<int, integer>::value));
        REQUIRE((!is_rshiftable_inplace<int, integer>::value));
        REQUIRE((!is_rshiftable_inplace<integer, double>::value));
        REQUIRE((!is_rshiftable_inplace<integer, integer>::value));
    }
};

TEST_CASE("shift")
{
    tuple_for_each(sizes{}, shift_tester{});
}

template <typename T, typename U>
using mod_t = decltype(std::declval<const T &>() << std::declval<const U &>());

template <typename T, typename U>
using inplace_mod_t = decltype(std::declval<T &>() <<= std::declval<const U &>());

template <typename T, typename U>
using is_modable = is_detected<mod_t, T, U>;

template <typename T, typename U>
using is_modable_inplace = is_detected<inplace_mod_t, T, U>;

struct mod_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n1{4}, n2{-2};
        REQUIRE((lex_cast(n1 % n2) == "0"));
        REQUIRE((std::is_same<decltype(n1 % n2), integer>::value));
        REQUIRE((lex_cast(n1 % char(3)) == "1"));
        REQUIRE((lex_cast(char(3) % n2) == "1"));
        REQUIRE((std::is_same<decltype(n1 % char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) % n2), integer>::value));
        REQUIRE((lex_cast(-n1 % (unsigned char)(3)) == "-1"));
        REQUIRE((lex_cast((unsigned char)(3) % n2) == "1"));
        REQUIRE((lex_cast(n1 % short(3)) == "1"));
        REQUIRE((lex_cast(-short(3) % n2) == "-1"));
        REQUIRE((lex_cast(n1 % -3) == "1"));
        REQUIRE((lex_cast(3 % -n2) == "1"));
        REQUIRE((std::is_same<decltype(n1 % 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 % n2), integer>::value));
        REQUIRE((lex_cast(n1 % 3u) == "1"));
        REQUIRE((lex_cast(3u % n2) == "1"));
        REQUIRE((lex_cast(0u % n2) == "0"));
        // In-place mod.
        integer retval{-2};
        retval %= n1;
        REQUIRE((lex_cast(retval) == "-2"));
        retval = 3;
        retval %= 2;
        REQUIRE((lex_cast(retval) == "1"));
        retval = -3;
        retval %= short(2);
        REQUIRE((lex_cast(retval) == "-1"));
        retval %= (signed char)(-1);
        REQUIRE((lex_cast(retval) == "0"));
        retval = 26;
        retval %= (long long)(-5);
        REQUIRE((lex_cast(retval) == "1"));
        retval = -19;
        retval %= (unsigned long long)(7);
        REQUIRE((lex_cast(retval) == "-5"));
        // CppInteroperable on the left.
        int n = 3;
        n %= integer{2};
        REQUIRE(n == 1);
        n = -3;
        n %= integer{2};
        REQUIRE(n == -1);
        // Error checking.
        REQUIRE_THROWS_PREDICATE(integer{1} % integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(integer{1} % 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(1 % integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval %= integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval %= 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE((!is_modable<integer, std::string>::value));
        REQUIRE((!is_modable<integer, double>::value));
        REQUIRE((!is_modable<std::string, integer>::value));
        REQUIRE((!is_modable_inplace<integer, std::string>::value));
        REQUIRE((!is_modable_inplace<const integer, int>::value));
        REQUIRE((!is_modable_inplace<std::string, integer>::value));
        REQUIRE((!is_modable_inplace<const int, integer>::value));
        REQUIRE((!is_modable<int, integer>::value));
        REQUIRE((!is_modable_inplace<int, integer>::value));
    }
};

TEST_CASE("mod")
{
    tuple_for_each(sizes{}, mod_tester{});
}

struct rel_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n1{4}, n2{-2};

        REQUIRE(n1 != n2);
        REQUIRE(n1 == n1);
        REQUIRE(integer{} == integer{});
        REQUIRE(integer{} == 0);
        REQUIRE(0 == integer{});
        REQUIRE(n1 == 4);
        REQUIRE(4u == n1);
        REQUIRE(n1 != 3);
        REQUIRE((signed char)-3 != n1);
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

        REQUIRE(n2 < n1);
        REQUIRE(n2 < 0);
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

        REQUIRE(n1 > n2);
        REQUIRE(0 > n2);
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

        REQUIRE(n2 <= n1);
        REQUIRE(n1 <= n1);
        REQUIRE(integer{} <= integer{});
        REQUIRE(integer{} <= 0);
        REQUIRE(0 <= integer{});
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

        REQUIRE(n1 >= n2);
        REQUIRE(n1 >= n1);
        REQUIRE(integer{} >= integer{});
        REQUIRE(integer{} >= 0);
        REQUIRE(0 >= integer{});
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
    }
};

TEST_CASE("rel")
{
    tuple_for_each(sizes{}, rel_tester{});
}
#endif