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

#include <algorithm>
#include <complex>
#include <initializer_list>
#include <random>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(cert-err58-cpp, cert-msc32-c, cert-msc51-cpp, cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937 rng;

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

using int_t = integer<1>;
using rat_t = rational<1>;

template <typename T, typename U>
using equal_t = decltype(std::declval<const T &>() == std::declval<const U &>());

template <typename T, typename U>
using inequal_t = decltype(std::declval<const T &>() != std::declval<const U &>());

template <typename T, typename U>
using is_eq_comparable = std::integral_constant<bool, std::is_same<detail::detected_t<equal_t, T, U>, bool>::value>;

template <typename T, typename U>
using is_ineq_comparable = std::integral_constant<bool, std::is_same<detail::detected_t<inequal_t, T, U>, bool>::value>;

#if defined(__INTEL_COMPILER)
#define MPPP_INTEL_CONSTEXPR const
#else
#define MPPP_INTEL_CONSTEXPR constexpr
#endif

TEST_CASE("real128 equality")
{
    REQUIRE((std::is_same<decltype(real128{} == real128{}), bool>::value));
    REQUIRE((std::is_same<decltype(real128{} != real128{}), bool>::value));
    REQUIRE((is_eq_comparable<real128, real128>::value));
    REQUIRE((!is_eq_comparable<real128, std::string>::value));
    REQUIRE((!is_eq_comparable<std::string, real128>::value));
    REQUIRE((is_ineq_comparable<real128, real128>::value));
    REQUIRE((!is_ineq_comparable<real128, std::string>::value));
    REQUIRE((!is_ineq_comparable<std::string, real128>::value));
    constexpr bool c0 = real128{} == real128{};
    constexpr bool c0a = real128{} != real128{};
    MPPP_INTEL_CONSTEXPR bool c0b = real128_equal_to(real128{}, real128{});
    REQUIRE(c0);
    REQUIRE(!c0a);
    REQUIRE(c0b);
    constexpr bool c1 = real128{-1} == real128{1};
    constexpr bool c1a = real128{-1} != real128{1};
    MPPP_INTEL_CONSTEXPR bool c1b = real128_equal_to(real128{-1}, real128{1});
    REQUIRE(!c1);
    REQUIRE(c1a);
    REQUIRE(!c1b);
    constexpr bool c2 = real128{-1} == -1;
    constexpr bool c2a = real128{-1} != -1;
    constexpr bool c2b = real128{1} == wchar_t{1};
    constexpr bool c2c = real128{-1} != wchar_t{1};
    constexpr bool c2d = -1 == real128{-1};
    constexpr bool c2e = -1 != real128{-1};
    constexpr bool c2f = wchar_t{1} == real128{1};
    constexpr bool c2g = wchar_t{1} != real128{-1};
    REQUIRE(c2);
    REQUIRE(!c2a);
    REQUIRE(c2b);
    REQUIRE(c2c);
    REQUIRE(c2d);
    REQUIRE(!c2e);
    REQUIRE(c2f);
    REQUIRE(c2g);
    REQUIRE(!(1.23 == real128{-1}));
    REQUIRE(1.23 != real128{-1});
    constexpr bool c3 = 1.23 == real128{-1};
    constexpr bool c3a = 1.23 != real128{-1};
    REQUIRE(!c3);
    REQUIRE(c3a);
    REQUIRE(real128{10} == int_t{10});
    REQUIRE(int_t{10} == real128{10});
    REQUIRE(real128{10} != int_t{-10});
    REQUIRE(int_t{-10} != real128{10});
    REQUIRE((real128{"1.5"} == rat_t{3, 2}));
    REQUIRE((rat_t{3, 2} == real128{"1.5"}));
    REQUIRE((real128{"1.5"} != rat_t{3, 5}));
    REQUIRE((rat_t{-3, 2} != real128{"1.5"}));
    REQUIRE(real128_inf() == real128_inf());
    REQUIRE(-real128_inf() != real128_inf());
    REQUIRE(real128_nan() != real128_nan());
    REQUIRE(-real128_nan() != -real128_nan());
    REQUIRE(!(real128_nan() == real128_nan()));
    REQUIRE(!(-real128_nan() == -real128_nan()));
    REQUIRE(real128_equal_to(real128_inf(), real128_inf()));
    REQUIRE(!real128_equal_to(-real128_inf(), real128_inf()));
    REQUIRE(real128_equal_to(real128_nan(), real128_nan()));
    REQUIRE(real128_equal_to(real128_nan(), -real128_nan()));
    REQUIRE(real128_equal_to(-real128_nan(), real128_nan()));
    REQUIRE(!real128_equal_to(real128{-1}, real128_nan()));
    REQUIRE(!real128_equal_to(real128_nan(), real128{-1}));
#if defined(MPPP_HAVE_GCC_INT128)
    constexpr bool d1 = real128{1} == __int128_t{1};
    REQUIRE(d1);
    constexpr bool d2 = __int128_t{1} == real128{1};
    REQUIRE(d2);
    constexpr bool d3 = real128{1} == __uint128_t{1};
    REQUIRE(d3);
    constexpr bool d4 = __uint128_t{1} == real128{1};
    REQUIRE(d4);
    constexpr bool e1 = real128{2} != __int128_t{1};
    REQUIRE(e1);
    constexpr bool e2 = __int128_t{2} != real128{1};
    REQUIRE(e2);
    constexpr bool e3 = real128{2} != __uint128_t{1};
    REQUIRE(e3);
    constexpr bool e4 = __uint128_t{2} != real128{1};
    REQUIRE(e4);
#endif

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(real128{2} == 2.l);
    REQUIRE(2.l == real128{2});

    REQUIRE(real128{2} != 1.l);
    REQUIRE(1.l != real128{2});
#else
    REQUIRE(!is_eq_comparable<real128, long double>::value);
    REQUIRE(!is_eq_comparable<long double, real128>::value);

    REQUIRE(!is_ineq_comparable<real128, long double>::value);
    REQUIRE(!is_ineq_comparable<long double, real128>::value);
#endif

    // Comparisons with std::complex.
    REQUIRE(real128{42} == std::complex<float>{42, 0});
    REQUIRE(std::complex<float>{42, 0} == real128{42});
    REQUIRE(real128{43} != std::complex<float>{42, 0});
    REQUIRE(std::complex<float>{43, 0} != real128{42});
    REQUIRE(real128{43} != std::complex<float>{42, 1});
    REQUIRE(std::complex<float>{43, 1} != real128{42});

    REQUIRE(real128{42} == std::complex<double>{42, 0});
    REQUIRE(std::complex<double>{42, 0} == real128{42});
    REQUIRE(real128{43} != std::complex<double>{42, 0});
    REQUIRE(std::complex<double>{43, 0} != real128{42});
    REQUIRE(real128{43} != std::complex<double>{42, 1});
    REQUIRE(std::complex<double>{43, 1} != real128{42});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(real128{42} == std::complex<long double>{42, 0});
    REQUIRE(std::complex<long double>{42, 0} == real128{42});
    REQUIRE(real128{43} != std::complex<long double>{42, 0});
    REQUIRE(std::complex<long double>{42, 0} != real128{43});
    REQUIRE(real128{43} != std::complex<long double>{42, 1});
    REQUIRE(std::complex<long double>{42, 1} != real128{43});
#else
    REQUIRE(!is_eq_comparable<real128, std::complex<long double>>::value);
    REQUIRE(!is_eq_comparable<std::complex<long double>, real128>::value);

    REQUIRE(!is_ineq_comparable<real128, std::complex<long double>>::value);
    REQUIRE(!is_ineq_comparable<std::complex<long double>, real128>::value);
#endif

    // Test constexpr-ness.
    constexpr auto cc1 = real128{42} == std::complex<float>{42, 0};
    REQUIRE(cc1);
    constexpr auto cc2 = real128{42} != std::complex<float>{42, 1};
    REQUIRE(cc2);
}

