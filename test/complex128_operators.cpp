// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <complex>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <mp++/complex128.hpp>
#include <mp++/config.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>

#include "catch.hpp"

using namespace mppp;

// Some tests involving constexpr result in an ICE on GCC < 6.
#if (defined(_MSC_VER) || defined(__clang__) || __GNUC__ >= 6) && MPPP_CPLUSPLUS >= 201402L

#define MPPP_ENABLE_CONSTEXPR_TESTS

#endif

#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)

static constexpr complex128 test_constexpr_incr()
{
    complex128 retval;
    ++retval;
    retval++;
    return retval;
}

static constexpr complex128 test_constexpr_decr()
{
    complex128 retval;
    --retval;
    retval--;
    return retval;
}

static constexpr complex128 test_constexpr_ipa()
{
    // complex128.
    complex128 c{1, 2};
    c += complex128{3, 4};

    // complex128 on the left.
    c += 1;
    c += real128{4};
#if MPPP_CPLUSPLUS >= 202002L
    c += std::complex<float>{1, 2};
#endif

    // complex128 on the right.
    int n = 4;
    n += complex128{4};
    real128 r{4};
    r += complex128{4};
#if MPPP_CPLUSPLUS >= 202002L
    std::complex<double> cd{1, 2};
    cd += complex128{4, 5};
#endif

#if MPPP_CPLUSPLUS >= 202002L
    // real128 on the left.
    r += std::complex<double>{1};
    // c++ complex on the left.
    std::complex<double> cd2{4, 5};
    cd2 += real128{3};
#endif

    return complex128{1, 2};
}

static constexpr complex128 test_constexpr_ips()
{
    // complex128.
    complex128 c{1, 2};
    c -= complex128{3, 4};

    // complex128 on the left.
    c -= 1;
    c -= real128{4};
#if MPPP_CPLUSPLUS >= 202002L
    c -= std::complex<float>{1, 2};
#endif

    // complex128 on the right.
    int n = 4;
    n -= complex128{4};
    real128 r{4};
    r -= complex128{4};
#if MPPP_CPLUSPLUS >= 202002L
    std::complex<double> cd{1, 2};
    cd -= complex128{4, 5};
#endif

#if MPPP_CPLUSPLUS >= 202002L
    // real128 on the left.
    r -= std::complex<double>{1};
    // c++ complex on the left.
    std::complex<double> cd2{4, 5};
    cd2 -= real128{3};
#endif

    return complex128{1, 2};
}

static constexpr complex128 test_constexpr_ipm()
{
    // complex128.
    complex128 c{1, 2};
    c *= complex128{3, 4};

    // complex128 on the left.
    c *= 1;
    c *= real128{4};
#if MPPP_CPLUSPLUS >= 202002L
    c *= std::complex<float>{1, 2};
#endif

    // complex128 on the right.
    int n = 4;
    n *= complex128{4};
    real128 r{4};
    r *= complex128{4};
#if MPPP_CPLUSPLUS >= 202002L
    std::complex<double> cd{1, 2};
    cd *= complex128{4, 5};
#endif

#if MPPP_CPLUSPLUS >= 202002L
    // real128 on the left.
    r *= std::complex<double>{1};
    // c++ complex on the left.
    std::complex<double> cd2{4, 5};
    cd2 *= real128{3};
#endif

    return complex128{1, 2};
}

#if 0

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

#endif

template <typename T, typename U>
using add_t = decltype(std::declval<const T &>() + std::declval<const U &>());

template <typename T, typename U>
using ip_add_t = decltype(std::declval<T &>() += std::declval<const U &>());

template <typename T, typename U>
using sub_t = decltype(std::declval<const T &>() - std::declval<const U &>());

template <typename T, typename U>
using ip_sub_t = decltype(std::declval<T &>() -= std::declval<const U &>());

template <typename T, typename U>
using mul_t = decltype(std::declval<const T &>() * std::declval<const U &>());

template <typename T, typename U>
using ip_mul_t = decltype(std::declval<T &>() *= std::declval<const U &>());

TEST_CASE("identity")
{
    REQUIRE(+complex128{3, 4}.m_value == cplex128{3, 4});
    REQUIRE((+complex128{3, -0.}).imag().signbit());

    constexpr auto c = +complex128{3, 4};
    REQUIRE(c.m_value == cplex128{3, 4});
}

