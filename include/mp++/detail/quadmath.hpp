// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_QUADMATH_HPP
#define MPPP_DETAIL_QUADMATH_HPP

#include <iostream>
// NOTE: extern "C" is already included in quadmath.h since GCC 4.8:
// https://stackoverflow.com/questions/13780219/link-libquadmath-with-c-on-linux
#include <quadmath.h>

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
struct ieee_t
#ifdef __MINGW32__
    // On mingw targets the ms-bitfields option is active by default.
    // Therefore enforce gnu-bitfield style.
    __attribute__((gcc_struct))
#endif
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    unsigned negative : 1;
    unsigned exponent : 15;
    // ull provides at least 64 bits of storage, so it is ok
    // to use it here.
    unsigned long long mant_high : 48;
    unsigned long long mant_low : 64;
#else
    unsigned long long mant_low : 64;
    unsigned long long mant_high : 48;
    unsigned exponent : 15;
    unsigned negative : 1;
#endif
};

// The union.
union ieee_float128 {
    ::__float128 value;
    ieee_t i_eee;
};

inline void float128_stream(std::ostream &os, const ::__float128 &x)
{
    char buf[100];
    // NOTE: 36 decimal digits ensures that reading back the string always produces the same value.
    // https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format
    const auto n = ::quadmath_snprintf(buf, sizeof(buf), "%.36Qe", x);
    if (mppp_unlikely(n < 0)) {
        // TODO.
        throw;
    }
    if (mppp_unlikely(unsigned(n) >= sizeof(buf))) {
        // TODO.
        throw;
    }
    os << &buf[0];
}

inline ::__float128 str_to_float128(const char *s)
{
    char *endptr;
    auto retval = ::strtoflt128(s, &endptr);
    if (mppp_unlikely(endptr == s || *endptr != '\0')) {
        // NOTE: the first condition handles an empty string.
        // endptr will point to the first character in the string which
        // did not contribute to the construction of retval.
        // TODO.
        throw;
    }
    return retval;
}
}
}

#endif