template <typename T, typename U>
using lt_t = decltype(std::declval<const T &>() < std::declval<const U &>());

template <typename T, typename U>
using is_lt_comparable = std::integral_constant<bool, std::is_same<detail::detected_t<lt_t, T, U>, bool>::value>;

TEST_CASE("real128 lt")
{
    REQUIRE((std::is_same<decltype(real128{} < real128{}), bool>::value));
    REQUIRE((is_lt_comparable<real128, real128>::value));
    REQUIRE((!is_lt_comparable<real128, std::string>::value));
    REQUIRE((!is_lt_comparable<std::string, real128>::value));
    constexpr bool c0 = real128{} < real128{};
    MPPP_INTEL_CONSTEXPR bool c0b = real128_lt(real128{}, real128{});
    REQUIRE(!c0);
    REQUIRE(!c0b);
    constexpr bool c1 = real128{-1} < real128{1};
    MPPP_INTEL_CONSTEXPR bool c1b = real128_lt(real128{-1}, real128{1});
    REQUIRE(c1);
    REQUIRE(c1b);
    constexpr bool c2 = real128{1} < -1;
    constexpr bool c2a = real128{1} < -1;
    constexpr bool c2b = real128{0} < wchar_t{1};
    constexpr bool c2c = -1 < real128{1};
    constexpr bool c2d = wchar_t{1} < real128{};
    REQUIRE(!c2);
    REQUIRE(!c2a);
    REQUIRE(c2b);
    REQUIRE(c2c);
    REQUIRE(!c2d);
    constexpr bool c3 = 1.23 < real128{-1};
    constexpr bool c3a = 1.23 < real128{-1};
    REQUIRE(!c3);
    REQUIRE(!c3a);
    REQUIRE(!(real128{10} < int_t{10}));
    REQUIRE(!(int_t{10} < real128{10}));
    REQUIRE(!(real128{10} < int_t{-10}));
    REQUIRE(int_t{-10} < real128{10});
    REQUIRE(!(real128{"2"} < rat_t{3, 2}));
    REQUIRE((rat_t{3, 2} < real128{"2"}));
    REQUIRE(!(real128{"1.5"} < rat_t{3, 5}));
    REQUIRE((rat_t{-3, 2} < real128{"1.5"}));
    REQUIRE(!(real128_inf() < real128_inf()));
    REQUIRE(-real128_inf() < real128_inf());
    REQUIRE(!(real128_inf() < -real128_inf()));
    REQUIRE(!(real128_nan() < real128_nan()));
    REQUIRE(!(-real128_nan() < -real128_nan()));
    REQUIRE(!(real128_nan() < real128_nan()));
    REQUIRE(!(-real128_nan() < -real128_nan()));
    REQUIRE(!(3 < real128_nan()));
    REQUIRE(!(real128_nan() < 3));
    REQUIRE(!real128_lt(real128_inf(), real128_inf()));
    REQUIRE(real128_lt(-real128_inf(), real128_inf()));
    REQUIRE(!real128_lt(real128_nan(), real128_nan()));
    REQUIRE(!real128_lt(real128_nan(), -real128_nan()));
    REQUIRE(!real128_lt(-real128_nan(), real128_nan()));
    REQUIRE(real128_lt(-real128_inf(), real128_nan()));
    REQUIRE(real128_lt(real128{-1}, real128_nan()));
    REQUIRE(real128_lt(real128{100}, real128_nan()));
    REQUIRE(real128_lt(real128_inf(), real128_nan()));
    REQUIRE(!real128_lt(real128_nan(), -real128_inf()));
    REQUIRE(!real128_lt(real128_nan(), real128{-1}));
    REQUIRE(!real128_lt(real128_nan(), real128{100}));
    REQUIRE(!real128_lt(real128_nan(), real128_inf()));
#if defined(MPPP_HAVE_GCC_INT128)
    constexpr bool d1 = real128{1} < __int128_t{1};
    REQUIRE(!d1);
    constexpr bool d2 = __int128_t{1} < real128{1};
    REQUIRE(!d2);
    constexpr bool d3 = real128{1} < __uint128_t{1};
    REQUIRE(!d3);
    constexpr bool d4 = __uint128_t{1} < real128{1};
    REQUIRE(!d4);
#endif

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(real128{2} < 3.l);
    REQUIRE(3.l < real128{4});
#else
    REQUIRE(!is_lt_comparable<real128, long double>::value);
    REQUIRE(!is_lt_comparable<long double, real128>::value);
#endif
}