TEST_CASE("negation")
{
    REQUIRE(-complex128{3, 4}.m_value == cplex128{-3, -4});
    REQUIRE(!(-complex128{3, -0.}).imag().signbit());
    REQUIRE(!(-complex128{-0., 3}).real().signbit());
    REQUIRE((-complex128{3, 0.}).imag().signbit());
    REQUIRE((-complex128{0., 3}).real().signbit());

    constexpr auto c = -complex128{3, 4};
    REQUIRE(c.m_value == cplex128{-3, -4});
}

TEST_CASE("incdec")
{
    complex128 x{5, 6};
    REQUIRE(((++x).m_value == cplex128{6, 6}));
    REQUIRE(((x++).m_value == cplex128{6, 6}));
    REQUIRE(((x).m_value == cplex128{7, 6}));
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    constexpr auto z4 = test_constexpr_incr();
    REQUIRE((z4.m_value == 2));
#endif

    REQUIRE(((--x).m_value == cplex128{6, 6}));
    REQUIRE(((x--).m_value == cplex128{6, 6}));
    REQUIRE(((x).m_value == cplex128{5, 6}));
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    constexpr auto z6 = test_constexpr_decr();
    REQUIRE((z6.m_value == -2));
#endif
}

#if defined(__INTEL_COMPILER)

#define MPPP_LOCAL_CONSTEXPR
#define MPPP_LOCAL_CONSTEXPR_14

#else

#define MPPP_LOCAL_CONSTEXPR constexpr
#define MPPP_LOCAL_CONSTEXPR_14 MPPP_CONSTEXPR_14

#endif

TEST_CASE("binary_add")
{
    // complex128-complex128.
    const MPPP_LOCAL_CONSTEXPR auto res0 = complex128{1, 2} + complex128{3, 4};
    REQUIRE(std::is_same<decltype(res0), const complex128>::value);
    REQUIRE(res0 == complex128{4, 6});

    // complex128-real128.
    const MPPP_LOCAL_CONSTEXPR auto res1 = complex128{1, 2} + real128{3};
    REQUIRE(std::is_same<decltype(res1), const complex128>::value);
    REQUIRE(res1 == complex128{4, 2});

    const MPPP_LOCAL_CONSTEXPR auto res2 = real128{3} + complex128{1, 2};
    REQUIRE(std::is_same<decltype(res2), const complex128>::value);
    REQUIRE(res2 == complex128{4, 2});

    // complex128-C++ arithmetic.
    const MPPP_LOCAL_CONSTEXPR auto res3 = 3 + complex128{1, 2};
    REQUIRE(std::is_same<decltype(res3), const complex128>::value);
    REQUIRE(res3 == complex128{4, 2});

    const MPPP_LOCAL_CONSTEXPR auto res4 = complex128{1, 2} + 3.f;
    REQUIRE(std::is_same<decltype(res4), const complex128>::value);
    REQUIRE(res4 == complex128{4, 2});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    const MPPP_LOCAL_CONSTEXPR auto res4a = complex128{1, 2} + 3.l;
    REQUIRE(std::is_same<decltype(res4a), const complex128>::value);
    REQUIRE(res4a == complex128{4, 2});
#else
    REQUIRE(!detail::is_detected<add_t, complex128, long double>::value);
    REQUIRE(!detail::is_detected<add_t, long double, complex128>::value);
#endif

    // complex128-mp++ types.
    const auto res5 = complex128{1, 2} + 3_z1;
    REQUIRE(std::is_same<decltype(res5), const complex128>::value);
    REQUIRE(res5 == complex128{4, 2});

    const auto res6 = 3_q1 + complex128{1, 2};
    REQUIRE(std::is_same<decltype(res6), const complex128>::value);
    REQUIRE(res6 == complex128{4, 2});

    // complex128-c++ complex.
    const MPPP_LOCAL_CONSTEXPR_14 auto res7 = complex128{1, 2} + std::complex<float>{3, 4};
    REQUIRE(std::is_same<decltype(res7), const complex128>::value);
    REQUIRE(res7 == complex128{4, 6});

    const MPPP_LOCAL_CONSTEXPR_14 auto res8 = std::complex<double>{3, 4} + complex128{1, 2};
    REQUIRE(std::is_same<decltype(res8), const complex128>::value);
    REQUIRE(res8 == complex128{4, 6});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    const MPPP_LOCAL_CONSTEXPR_14 auto res8a = std::complex<long double>{3, 4} + complex128{1, 2};
    REQUIRE(std::is_same<decltype(res8a), const complex128>::value);
    REQUIRE(res8a == complex128{4, 6});
#else
    REQUIRE(!detail::is_detected<add_t, complex128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<add_t, std::complex<long double>, complex128>::value);
#endif

    // real128-c++ complex.
    const MPPP_LOCAL_CONSTEXPR_14 auto res9 = std::complex<float>{1, 2} + real128{3};
    REQUIRE(std::is_same<decltype(res9), const complex128>::value);
    REQUIRE(res9 == complex128{4, 2});

    const MPPP_LOCAL_CONSTEXPR_14 auto res10 = real128{3} + std::complex<double>{1, 2};
    REQUIRE(std::is_same<decltype(res10), const complex128>::value);
    REQUIRE(res10 == complex128{4, 2});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    const MPPP_LOCAL_CONSTEXPR_14 auto res10a = real128{3} + std::complex<long double>{1, 2};
    REQUIRE(std::is_same<decltype(res10a), const complex128>::value);
    REQUIRE(res10a == complex128{4, 2});
#else
    REQUIRE(!detail::is_detected<add_t, real128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<add_t, std::complex<long double>, real128>::value);
#endif
}

