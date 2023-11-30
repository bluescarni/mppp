// Copyright 2016-2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_MPFR_HPP
#define MPPP_DETAIL_MPFR_HPP

#include <type_traits>

#include <mpfr.h>

#include <mp++/config.hpp>

#if MPFR_VERSION_MAJOR < 3

#error Minimum supported MPFR version is 3.

#endif

#if GMP_NAIL_BITS

// See:
// https://gmplib.org/list-archives/gmp-discuss/2012-November/005189.html

#error MPFR is incompatible with GMP if nails are enabled.

#endif

MPPP_BEGIN_NAMESPACE

// Minimum precision for a real.
constexpr ::mpfr_prec_t real_prec_min()
{
    // NOTE: Arb wants at least 2 bits, so let's
    // make sure we don't allow 1-bit numbers.
    return MPFR_PREC_MIN > 2 ? MPFR_PREC_MIN : 2;
}

// Maximum precision for a real.
constexpr ::mpfr_prec_t real_prec_max()
{
    // For the max precision, we remove 7 bits from the MPFR_PREC_MAX value (as the MPFR docs warn
    // to never set the precision "close" to the max value).
    return MPFR_PREC_MAX >> 7;
}

// The MPFR structure underlying mpfr_t.
using mpfr_struct_t = std::remove_extent<::mpfr_t>::type;

namespace detail
{

// Check if a precision value is in the allowed range.
constexpr bool real_prec_check(::mpfr_prec_t p)
{
    return p >= real_prec_min() && p <= real_prec_max();
}

// Simple RAII holder for MPFR floats.
struct mpfr_raii {
    // A constructor from a precision value, will set the value to NaN.
    // No check is performed on the precision.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit mpfr_raii(::mpfr_prec_t prec)
    {
        ::mpfr_init2(&m_mpfr, prec);
    }
    // Disable all the other ctors/assignment ops.
    mpfr_raii(const mpfr_raii &) = delete;
    mpfr_raii(mpfr_raii &&) = delete;
    mpfr_raii &operator=(const mpfr_raii &) = delete;
    mpfr_raii &operator=(mpfr_raii &&) = delete;
    ~mpfr_raii()
    {
        ::mpfr_clear(&m_mpfr);
    }
    mpfr_struct_t m_mpfr;
};

} // namespace detail

MPPP_END_NAMESPACE

#endif
