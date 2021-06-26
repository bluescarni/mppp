// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/complex128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("literal")
{
    REQUIRE(1.1_icq == complex128{0, real128{"1.1"}});
    REQUIRE(1.1_rq + 1.1_icq == complex128{real128{"1.1"}, real128{"1.1"}});
}
