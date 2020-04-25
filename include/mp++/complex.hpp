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

    class re_extractor
    {
    public:
        explicit re_extractor(complex &c) : m_c(c), m_value(real::shallow_copy_t{}, mpc_realref(&c.m_mpc)) {}

        re_extractor(const re_extractor &) = delete;
        re_extractor(re_extractor &&) = delete;
        re_extractor &operator=(const re_extractor &) = delete;
        re_extractor &operator=(re_extractor &&) = delete;

        ~re_extractor()
        {
            mpc_realref(&m_c.m_mpc)[0] = *m_value.get_mpfr_t();
            m_value._get_mpfr_t()->_mpfr_d = nullptr;
        }

        real &get()
        {
            return m_value;
        }

    private:
        complex &m_c;
        real m_value;
    };

    class const_re_extractor
    {
    public:
        explicit const_re_extractor(const complex &c) : m_c(c), m_value(real::shallow_copy_t{}, mpc_realref(&c.m_mpc))
        {
        }

        const_re_extractor(const const_re_extractor &) = delete;
        const_re_extractor(const_re_extractor &&) = delete;
        const_re_extractor &operator=(const const_re_extractor &) = delete;
        const_re_extractor &operator=(const_re_extractor &&) = delete;

        ~const_re_extractor()
        {
            m_value._get_mpfr_t()->_mpfr_d = nullptr;
        }

        const real &get() const
        {
            return m_value;
        }

    private:
        const complex &m_c;
        real m_value;
    };

    class im_extractor
    {
    public:
        explicit im_extractor(complex &c) : m_c(c), m_value(real::shallow_copy_t{}, mpc_imagref(&c.m_mpc)) {}

        im_extractor(const im_extractor &) = delete;
        im_extractor(im_extractor &&) = delete;
        im_extractor &operator=(const im_extractor &) = delete;
        im_extractor &operator=(im_extractor &&) = delete;

        ~im_extractor()
        {
            mpc_imagref(&m_c.m_mpc)[0] = *m_value.get_mpfr_t();
            m_value._get_mpfr_t()->_mpfr_d = nullptr;
        }

        real &get()
        {
            return m_value;
        }

    private:
        complex &m_c;
        real m_value;
    };

    class const_im_extractor
    {
    public:
        explicit const_im_extractor(const complex &c) : m_c(c), m_value(real::shallow_copy_t{}, mpc_imagref(&c.m_mpc))
        {
        }

        const_im_extractor(const const_im_extractor &) = delete;
        const_im_extractor(const_im_extractor &&) = delete;
        const_im_extractor &operator=(const const_im_extractor &) = delete;
        const_im_extractor &operator=(const_im_extractor &&) = delete;

        ~const_im_extractor()
        {
            m_value._get_mpfr_t()->_mpfr_d = nullptr;
        }

        const real &get() const
        {
            return m_value;
        }

    private:
        const complex &m_c;
        real m_value;
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
