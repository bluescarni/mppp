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

#include <mp++/complex.hpp>
#include <mp++/config.hpp>
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

TEST_CASE("agm1")
{
    const auto cmp1
        = 1.2049597176136955190833988540153239038944_r128 + 1.006180300341415795767582267103891529043_icr128;
    {
        // Member function.
        auto c = 1.1_r128 + 2.3_icr128;
        c.agm1();
        REQUIRE(abs(c - cmp1) < pow(2_r128, -125));
        REQUIRE(c.get_prec() == 128);
    }
    {
        // rop overload.
        complex c1, c2 = 1.1_r128 + 2.3_icr128;
        const auto p = c2.get_prec();
        REQUIRE(&agm1(c1, c2) == &c1);
        REQUIRE(std::is_same<decltype(agm1(c1, c2)), complex &>::value);
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -125));
        REQUIRE(c1.get_prec() == p);

        // Move, but won't steal because rop
        // has higher precision.
        c1 = complex{0, complex_prec_t(c2.get_prec() + 1)};
        agm1(c1, std::move(c2));
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -125));
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == 1.1_r128 + 2.3_icr128);

        // Move, will steal.
        c1 = complex{};
        agm1(c1, std::move(c2));
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -125));
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{});
    }
    {
        // return overload.
        REQUIRE(abs(agm1(1.1_r128 + 2.3_icr128) - cmp1) < pow(2_r128, -125));
        REQUIRE(std::is_same<decltype(agm1(complex{1, 2})), complex>::value);

        // move, will steal.
        complex c1 = 1.1_r128 + 2.3_icr128;
        const auto p = c1.get_prec();
        auto c2 = agm1(std::move(c1));
        REQUIRE(abs(c2 - cmp1) < pow(2_r128, -125));
        REQUIRE(c2.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
}

#if defined(MPPP_ARB_HAVE_ACB_AGM)

TEST_CASE("agm")
{
    // Ternary agm.
    {
        complex c1, c2, c3;
        agm(c1, c2, c3);
        REQUIRE(std::is_same<complex &, decltype(agm(c1, c2, c3))>::value);
        REQUIRE(c1 == 0);
        REQUIRE(c1.get_prec() == real_prec_min());

        const auto cmp1
            = 4.1177969267892181490263289835411230858823_r128 - 1.1492300728331716779787765033036225824577_icr128;

        c2 = 4_r128 - 5_icr128;
        c3 = 3_r128 + 2_icr128;
        agm(c1, c2, c3);
        REQUIRE(abs(c1 - cmp1) <= pow(2_r128, -120));
        REQUIRE(c1.get_prec() == 128);

        c2 = complex{c2, complex_prec_t(129)};
        agm(c1, c2, c3);
        REQUIRE(abs(c1 - cmp1) <= pow(2_r128, -120));
        REQUIRE(c1.get_prec() == 129);

        c3 = complex{c3, complex_prec_t(130)};
        agm(c1, c2, c3);
        REQUIRE(abs(c1 - cmp1) <= pow(2_r128, -120));
        REQUIRE(c1.get_prec() == 130);

        // Overlapping.
        c1 = c2;
        agm(c1, c1, c1);
        REQUIRE(abs(c1 - c2) <= pow(2_r128, -120));
        REQUIRE(c1.get_prec() == 129);

        c1 = c3;
        agm(c1, c2, c1);
        REQUIRE(abs(c1 - cmp1) <= pow(2_r128, -120));
        REQUIRE(c1.get_prec() == 130);

        // Check moves.
        c1 = complex{};
        c2 = 4_r128 - 5_icr128;
        c3 = 3_r128 + 2_icr128;
        agm(c1, std::move(c2), c3);
        REQUIRE(abs(c1 - cmp1) <= pow(2_r128, -120));
        REQUIRE(c1.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2.is_valid());
        REQUIRE(c2.zero_p());
        REQUIRE(c2.get_prec() == real_prec_min());

        c1 = complex{};
        c2 = 4_r128 - 5_icr128;
        c3 = 3_r128 + 2_icr128;
        agm(c1, c2, std::move(c3));
        REQUIRE(abs(c1 - cmp1) <= pow(2_r128, -120));
        REQUIRE(c1.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c3.is_valid());
        REQUIRE(c3.zero_p());
        REQUIRE(c3.get_prec() == real_prec_min());

        c1 = complex{};
        c2 = 4_r128 - 5_icr128;
        c3 = complex{3_r128 + 2_icr128, complex_prec_t(129)};
        agm(c1, std::move(c2), std::move(c3));
        REQUIRE(abs(c1 - cmp1) <= pow(2_r128, -120));
        REQUIRE(c1.get_prec() == 129);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c3.is_valid());
        REQUIRE(c3.zero_p());
        REQUIRE(c3.get_prec() == real_prec_min());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == 4_r128 - 5_icr128);
        REQUIRE(c2.get_prec() == 128);
    }

    // Binary complex-complex.
    {
        auto c1 = 4_r128 - 5_icr128, c2 = 3_r128 + 2_icr128;

        const auto cmp1
            = 4.1177969267892181490263289835411230858823_r128 - 1.1492300728331716779787765033036225824577_icr128;

        REQUIRE(std::is_same<complex, decltype(agm(c1, c2))>::value);
        REQUIRE(abs(agm(c1, c2) - cmp1) <= pow(2_r128, -120));
        REQUIRE(agm(c1, c2).get_prec() == 128);

        c2 = complex{3, 2, complex_prec_t(2)};
        REQUIRE(abs(agm(c1, c2) - cmp1) <= pow(2_r128, -120));
        REQUIRE(agm(c1, c2).get_prec() == 128);

        c2 = 3_r128 + 2_icr128;
        c1 = complex{4, -5, complex_prec_t(4)};
        REQUIRE(abs(agm(c1, c2) - cmp1) <= pow(2_r128, -120));
        REQUIRE(agm(c1, c2).get_prec() == 128);

        // Check moves.
        c1 = 4_r128 - 5_icr128;
        c2 = complex{3, 2, complex_prec_t(32)};
        auto ret = agm(std::move(c1), c2);
        REQUIRE(abs(ret - cmp1) <= pow(2_r128, -120));
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
        c1 = complex{4, -5, complex_prec_t(16)};
        c2 = 3_r128 + 2_icr128;
        ret = agm(std::move(c1), std::move(c2));
        REQUIRE(abs(ret - cmp1) <= pow(2_r128, -120));
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c2.is_valid());
    }

    // Complex-(non complex).
    {
        const auto cmp1 = 2.9135820620938137383728647665301639499433_r128;

        complex c{4_r128};
        real r{2};
        REQUIRE(std::is_same<complex, decltype(agm(c, r))>::value);
        REQUIRE(abs(cmp1 - agm(c, r)) <= pow(2_r128, -120));
        REQUIRE(agm(c, r).get_prec() == 128);

        r = real{2, 2};
        REQUIRE(abs(cmp1 - agm(c, r)) <= pow(2_r128, -120));
        REQUIRE(agm(c, r).get_prec() == 128);

        r = 2_r128;
        c = complex{4, complex_prec_t(2)};
        REQUIRE(abs(cmp1 - agm(c, r)) <= pow(2_r128, -120));
        REQUIRE(agm(c, r).get_prec() == 128);

        // Check moves.
        c = complex{4_r128};
        r = real{2, 32};
        auto ret = agm(std::move(c), r);
        REQUIRE(abs(cmp1 - ret) <= pow(2_r128, -120));
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(16)};
        r = 2_r128;
        ret = agm(std::move(c), std::move(r));
        REQUIRE(abs(cmp1 - ret) <= pow(2_r128, -120));
        REQUIRE(ret.get_prec() == 128);
        // NOTE: both r and c must be valid, because the
        // result's precision needs to be 128 and the complex
        // argument has only 16 bits of precision (and we cannot
        // steal from the higher-precision real argument).
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r.is_valid());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());

        // Try with other non-complex types.
        REQUIRE(agm(c, 4) == 4);
        REQUIRE(agm(c, 4.) == 4);
        REQUIRE(agm(c, 4_z1) == 4);
        REQUIRE(agm(c, 4_q1) == 4);
        REQUIRE(agm(c, std::complex<double>{4, 0}) == 4);

