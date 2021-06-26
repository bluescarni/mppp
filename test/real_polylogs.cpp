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

TEST_CASE("real li2")
{
    {
        real r0{-1};
        r0.li2();
        REQUIRE(std::is_same<real &, decltype(r0.li2())>::value);
        REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
        REQUIRE(abs(r0 + 0.8224670334241132) < 1E-5);
        real rop;
        r0 = real{-1};
        REQUIRE(abs(li2(rop, r0) + 0.8224670334241132) < 1E-5);
        REQUIRE(std::is_same<real &, decltype(li2(rop, r0))>::value);
        REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
        REQUIRE(abs(li2(r0) + 0.8224670334241132) < 1E-5);
        REQUIRE(std::is_same<real, decltype(li2(r0))>::value);
        REQUIRE(abs(li2(std::move(r0)) + 0.8224670334241132) < 1E-5);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());
    }

    // op == 1.
    {
        real r0{1};
        r0.li2();
        REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
        REQUIRE(r0.nan_p());
        real rop;
        r0 = real{1};
        REQUIRE(li2(rop, r0).nan_p());
        REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
        REQUIRE(li2(r0).nan_p());
        REQUIRE(li2(std::move(r0)).nan_p());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());
    }

    // op > 1.
    {
        real r0{1.1};
        r0.li2();
        REQUIRE(r0.get_prec() == detail::real_deduce_precision(1.1));
        REQUIRE(r0.nan_p());
        real rop;
        r0 = real{1.2};
        REQUIRE(li2(rop, r0).nan_p());
        REQUIRE(rop.get_prec() == detail::real_deduce_precision(1.2));
        REQUIRE(li2(r0).nan_p());
        REQUIRE(li2(std::move(r0)).nan_p());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());
    }

    // op == +inf.
    {
        real r0{"inf", 112};
        r0.li2();
        REQUIRE(r0.get_prec() == 112);
        REQUIRE(r0.nan_p());
        real rop;
        r0 = real{"inf", 113};
        REQUIRE(li2(rop, r0).nan_p());
        REQUIRE(rop.get_prec() == 113);
        REQUIRE(li2(r0).nan_p());
        REQUIRE(li2(std::move(r0)).nan_p());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());
    }

    // op == -inf.
    {
        real r0{"-inf", 112};
        r0.li2();
        REQUIRE(r0.get_prec() == 112);
        REQUIRE(r0.inf_p());
        REQUIRE(r0 < 0);
        real rop;
        r0 = real{"-inf", 113};
        REQUIRE(li2(rop, r0).inf_p());
        REQUIRE(rop.get_prec() == 113);
        REQUIRE(rop < 0);
        REQUIRE(li2(r0).inf_p());
        REQUIRE(li2(r0) < 0);
        REQUIRE(li2(std::move(r0)).inf_p());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());
    }

    // op == nan.
    {
        real r0{"nan", 112};
        r0.li2();
        REQUIRE(r0.get_prec() == 112);
        REQUIRE(r0.nan_p());
        real rop;
        r0 = real{"-nan", 113};
        REQUIRE(li2(rop, r0).nan_p());
        REQUIRE(rop.get_prec() == 113);
        REQUIRE(li2(r0).nan_p());
        REQUIRE(li2(std::move(r0)).nan_p());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r0.is_valid());
    }
}

#if defined(MPPP_WITH_ARB)

TEST_CASE("real polylog_si")
{
    // Check a computation.
    REQUIRE(abs(0.6864849629546578850955782387443359_r128 - polylog_si(3, .625_r128)) < pow(2_r128, -110));

    // Binary form.
    REQUIRE(polylog_si(3, .625_r128).get_prec() == 128);
    REQUIRE(polylog_si(3, .625_r256).get_prec() == 256);
    {
        real r1{1.25, 32};
        REQUIRE(polylog_si(-3, r1).get_prec() == 32);
        polylog_si(-3, std::move(r1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r1.is_valid());
        r1 = real{2.5, 16};
        polylog_si(4, std::move(r1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r1.is_valid());
    }

    // Ternary form.
    {
        real r, r1{".1", 32};
        polylog_si(r, 3, r1);
        REQUIRE(r.get_prec() == 32);
        REQUIRE(r == polylog_si(3, r1));

        r = real{};
        polylog_si(r, 3, std::move(r1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r1 == real{});
        REQUIRE(r1.get_prec() == real_prec_min());
        REQUIRE(r == polylog_si(3, real{".1", 32}));
    }
}

TEST_CASE("real polylog")
{
    // Check a computation.
    REQUIRE(abs(0.6664622747095047528758921199030404761132092877313726623777496432_r128 - polylog(3.5_r128, .625_r128))
            < pow(2_r128, -110));

    // Binary form.
    REQUIRE(polylog(-.25_r128, 6.25_r128).get_prec() == 128);
    REQUIRE(polylog(-.25_r256, 6.25_r128).get_prec() == 256);
    REQUIRE(polylog(-.25_r128, 6.25_r256).get_prec() == 256);
    REQUIRE(polylog(real{.25, 32}, 1 / 10_q1)
            == polylog(real{.25, 32},
                       real{".1", detail::c_max(::mpfr_prec_t(32), detail::real_deduce_precision(1 / 10_q1))}));
    REQUIRE(polylog(1 / 10_q1, real{.25, 32})
            == polylog(real{".1", detail::c_max(::mpfr_prec_t(32), detail::real_deduce_precision(1 / 10_q1))},
                       real{.25, 32}));
    {
        real r1{1.25, 32}, r2{2.5, 64};
        REQUIRE(polylog(r1, r2).get_prec() == 64);
        polylog(r1, std::move(r2));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r2.is_valid());
        r2 = real{2.5, 16};
        polylog(std::move(r1), r2);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r1.is_valid());
    }

    // Ternary form.
    {
        real r, r1{".1", 32}, r2{".3", 33};
        polylog(r, r1, r2);
        REQUIRE(r.get_prec() == 33);
        REQUIRE(r == polylog(r1, r2));

        r = real{};
        polylog(r, r1, std::move(r2));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r2 == real{});
        REQUIRE(r2.get_prec() == real_prec_min());
        REQUIRE(r == polylog(r1, real{".3", 33}));

        r = real{};
        r2 = real{".3", 33};
        polylog(r, std::move(r2), r1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r2 == real{});
        REQUIRE(r2.get_prec() == real_prec_min());
        REQUIRE(r == polylog(real{".3", 33}, r1));

        r = real{};
        r2 = real{".3", 33};
        polylog(r, std::move(r2), std::move(r1));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r2 == real{});
        REQUIRE(r2.get_prec() == real_prec_min());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r1 == real{".1", 32});
        REQUIRE(r1.get_prec() == 32);
        REQUIRE(r == polylog(real{".3", 33}, r1));
    }
}

#endif
