// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_BENCHMARK_TRACK_MALLOC_HPP
#define MPPP_BENCHMARK_TRACK_MALLOC_HPP

// Machinery to check the presence of GLIBC. See:
// https://sourceforge.net/p/predef/wiki/Libraries/
#include <climits>

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)

#define MPPP_HAVE_GLIBC

#endif

#if defined(MPPP_HAVE_GLIBC)

#include <cstddef>
#include <string>

namespace mppp_bench
{

struct __attribute__((visibility("default"))) malloc_tracker {
    explicit malloc_tracker(const char *);
    ~malloc_tracker();
    // NOTE: it is important than m_name
    // is inited befor m_count, so that
    // we don't end up counting allocations
    // from the string.
    std::string m_name;
    std::size_t m_count;
};

} // namespace mppp_bench

#else

namespace mppp_bench
{

struct malloc_tracker {
    explicit malloc_tracker(const char *) {}
    ~malloc_tracker() {}
};

} // namespace mppp_bench

#endif

#endif
