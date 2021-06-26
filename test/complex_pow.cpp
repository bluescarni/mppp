// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <complex>
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

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("pow")
{
    // Ternary pow.
    {
        complex c1, c2, c3;
        mppp::pow(c1, c2, c3);
        REQUIRE(std::is_same<complex &, decltype(mppp::pow(c1, c2, c3))>::value);
        REQUIRE(c1 == 1);
        REQUIRE(c1.get_prec() == real_prec_min());

        c2 = 4;
        c3 = 2;
        mppp::pow(c1, c2, c3);
        REQUIRE(c1 == 16);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4));

        c2 = complex{4, complex_prec_t(detail::real_deduce_precision(4) + 1)};
        c3 = 2;
        mppp::pow(c1, c2, c3);
        REQUIRE(c1 == 16);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4) + 1);

        c2 = 2;
        c3 = complex{3, complex_prec_t(detail::real_deduce_precision(4) + 2)};
        mppp::pow(c1, c2, c3);
        REQUIRE(c1 == 8);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4) + 2);

        // Overlapping.
        c1 = 4;
        mppp::pow(c1, c1, c1);
        REQUIRE(c1 == 256);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4));

        c2 = 2;
        c1 = 4;
        mppp::pow(c1, c2, c1);
        REQUIRE(c1 == 16);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4));

        // Check moves.
        c1 = complex{};
        c2 = 3;
        c3 = 4;
        mppp::pow(c1, std::move(c2), c3);
        REQUIRE(c1 == 81);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2.is_valid());
        REQUIRE(c2.zero_p());
        REQUIRE(c2.get_prec() == real_prec_min());

        c1 = complex{};
        c2 = 3;
        c3 = 4;
        mppp::pow(c1, c2, std::move(c3));
        REQUIRE(c1 == 81);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c3.is_valid());
        REQUIRE(c3.zero_p());
        REQUIRE(c3.get_prec() == real_prec_min());

        c1 = complex{};
        c2 = 3;
        c3 = complex{4, complex_prec_t(detail::real_deduce_precision(4) + 1)};
        mppp::pow(c1, std::move(c2), std::move(c3));
        REQUIRE(c1 == 81);
        REQUIRE(c1.get_prec() == detail::real_deduce_precision(4) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c3.is_valid());
        REQUIRE(c3.zero_p());
        REQUIRE(c3.get_prec() == real_prec_min());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == 3);
        REQUIRE(c2.get_prec() == detail::real_deduce_precision(4));
    }

    // Complex-complex.
    {
        complex c1{4}, c2{2};
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c1, c2))>::value);
        REQUIRE(mppp::pow(c1, c2) == 16);
        REQUIRE(mppp::pow(c1, c2).get_prec() == detail::real_deduce_precision(4));

        c2 = complex{2, complex_prec_t(2)};
        REQUIRE(mppp::pow(c1, c2) == 16);
        REQUIRE(mppp::pow(c1, c2).get_prec() == detail::real_deduce_precision(4));

        c2 = 2;
        c1 = complex{4, complex_prec_t(2)};
        REQUIRE(mppp::pow(c1, c2) == 16);
        REQUIRE(mppp::pow(c1, c2).get_prec() == detail::real_deduce_precision(4));

        // Check moves.
        c1 = complex{4, complex_prec_t(64)};
        c2 = complex{2, complex_prec_t(32)};
        auto ret = mppp::pow(std::move(c1), c2);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == 64);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
        c1 = complex{4, complex_prec_t(16)};
        ret = mppp::pow(std::move(c1), std::move(c2));
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == 32);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c2.is_valid());
    }
    // Complex-real.
    {
        complex c{4};
        real r{2};
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, r))>::value);
        REQUIRE(mppp::pow(c, r) == 16);
        REQUIRE(mppp::pow(c, r).get_prec() == detail::real_deduce_precision(4));

        r = real{2, 2};
        REQUIRE(mppp::pow(c, r) == 16);
        REQUIRE(mppp::pow(c, r).get_prec() == detail::real_deduce_precision(4));

        r = 2;
        c = complex{4, complex_prec_t(2)};
        REQUIRE(mppp::pow(c, r) == 16);
        REQUIRE(mppp::pow(c, r).get_prec() == detail::real_deduce_precision(4));

        // Check moves.
        c = complex{4, complex_prec_t(64)};
        r = real{2, 32};
        auto ret = mppp::pow(std::move(c), r);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == 64);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(16)};
        ret = mppp::pow(std::move(c), std::move(r));
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == 32);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r.is_valid());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());
    }
    // Complex-integer.
    {
        complex c{4};
        auto n = 2_z1;
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, n))>::value);
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(2_z1)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec() == detail::real_deduce_precision(2_z1));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2_z1) + 1)};
        auto ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2_z1) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2_z1) - 1)};
        ret = mppp::pow(std::move(c), std::move(n));
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2_z1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());
    }
    // Complex-unsigned integral.
    {
        complex c{4};
        auto n = 2u;
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, n))>::value);
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(2u)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec() == detail::real_deduce_precision(2u));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2u) + 1)};
        auto ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2u) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2u) - 1)};
        ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2u));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());

