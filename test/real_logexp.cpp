// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>
#include <utility>

#include <mp++/config.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real exp")
{
    real r0{0};
    r0.exp();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    real rop;
    r0 = real{0};
    REQUIRE(exp(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(exp(r0) == 1);
    REQUIRE(exp(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real exp2")
{
    real r0{0};
    r0.exp2();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    real rop;
    r0 = real{0};
    REQUIRE(exp2(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(exp2(r0) == 1);
    REQUIRE(exp2(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(exp2(real{4}) == 16);
    REQUIRE(exp2(real{-4}) == 1 / exp2(real{4}));
    r0 = real{4};
    r0.exp2();
    REQUIRE(r0 == 16);
}

TEST_CASE("real exp10")
{
    real r0{0};
    r0.exp10();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    real rop;
    r0 = real{0};
    REQUIRE(exp10(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(exp10(r0) == 1);
    REQUIRE(exp10(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(exp10(real{4}) == 10000);
    REQUIRE(exp10(real{-4}) == 1 / exp10(real{4}));
    r0 = real{4};
    r0.exp10();
    REQUIRE(r0 == 10000);
}

TEST_CASE("real expm1")
{
    real r0{0};
    r0.expm1();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{0};
    REQUIRE(expm1(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(expm1(r0) == 0);
    REQUIRE(expm1(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(expm1(real{4}) == exp(real{4}) - 1);
    REQUIRE(expm1(real{-4}) == exp(real{-4}) - 1);
    r0 = real{4};
    r0.expm1();
    REQUIRE(r0 == exp(real{4}) - 1);
}

TEST_CASE("real log")
{
    real r0{1};
    r0.log();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{1};
    REQUIRE(log(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(log(r0) == 0);
    REQUIRE(log(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real log2")
{
    real r0{1};
    r0.log2();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{1};
    REQUIRE(log2(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(log2(r0) == 0);
    REQUIRE(log2(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(log2(real{4}) == 2);
    REQUIRE(log2(real{-4}).nan_p());
    r0 = real{4};
    r0.log2();
    REQUIRE(r0 == 2);
}

TEST_CASE("real log10")
{
    real r0{1};
    r0.log10();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{1};
    REQUIRE(log10(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(log10(r0) == 0);
    REQUIRE(log10(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(log10(real{100}) == 2);
    REQUIRE(log10(real{-100}).nan_p());
    r0 = real{100};
    r0.log10();
    REQUIRE(r0 == 2);
}

TEST_CASE("real log1p")
{
    real r0{0};
    r0.log1p();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{0};
    REQUIRE(log1p(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(log1p(r0) == 0);
    REQUIRE(log1p(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(log1p(real{99}) == log(real{100}));
    REQUIRE(log1p(real{-99}).nan_p());
    r0 = real{99};
    r0.log1p();
    REQUIRE(r0 == log(real{100}));
}

#if defined(MPPP_WITH_ARB)

TEST_CASE("real log_hypot")
{
    real r0{12, 450};
    log_hypot(r0, 1.2_r128, 1.3_r128);
    REQUIRE(abs(r0 - 0.57051650227603092432139119994424096918528_r128) < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);

    real tmp1{4, 32}, tmp2{5, 32};
    r0 = real{12, 16};
    log_hypot(r0, std::move(tmp1), tmp2);
    REQUIRE(abs(r0 - 1.85678603314) < 1E-8);
    REQUIRE(r0.get_prec() == 32);
    // Check tmp1 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp1 == real{12, 16});
    REQUIRE(tmp1.get_prec() == 16);
    tmp1 = real{4, 32};
    tmp2 = real{5, 32};
    r0 = real{12, 16};
    log_hypot(r0, tmp1, std::move(tmp2));
    REQUIRE(abs(r0 - 1.85678603314) < 1E-8);
    REQUIRE(r0.get_prec() == 32);
    // Check tmp2 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp2 == real{12, 16});
    REQUIRE(tmp2.get_prec() == 16);

    // Some tests for the binary form too.
    REQUIRE(abs(log_hypot(real{4, 32}, real{5, 32}) - 1.85678603314) < 1E-8);
    REQUIRE(log_hypot(real{4, 20}, real{5, 30}).get_prec() == 30);
    REQUIRE(log_hypot(real{4}, 5.) == log_hypot(real{4}, real{5.}));
    REQUIRE(log_hypot(5., real{4}) == log_hypot(real{5.}, real{4}));
    REQUIRE(log_hypot(real{4}, 5) == log_hypot(real{4}, real{5}));
    REQUIRE(log_hypot(5, real{4}) == log_hypot(real{5}, real{4}));
    REQUIRE(log_hypot(-5., real{4}) == log_hypot(real{-5.}, real{4}));
    REQUIRE(log_hypot(-5, real{4}) == log_hypot(real{-5}, real{4}));
    REQUIRE(log_hypot(real{4}, integer<1>{5}) == log_hypot(real{4}, real{integer<1>{5}}));
    REQUIRE(log_hypot(integer<1>{-5}, real{4}) == log_hypot(real{integer<1>{-5}}, real{4}));
    REQUIRE(log_hypot(real{4, detail::real_deduce_precision(0.) / 2}, 5.).get_prec()
            == detail::real_deduce_precision(0.));
    REQUIRE(log_hypot(4., real{5, detail::real_deduce_precision(0.) / 2}).get_prec()
            == detail::real_deduce_precision(0.));
    REQUIRE(log_hypot(real{4, detail::real_deduce_precision(0) / 2}, 5).get_prec() == detail::real_deduce_precision(0));
    REQUIRE(log_hypot(4, real{5, detail::real_deduce_precision(0) / 2}).get_prec() == detail::real_deduce_precision(0));

    // Test infinities.
    REQUIRE(log_hypot(real{"inf", 32}, real{3, 25}).inf_p());
    REQUIRE(log_hypot(real{"inf", 32}, real{3, 25}) > 0);
    REQUIRE(log_hypot(real{"inf", 32}, real{3, 25}).get_prec() == 32);
    REQUIRE(log_hypot(real{"-inf", 32}, real{3, 25}).inf_p());
    REQUIRE(log_hypot(real{"-inf", 32}, real{3, 25}) > 0);
    REQUIRE(log_hypot(real{"-inf", 32}, real{3, 25}).get_prec() == 32);

    REQUIRE(log_hypot(real{3, 25}, real{"inf", 32}).inf_p());
    REQUIRE(log_hypot(real{3, 25}, real{"inf", 32}) > 0);
    REQUIRE(log_hypot(real{3, 25}, real{"inf", 32}).get_prec() == 32);
    REQUIRE(log_hypot(real{3, 25}, real{"-inf", 32}).inf_p());
    REQUIRE(log_hypot(real{3, 25}, real{"-inf", 32}) > 0);
    REQUIRE(log_hypot(real{3, 25}, real{"-inf", 32}).get_prec() == 32);

    REQUIRE(log_hypot(real{"inf", 25}, real{"inf", 32}).inf_p());
    REQUIRE(log_hypot(real{"inf", 25}, real{"inf", 32}) > 0);
    REQUIRE(log_hypot(real{"inf", 25}, real{"inf", 32}).get_prec() == 32);
    REQUIRE(log_hypot(real{"-inf", 25}, real{"inf", 32}).inf_p());
    REQUIRE(log_hypot(real{"inf", 25}, real{"-inf", 32}) > 0);
    REQUIRE(log_hypot(real{"-inf", 25}, real{"-inf", 32}).get_prec() == 32);

    // Test nans.
    REQUIRE(log_hypot(real{"nan", 32}, real{3, 25}).nan_p());
    REQUIRE(log_hypot(real{"nan", 32}, real{3, 25}).get_prec() == 32);
    REQUIRE(log_hypot(real{"nan", 32}, real{"inf", 25}).nan_p());
    REQUIRE(log_hypot(real{"nan", 32}, real{"inf", 25}).get_prec() == 32);

    REQUIRE(log_hypot(real{3, 25}, real{"nan", 32}).nan_p());
    REQUIRE(log_hypot(real{3, 25}, real{"nan", 32}).get_prec() == 32);
    REQUIRE(log_hypot(real{"inf", 25}, real{"nan", 32}).nan_p());
    REQUIRE(log_hypot(real{"inf", 25}, real{"nan", 32}).get_prec() == 32);

    REQUIRE(log_hypot(real{"nan", 25}, real{"nan", 32}).nan_p());
    REQUIRE(log_hypot(real{"nan", 25}, real{"nan", 32}).get_prec() == 32);
}

TEST_CASE("real log_base_ui")
{
    const auto cmp = 0.055385892956318409565552851937651923559275_r128;

    real rop, r0{1};
    REQUIRE(log_base_ui(rop, r0, 42) == 0);
    REQUIRE(std::is_same<real &, decltype(log_base_ui(rop, r0, 42))>::value);
    REQUIRE(std::is_same<real, decltype(log_base_ui(r0, 42))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(log_base_ui(std::move(r0), 42) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = 1.23_r128;
    auto tmp = log_base_ui(std::move(r0), 42);
    REQUIRE(abs(tmp - cmp) <= 1E-35);
    REQUIRE(tmp.get_prec() == 128);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());

    r0 = real{};
    log_base_ui(r0, 1.23_r128, 42);
    REQUIRE(abs(r0 - cmp) <= 1E-35);
    REQUIRE(r0.get_prec() == 128);
    REQUIRE(abs(log_base_ui(1.23_r128, 42) - cmp) <= 1e-35);

    REQUIRE(log_base_ui(real{-100}, 42).nan_p());
    log_base_ui(r0, real{-100}, 42);
    REQUIRE(r0.nan_p());
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));

    REQUIRE(log_base_ui(real{"inf", 100}, 42).inf_p());
    REQUIRE(log_base_ui(real{"inf", 100}, 42) > 0);
    REQUIRE(log_base_ui(real{"inf", 100}, 42).get_prec() == 100);
    log_base_ui(r0, real{"inf", 100}, 42);
    REQUIRE(r0.inf_p());
    REQUIRE(r0 > 0);
    REQUIRE(r0.get_prec() == 100);

    REQUIRE(log_base_ui(real{"-inf", 100}, 42).nan_p());
    REQUIRE(log_base_ui(real{"-inf", 100}, 42).get_prec() == 100);
    log_base_ui(r0, real{"-inf", 100}, 42);
    REQUIRE(r0.nan_p());
    REQUIRE(r0.get_prec() == 100);
}

#endif
