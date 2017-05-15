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
    real()
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
        // See http://arblib.org/issues.html#integer-overflow.
        // Make sure we are within MPFR's bounds as well.
        constexpr arb_slong min_prec = (2 >= MPFR_PREC_MIN) ? 2 : static_cast<arb_slong>(MPFR_PREC_MIN);
        // Let's put 2**20 as safe max precision.
        constexpr arb_slong max_prec
            = ((2l << 20) < MPFR_PREC_MAX) ? (2l << 20) : static_cast<arb_slong>(MPFR_PREC_MAX);
        if (mppp_unlikely(prec > max_prec || prec < min_prec)) {
            // TODO throw.
            throw std::invalid_argument("");
        }
        m_prec = prec;
    }

private:
    ::arf_struct m_arf;
    arb_slong m_prec = 113;
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

    // Setup a suitable mpfr real.
    MPPP_MAYBE_TLS mpfr_raii mpfr;
    // NOTE: make sure ab is not too small either.
    ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(ab >= MPFR_PREC_MIN ? ab : MPFR_PREC_MIN));
    // Extract an mpfr from the arf.
    ::arf_get_mpfr(&mpfr.m_mpfr, r.get_arf_t(), MPFR_RNDN);

    // Get the string fractional representation via the MPFR function,
    // and wrap it into a smart pointer.
    ::mpfr_exp_t exp(0);
    char *cptr = ::mpfr_get_str(nullptr, &exp, 10, 0, &mpfr.m_mpfr, MPFR_RNDN);
    if (mppp_unlikely(!cptr)) {
        throw std::runtime_error("Error in the stream operator of real: the call to mpfr_get_str() failed");
    }
    smart_mpfr_str str(cptr, ::mpfr_free_str);

    // Copy into C++ string.
    MPPP_MAYBE_TLS std::string cpp_str;
    cpp_str.assign(str.get());

    // Insert the decimal point after the first digit.
    auto it = std::find_if(cpp_str.begin(), cpp_str.end(), [](char c) {
        // NOTE: check this answer:
        // http://stackoverflow.com/questions/13827180/char-ascii-relation
        // """
        // The mapping of integer values for characters does have one guarantee given
        // by the Standard: the values of the decimal digits are continguous.
        // (i.e., '1' - '0' == 1, ... '9' - '0' == 9)
        // """
        return c >= '0' && c <= '9';
    });
    // I can't see how cpp_str could not contain at least 1 digit. We already handled
    // the zero and nonfinite cases above.
    assert(it != cpp_str.end());
    cpp_str.insert(it + 1, '.');

    // Adjust the exponent.
    if (mppp_unlikely(exp == std::numeric_limits<::mpfr_exp_t>::min())) {
        throw std::overflow_error("Overflow in the conversion of a real to string");
    }
    --exp;

    // Add the exponent at the end of the string, if nonzero.
    if (exp) {
        cpp_str.append("e" + std::to_string(exp));
    }
    return os << cpp_str;
}
}

#else

#error The real.hpp header was included but mp++ was not configured with the MPPP_WITH_ARB option.

#endif

#endif
