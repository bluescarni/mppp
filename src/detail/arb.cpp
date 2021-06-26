// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <stdexcept>

#if defined(_MSC_VER) && !defined(__clang__)

// Disable some warnings for MSVC.
#pragma warning(push)
#pragma warning(disable : 4146)
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)

#endif

#include <flint/flint.h>
#include <flint/fmpz.h>

#include <arb.h>
#include <arb_hypgeom.h>
#include <arf.h>
#include <mag.h>

#if defined(MPPP_WITH_MPC)

#include <acb.h>

#endif

#if defined(_MSC_VER) && !defined(__clang__)

#pragma warning(pop)

#endif

#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/real.hpp>

#if defined(MPPP_WITH_MPC)

#include <mp++/complex.hpp>
#include <mp++/detail/mpc.hpp>

#endif

namespace mppp
{

namespace detail
{

namespace
{

// Minimal RAII struct to hold
// arb_t types.
// NOTE: we used to have an arf_raii struct here as well,
// but it is not used any more. This is the last commit
// where it is still present:
// 4c1b916e68e124ababb18311ca217715134ced3a
struct arb_raii {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    arb_raii()
    {
        ::arb_init(m_arb);
    }
    arb_raii(const arb_raii &) = delete;
    arb_raii(arb_raii &&) = delete;
    arb_raii &operator=(const arb_raii &) = delete;
    arb_raii &operator=(arb_raii &&) = delete;
    ~arb_raii()
    {
        ::arb_clear(m_arb);
    }
    ::arb_t m_arb;
};

#if defined(MPPP_WITH_MPC)

// Minimal RAII struct to hold
// acb_t types.
struct acb_raii {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    acb_raii()
    {
        ::acb_init(m_acb);
    }
    acb_raii(const acb_raii &) = delete;
    acb_raii(acb_raii &&) = delete;
    acb_raii &operator=(const acb_raii &) = delete;
    acb_raii &operator=(acb_raii &&) = delete;
    ~acb_raii()
    {
        ::acb_clear(m_acb);
    }
    ::acb_t m_acb;
};

#endif

// Small helper to turn an MPFR precision
// into an Arb precision.
::slong mpfr_prec_to_arb_prec(::mpfr_prec_t p)
{
    const auto ret = safe_cast<::slong>(p);

    // LCOV_EXCL_START
    // Check that ret is not too small.
    // NOTE: the minimum precision in Arb is 2.
    if (mppp_unlikely(ret < 2)) {
        throw std::invalid_argument("A precision of at least 2 bits is required in order to use Arb's functions");
    }

    // Check that ret is not too large. The Arb documentation suggests < 2**24
    // for 32-bit and < 2**36 for 64-bit. We use slightly smaller values.
    // NOTE: the docs of ulong state that it has exactly either 64 or 32 bit width,
    // depending on the architecture.
    if (nl_digits<::ulong>() == 64) {
        if (mppp_unlikely(ret > (1ll << 32))) {
            throw std::invalid_argument("A precision of " + to_string(ret) + " bits is too large for Arb's functions");
        }
    } else {
        if (mppp_unlikely(ret > (1ll << 20))) {
            throw std::invalid_argument("A precision of " + to_string(ret) + " bits is too large for Arb's functions");
        }
    }
    // LCOV_EXCL_STOP

    return ret;
}

// Helper to convert an arf_t into an mpfr_t.
// NOTE: at the moment we don't have easy ways to test
// the failure mode below. Perhaps in the future
// we'll have wrappers for Arb functions that can
// drastically increase the exponents, and we may be
// able to use them to test failures when converting
// from Arb to MPFR.
void arf_to_mpfr(::mpfr_t rop, const ::arf_t op)
{
    // NOTE: if op is not a special value,
    // we'll have to check if its exponent
    // if a multiprecision integer. In such a case,
    // arf_get_mpfr() will abort, and we want to avoid that.
    // See:
    // https://github.com/fredrik-johansson/arb/blob/e71718411e5f59615dce8e790f6f89bf208057a6/arf/get_mpfr.c#L31
    // LCOV_EXCL_START
    if (mppp_unlikely(!::arf_is_special(op) && COEFF_IS_MPZ(*ARF_EXPREF(op)))) {
        throw std::invalid_argument("In the conversion of an arf_t to an mpfr_t, the exponent of the arf_t object "
                                    "is too large for the conversion to be successful");
    }
    // LCOV_EXCL_STOP

    // Extract an mpfr from the arf.
    ::arf_get_mpfr(rop, op, MPFR_RNDN);
}

// Helper to convert an mpfr_t into an arb_t.
void mpfr_to_arb(::arb_t rop, const ::mpfr_t op)
{
    // Set the midpoint.
    // NOTE: this function will set rop *exactly* to op.
    ::arf_set_mpfr(arb_midref(rop), op);
    // Set the radius to zero.
    ::mag_zero(arb_radref(rop));
}

#if defined(MPPP_WITH_MPC)

// Helper to convert an mpc_t into an acb_t.
void mpc_to_acb(::acb_t rop, const ::mpc_t op)
{
    mpfr_to_arb(acb_realref(rop), mpc_realref(op));
    mpfr_to_arb(acb_imagref(rop), mpc_imagref(op));
}

// Helper to convert an acb_t into an mpc_t.
void acb_to_mpc(::mpc_t rop, const ::acb_t op)
{
    arf_to_mpfr(mpc_realref(rop), arb_midref(acb_realref(op)));
    arf_to_mpfr(mpc_imagref(rop), arb_midref(acb_imagref(op)));
}

#endif

} // namespace

// Helper for the implementation of unary Arb wrappers.
// NOTE: it would probably pay off to put a bunch of thread-local arb_raii
// objects in the unnamed namespace above, and use those, instead of function-local
// statics, to convert to/from MPFR. However such a scheme would not work
// on MinGW due to the thread_local issues, so as long as we support MinGW
// (or any platform with non-functional thread_local), we cannot adopt this approach.
#define MPPP_UNARY_ARB_WRAPPER(fname)                                                                                  \
    void arb_##fname(::mpfr_t rop, const ::mpfr_t op)                                                                  \
    {                                                                                                                  \
        MPPP_MAYBE_TLS arb_raii arb_rop, arb_op;                                                                       \
        /* Turn op into an arb. */                                                                                     \
        mpfr_to_arb(arb_op.m_arb, op);                                                                                 \
        /* Run the computation, using the precision of rop to mimic */                                                 \
        /* the behaviour of MPFR functions. */                                                                         \
        ::arb_##fname(arb_rop.m_arb, arb_op.m_arb, mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));                         \
        /* Write the result into rop. */                                                                               \
        arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));                                                                   \
    }

