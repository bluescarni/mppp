// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <complex>
#include <type_traits>
#include <utility>

#include <mp++/complex128.hpp>
#include <mp++/config.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

template <typename T, typename U>
using pow_t = decltype(mppp::pow(std::declval<const T &>(), std::declval<const U &>()));

TEST_CASE("pow")
{
    const complex128 cmp1{"(-1.8608933068808647973591319614158252,11.8367671067643704469371227607012796)"};
    const complex128 cmp2{"(-237,-3116)"};
    const complex128 cmp3{"(123.509798243644950752715439342573358,19.2439532792542041720472840371835795)"};

    // Complex-complex.
    REQUIRE(std::is_same<complex128, decltype(mppp::pow(complex128{3, 4}, complex128{5, 6}))>::value);
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, complex128{5, 6}).m_value - cmp1.m_value}) < 1E-32);

    // Complex-interoperable.
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, 5).m_value - cmp2.m_value}) < 1E-29);
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, __int128_t(5)).m_value - cmp2.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, __uint128_t(5)).m_value - cmp2.m_value}) < 1E-29);
#endif
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, 5.).m_value - cmp2.m_value}) < 1E-29);
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, 5.l).m_value - cmp2.m_value}) < 1E-29);
#else
    REQUIRE(!detail::is_detected<pow_t, complex128, long double>::value);
#endif
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, 5_z1).m_value - cmp2.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, 5_q1).m_value - cmp2.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, 5_rq).m_value - cmp2.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, std::complex<float>{5, 6}).m_value - cmp1.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, std::complex<double>{5, 6}).m_value - cmp1.m_value}) < 1E-29);
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(abs(complex128{mppp::pow(complex128{3, 4}, std::complex<long double>{5, 6}).m_value - cmp1.m_value})
            < 1E-29);
#else
    REQUIRE(!detail::is_detected<pow_t, complex128, std::complex<long double>>::value);
#endif

    // Interoperable-complex.
    REQUIRE(abs(complex128{mppp::pow(5, complex128{3, 4}).m_value - cmp3.m_value}) < 1E-29);
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(abs(complex128{mppp::pow(__int128_t(5), complex128{3, 4}).m_value - cmp3.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(__uint128_t(5), complex128{3, 4}).m_value - cmp3.m_value}) < 1E-29);
#endif
    REQUIRE(abs(complex128{mppp::pow(5., complex128{3, 4}).m_value - cmp3.m_value}) < 1E-29);
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(abs(complex128{mppp::pow(5.l, complex128{3, 4}).m_value - cmp3.m_value}) < 1E-29);
#else
    REQUIRE(!detail::is_detected<pow_t, long double, complex128>::value);
#endif
    REQUIRE(abs(complex128{mppp::pow(5_z1, complex128{3, 4}).m_value - cmp3.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(5_q1, complex128{3, 4}).m_value - cmp3.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(5_rq, complex128{3, 4}).m_value - cmp3.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(std::complex<float>{3, 4}, complex128{5, 6}).m_value - cmp1.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(std::complex<double>{3, 4}, complex128{5, 6}).m_value - cmp1.m_value}) < 1E-29);
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(abs(complex128{mppp::pow(std::complex<long double>{3, 4}, complex128{5, 6}).m_value - cmp1.m_value})
            < 1E-29);
#else
    REQUIRE(!detail::is_detected<pow_t, std::complex<long double>, complex128>::value);
#endif

    // real128-cppcomplex and vice-versa.
    REQUIRE(std::is_same<complex128, decltype(mppp::pow(5_rq, std::complex<float>{3, 4}))>::value);
    REQUIRE(std::is_same<complex128, decltype(mppp::pow(std::complex<float>{3, 4}, 5_rq))>::value);
    REQUIRE(abs(complex128{mppp::pow(5_rq, std::complex<float>{3, 4}).m_value - cmp3.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(std::complex<float>{3, 4}, 5_rq).m_value - cmp2.m_value}) < 1E-29);

    REQUIRE(std::is_same<complex128, decltype(mppp::pow(5_rq, std::complex<double>{3, 4}))>::value);
    REQUIRE(std::is_same<complex128, decltype(mppp::pow(std::complex<double>{3, 4}, 5_rq))>::value);
    REQUIRE(abs(complex128{mppp::pow(5_rq, std::complex<double>{3, 4}).m_value - cmp3.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(std::complex<double>{3, 4}, 5_rq).m_value - cmp2.m_value}) < 1E-29);

#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(std::is_same<complex128, decltype(mppp::pow(5_rq, std::complex<long double>{3, 4}))>::value);
    REQUIRE(std::is_same<complex128, decltype(mppp::pow(std::complex<long double>{3, 4}, 5_rq))>::value);
    REQUIRE(abs(complex128{mppp::pow(5_rq, std::complex<long double>{3, 4}).m_value - cmp3.m_value}) < 1E-29);
    REQUIRE(abs(complex128{mppp::pow(std::complex<long double>{3, 4}, 5_rq).m_value - cmp2.m_value}) < 1E-29);
#else
    REQUIRE(!detail::is_detected<pow_t, std::complex<long double>, real128>::value);
    REQUIRE(!detail::is_detected<pow_t, real128, std::complex<long double>>::value);
#endif
}
