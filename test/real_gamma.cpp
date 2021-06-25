// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <utility>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real gamma")
{
    real r0{1};
    r0.gamma();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    real rop;
    r0 = real{1};
    REQUIRE(gamma(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(gamma(r0) == 1);
    REQUIRE(gamma(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real lgamma")
{
    real r0{1};
    r0.lgamma();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{1};
    REQUIRE(lgamma(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(lgamma(r0) == 0);
    REQUIRE(lgamma(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real lngamma")
{
    real r0{1};
    r0.lngamma();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{1};
    REQUIRE(lngamma(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(lngamma(r0) == 0);
    REQUIRE(lngamma(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real digamma")
{
    real r0{2};
    r0.digamma();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(r0 - (digamma(real{3}) - 1 / real{2})) < 1E-8);
    real rop;
    r0 = real{2};
    REQUIRE(abs(digamma(rop, r0) - (digamma(real{3}) - 1 / real{2})) < 1E-8);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = real{2};
    REQUIRE(abs(digamma(r0) - (digamma(real{3}) - 1 / real{2})) < 1E-8);
    REQUIRE(abs(digamma(std::move(r0)) - (digamma(real{3}) - 1 / real{2})) < 1E-8);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

#if defined(MPPP_MPFR_HAVE_MPFR_GAMMA_INC)

TEST_CASE("real gamma_inc")
{
    real r0{12, 450};
    gamma_inc(r0, real{4}, real{5});
    REQUIRE(abs(r0 - (3 * gamma_inc(real{3}, real{5}) + pow(real{5}, 3) * exp(-real{5}))) < 1E-8);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    real tmp1{4}, tmp2{5};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    gamma_inc(r0, std::move(tmp1), tmp2);
    REQUIRE(abs(r0 - (3 * gamma_inc(real{3}, real{5}) + pow(real{5}, 3) * exp(-real{5}))) < 1E-8);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp1 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp1 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp1.get_prec() == detail::real_deduce_precision(0) / 2);
    tmp1 = real{4};
    tmp2 = real{5};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    gamma_inc(r0, tmp1, std::move(tmp2));
    REQUIRE(abs(r0 - (3 * gamma_inc(real{3}, real{5}) + pow(real{5}, 3) * exp(-real{5}))) < 1E-8);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp2 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp2 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp2.get_prec() == detail::real_deduce_precision(0) / 2);

    // Some tests for the binary form too.
    REQUIRE(abs(gamma_inc(real{4}, real{5}) - (3 * gamma_inc(real{3}, real{5}) + pow(real{5}, 3) * exp(-real{5})))
            < 1E-8);
    REQUIRE(gamma_inc(real{4, 20}, real{5, 30}).get_prec() == 30);
    REQUIRE(gamma_inc(real{4}, 5.) == gamma_inc(real{4}, real{5.}));
    REQUIRE(gamma_inc(5., real{4}) == gamma_inc(real{5.}, real{4}));
    REQUIRE(gamma_inc(real{4}, 5) == gamma_inc(real{4}, real{5}));
    REQUIRE(gamma_inc(5, real{4}) == gamma_inc(real{5}, real{4}));
    REQUIRE(gamma_inc(-5., real{4}) == gamma_inc(real{-5.}, real{4}));
    REQUIRE(gamma_inc(-5, real{4}) == gamma_inc(real{-5}, real{4}));
    REQUIRE(gamma_inc(real{4}, integer<1>{5}) == gamma_inc(real{4}, real{integer<1>{5}}));
    REQUIRE(gamma_inc(integer<1>{-5}, real{4}) == gamma_inc(real{integer<1>{-5}}, real{4}));
    REQUIRE(gamma_inc(real{4, detail::real_deduce_precision(0.) / 2}, 5.).get_prec()
            == detail::real_deduce_precision(0.));
    REQUIRE(gamma_inc(4., real{5, detail::real_deduce_precision(0.) / 2}).get_prec()
            == detail::real_deduce_precision(0.));
    REQUIRE(gamma_inc(real{4, detail::real_deduce_precision(0) / 2}, 5).get_prec() == detail::real_deduce_precision(0));
    REQUIRE(gamma_inc(4, real{5, detail::real_deduce_precision(0) / 2}).get_prec() == detail::real_deduce_precision(0));
}

#endif