// Implementation of the Arb MPFR wrappers.
MPPP_UNARY_ARB_WRAPPER(sqrt1pm1)

// NOTE: log_hypot needs special handling for certain
// input values.
void arb_log_hypot(::mpfr_t rop, const ::mpfr_t x, const ::mpfr_t y)
{
    // Special handling if at least one of x and y is an inf,
    // and the other is not a NaN.
    if (mpfr_inf_p(x) && !mpfr_nan_p(y)) {
        // x is inf, y not a nan. Return +inf.
        ::mpfr_set_inf(rop, 1);
    } else if (!mpfr_nan_p(x) && mpfr_inf_p(y)) {
        // y is inf, x not a nan. Return +inf.
        ::mpfr_set_inf(rop, 1);
    } else {
        MPPP_MAYBE_TLS arb_raii arb_rop, arb_x, arb_y;

        mpfr_to_arb(arb_x.m_arb, x);
        mpfr_to_arb(arb_y.m_arb, y);

        ::arb_log_hypot(arb_rop.m_arb, arb_x.m_arb, arb_y.m_arb, mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));

        arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));
    }
}

void arb_log_base_ui(::mpfr_t rop, const ::mpfr_t op, unsigned long b)
{
    MPPP_MAYBE_TLS arb_raii arb_rop, arb_op;

    mpfr_to_arb(arb_op.m_arb, op);

    ::arb_log_base_ui(arb_rop.m_arb, arb_op.m_arb, safe_cast<::ulong>(b), mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));

    arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));
}