template <typename T, typename U>
using lte_t = decltype(std::declval<const T &>() <= std::declval<const U &>());

template <typename T, typename U>
using is_lte_comparable = std::integral_constant<bool, std::is_same<detail::detected_t<lte_t, T, U>, bool>::value>;

TEST_CASE("real128 lte")
{
    REQUIRE((std::is_same<decltype(real128{} <= real128{}), bool>::value));
    REQUIRE((is_lte_comparable<real128, real128>::value));
    REQUIRE((!is_lte_comparable<real128, std::string>::value));
    REQUIRE((!is_lte_comparable<std::string, real128>::value));
    constexpr bool c0 = real128{} <= real128{};
    REQUIRE(c0);
    constexpr bool c1 = real128{-1} <= real128{1};
    REQUIRE(c1);
    constexpr bool c2 = real128{1} <= -1;
    constexpr bool c2a = real128{1} <= wchar_t{1};
    REQUIRE(!c2);
    REQUIRE(c2a);
    constexpr bool c3 = 1.23 <= real128{-1};
    REQUIRE(!c3);
    REQUIRE(real128{10} <= int_t{10});
    REQUIRE(int_t{10} <= real128{10});
    REQUIRE(!(real128{10} <= int_t{-10}));
    REQUIRE(int_t{-10} <= real128{10});
    REQUIRE(!(real128{"2"} <= rat_t{3, 2}));
    REQUIRE((rat_t{3, 2} <= real128{"2"}));
    REQUIRE(!(real128{"1.5"} <= rat_t{3, 5}));
    REQUIRE((rat_t{-3, 2} <= real128{"1.5"}));
    REQUIRE(real128_inf() <= real128_inf());
    REQUIRE(-real128_inf() <= real128_inf());
    REQUIRE(!(real128_inf() <= -real128_inf()));
    REQUIRE(!(real128_nan() <= real128_nan()));
    REQUIRE(!(-real128_nan() <= -real128_nan()));
    REQUIRE(!(real128_nan() <= real128_nan()));
    REQUIRE(!(-real128_nan() <= -real128_nan()));
    REQUIRE(!(3 <= real128_nan()));
    REQUIRE(!(real128_nan() <= 3));
#if defined(MPPP_HAVE_GCC_INT128)
    constexpr bool d1 = real128{1} <= __int128_t{1};
    REQUIRE(d1);
    constexpr bool d2 = __int128_t{0} <= real128{1};
    REQUIRE(d2);
    constexpr bool d3 = real128{1} <= __uint128_t{1};
    REQUIRE(d3);
    constexpr bool d4 = __uint128_t{0} <= real128{1};
    REQUIRE(d4);
#endif

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(real128{2} <= 3.l);
    REQUIRE(3.l <= real128{3});
#else
    REQUIRE(!is_lte_comparable<real128, long double>::value);
    REQUIRE(!is_lte_comparable<long double, real128>::value);
#endif
}