#if defined(MPPP_HAVE_GCC_INT128)
        // Try large value.
        c = complex{0, complex_prec_t(64)};
        REQUIRE(mppp::pow(c, __uint128_t(-1)) == 0);
        REQUIRE(mppp::pow(c, __uint128_t(-1)).get_prec() == 128);

        ret = mppp::pow(std::move(c), __uint128_t(-1));
        REQUIRE(ret == 0);
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());

        c = complex{0, complex_prec_t(256)};
        ret = mppp::pow(std::move(c), __uint128_t(-1));
        REQUIRE(ret == 0);
        REQUIRE(ret.get_prec() == 256);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
#endif

        // Special casing for bool.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(true) + 1)};
        REQUIRE(mppp::pow(c, true) == 4);
        REQUIRE(mppp::pow(c, true).get_prec() == detail::real_deduce_precision(true) + 1);

        ret = mppp::pow(std::move(c), false);
        REQUIRE(ret == 1);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(true) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
    }
    // Complex-signed integral.
    {
        complex c{4};
        auto n = 2;
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, n))>::value);
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(2)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec() == detail::real_deduce_precision(2));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2) + 1)};
        auto ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2) - 1)};
        ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());

#if defined(MPPP_HAVE_GCC_INT128)
        // Try large value.
        c = complex{0, complex_prec_t(64)};
        auto big_n = __int128_t(1) << 87;
        REQUIRE(mppp::pow(c, big_n) == 0);
        REQUIRE(mppp::pow(c, big_n).get_prec() == 128);

        ret = mppp::pow(std::move(c), big_n);
        REQUIRE(ret == 0);
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());

        c = complex{0, complex_prec_t(256)};
        ret = mppp::pow(std::move(c), big_n);
        REQUIRE(ret == 0);
        REQUIRE(ret.get_prec() == 256);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
#endif
    }
    // Complex-float.
    {
        complex c{4};
        auto n = 2.f;
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, n))>::value);
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(2.f)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec() == detail::real_deduce_precision(2.f));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2.f) + 1)};
        auto ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2.f) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2.f) - 1)};
        ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2.f));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());
    }
    // Complex-double.
    {
        complex c{4};
        auto n = 2.;
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, n))>::value);
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(2.)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec() == detail::real_deduce_precision(2.));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2.) + 1)};
        auto ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2.) - 1)};
        ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());
    }
    // Complex-long double.
    {
        complex c{4};
        auto n = 2.l;
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, n))>::value);
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(2.l)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec() == detail::real_deduce_precision(2.l));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2.l) + 1)};
        auto ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2.l) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2.l) - 1)};
        ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2.l));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());
    }
    // Complex-rational.
    {
        complex c{4};
        auto n = 2_q1;
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, n))>::value);
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(2_q1)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec() == detail::real_deduce_precision(2_q1));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2_q1) + 1)};
        auto ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2_q1) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2_q1) - 1)};
        ret = mppp::pow(std::move(c), std::move(n));
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2_q1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());

        // Check a fractional exponent.
        n = 1 / 3_q1;
        const auto prec_n = detail::real_deduce_precision(n);
        ret = mppp::pow(complex{4, -2, complex_prec_t(real_prec_min())}, n);
        REQUIRE(
            abs(ret
                - complex{"(1.6279118765495024499290053274773304113288,-0.25361494195725942470941540875401078084663)",
                          complex_prec_t(prec_n)})
            < mppp::pow(real{2}, -(prec_n - 3)));

        if (prec_n < 256) {
            ret = mppp::pow(complex{4, -2, complex_prec_t(256)}, n);
            REQUIRE(abs(ret
                        - complex{"(1.627911876549502449929005327477330411328839929022204293845168284041373445605524,-"
                                  "0.2536149419572594247094154087540107808466256405866009492031553391634621868551176)",
                                  complex_prec_t(256)})
                    < mppp::pow(real{2}, -250));
        }
    }