MPPP_UNARY_ARB_WRAPPER(sin_pi)
MPPP_UNARY_ARB_WRAPPER(cos_pi)
MPPP_UNARY_ARB_WRAPPER(tan_pi)
MPPP_UNARY_ARB_WRAPPER(cot_pi)
MPPP_UNARY_ARB_WRAPPER(sinc)

// NOTE: sinc_pi needs special handling for certain
// input values.
void arb_sinc_pi(::mpfr_t rop, const ::mpfr_t op)
{
    // The Arb function does not seem to handle
    // well infs or nans.
    if (mpfr_inf_p(op)) {
        ::mpfr_set_zero(rop, 1);
    } else if (mpfr_nan_p(op)) {
        ::mpfr_set_nan(rop);
    } else {
        MPPP_MAYBE_TLS arb_raii arb_rop, arb_op;
        mpfr_to_arb(arb_op.m_arb, op);

        ::arb_sinc_pi(arb_rop.m_arb, arb_op.m_arb, mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));

        arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));
    }
}

// NOTE: arb_hypgeom_bessel_j/y need special casing to handle
// positive infinity.
void arb_hypgeom_bessel_j(::mpfr_t rop, const ::mpfr_t op1, const ::mpfr_t op2)
{
    if (::mpfr_number_p(op1) != 0 && mpfr_inf_p(op2) != 0 && mpfr_sgn(op2) > 0) {
        // jx(nu, +infty) is zero for every finite nu.
        ::mpfr_set_zero(rop, 1);
    } else {
        MPPP_MAYBE_TLS arb_raii arb_rop, arb_op1, arb_op2;

        mpfr_to_arb(arb_op1.m_arb, op1);
        mpfr_to_arb(arb_op2.m_arb, op2);

        ::arb_hypgeom_bessel_j(arb_rop.m_arb, arb_op1.m_arb, arb_op2.m_arb, mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));

        arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));
    }
}

void arb_hypgeom_bessel_y(::mpfr_t rop, const ::mpfr_t op1, const ::mpfr_t op2)
{
    if (::mpfr_number_p(op1) != 0 && mpfr_inf_p(op2) != 0 && mpfr_sgn(op2) > 0) {
        // yx(nu, +infty) is zero for every finite nu.
        ::mpfr_set_zero(rop, 1);
    } else {
        MPPP_MAYBE_TLS arb_raii arb_rop, arb_op1, arb_op2;

        mpfr_to_arb(arb_op1.m_arb, op1);
        mpfr_to_arb(arb_op2.m_arb, op2);

        ::arb_hypgeom_bessel_y(arb_rop.m_arb, arb_op1.m_arb, arb_op2.m_arb, mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));

        arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));
    }
}

// NOTE: the Lambert W wrappers are needed because Arb has a single
// function for both w0 and wm1.
void arb_lambert_w0(::mpfr_t rop, const ::mpfr_t op)
{
    MPPP_MAYBE_TLS arb_raii arb_rop, arb_op;

    mpfr_to_arb(arb_op.m_arb, op);

    ::arb_lambertw(arb_rop.m_arb, arb_op.m_arb, 0, mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));

    arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));
}

void arb_lambert_wm1(::mpfr_t rop, const ::mpfr_t op)
{
    MPPP_MAYBE_TLS arb_raii arb_rop, arb_op;

    mpfr_to_arb(arb_op.m_arb, op);

    ::arb_lambertw(arb_rop.m_arb, arb_op.m_arb, 1, mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));

    arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));
}

