// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

#include <gmp.h>

#include <mp++/integer.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

struct probab_prime_p_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        detail::mpz_raii m1;
        integer n1;
        REQUIRE((probab_prime_p(n1) == mpz_probab_prime_p(&m1.m_mpz, 25)));
        REQUIRE((n1.probab_prime_p() == mpz_probab_prime_p(&m1.m_mpz, 25)));
        mpz_set_ui(&m1.m_mpz, 1u);
        n1 = integer(1);
        REQUIRE((probab_prime_p(n1, 12) == mpz_probab_prime_p(&m1.m_mpz, 25)));
        REQUIRE((n1.probab_prime_p(12) == mpz_probab_prime_p(&m1.m_mpz, 25)));
        mpz_set_ui(&m1.m_mpz, 123u);
        n1 = integer(123);
        REQUIRE((probab_prime_p(n1) == mpz_probab_prime_p(&m1.m_mpz, 25)));
        REQUIRE((n1.probab_prime_p() == mpz_probab_prime_p(&m1.m_mpz, 25)));
        // Couple of sanity checks.
        REQUIRE((probab_prime_p(integer{17}) != 0));
        REQUIRE((probab_prime_p(integer{49979687ll}) != 0));
        REQUIRE((probab_prime_p(integer{128}) == 0));
        // Test errors.
        REQUIRE_THROWS_PREDICATE(probab_prime_p(n1, 0), std::invalid_argument, [](const std::invalid_argument &ex) {
            return std::string(ex.what())
                   == "The number of primality tests must be at least 1, but a value of "
                      "0 was provided instead";
        });
        REQUIRE_THROWS_PREDICATE(n1.probab_prime_p(-1), std::invalid_argument, [](const std::invalid_argument &ex) {
            return std::string(ex.what())
                   == "The number of primality tests must be at least 1, but a value of "
                      "-1 was provided instead";
        });
        n1 = integer(-123);
        REQUIRE_THROWS_PREDICATE(probab_prime_p(n1), std::invalid_argument, [](const std::invalid_argument &ex) {
            return std::string(ex.what()) == "Cannot run primality tests on the negative number -123";
        });
        REQUIRE_THROWS_PREDICATE(n1.probab_prime_p(), std::invalid_argument, [](const std::invalid_argument &ex) {
            return std::string(ex.what()) == "Cannot run primality tests on the negative number -123";
        });
    }
};

TEST_CASE("probab_prime_p")
{
    tuple_for_each(sizes{}, probab_prime_p_tester{});
}
