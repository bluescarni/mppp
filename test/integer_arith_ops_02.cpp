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
        using integer = integer<S::value>;
        integer n1{4}, n2{-2};
        REQUIRE((lex_cast(n1 / n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 / n2), integer>::value));
        REQUIRE((lex_cast(n1 / char(4)) == "1"));
        REQUIRE((lex_cast(char(4) / n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 / char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) / n2), integer>::value));
        REQUIRE((lex_cast(n1 / static_cast<unsigned char>(4)) == "1"));
        REQUIRE((lex_cast(static_cast<unsigned char>(4) / n2) == "-2"));
        REQUIRE((lex_cast(n1 / short(4)) == "1"));
        REQUIRE((lex_cast(short(4) / n2) == "-2"));
        REQUIRE((lex_cast(n1 / 4) == "1"));
        REQUIRE((lex_cast(4 / n2) == "-2"));
        REQUIRE((lex_cast(n1 / wchar_t{4}) == "1"));
        REQUIRE((lex_cast(wchar_t{4} / n2) == "-2"));
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
        REQUIRE(std::is_same<std::complex<float>, decltype(n1 / std::complex<float>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<float>, decltype(std::complex<float>{4, 0} / n1)>::value);
        REQUIRE(n1 / std::complex<float>{4, 0} == std::complex<float>{1, 0});
        REQUIRE(std::complex<float>{4, 0} / n1 == std::complex<float>{1, 0});

        REQUIRE(std::is_same<std::complex<double>, decltype(n1 / std::complex<double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(std::complex<double>{4, 0} / n1)>::value);
        REQUIRE(n1 / std::complex<double>{4, 0} == std::complex<double>{1, 0});
        REQUIRE(std::complex<double>{4, 0} / n1 == std::complex<double>{1, 0});

#if defined(MPPP_WITH_MPFR)
        REQUIRE(std::is_same<std::complex<long double>, decltype(n1 / std::complex<long double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<long double>, decltype(std::complex<long double>{4, 0} / n1)>::value);
        REQUIRE(n1 / std::complex<long double>{4, 0} == std::complex<long double>{1, 0});
        REQUIRE(std::complex<long double>{4, 0} / n1 == std::complex<long double>{1, 0});
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((n1 / __uint128_t{4} == 1));
        REQUIRE((__uint128_t{4} / n2 == -2));
        REQUIRE((n1 / __int128_t{-4} == -1));
        REQUIRE((__int128_t{-4} / n1 == -1));
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
        retval /= static_cast<signed char>(-1);
        REQUIRE((lex_cast(retval) == "2"));
        retval /= -5ll;
        REQUIRE((lex_cast(retval) == "0"));
        retval = -20;
        retval /= 20ull;
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
        retval = 24;
        retval /= std::complex<float>{2, 0};
        REQUIRE(std::is_same<integer &, decltype(retval /= std::complex<float>{2, 0})>::value);
        REQUIRE(retval == 12);
        REQUIRE_THROWS_PREDICATE((retval /= std::complex<float>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(-12.f) + " to an integer";
                                 });

        retval /= std::complex<double>{2, 0};
        REQUIRE(std::is_same<integer &, decltype(retval /= std::complex<double>{2, 0})>::value);
        REQUIRE(retval == 6);
        REQUIRE_THROWS_PREDICATE((retval /= std::complex<double>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(-6.) + " to an integer";
                                 });
#if defined(MPPP_WITH_MPFR)
        retval /= std::complex<long double>{2, 0};
        REQUIRE(std::is_same<integer &, decltype(retval /= std::complex<long double>{2, 0})>::value);
        REQUIRE(retval == 3);
        REQUIRE_THROWS_PREDICATE((retval /= std::complex<long double>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(-3.l) + " to an integer";
                                 });
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        retval = 1;
        retval /= __uint128_t{1};
        REQUIRE((retval == 1));
        retval /= __int128_t{-1};
        REQUIRE((retval == -1));
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
#if defined(MPPP_WITH_MPFR)
        long double ld = 4;
        ld /= integer{2};
        REQUIRE(std::is_same<long double &, decltype(ld /= integer{2})>::value);
        REQUIRE(ld == 2);
#endif
        std::complex<float> cf{4, 2};
        cf /= integer{2};
        REQUIRE(std::is_same<std::complex<float> &, decltype(cf /= integer{2})>::value);
        REQUIRE(cf == std::complex<float>{2, 1});

        std::complex<double> cd{4, 2};
        cd /= integer{2};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd /= integer{2})>::value);
        REQUIRE(cd == std::complex<double>{2, 1});
#if defined(MPPP_WITH_MPFR)
        std::complex<long double> cld{4, 2};
        cld /= integer{2};
        REQUIRE(std::is_same<std::complex<long double> &, decltype(cld /= integer{2})>::value);
        REQUIRE(cld == std::complex<long double>{2, 1});
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        __int128_t n128{-7};
        n128 /= integer{5};
        REQUIRE((n128 == -1));
        __uint128_t un128{6};
        un128 /= integer{3};
        REQUIRE((un128 == 2));
#endif
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
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE_THROWS_PREDICATE(integer{1} / __uint128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(integer{1} / __int128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval /= __uint128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval /= __int128_t{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
#endif
        if (std::numeric_limits<double>::is_iec559) {
            REQUIRE((integer{4} / 0. == std::numeric_limits<double>::infinity()));
            REQUIRE((integer{-4} / 0. == -std::numeric_limits<double>::infinity()));
            REQUIRE_THROWS_PREDICATE(retval /= 0., std::domain_error, [&retval](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot assign the non-finite floating-point value "
                              + (retval.sgn() > 0 ? std::to_string(std::numeric_limits<double>::infinity())
                                                  : std::to_string(-std::numeric_limits<double>::infinity()))
                              + " to an integer";
            });
        }
        // Type traits.
        REQUIRE((!is_divisible<integer, std::string>::value));
        REQUIRE((!is_divisible<std::string, integer>::value));
        REQUIRE((!is_divisible_inplace<integer, std::string>::value));
        REQUIRE((!is_divisible_inplace<const integer, int>::value));
        REQUIRE((!is_divisible_inplace<std::string, integer>::value));
        REQUIRE((!is_divisible_inplace<const int, integer>::value));

        // In-place div with self.
        retval = -5;
        retval /= *&retval;
        REQUIRE(retval == 1);
    }
};

TEST_CASE("div")
{
    tuple_for_each(sizes{}, div_tester{});
}

#if defined(_MSC_VER)

#pragma warning(pop)

#endif
