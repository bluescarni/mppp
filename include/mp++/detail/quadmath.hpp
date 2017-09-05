// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_QUADMATH_HPP
#define MPPP_DETAIL_QUADMATH_HPP

#include <cstdint>
#include <iostream>
// NOTE: extern "C" is already included in quadmath.h since GCC 4.8:
// https://stackoverflow.com/questions/13780219/link-libquadmath-with-c-on-linux
#include <quadmath.h>
#include <stdexcept>
#include <string>

#include <mp++/config.hpp>

namespace mppp
{

inline namespace detail
{
// Machinery for low-level manipulation of __float128. Inspired by:
// https://github.com/gcc-mirror/gcc/blob/master/libquadmath/quadmath-imp.h
// Note that the union machinery is technically UB, but well-defined on GCC as
// an extension:
// https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#Type%2Dpunning
// quadmath so far is limited to GCC and perhaps clang, which I'd imagine
// implements the same extension for GCC compatibility. So we should be ok.

// The ieee fields.
struct ieee_t {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    std::uint_least8_t negative : 1;
    std::uint_least16_t exponent : 15;
    std::uint_least64_t mant_high : 48;
    std::uint_least64_t mant_low : 64;
#else
    std::uint_least64_t mant_low : 64;
    std::uint_least64_t mant_high : 48;
    std::uint_least16_t exponent : 15;
    std::uint_least8_t negative : 1;
#endif
}
#ifdef __MINGW32__
// On mingw targets the ms-bitfields option is active by default.
// Therefore enforce gnu-bitfield style.
__attribute__((gcc_struct))
#endif
;

// The union.
union ieee_float128 {
    __float128 value;
    ieee_t i_eee;
};

inline void float128_stream(std::ostream &os, const __float128 &x)
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
        throw std::runtime_error("A call to quadmath_snprintf() failed: a negative exit status of " + std::to_string(n)
                                 + " was returned");
    }
    if (mppp_unlikely(unsigned(n) >= sizeof(buf))) {
        throw std::runtime_error("A call to quadmath_snprintf() failed: the exit status " + std::to_string(n)
                                 + " is not less than the size of the internal buffer " + std::to_string(sizeof(buf)));
    }
    // LCOV_EXCL_STOP
    os << &buf[0];
}

inline __float128 str_to_float128(const char *s)
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
}
}

#endif
