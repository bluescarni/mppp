/* Copyright 2009-2016 Francesco Biscani (bluescarni@gmail.com)

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

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

struct add_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        integer n1{1}, n2{-2};
        REQUIRE((lex_cast(+n2) == "-2"));
        REQUIRE((lex_cast(n1 + n2) == "-1"));
        REQUIRE((std::is_same<decltype(n1 + n2), integer>::value));
        REQUIRE((lex_cast(n1 + char(4)) == "5"));
        REQUIRE((lex_cast(char(4) + n2) == "2"));
        REQUIRE((std::is_same<decltype(n1 + char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) + n2), integer>::value));
        REQUIRE((lex_cast(n1 + (unsigned char)(4)) == "5"));
        REQUIRE((lex_cast((unsigned char)(4) + n2) == "2"));
        REQUIRE((lex_cast(n1 + short(4)) == "5"));
        REQUIRE((lex_cast(short(4) + n2) == "2"));
        REQUIRE((lex_cast(n1 + 4) == "5"));
        REQUIRE((lex_cast(4 + n2) == "2"));
        REQUIRE((std::is_same<decltype(n1 + 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 + n2), integer>::value));
        REQUIRE((lex_cast(n1 + 4u) == "5"));
        REQUIRE((lex_cast(4u + n2) == "2"));
        REQUIRE((n1 + 4.f == 5.f));
        REQUIRE((4.f + n2 == 2.f));
        REQUIRE((std::is_same<decltype(n1 + 4.f), float>::value));
        REQUIRE((std::is_same<decltype(4.f + n2), float>::value));
        REQUIRE((n1 + 4. == 5.));
        REQUIRE((4. + n2 == 2.));
        REQUIRE((std::is_same<decltype(n1 + 4.), double>::value));
        REQUIRE((std::is_same<decltype(4. + n2), double>::value));
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE((n1 + 4.l == 5.l));
        REQUIRE((4.l + n2 == 2.l));
        REQUIRE((std::is_same<decltype(n1 + 4.l), long double>::value));
        REQUIRE((std::is_same<decltype(4.l + n2), long double>::value));
#endif
    }
};

TEST_CASE("add")
{
    tuple_for_each(sizes{}, add_tester{});
}
