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
        using integer = integer<S::value>;
        // Binary add.
        integer n1{1}, n2{-2};
        REQUIRE((lex_cast(+n2) == "-2"));
        REQUIRE((lex_cast(n1 + n2) == "-1"));
        REQUIRE((std::is_same<decltype(n1 + n2), integer>::value));
        REQUIRE((lex_cast(n1 + char(4)) == "5"));
        REQUIRE((lex_cast(char(4) + n2) == "2"));
        REQUIRE((std::is_same<decltype(n1 + char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) + n2), integer>::value));
        REQUIRE((lex_cast(n1 + static_cast<unsigned char>(4)) == "5"));
        REQUIRE((lex_cast(static_cast<unsigned char>(4) + n2) == "2"));
        REQUIRE((lex_cast(n1 + short(4)) == "5"));
        REQUIRE((lex_cast(short(4) + n2) == "2"));
        REQUIRE((lex_cast(n1 + 4) == "5"));
        REQUIRE((lex_cast(4 + n2) == "2"));
        REQUIRE((lex_cast(n1 + wchar_t{4}) == "5"));
        REQUIRE((lex_cast(wchar_t{4} + n2) == "2"));
        REQUIRE((std::is_same<decltype(n1 + 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 + n2), integer>::value));
        REQUIRE((lex_cast(n1 + 4u) == "5"));
        REQUIRE((lex_cast(4u + n2) == "2"));
        REQUIRE((n1 + 4.f == 5.f));
        REQUIRE((4.f + n2 == 2.f));
        REQUIRE((std::is_same<decltype(n1 + 4.f), float>::value));
        REQUIRE((std::is_same<decltype(4.f + n2), float>::value));
        REQUIRE((n1 + 4. == 5.));
        REQUIRE((4. + n2 == 2.));
        REQUIRE((std::is_same<decltype(n1 + 4.), double>::value));
        REQUIRE((std::is_same<decltype(4. + n2), double>::value));
#if defined(MPPP_WITH_MPFR)
        REQUIRE((n1 + 4.l == 5.l));
        REQUIRE((4.l + n2 == 2.l));
        REQUIRE((std::is_same<decltype(n1 + 4.l), long double>::value));
        REQUIRE((std::is_same<decltype(4.l + n2), long double>::value));
