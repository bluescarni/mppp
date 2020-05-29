// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>

#include <mp++/complex.hpp>

#include "catch.hpp"

using namespace mppp;

TEST_CASE("cmp_abs")
{
    using Catch::Matchers::Message;

    REQUIRE(cmp_abs(complex{1, 2}, complex{1, 2}) == 0);
    REQUIRE(cmp_abs(complex{1, 2}, complex{-1, 2}) == 0);
    REQUIRE(cmp_abs(complex{1, -2}, complex{-1, 2}) == 0);
    REQUIRE(cmp_abs(complex{-1, -2}, complex{-1, 2}) == 0);

    REQUIRE(cmp_abs(complex{-1, -2}, complex{1}) > 0);
    REQUIRE(cmp_abs(complex{-1, -2}, complex{-1}) > 0);
    REQUIRE(cmp_abs(complex{2}, complex{-1, -2}) < 0);
    REQUIRE(cmp_abs(complex{-2}, complex{-1, -2}) < 0);

    REQUIRE_THROWS_MATCHES(
        cmp_abs(complex{"(nan,1)", complex_prec_t(5)}, complex{1}), std::domain_error,
        Message(
            "Cannot compare the absolute values of two complex numbers if there are NaNs in the real/imaginary parts"));
    REQUIRE_THROWS_MATCHES(
        cmp_abs(complex{"(1, nan)", complex_prec_t(5)}, complex{1}), std::domain_error,
        Message(
            "Cannot compare the absolute values of two complex numbers if there are NaNs in the real/imaginary parts"));
    REQUIRE_THROWS_MATCHES(
        cmp_abs(complex{1}, complex{"(nan, 1)", complex_prec_t(5)}), std::domain_error,
        Message(
            "Cannot compare the absolute values of two complex numbers if there are NaNs in the real/imaginary parts"));
    REQUIRE_THROWS_MATCHES(
        cmp_abs(complex{1}, complex{"(1, nan)", complex_prec_t(5)}), std::domain_error,
        Message(
            "Cannot compare the absolute values of two complex numbers if there are NaNs in the real/imaginary parts"));
}
