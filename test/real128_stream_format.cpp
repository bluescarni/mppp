// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/real128.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>

using namespace mppp;

TEST_CASE("real128 stream format")
{
    std::cout << std::setfill('a') << std::showpos << std::uppercase << std::setw(20);
    std::cout << real128{"1.1"} << '\n';
}
