// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mp++/detail/mpfr.hpp>
#include <mp++/real.hpp>
#include <utility>

#include "test_utils.hpp"

using namespace mppp;

TEST_CASE("real sqrt")
{
    real r0{0};
    r0.sqrt();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0.zero_p());
    real rop;
    REQUIRE(sqrt(rop, r0).zero_p());
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sqrt(r0).zero_p());
    REQUIRE(sqrt(std::move(r0)).zero_p());
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

#if MPFR_VERSION_MAJOR >= 4

TEST_CASE("real rootn_ui")
{
    real r0{0};
    real rop;
    REQUIRE(rootn_ui(rop, r0, 3).zero_p());
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(rootn_ui(r0, 3).zero_p());
    REQUIRE(rootn_ui(std::move(r0), 3).zero_p());
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
