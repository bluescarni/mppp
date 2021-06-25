// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#if defined(__clang__) || defined(__GNUC__)

#pragma GCC diagnostic ignored "-Wconversion"

#endif

#include <type_traits>
#include <utility>

#include <mp++/config.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

using int_t = integer<1>;
using rat_t = rational<1>;

// Some tests involving constexpr result in an ICE on GCC < 6.
#if (defined(_MSC_VER) || defined(__clang__) || __GNUC__ >= 6) && MPPP_CPLUSPLUS >= 201402L

#define MPPP_ENABLE_CONSTEXPR_TESTS

#endif

#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)

static constexpr real128 test_constexpr_incr()
{
    real128 retval;
    ++retval;
    retval++;
    return retval;
}

static constexpr real128 test_constexpr_decr()
{
    real128 retval;
    --retval;
    retval--;
    return retval;
}

static constexpr real128 test_constexpr_ipa()
{
    real128 retval{1};
    retval += real128{-2};
    retval += 1.;
    retval += -1;
    int n = 3;
    n += real128{-2};
    return retval + n;
}

static constexpr real128 test_constexpr_ips()
{
    real128 retval{1};
    retval -= real128{-2};
    retval -= 1.;
    retval -= -1;
    int n = 3;
    n -= real128{-2};
    return retval + n;
}

static constexpr real128 test_constexpr_ipm()
{
    real128 retval{1};
    retval *= real128{-2};
    retval *= 2.;
    retval *= -1;
    int n = 3;
    n *= real128{-2};
    return retval * n;
}

static constexpr real128 test_constexpr_ipd()
{
    real128 retval{12};
    retval /= real128{-2};
    retval /= 3.;
    retval /= -2;
    int n = 6;
    n /= real128{-2};
    return n / retval;
}

#endif

template <typename T, typename U>
using binary_add_t = decltype(std::declval<const T &>() + std::declval<const U &>());

template <typename T, typename U>
using in_place_add_t = decltype(std::declval<T &>() += std::declval<const U &>());

template <typename T, typename U>
using binary_sub_t = decltype(std::declval<const T &>() - std::declval<const U &>());

template <typename T, typename U>
using in_place_sub_t = decltype(std::declval<T &>() -= std::declval<const U &>());

template <typename T, typename U>
using binary_mul_t = decltype(std::declval<const T &>() * std::declval<const U &>());

template <typename T, typename U>
using in_place_mul_t = decltype(std::declval<T &>() *= std::declval<const U &>());

template <typename T, typename U>
using binary_div_t = decltype(std::declval<const T &>() / std::declval<const U &>());