#if defined(MPPP_WITH_QUADMATH)
        REQUIRE(agm(c, 4_rq) == 4);
        REQUIRE(agm(c, 4_rq + 0_icq) == 4);
#endif
    }

    // (Non complex)-complex.
    {
        const auto cmp1 = 2.9135820620938137383728647665301639499433_r128;

        complex c{4_r128};
        real r{2};
        REQUIRE(std::is_same<complex, decltype(agm(r, c))>::value);
        REQUIRE(abs(cmp1 - agm(r, c)) <= pow(2_r128, -120));
        REQUIRE(agm(r, c).get_prec() == 128);

        r = real{2, 2};
        REQUIRE(abs(cmp1 - agm(r, c)) <= pow(2_r128, -120));
        REQUIRE(agm(r, c).get_prec() == 128);

        r = 2_r128;
        c = complex{4, complex_prec_t(2)};
        REQUIRE(abs(cmp1 - agm(r, c)) <= pow(2_r128, -120));
        REQUIRE(agm(r, c).get_prec() == 128);

        // Check moves.
        c = complex{4_r128};
        r = real{2, 32};
        auto ret = agm(r, std::move(c));
        REQUIRE(abs(cmp1 - ret) <= pow(2_r128, -120));
        REQUIRE(ret.get_prec() == 128);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        c = complex{4, complex_prec_t(16)};
        r = 2_r128;
        ret = agm(std::move(r), std::move(c));
        REQUIRE(abs(cmp1 - ret) <= pow(2_r128, -120));
        REQUIRE(ret.get_prec() == 128);
        // NOTE: both r and c must be valid, because the
        // result's precision needs to be 128 and the complex
        // argument has only 16 bits of precision (and we cannot
        // steal from the higher-precision real argument).
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r.is_valid());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c.is_valid());

        // Try with other non-complex types.
        REQUIRE(agm(4, c) == 4);
        REQUIRE(agm(4., c) == 4);
        REQUIRE(agm(4_z1, c) == 4);
        REQUIRE(agm(4_q1, c) == 4);
        REQUIRE(agm(std::complex<double>{4, 0}, c) == 4);

#if defined(MPPP_WITH_QUADMATH)
        REQUIRE(agm(4_rq, c) == 4);
        REQUIRE(agm(4_rq + 0_icq, c) == 4);
#endif
    }
}

#endif
