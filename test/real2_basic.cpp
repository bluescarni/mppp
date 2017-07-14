// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/real2.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("real constructors")
{
    real<1> r, r2{r};
    std::cout << real_prec_min() << '\n';
    std::cout << real_prec_max() << '\n';
    std::cout << r << '\n';
    std::cout << r.get_prec() << '\n';
    r.set_prec(34);
    std::cout << r << '\n';
    std::cout << r.get_prec() << '\n';
    r.set_prec(340);
    std::cout << r << '\n';
    std::cout << r.get_prec() << '\n';
    r.set_prec(323);
    std::cout << r << '\n';
    std::cout << r.get_prec() << '\n';
    r.set_prec(386);
    std::cout << r << '\n';
    std::cout << r.get_prec() << '\n';
    r.set_prec(8);
    std::cout << r << '\n';
    std::cout << r.get_prec() << '\n';
    std::cout << sizeof(r) << '\n';
    std::cout << sizeof(mpfr_struct_t) << '\n';
    std::cout << static_real<1>::max_prec() << '\n';
    std::cout << static_real<2>::max_prec() << '\n';
    std::cout << static_real<3>::max_prec() << '\n';
    std::cout << static_real<4>::max_prec() << '\n';
    std::cout << static_real<5>::max_prec() << '\n';
    std::cout << static_real<6>::max_prec() << '\n';
    constexpr auto foo = static_real<1>::max_prec();
}
