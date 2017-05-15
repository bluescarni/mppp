// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/mp++.hpp>

#include <cstddef>
#include <iostream>
#include <tuple>
#include <type_traits>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

struct rat_ctor_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rat_t = rational<S::value>;
        using int_t = typename rat_t::int_t;
        std::cout << "n static limbs: " << S::value << ", size: " << sizeof(rat_t) << '\n';
        rat_t q;
        q = rat_t{1, 2};
        std::cout << q << '\n';
        q = rat_t{2, -4};
        q = rat_t{2, int_t{-4}};
        std::cout << q << '\n';
        q = rat_t{1.3};
        std::cout << q << '\n';
        q = rat_t{1.3f};
        std::cout << q << '\n';
        rat_t q1, q2, rop;
        add(rop, q1, q2);
        std::cout << rop << '\n';
    }
};

TEST_CASE("rational constructors")
{
    tuple_for_each(sizes{}, rat_ctor_tester{});
}
