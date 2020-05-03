// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_COMPLEX_HPP
#define MPPP_COMPLEX_HPP

#include <mp++/config.hpp>

#if defined(MPPP_WITH_MPC)

#include <cassert>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#include <mp++/concepts.hpp>
#include <mp++/detail/fwd_decl.hpp>
#include <mp++/detail/mpc.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>

#if defined(MPPP_WITH_QUADMATH)

#include <mp++/complex128.hpp>
#include <mp++/real128.hpp>

#endif

namespace mppp
{

// Detect real-valued interoperable types
// for complex.
template <typename T>
using is_rv_complex_interoperable
    = detail::disjunction<is_cpp_arithmetic<detail::uncvref_t<T>>, detail::is_integer<detail::uncvref_t<T>>,
                          detail::is_rational<detail::uncvref_t<T>>, std::is_same<detail::uncvref_t<T>, real>
#if defined(MPPP_WITH_QUADMATH)
                          ,
                          std::is_same<detail::uncvref_t<T>, real128>
#endif
                          >;

// Detect interoperable types
// for complex.
template <typename T>
using is_complex_interoperable
    = detail::disjunction<is_rv_complex_interoperable<detail::uncvref_t<T>>, is_cpp_complex<detail::uncvref_t<T>>
#if defined(MPPP_WITH_QUADMATH)
                          ,
                          std::is_same<detail::uncvref_t<T>, complex128>
#endif
                          >;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL rv_complex_interoperable = is_rv_complex_interoperable<T>::value;

template <typename T>
MPPP_CONCEPT_DECL complex_interoperable = is_complex_interoperable<T>::value;

#endif

// Fwd declare swap.
void swap(complex &, complex &) noexcept;

// Strongly typed enum alias for mpfr_prec_t.
enum class complex_prec_t : ::mpfr_prec_t {};

class MPPP_DLL_PUBLIC complex
{
    // Utility function to check the precision upon init.
    static ::mpfr_prec_t check_init_prec(::mpfr_prec_t p)
    {
        if (mppp_unlikely(!detail::real_prec_check(p))) {
            throw std::invalid_argument("Cannot init a complex with a precision of " + detail::to_string(p)
                                        + ": the maximum allowed precision is " + detail::to_string(real_prec_max())
                                        + ", the minimum allowed precision is " + detail::to_string(real_prec_min()));
        }
        return p;
    }

public:
    // Default ctor.
    complex();
    // Copy ctor.
    complex(const complex &);
    // Move constructor.
    complex(complex &&other) noexcept
    {
        // Shallow copy other.
        m_mpc = other.m_mpc;
        // Mark the other as moved-from.
        other.m_mpc.re->_mpfr_d = nullptr;
    }

