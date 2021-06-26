// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <complex>
#include <limits>
#include <type_traits>
#include <utility>

#include <mp++/complex.hpp>
#include <mp++/config.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>

#if defined(MPPP_WITH_QUADMATH)

#include <mp++/complex128.hpp>
#include <mp++/real128.hpp>

#endif

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("identity")
{
    complex r1{4, 5};
    REQUIRE(+r1 == r1);

    // Check stealing.
    const auto p = r1.get_prec();
    auto r2 = +std::move(r1);
    REQUIRE(r2.get_prec() == p);
    REQUIRE(r2 == complex{4, 5});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r1.is_valid());
}

TEST_CASE("increment")
{
    complex r0{0};
    REQUIRE(++r0 == 1);
    REQUIRE(r0++ == 1);
    REQUIRE(r0 == 2);

    // Check precision handling.
    r0 = complex{0, complex_prec_t(4)};
    ++r0;
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(1));
    REQUIRE(r0 == 1);
    r0 = complex{0, complex_prec_t(4)};
    r0++;
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(1));
    REQUIRE(r0 == 1);
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("binary plus")
{
    // complex-complex.
    {
        complex r1{4, 5}, r2{-4, 7};
        const auto p = r1.get_prec();
        auto ret = r1 + r2;
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == r1.get_prec());

        // Test moves.
        ret = std::move(r1) + r2;
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r1.is_valid());

        r1 = complex{4, 5};
        ret = r1 + std::move(r2);
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r2.is_valid());

        r2 = complex{-4, 7};
        ret = std::move(r1) + std::move(r2);
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE((!r1.is_valid() || !r2.is_valid()));

        // Self add.
        r2 = complex{-4, 6};
        REQUIRE(r2 + r2 == complex{-8, 12});
    }
    // complex-real.
    {
        complex c1{45, 6, complex_prec_t(128)};
        real r1{23, 10};
        auto ret = c1 + r1;
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        ret = r1 + c1;
        REQUIRE(std::is_same<complex, decltype(r1 + c1)>::value);
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);

        // Try with higher precision on the non-complex argument.
        ret = c1 + real{23, 256};
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 256);
        ret = real{23, 256} + c1;
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 256);

        // Try with moves.
        auto c2 = c1;
        ret = complex{};
        ret = std::move(c1) + r1;
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = complex{};
        ret = r1 + std::move(c1);
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
    // complex-rv interoperable.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 + 6;
        REQUIRE(ret == complex{51, 6});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6)));
        ret = 6. + c1;
        REQUIRE(ret == complex{51, 6});
        REQUIRE(std::is_same<complex, decltype(6. + c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 + 45_z1;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45_z1));
        ret = 45_q1 + c1;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45_q1));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) + 45;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45) + 1)};
        ret = std::move(c1) + 45;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = 45. + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45.) + 1)};
        ret = 45. + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

#if defined(MPPP_WITH_QUADMATH)
        c1 = c2;
        ret = 45_rq + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = 45_rq + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
#endif
    }
    // complex-unsigned integral.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 + 6u;
        REQUIRE(ret == complex{51, 6});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6u)));
        ret = 6u + c1;
        REQUIRE(ret == complex{51, 6});
        REQUIRE(std::is_same<complex, decltype(6u + c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6u)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 + 45u;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        ret = 45u + c1;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) + 45u;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45u) + 1)};
        ret = std::move(c1) + 45u;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = 45u + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45u) + 1)};
        ret = 45u + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        // Bool special casing.
        c1 = c2;
        ret = true + c1;
        REQUIRE(ret == complex{2, 1});
        ret = c1 + false;
        REQUIRE(ret == complex{1, 1});
        ret = true + std::move(c1);
        REQUIRE(ret == complex{2, 1});
        c1 = c2;
        ret = std::move(c1) + false;
        REQUIRE(ret == complex{1, 1});

#if defined(MPPP_HAVE_GCC_INT128)
        // Try with a large integral.
        c1 = c2;
        ret = __uint128_t(-1) + std::move(c1);
        REQUIRE(ret == complex{1_z1 + __uint128_t(-1), 1, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(129)};
        ret = __uint128_t(-1) + std::move(c1);
        REQUIRE(ret == complex{1_z1 + __uint128_t(-1), 1, complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = std::move(c1) + __uint128_t(-1);
        REQUIRE(ret == complex{1_z1 + __uint128_t(-1), 1, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(129)};
        ret = std::move(c1) + __uint128_t(-1);
        REQUIRE(ret == complex{1_z1 + __uint128_t(-1), 1, complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
#endif
    }

    // Complex-std::complex.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 + std::complex<double>(6, 7);
        REQUIRE(ret == complex{51, 13});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));
        ret = std::complex<double>(6, 7) + c1;
        REQUIRE(ret == complex{51, 13});
        REQUIRE(std::is_same<complex, decltype(std::complex<double>(6, 7) + c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 + std::complex<double>(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        ret = std::complex<double>(6, 7) + c1;
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) + std::complex<double>(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(6.) + 1)};
        ret = std::move(c1) + std::complex<double>(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = std::complex<double>(6, 7) + std::move(c1);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(6.) + 1)};
        ret = std::complex<double>(6, 7) + std::move(c1);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }

#if defined(MPPP_WITH_QUADMATH)
    // Complex-complex128.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 + complex128(6, 7);
        REQUIRE(ret == complex{51, 13});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == 128);
        ret = complex128(6, 7) + c1;
        REQUIRE(ret == complex{51, 13});
        REQUIRE(std::is_same<complex, decltype(complex128(6, 7) + c1)>::value);
        REQUIRE(ret.get_prec() == 128);

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 + complex128(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 113);
        ret = complex128(6, 7) + c1;
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 113);

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) + complex128(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = std::move(c1) + complex128(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = complex128(6, 7) + std::move(c1);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = complex128(6, 7) + std::move(c1);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
#endif

    // real-std::complex.
    {
        real r{5, 5};
        auto ret = r + std::complex<double>{5, 6};
        REQUIRE(ret == complex{10, 6});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.));
        ret = std::complex<double>{5, 6} + r;
        REQUIRE(ret == complex{10, 6});
        REQUIRE(std::is_same<complex, decltype(std::complex<double>{5, 6} + r)>::value);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.));

        // Switch precisions around.
        r = real{5, detail::real_deduce_precision(5.) + 1};
        ret = r + std::complex<double>{5, 6};
        REQUIRE(ret == complex{10, 6});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.) + 1);
        ret = std::complex<double>{5, 6} + r;
        REQUIRE(ret == complex{10, 6});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.) + 1);
    }

#if defined(MPPP_WITH_QUADMATH)
    // real-complex128.
    {
        real r{5, 5};
        auto ret = r + complex128{5, 6};
        REQUIRE(ret == complex{10, 6});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == 113);
        ret = complex128{5, 6} + r;
        REQUIRE(ret == complex{10, 6});
        REQUIRE(std::is_same<complex, decltype(complex128{5, 6} + r)>::value);
        REQUIRE(ret.get_prec() == 113);

        // Switch precisions around.
        r = real{5, 114};
        ret = r + complex128{5, 6};
        REQUIRE(ret == complex{10, 6});
        REQUIRE(ret.get_prec() == 114);
        ret = complex128{5, 6} + r;
        REQUIRE(ret == complex{10, 6});
        REQUIRE(ret.get_prec() == 114);
    }
#endif
}

TEST_CASE("in-place plus")
{
    // complex-complex.
    {
        complex c1{1, 2}, c2{3, 4};
        c1 += c2;
        REQUIRE(std::is_same<complex &, decltype(c1 += c2)>::value);
        REQUIRE(c1 == complex{4, 6});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // Move which does not steal.
        c1 += std::move(c2);
        REQUIRE(c1 == complex{7, 10});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2.is_valid());

        // Move which steals.
        complex c3{4, 5, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        c1 += std::move(c3);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c3.is_valid());
        REQUIRE(c3 == complex{7, 10});
        REQUIRE(c3.get_prec() == detail::real_deduce_precision(1));

        // Self add.
        c3 += c3;
        REQUIRE(c3 == complex{14, 20});
    }
    // complex-real.
    {
        // Same precision.
        complex c1{1, 2};
        real r{4};
        c1 += r;
        REQUIRE(std::is_same<complex &, decltype(c1 += r)>::value);
        REQUIRE(c1 == complex{5, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // r with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 += r;
        REQUIRE(c1 == complex{5, 1});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // r with smaller precision.
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        c1 += r;
        REQUIRE(c1 == complex{5, 1});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1) + 1);
    }
    // complex-real valued.
    {
        // Other op with same precision.
        complex c1{1, 2};
        c1 += 4;
        REQUIRE(std::is_same<complex &, decltype(c1 += 4)>::value);
        REQUIRE(c1 == complex{5, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 += 4.;
        REQUIRE(c1 == complex{5, 1});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4.));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        c1 += 4;
        REQUIRE(c1 == complex{5, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1) + 1);
    }
    // complex-unsigned integral.
    {
        // Other op with same precision.
        complex c1{1u, 2u};
        c1 += 4u;
        REQUIRE(std::is_same<complex &, decltype(c1 += 4u)>::value);
        REQUIRE(c1 == complex{5, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1u));

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 += 4u;
        REQUIRE(c1 == complex{5, 1});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4u));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(1u) + 1)};
        c1 += 4u;
        REQUIRE(c1 == complex{5, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1u) + 1);

#if defined(MPPP_HAVE_GCC_INT128)
        // Test with large unsigned integral type.
        c1 = complex{1, 0, complex_prec_t(real_prec_min())};
        c1 += __uint128_t(-1);
        REQUIRE(std::is_same<complex &, decltype(c1 += __uint128_t(-1))>::value);
        REQUIRE(c1 == 1_z1 + __uint128_t(-1));
        REQUIRE(c1.get_prec() == 128);

        c1 = complex{1, 0, complex_prec_t(256)};
        c1 += __uint128_t(-1);
        REQUIRE(c1 == 1_z1 + __uint128_t(-1));
        REQUIRE(c1.get_prec() == 256);
