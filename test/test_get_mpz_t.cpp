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
#include <tuple>
#include <type_traits>

#include <mp++.hpp>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

static int ntries = 1000;

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

struct get_mpz_t_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n;
        auto ptr = n.get_mpz_t();
        REQUIRE(n.is_dynamic());
        REQUIRE((mpz_sgn(ptr) == 0));
        auto view = n.get_mpz_view();
        REQUIRE((mpz_sgn(view.get()) == 0));
        REQUIRE(view.get() == ptr);
        n = 12;
        REQUIRE(n.is_static());
        ptr = n.get_mpz_t();
        REQUIRE(n.is_dynamic());
        REQUIRE((mpz_sgn(ptr) == 1));
        REQUIRE((mpz_cmp_ui(ptr, 12) == 0));
        auto view2 = n.get_mpz_view();
        REQUIRE((mpz_sgn(view2.get()) == 1));
        REQUIRE(view2.get() == ptr);
        ptr = n.get_mpz_t();
        REQUIRE(n.is_dynamic());
        REQUIRE((mpz_sgn(ptr) == 1));
        REQUIRE((mpz_cmp_ui(ptr, 12) == 0));
        auto view2a = n.get_mpz_view();
        REQUIRE((mpz_sgn(view2a.get()) == 1));
        REQUIRE(view2a.get() == ptr);
        n = -23;
        REQUIRE(n.is_static());
        ptr = n.get_mpz_t();
        REQUIRE(n.is_dynamic());
        REQUIRE((mpz_sgn(ptr) == -1));
        REQUIRE((mpz_cmp_si(ptr, -23) == 0));
        auto view3 = n.get_mpz_view();
        REQUIRE((mpz_sgn(view3.get()) == -1));
        REQUIRE(view3.get() == ptr);
        ptr = n.get_mpz_t();
        REQUIRE(n.is_dynamic());
        REQUIRE((mpz_sgn(ptr) == -1));
        REQUIRE((mpz_cmp_si(ptr, -23) == 0));
        auto view3a = n.get_mpz_view();
        REQUIRE((mpz_sgn(view3a.get()) == -1));
        REQUIRE(view3a.get() == ptr);
    }
};

TEST_CASE("get_mpz_t")
{
    tuple_for_each(sizes{}, get_mpz_t_tester{});
}
