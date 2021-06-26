// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_TYPE_TRAITS_HPP
#define MPPP_DETAIL_TYPE_TRAITS_HPP

#include <mp++/config.hpp>

#include <limits>
#include <type_traits>

namespace mppp
{

namespace detail
{

// A bunch of useful utilities from C++14/C++17.

// http://en.cppreference.com/w/cpp/types/void_t
#if MPPP_CPLUSPLUS >= 201703L

using std::void_t;

#else

template <typename... Ts>
struct make_void {
    typedef void type;
};

template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

#endif

// http://en.cppreference.com/w/cpp/experimental/is_detected
template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

// http://en.cppreference.com/w/cpp/experimental/nonesuch
struct nonesuch {
    nonesuch() = delete;
    ~nonesuch() = delete;
    nonesuch(nonesuch &&) = delete;
    nonesuch(nonesuch const &) = delete;
    void operator=(nonesuch const &) = delete;
    void operator=(nonesuch &&) = delete;
};

template <template <class...> class Op, class... Args>
using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

// http://en.cppreference.com/w/cpp/types/conjunction
// http://en.cppreference.com/w/cpp/types/disjunction
// http://en.cppreference.com/w/cpp/types/negation
#if MPPP_CPLUSPLUS >= 201703L

using std::conjunction;
using std::disjunction;
using std::negation;

#else

template <class...>
struct conjunction : std::true_type {
};

template <class B1>
struct conjunction<B1> : B1 {
};

template <class B1, class... Bn>
struct conjunction<B1, Bn...> : std::conditional<B1::value != false, conjunction<Bn...>, B1>::type {
};

template <class...>
struct disjunction : std::false_type {
};

template <class B1>
struct disjunction<B1> : B1 {
};

template <class B1, class... Bn>
struct disjunction<B1, Bn...> : std::conditional<B1::value != false, B1, disjunction<Bn...>>::type {
};

template <class B>
struct negation : std::integral_constant<bool, !B::value> {
};

#endif

// Small helpers, like C++14.
template <bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T>
using remove_cv_t = typename std::remove_cv<T>::type;

template <typename T>
using remove_extent_t = typename std::remove_extent<T>::type;

template <typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

// Some handy aliases.
template <typename T>
using unref_t = typename std::remove_reference<T>::type;

template <typename T>
using uncvref_t = remove_cv_t<unref_t<T>>;

// Detect non-const rvalue references.
template <typename T>
using is_ncrvr = conjunction<std::is_rvalue_reference<T>, negation<std::is_const<unref_t<T>>>>;

// Provide internal implementation of some std type traits,
// we will augment them with non-standard types defined
// on some compilers.
template <typename T>
using is_integral = disjunction<std::is_integral<T>
#if defined(MPPP_HAVE_GCC_INT128)
                                ,
                                // NOTE: for many of these type traits, the result must hold regardless
                                // of the cv qualifications of T. Hence, we remove them.
                                // http://eel.is/c++draft/meta.unary.cat
                                std::is_same<remove_cv_t<T>, __int128_t>, std::is_same<remove_cv_t<T>, __uint128_t>
#endif
                                >;

template <typename T>
using is_signed = disjunction<std::is_signed<T>
#if defined(MPPP_HAVE_GCC_INT128)
                              ,
                              std::is_same<remove_cv_t<T>, __int128_t>
#endif
                              >;

template <typename T>
using is_unsigned = disjunction<std::is_unsigned<T>
#if defined(MPPP_HAVE_GCC_INT128)
                                ,
                                std::is_same<remove_cv_t<T>, __uint128_t>
#endif
                                >;

// make_unsigned machinery,
template <typename T, typename = void>
struct make_unsigned_impl {
    using type = typename std::make_unsigned<T>::type;
};

// NOTE: before GCC 4.9.1 the specialisation of std::make_unsigned for wchar_t
// is broken. See the bugreport here:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?format=multiple&id=60326
//
// It seems bits are also missing for char16_t and char32_t, but apparently
// not in relation to make_unsigned (only make_signed, which we don't use
// so far). This workaround is lifted directly from the commit cited
// in the bugreport.
#if defined(__GNUC__) && __GNUC__ == 4 && (__GNUC_MINOR__ < 9 || (__GNUC_MINOR__ == 9 && __GNUC_PATCHLEVEL__ < 1))

#if defined(_GLIBCXX_USE_WCHAR_T) && !defined(__WCHAR_UNSIGNED__)

template <>
struct make_unsigned_impl<wchar_t> {
    using type = typename make_unsigned_impl<__WCHAR_TYPE__>::type;
};

#endif

#endif

#if defined(MPPP_HAVE_GCC_INT128)

// NOTE: make_unsigned is supposed to preserve cv qualifiers, hence the non-trivial implementation.
template <typename T>
struct make_unsigned_impl<T, enable_if_t<disjunction<std::is_same<remove_cv_t<T>, __uint128_t>,
                                                     std::is_same<remove_cv_t<T>, __int128_t>>::value>> {
    using tmp_type = typename std::conditional<std::is_const<T>::value, const __uint128_t, __uint128_t>::type;
    using type = typename std::conditional<std::is_volatile<T>::value, volatile tmp_type, tmp_type>::type;
};

#endif

template <typename T>
using make_unsigned_t = typename make_unsigned_impl<T>::type;

// Various numeric_limits utils.
template <typename T>
constexpr int nl_digits() noexcept
{
    return std::numeric_limits<T>::digits;
}

template <typename T>
constexpr T nl_min() noexcept
{
    return std::numeric_limits<T>::min();
}

template <typename T>
constexpr T nl_max() noexcept
{
    return std::numeric_limits<T>::max();
}

#if defined(MPPP_HAVE_GCC_INT128)

template <>
constexpr int nl_digits<__uint128_t>() noexcept
{
    return 128;
}

template <>
constexpr int nl_digits<__int128_t>() noexcept
{
    return 127;
}

template <>
constexpr __uint128_t nl_max<__uint128_t>() noexcept
{
    return ~__uint128_t(0);
}

template <>
constexpr __uint128_t nl_min<__uint128_t>() noexcept
{
    return 0;
}

template <>
constexpr __int128_t nl_max<__int128_t>() noexcept
{
    return static_cast<__int128_t>((__uint128_t(1) << 127u) - 1u);
}

template <>
constexpr __int128_t nl_min<__int128_t>() noexcept
{
    return -nl_max<__int128_t>() - 1;
}

#endif

// Need this little wrapper because MSVC is unhappy with constexpr
// functions in SFINAE.
template <typename T>
struct nl_constants {
    static constexpr int digits = nl_digits<T>();
};

} // namespace detail

} // namespace mppp

#endif
