// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#if defined(_MSC_VER)

// Disable some warnings for MSVC. These arise only in release mode apparently.
#pragma warning(push)
#pragma warning(disable : 4723)

#endif

#include <cstddef>
#include <tuple>
#include <type_traits>

#include <mp++/integer.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

struct sqr_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer ret;

        // A few simple tests.
        sqr(ret, integer{0});
        REQUIRE(ret == 0);

        sqr(ret, integer{1});
        REQUIRE(ret == 1);

        sqr(ret, integer{0});
        REQUIRE(ret == 0);

        sqr(ret, integer{2});
        REQUIRE(ret == 4);

        sqr(ret, integer{-1});
        REQUIRE(ret == 1);

        sqr(ret, integer{-2});
        REQUIRE(ret == 4);

        sqr(ret, integer{20883l});
        REQUIRE(ret == 436099689l);

        sqr(ret, integer{-8070l});
        REQUIRE(ret == 65124900l);
    }
};

TEST_CASE("sqr")
{
    tuple_for_each(sizes{}, sqr_tester{});
}

#if defined(_MSC_VER)

#pragma warning(pop)

#endif