template <typename T, typename U>
using gt_t = decltype(std::declval<const T &>() > std::declval<const U &>());

template <typename T, typename U>
using is_gt_comparable = std::integral_constant<bool, std::is_same<detail::detected_t<gt_t, T, U>, bool>::value>;

TEST_CASE("real128 gt")
{
    REQUIRE((std::is_same<decltype(real128{} > real128{}), bool>::value));
    REQUIRE((is_gt_comparable<real128, real128>::value));
    REQUIRE((!is_gt_comparable<real128, std::string>::value));
    REQUIRE((!is_gt_comparable<std::string, real128>::value));
    constexpr bool c0 = real128{} > real128{};
    MPPP_INTEL_CONSTEXPR bool c0b = real128_gt(real128{}, real128{});
    REQUIRE(!c0);
    REQUIRE(!c0b);
    constexpr bool c1 = real128{-1} > real128{1};
    MPPP_INTEL_CONSTEXPR bool c1b = real128_gt(real128{-1}, real128{1});
    REQUIRE(!c1);
    REQUIRE(!c1b);
    constexpr bool c2 = real128{1} > -1;
    constexpr bool c2a = real128{1} > -1;
    constexpr bool c2b = real128{1} > wchar_t{1};
    constexpr bool c2c = -1 > real128{1};
    constexpr bool c2d = wchar_t{1} > real128{1};
    REQUIRE(c2);
    REQUIRE(c2a);
    REQUIRE(!c2b);
    REQUIRE(!c2c);
    REQUIRE(!c2d);
    constexpr bool c3 = 1.23 > real128{-1};
    constexpr bool c3a = 1.23 > real128{-1};
    REQUIRE(c3);
    REQUIRE(c3a);
    REQUIRE(!(real128{10} > int_t{10}));
    REQUIRE(!(int_t{10} > real128{10}));
    REQUIRE(real128{10} > int_t{-10});
    REQUIRE(!(int_t{-10} > real128{10}));
    REQUIRE((real128{"2"} > rat_t{3, 2}));
    REQUIRE(!(rat_t{3, 2} > real128{"2"}));
    REQUIRE((real128{"1.5"} > rat_t{3, 5}));
    REQUIRE(!(rat_t{-3, 2} > real128{"1.5"}));
    REQUIRE(!(real128_inf() > real128_inf()));
    REQUIRE(!(-real128_inf() > real128_inf()));
    REQUIRE(real128_inf() > -real128_inf());
    REQUIRE(!(real128_nan() > real128_nan()));
    REQUIRE(!(-real128_nan() > -real128_nan()));
    REQUIRE(!(real128_nan() > real128_nan()));
    REQUIRE(!(-real128_nan() > -real128_nan()));
    REQUIRE(!(3 > real128_nan()));
    REQUIRE(!(real128_nan() > 3));
    REQUIRE(!real128_gt(real128_inf(), real128_inf()));
    REQUIRE(!real128_gt(-real128_inf(), real128_inf()));
    REQUIRE(!real128_gt(real128_nan(), real128_nan()));
    REQUIRE(!real128_gt(real128_nan(), -real128_nan()));
    REQUIRE(!real128_gt(-real128_nan(), real128_nan()));
    REQUIRE(!real128_gt(-real128_inf(), real128_nan()));
    REQUIRE(!real128_gt(real128{-1}, real128_nan()));
    REQUIRE(!real128_gt(real128{100}, real128_nan()));
    REQUIRE(!real128_gt(real128_inf(), real128_nan()));
    REQUIRE(real128_gt(real128_nan(), -real128_inf()));
    REQUIRE(real128_gt(real128_nan(), real128{-1}));
    REQUIRE(real128_gt(real128_nan(), real128{100}));
    REQUIRE(real128_gt(real128_nan(), real128_inf()));
#if defined(MPPP_HAVE_GCC_INT128)
    constexpr bool d1 = real128{1} > __int128_t{1};
    REQUIRE(!d1);
    constexpr bool d2 = __int128_t{1} > real128{1};
    REQUIRE(!d2);
    constexpr bool d3 = real128{1} > __uint128_t{1};
    REQUIRE(!d3);
    constexpr bool d4 = __uint128_t{1} > real128{1};
    REQUIRE(!d4);
#endif

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(real128{3} > 2.l);
    REQUIRE(4.l > real128{-3});
#else
    REQUIRE(!is_gt_comparable<real128, long double>::value);
    REQUIRE(!is_gt_comparable<long double, real128>::value);
#endif
}

