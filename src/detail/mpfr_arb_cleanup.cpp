// Copyright 2016-2026 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <iostream>

#include <mp++/config.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/real.hpp>

#if defined(MPPP_WITH_MPC)

#include <mp++/complex.hpp>
#include <mp++/detail/mpc.hpp>

#endif

#if defined(MPPP_WITH_FLINT)

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

MPPP_BEGIN_NAMESPACE

namespace detail
{

namespace
{

// NOTE: the cleanup machinery relies on a correct implementation of the thread_local keyword. If that is not available,
// we'll just skip the cleanup step altogether, which may result in memory leaks being reported by sanitizers and
// valgrind.
#if defined(MPPP_HAVE_THREAD_LOCAL)

// NOTE: MPFR has both thread-local and global caches. Thus, we use two cleanup functors, one thread local and one
// global.
//
// NOTE: the intent of these global cleanup objects is to automatically invoke cleanup actions for MPFR/flint when the
// program exits or when a thread finishes. Due to several C++ vagaries related to the order of construction and
// destruction of global objects, it is basically impossible to guarantee that this very simple and reasonable thing is
// successful in all possible scenarios. There are several bits of stupid fuckery conspiring against common sense,
// including that not even with constinit we can guarantee a specific order of destruction between
// statically-initialised and dynamically-initialised objects, and that as far as thread-local objects go, there is
// apparently absolutely no fucking portable way to ensure that an object is ALWAYS created on thread startup (it is not
// clear whether this is a defect in the standard, a choice left to the implementation or something else).
//
// As a result, these cleanup functors operate on a best-effort basis and, strictly speaking, they cannot guarantee that
// no leaks will be reported by memory checking tools or even that thread-local caches will be cleaned up when a thread
// exits.

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct mpfr_tl_cleanup {
    constexpr mpfr_tl_cleanup() = default;
    ~mpfr_tl_cleanup()
    {
#if !defined(NDEBUG)
        // NOTE: Access to cout from concurrent threads is safe as long as the
        // cout object is synchronized to the underlying C stream:
        //
        // https://stackoverflow.com/questions/6374264/is-cout-synchronized-thread-safe
        // http://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio
        //
        // By default, this is the case, but in theory someone might have changed the sync setting on cout by the time
        // we execute the following line. However, we print only in debug mode, so it should not be too much of a
        // problem in practice.
        //
        // NOLINTNEXTLINE(performance-avoid-endl)
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
        // NOLINTNEXTLINE(performance-avoid-endl)
        std::cout << "Cleaning up global MPFR caches." << std::endl;
#endif
        ::mpfr_free_cache2(MPFR_FREE_GLOBAL_CACHE);
    }
};

MPPP_CONSTINIT thread_local const mpfr_tl_cleanup mpfr_tl_cleanup_inst;
MPPP_CONSTINIT const mpfr_global_cleanup mpfr_global_cleanup_inst;

#if defined(MPPP_WITH_FLINT)

// A cleanup functor that will call flint_cleanup() on destruction. Same idea as the MPFR cleanup objects, see comments
// there.
//
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct flint_cleanup {
    constexpr flint_cleanup() = default;
    ~flint_cleanup()
    {
#if !defined(NDEBUG)
        // NOLINTNEXTLINE(performance-avoid-endl)
        std::cout << "Cleaning up thread local FLINT caches." << std::endl;
#endif
        ::flint_cleanup();
    }
};

MPPP_CONSTINIT thread_local const flint_cleanup flint_cleanup_inst;

#endif

#endif

} // namespace

} // namespace detail

// Destructor.
real::~real()
{
    // NOTE: these ignore() calls are meant to force a "use" of the cleanup objects in an effort to guarantee that they
    // are actually initialised (which in turn ensures that the cleanup actions in their destructors will be run). This
    // is necessary because in some situations the compiler may delay the initialisation of these objects until their
    // first use. By placing these calls inside the real dtor, the cleanup actions will be executed in all threads which
    // destroy at least one real before exiting (on a best-effort basis, see the notes above).
    //
    // Same for the complex class below.
    //
    // NOTE: in certain corner cases it is possible that mpfr_tl_cleanup_inst et al. have already been destroyed by the
    // time this runs (e.g., for a static real instance). This is still ok, the storage for these objects is still there
    // and it is ok to take the address of a destroyed object.
#if defined(MPPP_HAVE_THREAD_LOCAL)
    detail::ignore(&detail::mpfr_tl_cleanup_inst);
    detail::ignore(&detail::mpfr_global_cleanup_inst);
#if defined(MPPP_WITH_FLINT)
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
    // NOTE: see docs in the real dtor for an explanation of these ignore() calls. We repeat them here because MPC (and
    // Arb) might make use of MPFR's (and flint's) caches internally and creating complex instances does not necessarily
    // involve creating real objects.
#if defined(MPPP_HAVE_THREAD_LOCAL)
    detail::ignore(&detail::mpfr_tl_cleanup_inst);
    detail::ignore(&detail::mpfr_global_cleanup_inst);
#if defined(MPPP_WITH_FLINT)
    detail::ignore(&detail::flint_cleanup_inst);
#endif
#endif
    if (is_valid()) {
        // The object is not moved-from, destroy it.

        // Check that the imaginary part is also valid.
        assert(mpc_imagref(&m_mpc)->_mpfr_d);
        // Check that the precision of the imaginary part is equal to the precision of the real part.
        assert(get_prec() == mpfr_get_prec(mpc_imagref(&m_mpc)));
        // Check that the precision value is valid.
        assert(detail::real_prec_check(get_prec()));

        ::mpc_clear(&m_mpc);
    }
}

#endif

MPPP_END_NAMESPACE
