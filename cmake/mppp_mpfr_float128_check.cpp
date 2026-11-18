// Copyright 2016-2026 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Configure-time check for the binary128 conversion functions of MPFR
// (see the MPFR binary128 section in CMakeLists.txt).

#include <mpfr.h>

int main()
{
    mpfr_t x;
    mpfr_init2(x, 113);
    const int ret = mpfr_set_float128(x, __float128(1), MPFR_RNDN);
    const __float128 y = mpfr_get_float128(x, MPFR_RNDN);
    mpfr_clear(x);
    return ret + static_cast<int>(y);
}
