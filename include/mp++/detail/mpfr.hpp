// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_MPFR_HPP
#define MPPP_DETAIL_MPFR_HPP

#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <mpfr.h>
#include <type_traits>

#include <mp++/config.hpp>
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

/** @defgroup real_prec real_prec
 *  @{
 */

/// Minimum precision for a \link mppp::real real\endlink.
/**
 * \rststar
 * This compile-time constant represents the minimum valid precision
 * for a :cpp:class:`~mppp::real`. The returned value if guaranteed to be
 * not less than the ``MPFR_PREC_MIN`` MPFR constant.
 * \endrststar
 *
 * @return the minimum valid precision for a \link mppp::real real\endlink.
 */
constexpr ::mpfr_prec_t real_prec_min()
{
    return MPFR_PREC_MIN;
}

/// Maximum precision for a \link mppp::real real\endlink.
/**
 * \rststar
 * This compile-time constant represents the maximum valid precision
 * for a :cpp:class:`~mppp::real`. The returned value if guaranteed to be
 * not greater than the ``MPFR_PREC_MAX`` MPFR constant.
 * \endrststar
 *
 * @return the maximum valid precision for a \link mppp::real real\endlink.
 */
constexpr ::mpfr_prec_t real_prec_max()
{
    // For the max precision, we remove 7 bits from the MPFR_PREC_MAX value (as the MPFR docs warn
    // to never set the precision "close" to the max value).
    return MPFR_PREC_MAX >> 7;
}

/** @} */

/// The MPFR structure underlying <tt>mpfr_t</tt>.
/**
 * The MPFR \p mpfr_t type is an array of size 1 of an unspecified structure,
 * which is here aliased as <tt>mpfr_struct_t</tt>.
 */
typedef std::remove_extent<::mpfr_t>::type mpfr_struct_t;

inline namespace detail
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

// Smart pointer to handle the string output from mpfr.
using smart_mpfr_str = std::unique_ptr<char, void (*)(char *)>;

// A couple of sanity checks when constructing temporary mpfrs/mpfs from long double.
static_assert(std::numeric_limits<long double>::max_digits10 < nl_max<int>() / 4, "Overflow error.");
static_assert(std::numeric_limits<long double>::max_digits10 * 4 < nl_max<::mpfr_prec_t>(), "Overflow error.");
static_assert(std::numeric_limits<long double>::max_digits10 * 4 < nl_max<::mp_bitcnt_t>(), "Overflow error.");
static_assert(real_prec_check(static_cast<::mpfr_prec_t>(std::numeric_limits<long double>::max_digits10 * 4)),
              "The precision required to represent long double is outside the MPFR min/max precision bounds.");

// Machinery to call automatically mpfr_free_cache() at program shutdown,
// if this header is included.

extern "C" {

// NOTE: the cleanup function should have C linkage, as it will be passed to atexit()
// which is a function from the C standard library.
void mpfr_cleanup_function();
}

// The actual implementation.
inline void mpfr_cleanup_function()
{
#if !defined(NDEBUG)
    // NOTE: functions registered with atexit() may be called concurrently.
    // Access to cout from concurrent threads is safe as long as the
    // cout object is synchronized to the underlying C stream:
    // https://stackoverflow.com/questions/6374264/is-cout-synchronized-thread-safe
    // http://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio
    // By default, this is the case, but in theory someone might have changed
    // the sync setting on cout by the time we execute the following line.
    // However, we print only in debug mode, so it should not be too much of a problem
    // in practice.
    std::cout << "Cleaning up MPFR caches." << std::endl;
#endif
    ::mpfr_free_cache();
}

struct mpfr_cleanup {
    mpfr_cleanup()
    {
        std::atexit(mpfr_cleanup_function);
    }
};

#if MPPP_CPLUSPLUS < 201703L

// Inline variable emulation machinery for pre-C++17.
template <typename = void>
struct mpfr_cleanup_holder {
    static const mpfr_cleanup s_cleanup;
};

template <typename T>
const mpfr_cleanup mpfr_cleanup_holder<T>::s_cleanup;

inline void inst_mpfr_cleanup()
{
    auto ptr = &mpfr_cleanup_holder<>::s_cleanup;
    (void)ptr;
}

#else

inline const mpfr_cleanup mpfr_cleanup_register;

#endif
}
}

#endif
