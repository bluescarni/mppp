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
#include <cstddef>
#include <limits>
#include <ostream>

#include <mp++/concepts.hpp>
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
    static constexpr std::size_t sig_size = mpfr_custom_get_size(Prec);

    mpfr_struct_t m_mpfr;
    alignas(mp_limb_t) unsigned char m_storage[sig_size];

public:
    static constexpr mpfr_prec_t prec = Prec;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    static_real()
    {
        mpfr_custom_init(m_storage, Prec);
        mpfr_custom_init_set(&m_mpfr, MPFR_ZERO_KIND, 0, Prec, m_storage);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    static_real(const static_real &o)
    {
        mpfr_custom_init(m_storage, Prec);

        const auto o_kind = mpfr_custom_get_kind(&o.m_mpfr);
        const auto o_exp = mpfr_custom_get_exp(&o.m_mpfr);
        const auto *const o_sig = static_cast<const unsigned char *>(mpfr_custom_get_significand(&o.m_mpfr));

        std::copy(o_sig, o_sig + sig_size, m_storage);

        mpfr_custom_init_set(&m_mpfr, o_kind, o_exp, Prec, m_storage);
    }
    // NOTE: move equivalent to copy.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    static_real(static_real &&o) noexcept : static_real(o) {}
    static_real &operator=(const static_real &o)
    {
        if (mppp_likely(this != &o)) {
            mpfr_set(&m_mpfr, &o.m_mpfr, MPFR_RNDN);
        }

        return *this;
    }
    ~static_real() = default;

    // NOTE: move equivalent to copy.
    static_real &operator=(static_real &&o) noexcept
    {
        // NOLINTNEXTLINE(misc-unconventional-assign-operator)
        return *this = o;
    }

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

template <mpfr_prec_t Prec>
inline std::ostream &operator<<(std::ostream &os, const static_real<Prec> &s)
{
    return detail::mpfr_t_to_stream(os, s.get_mpfr_t());
}

} // namespace mppp

#endif
