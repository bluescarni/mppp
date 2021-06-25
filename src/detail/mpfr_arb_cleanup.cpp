// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <cassert>
#include <iostream>

#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/real.hpp>

#if defined(MPPP_WITH_MPC)

#include <mp++/complex.hpp>
#include <mp++/detail/mpc.hpp>

#endif

#if defined(MPPP_WITH_ARB)

#if defined(_MSC_VER) && !defined(__clang__)

// Disable some warnings for MSVC.
#pragma warning(push)
#pragma warning(disable : 4146)
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)

#endif

#include <flint/flint.h>

#if defined(_MSC_VER) && !defined(__clang__)

#pragma warning(pop)

#endif

#endif

namespace mppp
{

namespace detail
{

namespace
{

// NOTE: the cleanup machinery relies on a correct implementation
// of the thread_local keyword. If that is not available, we'll
// just skip the cleanup step altogether, which may result
// in "memory leaks" being reported by sanitizers and valgrind.
#if defined(MPPP_HAVE_THREAD_LOCAL)

#if MPFR_VERSION_MAJOR < 4

// A cleanup functor that will call mpfr_free_cache()
// on destruction.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct mpfr_cleanup {
    // NOTE: marking the ctor constexpr ensures that the initialisation
    // of objects of this class with static storage duration is sequenced
    // before the dynamic initialisation of objects with static storage
    // duration:
    // https://en.cppreference.com/w/cpp/language/constant_initialization
    // This essentially means that we are sure that mpfr_free_cache()
    // will be called after the destruction of any static real object
    // (because real doesn't have a constexpr constructor and thus
    // static reals are destroyed before static objects of this class).
    constexpr mpfr_cleanup() = default;
    ~mpfr_cleanup()
    {
#if !defined(NDEBUG)
        // NOTE: Access to cout from concurrent threads is safe as long as the
        // cout object is synchronized to the underlying C stream:
        // https://stackoverflow.com/questions/6374264/is-cout-synchronized-thread-safe
        // http://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio
        // By default, this is the case, but in theory someone might have changed
        // the sync setting on cout by the time we execute the following line.
        // However, we print only in debug mode, so it should not be too much of a problem
        // in practice.
        std::cout << "Cleaning up MPFR caches." << std::endl;
#endif
        ::mpfr_free_cache();
    }
};

// Instantiate a cleanup object for each thread.
MPPP_CONSTINIT thread_local const mpfr_cleanup mpfr_cleanup_inst;

#else

// NOTE: in MPFR >= 4, there are both local caches and thread-specific caches.
// Thus, we use two cleanup functors, one thread local and one global.

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct mpfr_tl_cleanup {
    constexpr mpfr_tl_cleanup() = default;
    ~mpfr_tl_cleanup()
    {
#if !defined(NDEBUG)
        std::cout << "Cleaning up thread local MPFR caches." << std::endl;
#endif
        ::mpfr_free_cache2(MPFR_FREE_LOCAL_CACHE);
    }
};

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct mpfr_global_cleanup {
    constexpr mpfr_global_cleanup() = default;
    ~mpfr_global_cleanup()
    {
#if !defined(NDEBUG)
        std::cout << "Cleaning up global MPFR caches." << std::endl;
#endif
        ::mpfr_free_cache2(MPFR_FREE_GLOBAL_CACHE);
    }
};

#if defined(__INTEL_COMPILER)

#pragma warning(push)
#pragma warning(disable : 854)

#endif

MPPP_CONSTINIT thread_local const mpfr_tl_cleanup mpfr_tl_cleanup_inst;
// NOTE: because the destruction of thread-local objects
// always happens before the destruction of objects with
// static storage duration, the global cleanup will always
// be performed after thread-local cleanup.
MPPP_CONSTINIT const mpfr_global_cleanup mpfr_global_cleanup_inst;

#if defined(__INTEL_COMPILER)

#pragma warning(pop)

#endif

#endif

#if defined(MPPP_WITH_ARB)

// A cleanup functor that will call flint_cleanup()
// on destruction.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct flint_cleanup {
    // NOTE: marking the ctor constexpr ensures that the initialisation
    // of objects of this class with static storage duration is sequenced
    // before the dynamic initialisation of objects with static storage
    // duration:
    // https://en.cppreference.com/w/cpp/language/constant_initialization
    // This essentially means that we are sure that flint_cleanup()
    // will be called after the destruction of any static real object
    // (because real doesn't have a constexpr constructor and thus
    // static reals are destroyed before static objects of this class).
    constexpr flint_cleanup() = default;
    ~flint_cleanup()
    {
#if !defined(NDEBUG)
        // NOTE: Access to cout from concurrent threads is safe as long as the
        // cout object is synchronized to the underlying C stream:
        // https://stackoverflow.com/questions/6374264/is-cout-synchronized-thread-safe
        // http://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio
        // By default, this is the case, but in theory someone might have changed
        // the sync setting on cout by the time we execute the following line.
        // However, we print only in debug mode, so it should not be too much of a problem
        // in practice.
        std::cout << "Cleaning up thread local FLINT caches." << std::endl;
#endif
        ::flint_cleanup();
    }
};

#if defined(__INTEL_COMPILER)

#pragma warning(push)
#pragma warning(disable : 854)

#endif

// Instantiate a cleanup object for each thread.
MPPP_CONSTINIT thread_local const flint_cleanup flint_cleanup_inst;

#if defined(__INTEL_COMPILER)

#pragma warning(pop)

#endif

#endif

#endif

} // namespace

} // namespace detail

