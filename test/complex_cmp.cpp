// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>

#include <mp++/complex.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("cmpabs")
{
    using Catch::Matchers::Message;

    REQUIRE(cmpabs(complex{1, 2}, complex{1, 2}) == 0);
    REQUIRE(cmpabs(complex{1, 2}, complex{-1, 2}) == 0);
    REQUIRE(cmpabs(complex{1, -2}, complex{-1, 2}) == 0);
    REQUIRE(cmpabs(complex{-1, -2}, complex{-1, 2}) == 0);

    REQUIRE(cmpabs(complex{-1, -2}, complex{1}) > 0);
    REQUIRE(cmpabs(complex{-1, -2}, complex{-1}) > 0);
    REQUIRE(cmpabs(complex{2}, complex{-1, -2}) < 0);
    REQUIRE(cmpabs(complex{-2}, complex{-1, -2}) < 0);

    REQUIRE_THROWS_MATCHES(
        cmpabs(complex{"(nan,1)", complex_prec_t(5)}, complex{1}), std::domain_error,
        Message(
            "Cannot compare the absolute values of two complex numbers if there are NaNs in the real/imaginary parts"));
    REQUIRE_THROWS_MATCHES(
        cmpabs(complex{"(1, nan)", complex_prec_t(5)}, complex{1}), std::domain_error,
        Message(
            "Cannot compare the absolute values of two complex numbers if there are NaNs in the real/imaginary parts"));
    REQUIRE_THROWS_MATCHES(
        cmpabs(complex{1}, complex{"(nan, 1)", complex_prec_t(5)}), std::domain_error,
        Message(
            "Cannot compare the absolute values of two complex numbers if there are NaNs in the real/imaginary parts"));
    REQUIRE_THROWS_MATCHES(
        cmpabs(complex{1}, complex{"(1, nan)", complex_prec_t(5)}), std::domain_error,
        Message(
            "Cannot compare the absolute values of two complex numbers if there are NaNs in the real/imaginary parts"));
}

TEST_CASE("inf_p")
{
    REQUIRE(!complex{}.inf_p());
    REQUIRE(!inf_p(complex{}));

    REQUIRE(!complex{1, 2}.inf_p());
    REQUIRE(!inf_p(complex{1, 2}));

    REQUIRE(complex{"(inf, 2)", complex_prec_t(32)}.inf_p());
    REQUIRE(complex{"(-inf, 2)", complex_prec_t(32)}.inf_p());
    REQUIRE(inf_p(complex{"(inf, 2)", complex_prec_t(32)}));
    REQUIRE(inf_p(complex{"(-inf, 2)", complex_prec_t(32)}));

    REQUIRE(complex{"(2, inf)", complex_prec_t(32)}.inf_p());
    REQUIRE(complex{"(2, -inf)", complex_prec_t(32)}.inf_p());
    REQUIRE(inf_p(complex{"(2, -inf)", complex_prec_t(32)}));
    REQUIRE(inf_p(complex{"(2, -inf)", complex_prec_t(32)}));

    REQUIRE(complex{"(inf, nan)", complex_prec_t(32)}.inf_p());
    REQUIRE(complex{"(-inf, nan)", complex_prec_t(32)}.inf_p());
    REQUIRE(inf_p(complex{"(inf, nan)", complex_prec_t(32)}));
    REQUIRE(inf_p(complex{"(-inf, nan)", complex_prec_t(32)}));

    REQUIRE(complex{"(nan, inf)", complex_prec_t(32)}.inf_p());
    REQUIRE(complex{"(nan, -inf)", complex_prec_t(32)}.inf_p());
    REQUIRE(inf_p(complex{"(nan, -inf)", complex_prec_t(32)}));
    REQUIRE(inf_p(complex{"(nan, -inf)", complex_prec_t(32)}));

    REQUIRE(!complex{"(nan, -nan)", complex_prec_t(32)}.inf_p());
    REQUIRE(!inf_p(complex{"(-nan, nan)", complex_prec_t(32)}));

    REQUIRE(!complex{"(2, -nan)", complex_prec_t(32)}.inf_p());
    REQUIRE(!inf_p(complex{"(2, -nan)", complex_prec_t(32)}));

    REQUIRE(!complex{"(nan, -2)", complex_prec_t(32)}.inf_p());
    REQUIRE(!inf_p(complex{"(-nan, 2)", complex_prec_t(32)}));
}

TEST_CASE("number_p")
{
    REQUIRE(complex{}.number_p());
    REQUIRE(number_p(complex{}));

    REQUIRE(complex{1, 2}.number_p());
    REQUIRE(number_p(complex{1, 2}));

    REQUIRE(!complex{"(inf, 2)", complex_prec_t(32)}.number_p());
    REQUIRE(!complex{"(-inf, 2)", complex_prec_t(32)}.number_p());
    REQUIRE(!number_p(complex{"(inf, 2)", complex_prec_t(32)}));
    REQUIRE(!number_p(complex{"(-inf, 2)", complex_prec_t(32)}));

    REQUIRE(!complex{"(2, inf)", complex_prec_t(32)}.number_p());
    REQUIRE(!complex{"(2, -inf)", complex_prec_t(32)}.number_p());
    REQUIRE(!number_p(complex{"(2, -inf)", complex_prec_t(32)}));
    REQUIRE(!number_p(complex{"(2, -inf)", complex_prec_t(32)}));

    REQUIRE(!complex{"(inf, nan)", complex_prec_t(32)}.number_p());
    REQUIRE(!complex{"(-inf, nan)", complex_prec_t(32)}.number_p());
    REQUIRE(!number_p(complex{"(inf, nan)", complex_prec_t(32)}));
    REQUIRE(!number_p(complex{"(-inf, nan)", complex_prec_t(32)}));

    REQUIRE(!complex{"(nan, inf)", complex_prec_t(32)}.number_p());
    REQUIRE(!complex{"(nan, -inf)", complex_prec_t(32)}.number_p());
    REQUIRE(!number_p(complex{"(nan, -inf)", complex_prec_t(32)}));
    REQUIRE(!number_p(complex{"(nan, -inf)", complex_prec_t(32)}));

    REQUIRE(!complex{"(nan, -nan)", complex_prec_t(32)}.number_p());
    REQUIRE(!number_p(complex{"(-nan, nan)", complex_prec_t(32)}));

    REQUIRE(!complex{"(2, -nan)", complex_prec_t(32)}.number_p());
    REQUIRE(!number_p(complex{"(2, -nan)", complex_prec_t(32)}));

    REQUIRE(!complex{"(nan, -2)", complex_prec_t(32)}.number_p());
    REQUIRE(!number_p(complex{"(-nan, 2)", complex_prec_t(32)}));
}
