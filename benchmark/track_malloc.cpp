// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
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

namespace
{

// The tracking counter. This will be increased
// each time the malloc() override below is
// invoked.
std::atomic<std::size_t> malloc_counter(0);

} // namespace

// NOTE: see here for an explanation:
// https://stackoverflow.com/questions/17803456/an-alternative-for-the-deprecated-malloc-hook-functionality-of-glibc
extern "C" void *__libc_malloc(std::size_t size);

extern "C" __attribute__((visibility("default"))) void *malloc(std::size_t size) throw()
{
    ++malloc_counter;
    return ::__libc_malloc(size);
}

namespace mppp_bench
{

// NOTE: fetch the current count after creating the string,
// so that we don't record the string allocation in the counter.
malloc_tracker::malloc_tracker(const char *s) : m_name(s), m_count(malloc_counter.load()) {}

malloc_tracker::~malloc_tracker()
{
    // NOTE: compute the total number of allocations
    // before outputting to stream, so that we avoid
    // counting possible allocations from stream operations.
    const auto tot_nalloc = malloc_counter.load() - m_count;
    std::cout << "Tracker '" << m_name << "' observed " << tot_nalloc << " malloc() calls." << std::endl;
}

} // namespace mppp_bench

#endif
