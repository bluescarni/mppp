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
#include <utility>

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

struct view_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n;
        const auto &nc = static_cast<const integer &>(n);
        REQUIRE((mpz_sgn(n.get_mpz_view().get()) == 0));
        REQUIRE(n.get_mpz_view().get()->_mp_d == n._get_union().g_st().m_limbs.data());
        REQUIRE(n.get_mpz_view().get()->_mp_d == nc._get_union().g_st().m_limbs.data());
        {
            auto v = n.get_mpz_view();
            REQUIRE(v.m_ptr == &v.m_static_view);
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
            REQUIRE(v.m_ptr == &v.m_static_view);
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
            REQUIRE(v.m_ptr == &v.m_static_view);
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
            REQUIRE(v.m_ptr == &v.m_static_view);
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
            // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
            REQUIRE(v.get()->_mp_d == v2.get()->_mp_d);
            REQUIRE(v2.get()->_mp_d == n._get_union().g_st().m_limbs.data());
            REQUIRE(v.m_ptr == &v.m_static_view);
            REQUIRE(v2.m_ptr == &v2.m_static_view);
            REQUIRE(v.m_ptr != v2.m_ptr);
            REQUIRE((mpz_cmp_ui(v2.get(), 5u) == 0));
        }
        n.promote();
        {
            auto v = n.get_mpz_view();
            auto v2 = std::move(v);
            // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