// NOTE: special handling for polylog with integral order.
void arb_polylog_si(::mpfr_t rop, long n, const ::mpfr_t op)
{
    MPPP_MAYBE_TLS arb_raii arb_rop, arb_op;

    mpfr_to_arb(arb_op.m_arb, op);

    ::arb_polylog_si(arb_rop.m_arb, safe_cast<::slong>(n), arb_op.m_arb, mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));

    arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));
}

// NOTE: we don't have a generic binary wrapper (yet).
void arb_polylog(::mpfr_t rop, const ::mpfr_t op1, const ::mpfr_t op2)
{
    MPPP_MAYBE_TLS arb_raii arb_rop, arb_op1, arb_op2;

    mpfr_to_arb(arb_op1.m_arb, op1);
    mpfr_to_arb(arb_op2.m_arb, op2);

    ::arb_polylog(arb_rop.m_arb, arb_op1.m_arb, arb_op2.m_arb, mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));

    arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));
}

#undef MPPP_UNARY_ARB_WRAPPER

#if defined(MPPP_WITH_MPC)

// Helper for the implementation of unary Acb wrappers.
// NOTE: it would probably pay off to put a bunch of thread-local acb_raii
// objects in the unnamed namespace above, and use those, instead of function-local
// statics, to convert to/from MPC. However such a scheme would not work
// on MinGW due to the thread_local issues, so as long as we support MinGW
// (or any platform with non-functional thread_local), we cannot adopt this approach.
#define MPPP_UNARY_ACB_WRAPPER(fname)                                                                                  \
    void acb_##fname(::mpc_t rop, const ::mpc_t op)                                                                    \
    {                                                                                                                  \
        MPPP_MAYBE_TLS acb_raii acb_rop, acb_op;                                                                       \
        /* Turn op into an acb. */                                                                                     \
        mpc_to_acb(acb_op.m_acb, op);                                                                                  \
        /* Run the computation, using the precision of rop to mimic */                                                 \
        /* the behaviour of MPC functions. */                                                                          \
        /* NOTE: the precision of rop is taken from its real part, */                                                  \
        /* as done in the complex class. */                                                                            \
        ::acb_##fname(acb_rop.m_acb, acb_op.m_acb, mpfr_prec_to_arb_prec(mpfr_get_prec(mpc_realref(rop))));            \
        /* Write the result into rop. */                                                                               \
        acb_to_mpc(rop, acb_rop.m_acb);                                                                                \
    }

// inv() needs special handling for some arguments.
void acb_inv(::mpc_t rop, const ::mpc_t op)
{
    // NOTE: follow Annex G of the C standard:
    // http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf
    if (mpc_is_inf(op)) {
        // 1/inf results in a zero.
        ::mpfr_set_zero(mpc_realref(rop), 0);
        ::mpfr_set_zero(mpc_imagref(rop), 0);
    } else if (mpc_is_zero(op)) {
        // 1/0 results in an infinity.
        ::mpfr_set_inf(mpc_realref(rop), 0);
    } else {
        MPPP_MAYBE_TLS acb_raii acb_rop, acb_op;

        mpc_to_acb(acb_op.m_acb, op);

        ::acb_inv(acb_rop.m_acb, acb_op.m_acb, mpfr_prec_to_arb_prec(mpfr_get_prec(mpc_realref(rop))));

        acb_to_mpc(rop, acb_rop.m_acb);
    }
}

// rec_sqrt() needs special handling for some arguments.
void acb_rec_sqrt(::mpc_t rop, const ::mpc_t op)
{
    // NOTE: follow Annex G of the C standard:
    // http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf
    if (mpc_is_inf(op)) {
        // 1/sqrt(inf) results in a zero.
        ::mpfr_set_zero(mpc_realref(rop), 0);
        ::mpfr_set_zero(mpc_imagref(rop), 0);
    } else if (mpc_is_zero(op)) {
        // 1/sqrt(0) results in an infinity.
        ::mpfr_set_inf(mpc_realref(rop), 0);
    } else {
        MPPP_MAYBE_TLS acb_raii acb_rop, acb_op;

        mpc_to_acb(acb_op.m_acb, op);

        ::acb_rsqrt(acb_rop.m_acb, acb_op.m_acb, mpfr_prec_to_arb_prec(mpfr_get_prec(mpc_realref(rop))));

        acb_to_mpc(rop, acb_rop.m_acb);
    }
}

