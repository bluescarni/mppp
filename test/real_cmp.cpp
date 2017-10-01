// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <mp++/real.hpp>
#include <stdexcept>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;
using namespace mppp_test;

TEST_CASE("real naninf")
{
    REQUIRE(!real{12}.nan_p());
    REQUIRE(!nan_p(real{12}));
    REQUIRE(!real{12}.inf_p());
    REQUIRE(!inf_p(real{12}));
    REQUIRE(real{12}.number_p());
    REQUIRE(number_p(real{12}));
    REQUIRE(!real{12}.zero_p());
    REQUIRE(!zero_p(real{12}));
    REQUIRE(real{12}.regular_p());
    REQUIRE(regular_p(real{12}));

    REQUIRE(!real{}.nan_p());
    REQUIRE(!nan_p(real{}));
    REQUIRE(!real{}.inf_p());
    REQUIRE(!inf_p(real{}));
    REQUIRE(real{}.number_p());
    REQUIRE(number_p(real{}));
    REQUIRE(real{}.zero_p());
    REQUIRE(zero_p(real{}));
    REQUIRE(!real{}.regular_p());
    REQUIRE(!regular_p(real{}));

    real_set_default_prec(128);
    REQUIRE(real{"nan"}.nan_p());
    REQUIRE(nan_p(real{"nan"}));
    REQUIRE(!real{"nan"}.inf_p());
    REQUIRE(!inf_p(real{"nan"}));
    REQUIRE(!real{"nan"}.number_p());
    REQUIRE(!number_p(real{"nan"}));
    REQUIRE(!real{"nan"}.zero_p());
    REQUIRE(!zero_p(real{"nan"}));
    REQUIRE(!real{"nan"}.regular_p());
    REQUIRE(!regular_p(real{"nan"}));

    REQUIRE(!real{"inf"}.nan_p());
    REQUIRE(!nan_p(real{"inf"}));
    REQUIRE(real{"inf"}.inf_p());
    REQUIRE(inf_p(real{"-inf"}));
    REQUIRE(!real{"inf"}.number_p());
    REQUIRE(!number_p(real{"inf"}));
    REQUIRE(!real{"inf"}.zero_p());
    REQUIRE(!zero_p(real{"inf"}));
    REQUIRE(!real{"inf"}.regular_p());
    REQUIRE(!regular_p(real{"-inf"}));
    real_reset_default_prec();
}

TEST_CASE("real sign")
{
    REQUIRE(real{}.sgn() == 0);
    REQUIRE(!real{}.signbit());
    REQUIRE(sgn(real{}) == 0);
    REQUIRE(!signbit(real{}));

    REQUIRE(real{2}.sgn() > 0);
    REQUIRE(!real{2}.signbit());
    REQUIRE(sgn(real{2}) > 0);
    REQUIRE(!signbit(real{2}));

    REQUIRE(real{-2}.sgn() < 0);
    REQUIRE(real{-2}.signbit());
    REQUIRE(sgn(real{-2}) < 0);
    REQUIRE(signbit(real{-2}));

    real_set_default_prec(128);

    REQUIRE(real{"-0"}.sgn() == 0);
    REQUIRE(real{"-0"}.signbit());
    REQUIRE(sgn(real{"-0"}) == 0);
    REQUIRE(signbit(real{"-0"}));

    REQUIRE(real{"inf"}.sgn() > 0);
    REQUIRE(!real{"inf"}.signbit());
    REQUIRE(sgn(real{"inf"}) > 0);
    REQUIRE(!signbit(real{"inf"}));

    REQUIRE(real{"-inf"}.sgn() < 0);
    REQUIRE(real{"-inf"}.signbit());
    REQUIRE(sgn(real{"-inf"}) < 0);
    REQUIRE(signbit(real{"-inf"}));

    REQUIRE_THROWS_PREDICATE(real{"nan"}.sgn(), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot determine the sign of a real NaN");
    });
    REQUIRE_THROWS_PREDICATE(sgn(real{"nan"}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot determine the sign of a real NaN");
    });

    real_reset_default_prec();
}
