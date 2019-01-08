// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <mp++/detail/mpfr.hpp>
#include <mp++/real.hpp>
#include <stdexcept>
#include <string>
#include <utility>

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
    REQUIRE(!real{12}.is_zero());
    REQUIRE(!is_zero(real{12}));
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
    REQUIRE(real{}.is_zero());
    REQUIRE(is_zero(real{}));
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
    REQUIRE(!real{"nan"}.is_zero());
    REQUIRE(!is_zero(real{"nan"}));
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
    REQUIRE(!real{"inf"}.is_zero());
    REQUIRE(!is_zero(real{"inf"}));
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

TEST_CASE("real cmp")
{
    REQUIRE(cmp(real{}, real{}) == 0);
    REQUIRE(cmp(real{1}, real{1}) == 0);
    REQUIRE(cmp(real{1}, real{0}) > 0);
    REQUIRE(cmp(real{-1}, real{0}) < 0);
    REQUIRE(cmp(real{"inf", 64}, real{45}) > 0);
    REQUIRE(cmp(-real{"inf", 64}, real{45}) < 0);
    REQUIRE(cmp(-real{"inf", 64}, -real{"inf", 4}) == 0);
    REQUIRE(cmp(real{"inf", 64}, real{"inf", 4}) == 0);
    REQUIRE_THROWS_PREDICATE(cmp(real{"nan", 5}, real{6}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
    REQUIRE_THROWS_PREDICATE(cmp(real{6}, real{"nan", 5}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
    REQUIRE_THROWS_PREDICATE(cmp(real{"nan", 5}, real{"nan", 5}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
}

TEST_CASE("real real_equal_to")
{
    REQUIRE(real_equal_to(real{}, real{}));
    REQUIRE(real_equal_to(real{1}, real{1}));
    REQUIRE(!real_equal_to(real{2}, real{1}));
    REQUIRE(!real_equal_to(real{1}, real{2}));
    REQUIRE(!real_equal_to(real{"-nan", 5}, real{2}));
    REQUIRE(real_equal_to(real{"nan", 5}, real{"-nan", 6}));
    REQUIRE(!real_equal_to(real{2}, real{"nan", 5}));
}

TEST_CASE("real real_lt")
{
    REQUIRE(!real_lt(real{}, real{}));
    REQUIRE(!real_lt(real{1}, real{1}));
    REQUIRE(!real_lt(real{2}, real{1}));
    REQUIRE(real_lt(real{1}, real{2}));
    REQUIRE(!real_lt(real{"-nan", 5}, real{2}));
    REQUIRE(!real_lt(real{"nan", 5}, real{"-nan", 6}));
    REQUIRE(real_lt(real{2}, real{"nan", 5}));
    real r0, r1{std::move(r0)};
    (void)r1;
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    REQUIRE(real_lt(real{2}, r0));
    REQUIRE(!real_lt(r0, real{2}));
    REQUIRE(!real_lt(r0, r0));
    REQUIRE(real_lt(real{"nan", 5}, r0));
    REQUIRE(!real_lt(r0, real{"-nan", 5}));
}

TEST_CASE("real real_gt")
{
    REQUIRE(!real_gt(real{}, real{}));
    REQUIRE(!real_gt(real{1}, real{1}));
    REQUIRE(real_gt(real{2}, real{1}));
    REQUIRE(!real_gt(real{1}, real{2}));
    REQUIRE(real_gt(real{"-nan", 5}, real{2}));
    REQUIRE(!real_gt(real{"nan", 5}, real{"-nan", 6}));
    REQUIRE(!real_gt(real{2}, real{"nan", 5}));
    real r0, r1{std::move(r0)};
    (void)r1;
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    REQUIRE(!real_gt(real{2}, r0));
    REQUIRE(real_gt(r0, real{2}));
    REQUIRE(!real_gt(r0, r0));
    REQUIRE(!real_gt(real{"nan", 5}, r0));
    REQUIRE(real_gt(r0, real{"-nan", 5}));
}

TEST_CASE("real is_one")
{
    REQUIRE(!is_one(real{}));
    REQUIRE(!real{}.is_one());
    REQUIRE(!is_one(real{-1}));
    REQUIRE(!real{-1}.is_one());
    REQUIRE(is_one(real{1}));
    REQUIRE(real{1}.is_one());
    REQUIRE(!is_one(real{1.001}));
    REQUIRE(!real{1.001}.is_one());
    REQUIRE(!is_one(real{"inf", 5}));
    REQUIRE(!real{"inf", 5}.is_one());
    REQUIRE(!is_one(real{"-inf", 5}));
    REQUIRE(!real{"-inf", 5}.is_one());
    REQUIRE(!::mpfr_erangeflag_p());
    REQUIRE(!is_one(real{"nan", 5}));
    REQUIRE(!::mpfr_erangeflag_p());
    REQUIRE(!real{"nan", 5}.is_one());
    REQUIRE(!::mpfr_erangeflag_p());
    REQUIRE(!is_one(real{"-nan", 5}));
    REQUIRE(!::mpfr_erangeflag_p());
    REQUIRE(!real{"-nan", 5}.is_one());
    REQUIRE(!::mpfr_erangeflag_p());
}