#endif
        REQUIRE(std::is_same<std::complex<float>, decltype(n1 + std::complex<float>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<float>, decltype(std::complex<float>{4, 0} + n1)>::value);
        REQUIRE(n1 + std::complex<float>{4, 0} == std::complex<float>{5, 0});
        REQUIRE(std::complex<float>{4, 0} + n1 == std::complex<float>{5, 0});

        REQUIRE(std::is_same<std::complex<double>, decltype(n1 + std::complex<double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(std::complex<double>{4, 0} + n1)>::value);
        REQUIRE(n1 + std::complex<double>{4, 0} == std::complex<double>{5, 0});
        REQUIRE(std::complex<double>{4, 0} + n1 == std::complex<double>{5, 0});

#if defined(MPPP_WITH_MPFR)
        REQUIRE(std::is_same<std::complex<long double>, decltype(n1 + std::complex<long double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<long double>, decltype(std::complex<long double>{4, 0} + n1)>::value);
        REQUIRE(n1 + std::complex<long double>{4, 0} == std::complex<long double>{5, 0});
        REQUIRE(std::complex<long double>{4, 0} + n1 == std::complex<long double>{5, 0});
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((n1 + __uint128_t{4} == 5));
        REQUIRE((__uint128_t{4} + n1 == 5));
        REQUIRE((n1 + __int128_t{-4} == -3));
        REQUIRE((__int128_t{-4} + n1 == -3));
        REQUIRE(n1 + detail::nl_max<__uint128_t>() == integer{detail::to_string(detail::nl_max<__uint128_t>())} + 1);
        REQUIRE(n1 + detail::nl_max<__int128_t>() == integer{detail::to_string(detail::nl_max<__int128_t>())} + 1);
        REQUIRE(-n1 + detail::nl_min<__int128_t>() == integer{detail::to_string(detail::nl_min<__int128_t>())} - 1);
#endif
        // In-place add.
        integer retval{1};
        retval += n1;
        REQUIRE((lex_cast(retval) == "2"));
        retval += 1;
        REQUIRE((lex_cast(retval) == "3"));
        retval += short(-1);
        REQUIRE((lex_cast(retval) == "2"));
        retval += static_cast<signed char>(-1);
        REQUIRE((lex_cast(retval) == "1"));
        retval += -5ll;
        REQUIRE((lex_cast(retval) == "-4"));
        retval += 20ull;
        REQUIRE((lex_cast(retval) == "16"));
        retval += 2.5f;
        REQUIRE((lex_cast(retval) == "18"));
        retval += -3.5;
        REQUIRE((lex_cast(retval) == "14"));
#if defined(MPPP_WITH_MPFR)
        retval += -1.5l;
        REQUIRE((lex_cast(retval) == "12"));
#endif
        retval = 12;
        retval += std::complex<float>{1, 0};
        REQUIRE(std::is_same<integer &, decltype(retval += std::complex<float>{1, 0})>::value);
        REQUIRE(retval == 13);
        REQUIRE_THROWS_PREDICATE((retval += std::complex<float>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(1.f) + " to an integer";
                                 });

        retval += std::complex<double>{1, 0};
        REQUIRE(std::is_same<integer &, decltype(retval += std::complex<double>{1, 0})>::value);
        REQUIRE(retval == 14);
        REQUIRE_THROWS_PREDICATE((retval += std::complex<double>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(1.) + " to an integer";
                                 });
#if defined(MPPP_WITH_MPFR)
        retval += std::complex<long double>{1, 0};
        REQUIRE(std::is_same<integer &, decltype(retval += std::complex<long double>{1, 0})>::value);
        REQUIRE(retval == 15);
        REQUIRE_THROWS_PREDICATE((retval += std::complex<long double>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(1.l) + " to an integer";
                                 });
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        retval = 12;
        retval += __uint128_t{6};
        REQUIRE((retval == 18));
        retval += __int128_t{-6};
        REQUIRE((retval == 12));
#endif
        if (std::numeric_limits<double>::is_iec559) {
            retval = 1;
            REQUIRE_THROWS_PREDICATE(
                retval += std::numeric_limits<double>::infinity(), std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot assign the non-finite floating-point value "
                                  + std::to_string(std::numeric_limits<double>::infinity()) + " to an integer";
                });
        }
        // In-place with interop on the lhs.
        short nl = 1;
        nl += integer{1};
        REQUIRE((std::is_same<short &, decltype(nl += integer{1})>::value));
        REQUIRE(nl == 2);
        nl += integer{-3};
        REQUIRE(nl == -1);
        unsigned long long unl = 1;
        unl += integer{1};
        REQUIRE(unl == 2);
        REQUIRE_THROWS_AS(unl += integer{-3}, std::overflow_error);
        REQUIRE_THROWS_AS(unl += integer{std::numeric_limits<unsigned long long>::max()}, std::overflow_error);
        double dl = 1.2;
        dl += integer{1};
        REQUIRE(dl == 1.2 + 1.);
        REQUIRE((std::is_same<double &, decltype(dl += integer{1})>::value));
        if (std::numeric_limits<double>::is_iec559) {
            dl = std::numeric_limits<double>::infinity();
            dl += integer{1};
            REQUIRE(dl == std::numeric_limits<double>::infinity());
        }
#if defined(MPPP_WITH_MPFR)
        long double ld = 4;
        ld += integer{1};
        REQUIRE(std::is_same<long double &, decltype(ld += integer{1})>::value);
        REQUIRE(ld == 5);
#endif
        std::complex<float> cf{1, 2};
        cf += integer{2};
        REQUIRE(std::is_same<std::complex<float> &, decltype(cf += integer{2})>::value);
        REQUIRE(cf == std::complex<float>{3, 2});

        std::complex<double> cd{1, 2};
        cd += integer{2};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd += integer{2})>::value);
        REQUIRE(cd == std::complex<double>{3, 2});
#if defined(MPPP_WITH_MPFR)
        std::complex<long double> cld{1, 2};
        cld += integer{2};
        REQUIRE(std::is_same<std::complex<long double> &, decltype(cld += integer{2})>::value);
        REQUIRE(cld == std::complex<long double>{3, 2});
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        __int128_t n128{-7};
        n128 += integer{5};
        REQUIRE((n128 == -2));
        __uint128_t un128{6};
        un128 += integer{5};
        REQUIRE((un128 == 11));