// rootn_ui.
void acb_rootn_ui(::mpc_t rop, const ::mpc_t op, unsigned long n)
{
    if (n == 0u) {
        // NOTE: like the MPFR function, set rop
        // to nan if n is zero.
        ::mpfr_set_nan(mpc_realref(rop));
        ::mpfr_set_nan(mpc_imagref(rop));
    } else if (mpc_is_inf(op)) {
        // inf**(1/n) results in an infinity.
        ::mpfr_set_inf(mpc_realref(rop), 0);
    } else {
        MPPP_MAYBE_TLS acb_raii acb_rop, acb_op;

        mpc_to_acb(acb_op.m_acb, op);

        ::acb_root_ui(acb_rop.m_acb, acb_op.m_acb, safe_cast<::ulong>(n),
                      mpfr_prec_to_arb_prec(mpfr_get_prec(mpc_realref(rop))));

        acb_to_mpc(rop, acb_rop.m_acb);
    }
}

MPPP_UNARY_ACB_WRAPPER(agm1)

#if defined(MPPP_ARB_HAVE_ACB_AGM)

void acb_agm(::mpc_t rop, const ::mpc_t op1, const ::mpc_t op2)
{
    MPPP_MAYBE_TLS acb_raii acb_rop, acb_op1, acb_op2;

    mpc_to_acb(acb_op1.m_acb, op1);
    mpc_to_acb(acb_op2.m_acb, op2);

    ::acb_agm(acb_rop.m_acb, acb_op1.m_acb, acb_op2.m_acb, mpfr_prec_to_arb_prec(mpfr_get_prec(mpc_realref(rop))));

    acb_to_mpc(rop, acb_rop.m_acb);
}

#endif

#undef MPPP_UNARY_ACB_WRAPPER

#endif

} // namespace detail

// In-place sqrt1pm1.
real &real::sqrt1pm1()
{
    return self_mpfr_unary_nornd(detail::arb_sqrt1pm1);
}

// In-place sin_pi.
real &real::sin_pi()
{
    return self_mpfr_unary_nornd(detail::arb_sin_pi);
}

// In-place cos_pi.
real &real::cos_pi()
{
    return self_mpfr_unary_nornd(detail::arb_cos_pi);
}

// In-place tan_pi.
real &real::tan_pi()
{
    return self_mpfr_unary_nornd(detail::arb_tan_pi);
}

// In-place cot_pi.
real &real::cot_pi()
{
    return self_mpfr_unary_nornd(detail::arb_cot_pi);
}

// In-place sinc.
real &real::sinc()
{
    return self_mpfr_unary_nornd(detail::arb_sinc);
}

// In-place sinc_pi.
real &real::sinc_pi()
{
    return self_mpfr_unary_nornd(detail::arb_sinc_pi);
}

// In-place Lambert W functions.
real &real::lambert_w0()
{
    return self_mpfr_unary_nornd(detail::arb_lambert_w0);
}

real &real::lambert_wm1()
{
    return self_mpfr_unary_nornd(detail::arb_lambert_wm1);
}

#if defined(MPPP_WITH_MPC)

// In-place inversion.
complex &complex::inv()
{
    return self_mpc_unary_nornd(detail::acb_inv);
}

// In-place reciprocal sqrt.
complex &complex::rec_sqrt()
{
    return self_mpc_unary_nornd(detail::acb_rec_sqrt);
}

// AGM.
complex &complex::agm1()
{
    return self_mpc_unary_nornd(detail::acb_agm1);
}

#endif

} // namespace mppp