#endif
    }
    // Special casing for bool.
    {
        // Other op with same precision.
        complex c1{true, false};
        c1 += true;
        REQUIRE(std::is_same<complex &, decltype(c1 += true)>::value);
        REQUIRE(c1 == complex{2, 0});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true));

        // Other op with higher precision.
        c1 = complex{true, false, complex_prec_t(real_prec_min())};
        c1 += true;
        REQUIRE(c1 == complex{2, 0});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(true) + 1)};
        c1 += false;
        REQUIRE(c1 == complex{1, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true) + 1);
    }

    // complex-std::complex.
    {
        // Other op with same precision.
        complex c1{1., 2.};
        c1 += std::complex<double>{3, 4};
        REQUIRE(std::is_same<complex &, decltype(c1 += std::complex<double>{3, 4})>::value);
        REQUIRE(c1 == complex{4, 6});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.));

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 += std::complex<double>{3, 4};
        REQUIRE(c1 == complex{4, 5});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4.));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(1.) + 1)};
        c1 += std::complex<double>{3, 4};
        REQUIRE(c1 == complex{4, 6});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.) + 1);
    }

#if defined(MPPP_WITH_QUADMATH)
    // complex-complex128.
    {
        // Other op with same precision.
        complex c1{1., 2., complex_prec_t(113)};
        c1 += complex128{3, 4};
        REQUIRE(std::is_same<complex &, decltype(c1 += complex128{3, 4})>::value);
        REQUIRE(c1 == complex{4, 6});
        REQUIRE(c1.get_prec() == 113);

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 += complex128{3, 4};
        REQUIRE(c1 == complex{4, 5});
        REQUIRE(c1.get_prec() == 113);

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(114)};
        c1 += std::complex<double>{3, 4};
        REQUIRE(c1 == complex{4, 6});
        REQUIRE(c1.get_prec() == 114);
    }
#endif

    // complex interoperable-complex.
    {
        int n = 5;
        n += complex{4, 0};
        REQUIRE(std::is_same<int &, decltype(n += complex{4, 0})>::value);
        REQUIRE(n == 9);

        // Check move semantics.
        complex c{4, 0, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        n += std::move(c);
        REQUIRE(n == 13);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());

        // Check conversion failure.
        REQUIRE_THROWS_AS((n += complex{4, 1}), std::domain_error);
        REQUIRE(n == 13);
        if (std::numeric_limits<double>::has_infinity) {
            REQUIRE_THROWS_AS((n += complex{std::numeric_limits<double>::infinity(), 0}), std::domain_error);
            REQUIRE(n == 13);
        }

        // Try with complex-valued too.
        std::complex<double> cd{4, 5};
        cd += complex{4, 1};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd += complex{4, 1})>::value);
        REQUIRE(cd == std::complex<double>{8, 6});

#if defined(MPPP_WITH_QUADMATH)
        complex128 cq{4, 5};
        cq += complex{4, 1};
        REQUIRE(std::is_same<complex128 &, decltype(cq += complex{4, 1})>::value);
        REQUIRE(cq == complex128{8, 6});
#endif
    }

    // real-complex valued
    {
        real r{4, 5};
        r += std::complex<double>{4, 0};
        REQUIRE(std::is_same<real &, decltype(r += std::complex<double>{4, 0})>::value);
        REQUIRE(r == 8);
        REQUIRE(r.get_prec() == detail::real_deduce_precision(1.));

        // Check conversion failure.
        REQUIRE_THROWS_AS((r += complex{4, 1}), std::domain_error);
        REQUIRE(r == 8);

#if defined(MPPP_WITH_QUADMATH)
        r += complex128{4, 0};
        REQUIRE(std::is_same<real &, decltype(r += complex128{4, 0})>::value);
        REQUIRE(r == 12);
        REQUIRE(r.get_prec() == detail::c_max(detail::real_deduce_precision(1.), mpfr_prec_t(113)));
#endif
    }

    // complex valued-real.
    {
        std::complex<double> c{1, 2};
        c += real{2, 5};
        REQUIRE(std::is_same<std::complex<double> &, decltype(c += real{2, 5})>::value);
        REQUIRE(c == std::complex<double>{3, 2});

        // Check move semantics.
        real r{4, detail::real_deduce_precision(1.) + 1};
        c += std::move(r);
        REQUIRE(c == std::complex<double>{7, 2});
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());

#if defined(MPPP_WITH_QUADMATH)
        complex128 c2{3, 4};
        c2 += real{2, 114};
        REQUIRE(std::is_same<complex128 &, decltype(c2 += real{2, 114})>::value);
        REQUIRE(c2 == complex128{5, 4});
#endif
    }
}

TEST_CASE("negation")
{
    complex r1{4, 5};
    REQUIRE(-r1 == complex{-4, -5});

    // Check stealing.
    const auto p = r1.get_prec();
    auto r2 = -std::move(r1);
    REQUIRE(r2.get_prec() == p);
    REQUIRE(r2 == complex{-4, -5});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r1.is_valid());
}

TEST_CASE("decrement")
{
    complex r0{0};
    REQUIRE(--r0 == -1);
    REQUIRE(r0-- == -1);
    REQUIRE(r0 == -2);

    // Check precision handling.
    r0 = complex{0, complex_prec_t(4)};
    --r0;
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(1));
    REQUIRE(r0 == -1);
    r0 = complex{0, complex_prec_t(4)};
    r0--;
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(1));
    REQUIRE(r0 == -1);
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("binary minus")
{
    // complex-complex.
    {
        complex r1{4, 5}, r2{-4, 7};
        const auto p = r1.get_prec();
        auto ret = r1 - r2;
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret == complex{8, -2});
        REQUIRE(ret.get_prec() == r1.get_prec());

        // Test moves.
        ret = std::move(r1) - r2;
        REQUIRE(ret == complex{8, -2});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r1.is_valid());

        r1 = complex{4, 5};
        ret = r1 - std::move(r2);
        REQUIRE(ret == complex{8, -2});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r2.is_valid());

        r2 = complex{-4, 7};
        ret = std::move(r1) - std::move(r2);
        REQUIRE(ret == complex{8, -2});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE((!r1.is_valid() || !r2.is_valid()));

        // Self sub.
        r2 = complex{-4, 6};
        REQUIRE(r2 - r2 == complex{});
    }
    // complex-real.
    {
        complex c1{45, 6, complex_prec_t(128)};
        real r1{23, 10};
        auto ret = c1 - r1;
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret == complex{22, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        ret = r1 - c1;
        REQUIRE(std::is_same<complex, decltype(r1 - c1)>::value);
        REQUIRE(ret == complex{-22, -6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);

        // Try with higher precision on the non-complex argument.
        ret = c1 - real{23, 256};
        REQUIRE(ret == complex{22, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 256);
        ret = real{23, 256} - c1;
        REQUIRE(ret == complex{-22, -6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 256);

        // Try with moves.
        auto c2 = c1;
        ret = complex{};
        ret = std::move(c1) - r1;
        REQUIRE(ret == complex{22, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = complex{};
        ret = r1 - std::move(c1);
        REQUIRE(ret == complex{-22, -6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
    // complex-rv interoperable.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 - 6;
        REQUIRE(ret == complex{39, 6});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6)));
        ret = 6. - c1;
        REQUIRE(ret == complex{-39, -6});
        REQUIRE(std::is_same<complex, decltype(6. - c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 - 45_z1;
        REQUIRE(ret == complex{-44, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45_z1));
        ret = 45_q1 - c1;
        REQUIRE(ret == complex{44, -1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45_q1));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) - 45;
        REQUIRE(ret == complex{-44, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45) + 1)};
        ret = std::move(c1) - 45;
        REQUIRE(ret == complex{-44, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = 45. - std::move(c1);
        REQUIRE(ret == complex{44, -1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45.) + 1)};
        ret = 45. - std::move(c1);
        REQUIRE(ret == complex{44, -1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

#if defined(MPPP_WITH_QUADMATH)
        c1 = c2;
        ret = 45_rq - std::move(c1);
        REQUIRE(ret == complex{44, -1});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = 45_rq - std::move(c1);
        REQUIRE(ret == complex{44, -1});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
#endif
    }
    // complex-unsigned integral.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 - 6u;
        REQUIRE(ret == complex{39, 6});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6u)));
        ret = 6u - c1;
        REQUIRE(ret == complex{-39, -6});
        REQUIRE(std::is_same<complex, decltype(6u - c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6u)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 - 45u;
        REQUIRE(ret == complex{-44, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        ret = 45u - c1;
        REQUIRE(ret == complex{44, -1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) - 45u;
        REQUIRE(ret == complex{-44, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45u) + 1)};
        ret = std::move(c1) - 45u;
        REQUIRE(ret == complex{-44, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = 45u - std::move(c1);
        REQUIRE(ret == complex{44, -1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45u) + 1)};
        ret = 45u - std::move(c1);
        REQUIRE(ret == complex{44, -1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        // Bool special casing.
        c1 = c2;
        ret = true - c1;
        REQUIRE(ret == complex{0, -1});
        ret = c1 - false;
        REQUIRE(ret == complex{1, 1});
        ret = true - std::move(c1);
        REQUIRE(ret == complex{0, -1});
        c1 = c2;
        ret = std::move(c1) - false;
        REQUIRE(ret == complex{1, 1});

#if defined(MPPP_HAVE_GCC_INT128)
        // Try with a large integral.
        c1 = c2;
        ret = __uint128_t(-1) - std::move(c1);
        REQUIRE(ret == complex{__uint128_t(-1) - 1u, -1, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(129)};
        ret = __uint128_t(-1) - std::move(c1);
        REQUIRE(ret == complex{__uint128_t(-1) - 1u, -1, complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = std::move(c1) - __uint128_t(-1);
        REQUIRE(ret == complex{1_z1 - __uint128_t(-1), 1, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(129)};
        ret = std::move(c1) - __uint128_t(-1);
        REQUIRE(ret == complex{1_z1 - __uint128_t(-1), 1, complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
#endif
    }

    // Complex-std::complex.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 - std::complex<double>(6, 7);
        REQUIRE(ret == complex{39, -1});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));
        ret = std::complex<double>(6, 7) - c1;
        REQUIRE(ret == complex{-39, 1});
        REQUIRE(std::is_same<complex, decltype(std::complex<double>(6, 7) - c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 - std::complex<double>(6, 7);
        REQUIRE(ret == complex{-5, -6});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        ret = std::complex<double>(6, 7) - c1;
        REQUIRE(ret == complex{5, 6});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) - std::complex<double>(6, 7);
        REQUIRE(ret == complex{-5, -6});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(6.) + 1)};
        ret = std::move(c1) - std::complex<double>(6, 7);
        REQUIRE(ret == complex{-5, -6});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = std::complex<double>(6, 7) - std::move(c1);
        REQUIRE(ret == complex{5, 6});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(6.) + 1)};
        ret = std::complex<double>(6, 7) - std::move(c1);
        REQUIRE(ret == complex{5, 6});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }

#if defined(MPPP_WITH_QUADMATH)
    // Complex-complex128.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 - complex128(6, 7);
        REQUIRE(ret == complex{39, -1});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == 128);
        ret = complex128(6, 7) - c1;
        REQUIRE(ret == complex{-39, 1});
        REQUIRE(std::is_same<complex, decltype(complex128(6, 7) - c1)>::value);
        REQUIRE(ret.get_prec() == 128);

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 - complex128(6, 7);
        REQUIRE(ret == complex{-5, -6});
        REQUIRE(ret.get_prec() == 113);
        ret = complex128(6, 7) - c1;
        REQUIRE(ret == complex{5, 6});
        REQUIRE(ret.get_prec() == 113);

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) - complex128(6, 7);
        REQUIRE(ret == complex{-5, -6});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = std::move(c1) - complex128(6, 7);
        REQUIRE(ret == complex{-5, -6});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = complex128(6, 7) - std::move(c1);
        REQUIRE(ret == complex{5, 6});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = complex128(6, 7) - std::move(c1);
        REQUIRE(ret == complex{5, 6});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
#endif

    // real-std::complex.
    {
        real r{5, 5};
        auto ret = r - std::complex<double>{5, 6};
        REQUIRE(ret == complex{0, -6});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.));
        ret = std::complex<double>{5, 6} - r;
        REQUIRE(ret == complex{0, 6});
        REQUIRE(std::is_same<complex, decltype(std::complex<double>{5, 6} - r)>::value);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.));

        // Switch precisions around.
        r = real{5, detail::real_deduce_precision(5.) + 1};
        ret = r - std::complex<double>{5, 6};
        REQUIRE(ret == complex{0, -6});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.) + 1);
        ret = std::complex<double>{5, 6} - r;
        REQUIRE(ret == complex{0, 6});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.) + 1);
    }

