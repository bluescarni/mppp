// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstdlib>
#include <iostream>

#include <mp++/detail/mpfr.hpp>
#include <mp++/real.hpp>

namespace mppp
{

namespace detail
{

namespace
{

// Machinery to call automatically mpfr_free_cache() at program shutdown.
extern "C" {
// NOTE: the cleanup function should have C linkage, as it will be passed to atexit()
// which is a function from the C standard library.
static void mpfr_cleanup_function()
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
}

} // namespace

} // namespace detail

real::mpfr_cleanup::mpfr_cleanup()
{
    std::atexit(detail::mpfr_cleanup_function);
}

const real::mpfr_cleanup real::s_cleanup;

} // namespace mppp
