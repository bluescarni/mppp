// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <limits>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>
#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif
#include <stdexcept>
#include <string>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;
using namespace mppp_test;

using int_t = integer<1>;
using rat_t = rational<1>;

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
    // Integrals.
    REQUIRE(cmp(real{1}, 1) == 0);
    REQUIRE(cmp(1u, real{0}) > 0);
    REQUIRE(cmp(-1l, real{0}) < 0);
    REQUIRE(cmp(real{"inf", 64}, 45ull) > 0);
    REQUIRE(cmp(45ll, real{"inf", 64}) < 0);
    REQUIRE_THROWS_PREDICATE(cmp(real{"nan", 5}, 6), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
    REQUIRE_THROWS_PREDICATE(cmp(6, real{"nan", 5}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
    // FP.
    REQUIRE(cmp(real{1}, 1.f) == 0);
    REQUIRE(cmp(1., real{0}) > 0);
    REQUIRE(cmp(-1.l, real{0}) < 0);
    REQUIRE(cmp(real{"inf", 64}, 45.) > 0);
    REQUIRE(cmp(45.f, real{"inf", 64}) < 0);
    REQUIRE_THROWS_PREDICATE(cmp(real{"nan", 5}, 6.), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
    REQUIRE_THROWS_PREDICATE(cmp(6., real{"nan", 5}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
    if (std::numeric_limits<double>::has_quiet_NaN) {
        REQUIRE_THROWS_PREDICATE(
            cmp(real{5}, std::numeric_limits<double>::quiet_NaN()), std::domain_error, [](const std::domain_error &ex) {
                return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
            });
        REQUIRE_THROWS_PREDICATE(
            cmp(std::numeric_limits<double>::quiet_NaN(), real{5}), std::domain_error, [](const std::domain_error &ex) {
                return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
            });
    }
    // integer/rational.
    REQUIRE(cmp(real{1}, int_t{1}) == 0);
    REQUIRE(cmp(rat_t{1}, real{0}) > 0);
    REQUIRE(cmp(-int_t{1}, real{0}) < 0);
    REQUIRE(cmp(real{"inf", 64}, rat_t{45}) > 0);
    REQUIRE(cmp(int_t{45}, real{"inf", 64}) < 0);
    REQUIRE_THROWS_PREDICATE(cmp(real{"nan", 5}, rat_t{6}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
    REQUIRE_THROWS_PREDICATE(cmp(int_t{6}, real{"nan", 5}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE(cmp(real{1}, real128{1}) == 0);
    REQUIRE(cmp(real128{1}, real{0}) > 0);
    REQUIRE(cmp(-real128{1}, real{0}) < 0);
    REQUIRE(cmp(real{"inf", 64}, real128{45}) > 0);
    REQUIRE(cmp(real128{45}, real{"inf", 64}) < 0);
    REQUIRE(cmp(real128_inf(), real{"inf", 64}) == 0);
    REQUIRE(cmp(-real{"inf", 64}, -real128_inf()) == 0);
    REQUIRE_THROWS_PREDICATE(cmp(real{"nan", 5}, real128{6}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
    REQUIRE_THROWS_PREDICATE(cmp(real128{6}, real{"nan", 5}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
    REQUIRE_THROWS_PREDICATE(cmp(real{5}, real128_nan()), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
    REQUIRE_THROWS_PREDICATE(cmp(real128_nan(), real{5}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string("Cannot compare two reals if at least one of them is NaN");
    });
#endif
}