template <typename T, typename U>
using gte_t = decltype(std::declval<const T &>() >= std::declval<const U &>());

template <typename T, typename U>
using is_gte_comparable = std::integral_constant<bool, std::is_same<detail::detected_t<gte_t, T, U>, bool>::value>;

TEST_CASE("real128 gte")
{
    REQUIRE((std::is_same<decltype(real128{} >= real128{}), bool>::value));
    REQUIRE((is_gte_comparable<real128, real128>::value));
    REQUIRE((!is_gte_comparable<real128, std::string>::value));
    REQUIRE((!is_gte_comparable<std::string, real128>::value));
    constexpr bool c0 = real128{} >= real128{};
    REQUIRE(c0);
    constexpr bool c1 = real128{-1} >= real128{1};
    REQUIRE(!c1);
    constexpr bool c2 = real128{1} >= -1;
    constexpr bool c2a = real128{1} >= wchar_t{1};
    REQUIRE(c2);
    REQUIRE(c2a);
    constexpr bool c3 = 1.23 >= real128{-1};
    REQUIRE(c3);
    REQUIRE(real128{10} >= int_t{10});
    REQUIRE(int_t{10} >= real128{10});
    REQUIRE(real128{10} >= int_t{-10});
    REQUIRE(!(int_t{-10} >= real128{10}));
    REQUIRE((real128{2} >= rat_t{3, 2}));
    REQUIRE(!(rat_t{3, 2} >= real128{"2"}));
    REQUIRE((real128{"1.5"} >= rat_t{3, 5}));
    REQUIRE(!(rat_t{-3, 2} >= real128{"1.5"}));
    REQUIRE(real128_inf() >= real128_inf());
    REQUIRE(!(-real128_inf() >= real128_inf()));
    REQUIRE(real128_inf() >= -real128_inf());
    REQUIRE(!(real128_nan() >= real128_nan()));
    REQUIRE(!(-real128_nan() >= -real128_nan()));
    REQUIRE(!(real128_nan() >= real128_nan()));
    REQUIRE(!(-real128_nan() >= -real128_nan()));
    REQUIRE(!(3 >= real128_nan()));
    REQUIRE(!(real128_nan() >= 3));
#if defined(MPPP_HAVE_GCC_INT128)
    constexpr bool d1 = real128{1} >= __int128_t{1};
    REQUIRE(d1);
    constexpr bool d2 = __int128_t{0} >= real128{1};
    REQUIRE(!d2);
    constexpr bool d3 = real128{1} >= __uint128_t{1};
    REQUIRE(d3);
    constexpr bool d4 = __uint128_t{2} >= real128{1};
    REQUIRE(d4);
#endif

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(real128{3} >= 2.l);
    REQUIRE(4.l >= real128{4});
#else
    REQUIRE(!is_gte_comparable<real128, long double>::value);
    REQUIRE(!is_gte_comparable<long double, real128>::value);
#endif
}

