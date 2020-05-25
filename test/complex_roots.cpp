// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>
#include <utility>

#include <mp++/complex.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"

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
    REQUIRE(!r0.is_valid());
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
