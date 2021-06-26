// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <utility>

#include <boost/multiprecision/mpfr.hpp>

#include <mp++/real.hpp>

#include "track_malloc.hpp"

using real = mppp::real;
using mpfr_float = boost::multiprecision::mpfr_float;

// An example taken from here:
// https://www.boost.org/doc/libs/1_72_0/libs/multiprecision/doc/html/boost_multiprecision/intro.html#boost_multiprecision.intro.expression_templates
template <typename T>
T test_function(const T &x, bool move = false)
{
    T a[7] = {T{1.}, T{2.}, T{3.}, T{4.}, T{5.}, T{6.}, T{7.}};

    if (move) {
        return (((((std::move(a[6]) * x + a[5]) * x + a[4]) * x + a[3]) * x + a[2]) * x + a[1]) * x + a[0];
    } else {
        return (((((a[6] * x + a[5]) * x + a[4]) * x + a[3]) * x + a[2]) * x + a[1]) * x + a[0];
    }
}

int main()
{
    real arg1{42.};
    mpfr_float arg2{42.};

    {
        mppp_bench::malloc_tracker t{"bmp::mpfr_float"};
        test_function(arg2);
    }
    {
        mppp_bench::malloc_tracker t{"mppp::real"};
        test_function(arg1);
    }
    {
        mppp_bench::malloc_tracker t{"bmp::mpfr_float + move"};
        test_function(arg2, true);
    }
    {
        mppp_bench::malloc_tracker t{"mppp::real + move"};
        test_function(arg1, true);
    }

    return 0;
}
