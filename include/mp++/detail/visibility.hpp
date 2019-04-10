// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_VISIBILITY_HPP
#define MPPP_DETAIL_VISIBILITY_HPP

// Convenience macros for visibility attributes. Mostly insipred by:
// https://gcc.gnu.org/wiki/Visibility
#if defined(_WIN32) || defined(__CYGWIN__)

#if defined(mppp_EXPORTS)

#define MPPP_PUBLIC __declspec(dllexport)

#else

#define MPPP_PUBLIC __declspec(dllimport)

#endif

#else

#define MPPP_PUBLIC __attribute__((visibility("default")))

#endif

#endif