#if defined(MPPP_WITH_QUADMATH)
    // real-complex128.
    {
        real r{5, 5};
        auto ret = r - complex128{5, 6};
        REQUIRE(ret == complex{0, -6});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == 113);
        ret = complex128{5, 6} - r;
        REQUIRE(ret == complex{0, 6});
        REQUIRE(std::is_same<complex, decltype(complex128{5, 6} - r)>::value);
        REQUIRE(ret.get_prec() == 113);

        // Switch precisions around.
        r = real{5, 114};
        ret = r - complex128{5, 6};
        REQUIRE(ret == complex{0, -6});
        REQUIRE(ret.get_prec() == 114);
        ret = complex128{5, 6} - r;
        REQUIRE(ret == complex{0, 6});
        REQUIRE(ret.get_prec() == 114);
    }
#endif
}

TEST_CASE("in-place minus")
{
    // complex-complex.
    {
        complex c1{1, 2}, c2{3, 4};
        c1 -= c2;
        REQUIRE(std::is_same<complex &, decltype(c1 -= c2)>::value);
        REQUIRE(c1 == complex{-2, -2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // Move which does not steal.
        c1 -= std::move(c2);
        REQUIRE(c1 == complex{-5, -6});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2.is_valid());

        // Move which steals.
        complex c3{4, 5, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        c1 -= std::move(c3);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c3.is_valid());
        REQUIRE(c3 == complex{-5, -6});
        REQUIRE(c3.get_prec() == detail::real_deduce_precision(1));

        // Self sub.
        c3 -= *&c3;
        REQUIRE(c3 == complex{});
    }
    // complex-real.
    {
        // Same precision.
        complex c1{1, 2};
        real r{4};
        c1 -= r;
        REQUIRE(std::is_same<complex &, decltype(c1 -= r)>::value);
        REQUIRE(c1 == complex{-3, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // r with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 -= r;
        REQUIRE(c1 == complex{-3, 1});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // r with smaller precision.
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        c1 -= r;
        REQUIRE(c1 == complex{-3, 1});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1) + 1);
    }
    // complex-real valued.
    {
        // Other op with same precision.
        complex c1{1, 2};
        c1 -= 4;
        REQUIRE(std::is_same<complex &, decltype(c1 -= 4)>::value);
        REQUIRE(c1 == complex{-3, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 -= 4.;
        REQUIRE(c1 == complex{-3, 1});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4.));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        c1 -= 4;
        REQUIRE(c1 == complex{-3, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1) + 1);
    }
    // complex-unsigned integral.
    {
        // Other op with same precision.
        complex c1{1u, 2u};
        c1 -= 4u;
        REQUIRE(std::is_same<complex &, decltype(c1 -= 4u)>::value);
        REQUIRE(c1 == complex{-3, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1u));

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 -= 4u;
        REQUIRE(c1 == complex{-3, 1});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4u));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(1u) + 1)};
        c1 -= 4u;
        REQUIRE(c1 == complex{-3, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1u) + 1);

#if defined(MPPP_HAVE_GCC_INT128)
        // Test with large unsigned integral type.
        c1 = complex{1, 0, complex_prec_t(real_prec_min())};
        c1 -= __uint128_t(-1);
        REQUIRE(std::is_same<complex &, decltype(c1 -= __uint128_t(-1))>::value);
        REQUIRE(c1 == 1_z1 - __uint128_t(-1));
        REQUIRE(c1.get_prec() == 128);

        c1 = complex{1, 0, complex_prec_t(256)};
        c1 -= __uint128_t(-1);
        REQUIRE(c1 == 1_z1 - __uint128_t(-1));
        REQUIRE(c1.get_prec() == 256);
#endif
    }
    // Special casing for bool.
    {
        // Other op with same precision.
        complex c1{true, false};
        c1 -= true;
        REQUIRE(std::is_same<complex &, decltype(c1 -= true)>::value);
        REQUIRE(c1 == complex{0, 0});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true));

        // Other op with higher precision.
        c1 = complex{true, false, complex_prec_t(real_prec_min())};
        c1 -= true;
        REQUIRE(c1 == complex{0, 0});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(true) + 1)};
        c1 -= false;
        REQUIRE(c1 == complex{1, 2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true) + 1);
    }

    // complex-std::complex.
    {
        // Other op with same precision.
        complex c1{1., 2.};
        c1 -= std::complex<double>{3, 4};
        REQUIRE(std::is_same<complex &, decltype(c1 -= std::complex<double>{3, 4})>::value);
        REQUIRE(c1 == complex{-2, -2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.));

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 -= std::complex<double>{3, 4};
        REQUIRE(c1 == complex{-2, -3});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4.));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(1.) + 1)};
        c1 -= std::complex<double>{3, 4};
        REQUIRE(c1 == complex{-2, -2});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.) + 1);
    }

#if defined(MPPP_WITH_QUADMATH)
    // complex-complex128.
    {
        // Other op with same precision.
        complex c1{1., 2., complex_prec_t(113)};
        c1 -= complex128{3, 4};
        REQUIRE(std::is_same<complex &, decltype(c1 -= complex128{3, 4})>::value);
        REQUIRE(c1 == complex{-2, -2});
        REQUIRE(c1.get_prec() == 113);

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 -= complex128{3, 4};
        REQUIRE(c1 == complex{-2, -3});
        REQUIRE(c1.get_prec() == 113);

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(114)};
        c1 -= std::complex<double>{3, 4};
        REQUIRE(c1 == complex{-2, -2});
        REQUIRE(c1.get_prec() == 114);
    }
#endif

    // complex interoperable-complex.
    {
        int n = 5;
        n -= complex{4, 0};
        REQUIRE(std::is_same<int &, decltype(n -= complex{4, 0})>::value);
        REQUIRE(n == 1);

        // Check move semantics.
        complex c{4, 0, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        n -= std::move(c);
        REQUIRE(n == -3);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());

        // Check conversion failure.
        REQUIRE_THROWS_AS((n -= complex{4, 1}), std::domain_error);
        REQUIRE(n == -3);
        if (std::numeric_limits<double>::has_infinity) {
            REQUIRE_THROWS_AS((n -= complex{std::numeric_limits<double>::infinity(), 0}), std::domain_error);
            REQUIRE(n == -3);
        }

        // Try with complex-valued too.
        std::complex<double> cd{4, 5};
        cd -= complex{4, 1};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd -= complex{4, 1})>::value);
        REQUIRE(cd == std::complex<double>{0, 4});

#if defined(MPPP_WITH_QUADMATH)
        complex128 cq{4, 5};
        cq -= complex{4, 1};
        REQUIRE(std::is_same<complex128 &, decltype(cq -= complex{4, 1})>::value);
        REQUIRE(cq == complex128{0, 4});
#endif
    }

    // real-complex valued
    {
        real r{4, 5};
        r -= std::complex<double>{4, 0};
        REQUIRE(std::is_same<real &, decltype(r -= std::complex<double>{4, 0})>::value);
        REQUIRE(r == 0);
        REQUIRE(r.get_prec() == detail::real_deduce_precision(1.));

        // Check conversion failure.
        REQUIRE_THROWS_AS((r -= complex{4, 1}), std::domain_error);
        REQUIRE(r == 0);

#if defined(MPPP_WITH_QUADMATH)
        r -= complex128{4, 0};
        REQUIRE(std::is_same<real &, decltype(r -= complex128{4, 0})>::value);
        REQUIRE(r == -4);
        REQUIRE(r.get_prec() == detail::c_max(detail::real_deduce_precision(1.), mpfr_prec_t(113)));
#endif
    }

    // complex valued-real.
    {
        std::complex<double> c{1, 2};
        c -= real{2, 5};
        REQUIRE(std::is_same<std::complex<double> &, decltype(c -= real{2, 5})>::value);
        REQUIRE(c == std::complex<double>{-1, 2});

        // Check move semantics.
        real r{4, detail::real_deduce_precision(1.) + 1};
        c -= std::move(r);
        REQUIRE(c == std::complex<double>{-5, 2});
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());

