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
    real2<1> r, r2{r};
}
