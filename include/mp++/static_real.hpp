// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_STATIC_REAL_HPP
#define MPPP_STATIC_REAL_HPP

#include <limits>
#include <ostream>

#include <mp++/concepts.hpp>
#include <mp++/config.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/real_common.hpp>

namespace mppp
{

namespace detail
{

// Compile-time function to determine if a static_real with precision
// Prec is higher in the numerical hierarchy than the type T.
template <typename T, mpfr_prec_t Prec>
constexpr bool sr_higher_than()
{
    if constexpr (is_cpp_floating_point<T>::value) {
        static_assert(std::numeric_limits<T>::radix == 2,
                      "C++ floating-point types in bases other than 2 are not supported.");

        return Prec > std::numeric_limits<T>::digits;
    } else if constexpr (is_cpp_integral<T>::value) {
        return true;
    }
}

} // namespace detail

template <mpfr_prec_t Prec>
class static_real
{
    static_assert(Prec >= real_prec_min() && Prec <= real_prec_max(),
                  "The precision selected for a static_real is outside the valid range.");

    mpfr_struct_t m_mpfr;
    alignas(mp_limb_t) unsigned char m_storage[mpfr_custom_get_size(Prec)];

public:
    static constexpr mpfr_prec_t prec = Prec;

    static_real()
    {
        mpfr_custom_init(m_storage, Prec);
        mpfr_custom_init_set(&m_mpfr, MPFR_ZERO_KIND, 0, Prec, m_storage);
    }
    static_real(const static_real &o) : static_real()
    {
        // NOTE: it might be possible to pass the value of o directly to
        // mpfr_custom_init_set, but the logic looks a bit complicated.
        // It might also be possible to do a straight bit copy of o
        // and then adjust the pointer in m_mpfr, but let's keep it simple
        // for now.
        mpfr_set(&m_mpfr, &o.m_mpfr, MPFR_RNDN);
    }
    // NOTE: move equivalent to copy.
    static_real(static_real &&o) noexcept : static_real(o) {}
    static_real &operator=(const static_real &o)
    {
        mpfr_set(&m_mpfr, &o.m_mpfr, MPFR_RNDN);
        return *this;
    }
    // NOTE: move equivalent to copy.
    static_real &operator=(static_real &&o) noexcept
    {
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
