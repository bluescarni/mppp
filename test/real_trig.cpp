// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mp++/real.hpp>
#include <utility>

#include "test_utils.hpp"

using namespace mppp;

TEST_CASE("real sin cos")
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
    REQUIRE(acos(std::move(r0)) == real_pi(r0.get_prec()) / 2);
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
    REQUIRE(atan(std::move(r0)) == real_pi(r0.get_prec()) / 4);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}