#endif
        // Increment ops.
        retval = integer{0};
        REQUIRE((lex_cast(++retval) == "1"));
        REQUIRE((lex_cast(++retval) == "2"));
        REQUIRE((std::is_same<decltype(++retval), integer &>::value));
        retval = integer{-2};
        ++retval;
        REQUIRE((lex_cast(retval) == "-1"));
        ++retval;
        REQUIRE((lex_cast(retval) == "0"));
        ++retval;
        REQUIRE((lex_cast(retval) == "1"));
        REQUIRE((lex_cast(retval++) == "1"));
        REQUIRE((lex_cast(retval++) == "2"));
        REQUIRE((lex_cast(retval++) == "3"));
        // Couple of tests at the boundaries
        detail::mpz_raii tmp;
        retval = integer{GMP_NUMB_MAX};
        mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        ++retval;
        mpz_add_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
        retval = integer{GMP_NUMB_MAX};
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        retval++;
        mpz_add_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
        retval = integer{GMP_NUMB_MAX};
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        retval++;
        mpz_add_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
        // Type traits.
        REQUIRE((!is_addable<integer, std::string>::value));
        REQUIRE((!is_addable<std::string, integer>::value));
        REQUIRE((!is_addable_inplace<integer, std::string>::value));
        REQUIRE((!is_addable_inplace<const integer, int>::value));
        REQUIRE((!is_addable_inplace<std::string, integer>::value));
        REQUIRE((!is_addable_inplace<const int, integer>::value));
        // In-place add with self.
        retval = -5;
        retval += retval;
        REQUIRE(retval == -10);
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
        using integer = integer<S::value>;
        integer n1{1}, n2{-2};
        REQUIRE((lex_cast(-n2) == "2"));
        REQUIRE((lex_cast(n1 - n2) == "3"));
        REQUIRE((std::is_same<decltype(n1 - n2), integer>::value));
        REQUIRE((lex_cast(n1 - char(4)) == "-3"));
        REQUIRE((lex_cast(char(4) - n2) == "6"));
        REQUIRE((std::is_same<decltype(n1 - char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) - n2), integer>::value));
        REQUIRE((lex_cast(n1 - static_cast<unsigned char>(4)) == "-3"));
        REQUIRE((lex_cast(static_cast<unsigned char>(4) - n2) == "6"));
        REQUIRE((lex_cast(n1 - short(4)) == "-3"));
        REQUIRE((lex_cast(short(4) - n2) == "6"));
        REQUIRE((lex_cast(n1 - 4) == "-3"));
        REQUIRE((lex_cast(4 - n2) == "6"));
        REQUIRE((lex_cast(n1 - wchar_t{4}) == "-3"));
        REQUIRE((lex_cast(wchar_t{4} - n2) == "6"));
        REQUIRE((std::is_same<decltype(n1 - 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 - n2), integer>::value));
        REQUIRE((lex_cast(n1 - 4u) == "-3"));
        REQUIRE((lex_cast(4u - n2) == "6"));
        REQUIRE((n1 - 4.f == -3.f));
        REQUIRE((4.f - n2 == 6.f));
        REQUIRE((std::is_same<decltype(n1 - 4.f), float>::value));
        REQUIRE((std::is_same<decltype(4.f - n2), float>::value));
        REQUIRE((n1 - 4. == -3.));
        REQUIRE((4. - n2 == 6.));
        REQUIRE((std::is_same<decltype(n1 - 4.), double>::value));
        REQUIRE((std::is_same<decltype(4. - n2), double>::value));
#if defined(MPPP_WITH_MPFR)
        REQUIRE((n1 - 4.l == -3.l));
        REQUIRE((4.l - n2 == 6.l));
        REQUIRE((std::is_same<decltype(n1 - 4.l), long double>::value));
        REQUIRE((std::is_same<decltype(4.l - n2), long double>::value));