TEST_CASE("real128 sort")
{
    std::vector<real128> v0{real128{1}, real128{2}, real128{3}, real128{4}, real128{5}};
    std::shuffle(v0.begin(), v0.end(), rng);
    std::sort(v0.begin(), v0.end());
    REQUIRE((v0 == std::vector<real128>{real128{1}, real128{2}, real128{3}, real128{4}, real128{5}}));
    v0 = std::vector<real128>{real128{1}, real128{2}, real128{3}, real128_nan(), -real128_nan()};
    std::shuffle(v0.begin(), v0.end(), rng);
    std::sort(v0.begin(), v0.end(), mppp::real128_lt);
    REQUIRE(
        (std::vector<real128>(v0.begin(), v0.begin() + 3) == std::vector<real128>{real128{1}, real128{2}, real128{3}}));
    REQUIRE(v0[3].isnan());
    REQUIRE(v0[4].isnan());
    std::shuffle(v0.begin(), v0.end(), rng);
    std::sort(v0.begin(), v0.end(), mppp::real128_gt);
    REQUIRE((std::vector<real128>(v0.begin() + 2, v0.begin() + 5)
             == std::vector<real128>{real128{3}, real128{2}, real128{1}}));
    REQUIRE(v0[0].isnan());
    REQUIRE(v0[1].isnan());
    std::set<real128, bool (*)(const real128 &, const real128 &)> s0(mppp::real128_lt);
    REQUIRE(s0.emplace(10).second);
    REQUIRE(!s0.emplace(10).second);
    REQUIRE(s0.emplace(1).second);
    REQUIRE(s0.emplace(real128_nan()).second);
    REQUIRE(!s0.emplace(-real128_nan()).second);
    REQUIRE(s0.emplace(2).second);
    REQUIRE(s0.emplace(3).second);
    v0 = std::vector<real128>{real128{1}, real128{2}, real128{3}, real128{10}, real128_nan()};
    REQUIRE(std::equal(v0.begin(), v0.begin() + 4, s0.begin()));
    REQUIRE(s0.rbegin()->isnan());
}
