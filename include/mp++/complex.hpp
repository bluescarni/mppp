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

#include <cassert>
#include <ostream>
#include <stdexcept>
#include <utility>

#include <mp++/detail/fwd_decl.hpp>
#include <mp++/detail/mpc.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/real.hpp>

namespace mppp
{

class MPPP_DLL_PUBLIC complex
{
    // Utility function to check the precision upon init.
    static ::mpfr_prec_t check_init_prec(::mpfr_prec_t p)
    {
        if (mppp_unlikely(!detail::real_prec_check(p))) {
            throw std::invalid_argument("Cannot init a complex with a precision of " + detail::to_string(p)
                                        + ": the maximum allowed precision is " + detail::to_string(real_prec_max())
                                        + ", the minimum allowed precision is " + detail::to_string(real_prec_min()));
        }
        return p;
    }

public:
    // Default ctor.
    complex();
    // Copy ctor.
    complex(const complex &);
    // Move constructor.
    complex(complex &&other) noexcept
    {
        // Shallow copy other.
        m_mpc = other.m_mpc;
        // Mark the other as moved-from.
        other.m_mpc.re->_mpfr_d = nullptr;
    }

    // Copy constructor with custom precision.
    explicit complex(const complex &, ::mpfr_prec_t);

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
        assert(mpfr_get_prec(mpc_realref(&m_mpc)) == mpfr_get_prec(mpc_imagref(&m_mpc)));

        return mpfr_get_prec(mpc_realref(&m_mpc));
    }

private:
    mpc_struct_t m_mpc;
};

MPPP_DLL_PUBLIC std::ostream &operator<<(std::ostream &, const complex &);

} // namespace mppp

#else

#error The complex.hpp header was included but mp++ was not configured with the MPPP_WITH_MPC option.

#endif

#endif