TEST_CASE("in_place_add")
{
    complex128 c0{1, 2};
    c0 += complex128{-3, 4};
    REQUIRE(c0 == complex128{-2, 6});

    // With real128.
    c0 += real128{4};
    REQUIRE(c0 == complex128{2, 6});

    real128 r0{12};
    r0 += complex128{4};
    REQUIRE(r0 == 16);
    REQUIRE_THROWS_AS((r0 += complex128{4, 5}), std::domain_error);

    // With C++ arithmetic types.
    c0 += 4;
    REQUIRE(c0 == complex128{6, 6});
    c0 += -7.f;
    REQUIRE(c0 == complex128{-1, 6});

    auto n0 = 7ll;
    n0 += complex128{-2};
    REQUIRE(n0 == 5);
    REQUIRE_THROWS_AS((n0 += complex128{4, 5}), std::domain_error);
    auto x0 = 6.;
    x0 += complex128{1};
    REQUIRE(x0 == 7);
    REQUIRE_THROWS_AS((x0 += complex128{4, 5}), std::domain_error);

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    c0 += 6.l;
    REQUIRE(c0 == complex128{5, 6});
    // Reset c0.
    c0 = complex128{-1, 6};

    auto xl0 = 7.l;
    xl0 += complex128{3};
    REQUIRE(xl0 == 10);
    REQUIRE_THROWS_AS((xl0 += complex128{4, 5}), std::domain_error);
#else
    REQUIRE(!detail::is_detected<ip_add_t, complex128, long double>::value);
    REQUIRE(!detail::is_detected<ip_add_t, long double, complex128>::value);
#endif

    // With integer and rational.
    c0 += 6_z1;
    REQUIRE(c0 == complex128{5, 6});
    auto z0 = 123_z1;
    z0 += complex128{10};
    REQUIRE(z0 == 133);
    REQUIRE_THROWS_AS((z0 += complex128{4, 5}), std::domain_error);

    c0 += 4_q1;
    REQUIRE(c0 == complex128{9, 6});
    auto q0 = 10_q1;
    q0 += complex128{1};
    REQUIRE(q0 == 11);
    REQUIRE_THROWS_AS((q0 += complex128{4, 5}), std::domain_error);

    // C++ complex.
    c0 += std::complex<float>{1, 2};
    REQUIRE(c0 == complex128{10, 8});
    auto c1 = std::complex<double>{3, 4};
    c1 += complex128{-5, -7};
    REQUIRE(c1 == std::complex<double>{-2, -3});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    c0 += std::complex<long double>{1, 2};
    REQUIRE(c0 == complex128{11, 10});
    // Reset c0.
    c0 = complex128{10, 8};
    auto c2 = std::complex<long double>{3, 4};
    c2 += complex128{3, 6};
    REQUIRE(c2 == std::complex<long double>{6, 10});
#else
    REQUIRE(!detail::is_detected<ip_add_t, complex128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<ip_add_t, std::complex<long double>, complex128>::value);
#endif

    // real128-C++ complex.
    r0 = 10;
    r0 += std::complex<float>{6, 0};
    REQUIRE(r0 == 16);
    REQUIRE_THROWS_AS((r0 += complex128{4, 5}), std::domain_error);
    c1 = std::complex<double>{4, 5};
    c1 += real128{-9};
    REQUIRE(c1 == std::complex<double>{-5, 5});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    r0 += std::complex<long double>{-3, 0};
    REQUIRE(r0 == 13);
    r0 = 16;
    c2 = std::complex<long double>{4, 1};
    c2 += real128{6};
    REQUIRE(c2 == std::complex<long double>{10, 1});
#else
    REQUIRE(!detail::is_detected<ip_add_t, real128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<ip_add_t, std::complex<long double>, real128>::value);
#endif

#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    // Test constexprness.
    constexpr auto tc = test_constexpr_ipa();
    REQUIRE(tc == complex128{1, 2});
#endif
}

