// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>

#include <mp++/real.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using int_t = integer<1>;

TEST_CASE("real set_z_2exp")
{
    real r0{45};
    set_z_2exp(r0, int_t{2}, 4);
    REQUIRE(r0 == 32);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(45));
    set_z_2exp(r0, int_t{-1}, -1);
    REQUIRE(r0 == real{"-.5", 7});
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(45));
    set_z_2exp(r0, int_t{0}, -1);
    REQUIRE(r0 == real{});
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(45));
    REQUIRE(!r0.signbit());
}

TEST_CASE("real get_z_2exp")
{
    real r0{45};
    set_z_2exp(r0, int_t{2}, 4);
    int_t n;
    auto exp = get_z_2exp(n, r0);
    REQUIRE(real{n} * pow(2, real{exp}) == r0);
    set_z_2exp(r0, int_t{-2}, -4);
    exp = get_z_2exp(n, r0);
    REQUIRE(real{n} * pow(2, real{exp}) == r0);
    const auto old_n(n);
    REQUIRE_THROWS_PREDICATE(get_z_2exp(n, real{"nan", 5}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot extract the significand and the exponent of a non-finite real");
    });
    REQUIRE_THROWS_PREDICATE(get_z_2exp(n, real{"inf", 5}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot extract the significand and the exponent of a non-finite real");
    });
    REQUIRE_THROWS_PREDICATE(get_z_2exp(n, real{"-inf", 5}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot extract the significand and the exponent of a non-finite real");
    });
    REQUIRE(n == old_n);
    exp = get_z_2exp(n, real{});
    (void)exp;
    REQUIRE(n.is_zero());
}

TEST_CASE("real set_ui_si_2exp")
{
    real r0{45};
    set_ui_2exp(r0, 2, 4);
    REQUIRE(r0 == 32);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(45));
    set_si_2exp(r0, 2, 5);
    REQUIRE(r0 == 64);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(45));
    set_si_2exp(r0, -1, -1);
    REQUIRE(r0 == real{"-.5", 7});
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(45));
    set_si_2exp(r0, 0, -1);
    REQUIRE(r0 == real{});
    REQUIRE(!r0.signbit());
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(45));
    set_ui_2exp(r0, 0, -1);
    REQUIRE(r0 == real{});
    REQUIRE(!r0.signbit());
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(45));
}
