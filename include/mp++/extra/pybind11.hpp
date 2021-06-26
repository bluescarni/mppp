// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_EXTRA_PYBIND11_HPP
#define MPPP_EXTRA_PYBIND11_HPP

#if defined(__clang__) || defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

#endif

#include <Python.h>
#include <pybind11/pybind11.h>

#if PY_MAJOR_VERSION < 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION < 7)

#error Only Python >= 2.7 is supported.

#endif

#if PY_MAJOR_VERSION == 2

// This includes the definition of the long integer structure
// in Python 2.x.
#include <longintrepr.h>

#endif

#include <mp++/mp++.hpp>

#include <cassert>
#include <cstddef>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace mppp_pybind11
{

namespace py = pybind11;

namespace detail
{

template <typename = void>
struct globals_ {
    static bool inited;
    static std::unique_ptr<py::module> mpmath;
    static std::unique_ptr<py::object> mpmath_mp;
    static std::unique_ptr<py::object> mpf_class;
    static std::unique_ptr<py::object> mpc_class;
    static std::unique_ptr<py::object> mpf_isinf;
    static std::unique_ptr<py::object> mpf_isnan;
    static std::unique_ptr<py::object> fraction_class;
    static std::unique_ptr<py::int_> gmp_numb_bits;
};

template <typename T>
bool globals_<T>::inited = false;

template <typename T>
std::unique_ptr<py::module> globals_<T>::mpmath;

template <typename T>
std::unique_ptr<py::object> globals_<T>::mpmath_mp;

template <typename T>
std::unique_ptr<py::object> globals_<T>::mpf_class;

template <typename T>
std::unique_ptr<py::object> globals_<T>::mpc_class;

template <typename T>
std::unique_ptr<py::object> globals_<T>::mpf_isinf;

template <typename T>
std::unique_ptr<py::object> globals_<T>::mpf_isnan;

template <typename T>
std::unique_ptr<py::object> globals_<T>::fraction_class;

template <typename T>
std::unique_ptr<py::int_> globals_<T>::gmp_numb_bits;

using globals = globals_<>;

// Cleanup function to clear global variables
inline void cleanup()
{
#if !defined(NDEBUG)
    std::cout << "Cleaning up mppp_pybind11 data." << std::endl;
#endif
    globals::mpmath.reset();
    globals::mpmath_mp.reset();
    globals::mpf_class.reset();
    globals::mpc_class.reset();
    globals::mpf_isinf.reset();
    globals::mpf_isnan.reset();
    globals::fraction_class.reset();
    globals::gmp_numb_bits.reset();
}
} // namespace detail

// Initialisation function for the pybind11 integration.
inline void init()
{
    if (detail::globals::inited) {
        // Don't do anything if we inited already.
        return;
    }

    // Small helper to clean up if something goes wrong
    // during initialisation.
    struct auto_cleaner {
        ~auto_cleaner()
        {
            if (m_armed) {
                detail::cleanup();
            }
        }
        bool m_armed = true;
    };
    auto_cleaner ac;

    // Register the cleanup function.
    // https://github.com/pybind/pybind11/pull/1169
    py::module::import("atexit").attr("register")(py::cpp_function(detail::cleanup));

    // GMP bits setup.
    detail::globals::gmp_numb_bits.reset(new py::int_(GMP_NUMB_BITS));

    // Detect and import mpmath bits.
    py::module mpmath_mod;
    bool have_mpmath = false;
    try {
        mpmath_mod = py::module::import("mpmath");
        detail::globals::mpmath.reset(new py::module(mpmath_mod));
        have_mpmath = true;
    } catch (py::error_already_set &eas) {
        // NOTE: this will restore the current Python error flags,
        // and set them to null in eas. When eas is destroyed,
        // no further action will be taken as all its members are null.
        eas.restore();
        assert(::PyErr_Occurred() != nullptr);
        if (::PyErr_ExceptionMatches(::PyExc_ImportError)) {
            // On ImportError, we just clear all flags and move on.
            ::PyErr_Clear();
        } else {
            // If the error was something else, we re-throw it be throwing
            // a new pybind exception.
            throw py::error_already_set();
        }
    }
    if (have_mpmath) {
        auto mpmath_mp = mpmath_mod.attr("mp");
        detail::globals::mpmath_mp.reset(new py::object(std::move(mpmath_mp)));
        auto mpf_class = mpmath_mod.attr("mpf");
        detail::globals::mpf_class.reset(new py::object(std::move(mpf_class)));
        auto mpc_class = mpmath_mod.attr("mpc");
        detail::globals::mpc_class.reset(new py::object(std::move(mpc_class)));
        auto mpf_isinf = mpmath_mod.attr("isinf");
        detail::globals::mpf_isinf.reset(new py::object(std::move(mpf_isinf)));
        auto mpf_isnan = mpmath_mod.attr("isnan");
        detail::globals::mpf_isnan.reset(new py::object(std::move(mpf_isnan)));
    }

    // The fraction class.
    auto fraction_class = py::module::import("fractions").attr("Fraction");
    detail::globals::fraction_class.reset(new py::object(std::move(fraction_class)));

    // Tanslation for mp++ exceptions.
    py::register_exception_translator([](std::exception_ptr p) {
        try {
            if (p) {
                std::rethrow_exception(p);
            }
        } catch (const mppp::zero_division_error &e) {
            ::PyErr_SetString(::PyExc_ZeroDivisionError, e.what());
        }
    });

    // Mark as inited.
    detail::globals::inited = true;

    // Disarm the auto cleaner.
    ac.m_armed = false;
}

namespace detail
{

// Convert a Python long object to an mppp integer.
template <std::size_t SSize>
inline mppp::integer<SSize> py_long_to_mppp_int(const ::PyLongObject *nptr)
{
    // Get its signed size.
    const auto ob_size =
#if PY_MAJOR_VERSION == 2
        nptr->ob_size;
#else
        nptr->ob_base.ob_size;
#endif
    if (!ob_size) {
        // The Python integer is zero, return a def cted value.
        return mppp::integer<SSize>{};
    }
    // Get the limbs array.
    const auto ob_digit = nptr->ob_digit;
    const bool neg = ob_size < 0;
    using size_type = typename std::make_unsigned<typename std::remove_const<decltype(ob_size)>::type>::type;
    auto abs_ob_size = neg ? mppp::detail::nint_abs(ob_size) : static_cast<size_type>(ob_size);
    static_assert(PyLong_SHIFT <= std::numeric_limits<::mp_bitcnt_t>::max(), "Overflow error.");
    if (mppp_unlikely(static_cast<::mp_bitcnt_t>(PyLong_SHIFT)
                      > std::numeric_limits<::mp_bitcnt_t>::max() / abs_ob_size)) {
        throw std::overflow_error("Overflow in the computation of the size of a Python integer");
    }
    const auto nbits = static_cast<::mp_bitcnt_t>(static_cast<::mp_bitcnt_t>(PyLong_SHIFT) * abs_ob_size);
    // Construct the retval with the necessary number of bits.
    mppp::integer<SSize> retval{mppp::integer_bitcnt_t(nbits)};
    // Init it with the first digit. The Python integer is nonzero, so this is safe.
    retval = ob_digit[--abs_ob_size];
    // Add the remaining limbs.
    while (abs_ob_size) {
        // NOTE: a possible optimisation here is to unroll the shifting in order to
        // reduce the loop iterations.
        retval <<= PyLong_SHIFT;
        retval += ob_digit[--abs_ob_size];
    }
    // Negate if necessary.
    if (neg) {
        retval.neg();
    }
    return retval;
}

// Convert a python integer to an mppp integer.
template <std::size_t SSize>
inline bool py_integer_to_mppp_int(mppp::integer<SSize> &rop, const ::PyObject *source)
{
#if PY_MAJOR_VERSION == 2
    const bool is_int = PyInt_Check(source), is_long = PyLong_Check(source);
    if (!is_int && !is_long) {
        return false;
    }
    if (is_long) {
        assert(!is_int);
        rop = py_long_to_mppp_int<SSize>((const ::PyLongObject *)source);
        return true;
    }
    assert(is_int);
    rop = ((const ::PyIntObject *)source)->ob_ival;
    return true;
#else
    if (!PyLong_Check(source)) {
        return false;
    }
    rop = py_long_to_mppp_int<SSize>((const ::PyLongObject *)source);
    return true;
#endif
}

// Convert mppp integer to a python integer.
// NOTE: this can be improved via the direct C API at least in the following ways:
// - pybind11 always stores long ints in int_, for Python 2 this is suboptimal,
// - using the number protocol, https://docs.python.org/3.6/c-api/number.html,
//   may be more efficient that calling int_ methods via pybind11.
template <std::size_t SSize>
inline py::int_ mppp_int_to_py(const mppp::integer<SSize> &src)
{
    if (src.is_zero()) {
        // Just return a def-cted int for zero.
        return py::int_();
    }
    // Get a pointer to the limbs.
    const ::mp_limb_t *ptr = src.is_static() ? src._get_union().g_st().m_limbs.data() : src._get_union().g_dy()._mp_d;
    // Get the size.
    auto size = src.size();
    assert(size);
    // Init the retval.
    py::int_ retval(ptr[--size] & GMP_NUMB_MASK);
    // Add the rest of the limbs arithmetically.
    while (size) {
        retval = retval.attr("__lshift__")(*mppp_pybind11::detail::globals::gmp_numb_bits)
                     .attr("__add__")(py::int_(ptr[--size] & GMP_NUMB_MASK));
    }
    // Negate if needed.
    if (src.sgn() < 0) {
        retval = retval.attr("__neg__")();
    }
    return retval;
}

#if defined(MPPP_WITH_QUADMATH)

inline bool py_handle_to_real128(mppp::real128 &value, py::handle src)
{
    if (!globals::mpmath || !::PyObject_IsInstance(src.ptr(), globals::mpf_class->ptr())) {
        return false;
    }
    const auto prec = src.attr("context").attr("prec").cast<decltype(mppp::real128_sig_digits())>();
    if (prec != mppp::real128_sig_digits()) {
        return false;
    }
    const auto mpf_tuple = src.attr("_mpf_").cast<py::tuple>();
    auto neg_if_needed = [&value, &mpf_tuple]() {
        if (mpf_tuple[0].cast<int>()) {
            value = -value;
        }
    };
    if ((*globals::mpf_isinf)(src).cast<bool>()) {
        value = mppp::real128_inf();
        neg_if_needed();
    } else if ((*globals::mpf_isnan)(src).cast<bool>()) {
        value = mppp::real128_nan();
    } else {
        mppp::integer<1> sig;
        if (!py_integer_to_mppp_int(sig, py::int_(mpf_tuple[1]).ptr())) {
            throw std::runtime_error("Could not interpret the significand of an mpf value as an integer object");
        }
        // NOTE: we have to be careful here. We might have a very large significand
        // coming in from the _mpf_ tuple (i.e., very high precision), even if the final value
        // is small. If the significand is larger than 113 bits, then we will scale it down
        // and adjust the exponent accordingly.
        auto exp = mppp::integer<1>{mpf_tuple[2].cast<long>()};
        const auto sig_nbits = sig.nbits();
        if (sig_nbits > mppp::real128_sig_digits()) {
            sig >>= (sig_nbits - mppp::real128_sig_digits());
            exp += (sig_nbits - mppp::real128_sig_digits());
        }
        // Now we can set safely the real128 to the significand.
        value = sig;
        // Offset by the (possibly adjusted) exponent.
        value = scalbln(value, static_cast<long>(exp));
        neg_if_needed();
    }
    return true;
}

inline bool py_handle_to_complex128(mppp::complex128 &value, py::handle src)
{
    if (!globals::mpmath || !::PyObject_IsInstance(src.ptr(), globals::mpc_class->ptr())) {
        return false;
    }

    mppp::real128 re, im;

    if (py_handle_to_real128(re, src.attr("real")) && py_handle_to_real128(im, src.attr("imag"))) {
        value.set_real(re);
        value.set_imag(im);

        return true;
    } else {
        return false;
    }
}

inline py::object real128_to_py_object(const mppp::real128 &src)
{
    if (!globals::mpmath) {
        throw std::runtime_error("Cannot convert a real128 to an mpf if mpmath is not available");
    }
    const auto prec = globals::mpmath_mp->attr("prec").cast<decltype(mppp::real128_sig_digits())>();
    if (prec != mppp::real128_sig_digits()) {
        throw std::invalid_argument(
            "Cannot convert the real128 " + src.to_string() + " to an mpf: the precision of real128 ("
            + std::to_string(mppp::real128_sig_digits()) + ") is different from the current mpf precision ("
            + std::to_string(prec) + "). Please change the current mpf precision to "
            + std::to_string(mppp::real128_sig_digits()) + " in order to avoid this error");
    }
    if (isinf(src)) {
        if (std::numeric_limits<double>::has_infinity) {
            return (*globals::mpf_class)(src > 0 ? std::numeric_limits<double>::infinity()
                                                 : -std::numeric_limits<double>::infinity());
        } else {
            return (*globals::mpf_class)(src > 0 ? "inf" : "-inf");
        }
    }
    if (isnan(src)) {
        if (std::numeric_limits<double>::has_quiet_NaN) {
            return (*globals::mpf_class)(std::numeric_limits<double>::quiet_NaN());
        } else {
            return (*globals::mpf_class)("nan");
        }
    }
    int exp;
    const auto fr = scalbln(frexp(src, &exp), mppp::detail::safe_cast<long>(mppp::real128_sig_digits()));
    return (*globals::mpf_class)(py::make_tuple(mppp_int_to_py(mppp::integer<1>(fr)),
                                                static_cast<long>(mppp::integer<1>{exp} - mppp::real128_sig_digits())));
}

inline py::handle real128_to_py_handle(const mppp::real128 &src)
{
    return real128_to_py_object(src).release();
}

inline py::handle complex128_to_py_handle(const mppp::complex128 &src)
{
    auto re = real128_to_py_object(src.real());
    auto im = real128_to_py_object(src.imag());

    return (*globals::mpc_class)(re, im).release();
}

#endif

#if defined(MPPP_WITH_MPFR)

inline bool py_handle_to_real(mppp::real &value, py::handle src)
{
    if (!globals::mpmath || !::PyObject_IsInstance(src.ptr(), globals::mpf_class->ptr())) {
        return false;
    }
    const auto prec = src.attr("context").attr("prec").cast<::mpfr_prec_t>();
    value.set_prec(prec);
    // NOTE: the _mpf_ tuple contains three elements:
    // - the sign of the mpf,
    // - an integral value n,
    // - an exponent e such that n*2**e equals the mpf.
    const auto mpf_tuple = src.attr("_mpf_").cast<py::tuple>();
    // Little helper to negate the value, depending on the sign in the tuple.
    auto neg_if_needed = [&value, &mpf_tuple]() {
        if (mpf_tuple[0].cast<int>()) {
            value.neg();
        }
    };
    if ((*globals::mpf_isinf)(src).cast<bool>()) {
        // Handle inf.
        set_inf(value);
        neg_if_needed();
    } else if ((*globals::mpf_isnan)(src).cast<bool>()) {
        // Handle NaN.
        set_nan(value);
    } else {
        // Normal value.
        mppp::integer<1> sig;
        // NOTE: the tuple returned by _mpf_ might contain an mpz from gmpy, instead of a Python integer. Thus, we
        // need to convert to a Python integer in order to be extra sure we pass an object of the correct type
        // to py_integer_to_mppp_int().
        // NOTE: we use the explicit pybind11::int_ name because apparently there are
        // shadowing problems when including other pybind11 header. See the reference to a
        // similar problem here (in our case the issue came from including pybind11/stl_bind.h):
        // https://github.com/pybind/pybind11/issues/352
        if (!py_integer_to_mppp_int(sig, pybind11::int_(mpf_tuple[1]).ptr())) {
            throw std::runtime_error("Could not interpret the significand of an mpf value as an integer object");
        }
        set_z_2exp(value, sig, mpf_tuple[2].cast<::mpfr_exp_t>());
        neg_if_needed();
    }
    return true;
}

inline py::object real_to_py_object(const mppp::real &src)
{
    if (!globals::mpmath) {
        throw std::runtime_error("Cannot convert a real to an mpf if mpmath is not available");
    }
    const auto prec = globals::mpmath_mp->attr("prec").cast<::mpfr_prec_t>();
    const auto src_prec = src.get_prec();
    if (prec < src_prec) {
        throw std::invalid_argument("Cannot convert the real " + src.to_string()
                                    + " to an mpf: the precision of the real (" + std::to_string(src_prec)
                                    + ") is greater than the current mpf precision (" + std::to_string(prec)
                                    + "). Please increase the current mpf precision to at least "
                                    + std::to_string(src_prec) + " in order to avoid this error");
    }
    // Handle special values first.
    if (src.inf_p()) {
        if (std::numeric_limits<double>::has_infinity) {
            return (*globals::mpf_class)(src.sgn() > 0 ? std::numeric_limits<double>::infinity()
                                                       : -std::numeric_limits<double>::infinity());
        } else {
            return (*globals::mpf_class)(src.sgn() > 0 ? "inf" : "-inf");
        }
    }
    if (src.nan_p()) {
        if (std::numeric_limits<double>::has_quiet_NaN) {
            return (*globals::mpf_class)(std::numeric_limits<double>::quiet_NaN());
        } else {
            return (*globals::mpf_class)("nan");
        }
    }
    mppp::integer<1> tmp;
    // NOTE: this function will run internal checks on overflow.
    const auto exp = get_z_2exp(tmp, src);
    return (*globals::mpf_class)(py::make_tuple(mppp_int_to_py(tmp), exp));
}

inline py::handle real_to_py_handle(const mppp::real &src)
{
    return real_to_py_object(src).release();
}

#endif

#if defined(MPPP_WITH_MPC)

inline bool py_handle_to_complex(mppp::complex &value, py::handle src)
{
    if (!globals::mpmath || !::PyObject_IsInstance(src.ptr(), globals::mpc_class->ptr())) {
        return false;
    }

    mppp::real re, im;

    if (py_handle_to_real(re, src.attr("real")) && py_handle_to_real(im, src.attr("imag"))) {
        value = mppp::complex{std::move(re), std::move(im)};

        return true;
    } else {
        return false;
    }
}

inline py::handle complex_to_py_handle(const mppp::complex &src)
{
    mppp::complex::re_cref r{src};
    mppp::complex::im_cref i{src};

    auto re = real_to_py_object(*r);
    auto im = real_to_py_object(*i);

    return (*globals::mpc_class)(re, im).release();
}

#endif

} // namespace detail

} // namespace mppp_pybind11

