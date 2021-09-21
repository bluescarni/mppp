// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// #include <mp++/config.hpp>

// #include <atomic>
// #include <cmath>
// #include <complex>
// #include <initializer_list>
// #include <iomanip>
// #include <limits>
// #include <random>
// #include <sstream>
// #include <stdexcept>
// #include <string>
// #include <thread>
// #include <tuple>
// #include <type_traits>
// #include <utility>
// #include <vector>

// #if defined(MPPP_HAVE_STRING_VIEW)
// #include <string_view>
// #endif

// #include <mp++/detail/gmp.hpp>
// #include <mp++/detail/mpfr.hpp>
// #include <mp++/detail/type_traits.hpp>
// #include <mp++/integer.hpp>
// #include <mp++/rational.hpp>
// #include <mp++/real.hpp>
#include <mp++/static_real.hpp>
// #include <mp++/type_name.hpp>

#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif

#include "catch.hpp"
// #include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
// using namespace mppp_test;

TEST_CASE("static_real constructors")
{
    static_real<128> r, r2(r);
}
