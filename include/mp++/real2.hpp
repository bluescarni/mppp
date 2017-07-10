// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_REAL2_HPP
#define MPPP_REAL2_HPP

#include <mp++/config.hpp>

#if defined(MPPP_WITH_MPFR)

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <mp++/detail/mpfr.hpp>

namespace mppp
{

inline namespace detail
{

static_assert(MPFR_PREC_MIN > 0, "The minimum MPFR precision must be strictly positive.");

// TODO further precision checks:
// - min/max values must be able to be negated safely (use safe abs sint? and redefine the max if needed)
//   basically, do something similar to the other real class. Keep also in mind the mpfr warnings
//   about too large prec, probably we want to be conservative here.
// - for a given SSize, the static storage must be enough to fit a min prec real (do a static
//   assert in conjunction with the mpfr_custom_get_size() macro)
// TODO mpfr struct checks (see integer)

template <std::size_t SSize>
struct static_real {
    // Let's put a hard cap and sanity check on the static size.
    static_assert(SSize > 0u && SSize <= 64u, "Invalid static size for real2.");
    mpfr_struct_t get_mpfr()
    {
        // NOTE: here we will assume the precision is set to a value which can
        // always be negated safely.
        return mpfr_struct_t{-_mpfr_prec, _mpfr_sign, _mpfr_exp, m_limbs.data()};
    }
    ::mpfr_prec_t _mpfr_prec;
    ::mpfr_sign_t _mpfr_sign;
    ::mpfr_exp_t _mpfr_exp;
    std::array<::mp_limb_t, SSize> m_limbs;
};

template <std::size_t SSize>
union real_union {
    using s_storage = static_real<SSize>;
    using d_storage = mpfr_struct_t;
    real_union()
    {
        // Activate the static member.
        ::new (static_cast<void *>(&m_st)) s_storage;

        // Static member is active. Init it with a minimum precision zero.
        // Start by zero filling the limbs array. The reason we do this
        // is that we are not sure the mpfr api will init the whole array,
        // so we could be left with uninited values in the array, thus
        // ending up reading uninited values during copy/move. It's not
        // clear 100% if this is UB, but better safe than sorry.
        std::fill(m_st.m_limbs.begin(), m_st.m_limbs.end(), ::mp_limb_t(0));

        // A temporary mpfr struct for use with the mpfr custom interface.
        mpfr_struct_t tmp;
        // Init the limbs first, as indicated by the mpfr docs.
        // TODO static assert enough storage.
        mpfr_custom_init(m_st.m_limbs.data(), MPFR_PREC_MIN);
        // Do the custom init with a zero value, exponent 0 (unused), minimum precision (must match
        // the previous mpfr_custom_init() call), and the limbs array pointer.
        mpfr_custom_init_set(&tmp, MPFR_ZERO_KIND, 0, MPFR_PREC_MIN, m_st.m_limbs.data());

        // Copy over from tmp. The precision is set to the negative of the actual precision to signal static storage.
        assert(tmp._mpfr_prec == MPFR_PREC_MIN);
        m_st._mpfr_prec = -MPFR_PREC_MIN;
        m_st._mpfr_sign = tmp._mpfr_sign;
        m_st._mpfr_exp = tmp._mpfr_exp;
    }
    real_union(const real_union &r)
    {
        if (r.is_static()) {
            // Activate and init the static member with a copy.
            ::new (static_cast<void *>(&m_st)) s_storage(r.g_st());
        } else {
            // Activate dynamic storage.
            ::new (static_cast<void *>(&m_dy)) d_storage;
            // Init with same precision as r, then set.
            ::mpfr_init2(&m_dy, mpfr_get_prec(&r.g_dy()));
            ::mpfr_set(&m_dy, &r.g_dy(), MPFR_RNDN);
            // NOTE: use the function (rather than the macro) in order
            // to avoid a warning about identical branches in recent GCC.
            assert((mpfr_get_prec)(&m_dy) > 0);
        }
    }
    ~real_union()
    {
        assert(m_st._mpfr_prec);
        if (is_static()) {
            g_st().~s_storage();
        } else {
            ::mpfr_clear(&g_dy());
            g_dy().~d_storage();
        }
    }
    // Check storage type.
    bool is_static() const
    {
        return m_st._mpfr_prec < 0;
    }
    bool is_dynamic() const
    {
        return m_st._mpfr_prec > 0;
    }
    // Getters for st and dy.
    const s_storage &g_st() const
    {
        assert(is_static());
        return m_st;
    }
    s_storage &g_st()
    {
        assert(is_static());
        return m_st;
    }
    const d_storage &g_dy() const
    {
        assert(is_dynamic());
        return m_dy;
    }
    d_storage &g_dy()
    {
        assert(is_dynamic());
        return m_dy;
    }
    // Data members.
    s_storage m_st;
    d_storage m_dy;
};
}

template <std::size_t SSize>
class real2
{
public:
    real2() = default;
    real2(const real2 &) = default;
    bool is_static() const
    {
        return m_real.is_static();
    }
    bool is_dynamic() const
    {
        return m_real.is_dynamic();
    }
    const real_union<SSize> &_get_union() const
    {
        return m_real;
    }
    real_union<SSize> &_get_union()
    {
        return m_real;
    }

private:
    real_union<SSize> m_real;
};

template <std::size_t SSize>
inline std::ostream &operator<<(std::ostream &os, const real2<SSize> &r)
{
    if (r.is_static()) {

    } else {
    }
}
}

#else

#error The real2.hpp header was included but mp++ was not configured with the MPPP_WITH_MPFR option.

#endif

#endif