TEST_CASE("binary_sub")
{
    // complex128-complex128.
    const MPPP_LOCAL_CONSTEXPR auto res0 = complex128{1, 2} - complex128{3, 4};
    REQUIRE(std::is_same<decltype(res0), const complex128>::value);
    REQUIRE(res0 == complex128{-2, -2});

    // complex128-real128.
    const MPPP_LOCAL_CONSTEXPR auto res1 = complex128{1, 2} - real128{3};
    REQUIRE(std::is_same<decltype(res1), const complex128>::value);
    REQUIRE(res1 == complex128{-2, 2});

    const MPPP_LOCAL_CONSTEXPR auto res2 = real128{3} - complex128{1, 2};
    REQUIRE(std::is_same<decltype(res2), const complex128>::value);
    REQUIRE(res2 == complex128{2, -2});

    // complex128-C++ arithmetic.
    const MPPP_LOCAL_CONSTEXPR auto res3 = 3 - complex128{1, 2};
    REQUIRE(std::is_same<decltype(res3), const complex128>::value);
    REQUIRE(res3 == complex128{2, -2});

    const MPPP_LOCAL_CONSTEXPR auto res4 = complex128{1, 2} - 3.f;
    REQUIRE(std::is_same<decltype(res4), const complex128>::value);
    REQUIRE(res4 == complex128{-2, 2});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    const MPPP_LOCAL_CONSTEXPR auto res4a = complex128{1, 2} - 3.l;
    REQUIRE(std::is_same<decltype(res4a), const complex128>::value);
    REQUIRE(res4a == complex128{-2, 2});
#else
    REQUIRE(!detail::is_detected<sub_t, complex128, long double>::value);
    REQUIRE(!detail::is_detected<sub_t, long double, complex128>::value);
#endif

    // complex128-mp++ types.
    const auto res5 = complex128{1, 2} - 3_z1;
    REQUIRE(std::is_same<decltype(res5), const complex128>::value);
    REQUIRE(res5 == complex128{-2, 2});

    const auto res6 = 3_q1 - complex128{1, 2};
    REQUIRE(std::is_same<decltype(res6), const complex128>::value);
    REQUIRE(res6 == complex128{2, -2});

    // complex128-c++ complex.
    const MPPP_LOCAL_CONSTEXPR_14 auto res7 = complex128{1, 2} - std::complex<float>{3, 4};
    REQUIRE(std::is_same<decltype(res7), const complex128>::value);
    REQUIRE(res7 == complex128{-2, -2});

    const MPPP_LOCAL_CONSTEXPR_14 auto res8 = std::complex<double>{3, 4} - complex128{1, 2};
    REQUIRE(std::is_same<decltype(res8), const complex128>::value);
    REQUIRE(res8 == complex128{2, 2});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    const MPPP_LOCAL_CONSTEXPR_14 auto res8a = std::complex<long double>{3, 4} - complex128{1, 2};
    REQUIRE(std::is_same<decltype(res8a), const complex128>::value);
    REQUIRE(res8a == complex128{2, 2});
#else
    REQUIRE(!detail::is_detected<sub_t, complex128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<sub_t, std::complex<long double>, complex128>::value);
#endif

    // real128-c++ complex.
    const MPPP_LOCAL_CONSTEXPR_14 auto res9 = std::complex<float>{1, 2} - real128{3};
    REQUIRE(std::is_same<decltype(res9), const complex128>::value);
    REQUIRE(res9 == complex128{-2, 2});

    const MPPP_LOCAL_CONSTEXPR_14 auto res10 = real128{3} - std::complex<double>{1, 2};
    REQUIRE(std::is_same<decltype(res10), const complex128>::value);
    REQUIRE(res10 == complex128{2, -2});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    const MPPP_LOCAL_CONSTEXPR_14 auto res10a = real128{3} - std::complex<long double>{1, 2};
    REQUIRE(std::is_same<decltype(res10a), const complex128>::value);
    REQUIRE(res10a == complex128{2, -2});
#else
    REQUIRE(!detail::is_detected<sub_t, real128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<sub_t, std::complex<long double>, real128>::value);
#endif
}

