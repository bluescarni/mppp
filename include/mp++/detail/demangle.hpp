// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_DEMANGLE_HPP
#define MPPP_DETAIL_DEMANGLE_HPP

#include <string>
#include <typeindex>
#include <typeinfo>

#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))

// GCC demangle. This is available also for clang, both with libstdc++ and libc++.
#include <cstdlib>
#include <cxxabi.h>
#include <memory>

#endif

namespace mppp
{
inline namespace detail
{

inline std::string demangle(const char *s)
{
#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))
    int status = -4;
    // NOTE: abi::__cxa_demangle will return a pointer allocated by std::malloc, which we will delete via std::free.
    std::unique_ptr<char, void (*)(void *)> res{::abi::__cxa_demangle(s, nullptr, nullptr, &status), std::free};
    // NOTE: it seems like clang with libc++ does not set the status variable properly.
    // We then check if anything was allocated by __cxa_demangle(), as here it mentions
    // that in case of failure the pointer will be set to null:
    // https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a01696.html
    return res.get() ? std::string(res.get()) : std::string(s);
#else
    // TODO demangling for other platforms. E.g.,
    // http://stackoverflow.com/questions/13777681/demangling-in-msvc
    return std::string(s);
#endif
}

// C++ string overload.
inline std::string demangle(const std::string &s)
{
    return demangle(s.c_str());
}

// Convenience overload for demangling type_index. Will also work with type_info
// due to implicit conversion.
inline std::string demangle(const std::type_index &t_idx)
{
    return demangle(t_idx.name());
}

// Convenience overload with template type.
template <typename T>
inline std::string demangle()
{
    return demangle(typeid(T));
}
}
}

#endif
