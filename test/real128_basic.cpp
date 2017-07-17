// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/real128.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("real128 constructors")
{
    __float128 x = str_to_float128("1.1");
    float128_stream(std::cout, x);
    constexpr real128 r1{1.l}, r2{2}, r3{__float128{1}};
    real128 blap{integer<1>{
        "1231238012938120938201938029138901283029138012983091283091283091283091201310012301301029381318240752987"}};
    float128_stream(std::cout, blap.value());
    std::cout << "\n";
    real128 blap2{integer<1>{1} << 120};
    float128_stream(std::cout, blap2.value());
    std::cout << "\n";
    real128 blap3{integer<1>{1} << 50};
    float128_stream(std::cout, blap3.value());
    std::cout << "\n";
    real128 blap4{integer<1>{1} << 500};
    float128_stream(std::cout, blap4.value());
    std::cout << "\n";
}