#if defined(MPPP_WITH_QUADMATH)
        complex128 c2{3, 4};
        c2 -= real{2, 114};
        REQUIRE(std::is_same<complex128 &, decltype(c2 -= real{2, 114})>::value);
        REQUIRE(c2 == complex128{1, 4});
#endif
    }
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("binary mul")
{
    // complex-complex.
    {
        complex r1{4, 5}, r2{-4, 7};
        const auto p = r1.get_prec();
        auto ret = r1 * r2;
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret == complex{-51, 8});
        REQUIRE(ret.get_prec() == r1.get_prec());

        // Test moves.
        ret = std::move(r1) * r2;
        REQUIRE(ret == complex{-51, 8});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r1.is_valid());

        r1 = complex{4, 5};
        ret = r1 * std::move(r2);
        REQUIRE(ret == complex{-51, 8});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r2.is_valid());

        r2 = complex{-4, 7};
        ret = std::move(r1) * std::move(r2);
        REQUIRE(ret == complex{-51, 8});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE((!r1.is_valid() || !r2.is_valid()));

        // Self mul.
        r2 = complex{-4, 6};
        REQUIRE(r2 * r2 == complex{-20, -48});
    }
    // complex-real.
    {
        complex c1{45, 6, complex_prec_t(128)};
        real r1{23, 10};
        auto ret = c1 * r1;
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret == complex{1035, 138, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        ret = r1 * c1;
        REQUIRE(std::is_same<complex, decltype(r1 * c1)>::value);
        REQUIRE(ret == complex{1035, 138, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);

        // Try with higher precision on the non-complex argument.
        ret = c1 * real{23, 256};
        REQUIRE(ret == complex{1035, 138, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 256);
        ret = real{23, 256} * c1;
        REQUIRE(ret == complex{1035, 138, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 256);

        // Try with moves.
        auto c2 = c1;
        ret = complex{};
        ret = std::move(c1) * r1;
        REQUIRE(ret == complex{1035, 138, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = complex{};
        ret = r1 * std::move(c1);
        REQUIRE(ret == complex{1035, 138, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
    // complex-rv interoperable.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 * 6.;
        REQUIRE(ret == complex{270, 36});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));
        ret = 6. * c1;
        REQUIRE(ret == complex{270, 36});
        REQUIRE(std::is_same<complex, decltype(6. * c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 * 45_z1;
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45_z1));
        ret = 45_q1 * c1;
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45_q1));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) * 45;
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45) + 1)};
        ret = std::move(c1) * 45;
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = 45. * std::move(c1);
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45.) + 1)};
        ret = 45. * std::move(c1);
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

#if defined(MPPP_WITH_QUADMATH)
        c1 = c2;
        ret = 45_rq * std::move(c1);
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = 45_rq * std::move(c1);
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
#endif
    }
    // complex-unsigned integral.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 * 6u;
        REQUIRE(ret == complex{270, 36});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6u)));
        ret = 6u * c1;
        REQUIRE(ret == complex{270, 36});
        REQUIRE(std::is_same<complex, decltype(6u * c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6u)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 * 45u;
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        ret = 45u * c1;
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) * 45u;
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45u) + 1)};
        ret = std::move(c1) * 45u;
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = 45u * std::move(c1);
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45u) + 1)};
        ret = 45u * std::move(c1);
        REQUIRE(ret == complex{45, 45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        // Bool special casing.
        c1 = c2;
        ret = true * c1;
        REQUIRE(ret == complex{1, 1});
        ret = c1 * false;
        REQUIRE(ret == complex{0, 0});
        ret = true * std::move(c1);
        REQUIRE(ret == complex{1, 1});
        c1 = c2;
        ret = std::move(c1) * false;
        REQUIRE(ret == complex{0, 0});

#if defined(MPPP_HAVE_GCC_INT128)
        // Try with a large integral.
        c1 = c2;
        ret = __uint128_t(-1) * std::move(c1);
        REQUIRE(ret == complex{__uint128_t(-1), __uint128_t(-1), complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(129)};
        ret = __uint128_t(-1) * std::move(c1);
        REQUIRE(ret == complex{__uint128_t(-1), __uint128_t(-1), complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = std::move(c1) * __uint128_t(-1);
        REQUIRE(ret == complex{__uint128_t(-1), __uint128_t(-1), complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(129)};
        ret = std::move(c1) * __uint128_t(-1);
        REQUIRE(ret == complex{__uint128_t(-1), __uint128_t(-1), complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
#endif
    }
    // complex-signed integral.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 * -6;
        REQUIRE(ret == complex{-270, -36});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6)));
        ret = -6 * c1;
        REQUIRE(ret == complex{-270, -36});
        REQUIRE(std::is_same<complex, decltype(6 * c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 * -45;
        REQUIRE(ret == complex{-45, -45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45));
        ret = -45 * c1;
        REQUIRE(ret == complex{-45, -45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) * -45;
        REQUIRE(ret == complex{-45, -45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(-45));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45) + 1)};
        ret = std::move(c1) * -45;
        REQUIRE(ret == complex{-45, -45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(-45) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = -45 * std::move(c1);
        REQUIRE(ret == complex{-45, -45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(-45));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45) + 1)};
        ret = -45 * std::move(c1);
        REQUIRE(ret == complex{-45, -45});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(-45) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

#if defined(MPPP_HAVE_GCC_INT128)
        // Try with a large integral.
        c1 = c2;
        const auto big_n = -(__int128_t(1) << 86);
        ret = big_n * std::move(c1);
        REQUIRE(ret == complex{big_n, big_n, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(129)};
        ret = big_n * std::move(c1);
        REQUIRE(ret == complex{big_n, big_n, complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = std::move(c1) * big_n;
        REQUIRE(ret == complex{big_n, big_n, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(129)};
        ret = std::move(c1) * big_n;
        REQUIRE(ret == complex{big_n, big_n, complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
#endif
    }

    // Complex-std::complex.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 * std::complex<double>(6, 7);
        REQUIRE(ret == complex{228, 351});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));
        ret = std::complex<double>(6, 7) * c1;
        REQUIRE(ret == complex{228, 351});
        REQUIRE(std::is_same<complex, decltype(std::complex<double>(6, 7) * c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 * std::complex<double>(6, 7);
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        ret = std::complex<double>(6, 7) * c1;
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) * std::complex<double>(6, 7);
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(6.) + 1)};
        ret = std::move(c1) * std::complex<double>(6, 7);
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = std::complex<double>(6, 7) * std::move(c1);
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(6.) + 1)};
        ret = std::complex<double>(6, 7) * std::move(c1);
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }

#if defined(MPPP_WITH_QUADMATH)
    // Complex-complex128.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 * complex128(6, 7);
        REQUIRE(ret == complex{228, 351});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == 128);
        ret = complex128(6, 7) * c1;
        REQUIRE(ret == complex{228, 351});
        REQUIRE(std::is_same<complex, decltype(complex128(6, 7) * c1)>::value);
        REQUIRE(ret.get_prec() == 128);

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 * complex128(6, 7);
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == 113);
        ret = complex128(6, 7) * c1;
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == 113);

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) * complex128(6, 7);
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = std::move(c1) * complex128(6, 7);
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = complex128(6, 7) * std::move(c1);
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = complex128(6, 7) * std::move(c1);
        REQUIRE(ret == complex{-1, 13});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
#endif

    // real-std::complex.
    {
        real r{5, 5};
        auto ret = r * std::complex<double>{5, 6};
        REQUIRE(ret == complex{25, 30});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.));
        ret = std::complex<double>{5, 6} * r;
        REQUIRE(ret == complex{25, 30});
        REQUIRE(std::is_same<complex, decltype(std::complex<double>{5, 6} * r)>::value);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.));

        // Switch precisions around.
        r = real{5, detail::real_deduce_precision(5.) + 1};
        ret = r * std::complex<double>{5, 6};
        REQUIRE(ret == complex{25, 30});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.) + 1);
        ret = std::complex<double>{5, 6} * r;
        REQUIRE(ret == complex{25, 30});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.) + 1);

        // Check moves.
        ret = std::move(r) * std::complex<double>{5, 6};
        REQUIRE(ret == complex{25, 30});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
        r = real{5, detail::real_deduce_precision(5.) + 1};
        ret = std::complex<double>{5, 6} * std::move(r);
        REQUIRE(ret == complex{25, 30});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
    }

#if defined(MPPP_WITH_QUADMATH)
    // real-complex128.
    {
        real r{5, 5};
        auto ret = r * complex128{5, 6};
        REQUIRE(ret == complex{25, 30});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == 113);
        ret = complex128{5, 6} * r;
        REQUIRE(ret == complex{25, 30});
        REQUIRE(std::is_same<complex, decltype(complex128{5, 6} * r)>::value);
        REQUIRE(ret.get_prec() == 113);

        // Switch precisions around.
        r = real{5, 114};
        ret = r * complex128{5, 6};
        REQUIRE(ret == complex{25, 30});
        REQUIRE(ret.get_prec() == 114);
        ret = complex128{5, 6} * r;
        REQUIRE(ret == complex{25, 30});
        REQUIRE(ret.get_prec() == 114);

        // Check moves.
        ret = std::move(r) * complex128{5, 6};
        REQUIRE(ret == complex{25, 30});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
        r = real{5, 114};
        ret = complex128{5, 6} * std::move(r);
        REQUIRE(ret == complex{25, 30});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
    }
#endif
}