#endif
        REQUIRE(std::is_same<std::complex<float>, decltype(n1 - std::complex<float>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<float>, decltype(std::complex<float>{4, 0} - n1)>::value);
        REQUIRE(n1 - std::complex<float>{4, 0} == std::complex<float>{-3, 0});
        REQUIRE(std::complex<float>{4, 0} - n1 == std::complex<float>{3, 0});

        REQUIRE(std::is_same<std::complex<double>, decltype(n1 - std::complex<double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(std::complex<double>{4, 0} - n1)>::value);
        REQUIRE(n1 - std::complex<double>{4, 0} == std::complex<double>{-3, 0});
        REQUIRE(std::complex<double>{4, 0} - n1 == std::complex<double>{3, 0});

#if defined(MPPP_WITH_MPFR)
        REQUIRE(std::is_same<std::complex<long double>, decltype(n1 - std::complex<long double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<long double>, decltype(std::complex<long double>{4, 0} - n1)>::value);
        REQUIRE(n1 - std::complex<long double>{4, 0} == std::complex<long double>{-3, 0});
        REQUIRE(std::complex<long double>{4, 0} - n1 == std::complex<long double>{3, 0});
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((n1 - __uint128_t{4} == -3));
        REQUIRE((__uint128_t{4} - n1 == 3));
        REQUIRE((n1 - __int128_t{-4} == 5));
        REQUIRE((__int128_t{-4} - n1 == -5));
        REQUIRE(-n1 - detail::nl_max<__uint128_t>() == -integer{detail::to_string(detail::nl_max<__uint128_t>())} - 1);
        REQUIRE(-n1 - detail::nl_max<__int128_t>() == -integer{detail::to_string(detail::nl_max<__int128_t>())} - 1);
        REQUIRE(-n1 - detail::nl_min<__int128_t>() == -integer{detail::to_string(detail::nl_min<__int128_t>())} - 1);
#endif
        // In-place sub.
        integer retval{1};
        retval -= n1;
        REQUIRE((lex_cast(retval) == "0"));
        retval -= 1;
        REQUIRE((lex_cast(retval) == "-1"));
        retval -= short(-1);
        REQUIRE((lex_cast(retval) == "0"));
        retval -= static_cast<signed char>(-1);
        REQUIRE((lex_cast(retval) == "1"));
        retval -= -5ll;
        REQUIRE((lex_cast(retval) == "6"));
        retval -= 20ull;
        REQUIRE((lex_cast(retval) == "-14"));
        retval -= 2.5f;
        REQUIRE((lex_cast(retval) == "-16"));
        retval -= -3.5;
        REQUIRE((lex_cast(retval) == "-12"));
#if defined(MPPP_WITH_MPFR)
        retval -= -1.5l;
        REQUIRE((lex_cast(retval) == "-10"));
#endif
        retval = 12;
        retval -= std::complex<float>{1, 0};
        REQUIRE(std::is_same<integer &, decltype(retval -= std::complex<float>{1, 0})>::value);
        REQUIRE(retval == 11);
        REQUIRE_THROWS_PREDICATE((retval -= std::complex<float>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(-1.f) + " to an integer";
                                 });

        retval -= std::complex<double>{1, 0};
        REQUIRE(std::is_same<integer &, decltype(retval -= std::complex<double>{1, 0})>::value);
        REQUIRE(retval == 10);
        REQUIRE_THROWS_PREDICATE((retval -= std::complex<double>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(-1.) + " to an integer";
                                 });

#if defined(MPPP_WITH_MPFR)
        retval -= std::complex<long double>{1, 0};
        REQUIRE(std::is_same<integer &, decltype(retval -= std::complex<long double>{1, 0})>::value);
        REQUIRE(retval == 9);
        REQUIRE_THROWS_PREDICATE((retval -= std::complex<long double>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(-1.l) + " to an integer";
                                 });
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        retval = -10;
        retval -= __uint128_t{6};
        REQUIRE((retval == -16));
        retval -= __int128_t{-6};
        REQUIRE((retval == -10));
#endif
        if (std::numeric_limits<double>::is_iec559) {
            retval = 1;
            REQUIRE_THROWS_PREDICATE(
                retval -= std::numeric_limits<double>::infinity(), std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot assign the non-finite floating-point value "
                                  + std::to_string(-std::numeric_limits<double>::infinity()) + " to an integer";
                });
        }
        // In-place with interop on the lhs.
        short nl = 1;
        nl -= integer{1};
        REQUIRE((std::is_same<short &, decltype(nl -= integer{1})>::value));
        REQUIRE(nl == 0);
        nl -= integer{-3};
        REQUIRE(nl == 3);
        unsigned long long unl = 1;
        unl -= integer{1};
        REQUIRE(unl == 0);
        REQUIRE_THROWS_AS(unl -= integer{1}, std::overflow_error);
        double dl = 1.2;
        dl -= integer{1};
        REQUIRE(dl == 1.2 - 1.);
        REQUIRE((std::is_same<double &, decltype(dl -= integer{1})>::value));
        if (std::numeric_limits<double>::is_iec559) {
            dl = std::numeric_limits<double>::infinity();
            dl -= integer{1};
            REQUIRE(dl == std::numeric_limits<double>::infinity());
        }
#if defined(MPPP_WITH_MPFR)
        long double ld = 4;
        ld -= integer{1};
        REQUIRE(std::is_same<long double &, decltype(ld -= integer{1})>::value);
        REQUIRE(ld == 3);
#endif
        std::complex<float> cf{1, 2};
        cf -= integer{2};
        REQUIRE(std::is_same<std::complex<float> &, decltype(cf -= integer{2})>::value);
        REQUIRE(cf == std::complex<float>{-1, 2});

        std::complex<double> cd{1, 2};
        cd -= integer{2};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd -= integer{2})>::value);
        REQUIRE(cd == std::complex<double>{-1, 2});
