// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>
#include <string>
#include <utility>

#include <mp++/config.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
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

    REQUIRE_THROWS_PREDICATE((real{"nan", real_prec_min()}).sgn(), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot determine the sign of a real NaN");
    });
    REQUIRE_THROWS_PREDICATE(sgn((real{"nan", real_prec_min()})), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot determine the sign of a real NaN");
    });
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
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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

TEST_CASE("real cmpabs")
{
    REQUIRE(cmpabs(real{}, real{}) == 0);
    REQUIRE(cmpabs(real{1}, real{1}) == 0);
    REQUIRE(cmpabs(real{1}, real{0}) > 0);
    REQUIRE(cmpabs(real{-1}, real{0}) > 0);
    REQUIRE(cmpabs(real{0}, real{1}) < 0);
    REQUIRE(cmpabs(real{0}, real{-1}) < 0);
    REQUIRE(cmpabs(real{"inf", 64}, real{45}) > 0);
    REQUIRE(cmpabs(-real{"inf", 64}, real{45}) > 0);
    REQUIRE(cmpabs(real{45}, real{"inf", 64}) < 0);
    REQUIRE(cmpabs(real{45}, -real{"inf", 64}) < 0);
    REQUIRE(cmpabs(-real{"inf", 64}, -real{"inf", 4}) == 0);
    REQUIRE(cmpabs(real{"inf", 64}, -real{"inf", 4}) == 0);
    REQUIRE_THROWS_PREDICATE(cmpabs(real{"nan", 5}, real{6}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what()
               == std::string("Cannot compare the absolute values of two reals if at least one of them is NaN");
    });
    REQUIRE_THROWS_PREDICATE(cmpabs(real{6}, real{"nan", 5}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what()
               == std::string("Cannot compare the absolute values of two reals if at least one of them is NaN");
    });
    REQUIRE_THROWS_PREDICATE(
        cmpabs(real{"nan", 5}, real{"nan", 5}), std::domain_error, [](const std::domain_error &ex) {
            return ex.what()
                   == std::string("Cannot compare the absolute values of two reals if at least one of them is NaN");
        });
}

TEST_CASE("real cmp_2_exp")
{
    REQUIRE(cmp_ui_2exp(real{}, 0, 0) == 0);
    REQUIRE(cmp_si_2exp(real{}, 0, 0) == 0);

    REQUIRE(cmp_ui_2exp(real{16}, 4, 2) == 0);
    REQUIRE(cmp_si_2exp(real{16}, 4, 2) == 0);

    REQUIRE(cmp_ui_2exp(real{2}, 0, 0) > 0);
    REQUIRE(cmp_si_2exp(real{2}, 0, 0) > 0);

    REQUIRE(cmp_ui_2exp(real{-2}, 4, 2) < 0);
    REQUIRE(cmp_si_2exp(real{-32}, -4, 2) < 0);

    REQUIRE_THROWS_PREDICATE(cmp_ui_2exp(real{"nan", 5}, 4, 2), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare a real NaN to an integral multiple of a power of 2");
    });
    REQUIRE_THROWS_PREDICATE(cmp_si_2exp(real{"nan", 5}, 4, 2), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare a real NaN to an integral multiple of a power of 2");
    });
}