TEST_CASE("in-place mul")
{
    // complex-complex.
    {
        complex c1{1, 2}, c2{3, 4};
        c1 *= c2;
        REQUIRE(std::is_same<complex &, decltype(c1 *= c2)>::value);
        REQUIRE(c1 == complex{-5, 10});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // Move which does not steal.
        c1 *= std::move(c2);
        REQUIRE(c1 == complex{-55, 10});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2.is_valid());

        // Move which steals.
        complex c3{4, 5, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        c1 *= std::move(c3);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c3.is_valid());
        REQUIRE(c3 == complex{-55, 10});
        REQUIRE(c3.get_prec() == detail::real_deduce_precision(1));

        // Self add.
        c3 *= c3;
        REQUIRE(c3 == complex{2925, -1100});
    }
    // complex-real.
    {
        // Same precision.
        complex c1{1, 2};
        real r{4};
        c1 *= r;
        REQUIRE(std::is_same<complex &, decltype(c1 *= r)>::value);
        REQUIRE(c1 == complex{4, 8});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // r with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 *= r;
        REQUIRE(c1 == complex{4, 4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // r with smaller precision.
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        c1 *= r;
        REQUIRE(c1 == complex{4, 4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1) + 1);
    }
    // complex-real valued.
    {
        // Other op with same precision.
        complex c1{1, 2, complex_prec_t(detail::real_deduce_precision(4.))};
        c1 *= 4.;
        REQUIRE(std::is_same<complex &, decltype(c1 *= 4.)>::value);
        REQUIRE(c1 == complex{4, 8});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.));

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 *= 4.;
        REQUIRE(c1 == complex{4, 4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4.));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(1.) + 1)};
        c1 *= 4.;
        REQUIRE(c1 == complex{4, 8});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.) + 1);
    }
    // complex-unsigned integral.
    {
        // Other op with same precision.
        complex c1{1u, 2u};
        c1 *= 4u;
        REQUIRE(std::is_same<complex &, decltype(c1 *= 4u)>::value);
        REQUIRE(c1 == complex{4, 8});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1u));

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 *= 4u;
        REQUIRE(c1 == complex{4, 4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4u));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(1u) + 1)};
        c1 *= 4u;
        REQUIRE(c1 == complex{4, 8});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1u) + 1);

#if defined(MPPP_HAVE_GCC_INT128)
        // Test with large unsigned integral type.
        c1 = complex{1, 0, complex_prec_t(real_prec_min())};
        c1 *= __uint128_t(-1);
        REQUIRE(std::is_same<complex &, decltype(c1 *= __uint128_t(-1))>::value);
        REQUIRE(c1 == 1_z1 * __uint128_t(-1));
        REQUIRE(c1.get_prec() == 128);

        c1 = complex{1, 0, complex_prec_t(256)};
        c1 *= __uint128_t(-1);
        REQUIRE(c1 == 1_z1 * __uint128_t(-1));
        REQUIRE(c1.get_prec() == 256);
#endif
    }
    // Special casing for bool.
    {
        // Other op with same precision.
        complex c1{true, false};
        c1 *= true;
        REQUIRE(std::is_same<complex &, decltype(c1 *= true)>::value);
        REQUIRE(c1 == complex{1, 0});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true));

        // Other op with higher precision.
        c1 = complex{true, false, complex_prec_t(real_prec_min())};
        c1 *= true;
        REQUIRE(c1 == complex{1, 0});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(true) + 1)};
        c1 *= false;
        REQUIRE(c1 == complex{0, 0});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true) + 1);
    }
    // complex-signed integral.
    {
        // Other op with same precision.
        complex c1{1u, 2u};
        c1 *= 4;
        REQUIRE(std::is_same<complex &, decltype(c1 *= 4)>::value);
        REQUIRE(c1 == complex{4, 8});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 *= 4;
        REQUIRE(c1 == complex{4, 4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        c1 *= 4;
        REQUIRE(c1 == complex{4, 8});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1) + 1);

#if defined(MPPP_HAVE_GCC_INT128)
        // Test with large unsigned integral type.
        c1 = complex{1, 0, complex_prec_t(real_prec_min())};
        const auto big_n = -(__int128_t(1) << 87);
        c1 *= big_n;
        REQUIRE(std::is_same<complex &, decltype(c1 *= big_n)>::value);
        REQUIRE(c1 == 1_z1 * big_n);
        REQUIRE(c1.get_prec() == 128);

        c1 = complex{1, 0, complex_prec_t(256)};
        c1 *= big_n;
        REQUIRE(c1 == 1_z1 * big_n);
        REQUIRE(c1.get_prec() == 256);
#endif
    }

    // complex-std::complex.
    {
        // Other op with same precision.
        complex c1{1., 2.};
        c1 *= std::complex<double>{3, 4};
        REQUIRE(std::is_same<complex &, decltype(c1 *= std::complex<double>{3, 4})>::value);
        REQUIRE(c1 == complex{-5, 10});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.));

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 *= std::complex<double>{3, 4};
        REQUIRE(c1 == complex{-1, 7});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4.));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(1.) + 1)};
        c1 *= std::complex<double>{3, 4};
        REQUIRE(c1 == complex{-5, 10});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.) + 1);
    }

#if defined(MPPP_WITH_QUADMATH)
    // complex-complex128.
    {
        // Other op with same precision.
        complex c1{1., 2., complex_prec_t(113)};
        c1 *= complex128{3, 4};
        REQUIRE(std::is_same<complex &, decltype(c1 *= complex128{3, 4})>::value);
        REQUIRE(c1 == complex{-5, 10});
        REQUIRE(c1.get_prec() == 113);

        // Other op with higher precision.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        c1 *= complex128{3, 4};
        REQUIRE(c1 == complex{-1, 7});
        REQUIRE(c1.get_prec() == 113);

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(114)};
        c1 *= std::complex<double>{3, 4};
        REQUIRE(c1 == complex{-5, 10});
        REQUIRE(c1.get_prec() == 114);
    }
#endif

    // complex interoperable-complex.
    {
        int n = 5;
        n *= complex{4, 0};
        REQUIRE(std::is_same<int &, decltype(n *= complex{4, 0})>::value);
        REQUIRE(n == 20);

        // Check move semantics.
        complex c{4, 0, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        n *= std::move(c);
        REQUIRE(n == 80);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());

        // Check conversion failure.
        REQUIRE_THROWS_AS((n *= complex{4, 1}), std::domain_error);
        REQUIRE(n == 80);
        if (std::numeric_limits<double>::has_infinity) {
            REQUIRE_THROWS_AS((n *= complex{std::numeric_limits<double>::infinity(), 0}), std::domain_error);
            REQUIRE(n == 80);
        }

        // Try with complex-valued too.
        std::complex<double> cd{4, 5};
        cd *= complex{4, 1};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd *= complex{4, 1})>::value);
        REQUIRE(cd == std::complex<double>{11, 24});

#if defined(MPPP_WITH_QUADMATH)
        complex128 cq{4, 5};
        cq *= complex{4, 1};
        REQUIRE(std::is_same<complex128 &, decltype(cq *= complex{4, 1})>::value);
        REQUIRE(cq == complex128{11, 24});
#endif
    }

    // real-complex valued
    {
        real r{4, 5};
        r *= std::complex<double>{4, 0};
        REQUIRE(std::is_same<real &, decltype(r *= std::complex<double>{4, 0})>::value);
        REQUIRE(r == 16);
        REQUIRE(r.get_prec() == detail::real_deduce_precision(1.));

        // Check conversion failure.
        REQUIRE_THROWS_AS((r *= complex{4, 1}), std::domain_error);
        REQUIRE(r == 16);

#if defined(MPPP_WITH_QUADMATH)
        r *= complex128{4, 0};
        REQUIRE(std::is_same<real &, decltype(r *= complex128{4, 0})>::value);
        REQUIRE(r == 64);
        REQUIRE(r.get_prec() == detail::c_max(detail::real_deduce_precision(1.), mpfr_prec_t(113)));
#endif
    }

    // complex valued-real.
    {
        std::complex<double> c{1, 2};
        c *= real{2, 5};
        REQUIRE(std::is_same<std::complex<double> &, decltype(c *= real{2, 5})>::value);
        REQUIRE(c == std::complex<double>{2, 4});

        // Check move semantics.
        real r{4, detail::real_deduce_precision(1.) + 1};
        c *= std::move(r);
        REQUIRE(c == std::complex<double>{8, 16});
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());

#if defined(MPPP_WITH_QUADMATH)
        complex128 c2{3, 4};
        c2 *= real{2, 114};
        REQUIRE(std::is_same<complex128 &, decltype(c2 *= real{2, 114})>::value);
        REQUIRE(c2 == complex128{6, 8});
