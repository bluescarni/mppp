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

template <typename T, typename U>
using divvv_t = decltype(std::declval<const T &>() / std::declval<const U &>());

template <typename T, typename U>
using inplace_divvv_t = decltype(std::declval<T &>() /= std::declval<const U &>());

template <typename T, typename U>
using is_divisible = detail::is_detected<divvv_t, T, U>;

template <typename T, typename U>
using is_divisible_inplace = detail::is_detected<inplace_divvv_t, T, U>;

struct div_tester {
    template <typename S>
    // NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
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
        n1 = 4;
        REQUIRE(std::is_same<std::complex<float>, decltype(n1 / std::complex<float>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<float>, decltype(std::complex<float>{4, 0} / n1)>::value);
        REQUIRE(n1 / std::complex<float>{4, 0} == std::complex<float>{1, 0});
        REQUIRE(std::complex<float>{4, 0} / n1 == std::complex<float>{1, 0});
        if (std::numeric_limits<float>::is_iec559) {
            REQUIRE((n1 / 2) / std::complex<float>{4, 0} == std::complex<float>{.5f, 0});
            REQUIRE(std::complex<float>{4, 0} / (n1 / 2) == std::complex<float>{2.f, 0});
        }

        REQUIRE(std::is_same<std::complex<double>, decltype(n1 / std::complex<double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(std::complex<double>{4, 0} / n1)>::value);
        REQUIRE(n1 / std::complex<double>{4, 0} == std::complex<double>{1, 0});
        REQUIRE(std::complex<double>{4, 0} / n1 == std::complex<double>{1, 0});
        if (std::numeric_limits<double>::is_iec559) {
            REQUIRE((n1 / 2) / std::complex<double>{4, 0} == std::complex<double>{.5, 0});
            REQUIRE(std::complex<double>{4, 0} / (n1 / 2) == std::complex<double>{2., 0});
        }

#if defined(MPPP_WITH_MPFR)
        REQUIRE(std::is_same<std::complex<long double>, decltype(n1 / std::complex<long double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<long double>, decltype(std::complex<long double>{4, 0} / n1)>::value);
        REQUIRE(n1 / std::complex<long double>{4, 0} == std::complex<long double>{1, 0});
        REQUIRE(std::complex<long double>{4, 0} / n1 == std::complex<long double>{1, 0});
        if (std::numeric_limits<long double>::is_iec559) {
            REQUIRE((n1 / 2) / std::complex<long double>{4, 0} == std::complex<long double>{.5l, 0});
            REQUIRE(std::complex<long double>{4, 0} / (n1 / 2) == std::complex<long double>{2.l, 0});
        }
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
        retval = 64;
        retval /= std::complex<float>{2, 0};
        REQUIRE(std::is_same<rational &, decltype(retval /= std::complex<float>{2, 0})>::value);
        REQUIRE(retval == 32);
        if (std::numeric_limits<float>::is_iec559) {
            retval /= std::complex<float>{.25f, 0};
            REQUIRE(retval == rational{128});
            retval = 32;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval /= std::complex<float>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(-32.f);
            });

        retval /= std::complex<double>{2, 0};
        REQUIRE(std::is_same<rational &, decltype(retval /= std::complex<double>{2, 0})>::value);
        REQUIRE(retval == 16);
        if (std::numeric_limits<double>::is_iec559) {
            retval /= std::complex<double>{.25, 0};
            REQUIRE(retval == rational{64});
            retval = 16;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval /= std::complex<double>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(-16.);
            });
#if defined(MPPP_WITH_MPFR)
        retval /= std::complex<long double>{2, 0};
        REQUIRE(std::is_same<rational &, decltype(retval /= std::complex<long double>{2, 0})>::value);
        REQUIRE(retval == 8);
        if (std::numeric_limits<double>::is_iec559) {
            retval /= std::complex<double>{.25l, 0};
            REQUIRE(retval == rational{32});
            retval = 8;
        }
        REQUIRE_THROWS_PREDICATE(
            (retval /= std::complex<long double>{0, 1}), std::domain_error, [](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(-8.l);
            });
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
            n = detail::nl_max<int>();
            REQUIRE_THROWS_AS(n /= (rational{1, 2}), std::overflow_error);
            n = detail::nl_min<int>();
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
        std::complex<float> cf{4, 2};
        cf /= rational{2};
        REQUIRE(std::is_same<std::complex<float> &, decltype(cf /= rational{2})>::value);
        REQUIRE(cf == std::complex<float>{2, 1});
        if (std::numeric_limits<float>::is_iec559) {
            cf /= rational{2};
            REQUIRE(cf == std::complex<float>{1, .5f});
        }

        std::complex<double> cd{4, 2};
        cd /= rational{2};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd /= rational{2})>::value);
        REQUIRE(cd == std::complex<double>{2, 1});
        if (std::numeric_limits<double>::is_iec559) {
            cd /= rational{2};
            REQUIRE(cd == std::complex<double>{1, .5});
        }

#if defined(MPPP_WITH_MPFR)
        std::complex<long double> cld{4, 2};
        cld /= rational{2};
        REQUIRE(std::is_same<std::complex<long double> &, decltype(cld /= rational{2})>::value);
        REQUIRE(cld == std::complex<long double>{2, 1});
        if (std::numeric_limits<long double>::is_iec559) {
            cld /= rational{2};
            REQUIRE(cld == std::complex<long double>{1, .5l});
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
        retval /= *&retval;
        REQUIRE(retval == rational(1));
    }
};

TEST_CASE("div")
{
    tuple_for_each(sizes{}, div_tester{});
}
