// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_STATIC_REAL_HPP
#define MPPP_STATIC_REAL_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <ostream>

#include <mp++/config.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/real_common.hpp>

namespace mppp
{

template <mpfr_prec_t Prec>
class static_real
{
    static_assert(Prec >= real_prec_min() && Prec <= real_prec_max(),
                  "The precision selected for a static_real is outside the valid range.");

    // Significand's size in bytes.
    static constexpr std::size_t sig_size_bytes = mpfr_custom_get_size(Prec);
    // Significand size in number of mp_limb_t.
    static constexpr std::size_t sig_size_limbs
        = sig_size_bytes / sizeof(::mp_limb_t) + static_cast<std::size_t>(sig_size_bytes % sizeof(::mp_limb_t) != 0u);
    static_assert(sig_size_limbs > 0u, "Invalid size for a static real.");

    // The MPFR struct.
    mpfr_struct_t m_mpfr;
    // The significand.
    ::mp_limb_t m_sig[sig_size_limbs];

public:
    static constexpr mpfr_prec_t prec = Prec;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    static_real()
    {
        // Init the significand and set the m_mpfr up.
        mpfr_custom_init(m_sig, Prec);
        mpfr_custom_init_set(&m_mpfr, MPFR_ZERO_KIND, 0, Prec, m_sig);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    static_real(const static_real &o)
    {
        assert(o.m_mpfr._mpfr_prec == prec);

        // Init the significand.
        mpfr_custom_init(m_sig, Prec);

        // Fetch kind, exponent and pointer to the significand.
        const auto o_kind = mpfr_custom_get_kind(&o.m_mpfr);
        const auto o_exp = mpfr_custom_get_exp(&o.m_mpfr);
        const auto *const o_sig = static_cast<const ::mp_limb_t *>(mpfr_custom_get_significand(&o.m_mpfr));
        assert(o_sig == o.m_sig);

        // Copy over the significand.
        std::copy(o_sig, o_sig + sig_size_limbs, m_sig);

        // Set the m_mpfr up.
        mpfr_custom_init_set(&m_mpfr, o_kind, o_exp, Prec, m_sig);
    }
    // NOTE: move equivalent to copy.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    static_real(static_real &&o) noexcept : static_real(o) {}
    static_real &operator=(const static_real &o)
    {
        assert(m_mpfr._mpfr_prec == o.m_mpfr._mpfr_prec);
        assert(m_mpfr._mpfr_prec == prec);

        if (mppp_likely(this != &o)) {
            // NOTE: here we take advantage of the internals
            // of mpfr_struct_t.
            // NOTE: no need to copy over the precision or to alter
            // the pointer to the significand.
            m_mpfr._mpfr_sign = o.m_mpfr._mpfr_sign;
            m_mpfr._mpfr_exp = o.m_mpfr._mpfr_exp;
            const auto *const o_sig = static_cast<const ::mp_limb_t *>(mpfr_custom_get_significand(&o.m_mpfr));
            assert(o_sig == o.m_sig);

            // Copy over the significand.
            std::copy(o_sig, o_sig + sig_size_limbs, m_sig);
        }

        return *this;
    }
    // NOTE: move equivalent to copy.
    static_real &operator=(static_real &&o) noexcept
    {
        // NOLINTNEXTLINE(misc-unconventional-assign-operator, cppcoreguidelines-c-copy-assignment-signature)
        return *this = o;
    }
    ~static_real() = default;

    // Const reference to the internal mpfr_t.
    MPPP_NODISCARD const mpfr_struct_t *get_mpfr_t() const
    {
        return &m_mpfr;
    }
    // Mutable reference to the internal mpfr_t.
    mpfr_struct_t *_get_mpfr_t()
    {
        return &m_mpfr;
    }
};

#if MPPP_CPLUSPLUS < 201703L

// NOTE: from C++17 static constexpr members are implicitly inline, and it's not necessary
// any more (actually, it's deprecated) to re-declare them outside the class.
// https://stackoverflow.com/questions/39646958/constexpr-static-member-before-after-c17

template <mpfr_prec_t Prec>
constexpr mpfr_prec_t static_real<Prec>::prec;

#endif

template <mpfr_prec_t Prec>
inline std::ostream &operator<<(std::ostream &os, const static_real<Prec> &s)
{
    return detail::mpfr_t_to_stream(os, s.get_mpfr_t());
}

} // namespace mppp

#endif
