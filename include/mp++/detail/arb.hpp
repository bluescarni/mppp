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
}
}

#endif
