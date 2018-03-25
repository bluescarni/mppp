// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_DEMANGLE_HPP
#define MPPP_DETAIL_DEMANGLE_HPP

#include <mp++/config.hpp>

#if MPPP_CPLUSPLUS >= 201703L
#include <string_view>
#endif
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

#include <mp++/detail/type_traits.hpp>

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

#if MPPP_CPLUSPLUS >= 201703L

// C++17 string view overload.
inline std::string demangle(const std::string_view &s)
{
    return demangle(s.data());
}

#endif

// Convenience overload for demangling type_index. Will also work with type_info
// due to implicit conversion.
inline std::string demangle(const std::type_index &t_idx)
{
    return demangle(t_idx.name());
}

// Little wrapper to call typeid(T).name(). Necessary because sometimes
// typeid() on 128-bit ints is not working, and we need to work around that
// (see below). Works only if T is not cv-qualified and not a reference.
template <typename T>
inline std::string typeid_name_wrap()
{
    static_assert(std::is_same<T, uncvref_t<T>>::value, "The type T cannot be cv-qualified.");
    return typeid(T).name();
}

#if defined(MPPP_HAVE_GCC_INT128) && defined(__apple_build_version__)

// NOTE: it seems like on OSX typeid() is not working properly for 128-bit ints, hence
// we re-implement the functionality via the wrapper. Here we use the type names
// that are returned in a linux environment.
template <>
inline std::string typeid_name_wrap<__int128_t>()
{
    return "__int128";
}

template <>
inline std::string typeid_name_wrap<__uint128_t>()
{
    return "unsigned __int128";
}

#endif

// Demangler for type T. It will first strip reference and cv qualifications
// from T, and then re-decorate the resulting demangled name with the original
// qualifications. The reason we do this is because typeid ignores references
// and cv qualifications:
// http://en.cppreference.com/w/cpp/language/typeid
// See also this SO answer:
// https://stackoverflow.com/questions/28621844/is-there-a-typeid-for-references
template <typename T>
inline std::string demangle()
{
    // Get the uncvreffed demangled name.
    std::string ret = demangle(typeid_name_wrap<uncvref_t<T>>());
    // Redecorate it with cv qualifiers.
    const unsigned flag
        = unsigned(std::is_const<unref_t<T>>::value) + (unsigned(std::is_volatile<unref_t<T>>::value) << 1u);
    switch (flag) {
        case 0u:
            // NOTE: handle this explicitly to keep compiler warnings at bay.
            break;
        case 1u:
            ret += " const";
            break;
        case 2u:
            ret += " volatile";
            break;
        case 3u:
            ret += " const volatile";
    }
    // Re-add the reference, if necessary.
    if (std::is_lvalue_reference<T>::value) {
        ret += " &";
    } else if (std::is_rvalue_reference<T>::value) {
        ret += " &&";
    }
    return ret;
}
} // namespace detail
} // namespace mppp

#if defined(_MSC_VER)

#pragma warning(pop)

#endif

#endif
