// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>
#include <mp++/detail/demangle.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <string>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("demangle")
{
    std::cout << demangle<std::string>() << '\n';
    std::cout << demangle<int>() << '\n';
    std::cout << demangle<long double>() << '\n';
    std::cout << demangle<std::vector<std::vector<float>>>() << '\n';
    std::cout << demangle<integer<1>>() << '\n';
    std::cout << demangle<rational<2>>() << '\n';
#if defined(MPPP_HAVE_GCC_INT128) && !defined(__apple_build_version__)
    std::cout << demangle<__int128_t>() << '\n';
    std::cout << demangle<__uint128_t>() << '\n';
#endif
}
