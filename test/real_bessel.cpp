// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <utility>

#include <mp++/config.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real j0")
{
    real r0{0};
    r0.j0();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    real rop;
    r0 = real{0};
    REQUIRE(j0(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(j0(r0) == 1);
    REQUIRE(j0(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real j1")
{
    real r0{0};
    r0.j1();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{0};
    REQUIRE(j1(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(j1(r0) == 0);
    REQUIRE(j1(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real jn")
{
    real rop, r0{0};
    REQUIRE(jn(rop, 0, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(jn(rop, 0, real{45}) == j0(real{45}));
    REQUIRE(jn(rop, 1, real{45}) == j1(real{45}));
    REQUIRE(jn(0, r0) == 1);
    REQUIRE(jn(0, std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real y0")
{
    real r0{0};
    r0.y0();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == real{"-inf", 100});
    real rop;
    r0 = real{0};
    REQUIRE(y0(rop, r0) == real{"-inf", 100});
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(y0(r0) == real{"-inf", 100});
    REQUIRE(y0(std::move(r0)) == real{"-inf", 100});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real y1")
{
    real r0{0};
    r0.y1();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == real{"-inf", 100});
    real rop;
    r0 = real{0};
    REQUIRE(y1(rop, r0) == real{"-inf", 100});
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(y1(r0) == real{"-inf", 100});
    REQUIRE(y1(std::move(r0)) == real{"-inf", 100});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real yn")
{
    real rop, r0{0};
    REQUIRE(yn(rop, 0, r0) == real{"-inf", 100});
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(yn(rop, 0, real{45}) == y0(real{45}));
    REQUIRE(yn(rop, 1, real{45}) == y1(real{45}));
    REQUIRE(yn(0, r0) == real{"-inf", 100});
    REQUIRE(yn(0, std::move(r0)) == real{"-inf", 100});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

#if defined(MPPP_WITH_ARB)

TEST_CASE("real jx")
{
    // Check a computation.
    REQUIRE(abs(2.883261377881473275518885803325280977594e-1_r128 - jx(-.25_r128, 6.25_r128)) < pow(2_r128, -120));
    // The special case.
    REQUIRE(jx(1.234, real{"+inf", 50}).zero_p());
    REQUIRE(jx(-1.234, real{"+inf", 50}).zero_p());
    // Special relation.
    REQUIRE(abs(jx(.5_r128, 1.23_r128) - sqrt(2_r128 / (real_pi(128) * 1.23_r128)) * sin(1.23_r128))
            < pow(2_r128, -120));

    // Binary form.
    REQUIRE(jx(-.25_r128, 6.25_r128).get_prec() == 128);
    REQUIRE(jx(-.25_r256, 6.25_r128).get_prec() == 256);
    REQUIRE(jx(-.25_r128, 6.25_r256).get_prec() == 256);
    REQUIRE(
        jx(real{.25, 32}, 1 / 10_q1)
        == jx(real{.25, 32}, real{".1", detail::c_max(::mpfr_prec_t(32), detail::real_deduce_precision(1 / 10_q1))}));
    REQUIRE(
        jx(1 / 10_q1, real{.25, 32})
        == jx(real{".1", detail::c_max(::mpfr_prec_t(32), detail::real_deduce_precision(1 / 10_q1))}, real{.25, 32}));
    {
        real r1{1.25, 32}, r2{2.5, 64};
        REQUIRE(jx(r1, r2).get_prec() == 64);
        jx(r1, std::move(r2));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r2.is_valid());
        r2 = real{2.5, 16};
        jx(std::move(r1), r2);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r1.is_valid());
    }

    // Ternary form.
    {
        real r, r1{"1.1", 32}, r2{"2.3", 33};
        jx(r, r1, r2);
        REQUIRE(r.get_prec() == 33);
        REQUIRE(r == jx(r1, r2));

        r = real{};
        jx(r, r1, std::move(r2));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r2 == real{});
        REQUIRE(r2.get_prec() == real_prec_min());
        REQUIRE(r == jx(r1, real{"2.3", 33}));

        r = real{};
        r2 = real{"2.3", 33};
        jx(r, std::move(r2), r1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r2 == real{});
        REQUIRE(r2.get_prec() == real_prec_min());
        REQUIRE(r == jx(real{"2.3", 33}, r1));

        r = real{};
        r2 = real{"2.3", 33};
        jx(r, std::move(r2), std::move(r1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r2 == real{});
        REQUIRE(r2.get_prec() == real_prec_min());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r1 == real{"1.1", 32});
        REQUIRE(r1.get_prec() == 32);
        REQUIRE(r == jx(real{"2.3", 33}, r1));
    }
}

TEST_CASE("real yx")
{
    // Check a computation.
    REQUIRE(abs(-1.359849816874681335170387972522093245940e-1_r128 - yx(-.25_r128, 6.25_r128)) < pow(2_r128, -120));
    // The special case.
    REQUIRE(yx(1.234, real{"+inf", 50}).zero_p());
    REQUIRE(yx(-1.234, real{"+inf", 50}).zero_p());

    // Binary form.
    REQUIRE(yx(-.25_r128, 6.25_r128).get_prec() == 128);
    REQUIRE(yx(-.25_r256, 6.25_r128).get_prec() == 256);
    REQUIRE(yx(-.25_r128, 6.25_r256).get_prec() == 256);
    REQUIRE(
        yx(real{.25, 32}, 1 / 10_q1)
        == yx(real{.25, 32}, real{".1", detail::c_max(::mpfr_prec_t(32), detail::real_deduce_precision(1 / 10_q1))}));
    REQUIRE(
        yx(1 / 10_q1, real{.25, 32})
        == yx(real{".1", detail::c_max(::mpfr_prec_t(32), detail::real_deduce_precision(1 / 10_q1))}, real{.25, 32}));
    {
        real r1{1.25, 32}, r2{2.5, 64};
        REQUIRE(yx(r1, r2).get_prec() == 64);
        yx(r1, std::move(r2));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r2.is_valid());
        r2 = real{2.5, 16};
        yx(std::move(r1), r2);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r1.is_valid());
    }

    // Ternary form.
    {
        real r, r1{"1.1", 32}, r2{"2.3", 33};
        yx(r, r1, r2);
        REQUIRE(r.get_prec() == 33);
        REQUIRE(r == yx(r1, r2));

        r = real{};
        yx(r, r1, std::move(r2));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r2 == real{});
        REQUIRE(r2.get_prec() == real_prec_min());
        REQUIRE(r == yx(r1, real{"2.3", 33}));

        r = real{};
        r2 = real{"2.3", 33};
        yx(r, std::move(r2), r1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r2 == real{});
        REQUIRE(r2.get_prec() == real_prec_min());
        REQUIRE(r == yx(real{"2.3", 33}, r1));

        r = real{};
        r2 = real{"2.3", 33};
        yx(r, std::move(r2), std::move(r1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r2 == real{});
        REQUIRE(r2.get_prec() == real_prec_min());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r1 == real{"1.1", 32});
        REQUIRE(r1.get_prec() == 32);
        REQUIRE(r == yx(real{"2.3", 33}, r1));
    }
}

#endif
