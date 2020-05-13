// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/complex.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"

using namespace mppp;

TEST_CASE("neg")
{
    complex c{1, 2};
    c.neg();
    REQUIRE(c == complex{-1, -2});
    REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
}

TEST_CASE("conj")
{
    complex c{1, 2};
    c.conj();
    REQUIRE(c == complex{1, -2});
    REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
}

TEST_CASE("abs")
{
    complex c{3, 4};
    c.abs();
    REQUIRE(c == complex{5, 0});
    REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
}

TEST_CASE("norm")
{
    complex c{3, 4};
    c.norm();
    REQUIRE(c == complex{25, 0});
    REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
}

TEST_CASE("arg")
{
    complex c{1, 1};
    c.arg();
    REQUIRE(c == real_pi(detail::real_deduce_precision(1)) / 4);
    REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
}

TEST_CASE("proj")
{
    complex c{42, -43};
    c.proj();
    REQUIRE(c == complex{42, -43});
    REQUIRE(c.get_prec() == detail::real_deduce_precision(1));

    c = complex{"(inf, 123)", complex_prec_t(42)};
    c.proj();
    REQUIRE(c == complex{"(inf, 0)", complex_prec_t(42)});
    REQUIRE(c.get_prec() == 42);
    {
        complex::im_cref im{c};

        REQUIRE(!im->signbit());
    }

    c = complex{"(inf, -123)", complex_prec_t(42)};
    c.proj();
    REQUIRE(c == complex{"(inf, 0)", complex_prec_t(42)});
    REQUIRE(c.get_prec() == 42);
    {
        complex::im_cref im{c};

        REQUIRE(im->signbit());
    }
}
