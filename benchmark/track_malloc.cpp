// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "track_malloc.hpp"

#if defined(MPPP_HAVE_GLIBC)

#include <atomic>
#include <cstddef>
#include <iostream>

namespace mppp_bench
{

namespace detail
{

namespace
{

// The tracking counter. This will be increased
// each time the malloc() override below is
// invoked.
std::atomic<std::size_t> malloc_counter(0);

} // namespace

} // namespace detail

} // namespace mppp_bench

// NOTE: see here for an explanation:
// https://stackoverflow.com/questions/17803456/an-alternative-for-the-deprecated-malloc-hook-functionality-of-glibc
extern "C" void *__libc_malloc(std::size_t size);

// NOTE: apparently it is not necessary to declare
// this function as visible.
extern "C" void *malloc(std::size_t size)
{
    ++mppp_bench::detail::malloc_counter;
    return ::__libc_malloc(size);
}

namespace mppp_bench
{

malloc_tracker::malloc_tracker(const char *s) : m_name(s), m_count(detail::malloc_counter.load()) {}

malloc_tracker::~malloc_tracker()
{
    // NOTE: compute the total number of allocations
    // before outputting to stream, so that we avoid
    // counting possible allocations from stream operations.
    const auto tot_nalloc = detail::malloc_counter.load() - m_count;
    std::cout << "Tracker '" << m_name << "' observed " << tot_nalloc << " malloc() calls." << std::endl;
}

} // namespace mppp_bench

#endif
