// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_REAL_HPP
#define MPPP_REAL_HPP

#include <mp++/config.hpp>

#if defined(MPPP_WITH_ARB)

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <mp++/detail/arb.hpp>
#include <mp++/detail/fwd_decl.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>

namespace mppp
{

inline namespace detail
{
// constexpr max/min implementations.
template <typename T>
constexpr T c_max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
constexpr T c_min(T a, T b)
{
    return a < b ? a : b;
}

using mpfr_min_prec_t = std::make_unsigned<uncvref_t<decltype(MPFR_PREC_MIN)>>::type;

// Minimum precision allowed for real values. It's the max value between the minimum precs of mpfr and arb,
// and guaranteed to be representable by slong (otherwise, a static assert will fire).
constexpr mpfr_min_prec_t real_min_prec_impl()
{
    // Min prec for arb is 2.
    return c_max(static_cast<mpfr_min_prec_t>(MPFR_PREC_MIN), static_cast<mpfr_min_prec_t>(2));
}

static_assert(real_min_prec_impl() <= static_cast<std::make_unsigned<slong>::type>(std::numeric_limits<slong>::max()),
              "The minimum precision for real cannot be represented by slong.");

// Maximum precision allowed for real values. For MPFR there's a macro, for arb the documentation suggests
// < 2**24 for 32-bit and < 2**36 for 64-bit.
// http://arblib.org/issues.html#integer-overflow
constexpr unsigned long long arb_max_prec()
{
    // We use slightly smaller max prec values for arb.
    // NOTE: the docs of ulong state that it has exactly either 64 or 32 bit width.
    return std::numeric_limits<ulong>::digits == 64 ? (1ull << 32) : (1ull << 20);
}

constexpr unsigned long long real_max_prec_impl()
{
    return c_min(arb_max_prec(), static_cast<unsigned long long>(MPFR_PREC_MAX));
}

static_assert(real_max_prec_impl() <= static_cast<std::make_unsigned<slong>::type>(std::numeric_limits<slong>::max()),
              "The maximum precision for real cannot be represented by slong.");

constexpr slong real_min_prec()
{
    return static_cast<slong>(real_min_prec_impl());
}

constexpr slong real_max_prec()
{
    return static_cast<slong>(real_max_prec_impl());
}

static_assert(real_min_prec() <= real_max_prec(),
              "The minimum precision for real is larger than the maximum precision.");
}

class real
{
public:
    real()
    {
        ::arf_init(&m_arf);
        m_prec = real_min_prec();
    }
    ~real()
    {
        ::arf_clear(&m_arf);
    }
    const ::arf_struct *get_arf_t() const
    {
        return &m_arf;
    }
    ::arf_struct *get_arf_t()
    {
        return &m_arf;
    }
    slong get_prec() const
    {
        return m_prec;
    }
    void set_prec(slong prec)
    {
        if (mppp_unlikely(prec > real_max_prec() || prec < real_min_prec())) {
            throw std::invalid_argument("An invalid precision of " + std::to_string(prec)
                                        + " was specified for a real object (the minimum allowed precision is "
                                        + std::to_string(real_min_prec()) + ", while the maximum allowed precision is "
                                        + std::to_string(real_max_prec()) + ")");
        }
        m_prec = prec;
    }

private:
    ::arf_struct m_arf;
    slong m_prec;
};

inline std::ostream &operator<<(std::ostream &os, const real &r)
{
    // Handle special values first.
    if (::arf_is_nan(r.get_arf_t())) {
        os << "nan";
        return os;
    }

    if (::arf_is_pos_inf(r.get_arf_t())) {
        os << "inf";
        return os;
    }

    if (::arf_is_neg_inf(r.get_arf_t())) {
        os << "-inf";
        return os;
    }

    // Setup a suitable mpfr real. We will set the actual precision in the following line.
    MPPP_MAYBE_TLS mpfr_raii mpfr(static_cast<::mpfr_prec_t>(real_min_prec()));
    // NOTE: the precision of r is always guaranteed to be a valid precision for both MPFR and arb.
    ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(r.get_prec()));
    // Extract an mpfr from the arf.
    ::arf_get_mpfr(&mpfr.m_mpfr, r.get_arf_t(), MPFR_RNDN);

    // Get the string fractional representation via the MPFR function,
    // and wrap it into a smart pointer.
    ::mpfr_exp_t exp(0);
    smart_mpfr_str str(::mpfr_get_str(nullptr, &exp, 10, 0, &mpfr.m_mpfr, MPFR_RNDN), ::mpfr_free_str);
    if (mppp_unlikely(!str)) {
        throw std::runtime_error("Error in the stream operator of real: the call to mpfr_get_str() failed");
    }

    // Print the string, inserting a decimal point after the first digit.
    bool dot_added = false;
    for (auto cptr = str.get(); *cptr != '\0'; ++cptr) {
        os << (*cptr);
        // NOTE: check this answer:
        // http://stackoverflow.com/questions/13827180/char-ascii-relation
        // """
        // The mapping of integer values for characters does have one guarantee given
        // by the Standard: the values of the decimal digits are continguous.
        // (i.e., '1' - '0' == 1, ... '9' - '0' == 9)
        // """
        if (!dot_added && *cptr >= '0' && *cptr <= '9') {
            os << '.';
            dot_added = true;
        }
    }
    assert(dot_added);

    // Adjust the exponent.
    if (mppp_unlikely(exp == std::numeric_limits<::mpfr_exp_t>::min())) {
        throw std::overflow_error("Overflow in the conversion of a real to string");
    }
    --exp;
    if (exp && !mpfr_zero_p(&mpfr.m_mpfr)) {
        // Add the exponent at the end of the string, if both the value and the exponent
        // are nonzero.
        os << "e" << exp;
    }
    return os;
}
}

#else

#error The real.hpp header was included but mp++ was not configured with the MPPP_WITH_ARB option.

#endif

#endif