TEST_CASE("in_place_sub")
{
    complex128 c0{1, 2};
    c0 -= complex128{-3, 4};
    REQUIRE(c0 == complex128{4, -2});

    // With real128.
    c0 -= real128{4};
    REQUIRE(c0 == complex128{0, -2});

    real128 r0{12};
    r0 -= complex128{4};
    REQUIRE(r0 == 8);
    REQUIRE_THROWS_AS((r0 -= complex128{4, 5}), std::domain_error);

    // With C++ arithmetic types.
    c0 -= 4;
    REQUIRE(c0 == complex128{-4, -2});
    c0 -= -7.f;
    REQUIRE(c0 == complex128{3, -2});

    auto n0 = 7ll;
    n0 -= complex128{-2};
    REQUIRE(n0 == 9);
    REQUIRE_THROWS_AS((n0 -= complex128{4, 5}), std::domain_error);
    auto x0 = 6.;
    x0 -= complex128{1};
    REQUIRE(x0 == 5);
    REQUIRE_THROWS_AS((x0 -= complex128{4, 5}), std::domain_error);

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    c0 -= 6.l;
    REQUIRE(c0 == complex128{-3, -2});
    // Reset c0.
    c0 = complex128{3, -2};

    auto xl0 = 7.l;
    xl0 -= complex128{3};
    REQUIRE(xl0 == 4);
    REQUIRE_THROWS_AS((xl0 -= complex128{4, 5}), std::domain_error);
#else
    REQUIRE(!detail::is_detected<ip_sub_t, complex128, long double>::value);
    REQUIRE(!detail::is_detected<ip_sub_t, long double, complex128>::value);
#endif

    // With integer and rational.
    c0 -= 6_z1;
    REQUIRE(c0 == complex128{-3, -2});
    auto z0 = 123_z1;
    z0 -= complex128{10};
    REQUIRE(z0 == 113);
    REQUIRE_THROWS_AS((z0 -= complex128{4, 5}), std::domain_error);

    c0 -= 4_q1;
    REQUIRE(c0 == complex128{-7, -2});
    auto q0 = 10_q1;
    q0 -= complex128{1};
    REQUIRE(q0 == 9);
    REQUIRE_THROWS_AS((q0 -= complex128{4, 5}), std::domain_error);

    // C++ complex.
    c0 -= std::complex<float>{1, 2};
    REQUIRE(c0 == complex128{-8, -4});
    auto c1 = std::complex<double>{3, 4};
    c1 -= complex128{-5, -7};
    REQUIRE(c1 == std::complex<double>{8, 11});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    c0 -= std::complex<long double>{1, 2};
    REQUIRE(c0 == complex128{-9, -6});
    // Reset c0.
    c0 = complex128{-8, -4};
    auto c2 = std::complex<long double>{3, 4};
    c2 -= complex128{3, 6};
    REQUIRE(c2 == std::complex<long double>{0, -2});
#else
    REQUIRE(!detail::is_detected<ip_sub_t, complex128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<ip_sub_t, std::complex<long double>, complex128>::value);
#endif

    // real128-C++ complex.
    r0 = 10;
    r0 -= std::complex<float>{6, 0};
    REQUIRE(r0 == 4);
    REQUIRE_THROWS_AS((r0 -= complex128{4, 5}), std::domain_error);
    c1 = std::complex<double>{4, 5};
    c1 -= real128{-9};
    REQUIRE(c1 == std::complex<double>{13, 5});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    r0 -= std::complex<long double>{-3, 0};
    REQUIRE(r0 == 7);
    r0 = 16;
    c2 = std::complex<long double>{4, 1};
    c2 -= real128{6};
    REQUIRE(c2 == std::complex<long double>{-2, 1});
#else
    REQUIRE(!detail::is_detected<ip_sub_t, real128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<ip_sub_t, std::complex<long double>, real128>::value);
#endif

#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    // Test constexprness.
    constexpr auto tc = test_constexpr_ips();
    REQUIRE(tc == complex128{1, 2});
#endif
}

