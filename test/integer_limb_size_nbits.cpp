// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/detail/gmp.hpp>
#include <mp++/integer.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("limb_size_nbits")
{
    REQUIRE(detail::limb_size_nbits(1) == 1u);
    REQUIRE(detail::limb_size_nbits(2) == 2u);
    REQUIRE(detail::limb_size_nbits(3) == 2u);
    REQUIRE(detail::limb_size_nbits(4) == 3u);
    REQUIRE(detail::limb_size_nbits(::mp_limb_t(1) << (GMP_NUMB_BITS - 1)) == GMP_NUMB_BITS);
    REQUIRE(detail::limb_size_nbits(::mp_limb_t(1) << (GMP_NUMB_BITS - 2)) == GMP_NUMB_BITS - 1);
    REQUIRE(detail::limb_size_nbits((::mp_limb_t(1) << (GMP_NUMB_BITS - 1)) + 1) == GMP_NUMB_BITS);
    REQUIRE(detail::limb_size_nbits(::mp_limb_t(1) << (GMP_NUMB_BITS - 2)) == GMP_NUMB_BITS - 1);
    REQUIRE(detail::limb_size_nbits((::mp_limb_t(1) << (GMP_NUMB_BITS - 2)) + 1) == GMP_NUMB_BITS - 1);
    // Test the GMP implementation, which we do not cover in the CI.
    ::mp_limb_t l = 1;
    REQUIRE(static_cast<unsigned>(mpn_sizeinbase(&l, 1, 2)) == 1u);
    l = 2;
    REQUIRE(static_cast<unsigned>(mpn_sizeinbase(&l, 1, 2)) == 2u);
    l = 3;
    REQUIRE(static_cast<unsigned>(mpn_sizeinbase(&l, 1, 2)) == 2u);
    l = 4;
    REQUIRE(static_cast<unsigned>(mpn_sizeinbase(&l, 1, 2)) == 3u);
    l = ::mp_limb_t(1) << (GMP_NUMB_BITS - 1);
    REQUIRE(static_cast<unsigned>(mpn_sizeinbase(&l, 1, 2)) == GMP_NUMB_BITS);
    l += 1;
    REQUIRE(static_cast<unsigned>(mpn_sizeinbase(&l, 1, 2)) == GMP_NUMB_BITS);
    l = ::mp_limb_t(1) << (GMP_NUMB_BITS - 2);
    REQUIRE(static_cast<unsigned>(mpn_sizeinbase(&l, 1, 2)) == GMP_NUMB_BITS - 1);
    l += 1;
    REQUIRE(static_cast<unsigned>(mpn_sizeinbase(&l, 1, 2)) == GMP_NUMB_BITS - 1);
}