#endif
    }
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("binary div")
{
    // complex-complex.
    {
        complex r1{11, 24}, r2{4, 5};
        const auto p = r1.get_prec();
        auto ret = r1 / r2;
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret == complex{4, 1});
        REQUIRE(ret.get_prec() == r1.get_prec());

        // Test moves.
        ret = std::move(r1) / r2;
        REQUIRE(ret == complex{4, 1});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r1.is_valid());

        r1 = complex{11, 24};
        ret = r1 / std::move(r2);
        REQUIRE(ret == complex{4, 1});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r2.is_valid());

        r2 = complex{4, 5};
        ret = std::move(r1) / std::move(r2);
        REQUIRE(ret == complex{4, 1});
        REQUIRE(ret.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE((!r1.is_valid() || !r2.is_valid()));

        // Self div.
        r2 = complex{-4, 6};
        REQUIRE(r2 / r2 == complex{1, 0});
    }
    // complex-real.
    {
        complex c1{44, 4, complex_prec_t(128)};
        real r1{2, 10};
        auto ret = c1 / r1;
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret == complex{22, 2, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        ret = r1 / c1;
        REQUIRE(std::is_same<complex, decltype(r1 / c1)>::value);
        REQUIRE(ret == complex{88 / 1952_q1, -8 / 1952_q1, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);

        // Try with higher precision on the non-complex argument.
        ret = c1 / real{2, 256};
        REQUIRE(ret == complex{22, 2, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 256);
        ret = real{2, 256} / c1;
        REQUIRE(ret == complex{88 / 1952_q1, -8 / 1952_q1, complex_prec_t(256)});
        REQUIRE(ret.get_prec() == 256);

        // Try with moves.
        auto c2 = c1;
        ret = complex{};
        ret = std::move(c1) / r1;
        REQUIRE(ret == complex{22, 2, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = complex{};
        ret = r1 / std::move(c1);
        REQUIRE(ret == complex{88 / 1952_q1, -8 / 1952_q1, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
    // complex-rv interoperable.
    {
        complex c1{44, 4, complex_prec_t(128)};
        auto ret = c1 / 2;
        REQUIRE(ret == complex{22, 2});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(2)));
        ret = 2. / c1;
        REQUIRE(ret
                == complex{88 / 1952_q1, -8 / 1952_q1,
                           complex_prec_t(std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(2.)))});
        REQUIRE(std::is_same<complex, decltype(2. / c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{2, -8, complex_prec_t(real_prec_min())};
        ret = c1 / 2_z1;
        REQUIRE(ret == complex{1, -4});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2_z1));
        ret = 45_q1 / c1;
        REQUIRE(ret == complex{90 / 68_q1, 360 / 68_q1, complex_prec_t(detail::real_deduce_precision(45_q1))});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45_q1));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) / 2;
        REQUIRE(ret == complex{1, -4});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{44, 4, complex_prec_t(detail::real_deduce_precision(2) + 1)};
        ret = std::move(c1) / 2;
        REQUIRE(ret == complex{22, 2});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = 45. / std::move(c1);
        REQUIRE(ret == complex{90 / 68_q1, 360 / 68_q1, complex_prec_t(detail::real_deduce_precision(45.))});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{2, -8, complex_prec_t(detail::real_deduce_precision(45.) + 1)};
        ret = 45. / std::move(c1);
        REQUIRE(ret == complex{90 / 68_q1, 360 / 68_q1, complex_prec_t(detail::real_deduce_precision(45.) + 1)});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

#if defined(MPPP_WITH_QUADMATH)
        c1 = c2;
        ret = 45_rq / std::move(c1);
        REQUIRE(ret == complex{90 / 68_q1, 360 / 68_q1, complex_prec_t(113)});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{2, -8, complex_prec_t(114)};
        ret = 45_rq / std::move(c1);
        REQUIRE(ret == complex{90 / 68_q1, 360 / 68_q1, complex_prec_t(114)});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
#endif
    }
    // complex-unsigned integral.
    {
        complex c1{44, 4, complex_prec_t(128)};
        auto ret = c1 / 2u;
        REQUIRE(ret == complex{22, 2});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(2u)));
        ret = 2u / c1;
        REQUIRE(ret
                == complex{88 / 1952_q1, -8 / 1952_q1,
                           complex_prec_t(std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(2u)))});
        REQUIRE(std::is_same<complex, decltype(2u / c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(2u)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{2, -8, complex_prec_t(real_prec_min())};
        ret = c1 / 2u;
        REQUIRE(ret == complex{1, -4});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2u));
        ret = 45u / c1;
        REQUIRE(ret == complex{90 / 68_q1, 360 / 68_q1, complex_prec_t(detail::real_deduce_precision(45u))});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) / 2u;
        REQUIRE(ret == complex{1, -4});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2u));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{44, 4, complex_prec_t(detail::real_deduce_precision(2u) + 1)};
        ret = std::move(c1) / 2u;
        REQUIRE(ret == complex{22, 2});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2u) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = 45u / std::move(c1);
        REQUIRE(ret == complex{90 / 68_q1, 360 / 68_q1, complex_prec_t(detail::real_deduce_precision(45u))});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{2, -8, complex_prec_t(detail::real_deduce_precision(45u) + 1)};
        ret = 45u / std::move(c1);
        REQUIRE(ret == complex{90 / 68_q1, 360 / 68_q1, complex_prec_t(detail::real_deduce_precision(45u) + 1)});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        // Bool special casing.
        c1 = c2;
        ret = false / c1;
        REQUIRE(ret == complex{});
        ret = c1 / true;
        REQUIRE(ret == c1);
        ret = false / std::move(c1);
        REQUIRE(ret == complex{});
        c1 = c2;
        ret = std::move(c1) / true;
        REQUIRE(ret == c2);

#if defined(MPPP_HAVE_GCC_INT128)
        // Try with a large integral.
        c1 = complex{1, complex_prec_t(real_prec_min())};
        ret = __uint128_t(-1) / std::move(c1);
        REQUIRE(ret == complex{__uint128_t(-1), 0, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, complex_prec_t(129)};
        ret = __uint128_t(-1) / std::move(c1);
        REQUIRE(ret == complex{__uint128_t(-1), 0, complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = complex{1, complex_prec_t(real_prec_min())};
        ret = std::move(c1) / __uint128_t(-1);
        REQUIRE(ret == complex{1_q1 / __uint128_t(-1), complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{1, complex_prec_t(129)};
        ret = std::move(c1) / __uint128_t(-1);
        REQUIRE(ret == complex{1_q1 / __uint128_t(-1), complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
#endif
    }

    // Complex-std::complex.
    {
        complex c1{11, 24, complex_prec_t(128)};
        auto ret = c1 / std::complex<double>(4, 5);
        REQUIRE(ret == complex{4, 1});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));
        ret = std::complex<double>(35, 13) / c1;
        REQUIRE(ret == complex{1, -1});
        REQUIRE(std::is_same<complex, decltype(std::complex<double>(35, 13) / c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{4, 0, complex_prec_t(real_prec_min())};
        ret = c1 / std::complex<double>(2, 2);
        REQUIRE(ret == complex{1, -1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        ret = std::complex<double>(8, -12) / c1;
        REQUIRE(ret == complex{2, -3});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) / std::complex<double>(2, 2);
        REQUIRE(ret == complex{1, -1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{4, -8, complex_prec_t(detail::real_deduce_precision(6.) + 1)};
        ret = std::move(c1) / std::complex<double>(2, 0);
        REQUIRE(ret == complex{2, -4});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = std::complex<double>(8, -12) / std::move(c1);
        REQUIRE(ret == complex{2, -3});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{4, 0, complex_prec_t(detail::real_deduce_precision(6.) + 1)};
        ret = std::complex<double>(8, -12) / std::move(c1);
        REQUIRE(ret == complex{2, -3});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }

#if defined(MPPP_WITH_QUADMATH)
    // Complex-complex128.
    {
        complex c1{11, 24, complex_prec_t(128)};
        auto ret = c1 / complex128(4, 5);
        REQUIRE(ret == complex{4, 1});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == 128);
        ret = complex128(35, 13) / c1;
        REQUIRE(ret == complex{1, -1});
        REQUIRE(std::is_same<complex, decltype(complex128(6, 7) / c1)>::value);
        REQUIRE(ret.get_prec() == 128);

        // Try with higher precision on the non-complex argument.
        c1 = complex{4, 0, complex_prec_t(real_prec_min())};
        ret = c1 / complex128(2, 2);
        REQUIRE(ret == complex{1, -1});
        REQUIRE(ret.get_prec() == 113);
        ret = complex128(8, -12) / c1;
        REQUIRE(ret == complex{2, -3});
        REQUIRE(ret.get_prec() == 113);

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) / complex128(2, 2);
        REQUIRE(ret == complex{1, -1});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{4, -8, complex_prec_t(114)};
        ret = std::move(c1) / complex128(2, 0);
        REQUIRE(ret == complex{2, -4});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = complex128(8, -12) / std::move(c1);
        REQUIRE(ret == complex{2, -3});
        REQUIRE(ret.get_prec() == 113);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1.is_valid());
        c1 = complex{4, 0, complex_prec_t(114)};
        ret = complex128(8, -12) / std::move(c1);
        REQUIRE(ret == complex{2, -3});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
#endif

    // real-std::complex.
    {
        real r{50, 5};
        auto ret = r / std::complex<double>{6, -8};
        REQUIRE(ret == complex{3, 4});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.));
        ret = std::complex<double>{50, 600} / r;
        REQUIRE(ret == complex{1, 12});
        REQUIRE(std::is_same<complex, decltype(std::complex<double>{5, 6} / r)>::value);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.));

        // Switch precisions around.
        r = real{50, detail::real_deduce_precision(5.) + 1};
        ret = r / std::complex<double>{6, -8};
        REQUIRE(ret == complex{3, 4});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.) + 1);
        ret = std::complex<double>{50, 600} / r;
        REQUIRE(ret == complex{1, 12});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.) + 1);

        // Check moves.
        r = real{50, detail::real_deduce_precision(5.) + 1};
        ret = std::complex<double>{50, 600} / std::move(r);
        REQUIRE(ret == complex{1, 12});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(5.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
    }

#if defined(MPPP_WITH_QUADMATH)
    // real-complex128.
    {
        real r{50, 5};
        auto ret = r / complex128{6, -8};
        REQUIRE(ret == complex{3, 4});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == 113);
        ret = complex128{50, 600} / r;
        REQUIRE(ret == complex{1, 12});
        REQUIRE(std::is_same<complex, decltype(complex128{5, 6} / r)>::value);
        REQUIRE(ret.get_prec() == 113);

        // Switch precisions around.
        r = real{50, 114};
        ret = r / complex128{6, -8};
        REQUIRE(ret == complex{3, 4});
        REQUIRE(ret.get_prec() == 114);
        ret = complex128{50, 600} / r;
        REQUIRE(ret == complex{1, 12});
        REQUIRE(ret.get_prec() == 114);

        // Check moves.
        r = real{50, 114};
        ret = complex128{50, 600} / std::move(r);
        REQUIRE(ret == complex{1, 12});
        REQUIRE(ret.get_prec() == 114);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
    }
#endif
}

