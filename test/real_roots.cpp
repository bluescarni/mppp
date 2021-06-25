// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <utility>

#include <mp++/config.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real sqrt")
{
    real r0{0};
    r0.sqrt();
    REQUIRE(std::is_same<real &, decltype(r0.sqrt())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0.zero_p());
    real rop;
    REQUIRE(sqrt(rop, r0).zero_p());
    REQUIRE(std::is_same<real &, decltype(sqrt(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sqrt(r0).zero_p());
    REQUIRE(std::is_same<real, decltype(sqrt(r0))>::value);
    REQUIRE(sqrt(std::move(r0)).zero_p());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    r0 = real{16, 128};
    REQUIRE(sqrt(r0) == 4);
    REQUIRE(sqrt(r0).get_prec() == 128);
    rop = real{12, 40};
    sqrt(rop, r0);
    REQUIRE(rop == 4);
    REQUIRE(rop.get_prec() == 128);
    r0.sqrt();
    REQUIRE(r0 == 4);
    REQUIRE(r0.get_prec() == 128);
    // Negative value.
    r0 = real{-16, 128};
    REQUIRE(sqrt(r0).nan_p());
}

TEST_CASE("real rec_sqrt")
{
    real r0{1};
    r0.rec_sqrt();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    real rop;
    REQUIRE(rec_sqrt(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(rec_sqrt(r0) == 1);
    REQUIRE(rec_sqrt(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    r0 = real{16, 128};
    REQUIRE(rec_sqrt(r0) == 1 / real{4});
    REQUIRE(rec_sqrt(r0).get_prec() == 128);
    rop = real{12, 40};
    rec_sqrt(rop, r0);
    REQUIRE(rop == 1 / real{4});
    REQUIRE(rop.get_prec() == 128);
    r0.rec_sqrt();
    REQUIRE(r0 == 1 / real{4});
    REQUIRE(r0.get_prec() == 128);
    // Special cases.
    REQUIRE((rec_sqrt(real{0}) == real{"+inf", 32}));
    REQUIRE((rec_sqrt(-real{0}) == real{"+inf", 32}));
    REQUIRE((rec_sqrt(real{"+inf", 32}) == 0));
    REQUIRE(!(rec_sqrt(real{"+inf", 32}).signbit()));
    REQUIRE((rec_sqrt(real{"-3", 32}).nan_p()));
    REQUIRE((rec_sqrt(real{"-inf", 32}).nan_p()));
}

TEST_CASE("real cbrt")
{
    real r0{0};
    r0.cbrt();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0.zero_p());
    real rop;
    REQUIRE(cbrt(rop, r0).zero_p());
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(cbrt(r0).zero_p());
    REQUIRE(cbrt(std::move(r0)).zero_p());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    r0 = real{-27, 128};
    REQUIRE(cbrt(r0) == -3);
    REQUIRE(cbrt(r0).get_prec() == 128);
    rop = real{12, 40};
    cbrt(rop, r0);
    REQUIRE(rop == -3);
    REQUIRE(rop.get_prec() == 128);
    r0.cbrt();
    REQUIRE(r0 == -3);
    REQUIRE(r0.get_prec() == 128);
}

#if defined(MPPP_MPFR_HAVE_MPFR_ROOTN_UI)

TEST_CASE("real rootn_ui")
{
    real r0{0};
    real rop;
    REQUIRE(rootn_ui(rop, r0, 3).zero_p());
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(rootn_ui(r0, 3).zero_p());
    REQUIRE(rootn_ui(std::move(r0), 3).zero_p());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    r0 = real{-27, 128};
    REQUIRE(rootn_ui(r0, 3) == -3);
    REQUIRE(rootn_ui(r0, 3).get_prec() == 128);
    rop = real{12, 40};
    rootn_ui(rop, r0, 3);
    REQUIRE(rop == -3);
    REQUIRE(rop.get_prec() == 128);
    rootn_ui(r0, r0, 3);
    REQUIRE(r0 == -3);
    REQUIRE(r0.get_prec() == 128);
    // Special cases.
    REQUIRE(rootn_ui(real{123}, 0).nan_p());
    REQUIRE((rootn_ui(real{"-inf", 45}, 3) == real{"-inf", 45}));
    REQUIRE(rootn_ui(real{-123}, 8).nan_p());
    REQUIRE(!rootn_ui(real{"+0", 60}, 3).signbit());
    REQUIRE(rootn_ui(real{"-0", 60}, 3).signbit());
    REQUIRE(!rootn_ui(real{"+0", 60}, 4).signbit());
    REQUIRE(!rootn_ui(real{"-0", 60}, 4).signbit());
}

#endif

#if defined(MPPP_WITH_ARB)

TEST_CASE("real sqrt1pm1")
{
    real r0{0};
    r0.sqrt1pm1();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0.zero_p());
    real rop;
    REQUIRE(sqrt1pm1(rop, r0).zero_p());
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sqrt1pm1(r0).zero_p());
    REQUIRE(sqrt1pm1(std::move(r0)).zero_p());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = real{15, 128};
    REQUIRE(sqrt1pm1(r0) == 3);
    REQUIRE(sqrt1pm1(r0).get_prec() == 128);
    rop = real{12, 40};
    sqrt1pm1(rop, r0);
    REQUIRE(rop == 3);
    REQUIRE(rop.get_prec() == 128);
    r0.sqrt1pm1();
    REQUIRE(r0 == 3);
    REQUIRE(r0.get_prec() == 128);
    // Negative value.
    r0 = real{-16, 128};
    REQUIRE(sqrt1pm1(r0).nan_p());
    REQUIRE(sqrt1pm1(r0).get_prec() == 128);
    REQUIRE(sqrt1pm1(real{-16, 129}).nan_p());
    REQUIRE(sqrt1pm1(real{-16, 129}).get_prec() == 129);

    // Test infinity.
    REQUIRE(sqrt1pm1(real{"inf", 243}).inf_p());
    REQUIRE(sqrt1pm1(real{"inf", 243}) > 0);
    REQUIRE(sqrt1pm1(real{"inf", 243}).get_prec() == 243);
    REQUIRE(sqrt1pm1(real{"-inf", 243}).nan_p());
    REQUIRE(sqrt1pm1(real{"-inf", 243}).get_prec() == 243);

    // Test nan.
    REQUIRE(sqrt1pm1(real{"nan", 244}).nan_p());
    REQUIRE(sqrt1pm1(real{"nan", 244}).get_prec() == 244);

    // Test a known result.
    REQUIRE(
        abs(sqrt1pm1(1.1_r512)
            - 0.449137674618943857371866415716977172314013287475897308869592480711814437265368042171256319200361749775304608312117024175586888785578864947776625773207505235_r512)
        < mppp::pow(2_r512, -510));
}

#endif
