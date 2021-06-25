// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_MPFR_HPP
#define MPPP_DETAIL_MPFR_HPP

#include <limits>
#include <type_traits>

#include <mpfr.h>

#include <mp++/detail/gmp.hpp>
#include <mp++/detail/type_traits.hpp>

#if MPFR_VERSION_MAJOR < 3

#error Minimum supported MPFR version is 3.

#endif

#if GMP_NAIL_BITS

// See:
// https://gmplib.org/list-archives/gmp-discuss/2012-November/005189.html

#error MPFR is incompatible with GMP if nails are enabled.

#endif

namespace mppp
{

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

// Paranoia checks.
static_assert(real_prec_min() <= real_prec_max(), "The minimum real precision is larger than the maximum precision.");

// Zero precision has a special meaning, depending on the context. Thus, the minimum precision must be nonzero.
static_assert(real_prec_min() > 0, "The minimum real precision must be positive.");

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

// A couple of sanity checks when constructing temporary mpfrs/mpfs from long double.
static_assert(std::numeric_limits<long double>::max_digits10 < nl_max<int>() / 4, "Overflow error.");
static_assert(std::numeric_limits<long double>::max_digits10 * 4 < nl_max<::mpfr_prec_t>(), "Overflow error.");
static_assert(std::numeric_limits<long double>::max_digits10 * 4 < nl_max<::mp_bitcnt_t>(), "Overflow error.");
// NOLINTNEXTLINE(bugprone-misplaced-widening-cast)
static_assert(real_prec_check(static_cast<::mpfr_prec_t>(std::numeric_limits<long double>::max_digits10 * 4)),
              "The precision required to represent long double is outside the MPFR min/max precision bounds.");

} // namespace detail
} // namespace mppp

#endif