#if defined(MPPP_WITH_MPFR)
        std::complex<long double> cld{1, 2};
        cld -= integer{2};
        REQUIRE(std::is_same<std::complex<long double> &, decltype(cld -= integer{2})>::value);
        REQUIRE(cld == std::complex<long double>{-1, 2});
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        __int128_t n128{-7};
        n128 -= integer{5};
        REQUIRE((n128 == -12));
        __uint128_t un128{6};
        un128 -= integer{5};
        REQUIRE((un128 == 1));
#endif
        // Decrement ops.
        retval = integer{0};
        REQUIRE((lex_cast(--retval) == "-1"));
        REQUIRE((lex_cast(--retval) == "-2"));
        REQUIRE((std::is_same<decltype(--retval), integer &>::value));
        retval = integer{2};
        --retval;
        REQUIRE((lex_cast(retval) == "1"));
        --retval;
        REQUIRE((lex_cast(retval) == "0"));
        --retval;
        REQUIRE((lex_cast(retval) == "-1"));
        REQUIRE((lex_cast(retval--) == "-1"));
        REQUIRE((lex_cast(retval--) == "-2"));
        REQUIRE((lex_cast(retval--) == "-3"));
        // Couple of tests at the boundaries
        detail::mpz_raii tmp;
        retval = integer{GMP_NUMB_MAX};
        retval.neg();
        mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        --retval;
        mpz_sub_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
        retval = integer{GMP_NUMB_MAX};
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        retval.neg();
        mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        retval--;
        mpz_sub_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
        retval = integer{GMP_NUMB_MAX};
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        retval.neg();
        mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        retval--;
        mpz_sub_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
        // Type traits.
        REQUIRE((!is_subtractable<integer, std::string>::value));
        REQUIRE((!is_subtractable<std::string, integer>::value));
        REQUIRE((!is_subtractable_inplace<integer, std::string>::value));
        REQUIRE((!is_subtractable_inplace<const integer, int>::value));
        REQUIRE((!is_subtractable_inplace<std::string, integer>::value));
        REQUIRE((!is_subtractable_inplace<const int, integer>::value));
        // In-place sub with self.
        retval = -5;
        retval -= *&retval;
        REQUIRE(retval == 0);
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
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n1{1}, n2{-2};
        REQUIRE((lex_cast(n1 * n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 * n2), integer>::value));
        REQUIRE((lex_cast(n1 * char(4)) == "4"));
        REQUIRE((lex_cast(char(4) * n2) == "-8"));
        REQUIRE((std::is_same<decltype(n1 * char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) * n2), integer>::value));
        REQUIRE((lex_cast(n1 * static_cast<unsigned char>(4)) == "4"));
        REQUIRE((lex_cast(static_cast<unsigned char>(4) * n2) == "-8"));
        REQUIRE((lex_cast(n1 * short(4)) == "4"));
        REQUIRE((lex_cast(short(4) * n2) == "-8"));
        REQUIRE((lex_cast(n1 * 4) == "4"));
        REQUIRE((lex_cast(4 * n2) == "-8"));
        REQUIRE((lex_cast(n1 * wchar_t{4}) == "4"));
        REQUIRE((lex_cast(wchar_t{4} * n2) == "-8"));
        REQUIRE((std::is_same<decltype(n1 * 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 * n2), integer>::value));
        REQUIRE((lex_cast(n1 * 4u) == "4"));
        REQUIRE((lex_cast(4u * n2) == "-8"));
        REQUIRE((n1 * 4.f == 4.f));
        REQUIRE((4.f * n2 == -8.f));
        REQUIRE((std::is_same<decltype(n1 * 4.f), float>::value));
        REQUIRE((std::is_same<decltype(4.f * n2), float>::value));
        REQUIRE((n1 * 4. == 4.));
        REQUIRE((4. * n2 == -8.));
        REQUIRE((std::is_same<decltype(n1 * 4.), double>::value));
        REQUIRE((std::is_same<decltype(4. * n2), double>::value));