TEST_CASE("binary_mul")
{
    // complex128-complex128.
    const MPPP_LOCAL_CONSTEXPR auto res0 = complex128{1, 2} * complex128{3, 4};
    REQUIRE(std::is_same<decltype(res0), const complex128>::value);
    REQUIRE(res0 == complex128{-5, 10});

    // complex128-real128.
    const MPPP_LOCAL_CONSTEXPR auto res1 = complex128{1, 2} * real128{3};
    REQUIRE(std::is_same<decltype(res1), const complex128>::value);
    REQUIRE(res1 == complex128{3, 6});

    const MPPP_LOCAL_CONSTEXPR auto res2 = real128{3} * complex128{1, 2};
    REQUIRE(std::is_same<decltype(res2), const complex128>::value);
    REQUIRE(res2 == complex128{3, 6});

    // complex128-C++ arithmetic.
    const MPPP_LOCAL_CONSTEXPR auto res3 = 3 * complex128{1, 2};
    REQUIRE(std::is_same<decltype(res3), const complex128>::value);
    REQUIRE(res3 == complex128{3, 6});

    const MPPP_LOCAL_CONSTEXPR auto res4 = complex128{1, 2} * 3.f;
    REQUIRE(std::is_same<decltype(res4), const complex128>::value);
    REQUIRE(res4 == complex128{3, 6});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    const MPPP_LOCAL_CONSTEXPR auto res4a = complex128{1, 2} * 3.l;
    REQUIRE(std::is_same<decltype(res4a), const complex128>::value);
    REQUIRE(res4a == complex128{3, 6});
#else
    REQUIRE(!detail::is_detected<mul_t, complex128, long double>::value);
    REQUIRE(!detail::is_detected<mul_t, long double, complex128>::value);
#endif

    // complex128-mp++ types.
    const auto res5 = complex128{1, 2} * 3_z1;
    REQUIRE(std::is_same<decltype(res5), const complex128>::value);
    REQUIRE(res5 == complex128{3, 6});

    const auto res6 = 3_q1 * complex128{1, 2};
    REQUIRE(std::is_same<decltype(res6), const complex128>::value);
    REQUIRE(res6 == complex128{3, 6});

    // complex128-c++ complex.
    const MPPP_LOCAL_CONSTEXPR_14 auto res7 = complex128{1, 2} * std::complex<float>{3, 4};
    REQUIRE(std::is_same<decltype(res7), const complex128>::value);
    REQUIRE(res7 == complex128{-5, 10});

    const MPPP_LOCAL_CONSTEXPR_14 auto res8 = std::complex<double>{3, 4} * complex128{1, 2};
    REQUIRE(std::is_same<decltype(res8), const complex128>::value);
    REQUIRE(res8 == complex128{-5, 10});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    const MPPP_LOCAL_CONSTEXPR_14 auto res8a = std::complex<long double>{3, 4} * complex128{1, 2};
    REQUIRE(std::is_same<decltype(res8a), const complex128>::value);
    REQUIRE(res8a == complex128{-5, 10});
#else
    REQUIRE(!detail::is_detected<mul_t, complex128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<mul_t, std::complex<long double>, complex128>::value);
#endif

    // real128-c++ complex.
    const MPPP_LOCAL_CONSTEXPR_14 auto res9 = std::complex<float>{1, 2} * real128{3};
    REQUIRE(std::is_same<decltype(res9), const complex128>::value);
    REQUIRE(res9 == complex128{3, 6});

    const MPPP_LOCAL_CONSTEXPR_14 auto res10 = real128{3} * std::complex<double>{1, 2};
    REQUIRE(std::is_same<decltype(res10), const complex128>::value);
    REQUIRE(res10 == complex128{3, 6});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    const MPPP_LOCAL_CONSTEXPR_14 auto res10a = real128{3} * std::complex<long double>{1, 2};
    REQUIRE(std::is_same<decltype(res10a), const complex128>::value);
    REQUIRE(res10a == complex128{3, 6});
#else
    REQUIRE(!detail::is_detected<mul_t, real128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<mul_t, std::complex<long double>, real128>::value);
#endif
}

