// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
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

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

TEST_CASE("real pi")
{
    auto r0 = real_pi(12);
    REQUIRE(std::is_same<real, decltype(real_pi(12))>::value);
    REQUIRE(r0.get_prec() == 12);
    REQUIRE((r0 == real{"3.1416", 12}));
    REQUIRE_THROWS_PREDICATE(r0 = real_pi(0), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of 0: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    REQUIRE_THROWS_PREDICATE(r0 = real_pi(-1), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of -1: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    REQUIRE_THROWS_PREDICATE(r0 = real_pi(-100), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of -100: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    r0.set_prec(86);
    REQUIRE(real_pi(r0).get_prec() == 86);
    REQUIRE(std::is_same<real &, decltype(real_pi(r0))>::value);
    REQUIRE((r0 == real{"3.141592653589793238462643402", 86}));
}

TEST_CASE("real log2")
{
    auto r0 = real_log2(12);
    REQUIRE(std::is_same<real, decltype(real_log2(12))>::value);
    REQUIRE(r0.get_prec() == 12);
    REQUIRE((r0 == real{"0.693115", 12}));
    REQUIRE_THROWS_PREDICATE(r0 = real_log2(0), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of 0: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    REQUIRE_THROWS_PREDICATE(r0 = real_log2(-1), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of -1: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    REQUIRE_THROWS_PREDICATE(r0 = real_log2(-100), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of -100: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    r0.set_prec(86);
    REQUIRE(real_log2(r0).get_prec() == 86);
    REQUIRE(std::is_same<real &, decltype(real_log2(r0))>::value);
    REQUIRE((r0 == real{"0.6931471805599453094172321228", 86}));
}

TEST_CASE("real catalan")
{
    auto r0 = real_catalan(12);
    REQUIRE(std::is_same<real, decltype(real_catalan(12))>::value);
    REQUIRE(r0.get_prec() == 12);
    REQUIRE((r0 == real{"0.916016", 12}));
    REQUIRE_THROWS_PREDICATE(r0 = real_catalan(0), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of 0: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    REQUIRE_THROWS_PREDICATE(r0 = real_catalan(-1), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of -1: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    REQUIRE_THROWS_PREDICATE(r0 = real_catalan(-100), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of -100: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    r0.set_prec(86);
    REQUIRE(real_catalan(r0).get_prec() == 86);
    REQUIRE(std::is_same<real &, decltype(real_catalan(r0))>::value);
    REQUIRE((r0 == real{"0.9159655941772190150546035185", 86}));
}

TEST_CASE("real euler")
{
    auto r0 = real_euler(12);
    REQUIRE(std::is_same<real, decltype(real_euler(12))>::value);
    REQUIRE(r0.get_prec() == 12);
    REQUIRE((r0 == real{"0.577148", 12}));
    REQUIRE_THROWS_PREDICATE(r0 = real_euler(0), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of 0: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    REQUIRE_THROWS_PREDICATE(r0 = real_euler(-1), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of -1: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    REQUIRE_THROWS_PREDICATE(r0 = real_euler(-100), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real constant with a precision of -100: the value must be between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    r0.set_prec(86);
    REQUIRE(real_euler(r0).get_prec() == 86);
    REQUIRE(std::is_same<real &, decltype(real_euler(r0))>::value);
    REQUIRE((r0 == real{"0.5772156649015328606065120917", 86}));
}