namespace pybind11
{
namespace detail
{
template <std::size_t SSize>
struct type_caster<mppp::integer<SSize>> {
    PYBIND11_TYPE_CASTER(mppp::integer<SSize>, _("mppp::integer<") + _<SSize>() + _(">"));
    bool load(handle src, bool)
    {
        return mppp_pybind11::detail::py_integer_to_mppp_int(value, src.ptr());
    }
    static handle cast(const mppp::integer<SSize> &src, return_value_policy, handle)
    {
        return mppp_pybind11::detail::mppp_int_to_py(src).release();
    }
};

template <std::size_t SSize>
struct type_caster<mppp::rational<SSize>> {
    PYBIND11_TYPE_CASTER(mppp::rational<SSize>, _("mppp::rational<") + _<SSize>() + _(">"));
    bool load(handle src, bool)
    {
        if (!::PyObject_IsInstance(src.ptr(), mppp_pybind11::detail::globals::fraction_class->ptr())) {
            return false;
        }
        mppp::integer<SSize> num, den;
        if (!mppp_pybind11::detail::py_integer_to_mppp_int(num, src.attr("numerator").ptr())
            || !mppp_pybind11::detail::py_integer_to_mppp_int(den, src.attr("denominator").ptr())) {
            throw std::runtime_error(
                "Could not interpret the numerator/denominator of a Python fraction as integer objects");
        }
        value = mppp::rational<SSize>{std::move(num), std::move(den), false};
        return true;
    }
    static handle cast(const mppp::rational<SSize> &src, return_value_policy, handle)
    {
        return (*mppp_pybind11::detail::globals::fraction_class)(mppp_pybind11::detail::mppp_int_to_py(src.get_num()),
                                                                 mppp_pybind11::detail::mppp_int_to_py(src.get_den()))
            .release();
    }
};

#if defined(MPPP_WITH_MPFR)

template <>
struct type_caster<mppp::real> {
    PYBIND11_TYPE_CASTER(mppp::real, _("mppp::real"));
    bool load(handle src, bool)
    {
        return mppp_pybind11::detail::py_handle_to_real(value, src);
    }
    static handle cast(const mppp::real &src, return_value_policy, handle)
    {
        return mppp_pybind11::detail::real_to_py_handle(src);
    }
};

#endif

#if defined(MPPP_WITH_MPC)

template <>
struct type_caster<mppp::complex> {
    PYBIND11_TYPE_CASTER(mppp::complex, _("mppp::complex"));
    bool load(handle src, bool)
    {
        return mppp_pybind11::detail::py_handle_to_complex(value, src);
    }
    static handle cast(const mppp::complex &src, return_value_policy, handle)
    {
        return mppp_pybind11::detail::complex_to_py_handle(src);
    }
};

#endif

#if defined(MPPP_WITH_QUADMATH)

template <>
struct type_caster<mppp::real128> {
    PYBIND11_TYPE_CASTER(mppp::real128, _("mppp::real128"));
    bool load(handle src, bool)
    {
        return mppp_pybind11::detail::py_handle_to_real128(value, src);
    }
    static handle cast(const mppp::real128 &src, return_value_policy, handle)
    {
        return mppp_pybind11::detail::real128_to_py_handle(src);
    }
};

template <>
struct type_caster<mppp::complex128> {
    PYBIND11_TYPE_CASTER(mppp::complex128, _("mppp::complex128"));
    bool load(handle src, bool)
    {
        return mppp_pybind11::detail::py_handle_to_complex128(value, src);
    }
    static handle cast(const mppp::complex128 &src, return_value_policy, handle)
    {
        return mppp_pybind11::detail::complex128_to_py_handle(src);
    }
};

#endif

} // namespace detail

} // namespace pybind11

#if defined(__clang__) || defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

#endif
