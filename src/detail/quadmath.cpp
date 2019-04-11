// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <iostream>
#include <stdexcept>
#include <string>

#include <quadmath.h>

#include <mp++/config.hpp>
#include <mp++/detail/quadmath.hpp>
#include <mp++/detail/utils.hpp>

namespace mppp
{

namespace detail
{

void float128_stream(std::ostream &os, const __float128 &x)
{
    char buf[100];
    // NOTE: 36 decimal digits ensures that reading back the string always produces the same value.
    // https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format
    // We use 35 because the precision field in printf()-like functions refers to the number
    // of digits to the right of the decimal point, and we have one digit to the left of
    // the decimal point due to the scientific notation.
    const auto n = ::quadmath_snprintf(buf, sizeof(buf), "%.35Qe", x);
    // LCOV_EXCL_START
    if (mppp_unlikely(n < 0)) {
        throw std::runtime_error("A call to quadmath_snprintf() failed: a negative exit status of " + to_string(n)
                                 + " was returned");
    }
    if (mppp_unlikely(unsigned(n) >= sizeof(buf))) {
        throw std::runtime_error("A call to quadmath_snprintf() failed: the exit status " + to_string(n)
                                 + " is not less than the size of the internal buffer " + to_string(sizeof(buf)));
    }
    // LCOV_EXCL_STOP
    os << &buf[0];
}

__float128 str_to_float128(const char *s)
{
    char *endptr;
    auto retval = ::strtoflt128(s, &endptr);
    if (mppp_unlikely(endptr == s || *endptr != '\0')) {
        // NOTE: the first condition handles an empty string.
        // endptr will point to the first character in the string which
        // did not contribute to the construction of retval.
        throw std::invalid_argument("The string '" + std::string(s)
                                    + "' does not represent a valid quadruple-precision floating-point value");
    }
    return retval;
}

} // namespace detail

} // namespace mppp