    // Copy constructor with custom precision.
    explicit complex(const complex &, ::mpfr_prec_t);
    // Move constructor with custom precision.
    explicit complex(complex &&, ::mpfr_prec_t);

private:
    // A tag for private generic ctors.
    struct gtag {
    };
    // From real-valued interoperable types + optional precision.
    template <typename T, typename... Args>
    explicit complex(gtag, std::true_type, T &&x, const Args &... args)
    {
        // Init the real part from x + optional explicit precision.
        real re{std::forward<T>(x), static_cast<::mpfr_prec_t>(args)...};
        // The imaginary part is inited to +0, using either the explicit
        // precision (if provided) or the precision of the real part
        // otherwise.
        auto im = sizeof...(Args) ? real{real_kind::zero, 1, static_cast<::mpfr_prec_t>(args)...}
                                  : real{real_kind::zero, 1, re.get_prec()};

        // Shallow-copy into this.
        m_mpc.re[0] = *re.get_mpfr_t();
        m_mpc.im[0] = *im.get_mpfr_t();

        // Deactivate the temporaries.
        re._get_mpfr_t()->_mpfr_d = nullptr;
        im._get_mpfr_t()->_mpfr_d = nullptr;
    }
    // From complex-valued interoperable types + optional precision.
    // NOTE: this will delegate to the ctors from real + imaginary parts.
    // NOTE: no need for std::forward, as this constructor will involve
    // only std::complex or complex128.
    template <typename T, typename... Args>
    explicit complex(gtag, std::false_type, const T &c, const Args &... args)
        : complex(c.real(), c.imag(), static_cast<::mpfr_prec_t>(args)...)
    {
    }

public:
    // Ctor from interoperable types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <complex_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_complex_interoperable<T>::value, int> = 0>
#endif
    explicit complex(T &&x) : complex(gtag{}, is_rv_complex_interoperable<T>{}, std::forward<T>(x))
    {
    }
    // Ctor from interoperable types + precision.
#if defined(MPPP_HAVE_CONCEPTS)
    template <complex_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_complex_interoperable<T>::value, int> = 0>
#endif
    explicit complex(T &&x, complex_prec_t p) : complex(gtag{}, is_rv_complex_interoperable<T>{}, std::forward<T>(x), p)
    {
    }

private:
    // Implementation of the ctor from real and imaginary
    // parts + explicitly specified precision.
    template <typename T, typename U>
    void real_imag_ctor_impl(T &&re, U &&im, ::mpfr_prec_t p)
    {
        // Init real-imaginary parts with the input prec.
        real rp{std::forward<T>(re), p}, ip{std::forward<U>(im), p};

        // Shallow-copy into this.
        m_mpc.re[0] = *rp.get_mpfr_t();
        m_mpc.im[0] = *ip.get_mpfr_t();

        // Deactivate the temporaries.
        rp._get_mpfr_t()->_mpfr_d = nullptr;
        ip._get_mpfr_t()->_mpfr_d = nullptr;
    }

public:
    // Binary ctor from real-valued interoperable types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <rv_complex_interoperable T, rv_complex_interoperable U>
#else
    template <typename T, typename U,
              detail::enable_if_t<
                  detail::conjunction<is_rv_complex_interoperable<T>, is_rv_complex_interoperable<U>>::value, int> = 0>
#endif
    explicit complex(T &&re, U &&im)
    {
        // NOTE: the precision will be the largest between the
        // automatically-deduced ones for re and im.
        real_imag_ctor_impl(std::forward<T>(re), std::forward<U>(im),
                            detail::c_max(detail::real_deduce_precision(re), detail::real_deduce_precision(im)));
    }
    // Binary ctor from real-valued interoperable types + prec.
#if defined(MPPP_HAVE_CONCEPTS)
    template <rv_complex_interoperable T, rv_complex_interoperable U>
#else
    template <typename T, typename U,
              detail::enable_if_t<
                  detail::conjunction<is_rv_complex_interoperable<T>, is_rv_complex_interoperable<U>>::value, int> = 0>
#endif
    explicit complex(T &&re, U &&im, ::mpfr_prec_t p)
    {
        real_imag_ctor_impl(std::forward<T>(re), std::forward<U>(im), p);
    }

private:
    // A tag for private string ctors.
    struct stag {
    };
    MPPP_DLL_LOCAL void construct_from_c_string(const char *, int, ::mpfr_prec_t);
    explicit complex(const stag &, const char *, int, ::mpfr_prec_t);
    explicit complex(const stag &, const std::string &, int, ::mpfr_prec_t);
#if defined(MPPP_HAVE_STRING_VIEW)
    explicit complex(const stag &, const std::string_view &, int, ::mpfr_prec_t);
#endif

public:
    // Constructor from string, base and precision.
#if defined(MPPP_HAVE_CONCEPTS)
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    explicit complex(const T &s, int base, ::mpfr_prec_t p) : complex(stag{}, s, base, p)
    {
    }
    // Constructor from string and precision.
#if defined(MPPP_HAVE_CONCEPTS)
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    explicit complex(const T &s, ::mpfr_prec_t p) : complex(s, 10, p)
    {
    }
    // Constructor from range of characters, base and precision.
    explicit complex(const char *, const char *, int, ::mpfr_prec_t);
    // Constructor from range of characters and precision.
    explicit complex(const char *, const char *, ::mpfr_prec_t);

    // Constructor from mpc_t.
    explicit complex(const ::mpc_t);

    ~complex();

