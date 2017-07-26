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
    real r(2);
    std::cout << r << '\n';
    r.set_prec(53);
    ::arf_sqrt(r._get_arf_t(), r.get_arf_t(), 53, ARF_RND_NEAR);
    std::cout << r << '\n';
    std::cout << r.bits() << '\n';
    ::arf_set_d(r._get_arf_t(), 1. / (2 << 6) + 1. / (2 << 7));
    std::cout << r << '\n';
    ::arf_set_d(r._get_arf_t(), 1);
    std::cout << r << '\n';
    real r2(123u);
    std::cout << r2 << '\n';
    std::cout << real{integer<1>{"123213218372891837218937218937289137891273892173892713897128937128937"}} << '\n';
    std::cout << real{1.3}.bits() << '\n';
    std::cout << real{1.3f}.bits() << '\n';
    std::cout << real{1.3l}.bits() << '\n';
    std::cout << real{1.3l, 15}.bits() << '\n';
    std::cout << real{1.3l, 15} << '\n';
    std::cout << real{true} << '\n';
    std::cout << real{rational<1>{"321938201983012839208312/210938120938010101029381298"}} << '\n';
    std::cout << real{rational<1>{"34/89"}, 18} << '\n';
    std::cout << real{rational<1>{"34/89"}, 18}.bits() << '\n';
    std::cout << real{rational<1>{"34/89"}, 20} << '\n';
    std::cout << real{rational<1>{"34/89"}, 20}.bits() << '\n';
    std::cout << real{rational<1>{"34/89"}} << '\n';
    std::cout << real{rational<1>{"34/89"}}.bits() << '\n';
    std::cout << real{rational<1>{"1/3"}} << '\n';
    std::cout << real{rational<1>{"1/3"}, 64} << '\n';
    std::cout << real{rational<1>{"1/3"}, 256} << '\n';
    std::cout << real{1233495l, 10} << '\n';
}