TEST_CASE("in-place div")
{
    // complex-complex.
    {
        complex c1{909, -188}, c2{5, -6};
        c1 /= c2;
        REQUIRE(std::is_same<complex &, decltype(c1 /= c2)>::value);
        REQUIRE(c1 == complex{93, 74});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // Move which does not steal.
        c2 = complex{1, -2};
        c1 /= std::move(c2);
        REQUIRE(c1 == complex{-11, 52});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2.is_valid());

        // Move which steals.
        complex c3{3, 4, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        c1 /= std::move(c3);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c3.is_valid());
        REQUIRE(c3 == complex{-11, 52});
        REQUIRE(c1 == complex{7, 8});
        REQUIRE(c3.get_prec() == detail::real_deduce_precision(1));

        // Self div.
        c3 /= *&c3;
        REQUIRE(c3 == complex{1, 0});
    }
    // complex-real.
    {
        // Same precision.
        complex c1{8, -12};
        real r{4};
        c1 /= r;
        REQUIRE(std::is_same<complex &, decltype(c1 /= r)>::value);
        REQUIRE(c1 == complex{2, -3});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // r with higher precision.
        c1 = complex{8, -16, complex_prec_t(real_prec_min())};
        c1 /= r;
        REQUIRE(c1 == complex{2, -4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1));

        // r with smaller precision.
        c1 = complex{8, -16, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        c1 /= r;
        REQUIRE(c1 == complex{2, -4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1) + 1);
    }
    // complex-real valued.
    {
        // Other op with same precision.
        complex c1{8, -16, complex_prec_t(detail::real_deduce_precision(4.))};
        c1 /= 4.;
        REQUIRE(std::is_same<complex &, decltype(c1 /= 4.)>::value);
        REQUIRE(c1 == complex{2, -4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.));

        // Other op with higher precision.
        c1 = complex{8, -16, complex_prec_t(real_prec_min())};
        c1 /= 4;
        REQUIRE(c1 == complex{2, -4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4));

        // Other op with lower precision.
        c1 = complex{8, -16, complex_prec_t(detail::real_deduce_precision(1.) + 1)};
        c1 /= 4.;
        REQUIRE(c1 == complex{2, -4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.) + 1);
    }
    // complex-unsigned integral.
    {
        // Other op with same precision.
        complex c1{8u, 16u};
        c1 /= 4u;
        REQUIRE(std::is_same<complex &, decltype(c1 /= 4u)>::value);
        REQUIRE(c1 == complex{2, 4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1u));

        // Other op with higher precision.
        c1 = complex{8, -16, complex_prec_t(real_prec_min())};
        c1 /= 4u;
        REQUIRE(c1 == complex{2, -4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4u));

        // Other op with lower precision.
        c1 = complex{8, -16, complex_prec_t(detail::real_deduce_precision(1u) + 1)};
        c1 /= 4u;
        REQUIRE(c1 == complex{2, -4});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1u) + 1);

#if defined(MPPP_HAVE_GCC_INT128)
        // Test with large unsigned integral type.
        c1 = complex{8, -16, complex_prec_t(real_prec_min())};
        c1 /= __uint128_t(-1);
        REQUIRE(std::is_same<complex &, decltype(c1 /= __uint128_t(-1))>::value);
        REQUIRE(c1 == complex{8_q1 / __uint128_t(-1), -16_q1 / __uint128_t(-1), complex_prec_t(128)});
        REQUIRE(c1.get_prec() == 128);

        c1 = complex{8, -16, complex_prec_t(256)};
        c1 /= __uint128_t(-1);
        REQUIRE(c1 == complex{8_q1 / __uint128_t(-1), -16_q1 / __uint128_t(-1), complex_prec_t(256)});
        REQUIRE(c1.get_prec() == 256);
#endif
    }
    // Special casing for bool.
    {
        // Other op with same precision.
        complex c1{true, false};
        c1 /= true;
        REQUIRE(std::is_same<complex &, decltype(c1 /= true)>::value);
        REQUIRE(c1 == complex{1, 0});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true));

        // Other op with higher precision.
        c1 = complex{true, false, complex_prec_t(real_prec_min())};
        c1 /= true;
        REQUIRE(c1 == complex{1, 0});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true));

        // Other op with lower precision.
        c1 = complex{1, 2, complex_prec_t(detail::real_deduce_precision(true) + 1)};
        c1 /= false;
        REQUIRE(c1 == complex{"(inf,inf)", complex_prec_t(5)});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(true) + 1);
    }

    // complex-std::complex.
    {
        // Other op with same precision.
        complex c1{909, -188};
        c1 /= std::complex<double>{5, -6};
        REQUIRE(std::is_same<complex &, decltype(c1 /= std::complex<double>{3, 4})>::value);
        REQUIRE(c1 == complex{93, 74});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.));

        // Other op with higher precision.
        c1 = complex{8, -16, complex_prec_t(real_prec_min())};
        c1 /= std::complex<double>{2, -4};
        REQUIRE(c1 == complex{4, 0});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4.));

        // Other op with lower precision.
        c1 = complex{8, -16, complex_prec_t(detail::real_deduce_precision(1.) + 1)};
        c1 /= std::complex<double>{-2, 4};
        REQUIRE(c1 == complex{-4, 0});
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(1.) + 1);
    }

#if defined(MPPP_WITH_QUADMATH)
    // complex-complex128.
    {
        // Other op with same precision.
        complex c1{909, -188, complex_prec_t(113)};
        c1 /= complex128{5, -6};
        REQUIRE(std::is_same<complex &, decltype(c1 /= complex128{3, 4})>::value);
        REQUIRE(c1 == complex{93, 74});
        REQUIRE(c1.get_prec() == 113);

        // Other op with higher precision.
        c1 = complex{8, -16, complex_prec_t(real_prec_min())};
        c1 /= complex128{2, -4};
        REQUIRE(c1 == complex{4, 0});
        REQUIRE(c1.get_prec() == 113);

        // Other op with lower precision.
        c1 = complex{8, -16, complex_prec_t(114)};
        c1 /= std::complex<double>{-2, 4};
        REQUIRE(c1 == complex{-4, 0});
        REQUIRE(c1.get_prec() == 114);
    }
#endif

    // complex interoperable-complex.
    {
        int n = 4;
        n /= complex{-2, 0};
        REQUIRE(std::is_same<int &, decltype(n /= complex{4, 0})>::value);
        REQUIRE(n == -2);

        // Check move semantics.
        complex c{-2, 0, complex_prec_t(detail::real_deduce_precision(1) + 1)};
        n /= std::move(c);
        REQUIRE(n == 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());

        // Check conversion failure.
        REQUIRE_THROWS_AS((n /= complex{4, 1}), std::domain_error);
        REQUIRE(n == 1);
        if (std::numeric_limits<double>::has_quiet_NaN) {
            REQUIRE_THROWS_AS((n /= complex{std::numeric_limits<double>::quiet_NaN(), 0}), std::domain_error);
            REQUIRE(n == 1);
        }

        // Try with complex-valued too.
        std::complex<double> cd{8, 10};
        cd /= complex{-4, -5};
        REQUIRE(std::is_same<std::complex<double> &, decltype(cd /= complex{4, 1})>::value);
        REQUIRE(cd == std::complex<double>{-2, 0});

#if defined(MPPP_WITH_QUADMATH)
        complex128 cq{8, 10};
        cq /= complex{-4, -5};
        REQUIRE(std::is_same<complex128 &, decltype(cq /= complex{4, 1})>::value);
        REQUIRE(cq == complex128{-2, 0});
#endif
    }

    // real-complex valued
    {
        real r{8, 5};
        r /= std::complex<double>{4, 0};
        REQUIRE(std::is_same<real &, decltype(r /= std::complex<double>{4, 0})>::value);
        REQUIRE(r == 2);
        REQUIRE(r.get_prec() == detail::real_deduce_precision(1.));

        // Check conversion failure.
        REQUIRE_THROWS_AS((r /= complex{4, 1}), std::domain_error);
        REQUIRE(r == 2);

#if defined(MPPP_WITH_QUADMATH)
        r /= complex128{-2, 0};
        REQUIRE(std::is_same<real &, decltype(r /= complex128{4, 0})>::value);
        REQUIRE(r == -1);
        REQUIRE(r.get_prec() == detail::c_max(detail::real_deduce_precision(1.), mpfr_prec_t(113)));
#endif
    }

    // complex valued-real.
    {
        std::complex<double> c{4, -8};
        c /= real{2, 5};
        REQUIRE(std::is_same<std::complex<double> &, decltype(c /= real{2, 5})>::value);
        REQUIRE(c == std::complex<double>{2, -4});

        // Check move semantics.
        real r{2, detail::real_deduce_precision(1.) + 1};
        c /= std::move(r);
        REQUIRE(c == std::complex<double>{1, -2});
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());

#if defined(MPPP_WITH_QUADMATH)
        complex128 c2{8, 4};
        c2 /= real{2, 114};
        REQUIRE(std::is_same<complex128 &, decltype(c2 /= real{2, 114})>::value);
        REQUIRE(c2 == complex128{4, 2});
