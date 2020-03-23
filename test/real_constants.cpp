// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>
#include <string>
#include <type_traits>

#include <mp++/real.hpp>

#include "catch.hpp"
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
    r0.set_prec(86);
    REQUIRE(real_pi(r0).get_prec() == 86);
    REQUIRE((r0 == real{"3.141592653589793238462643402", 86}));
}
