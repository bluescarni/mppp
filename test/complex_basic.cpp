// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/complex.hpp>

#include "catch.hpp"

using namespace mppp;

TEST_CASE("complex constructors")
{
    complex c;
    {
        complex::re_extractor rex{c};
        complex::im_extractor iex{c};

        rex.get() += 45;
        iex.get() += 42;
    }
    {
        complex::const_re_extractor ex{c};

        std::cout << ex.get() << '\n';
    }
}