#endif
    }
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("eq ineq")
{
    // complex-complex.
    {
        REQUIRE(complex{1, 2} == complex{1, 2});
        REQUIRE(!(complex{1, 2} != complex{1, 2}));
        REQUIRE(complex{1, 2} != complex{2, 2});
        REQUIRE(!(complex{1, 2} == complex{2, 2}));

        // NaN testing.
        REQUIRE(!(complex{"(nan,0)", complex_prec_t(5)} == complex{"(nan,0)", complex_prec_t(5)}));
        REQUIRE(complex{"(nan,0)", complex_prec_t(5)} != complex{"(nan,0)", complex_prec_t(5)});
        REQUIRE(!(complex{"(nan,0)", complex_prec_t(5)} == complex{"(2,0)", complex_prec_t(5)}));
        REQUIRE(complex{"(nan,0)", complex_prec_t(5)} != complex{"(2,0)", complex_prec_t(5)});

        REQUIRE(!(complex{"(0,nan)", complex_prec_t(5)} == complex{"(0,nan)", complex_prec_t(5)}));
        REQUIRE(complex{"(0,nan)", complex_prec_t(5)} != complex{"(0,nan)", complex_prec_t(5)});
        REQUIRE(!(complex{"(0,nan)", complex_prec_t(5)} == complex{"(2,nan)", complex_prec_t(5)}));
        REQUIRE(complex{"(0,nan)", complex_prec_t(5)} != complex{"(2,nan)", complex_prec_t(5)});

        REQUIRE(!(complex{"(nan,nan)", complex_prec_t(5)} == complex{"(nan,nan)", complex_prec_t(5)}));
        REQUIRE(complex{"(nan,nan)", complex_prec_t(5)} != complex{"(nan,nan)", complex_prec_t(5)});
    }
    // complex-real valued (except signed integral).
    {
        REQUIRE(complex{2, 0} == 2.);
        REQUIRE(complex{-2, -0.} == real{-2});
        REQUIRE(!(complex{2, 0} != 2_z1));
        REQUIRE(!(complex{-2, -0.} != -2_q1));
        REQUIRE(2. == complex{2, 0});
        REQUIRE(real{-2} == complex{-2, -0.});
        REQUIRE(!(2_z1 != complex{2, 0}));
        REQUIRE(!(-2_q1 != complex{-2, -0.}));
#if defined(MPPP_WITH_QUADMATH)
        REQUIRE(complex{2, 0} == 2_rq);
        REQUIRE(!(complex{2, 0} != 2_rq));
        REQUIRE(2_rq == complex{2, 0});
        REQUIRE(!(2_rq != complex{2, 0}));
#endif

        REQUIRE(!(complex{3, 0} == 2.));
        REQUIRE(!(complex{-3, -0.} == real{-2}));
        REQUIRE(complex{3, 0} != 2_z1);
        REQUIRE(complex{-3, -0.} != 2_q1);
        REQUIRE(!(2. == complex{3, 0}));
        REQUIRE(!(real{-2} == complex{-3, -0.}));
        REQUIRE(2_z1 != complex{3, 0});
        REQUIRE(2_q1 != complex{-3, -0.});
#if defined(MPPP_WITH_QUADMATH)
        REQUIRE(!(complex{3, 0} == 2_rq));
        REQUIRE(complex{3, 0} != 2_rq);
        REQUIRE(!(2_rq == complex{3, 0}));
        REQUIRE(2_rq != complex{3, 0});
#endif

        REQUIRE(!(complex{2, 1} == 2.));
        REQUIRE(!(complex{-2, -1.} == real{-2}));
        REQUIRE(complex{2, 1} != 2_z1);
        REQUIRE(complex{-2, -1.} != 2_q1);
        REQUIRE(!(2. == complex{2, 1}));
        REQUIRE(!(real{-2} == complex{-2, -1.}));
        REQUIRE(2_z1 != complex{2, 1});
        REQUIRE(2_q1 != complex{-2, -1.});
#if defined(MPPP_WITH_QUADMATH)
        REQUIRE(!(complex{3, 1} == 2_rq));
        REQUIRE(complex{3, 1} != 2_rq);
        REQUIRE(!(2_rq == complex{3, 1}));
        REQUIRE(2_rq != complex{3, 1});
#endif

        // NaN testing.
        REQUIRE(!(complex{"(nan,0)", complex_prec_t(5)} == 1.));
        REQUIRE(complex{"(nan,0)", complex_prec_t(5)} != 1.);
        REQUIRE(!(complex{"(1.,nan)", complex_prec_t(5)} == 1.));
        REQUIRE(complex{"(1.,nan)", complex_prec_t(5)} != 1.);
        REQUIRE(!(1. == complex{"(nan,0)", complex_prec_t(5)}));
        REQUIRE(1. != complex{"(nan,0)", complex_prec_t(5)});
        REQUIRE(!(1. == complex{"(1.,nan)", complex_prec_t(5)}));
        REQUIRE(1. != complex{"(1.,nan)", complex_prec_t(5)});
#if defined(MPPP_WITH_QUADMATH)
        REQUIRE(!(complex{"(nan,0)", complex_prec_t(5)} == 1_rq));
        REQUIRE(complex{"(nan,0)", complex_prec_t(5)} != 1_rq);
        REQUIRE(!(complex{"(1.,nan)", complex_prec_t(5)} == 1_rq));
        REQUIRE(complex{"(1.,nan)", complex_prec_t(5)} != 1_rq);
        REQUIRE(!(1_rq == complex{"(nan,0)", complex_prec_t(5)}));
        REQUIRE(1_rq != complex{"(nan,0)", complex_prec_t(5)});
        REQUIRE(!(1_rq == complex{"(1.,nan)", complex_prec_t(5)}));
        REQUIRE(1_rq != complex{"(1.,nan)", complex_prec_t(5)});
#endif
    }
    // complex-signed integral.
    {
        REQUIRE(complex{2, 0} == 2);
        REQUIRE(!(complex{2, 0} != 2l));
        REQUIRE(2 == complex{2, 0});
        REQUIRE(!(2l != complex{2, 0}));

        REQUIRE(!(complex{3, 0} == 2));
        REQUIRE(!(complex{-3, -0.} == -2l));
        REQUIRE(complex{3, 0} != short(2));
        REQUIRE(complex{-3, -0.} != static_cast<signed char>(2));
        REQUIRE(!(2 == complex{3, 0}));
        REQUIRE(!(-2l == complex{-3, -0.}));
        REQUIRE(short(2) != complex{3, 0});
        REQUIRE(static_cast<signed char>(2) != complex{-3, -0.});

        REQUIRE(!(complex{2, 1} == 2));
        REQUIRE(!(complex{-2, -1.} == -2ll));
        REQUIRE(complex{2, 1} != 2);
        REQUIRE(complex{-2, -1.} != 2);
        REQUIRE(!(2 == complex{2, 1}));
        REQUIRE(!(-2ll == complex{-2, -1.}));
        REQUIRE(2 != complex{2, 1});
        REQUIRE(2 != complex{-2, -1.});

        // NaN testing.
        REQUIRE(!(complex{"(nan,0)", complex_prec_t(5)} == 1));
        REQUIRE(complex{"(nan,0)", complex_prec_t(5)} != 1l);
        REQUIRE(!(complex{"(1.,nan)", complex_prec_t(5)} == 1ll));
        REQUIRE(complex{"(1.,nan)", complex_prec_t(5)} != 1ll);
        REQUIRE(!(1 == complex{"(nan,0)", complex_prec_t(5)}));
        REQUIRE(1l != complex{"(nan,0)", complex_prec_t(5)});
        REQUIRE(!(1ll == complex{"(1.,nan)", complex_prec_t(5)}));
        REQUIRE(1ll != complex{"(1.,nan)", complex_prec_t(5)});
    }
    // complex-complex valued.
    {
        REQUIRE(complex{1, 2} == std::complex<double>{1, 2});
        REQUIRE(std::complex<double>{1, 2} == complex{1, 2});
        REQUIRE(!(complex{1, 2} != std::complex<double>{1, 2}));
        REQUIRE(!(std::complex<double>{1, 2} != complex{1, 2}));

        REQUIRE(complex{1, 3} != std::complex<double>{1, 2});
        REQUIRE(std::complex<double>{1, 2} != complex{1, 3});
        REQUIRE(!(complex{1, 3} == std::complex<double>{1, 2}));
        REQUIRE(!(std::complex<double>{1, 2} == complex{1, 3}));

        REQUIRE(complex{1, 2} != std::complex<double>{2, 2});
        REQUIRE(std::complex<double>{2, 2} != complex{1, 2});
        REQUIRE(!(complex{1, 2} == std::complex<double>{2, 2}));
        REQUIRE(!(std::complex<double>{2, 2} == complex{1, 2}));

#if defined(MPPP_WITH_QUADMATH)
        REQUIRE(complex{1, 2} == complex128{1, 2});
        REQUIRE(complex128{1, 2} == complex{1, 2});
        REQUIRE(!(complex{1, 2} != complex128{1, 2}));
        REQUIRE(!(complex128{1, 2} != complex{1, 2}));

        REQUIRE(complex{1, 3} != complex128{1, 2});
        REQUIRE(complex128{1, 2} != complex{1, 3});
        REQUIRE(!(complex{1, 3} == complex128{1, 2}));
        REQUIRE(!(complex128{1, 2} == complex{1, 3}));

        REQUIRE(complex{1, 2} != complex128{2, 2});
        REQUIRE(complex128{2, 2} != complex{1, 2});
        REQUIRE(!(complex{1, 2} == complex128{2, 2}));
        REQUIRE(!(complex128{2, 2} == complex{1, 2}));
#endif

        // NaN testing.
        if (std::numeric_limits<double>::has_quiet_NaN) {
            const auto dnan = std::numeric_limits<double>::quiet_NaN();

            REQUIRE(complex{"(1, nan)", complex_prec_t(5)} != std::complex<double>{1, dnan});
            REQUIRE(std::complex<double>{1, dnan} != complex{"(1, nan)", complex_prec_t(5)});
            REQUIRE(!(complex{"(1, nan)", complex_prec_t(5)} == std::complex<double>{1, dnan}));
            REQUIRE(!(std::complex<double>{1, dnan} == complex{"(1, nan)", complex_prec_t(5)}));

            REQUIRE(complex{"(nan,1)", complex_prec_t(5)} != std::complex<double>{dnan, 1});
            REQUIRE(std::complex<double>{dnan, 1} != complex{"(nan,1)", complex_prec_t(5)});
            REQUIRE(!(complex{"(nan,1)", complex_prec_t(5)} == std::complex<double>{dnan, 1}));
            REQUIRE(!(std::complex<double>{dnan, 1} == complex{"(nan,1)", complex_prec_t(5)}));

            REQUIRE(complex{"(nan,nan)", complex_prec_t(5)} != std::complex<double>{dnan, dnan});
            REQUIRE(std::complex<double>{dnan, dnan} != complex{"(nan,nan)", complex_prec_t(5)});
            REQUIRE(!(complex{"(nan,nan)", complex_prec_t(5)} == std::complex<double>{dnan, dnan}));
            REQUIRE(!(std::complex<double>{dnan, dnan} == complex{"(nan,nan)", complex_prec_t(5)}));
        }
    }

#if defined(MPPP_WITH_QUADMATH)
    REQUIRE(complex{"(1, nan)", complex_prec_t(5)} != complex128{"(1,nan)"});
    REQUIRE(complex128{"(1,nan)"} != complex{"(1, nan)", complex_prec_t(5)});
    REQUIRE(!(complex{"(1, nan)", complex_prec_t(5)} == complex128{"(1,nan)"}));
    REQUIRE(!(complex128{"(1,nan)"} == complex{"(1, nan)", complex_prec_t(5)}));

    REQUIRE(complex{"(nan,1)", complex_prec_t(5)} != complex128{"(nan,1)"});
    REQUIRE(complex128{"(nan,1)"} != complex{"(nan,1)", complex_prec_t(5)});
    REQUIRE(!(complex{"(nan,1)", complex_prec_t(5)} == complex128{"(nan,1)"}));
    REQUIRE(!(complex128{"(nan,1)"} == complex{"(nan,1)", complex_prec_t(5)}));

    REQUIRE(complex{"(nan,nan)", complex_prec_t(5)} != complex128{"(nan,nan)"});
    REQUIRE(complex128{"(nan,nan)"} != complex{"(nan,nan)", complex_prec_t(5)});
    REQUIRE(!(complex{"(nan,nan)", complex_prec_t(5)} == complex128{"(nan,nan)"}));
    REQUIRE(!(complex128{"(nan,nan)"} == complex{"(nan,nan)", complex_prec_t(5)}));
#endif
}
