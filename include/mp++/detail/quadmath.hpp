// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
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

#include <mp++/detail/visibility.hpp>

namespace mppp
{

namespace detail
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
#if defined(__MINGW32__)
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

MPPP_DLL_PUBLIC void float128_stream(std::ostream &, const __float128 &);

MPPP_DLL_PUBLIC __float128 str_to_float128(const char *);

} // namespace detail
} // namespace mppp

#endif