#if defined(MPPP_WITH_MPFR)
        REQUIRE((n1 * 4.l == 4.l));
        REQUIRE((4.l * n2 == -8.l));
        REQUIRE((std::is_same<decltype(n1 * 4.l), long double>::value));
        REQUIRE((std::is_same<decltype(4.l * n2), long double>::value));
#endif
        REQUIRE(std::is_same<std::complex<float>, decltype(n1 * std::complex<float>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<float>, decltype(std::complex<float>{4, 0} * n1)>::value);
        REQUIRE(n1 * std::complex<float>{4, 0} == std::complex<float>{4, 0});
        REQUIRE(std::complex<float>{4, 0} * n1 == std::complex<float>{4, 0});

        REQUIRE(std::is_same<std::complex<double>, decltype(n1 * std::complex<double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(std::complex<double>{4, 0} * n1)>::value);
        REQUIRE(n1 * std::complex<double>{4, 0} == std::complex<double>{4, 0});
        REQUIRE(std::complex<double>{4, 0} * n1 == std::complex<double>{4, 0});

#if defined(MPPP_WITH_MPFR)
        REQUIRE(std::is_same<std::complex<long double>, decltype(n1 * std::complex<long double>{4, 0})>::value);
        REQUIRE(std::is_same<std::complex<long double>, decltype(std::complex<long double>{4, 0} * n1)>::value);
        REQUIRE(n1 * std::complex<long double>{4, 0} == std::complex<long double>{4, 0});
        REQUIRE(std::complex<long double>{4, 0} * n1 == std::complex<long double>{4, 0});
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE((n1 * __uint128_t{4} == 4));
        REQUIRE((__uint128_t{4} * n1 == 4));
        REQUIRE((n1 * __int128_t{-4} == -4));
        REQUIRE((__int128_t{-4} * n1 == -4));
        REQUIRE(integer{2} * detail::nl_max<__uint128_t>()
                == 2 * integer{detail::to_string(detail::nl_max<__uint128_t>())});
        REQUIRE(integer{2} * detail::nl_max<__int128_t>()
                == 2 * integer{detail::to_string(detail::nl_max<__int128_t>())});
        REQUIRE(integer{2} * detail::nl_min<__int128_t>()
                == 2 * integer{detail::to_string(detail::nl_min<__int128_t>())});
#endif
        // In-place mul.
        integer retval{1};
        retval *= n1;
        REQUIRE((lex_cast(retval) == "1"));
        retval *= 1;
        REQUIRE((lex_cast(retval) == "1"));
        retval *= short(-1);
        REQUIRE((lex_cast(retval) == "-1"));
        retval *= static_cast<signed char>(-1);
        REQUIRE((lex_cast(retval) == "1"));
        retval *= -5ll;
        REQUIRE((lex_cast(retval) == "-5"));
        retval *= 20ull;
        REQUIRE((lex_cast(retval) == "-100"));
        retval *= 2.5f;
        REQUIRE((lex_cast(retval) == "-250"));
        retval *= -3.5;
        REQUIRE((lex_cast(retval) == "875"));
#if defined(MPPP_WITH_MPFR)
        retval *= -1.5l;
        REQUIRE((lex_cast(retval) == "-1312"));
#endif
        retval = 12;
        retval *= std::complex<float>{2, 0};
        REQUIRE(std::is_same<integer &, decltype(retval *= std::complex<float>{2, 0})>::value);
        REQUIRE(retval == 24);
        REQUIRE_THROWS_PREDICATE((retval *= std::complex<float>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(24.f) + " to an integer";
                                 });

        retval *= std::complex<double>{2, 0};
        REQUIRE(std::is_same<integer &, decltype(retval *= std::complex<double>{2, 0})>::value);
        REQUIRE(retval == 48);
        REQUIRE_THROWS_PREDICATE((retval *= std::complex<double>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(48.) + " to an integer";
                                 });
#if defined(MPPP_WITH_MPFR)
        retval *= std::complex<long double>{2, 0};
        REQUIRE(std::is_same<integer &, decltype(retval *= std::complex<long double>{2, 0})>::value);
        REQUIRE(retval == 96);
        REQUIRE_THROWS_PREDICATE((retval *= std::complex<double>{0, 1}), std::domain_error,
                                 [](const std::domain_error &ex) {
                                     return std::string(ex.what())
                                            == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                                   + detail::to_string(96.l) + " to an integer";
                                 });
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        retval = -1312;
        retval *= __uint128_t{2};
        REQUIRE((retval == -2624));
        retval *= __int128_t{-1};
        REQUIRE((retval == 2624));
#endif
        if (std::numeric_limits<double>::is_iec559) {
            retval = 1;
            REQUIRE_THROWS_PREDICATE(
                retval *= std::numeric_limits<double>::infinity(), std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot assign the non-finite floating-point value "
                                  + std::to_string(std::numeric_limits<double>::infinity()) + " to an integer";
                });
        }
        // In-place with interop on the lhs.
        short nl = 1;
        nl *= integer{3};
        REQUIRE((std::is_same<short &, decltype(nl *= integer{1})>::value));
        REQUIRE(nl == 3);
        nl *= integer{-3};
        REQUIRE(nl == -9);
        unsigned long long unl = 1;
        unl *= integer{2};
        REQUIRE(unl == 2);
        REQUIRE_THROWS_AS(unl *= integer{-1}, std::overflow_error);
        double dl = 1.2;
        dl *= integer{2};
        REQUIRE(dl == 1.2 * 2.);
        REQUIRE((std::is_same<double &, decltype(dl *= integer{1})>::value));
        if (std::numeric_limits<double>::is_iec559) {
            dl = std::numeric_limits<double>::infinity();
            dl *= integer{2};
            REQUIRE(dl == std::numeric_limits<double>::infinity());
        }
#if defined(MPPP_WITH_MPFR)
        long double ld = 4;
        ld *= integer{2};
        REQUIRE(std::is_same<long double &, decltype(ld *= integer{2})>::value);
        REQUIRE(ld == 8);
#endif
        std::complex<float> cf{1, 2};
        cf *= integer{2};
        REQUIRE(std::is_same<std::complex<float> &, decltype(cf *= integer{2})>::value);
        REQUIRE(cf == std::complex<float>{2, 4});

        std::complex<double> cd{1, 2};
        cd *= integer{2};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd *= integer{2})>::value);
        REQUIRE(cd == std::complex<double>{2, 4});
#if defined(MPPP_WITH_MPFR)
        std::complex<long double> cld{1, 2};
        cld *= integer{2};
        REQUIRE(std::is_same<std::complex<long double> &, decltype(cld *= integer{2})>::value);
        REQUIRE(cld == std::complex<long double>{2, 4});
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        __int128_t n128{-7};
        n128 *= integer{5};
        REQUIRE((n128 == -35));
        __uint128_t un128{6};
        un128 *= integer{5};
        REQUIRE((un128 == 30));
#endif
        // Type traits.
        REQUIRE((!is_multipliable<integer, std::string>::value));
        REQUIRE((!is_multipliable<std::string, integer>::value));
        REQUIRE((!is_multipliable_inplace<integer, std::string>::value));
        REQUIRE((!is_multipliable_inplace<const integer, int>::value));
        REQUIRE((!is_multipliable_inplace<std::string, integer>::value));
        REQUIRE((!is_multipliable_inplace<const int, integer>::value));
        // In-place mul with self.
        retval = -5;
        retval *= retval;
        REQUIRE(retval == 25);
    }
};

TEST_CASE("mul")
{
    tuple_for_each(sizes{}, mul_tester{});
}

#if defined(_MSC_VER)

#pragma warning(pop)

#endif
