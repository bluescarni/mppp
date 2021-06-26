// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>
#include <utility>

#include <mp++/complex.hpp>
#include <mp++/config.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("sqrt")
{
    complex r0{0};
    r0.sqrt();
    REQUIRE(std::is_same<complex &, decltype(r0.sqrt())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0.zero_p());
    complex rop;
    REQUIRE(sqrt(rop, r0).zero_p());
    REQUIRE(std::is_same<complex &, decltype(sqrt(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sqrt(r0).zero_p());
    REQUIRE(std::is_same<complex, decltype(sqrt(r0))>::value);
    REQUIRE(sqrt(std::move(r0)).zero_p());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    r0 = complex{16, 17, complex_prec_t(128)};
    REQUIRE(abs(sqrt(r0)
                - (4.4353824558800734853070281844863776932288_r128 + 1.9164074540474820480048239757004444314933_icr128))
            < pow(2_r128, -120));
    REQUIRE(sqrt(r0).get_prec() == 128);
    rop = real{12, 40};
    sqrt(rop, r0);
    REQUIRE(
        abs(rop - (4.4353824558800734853070281844863776932288_r128 + 1.9164074540474820480048239757004444314933_icr128))
        < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.sqrt();
    REQUIRE(
        abs(r0 - (4.4353824558800734853070281844863776932288_r128 + 1.9164074540474820480048239757004444314933_icr128))
        < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

#if defined(MPPP_WITH_ARB)

TEST_CASE("rec_sqrt")
{
    const auto cmp1
        = 0.52984103253104949318719835021445625746079_r128 + 0.33391728095862217076724902471316862541707_icr128;
    {
        // Member function.
        auto c = 1.1_r128 - 2.3_icr128;
        c.rec_sqrt();
        REQUIRE(abs(c - cmp1) < pow(2_r128, -126));
        REQUIRE(c.get_prec() == 128);
    }
    {
        // rop overload.
        complex c1, c2 = 1.1_r128 - 2.3_icr128;
        const auto p = c2.get_prec();
        REQUIRE(&rec_sqrt(c1, c2) == &c1);
        REQUIRE(std::is_same<decltype(rec_sqrt(c1, c2)), complex &>::value);
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -126));
        REQUIRE(c1.get_prec() == p);

        // Move, but won't steal because rop
        // has higher precision.
        c1 = complex{0, complex_prec_t(c2.get_prec() + 1)};
        rec_sqrt(c1, std::move(c2));
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -126));
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == 1.1_r128 - 2.3_icr128);

        // Move, will steal.
        c1 = complex{};
        rec_sqrt(c1, std::move(c2));
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -126));
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{});
    }
    {
        // return overload.
        REQUIRE(abs(rec_sqrt(1.1_r128 - 2.3_icr128) - cmp1) < pow(2_r128, -126));
        REQUIRE(std::is_same<decltype(rec_sqrt(complex{1, 2})), complex>::value);

        // move, will steal.
        complex c1 = 1.1_r128 - 2.3_icr128;
        const auto p = c1.get_prec();
        auto c2 = rec_sqrt(std::move(c1));
        REQUIRE(abs(c2 - cmp1) < pow(2_r128, -126));
        REQUIRE(c2.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }

    // Test the special cases.
    {
        complex c{0, complex_prec_t(128)};
        REQUIRE(c.rec_sqrt().inf_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(rec_sqrt(complex{0, complex_prec_t(128)}).inf_p());
    }
    {
        complex c{"(inf, 0)", complex_prec_t(128)};
        REQUIRE(c.rec_sqrt().zero_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(rec_sqrt(complex{"(inf, 0)", complex_prec_t(128)}).zero_p());
    }
    {
        complex c{"(inf, nan)", complex_prec_t(128)};
        REQUIRE(c.rec_sqrt().zero_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(rec_sqrt(complex{"(inf, nan)", complex_prec_t(128)}).zero_p());
    }
    {
        complex c{"(0, inf)", complex_prec_t(128)};
        REQUIRE(c.rec_sqrt().zero_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(rec_sqrt(complex{"(0, inf)", complex_prec_t(128)}).zero_p());
    }
    {
        complex c{"(nan, inf)", complex_prec_t(128)};
        REQUIRE(c.rec_sqrt().zero_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(rec_sqrt(complex{"(nan, inf)", complex_prec_t(128)}).zero_p());
    }
    {
        complex c{"(inf, inf)", complex_prec_t(128)};
        REQUIRE(c.rec_sqrt().zero_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(rec_sqrt(complex{"(inf, inf)", complex_prec_t(128)}).zero_p());
    }
}

TEST_CASE("rootn_ui")
{
    const auto cmp1
        = 1.0522402910411225087119818587236727778544_r128 + 0.077807112492992516119625079049766522389835_icr128;

    {
        // rop overload.
        complex c1, c2 = 1_r128 + 2_icr128;
        const auto p = c2.get_prec();
        REQUIRE(&rootn_ui(c1, c2, 15) == &c1);
        REQUIRE(std::is_same<decltype(rootn_ui(c1, c2, 15)), complex &>::value);
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -126));
        REQUIRE(c1.get_prec() == p);

        // Move, but won't steal because rop
        // has higher precision.
        c1 = complex{0, complex_prec_t(c2.get_prec() + 1)};
        rootn_ui(c1, std::move(c2), 15);
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -126));
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == 1_r128 + 2_icr128);

        // Move, will steal.
        c1 = complex{};
        rootn_ui(c1, std::move(c2), 15);
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -126));
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{});
    }
    {
        // return overload.
        REQUIRE(abs(rootn_ui(1_r128 + 2_icr128, 15) - cmp1) < pow(2_r128, -126));
        REQUIRE(std::is_same<decltype(rootn_ui(complex{1, 2}, 15)), complex>::value);

        // move, will steal.
        complex c1 = 1_r128 + 2_icr128;
        const auto p = c1.get_prec();
        auto c2 = rootn_ui(std::move(c1), 15);
        REQUIRE(abs(c2 - cmp1) < pow(2_r128, -126));
        REQUIRE(c2.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }

    // Test the special cases.
    {
        auto tmp = rootn_ui(complex{1, 2, complex_prec_t(32)}, 0);
        REQUIRE(complex::re_cref { tmp } -> nan_p());
        REQUIRE(complex::im_cref { tmp } -> nan_p());
        REQUIRE(tmp.get_prec() == 32);
    }
    {
        REQUIRE(rootn_ui(complex{"(inf, 0)", complex_prec_t(128)}, 15).inf_p());
    }
    {
        REQUIRE(rootn_ui(complex{"(inf, nan)", complex_prec_t(128)}, 15).inf_p());
    }
}

#endif
