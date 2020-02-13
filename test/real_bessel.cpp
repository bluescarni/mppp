// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <utility>

#include <mp++/real.hpp>

#include "catch.hpp"

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
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}
