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
#include <stdexcept>
#include <string>
#include <type_traits>

#include "test_utils.hpp"

using namespace mppp;
using namespace mppp_test;

TEST_CASE("real pi")
{
    auto r0 = real_pi(12);
    REQUIRE(r0.get_prec() == 12);
    REQUIRE((r0 == real{"3.1416", 12}));
    REQUIRE_THROWS_PREDICATE(r0 = real_pi(-1), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of -1: the value must be either zero or between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    REQUIRE_THROWS_PREDICATE(r0 = real_pi(-100), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of -100: the value must be either zero or between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    REQUIRE_THROWS_PREDICATE(r0 = real_pi(), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string{"Cannot init a real constant with an automatically-deduced precision if "
                              "the global default precision has not been set"};
    });
    real_set_default_prec(42);
    r0 = real_pi();
    REQUIRE(r0.get_prec() == 42);
    REQUIRE((r0 == real{"3.14159265359012", 42}));
    real_reset_default_prec();
    r0.set_prec(86);
    REQUIRE(real_pi(r0).get_prec() == 86);
    REQUIRE((r0 == real{"3.141592653589793238462643402", 86}));
}