#if defined(MPPP_WITH_QUADMATH)
    // Complex-real128.
    {
        complex c{4};
        auto n = 2_rq;
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, n))>::value);
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(2_rq)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(c, n) == 16);
        REQUIRE(mppp::pow(c, n).get_prec() == detail::real_deduce_precision(2_rq));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2_rq) + 1)};
        auto ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2_rq) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2_rq) - 1)};
        ret = mppp::pow(std::move(c), n);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2_rq));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());
    }
#endif
    // Complex-std::complex.
    {
        complex c{4};
        auto x = std::complex<double>{2, 0};
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, x))>::value);
        REQUIRE(mppp::pow(c, x) == 16);
        REQUIRE(mppp::pow(c, x).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(x)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(c, x) == 16);
        REQUIRE(mppp::pow(c, x).get_prec() == detail::real_deduce_precision(x));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(x) + 1)};
        auto ret = mppp::pow(std::move(c), x);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(x) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(x) - 1)};
        ret = mppp::pow(std::move(c), x);
        REQUIRE(ret == 16);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(x));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());
    }
#if defined(MPPP_WITH_QUADMATH)
    // Complex-complex128.
    {
        complex c{4};
        auto x = complex128{2, 0};
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, x))>::value);
        REQUIRE(mppp::pow(c, x) == 16);
        REQUIRE(mppp::pow(c, x).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(x)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(c, x) == 16);
        REQUIRE(mppp::pow(c, x).get_prec() == detail::real_deduce_precision(x));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(x) + 1)};
        auto ret = mppp::pow(std::move(c), x);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(x) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(x) - 1)};
        ret = mppp::pow(std::move(c), x);
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(x));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());
    }
#endif

    // real valued-complex.
    {
        complex c{4};
        auto x = 2.;
        REQUIRE(std::is_same<complex, decltype(mppp::pow(x, c))>::value);
        REQUIRE(mppp::pow(x, c) == 16);
        REQUIRE(mppp::pow(x, c).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(2.)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(x, c) == 16);
        REQUIRE(mppp::pow(x, c).get_prec() == detail::real_deduce_precision(2.));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2.) + 1)};
        auto ret = mppp::pow(x, std::move(c));
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2.) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(2.) - 1)};
        ret = mppp::pow(x, std::move(c));
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(2.));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());
    }
    // complex valued-complex.
    {
        complex c{4};
        auto x = std::complex<double>{2, 0};
        REQUIRE(std::is_same<complex, decltype(mppp::pow(x, c))>::value);
        REQUIRE(mppp::pow(x, c) == 16);
        REQUIRE(mppp::pow(x, c).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(x)));

        c = complex{4, complex_prec_t(real_prec_min())};
        REQUIRE(mppp::pow(x, c) == 16);
        REQUIRE(mppp::pow(x, c).get_prec() == detail::real_deduce_precision(x));

        // Check moves.
        c = complex{4, complex_prec_t(detail::real_deduce_precision(x) + 1)};
        auto ret = mppp::pow(x, std::move(c));
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(x) + 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(detail::real_deduce_precision(x) - 1)};
        ret = mppp::pow(x, std::move(c));
        REQUIRE(ret == 16);
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(x));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());
    }

    // real-complex valued.
    {
        real r{4};
        auto c = std::complex<double>{3, 0};
        REQUIRE(std::is_same<complex, decltype(mppp::pow(r, c))>::value);
        REQUIRE(mppp::pow(r, c) == 64);
        REQUIRE(mppp::pow(r, c).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(c)));

        r = real{4, real_prec_min()};
        REQUIRE(mppp::pow(r, c) == 64);
        REQUIRE(mppp::pow(r, c).get_prec() == detail::real_deduce_precision(c));
    }

    // complex valued-real.
    {
        real r{4};
        auto c = std::complex<double>{3, 0};
        REQUIRE(std::is_same<complex, decltype(mppp::pow(c, r))>::value);
        REQUIRE(mppp::pow(c, r) == 81);
        REQUIRE(mppp::pow(c, r).get_prec()
                == std::max<mpfr_prec_t>(detail::real_deduce_precision(4), detail::real_deduce_precision(c)));

        r = real{4, real_prec_min()};
        REQUIRE(mppp::pow(c, r) == 81);
        REQUIRE(mppp::pow(c, r).get_prec() == detail::real_deduce_precision(c));
    }
}
