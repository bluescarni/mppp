// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/mp++.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("real constructors")
{
    real r;
    std::cout << r << '\n';
    r.set_prec(53);
    ::arf_set_d(r.get_arf_t(), 1. / (2 << 6) + 1. / (2 << 7));
    std::cout << r << '\n';
    ::arf_set_d(r.get_arf_t(), 1);
    std::cout << r << '\n';
    real r2(123u);
    std::cout << r2 << '\n';
    std::cout << real{integer<1>{"123213218372891837218937218937289137891273892173892713897128937128937"}} << '\n';
    std::cout << real{1.3}.nbits() << '\n';
    std::cout << real{1.3f}.nbits() << '\n';
    std::cout << real{1.3l}.nbits() << '\n';
    std::cout << real{1.3l, 15}.nbits() << '\n';
    std::cout << real{1.3l, 15, true}.nbits() << '\n';
    std::cout << real{1.3l, 15, true} << '\n';
    std::cout << real{true} << '\n';
}
