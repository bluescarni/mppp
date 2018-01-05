// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
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

#include "test_utils.hpp"

using namespace mppp;

TEST_CASE("real integer_p")
{
    real r0{0};
    REQUIRE(r0.integer_p());
    REQUIRE(integer_p(r0));
    r0 = .1;
    REQUIRE(!r0.integer_p());
    REQUIRE(!integer_p(r0));
    r0 = -.1;
    REQUIRE(!r0.integer_p());
    REQUIRE(!integer_p(r0));
    r0 = 1;
    REQUIRE(r0.integer_p());
    REQUIRE(integer_p(r0));
    r0 = -1;
    REQUIRE(r0.integer_p());
    REQUIRE(integer_p(r0));
    r0 = 12345;
    REQUIRE(r0.integer_p());
    REQUIRE(integer_p(r0));
    r0 = real{"inf", 128};
    REQUIRE(!r0.integer_p());
    REQUIRE(!integer_p(r0));
    r0 = -real{"inf", 128};
    REQUIRE(!r0.integer_p());
    REQUIRE(!integer_p(r0));
    r0 = real{"nan", 128};
    REQUIRE(!r0.integer_p());
    REQUIRE(!integer_p(r0));
}

TEST_CASE("real trunc")
{
    real r0{0};
    REQUIRE(r0.trunc() == 0);
    r0 = 0.1;
    REQUIRE(r0.trunc() == 0);
    r0 = -0.1;
    REQUIRE(r0.trunc() == 0);
    r0 = 1.001;
    REQUIRE(r0.trunc() == 1);
    r0 = -1.001;
    REQUIRE(r0.trunc() == -1);
    r0 = 1.999;
    REQUIRE(r0.trunc() == 1);
    r0 = -1.9999;
    REQUIRE(r0.trunc() == -1);
    // The binary function.
    real tmp{45.67, 50};
    r0.set_prec(4);
    auto tmp_ptr = r0.get_mpfr_t()->_mpfr_d;
    trunc(r0, std::move(tmp));
    REQUIRE(r0 == 45);
    REQUIRE(get_prec(r0) == 50);
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == tmp_ptr);
    r0.set_prec(4);
    tmp = real{-49.99, 50};
    trunc(r0, real{-49.99, 50});
    REQUIRE(r0 == -49);
    REQUIRE(get_prec(r0) == 50);
    // The unary function.
    r0.set_prec(4);
    tmp = real{45.67, 50};
    r0 = trunc(std::move(tmp));
    REQUIRE(r0 == 45);
    REQUIRE(get_prec(r0) == 50);
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == nullptr);
    tmp = real{45.67, 50};
    r0 = trunc(tmp);
    REQUIRE(r0 == 45);
    REQUIRE(get_prec(r0) == 50);
    r0.set_prec(4);
    r0 = trunc(real{-49.99, 50});
    REQUIRE(r0 == -49);
    REQUIRE(get_prec(r0) == 50);
    // Failure modes.
    r0.set_nan();
    REQUIRE_THROWS_PREDICATE(r0.trunc(), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot truncate a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(trunc(r0, real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot truncate a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(trunc(real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot truncate a NaN value"};
    });
}
