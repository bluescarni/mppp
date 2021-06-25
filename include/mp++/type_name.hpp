// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_TYPE_NAME_HPP
#define MPPP_TYPE_NAME_HPP

#include <string>
#include <type_traits>
#include <typeinfo>

#include <mp++/config.hpp>
#include <mp++/detail/visibility.hpp>

namespace mppp
{

namespace detail
{

MPPP_DLL_PUBLIC std::string demangle_from_typeid(const char *);

template <typename T>
inline std::string demangle_impl()
{
    return demangle_from_typeid(typeid(T).name());
}

#if defined(MPPP_HAVE_GCC_INT128) && defined(__APPLE__)

// NOTE: on OSX, it seems like typeid() for 128bit types is not implemented.
// Thus, we sidestep typeid() and provide directly the demangled
// names of the bugged types. These are the same names returned on linux.

template <>
inline std::string demangle_impl<__int128_t>()
{
    return "__int128";
}

template <>
inline std::string demangle_impl<__int128_t *>()
{
    return "__int128*";
}

template <>
inline std::string demangle_impl<__int128_t const *>()
{
    return "__int128 const*";
}

template <>
inline std::string demangle_impl<__uint128_t>()
{
    return "__uint128";
}

template <>
inline std::string demangle_impl<__uint128_t *>()
{
    return "__uint128*";
}

template <>
inline std::string demangle_impl<__uint128_t const *>()
{
    return "__uint128 const*";
}

#endif

} // namespace detail

// Determine the name of the type T at runtime.
template <typename T>
inline std::string type_name()
{
    // Get the demangled name without cvref.
    auto ret = detail::demangle_impl<typename std::remove_cv<typename std::remove_reference<T>::type>::type>();

    // Redecorate it with cv qualifiers.
    constexpr unsigned flag = unsigned(std::is_const<typename std::remove_reference<T>::type>::value)
                              + (unsigned(std::is_volatile<typename std::remove_reference<T>::type>::value) << 1);
    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
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

} // namespace mppp

#endif
