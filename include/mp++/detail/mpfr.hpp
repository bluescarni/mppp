// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_MPFR_HPP
#define MPPP_DETAIL_MPFR_HPP

#include <cstdlib>
#include <iostream>
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

// Machinery to call automatically mpfr_free_cache() at program shutdown,
// if this header is included.

extern "C" {

// NOTE: the cleanup function should have C linkage, as it will be passed to atexit()
// which is a function fomr the C standard library.
void mpfr_cleanup_function();
}

// The actual implementation.
inline void mpfr_cleanup_function()
{
#if !defined(NDEBUG)
    // NOTE: functions registered with atexit() may be called concurrently.
    // Access to cout from concurrent threads is safe as long as the
    // cout object is synchronized to the underlying C stream:
    // https://stackoverflow.com/questions/6374264/is-cout-synchronized-thread-safe
    // http://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio
    // By default, this is the case, but in theory someone might have changed
    // the sync setting on cout by the time we execute the following line.
    // However, we print only in debug mode, so it should not be too much of a problem
    // in practice.
    std::cout << "Cleaning up MPFR caches." << std::endl;
#endif
    ::mpfr_free_cache();
}

struct mpfr_cleanup {
    mpfr_cleanup()
    {
        std::atexit(mpfr_cleanup_function);
    }
};

#if __cplusplus < 201703L

// Inline variable emulation machinery for pre-C++17.
template <typename = void>
struct mpfr_cleanup_holder {
    static const mpfr_cleanup s_cleanup;
};

template <typename T>
const mpfr_cleanup mpfr_cleanup_holder<T>::s_cleanup;

inline void inst_mpfr_cleanup()
{
    auto ptr = &mpfr_cleanup_holder<>::s_cleanup;
    (void)ptr;
}

#else

inline const mpfr_cleanup mpfr_cleanup_register;

#endif
}
}

#endif
