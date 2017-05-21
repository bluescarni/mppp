// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_MPFR_HPP
#define MPPP_DETAIL_MPFR_HPP

#include <limits>
#include <memory>
#include <mpfr.h>
#include <type_traits>

#include <mp++/detail/gmp.hpp>

#if MPFR_VERSION_MAJOR < 3

#error Minimum supported MPFR version is 3.

#endif

namespace mppp
{

inline namespace detail
{

// mpfr_t is an array of some struct.
using mpfr_struct_t = std::remove_extent<::mpfr_t>::type;

// Simple RAII holder for MPFR floats.
struct mpfr_raii {
    mpfr_raii(::mpfr_prec_t prec)
    {
        ::mpfr_init2(&m_mpfr, prec);
    }
    ~mpfr_raii()
    {
        ::mpfr_clear(&m_mpfr);
    }
    mpfr_struct_t m_mpfr;
};

// Smart pointer to handle the string output from mpfr.
using smart_mpfr_str = std::unique_ptr<char, void (*)(char *)>;

// A couple of sanity checks when constructing temporary mpfrs/mpfs from long double.
static_assert(std::numeric_limits<long double>::digits10 < std::numeric_limits<int>::max() / 4, "Overflow error.");
static_assert(std::numeric_limits<long double>::digits10 * 4 < std::numeric_limits<::mpfr_prec_t>::max(),
              "Overflow error.");
static_assert(std::numeric_limits<long double>::digits10 * 4 < std::numeric_limits<::mp_bitcnt_t>::max(),
              "Overflow error.");
}
}

#endif
