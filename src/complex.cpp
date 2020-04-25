// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>

#include <mp++/complex.hpp>
#include <mp++/detail/mpc.hpp>
#include <mp++/detail/mpfr.hpp>

namespace mppp
{

complex::complex()
{
    ::mpc_init2(&m_mpc, real_prec_min());
    ::mpfr_set_zero(mpc_realref(&m_mpc), 1);
    ::mpfr_set_zero(mpc_imagref(&m_mpc), 1);
}

complex::complex(const complex &other) : complex(&other.m_mpc) {}

complex::~complex()
{
    if (mpc_realref(&m_mpc)->_mpfr_d) {
        // The object is not moved-from, destroy it.

        // Check that the imaginary part is also
        // valid.
        assert(mpc_imagref(&m_mpc)->_mpfr_d);
        // Check that the precision of the imaginary
        // part is equal to the precision of the real
        // part.
        assert(get_prec() == mpfr_get_prec(mpc_imagref(&m_mpc)));
        // Check that the precision value is valid.
        assert(detail::real_prec_check(get_prec()));

        ::mpc_clear(&m_mpc);
    }
}

complex::complex(const ::mpc_t c)
{
    // Init with the same precision as other, and then set.
    ::mpc_init2(&m_mpc, mpfr_get_prec(mpc_realref(c)));
    ::mpc_set(&m_mpc, c, MPC_RNDNN);
}

} // namespace mppp