    // Copy assignment operator.
    complex &operator=(const complex &);
    // Move assignment operator.
    complex &operator=(complex &&other) noexcept
    {
        // NOTE: for generic code, std::swap() is not a particularly good way of implementing
        // the move assignment:
        //
        // https://stackoverflow.com/questions/6687388/why-do-some-people-use-swap-for-move-assignments
        //
        // Here however it is fine, as we know there are no side effects we need to maintain.
        //
        // NOTE: we use a raw std::swap() here (instead of mpc_swap()) because we don't know in principle
        // if mpc_swap() relies on the operands not to be in a moved-from state (although it's unlikely).
        std::swap(m_mpc, other.m_mpc);
        return *this;
    }

    class re_extractor
    {
    public:
        explicit re_extractor(complex &c) : m_c(c), m_value(real::shallow_copy_t{}, mpc_realref(&c.m_mpc)) {}

        re_extractor(const re_extractor &) = delete;
        re_extractor(re_extractor &&) = delete;
        re_extractor &operator=(const re_extractor &) = delete;
        re_extractor &operator=(re_extractor &&) = delete;

        ~re_extractor()
        {
            mpc_realref(&m_c.m_mpc)[0] = *m_value.get_mpfr_t();
            m_value._get_mpfr_t()->_mpfr_d = nullptr;
        }

        real &get()
        {
            return m_value;
        }

        real &operator*()
        {
            return m_value;
        }

        real *operator->()
        {
            return &m_value;
        }

    private:
        complex &m_c;
        real m_value;
    };

    class const_re_extractor
    {
    public:
        explicit const_re_extractor(const complex &c) : m_value(real::shallow_copy_t{}, mpc_realref(&c.m_mpc)) {}

        const_re_extractor(const const_re_extractor &) = delete;
        const_re_extractor(const_re_extractor &&) = delete;
        const_re_extractor &operator=(const const_re_extractor &) = delete;
        const_re_extractor &operator=(const_re_extractor &&) = delete;

        ~const_re_extractor()
        {
            m_value._get_mpfr_t()->_mpfr_d = nullptr;
        }

        const real &get() const
        {
            return m_value;
        }

        const real &operator*() const
        {
            return m_value;
        }

        const real *operator->() const
        {
            return &m_value;
        }

    private:
        real m_value;
    };

    class im_extractor
    {
    public:
        explicit im_extractor(complex &c) : m_c(c), m_value(real::shallow_copy_t{}, mpc_imagref(&c.m_mpc)) {}

        im_extractor(const im_extractor &) = delete;
        im_extractor(im_extractor &&) = delete;
        im_extractor &operator=(const im_extractor &) = delete;
        im_extractor &operator=(im_extractor &&) = delete;

        ~im_extractor()
        {
            mpc_imagref(&m_c.m_mpc)[0] = *m_value.get_mpfr_t();
            m_value._get_mpfr_t()->_mpfr_d = nullptr;
        }

        real &get()
        {
            return m_value;
        }

        real &operator*()
        {
            return m_value;
        }

        real *operator->()
        {
            return &m_value;
        }

    private:
        complex &m_c;
        real m_value;
    };

    class const_im_extractor
    {
    public:
        explicit const_im_extractor(const complex &c) : m_value(real::shallow_copy_t{}, mpc_imagref(&c.m_mpc)) {}

        const_im_extractor(const const_im_extractor &) = delete;
        const_im_extractor(const_im_extractor &&) = delete;
        const_im_extractor &operator=(const const_im_extractor &) = delete;
        const_im_extractor &operator=(const_im_extractor &&) = delete;

        ~const_im_extractor()
        {
            m_value._get_mpfr_t()->_mpfr_d = nullptr;
        }

        const real &get() const
        {
            return m_value;
        }

        const real &operator*() const
        {
            return m_value;
        }

        const real *operator->() const
        {
            return &m_value;
        }

    private:
        real m_value;
    };

#if MPPP_CPLUSPLUS >= 201703L
    auto real_cref() const
    {
        return const_re_extractor{*this};
    }
    auto imag_cref() const
    {
        return const_im_extractor{*this};
    }
    auto real_ref()
    {
        return re_extractor{*this};
    }
    auto imag_ref()
    {
        return im_extractor{*this};
    }
#endif

