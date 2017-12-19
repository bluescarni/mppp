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

#elif defined(_MSC_VER)

// Disable some warnings for MSVC.
#pragma warning(push)
#pragma warning(disable : 4091)

#include <mutex>

// clang-format off
#include <Windows.h>
#include <Dbghelp.h>
// clang-format on

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
#elif defined(_MSC_VER)
    // NOTE: the Windows function for demangling is not thread safe, we will have
    // to protect it with a mutex.
    // https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms681400(v=vs.85).aspx
    // Local static init is thread safe in C++11.
    static std::mutex mut;
    char undecorated_name[1024];
    ::DWORD ret;
    {
        std::lock_guard<std::mutex> lock{mut};
        ret = ::UnDecorateSymbolName(s, undecorated_name, sizeof(undecorated_name), UNDNAME_COMPLETE);
    }
    if (ret) {
        // Nonzero retval means success.
        return std::string(undecorated_name);
    }
    // Otherwise, return the mangled name.
    return std::string(s);
#else
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

#if defined(_MSC_VER)

#pragma warning(pop)

#endif

#endif
