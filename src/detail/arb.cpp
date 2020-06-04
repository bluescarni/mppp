// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>

#include <flint/flint.h>
#include <flint/fmpz.h>

#include <arb.h>
#include <arb_hypgeom.h>
#include <arf.h>
#include <mag.h>

#include <mp++/config.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/real.hpp>

namespace mppp
{

namespace detail
{

namespace
{

// Minimal RAII struct to hold
// arb_t types.
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

// Minimal RAII struct to hold
// arf_t types.
struct arf_raii {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    arf_raii()
    {
        ::arf_init(m_arf);
    }
    arf_raii(const arf_raii &) = delete;
    arf_raii(arf_raii &&) = delete;
    arf_raii &operator=(const arf_raii &) = delete;
    arf_raii &operator=(arf_raii &&) = delete;
    ~arf_raii()
    {
        ::arf_clear(m_arf);
    }
    ::arf_t m_arf;
};

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

MPPP_UNARY_ARB_WRAPPER(sin_pi)
MPPP_UNARY_ARB_WRAPPER(cos_pi)

// NOTE: tan_pi needs special handling for certain
// input values.
void arb_tan_pi(::mpfr_t rop, const ::mpfr_t op)
{
    MPPP_MAYBE_TLS arb_raii arb_op;
    mpfr_to_arb(arb_op.m_arb, op);

    // If op is exactly n/2 (with n an odd integer),
    // the Arb function will return nan rather than +-inf.
    // Handle this case specially.
    if (::arf_is_int(arb_midref(arb_op.m_arb)) == 0 && ::arf_is_int_2exp_si(arb_midref(arb_op.m_arb), -1) != 0) {
        MPPP_MAYBE_TLS arf_raii arf_tmp;

        // The strategy is to truncate op and then, based
        // on the parity of the result, return +inf or -inf.
        // Because Arb does not have a truncation primitive,
        // we need to use floor/ceil depending on the sign
        // of op.
        if (::arf_sgn(arb_midref(arb_op.m_arb)) == 1) {
            // op > 0.
            ::arf_floor(arf_tmp.m_arf, arb_midref(arb_op.m_arb));
            ::mpfr_set_inf(rop, ::arf_is_int_2exp_si(arf_tmp.m_arf, 1) != 0 ? 1 : -1);
        } else {
            // op < 0.
            assert(::arf_sgn(arb_midref(arb_op.m_arb)) == -1);
            ::arf_ceil(arf_tmp.m_arf, arb_midref(arb_op.m_arb));
            ::mpfr_set_inf(rop, ::arf_is_int_2exp_si(arf_tmp.m_arf, 1) != 0 ? -1 : 1);
        }
    } else {
        MPPP_MAYBE_TLS arb_raii arb_rop;

        ::arb_tan_pi(arb_rop.m_arb, arb_op.m_arb, mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));

        arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));
    }
}

// NOTE: cot_pi needs special handling for certain
// input values.
void arb_cot_pi(::mpfr_t rop, const ::mpfr_t op)
{
    MPPP_MAYBE_TLS arb_raii arb_rop, arb_op;
    mpfr_to_arb(arb_op.m_arb, op);

    // If op is exactly n (with n an integer),
    // the Arb function will return nan rather than +-inf.
    // Handle this case specially.
    if (::arf_is_int(arb_midref(arb_op.m_arb)) != 0) {
        ::mpfr_set_inf(rop, ::arf_is_int_2exp_si(arb_midref(arb_op.m_arb), 1) != 0 ? 1 : -1);
    } else {
        ::arb_cot_pi(arb_rop.m_arb, arb_op.m_arb, mpfr_prec_to_arb_prec(mpfr_get_prec(rop)));

        arf_to_mpfr(rop, arb_midref(arb_rop.m_arb));
    }
}

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

#undef MPPP_UNARY_ARB_WRAPPER

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

} // namespace mppp
