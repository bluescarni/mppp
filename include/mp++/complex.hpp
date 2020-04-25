// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_COMPLEX_HPP
#define MPPP_COMPLEX_HPP

#include <mp++/config.hpp>

#if defined(MPPP_WITH_MPC)

#include <utility>

#include <mp++/detail/fwd_decl.hpp>
#include <mp++/detail/mpc.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/real.hpp>

namespace mppp
{

class MPPP_DLL_PUBLIC complex
{
public:
    complex();
    complex(const complex &);

    explicit complex(const ::mpc_t);

    ~complex();

    class real_extractor
    {
    public:
        explicit real_extractor(complex &c) : m_c(c), value(real::shallow_copy_t{}, mpc_realref(&c.m_mpc)) {}

        real_extractor(const real_extractor &) = delete;
        real_extractor(real_extractor &&) = delete;
        real_extractor &operator=(const real_extractor &) = delete;
        real_extractor &operator=(real_extractor &&) = delete;

        ~real_extractor()
        {
            mpc_realref(&m_c.m_mpc)[0] = *value.get_mpfr_t();
            value._get_mpfr_t()->_mpfr_d = nullptr;
        }

    private:
        complex &m_c;

    public:
        real value;
    };

    class imag_extractor
    {
    public:
        explicit imag_extractor(complex &c) : m_c(c), value(real::shallow_copy_t{}, mpc_imagref(&c.m_mpc)) {}

        imag_extractor(const imag_extractor &) = delete;
        imag_extractor(imag_extractor &&) = delete;
        imag_extractor &operator=(const imag_extractor &) = delete;
        imag_extractor &operator=(imag_extractor &&) = delete;

        ~imag_extractor()
        {
            mpc_imagref(&m_c.m_mpc)[0] = *value.get_mpfr_t();
            value._get_mpfr_t()->_mpfr_d = nullptr;
        }

    private:
        complex &m_c;

    public:
        real value;
    };

    ::mpfr_prec_t get_prec() const
    {
        return mpfr_get_prec(mpc_realref(&m_mpc));
    }

private:
    mpc_struct_t m_mpc;
};

} // namespace mppp

#else

#error The complex.hpp header was included but mp++ was not configured with the MPPP_WITH_MPC option.

#endif

#endif
