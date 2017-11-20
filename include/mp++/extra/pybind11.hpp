// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
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
#pragma GCC diagnostic ignored "-Wdeprecated"

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

inline namespace detail
{

template <typename = void>
struct globals_ {
    static bool inited;
    static std::unique_ptr<py::module> mpmath;
    static std::unique_ptr<py::object> mpmath_mp;
    static std::unique_ptr<py::object> mpf_class;
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
    globals::mpf_isinf.reset();
    globals::mpf_isnan.reset();
    globals::fraction_class.reset();
    globals::gmp_numb_bits.reset();
}
}

// Main initialisation function.
inline void init(py::module &m)
{
    if (globals::inited) {
        // Don't do anything if we inited already.
        return;
    }

    // Small helper to clean up if something goes wrong
    // during initialisation.
    struct auto_cleaner {
        ~auto_cleaner()
        {
            if (m_armed) {
                cleanup();
            }
        }
        bool m_armed = true;
    };
    auto_cleaner ac;

    // Expose and register the cleanup function.
    m.def("_mppp_pybind11_cleanup", cleanup);
    py::module::import("atexit").attr("register")(m.attr("_mppp_pybind11_cleanup"));

    // GMP bits setup.
    globals::gmp_numb_bits.reset(new py::int_(GMP_NUMB_BITS));

    // Detect and import mpmath bits.
    py::module mpmath_mod;
    bool have_mpmath = false;
    try {
        mpmath_mod = py::module::import("mpmath");
        globals::mpmath.reset(new py::module(mpmath_mod));
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
        globals::mpmath_mp.reset(new py::object(std::move(mpmath_mp)));
        auto mpf_class = mpmath_mod.attr("mpf");
        globals::mpf_class.reset(new py::object(std::move(mpf_class)));
        auto mpf_isinf = mpmath_mod.attr("isinf");
        globals::mpf_isinf.reset(new py::object(std::move(mpf_isinf)));
        auto mpf_isnan = mpmath_mod.attr("isnan");
        globals::mpf_isnan.reset(new py::object(std::move(mpf_isnan)));
    }

    // The fraction class.
    auto fraction_class = py::module::import("fractions").attr("Fraction");
    globals::fraction_class.reset(new py::object(std::move(fraction_class)));

    // Mark as inited.
    globals::inited = true;

    // Disarm the auto cleaner.
    ac.m_armed = false;
}

inline namespace detail
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
    auto abs_ob_size = neg ? mppp::nint_abs(ob_size) : static_cast<size_type>(ob_size);
    static_assert(PyLong_SHIFT <= std::numeric_limits<::mp_bitcnt_t>::max(), "Overflow error.");
    if (mppp_unlikely(static_cast<::mp_bitcnt_t>(PyLong_SHIFT)
                      > std::numeric_limits<::mp_bitcnt_t>::max() / abs_ob_size)) {
        throw std::overflow_error("Overflow in the computation of the size of a Python integer");
    }
    const auto nbits = static_cast<::mp_bitcnt_t>(static_cast<::mp_bitcnt_t>(PyLong_SHIFT) * abs_ob_size);
    // Construct the retval with the necessary number of bits.
    mppp::integer<SSize> retval{mppp::integer_nbits_init{}, nbits};
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
        retval = retval.attr("__lshift__")(*mppp_pybind11::globals::gmp_numb_bits)
                     .attr("__add__")(py::int_(ptr[--size] & GMP_NUMB_MASK));
    }
    // Negate if needed.
    if (src.sgn() < 0) {
        retval = retval.attr("__neg__")();
    }
    return retval;
}
}
}

