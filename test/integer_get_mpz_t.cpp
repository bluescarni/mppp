// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
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
