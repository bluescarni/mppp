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
#include <utility>

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

struct view_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        integer n;
        const integer &nc = static_cast<const integer &>(n);
        REQUIRE((mpz_sgn(n.get_mpz_view().get()) == 0));
        REQUIRE(n.get_mpz_view().get()->_mp_d == n._get_union().g_st().m_limbs.data());
        REQUIRE(n.get_mpz_view().get()->_mp_d == nc._get_union().g_st().m_limbs.data());
        {
            auto v = n.get_mpz_view();
            REQUIRE(v.m_ptr == v.m_static_view);
        }
        n.promote();
        REQUIRE((mpz_sgn(n.get_mpz_view().get()) == 0));
        REQUIRE(n.get_mpz_view().get()->_mp_d == n._get_union().g_dy()._mp_d);
        {
            auto v = n.get_mpz_view();
            REQUIRE(v.m_ptr == &n._get_union().g_dy());
            REQUIRE(v.m_ptr == &nc._get_union().g_dy());
        }
        n = 1;
        REQUIRE((mpz_cmp_ui(n.get_mpz_view().get(), 1u) == 0));
        REQUIRE(n.get_mpz_view().get()->_mp_d == n._get_union().g_st().m_limbs.data());
        {
            auto v = n.get_mpz_view();
            REQUIRE(v.m_ptr == v.m_static_view);
        }
        n.promote();
        REQUIRE((mpz_cmp_ui(n.get_mpz_view().get(), 1u) == 0));
        REQUIRE(n.get_mpz_view().get()->_mp_d == n._get_union().g_dy()._mp_d);
        {
            auto v = n.get_mpz_view();
            REQUIRE(v.m_ptr == &n._get_union().g_dy());
            REQUIRE(v.m_ptr == &nc._get_union().g_dy());
        }
        n = -1;
        REQUIRE((mpz_cmp_ui(n.get_mpz_view().get(), 1u) < 0));
        REQUIRE(n.get_mpz_view().get()->_mp_d == n._get_union().g_st().m_limbs.data());
        {
            auto v = n.get_mpz_view();
            REQUIRE(v.m_ptr == v.m_static_view);
        }
        n.promote();
        REQUIRE((mpz_cmp_ui(n.get_mpz_view().get(), 1u) < 0));
        REQUIRE(n.get_mpz_view().get()->_mp_d == n._get_union().g_dy()._mp_d);
        {
            auto v = n.get_mpz_view();
            REQUIRE(v.m_ptr == &n._get_union().g_dy());
        }
        n = 2;
        REQUIRE((mpz_cmp_ui(n.get_mpz_view().get(), 1u) > 0));
        REQUIRE(n.get_mpz_view().get()->_mp_d == n._get_union().g_st().m_limbs.data());
        {
            auto v = n.get_mpz_view();
            REQUIRE(v.m_ptr == v.m_static_view);
        }
        n.promote();
        REQUIRE((mpz_cmp_ui(n.get_mpz_view().get(), 1u) > 0));
        REQUIRE(n.get_mpz_view().get()->_mp_d == n._get_union().g_dy()._mp_d);
        {
            auto v = n.get_mpz_view();
            REQUIRE(v.m_ptr == &n._get_union().g_dy());
        }
        // Test move construction.
        n = 5;
        {
            auto v = n.get_mpz_view();
            auto v2 = std::move(v);
            REQUIRE(v.get()->_mp_d == v2.get()->_mp_d);
            REQUIRE(v2.get()->_mp_d == n._get_union().g_st().m_limbs.data());
            REQUIRE(v.m_ptr == v.m_static_view);
            REQUIRE(v2.m_ptr == v2.m_static_view);
            REQUIRE(v.m_ptr != v2.m_ptr);
            REQUIRE((mpz_cmp_ui(v2.get(), 5u) == 0));
        }
        n.promote();
        {
            auto v = n.get_mpz_view();
            auto v2 = std::move(v);
            REQUIRE(v.get()->_mp_d == v2.get()->_mp_d);
            REQUIRE(v2.get()->_mp_d == n._get_union().g_dy()._mp_d);
            REQUIRE(v.m_ptr == &n._get_union().g_dy());
            REQUIRE(v2.m_ptr == &n._get_union().g_dy());
            REQUIRE((mpz_cmp_ui(v2.get(), 5u) == 0));
        }
    }
};

TEST_CASE("view")
{
    tuple_for_each(sizes{}, view_tester{});
}
