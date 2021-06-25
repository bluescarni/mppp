// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>
#include <string>
#include <utility>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real trig")
{
    real r0{0};
    r0.sin();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0.zero_p());
    real rop;
    REQUIRE(sin(rop, r0).zero_p());
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sin(r0).zero_p());
    REQUIRE(sin(std::move(r0)).zero_p());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.cos();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    rop = real{};
    r0 = real{0};
    REQUIRE(cos(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(cos(r0) == 1);
    REQUIRE(cos(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.tan();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    rop = real{1};
    r0 = real{0};
    REQUIRE(tan(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(tan(r0) == 0);
    REQUIRE(tan(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.sec();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    r0 = real{0};
    rop = real{1};
    REQUIRE(sec(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sec(r0) == 1);
    REQUIRE(sec(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.csc();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0.inf_p());
    r0 = real{0};
    rop = real{1};
    REQUIRE(csc(rop, r0).inf_p());
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(csc(r0).inf_p());
    REQUIRE(csc(std::move(r0)).inf_p());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.cot();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0.inf_p());
    r0 = real{0};
    rop = real{1};
    REQUIRE(cot(rop, r0).inf_p());
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(cot(r0).inf_p());
    REQUIRE(cot(std::move(r0)).inf_p());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.asin();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    rop = real{1};
    r0 = real{0};
    REQUIRE(asin(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(asin(r0) == 0);
    REQUIRE(asin(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.acos();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == real_pi(r0.get_prec()) / 2);
    rop = real{1};
    r0 = real{0};
    REQUIRE(acos(rop, r0) == real_pi(r0.get_prec()) / 2);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(acos(r0) == real_pi(r0.get_prec()) / 2);
    auto p_cmp = r0.get_prec();
    REQUIRE(acos(std::move(r0)) == real_pi(p_cmp) / 2);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{1};
    r0.atan();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == real_pi(r0.get_prec()) / 4);
    rop = real{2};
    r0 = real{1};
    REQUIRE(atan(rop, r0) == real_pi(r0.get_prec()) / 4);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(atan(r0) == real_pi(r0.get_prec()) / 4);
    p_cmp = r0.get_prec();
    REQUIRE(atan(std::move(r0)) == real_pi(p_cmp) / 4);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    // sin_cos.
    real sop{1, detail::real_deduce_precision(0) * 2}, cop{2, detail::real_deduce_precision(0) * 3};
    REQUIRE(sop.get_prec() != detail::real_deduce_precision(0));
    REQUIRE(cop.get_prec() != detail::real_deduce_precision(0));
    sin_cos(sop, cop, real{32});
    REQUIRE(sop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(cop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sop == sin(real{32}));
    REQUIRE(cop == cos(real{32}));

    REQUIRE_THROWS_PREDICATE(sin_cos(sop, sop, real{32}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string{
                   "In the real sin_cos() function, the return values 'sop' and 'cop' must be distinct objects"};
    });

    // Try with overlapping op/sop and op/cop.
    sop = real{1, detail::real_deduce_precision(0) * 2};
    cop = real{2, detail::real_deduce_precision(0) * 3};
    sin_cos(sop, cop, sop);
    REQUIRE(sop.get_prec() == detail::real_deduce_precision(0) * 2);
    REQUIRE(cop.get_prec() == detail::real_deduce_precision(0) * 2);
    REQUIRE(sop == sin(real{1, detail::real_deduce_precision(0) * 2}));
    REQUIRE(cop == cos(real{1, detail::real_deduce_precision(0) * 2}));

    sop = real{1, detail::real_deduce_precision(0) * 2};
    cop = real{2, detail::real_deduce_precision(0) * 3};
    sin_cos(sop, cop, cop);
    REQUIRE(sop.get_prec() == detail::real_deduce_precision(0) * 3);
    REQUIRE(cop.get_prec() == detail::real_deduce_precision(0) * 3);
    REQUIRE(sop == sin(real{2, detail::real_deduce_precision(0) * 3}));
    REQUIRE(cop == cos(real{2, detail::real_deduce_precision(0) * 3}));

    // atan2.
    r0 = real{12, 450};
    atan2(r0, real{4}, real{5});
    REQUIRE(r0 == atan(real{4} / real{5}));
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    real tmp1{4}, tmp2{5};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    atan2(r0, std::move(tmp1), tmp2);
    REQUIRE(r0 == atan(real{4} / real{5}));
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp1 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp1 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp1.get_prec() == detail::real_deduce_precision(0) / 2);
    tmp1 = real{4};
    tmp2 = real{5};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    atan2(r0, tmp1, std::move(tmp2));
    REQUIRE(r0 == atan(real{4} / real{5}));
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp2 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp2 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp2.get_prec() == detail::real_deduce_precision(0) / 2);

    // Some tests for the binary form too.
    REQUIRE(atan2(real{4}, real{5}) == atan(real{4} / real{5}));
    REQUIRE(atan2(real{4, 20}, real{5, 30}).get_prec() == 30);
    REQUIRE(atan2(real{4}, 5.) == atan2(real{4}, real{5.}));
    REQUIRE(atan2(5., real{4}) == atan2(real{5.}, real{4}));
    REQUIRE(atan2(real{4}, 5) == atan2(real{4}, real{5}));
    REQUIRE(atan2(5, real{4}) == atan2(real{5}, real{4}));
    REQUIRE(atan2(real{4}, -5.) == atan2(real{4}, real{-5.}));
    REQUIRE(atan2(-5., real{4}) == atan2(real{-5.}, real{4}));
    REQUIRE(atan2(real{4}, -5) == atan2(real{4}, real{-5}));
    REQUIRE(atan2(-5, real{4}) == atan2(real{-5}, real{4}));
    REQUIRE(atan2(real{4}, integer<1>{-5}) == atan2(real{4}, real{integer<1>{-5}}));
    REQUIRE(atan2(integer<1>{-5}, real{4}) == atan2(real{integer<1>{-5}}, real{4}));
    REQUIRE(atan2(real{4, detail::real_deduce_precision(0.) / 2}, 5.).get_prec() == detail::real_deduce_precision(0.));
    REQUIRE(atan2(4., real{5, detail::real_deduce_precision(0.) / 2}).get_prec() == detail::real_deduce_precision(0.));
    REQUIRE(atan2(real{4, detail::real_deduce_precision(0) / 2}, 5).get_prec() == detail::real_deduce_precision(0));
    REQUIRE(atan2(4, real{5, detail::real_deduce_precision(0) / 2}).get_prec() == detail::real_deduce_precision(0));
}

#if defined(MPPP_WITH_ARB)

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real trig arb")
{
    {
        auto r0 = 1.23_r128;
        r0.sin_pi();
        REQUIRE(r0.get_prec() == 128);
        REQUIRE(abs(r0 - sin(real_pi(128) * 1.23_r128)) < mppp::pow(2_r128, -126));
        real rop{2, 12};
        r0 = 1.23_r128;
        sin_pi(rop, r0);
        REQUIRE(rop.get_prec() == 128);
        REQUIRE(abs(rop - sin(real_pi(128) * 1.23_r128)) < mppp::pow(2_r128, -126));
        REQUIRE(sin_pi(std::move(r0)) == rop);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());

        REQUIRE(sin_pi(real{"inf", 128}).nan_p());
        REQUIRE(sin_pi(real{"inf", 128}).get_prec() == 128);
        REQUIRE(sin_pi(-real{"inf", 128}).nan_p());
        REQUIRE(sin_pi(-real{"inf", 128}).get_prec() == 128);
        REQUIRE(sin_pi(real{"nan", 128}).nan_p());
        REQUIRE(sin_pi(real{"nan", 128}).get_prec() == 128);
    }

    {
        auto r0 = 1.23_r128;
        r0.cos_pi();
        REQUIRE(r0.get_prec() == 128);
        REQUIRE(abs(r0 - cos(real_pi(128) * 1.23_r128)) < mppp::pow(2_r128, -126));
        real rop{2, 12};
        r0 = 1.23_r128;
        cos_pi(rop, r0);
        REQUIRE(rop.get_prec() == 128);
        REQUIRE(abs(rop - cos(real_pi(128) * 1.23_r128)) < mppp::pow(2_r128, -126));
        REQUIRE(cos_pi(std::move(r0)) == rop);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());

        REQUIRE(cos_pi(real{"inf", 128}).nan_p());
        REQUIRE(cos_pi(real{"inf", 128}).get_prec() == 128);
        REQUIRE(cos_pi(-real{"inf", 128}).nan_p());
        REQUIRE(cos_pi(-real{"inf", 128}).get_prec() == 128);
        REQUIRE(cos_pi(real{"nan", 128}).nan_p());
        REQUIRE(cos_pi(real{"nan", 128}).get_prec() == 128);
    }

    {
        auto r0 = 1.23_r128;
        r0.tan_pi();
        REQUIRE(r0.get_prec() == 128);
        REQUIRE(abs(r0 - tan(real_pi(128) * 1.23_r128)) < mppp::pow(2_r128, -126));
        real rop{2, 12};
        r0 = 1.23_r128;
        tan_pi(rop, r0);
        REQUIRE(rop.get_prec() == 128);
        REQUIRE(abs(rop - tan(real_pi(128) * 1.23_r128)) < mppp::pow(2_r128, -126));
        REQUIRE(tan_pi(std::move(r0)) == rop);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());

        REQUIRE(tan_pi(real{"inf", 128}).nan_p());
        REQUIRE(tan_pi(real{"inf", 128}).get_prec() == 128);
        REQUIRE(tan_pi(-real{"inf", 128}).nan_p());
        REQUIRE(tan_pi(-real{"inf", 128}).get_prec() == 128);
        REQUIRE(tan_pi(real{"nan", 128}).nan_p());
        REQUIRE(tan_pi(real{"nan", 128}).get_prec() == 128);

        // Special values.
        REQUIRE(tan_pi(0_r128).zero_p());
        REQUIRE(tan_pi(0_r128).get_prec() == 128);

        REQUIRE(tan_pi(0.5_r128).nan_p());
        REQUIRE(tan_pi(0.5_r128).get_prec() == 128);

        REQUIRE(tan_pi(1.5_r128).nan_p());
        REQUIRE(tan_pi(1.5_r128).get_prec() == 128);

        REQUIRE(tan_pi(2.5_r128).nan_p());
        REQUIRE(tan_pi(2.5_r128).get_prec() == 128);

        REQUIRE(tan_pi(3.5_r128).nan_p());
        REQUIRE(tan_pi(3.5_r128).get_prec() == 128);

        REQUIRE(tan_pi(650.5_r128).nan_p());
        REQUIRE(tan_pi(650.5_r128).get_prec() == 128);

        REQUIRE(tan_pi(-0.5_r128).nan_p());
        REQUIRE(tan_pi(-0.5_r128).get_prec() == 128);

        REQUIRE(tan_pi(-1.5_r128).nan_p());
        REQUIRE(tan_pi(-1.5_r128).get_prec() == 128);

        REQUIRE(tan_pi(-2.5_r128).nan_p());
        REQUIRE(tan_pi(-2.5_r128).get_prec() == 128);

        REQUIRE(tan_pi(-3.5_r128).nan_p());
        REQUIRE(tan_pi(-3.5_r128).get_prec() == 128);

        REQUIRE(tan_pi(-650.5_r128).nan_p());
        REQUIRE(tan_pi(-650.5_r128).get_prec() == 128);
    }

    {
        auto r0 = 1.23_r128;
        r0.cot_pi();
        REQUIRE(r0.get_prec() == 128);
        REQUIRE(abs(r0 - cot(real_pi(128) * 1.23_r128)) < mppp::pow(2_r128, -126));
        real rop{2, 12};
        r0 = 1.23_r128;
        cot_pi(rop, r0);
        REQUIRE(rop.get_prec() == 128);
        REQUIRE(abs(rop - cot(real_pi(128) * 1.23_r128)) < mppp::pow(2_r128, -126));
        REQUIRE(cot_pi(std::move(r0)) == rop);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());

        REQUIRE(cot_pi(real{"inf", 128}).nan_p());
        REQUIRE(cot_pi(real{"inf", 128}).get_prec() == 128);
        REQUIRE(cot_pi(-real{"inf", 128}).nan_p());
        REQUIRE(cot_pi(-real{"inf", 128}).get_prec() == 128);
        REQUIRE(cot_pi(real{"nan", 128}).nan_p());
        REQUIRE(cot_pi(real{"nan", 128}).get_prec() == 128);

        // Special values.
        REQUIRE(cot_pi(0_r128).nan_p());
        REQUIRE(cot_pi(0_r128).get_prec() == 128);

        REQUIRE(cot_pi(1_r128).nan_p());
        REQUIRE(cot_pi(1_r128).get_prec() == 128);

        REQUIRE(cot_pi(2._r128).nan_p());
        REQUIRE(cot_pi(2._r128).get_prec() == 128);

        REQUIRE(cot_pi(3_r128).nan_p());
        REQUIRE(cot_pi(3_r128).get_prec() == 128);

        REQUIRE(cot_pi(650_r128).nan_p());
        REQUIRE(cot_pi(650_r128).get_prec() == 128);

        REQUIRE(cot_pi(-1_r128).nan_p());
        REQUIRE(cot_pi(-1_r128).get_prec() == 128);

        REQUIRE(cot_pi(-2._r128).nan_p());
        REQUIRE(cot_pi(-2._r128).get_prec() == 128);

        REQUIRE(cot_pi(-3_r128).nan_p());
        REQUIRE(cot_pi(-3_r128).get_prec() == 128);

        REQUIRE(cot_pi(-650_r128).nan_p());
        REQUIRE(cot_pi(-650_r128).get_prec() == 128);
    }

    {
        auto r0 = 1.23_r128;
        r0.sinc();
        REQUIRE(r0.get_prec() == 128);
        REQUIRE(abs(r0 - sin(1.23_r128) / 1.23_r128) < mppp::pow(2_r128, -126));
        real rop{2, 12};
        r0 = 1.23_r128;
        sinc(rop, r0);
        REQUIRE(rop.get_prec() == 128);
        REQUIRE(abs(rop - sin(1.23_r128) / 1.23_r128) < mppp::pow(2_r128, -126));
        REQUIRE(sinc(std::move(r0)) == rop);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());

        REQUIRE(sinc(real{"inf", 128}).zero_p());
        REQUIRE(sinc(real{"inf", 128}).get_prec() == 128);
        REQUIRE(sinc(-real{"inf", 128}).zero_p());
        REQUIRE(sinc(-real{"inf", 128}).get_prec() == 128);
        REQUIRE(sinc(real{"nan", 128}).nan_p());
        REQUIRE(sinc(real{"nan", 128}).get_prec() == 128);
        REQUIRE(sinc(real{0, 128}) == 1);
        REQUIRE(sinc(real{0, 128}).get_prec() == 128);
    }

    {
        auto r0 = 1.23_r128;
        r0.sinc_pi();
        REQUIRE(r0.get_prec() == 128);
        REQUIRE(abs(r0 - sin(real_pi(128) * 1.23_r128) / (real_pi(128) * 1.23_r128)) < mppp::pow(2_r128, -126));
        real rop{2, 12};
        r0 = 1.23_r128;
        sinc_pi(rop, r0);
        REQUIRE(rop.get_prec() == 128);
        REQUIRE(abs(rop - sin(real_pi(128) * 1.23_r128) / (real_pi(128) * 1.23_r128)) < mppp::pow(2_r128, -126));
        REQUIRE(sinc_pi(std::move(r0)) == rop);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());

        REQUIRE(sinc_pi(real{"inf", 128}).zero_p());
        REQUIRE(sinc_pi(real{"inf", 128}).get_prec() == 128);
        REQUIRE(sinc_pi(-real{"inf", 128}).zero_p());
        REQUIRE(sinc_pi(-real{"inf", 128}).get_prec() == 128);
        REQUIRE(sinc_pi(real{"nan", 128}).nan_p());
        REQUIRE(sinc_pi(real{"nan", 128}).get_prec() == 128);
        REQUIRE(sinc_pi(real{0, 128}) == 1);
        REQUIRE(sinc_pi(real{0, 128}).get_prec() == 128);
    }
}

#endif
