// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>
#include <utility>

#include <mp++/real.hpp>

#include "catch.hpp"

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
        REQUIRE(!r0.is_valid());
    }
}
