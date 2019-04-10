// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <string>

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

#include <mp++/demangle.hpp>

namespace mppp
{

namespace detail
{

std::string demangle_from_typeid(const char *s)
{
#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))
    // NOTE: wrap std::free() in a local lambda, so we avoid
    // potential ambiguities when taking the address of std::free().
    // See:
    // https://stackoverflow.com/questions/27440953/stdunique-ptr-for-c-functions-that-need-free
    auto deleter = [](void *ptr) { std::free(ptr); };

    // NOTE: abi::__cxa_demangle will return a pointer allocated by std::malloc, which we will delete via std::free().
    std::unique_ptr<char, decltype(deleter)> res{::abi::__cxa_demangle(s, nullptr, nullptr, nullptr), deleter};

    // NOTE: return the original string if demangling fails.
    return res ? std::string(res.get()) : std::string(s);
#elif defined(_MSC_VER)
    // NOTE: the Windows function for demangling is not thread safe, we will have
    // to protect it with a mutex.
    // https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms681400(v=vs.85).aspx
    // Local static init is thread safe in C++11.
    static std::mutex mut;
    char undecorated_name[1024];
    auto und_name = static_cast<::PSTR>(undecorated_name);
    auto size = static_cast<::DWORD>(sizeof(undecorated_name));
    const auto ret = [s, und_name, size]() {
        std::lock_guard<std::mutex> lock{mut};
        return ::UnDecorateSymbolName(s, und_name, size, UNDNAME_COMPLETE);
    }();
    // Nonzero retval means success. Otherwise, return the mangled name.
    return ret ? std::string(undecorated_name) : std::string(s);
#else
    // If no demangling is available, just return the mangled name.
    return std::string(s);
#endif
}

} // namespace detail

} // namespace mppp

#if defined(_MSC_VER)

#pragma warning(pop)

#endif