TEST_CASE("in_place_mul")
{
    complex128 c0{1, 2};
    c0 *= complex128{-3, 4};
    REQUIRE(c0 == complex128{-11, -2});

    // With real128.
    c0 *= real128{4};
    REQUIRE(c0 == complex128{-44, -8});

    real128 r0{12};
    r0 *= complex128{4};
    REQUIRE(r0 == 48);
    REQUIRE_THROWS_AS((r0 *= complex128{4, 5}), std::domain_error);

    // With C++ arithmetic types.
    c0 *= 4;
    REQUIRE(c0 == complex128{-176, -32});
    c0 *= -7.f;
    REQUIRE(c0 == complex128{1232, 224});

    auto n0 = 7ll;
    n0 *= complex128{-2};
    REQUIRE(n0 == -14);
    REQUIRE_THROWS_AS((n0 *= complex128{4, 5}), std::domain_error);
    auto x0 = 6.;
    x0 *= complex128{2};
    REQUIRE(x0 == 12);
    REQUIRE_THROWS_AS((x0 *= complex128{4, 5}), std::domain_error);

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    c0 *= 6.l;
    REQUIRE(c0 == complex128{7392, 1344});
    // Reset c0.
    c0 = complex128{1232, 224};

    auto xl0 = 7.l;
    xl0 *= complex128{3};
    REQUIRE(xl0 == 21);
    REQUIRE_THROWS_AS((xl0 *= complex128{4, 5}), std::domain_error);
#else
    REQUIRE(!detail::is_detected<ip_mul_t, complex128, long double>::value);
    REQUIRE(!detail::is_detected<ip_mul_t, long double, complex128>::value);
#endif

    // With integer and rational.
    c0 *= 6_z1;
    REQUIRE(c0 == complex128{7392, 1344});
    auto z0 = 123_z1;
    z0 *= complex128{10};
    REQUIRE(z0 == 1230);
    REQUIRE_THROWS_AS((z0 *= complex128{4, 5}), std::domain_error);

    c0 *= 4_q1;
    REQUIRE(c0 == complex128{29568l, 5376l});
    auto q0 = 10_q1;
    q0 *= complex128{2};
    REQUIRE(q0 == 20);
    REQUIRE_THROWS_AS((q0 *= complex128{4, 5}), std::domain_error);

    // C++ complex.
    c0 *= std::complex<float>{1, 2};
    REQUIRE(c0 == complex128{18816l, 64512l});
    auto c1 = std::complex<double>{3, 4};
    c1 *= complex128{-5, -7};
    REQUIRE(c1 == std::complex<double>{13, -41});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    c0 *= std::complex<long double>{1, -2};
    REQUIRE(c0 == complex128{147840l, 26880l});
    // Reset c0.
    c0 = complex128{29568l, 5376l};
    auto c2 = std::complex<long double>{3, 4};
    c2 *= complex128{3, 6};
    REQUIRE(c2 == std::complex<long double>{-15, 30});
#else
    REQUIRE(!detail::is_detected<ip_mul_t, complex128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<ip_mul_t, std::complex<long double>, complex128>::value);
#endif

    // real128-C++ complex.
    r0 = 10;
    r0 *= std::complex<float>{6, 0};
    REQUIRE(r0 == 60);
    REQUIRE_THROWS_AS((r0 *= complex128{4, 5}), std::domain_error);
    c1 = std::complex<double>{4, 5};
    c1 *= real128{-9};
    REQUIRE(c1 == std::complex<double>{-36, -45});

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    r0 *= std::complex<long double>{-3, 0};
    REQUIRE(r0 == -180);
    r0 = 16;
    c2 = std::complex<long double>{4, 1};
    c2 *= real128{6};
    REQUIRE(c2 == std::complex<long double>{24, 6});
#else
    REQUIRE(!detail::is_detected<ip_mul_t, real128, std::complex<long double>>::value);
    REQUIRE(!detail::is_detected<ip_mul_t, std::complex<long double>, real128>::value);
#endif

#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    // Test constexprness.
    constexpr auto tc = test_constexpr_ipm();
    REQUIRE(tc == complex128{1, 2});
#endif
}
