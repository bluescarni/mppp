// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/complex.hpp>
#include <mp++/config.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("literals test")
{
    REQUIRE(0_icr128 == complex{0, 0, complex_prec_t(128)});
    REQUIRE(123_icr128 == complex{0, 123, complex_prec_t(128)});
    REQUIRE((123_icr128).get_prec() == 128);
    REQUIRE(-123_icr128 == complex{0, -123, complex_prec_t(128)});
    REQUIRE((-123_icr128).get_prec() == 128);

#if MPPP_CPLUSPLUS >= 201703L
    // Hex literals are supported as well.
    REQUIRE(0x123.p-7_icr128 == complex{"(0,0x123.p-7)", 16, complex_prec_t(128)});
    REQUIRE(-0X123.p-7_icr128 == -complex{"(0,0x123.p-7)", 16, complex_prec_t(128)});
#endif

    REQUIRE(0_icr256 == complex{0, 0, complex_prec_t(128)});
    REQUIRE(123_icr256 == complex{0, 123, complex_prec_t(256)});
    REQUIRE((123_icr256).get_prec() == 256);
    REQUIRE(-123_icr256 == complex{0, -123, complex_prec_t(256)});
    REQUIRE((-123_icr256).get_prec() == 256);

#if MPPP_CPLUSPLUS >= 201703L
    // Hex literals are supported as well.
    REQUIRE(0x123.p-7_icr256 == complex{"(0,0x123.p-7)", 16, complex_prec_t(256)});
    REQUIRE(-0X123.p-7_icr256 == -complex{"(0,0x123.p-7)", 16, complex_prec_t(256)});
#endif

    REQUIRE(0_icr512 == complex{0, 0, complex_prec_t(128)});
    REQUIRE(123_icr512 == complex{0, 123, complex_prec_t(512)});
    REQUIRE((123_icr512).get_prec() == 512);
    REQUIRE(-123_icr512 == complex{0, -123, complex_prec_t(512)});
    REQUIRE((-123_icr512).get_prec() == 512);

#if MPPP_CPLUSPLUS >= 201703L
    // Hex literals are supported as well.
    REQUIRE(0x123.p-7_icr512 == complex{"(0,0x123.p-7)", 16, complex_prec_t(512)});
    REQUIRE(-0X123.p-7_icr512 == -complex{"(0,0x123.p-7)", 16, complex_prec_t(512)});
#endif

    REQUIRE(0_icr1024 == complex{0, 0, complex_prec_t(128)});
    REQUIRE(123_icr1024 == complex{0, 123, complex_prec_t(1024)});
    REQUIRE((123_icr1024).get_prec() == 1024);
    REQUIRE(-123_icr1024 == complex{0, -123, complex_prec_t(1024)});
    REQUIRE((-123_icr1024).get_prec() == 1024);

#if MPPP_CPLUSPLUS >= 201703L
    // Hex literals are supported as well.
    REQUIRE(0x123.p-7_icr1024 == complex{"(0,0x123.p-7)", 16, complex_prec_t(1024)});
    REQUIRE(-0X123.p-7_icr1024 == -complex{"(0,0x123.p-7)", 16, complex_prec_t(1024)});
#endif
}