namespace pybind11
{
namespace detail
{
template <std::size_t SSize>
struct type_caster<mppp::integer<SSize>> {
    PYBIND11_TYPE_CASTER(mppp::integer<SSize>, _("integer[") + _<SSize>() + _("]"));
    bool load(handle src, bool)
    {
        return mppp_pybind11::py_integer_to_mppp_int(value, src.ptr());
    }
    static handle cast(const mppp::integer<SSize> &src, return_value_policy, handle)
    {
        return mppp_pybind11::mppp_int_to_py(src).release();
    }
};

template <std::size_t SSize>
struct type_caster<mppp::rational<SSize>> {
    PYBIND11_TYPE_CASTER(mppp::rational<SSize>, _("rational[") + _<SSize>() + _("]"));
    bool load(handle src, bool)
    {
        if (!::PyObject_IsInstance(src.ptr(), mppp_pybind11::globals::fraction_class->ptr())) {
            return false;
        }
        mppp::integer<SSize> num, den;
        if (!mppp_pybind11::py_integer_to_mppp_int(num, src.attr("numerator").ptr())
            || !mppp_pybind11::py_integer_to_mppp_int(den, src.attr("denominator").ptr())) {
            throw std::runtime_error(
                "Could not interpret the numerator/denominator of a Python fraction as integer objects");
        }
        value = mppp::rational<SSize>{std::move(num), std::move(den), false};
        return true;
    }
    static handle cast(const mppp::rational<SSize> &src, return_value_policy, handle)
    {
        return (*mppp_pybind11::globals::fraction_class)(mppp_pybind11::mppp_int_to_py(src.get_num()),
                                                         mppp_pybind11::mppp_int_to_py(src.get_den()))
            .release();
    }
};

#if defined(MPPP_WITH_MPFR)

template <>
struct type_caster<mppp::real> {
    PYBIND11_TYPE_CASTER(mppp::real, _("real"));
    bool load(handle src, bool)
    {
        if (!mppp_pybind11::globals::mpmath
            || !::PyObject_IsInstance(src.ptr(), mppp_pybind11::globals::mpf_class->ptr())) {
            return false;
        }
        const auto prec = src.attr("context").attr("prec").cast<::mpfr_prec_t>();
        value.set_prec(prec);
        const auto mpf_tuple = src.attr("_mpf_").cast<tuple>();
        auto neg_if_needed = [this, &mpf_tuple]() {
            if (mpf_tuple[0].cast<int>()) {
                this->value.neg();
            }
        };
        if ((*mppp_pybind11::globals::mpf_isinf)(src).cast<bool>()) {
            set_inf(value);
            neg_if_needed();
        } else if ((*mppp_pybind11::globals::mpf_isnan)(src).cast<bool>()) {
            set_nan(value);
        } else {
            mppp::integer<1> sig;
            if (!mppp_pybind11::py_integer_to_mppp_int(sig, int_(mpf_tuple[1]).ptr())) {
                throw std::runtime_error("Could not interpret the significand of an mpf value as an integer object");
            }
            set_z_2exp(value, sig, mpf_tuple[2].cast<::mpfr_exp_t>());
            neg_if_needed();
        }
        return true;
    }
    static handle cast(const mppp::real &src, return_value_policy, handle)
    {
        if (!mppp_pybind11::globals::mpmath) {
            throw std::runtime_error("Cannot convert a real to an mpf if mpmath is not available");
        }
        if (src.inf_p()) {
            if (std::numeric_limits<double>::has_infinity) {
                return (*mppp_pybind11::globals::mpf_class)(src.sgn() > 0 ? std::numeric_limits<double>::infinity()
                                                                          : -std::numeric_limits<double>::infinity())
                    .release();
            } else {
                return (*mppp_pybind11::globals::mpf_class)(src.sgn() > 0 ? "inf" : "-inf").release();
            }
        }
        if (src.nan_p()) {
            if (std::numeric_limits<double>::has_quiet_NaN) {
                return (*mppp_pybind11::globals::mpf_class)(std::numeric_limits<double>::quiet_NaN()).release();
            } else {
                return (*mppp_pybind11::globals::mpf_class)("nan").release();
            }
        }
        const auto prec = mppp_pybind11::globals::mpmath_mp->attr("prec").cast<::mpfr_prec_t>();
        const auto src_prec = src.get_prec();
        if (prec < src_prec) {
            throw std::invalid_argument("Cannot convert the real " + src.to_string()
                                        + " to an mpf: the precision of the real (" + std::to_string(src_prec)
                                        + ") is smaller than the current mpf precision (" + std::to_string(prec)
                                        + "). Please increase the current mpf precision to at least "
                                        + std::to_string(src_prec) + " in order to avoid this error");
        }
        mppp::integer<1> tmp;
        const auto exp = get_z_2exp(tmp, src);
        return (*mppp_pybind11::globals::mpf_class)(make_tuple(mppp_pybind11::mppp_int_to_py(tmp), exp)).release();
    }
};

#endif

#if defined(MPPP_WITH_QUADMATH)

template <>
struct type_caster<mppp::real128> {
    PYBIND11_TYPE_CASTER(mppp::real128, _("real128"));
    bool load(handle src, bool)
    {
        if (!mppp_pybind11::globals::mpmath
            || !::PyObject_IsInstance(src.ptr(), mppp_pybind11::globals::mpf_class->ptr())) {
            return false;
        }
        const auto prec = src.attr("context").attr("prec").cast<::mpfr_prec_t>();
        if (prec != mppp::safe_cast<::mpfr_prec_t>(mppp::real128_sig_digits())) {
            return false;
        }
        const auto mpf_tuple = src.attr("_mpf_").cast<tuple>();
        auto neg_if_needed = [this, &mpf_tuple]() {
            if (mpf_tuple[0].cast<int>()) {
                this->value = -(this->value);
            }
        };
        if ((*mppp_pybind11::globals::mpf_isinf)(src).cast<bool>()) {
            value = mppp::real128_inf();
            neg_if_needed();
        } else if ((*mppp_pybind11::globals::mpf_isnan)(src).cast<bool>()) {
            value = mppp::real128_nan();
        } else {
            mppp::integer<1> sig;
            if (!mppp_pybind11::py_integer_to_mppp_int(sig, int_(mpf_tuple[1]).ptr())) {
                throw std::runtime_error("Could not interpret the significand of an mpf value as an integer object");
            }
            value = sig;
            value = scalbln(value, mpf_tuple[2].cast<long>());
            neg_if_needed();
        }
        return true;
    }
    static handle cast(const mppp::real128 &src, return_value_policy, handle)
    {
        if (!mppp_pybind11::globals::mpmath) {
            throw std::runtime_error("Cannot convert a real128 to an mpf if mpmath is not available");
        }
        if (isinf(src)) {
            if (std::numeric_limits<double>::has_infinity) {
                return (*mppp_pybind11::globals::mpf_class)(src > 0 ? std::numeric_limits<double>::infinity()
                                                                    : -std::numeric_limits<double>::infinity())
                    .release();
            } else {
                return (*mppp_pybind11::globals::mpf_class)(src > 0 ? "inf" : "-inf").release();
            }
        }
        if (isnan(src)) {
            if (std::numeric_limits<double>::has_quiet_NaN) {
                return (*mppp_pybind11::globals::mpf_class)(std::numeric_limits<double>::quiet_NaN()).release();
            } else {
                return (*mppp_pybind11::globals::mpf_class)("nan").release();
            }
        }
        const auto prec = mppp_pybind11::globals::mpmath_mp->attr("prec").cast<::mpfr_prec_t>();
        if (prec != mppp::safe_cast<::mpfr_prec_t>(mppp::real128_sig_digits())) {
            throw std::invalid_argument(
                "Cannot convert the real128 " + src.to_string() + " to an mpf: the precision of real128 ("
                + std::to_string(mppp::real128_sig_digits()) + ") is different from the current mpf precision ("
                + std::to_string(prec) + "). Please change the current mpf precision to "
                + std::to_string(mppp::real128_sig_digits()) + " in order to avoid this error");
        }
        int exp;
        auto fr = mppp::real128{::frexpq(src.m_value, &exp)};
        fr = scalbln(fr, mppp::safe_cast<long>(mppp::real128_sig_digits()));
        return (*mppp_pybind11::globals::mpf_class)(
                   make_tuple(mppp_pybind11::mppp_int_to_py(mppp::integer<1>(fr)),
                              static_cast<long>(mppp::integer<1>{exp} - mppp::real128_sig_digits())))
            .release();
    }
};

#endif
}
}

#if defined(__clang__) || defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

#endif
