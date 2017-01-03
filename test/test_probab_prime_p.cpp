/* Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)

This file is part of the mp++ library.

The mp++ library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The mp++ library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the mp++ library.  If not,
see https://www.gnu.org/licenses/. */

#include <cstddef>
#include <gmp.h>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

#include <mp++.hpp>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

static int ntries = 1000;

using namespace mppp;
using namespace mppp::mppp_impl;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

struct probab_prime_p_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        mpz_raii m1;
        integer n1;
        REQUIRE((probab_prime_p(n1) == ::mpz_probab_prime_p(&m1.m_mpz, 25)));
        REQUIRE((n1.probab_prime_p() == ::mpz_probab_prime_p(&m1.m_mpz, 25)));
        ::mpz_set_ui(&m1.m_mpz, 1u);
        n1 = integer(1);
        REQUIRE((probab_prime_p(n1, 12) == ::mpz_probab_prime_p(&m1.m_mpz, 25)));
        REQUIRE((n1.probab_prime_p(12) == ::mpz_probab_prime_p(&m1.m_mpz, 25)));
        ::mpz_set_ui(&m1.m_mpz, 123u);
        n1 = integer(123);
        REQUIRE((probab_prime_p(n1) == ::mpz_probab_prime_p(&m1.m_mpz, 25)));
        REQUIRE((n1.probab_prime_p() == ::mpz_probab_prime_p(&m1.m_mpz, 25)));
        // Couple of sanity checks.
        REQUIRE((probab_prime_p(integer{17}) != 0));
        REQUIRE((probab_prime_p(integer{49979687ll}) != 0));
        REQUIRE((probab_prime_p(integer{128}) == 0));
        // Test errors.
        REQUIRE_THROWS_PREDICATE(probab_prime_p(n1, 0), std::invalid_argument, [](const std::invalid_argument &ex) {
            return std::string(ex.what()) == "The number of primality tests must be at least 1, but a value of "
                                             "0 was provided instead";
        });
        REQUIRE_THROWS_PREDICATE(n1.probab_prime_p(-1), std::invalid_argument, [](const std::invalid_argument &ex) {
            return std::string(ex.what()) == "The number of primality tests must be at least 1, but a value of "
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