    ::mpfr_prec_t get_prec() const
    {
        assert(mpfr_get_prec(mpc_realref(&m_mpc)) == mpfr_get_prec(mpc_imagref(&m_mpc)));

        return mpfr_get_prec(mpc_realref(&m_mpc));
    }

private:
    // Utility function to check precision in set_prec().
    static ::mpfr_prec_t check_set_prec(::mpfr_prec_t p)
    {
        if (mppp_unlikely(!detail::real_prec_check(p))) {
            throw std::invalid_argument("Cannot set the precision of a complex to the value " + detail::to_string(p)
                                        + ": the maximum allowed precision is " + detail::to_string(real_prec_max())
                                        + ", the minimum allowed precision is " + detail::to_string(real_prec_min()));
        }
        return p;
    }
    // mpc_set_prec() wrapper, with or without prec checking.
    template <bool Check>
    void set_prec_impl(::mpfr_prec_t p)
    {
        ::mpc_set_prec(&m_mpc, Check ? check_set_prec(p) : p);
    }
    // Wrapper for setting the precision while preserving
    // the original value, with or without precision checking.
    template <bool Check>
    void prec_round_impl(::mpfr_prec_t p_)
    {
        const auto p = Check ? check_set_prec(p_) : p_;
        ::mpfr_prec_round(mpc_realref(&m_mpc), p, MPFR_RNDN);
        ::mpfr_prec_round(mpc_imagref(&m_mpc), p, MPFR_RNDN);
    }

public:
    const mpc_struct_t *get_mpc_t() const
    {
        return &m_mpc;
    }
    mpc_struct_t *_get_mpc_t()
    {
        return &m_mpc;
    }

    // Check validity.
    bool is_valid() const noexcept
    {
        return mpc_realref(&m_mpc)->_mpfr_d != nullptr;
    }

private:
    mpc_struct_t m_mpc;
};

template <typename T, typename U>
using are_complex_op_types = detail::disjunction<
    detail::conjunction<std::is_same<complex, detail::uncvref_t<T>>, std::is_same<complex, detail::uncvref_t<U>>>,
    detail::conjunction<std::is_same<complex, detail::uncvref_t<T>>, is_complex_interoperable<U>>,
    detail::conjunction<std::is_same<complex, detail::uncvref_t<U>>, is_complex_interoperable<T>>>;

template <typename T, typename U>
using are_complex_in_place_op_types
    = detail::conjunction<are_complex_op_types<T, U>, detail::negation<std::is_const<detail::unref_t<T>>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL complex_op_types = are_complex_op_types<T, U>::value;

template <typename T, typename U>
MPPP_CONCEPT_DECL complex_in_place_op_types = are_complex_in_place_op_types<T, U>::value;

#endif

// Swap.
inline void swap(complex &a, complex &b) noexcept
{
    ::mpc_swap(a._get_mpc_t(), b._get_mpc_t());
}

MPPP_DLL_PUBLIC std::ostream &operator<<(std::ostream &, const complex &);

namespace detail
{

MPPP_DLL_PUBLIC bool dispatch_complex_equality(const complex &, const complex &);

template <typename T, enable_if_t<is_rv_complex_interoperable<T>::value, int> = 0>
inline bool dispatch_complex_equality(const complex &c, const T &x)
{
    complex::const_re_extractor rex{c};

    return mpfr_zero_p(mpc_imagref(c.get_mpc_t())) != 0 && *rex == x;
}

template <typename T, enable_if_t<!is_rv_complex_interoperable<T>::value, int> = 0>
inline bool dispatch_complex_equality(const complex &c1, const T &c2)
{
    complex::const_re_extractor rex{c1};
    complex::const_im_extractor iex{c1};

    return *rex == c2.real() && *iex == c2.imag();
}

template <typename T>
inline bool dispatch_complex_equality(const T &x, const complex &c)
{
    return dispatch_complex_equality(c, x);
}

} // namespace detail

// Equality.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires complex_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_complex_op_types<T, U>::value, int> = 0>
#endif
    inline bool operator==(const T &x, const U &y)
{
    return detail::dispatch_complex_equality(x, y);
}

// Inequality.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires complex_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_complex_op_types<T, U>::value, int> = 0>
#endif
    inline bool operator!=(const T &x, const U &y)
{
    return !(x == y);
}

} // namespace mppp

#else

#error The complex.hpp header was included but mp++ was not configured with the MPPP_WITH_MPC option.

#endif

#endif
