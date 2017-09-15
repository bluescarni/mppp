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

TEST_CASE("real basic")
{
    std::cout << real{1ll} << '\n';
    std::cout << real{1.l} << '\n';
    real r{12356732};
    std::cout << r.prec_round(120) << '\n';
    std::cout << r.prec_round(12) << '\n';
    std::cout << real{true} << '\n';
    std::cout << real{integer<1>{1}} << '\n';
    std::cout << static_cast<integer<1>>(real{integer<1>{1}}) << '\n';
    std::cout << real{rational<1>{1, 3}} << '\n';
#if defined(MPPP_WITH_QUADMATH)
    std::cout << real{-real128{"1.3E200"}} << '\n';
    std::cout << -real128{"1.3E200"} << '\n';
    std::cout << real{-real128{"1.3E-200"}} << '\n';
    std::cout << -real128{"1.3E-200"} << '\n';
    std::cout << real128{"1E-4940"} << '\n';
    std::cout << real{real128{"1E-4940"}} << '\n';
#endif
}
