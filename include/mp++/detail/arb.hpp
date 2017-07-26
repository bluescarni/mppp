// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_ARB_HPP
#define MPPP_DETAIL_ARB_HPP

#include <arf.h>
#include <cstdlib>
#include <flint/flint.h>

namespace mppp
{

inline namespace detail
{

// Simple RAII holder for arf floats.
struct arf_raii {
    arf_raii()
    {
        ::arf_init(&m_arf);
    }
    ~arf_raii()
    {
        ::arf_clear(&m_arf);
    }
    ::arf_struct m_arf;
};

// Machinery to call automatically flint_cleanup() at program shutdown,
// if this header is included.
struct arb_cleanup {
    arb_cleanup()
    {
        std::atexit(::flint_cleanup);
    }
};

template <typename = void>
struct arb_cleanup_holder {
    static arb_cleanup s_cleanup;
};

template <typename T>
arb_cleanup arb_cleanup_holder<T>::s_cleanup;

inline void inst_arb_cleanup()
{
    auto ptr = &arb_cleanup_holder<>::s_cleanup;
    (void)ptr;
}
}
}

#endif