// Destructor.
real::~real()
{
#if defined(MPPP_HAVE_THREAD_LOCAL)
#if MPFR_VERSION_MAJOR < 4
    // NOTE: make sure we "use" the cleanup instantiation functor,
    // so that the compiler is forced to invoke its constructor.
    // This ensures that, as long as at least one real is created, the mpfr_free_cache()
    // function is called on shutdown.
    detail::ignore(&detail::mpfr_cleanup_inst);
#else
    detail::ignore(&detail::mpfr_tl_cleanup_inst);
    detail::ignore(&detail::mpfr_global_cleanup_inst);
#endif
#if defined(MPPP_WITH_ARB)
    detail::ignore(&detail::flint_cleanup_inst);
#endif
#endif
    if (is_valid()) {
        // The object is not moved-from, destroy it.
        assert(detail::real_prec_check(get_prec()));
        ::mpfr_clear(&m_mpfr);
    }
}

#if defined(MPPP_WITH_MPC)

complex::~complex()
{
#if defined(MPPP_HAVE_THREAD_LOCAL)
#if MPFR_VERSION_MAJOR < 4
    // NOTE: make sure we "use" the cleanup instantiation functor,
    // so that the compiler is forced to invoke its constructor.
    // This ensures that, as long as at least one real is created, the mpfr_free_cache()
    // function is called on shutdown.
    detail::ignore(&detail::mpfr_cleanup_inst);
#else
    detail::ignore(&detail::mpfr_tl_cleanup_inst);
    detail::ignore(&detail::mpfr_global_cleanup_inst);
#endif
#if defined(MPPP_WITH_ARB)
    detail::ignore(&detail::flint_cleanup_inst);
#endif
#endif
    if (is_valid()) {
        // The object is not moved-from, destroy it.

        // Check that the imaginary part is also
        // valid.
        assert(mpc_imagref(&m_mpc)->_mpfr_d);
        // Check that the precision of the imaginary
        // part is equal to the precision of the real
        // part.
        assert(get_prec() == mpfr_get_prec(mpc_imagref(&m_mpc)));
        // Check that the precision value is valid.
        assert(detail::real_prec_check(get_prec()));

        ::mpc_clear(&m_mpc);
    }
}

#endif

} // namespace mppp
