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

#include <mp++/detail/arb.hpp>
#include <mp++/detail/fwd_decl.hpp>
#include <mp++/detail/mpfr.hpp>

namespace mppp
{

class real
{
public:
    using arb_slong = arb_slong_impl;

private:
    // Make sure that arb_slong can represent the full precision range of MPFR.
    static_assert(MPFR_PREC_MAX <= std::numeric_limits<arb_slong>::max(), "Overflow error.");
    // Minimum precision: it's 2 for ARB, MPFR_PREC_MIN for MPFR. Pick the highest value.
    static constexpr arb_slong min_prec()
    {
        return static_cast<arb_slong>((MPFR_PREC_MIN <= 2) ? 2 : (MPFR_PREC_MIN));
    }
    // Maximum precision: it's MPFR_PREC_MAX for MPFR, while for ARB we pick arbitrarily
    // 2**20, based on the ARB documentation. Select the smallest value.
    // http://arblib.org/issues.html#integer-overflow.
    static constexpr arb_slong max_prec()
    {
        return static_cast<arb_slong>(((2l << 20) <= MPFR_PREC_MAX) ? (2l << 20) : (MPFR_PREC_MAX));
    }

public:
    real() : m_prec(min_prec())
    {
        ::arf_init(&m_arf);
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
    arb_slong get_prec() const
    {
        return m_prec;
    }
    void set_prec(arb_slong prec)
    {
        if (mppp_unlikely(prec > max_prec() || prec < min_prec())) {
            // TODO throw.
            throw std::invalid_argument("");
        }
        m_prec = prec;
    }

private:
    ::arf_struct m_arf;
    arb_slong m_prec;
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

    if (::arf_is_zero(r.get_arf_t())) {
        os << "0.";
        return os;
    }

    // Number of bits needed to represent the mantissa of r.
    // We will use this value to init an mpfr real with enough bits
    // to represent exactly r.
    const auto ab = ::arf_bits(r.get_arf_t());
    // Sanity check wrt the maximum allowed prec in MPFR.
    if (mppp_unlikely(ab > MPFR_PREC_MAX)) {
        // TODO error message.
        throw;
    }

    // Setup a suitable mpfr real. The precision of 53 is just a random value for
    // init, we will set the actual precision in the following line.
    MPPP_MAYBE_TLS mpfr_raii mpfr(53);
    // NOTE: make sure ab is not too small either.
    ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(ab >= MPFR_PREC_MIN ? ab : MPFR_PREC_MIN));
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
    if (exp) {
        // Add the exponent at the end of the string, if nonzero.
        os << "e" << exp;
    }
    return os;
}
}

#else

#error The real.hpp header was included but mp++ was not configured with the MPPP_WITH_ARB option.

#endif

#endif