template <typename T, typename U>
using in_place_div_t = decltype(std::declval<T &>() /= std::declval<const U &>());

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real128 ops")
{
    real128 x;
    REQUIRE((std::is_same<decltype(+x), real128>::value));
    REQUIRE((std::is_same<decltype(real128{56} + real128{3}), real128>::value));
    REQUIRE((std::is_same<decltype(x + 3), real128>::value));
    REQUIRE((std::is_same<decltype(3. + x), real128>::value));
    REQUIRE((std::is_same<decltype(x + int_t{3}), real128>::value));
    REQUIRE((std::is_same<decltype(int_t{3} + x), real128>::value));
    REQUIRE((std::is_same<decltype(-x), real128>::value));
    REQUIRE((std::is_same<decltype(real128{56} - real128{3}), real128>::value));
    REQUIRE((std::is_same<decltype(x - 3), real128>::value));
    REQUIRE((std::is_same<decltype(3. - x), real128>::value));
    REQUIRE((std::is_same<decltype(x - int_t{3}), real128>::value));
    REQUIRE((std::is_same<decltype(int_t{3} - x), real128>::value));
    REQUIRE((std::is_same<decltype(real128{56} * real128{3}), real128>::value));
    REQUIRE((std::is_same<decltype(x * 3), real128>::value));
    REQUIRE((std::is_same<decltype(3. * x), real128>::value));
    REQUIRE((std::is_same<decltype(x * int_t{3}), real128>::value));
    REQUIRE((std::is_same<decltype(int_t{3} * x), real128>::value));
    REQUIRE((std::is_same<decltype(real128{56} / real128{3}), real128>::value));
    REQUIRE((std::is_same<decltype(x / 3), real128>::value));
    REQUIRE((std::is_same<decltype(3. / x), real128>::value));
    REQUIRE((std::is_same<decltype(x / int_t{3}), real128>::value));
    REQUIRE((std::is_same<decltype(int_t{3} / x), real128>::value));
    REQUIRE(((+x).m_value == 0));
    x = -145;
    REQUIRE(((+x).m_value == -145));
    real128 y{12};
    x = -5;
    REQUIRE(((x + y).m_value == 7));
    constexpr auto z1 = real128{56} + real128{3};
    constexpr auto z1a = +z1;
    REQUIRE((z1.m_value == 59));
    REQUIRE((z1a.m_value == 59));
    REQUIRE(((x + 3).m_value == -2));
    REQUIRE(((x + wchar_t{3}).m_value == -2));
    REQUIRE(((x + 2.).m_value == -3));
    REQUIRE(((3 + x).m_value == -2));
    REQUIRE(((wchar_t{3} + x).m_value == -2));
    REQUIRE(((2. + x).m_value == -3));
    constexpr auto z2 = real128{56} + 3;
    REQUIRE((z2.m_value == 59));
    constexpr auto z3 = 3.f + real128{56};
    REQUIRE((z3.m_value == 59));
    REQUIRE(((x + int_t{3}).m_value == -2));
    REQUIRE(((int_t{3} + x).m_value == -2));
    REQUIRE(((x + rat_t{3, 2}).m_value == real128{"-3.5"}.m_value));
    REQUIRE(((rat_t{3, 2} + x).m_value == real128{"-3.5"}.m_value));
    x = 5;
    REQUIRE(((++x).m_value == 6));
    REQUIRE(((x++).m_value == 6));
    REQUIRE(((x).m_value == 7));
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    constexpr auto z4 = test_constexpr_incr();
    REQUIRE((z4.m_value == 2));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    constexpr auto q1 = real128{1} + __int128_t{2};
    REQUIRE(q1 == 3);
    constexpr auto q2 = __int128_t{2} + real128{1};
    REQUIRE(q2 == 3);
    constexpr auto q3 = real128{1} + __uint128_t{2};
    REQUIRE(q3 == 3);
    constexpr auto q4 = __uint128_t{2} + real128{1};
    REQUIRE(q4 == 3);
#endif
    REQUIRE(((-real128{}).m_value == 0));
    REQUIRE((-real128{}).signbit());
    REQUIRE(((-real128{123}).m_value == -123));
    REQUIRE(((-real128{-123}).m_value == 123));
    constexpr auto z5 = -real128{-45};
    REQUIRE(((x - 3).m_value == 4));
    REQUIRE(((x - wchar_t{3}).m_value == 4));
    REQUIRE(((x - 2.).m_value == 5));
    REQUIRE(((3 - x).m_value == -4));
    REQUIRE(((wchar_t{3} - x).m_value == -4));
    REQUIRE(((2. - x).m_value == -5));
    constexpr auto z5a = real128{56} - 3;
    REQUIRE((z5a.m_value == 53));
    constexpr auto z5b = 3.f - real128{56};
    REQUIRE((z5b.m_value == -53));
    REQUIRE(((x - int_t{3}).m_value == 4));
    REQUIRE(((int_t{3} - x).m_value == -4));
    REQUIRE(((x - rat_t{3, 2}).m_value == real128{"5.5"}.m_value));
    REQUIRE(((rat_t{3, 2} - x).m_value == real128{"-5.5"}.m_value));
    REQUIRE((z5.m_value == 45));
    REQUIRE(((--x).m_value == 6));
    REQUIRE(((x--).m_value == 6));
    REQUIRE(((x).m_value == 5));
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    constexpr auto z6 = test_constexpr_decr();
    REQUIRE((z6.m_value == -2));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    constexpr auto s1 = real128{1} - __int128_t{2};
    REQUIRE(s1 == -1);
    constexpr auto s2 = __int128_t{2} - real128{1};
    REQUIRE(s2 == 1);
    constexpr auto s3 = real128{1} - __uint128_t{2};
    REQUIRE(s3 == -1);
    constexpr auto s4 = __uint128_t{2} - real128{1};
    REQUIRE(s4 == 1);
#endif
    REQUIRE(((x * 3).m_value == 15));
    REQUIRE(((x * wchar_t{3}).m_value == 15));
    REQUIRE(((x * 2.).m_value == 10));
    REQUIRE(((-3 * x).m_value == -15));
    REQUIRE(((wchar_t{3} * x).m_value == 15));
    REQUIRE(((2. * x).m_value == 10));
    constexpr auto z7 = real128{56} * 3;
    REQUIRE((z7.m_value == 168));
    constexpr auto z8 = 3.f * -real128{56};
    REQUIRE((z8.m_value == -168));
    REQUIRE(((x * int_t{3}).m_value == 15));
    REQUIRE(((int_t{3} * -x).m_value == -15));
    REQUIRE(((x * rat_t{3, 2}).m_value == real128{"7.5"}.m_value));
    REQUIRE(((rat_t{3, 2} * x).m_value == real128{"7.5"}.m_value));
#if defined(MPPP_HAVE_GCC_INT128)
    constexpr auto t1 = real128{1} * __int128_t{2};
    REQUIRE(t1 == 2);
    constexpr auto t2 = __int128_t{2} * real128{1};
    REQUIRE(t2 == 2);
    constexpr auto t3 = real128{1} * __uint128_t{2};
    REQUIRE(t3 == 2);
    constexpr auto t4 = __uint128_t{2} * real128{1};
    REQUIRE(t4 == 2);
#endif
    x = 12;
    REQUIRE(((x / 3).m_value == 4));
    REQUIRE(((x / wchar_t{3}).m_value == 4));
    REQUIRE(((x / 2.).m_value == 6));
    REQUIRE(((-6 / x).m_value == real128{"-.5"}.m_value));
    REQUIRE(((3. / x).m_value == real128{".25"}.m_value));
    REQUIRE(((wchar_t{3} / x).m_value == real128{".25"}.m_value));
    constexpr auto z9 = real128{56} / 2;
    REQUIRE((z9.m_value == 28));
    constexpr auto z10 = 3.f / -real128{12};
    REQUIRE((z10.m_value == -real128{".25"}.m_value));
    REQUIRE(((x / int_t{3}).m_value == 4));
    REQUIRE(((int_t{3} / -x).m_value == -real128{".25"}.m_value));
    REQUIRE(((x / rat_t{3, 2}).m_value == 8));
    REQUIRE(((rat_t{3, 2} / x).m_value == real128{".125"}.m_value));
#if defined(MPPP_HAVE_GCC_INT128)
    constexpr auto u1 = real128{4} / __int128_t{2};
    REQUIRE(u1 == 2);
    constexpr auto u2 = __int128_t{2} / real128{1};
    REQUIRE(u2 == 2);
    constexpr auto u3 = real128{4} / __uint128_t{2};
    REQUIRE(u3 == 2);
    constexpr auto u4 = __uint128_t{2} / real128{1};
    REQUIRE(u4 == 2);
#endif
    // In-place.
    x = -1;
    x += real128{-2};
    REQUIRE(x == -3);
    x += 2;
    REQUIRE(x == -1);
    x += -1.;
    REQUIRE(x == -2);
    int n = 5;
    n += real128{-3};
    REQUIRE(n == 2);
    double d = -6;
    d += real128{1};
    REQUIRE(d == -5.);
    x = 10;
    x += int_t{1};
    REQUIRE(x == 11);
    int_t nm{-12};
    nm += real128{2};
    REQUIRE(nm == -10);
    x += rat_t{3};
    REQUIRE(x == 14);
    rat_t q{5, 2};
    q += real128{-1.5};
    REQUIRE(q == 1);
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    constexpr real128 z11 = test_constexpr_ipa();
    REQUIRE(z11 == 0);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    {
        real128 z11a;
        z11a += __int128_t{5};
        REQUIRE(z11a == 5);
        z11a += __uint128_t{5};
        REQUIRE(z11a == 10);
        __int128_t n128 = 0;
        n128 += real128{4};
        REQUIRE(n128 == 4);
        __uint128_t un128 = 0;
        un128 += real128{4};
        REQUIRE(un128 == 4);
    }
#endif
    x = -1;
    x -= real128{-2};
    REQUIRE(x == 1);
    x -= 2;
    REQUIRE(x == -1);
    x -= -1.;
    REQUIRE(x == 0);
    n = 5;
    n -= real128{-3};
    REQUIRE(n == 8);
    d = -6;
    d -= real128{1};
    REQUIRE(d == -7.);
    x = 10;
    x -= int_t{1};
    REQUIRE(x == 9);
    nm = -12;
    nm -= real128{2};
    REQUIRE(nm == -14);
    x -= rat_t{3};
    REQUIRE(x == 6);
    q = rat_t{5, 2};
    q -= real128{-1.5};
    REQUIRE(q == 4);
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    constexpr real128 z12 = test_constexpr_ips();
    REQUIRE(z12 == 8);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    {
        real128 z11a;
        z11a -= __int128_t{5};
        REQUIRE(z11a == -5);
        z11a -= __uint128_t{5};
        REQUIRE(z11a == -10);
        __int128_t n128 = 0;
        n128 -= real128{4};
        REQUIRE(n128 == -4);
        __uint128_t un128 = 6;
        un128 -= real128{4};
        REQUIRE(un128 == 2);
    }
#endif
    x = -1;
    x *= real128{-2};
    REQUIRE(x == 2);
    x *= 2;
    REQUIRE(x == 4);
    x *= -1.;
    REQUIRE(x == -4);
    n = 5;
    n *= real128{-3};
    REQUIRE(n == -15);
    d = -6;
    d *= real128{2};
    REQUIRE(d == -12.);
    x = 10;
    x *= int_t{2};
    REQUIRE(x == 20);
    nm = -12;
    nm *= real128{2};
    REQUIRE(nm == -24);
    x *= rat_t{3};
    REQUIRE(x == 60);
    q = rat_t{5, 2};
    q *= real128{-2};
    REQUIRE(q == -5);
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    constexpr real128 z13 = test_constexpr_ipm();
    REQUIRE(z13 == -24);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    {
        real128 z11a{1};
        z11a *= __int128_t{5};
        REQUIRE(z11a == 5);
        z11a *= __uint128_t{5};
        REQUIRE(z11a == 25);
        __int128_t n128 = 1;
        n128 *= real128{4};
        REQUIRE(n128 == 4);
        __uint128_t un128 = 1;
        un128 *= real128{4};
        REQUIRE(un128 == 4);
    }
#endif
    x = 12;
    x /= real128{-2};
    REQUIRE(x == -6);
    x /= -3;
    REQUIRE(x == 2);
    x /= -1.;
    REQUIRE(x == -2);
    n = 36;
    n /= real128{-3};
    REQUIRE(n == -12);
    d = -6;
    d /= real128{2};
    REQUIRE(d == -3);
    x = 10;
    x /= int_t{2};
    REQUIRE(x == 5);
    nm = -12;
    nm /= real128{2};
    REQUIRE(nm == -6);
    x /= rat_t{5};
    REQUIRE(x == 1);
    q = rat_t{5, 2};
    q /= real128{-2};
    REQUIRE((q == rat_t{5, -4}));
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    constexpr real128 z14 = test_constexpr_ipd();
    REQUIRE(z14 == -3);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    {
        real128 z11a{20};
        z11a /= __int128_t{5};
        REQUIRE(z11a == 4);
        z11a /= __uint128_t{2};
        REQUIRE(z11a == 2);
        __int128_t n128 = 6;
        n128 /= real128{2};
        REQUIRE(n128 == 3);
        __uint128_t un128 = 8;
        un128 /= real128{4};
        REQUIRE(un128 == 2);
    }
#endif

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    x = -5;

    REQUIRE((std::is_same<decltype(3.l + x), real128>::value));
    REQUIRE((std::is_same<decltype(x + 3.l), real128>::value));
    REQUIRE(((x + 2.l) == -3));
    REQUIRE(((2.l + x) == -3));
    {
        real128 tmp_x{1};
        auto xld = 5.l;
        REQUIRE((tmp_x += 1.l) == 2);
        REQUIRE((xld += tmp_x) == 7);
    }

    REQUIRE((std::is_same<decltype(3.l - x), real128>::value));
    REQUIRE((std::is_same<decltype(x - 3.l), real128>::value));
    REQUIRE(((x - 2.l) == -7));
    REQUIRE(((2.l - x) == 7));
    {
        real128 tmp_x{1};
        auto xld = 5.l;
        REQUIRE((tmp_x -= 2.l) == -1);
        REQUIRE((xld -= tmp_x) == 6);
    }

    REQUIRE((std::is_same<decltype(3.l * x), real128>::value));
    REQUIRE((std::is_same<decltype(x * 3.l), real128>::value));
    REQUIRE(((x * 2.l) == -10));
    REQUIRE(((2.l * x) == -10));
    {
        real128 tmp_x{1};
        auto xld = 5.l;
        REQUIRE((tmp_x *= 2.l) == 2);
        REQUIRE((xld *= tmp_x) == 10);
    }

    REQUIRE((std::is_same<decltype(3.l / x), real128>::value));
    REQUIRE((std::is_same<decltype(x / 3.l), real128>::value));
    REQUIRE(((x / 2.l) == real128{-5} / 2));
    REQUIRE(((2.l / x) == real128{2} / -5));
    {
        real128 tmp_x{4};
        auto xld = 6.l;
        REQUIRE((tmp_x /= 2.l) == 2);
        REQUIRE((xld /= tmp_x) == 3);
    }
#else
    REQUIRE(!detail::is_detected<binary_add_t, real128, long double>::value);
    REQUIRE(!detail::is_detected<binary_add_t, long double, real128>::value);
    REQUIRE(!detail::is_detected<in_place_add_t, real128, long double>::value);
    REQUIRE(!detail::is_detected<in_place_add_t, long double, real128>::value);

    REQUIRE(!detail::is_detected<binary_sub_t, real128, long double>::value);
    REQUIRE(!detail::is_detected<binary_sub_t, long double, real128>::value);
    REQUIRE(!detail::is_detected<in_place_sub_t, real128, long double>::value);
    REQUIRE(!detail::is_detected<in_place_sub_t, long double, real128>::value);

    REQUIRE(!detail::is_detected<binary_mul_t, real128, long double>::value);
    REQUIRE(!detail::is_detected<binary_mul_t, long double, real128>::value);
    REQUIRE(!detail::is_detected<in_place_mul_t, real128, long double>::value);
    REQUIRE(!detail::is_detected<in_place_mul_t, long double, real128>::value);

    REQUIRE(!detail::is_detected<binary_div_t, real128, long double>::value);
    REQUIRE(!detail::is_detected<binary_div_t, long double, real128>::value);
    REQUIRE(!detail::is_detected<in_place_div_t, real128, long double>::value);
    REQUIRE(!detail::is_detected<in_place_div_t, long double, real128>::value);
#endif
}
