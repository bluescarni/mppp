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

namespace detail
{

// Detect real-valued interoperable types
// for complex. For internal use only.
template <typename T>
using is_rv_complex_interoperable
    = disjunction<is_cpp_arithmetic<T>, is_integer<T>, is_rational<T>, std::is_same<T, real>
#if defined(MPPP_WITH_QUADMATH)
                  ,
                  std::is_same<T, real128>
#endif
                  >;

} // namespace detail

template <typename T>
using is_complex_interoperable = detail::disjunction<detail::is_rv_complex_interoperable<T>, is_cpp_complex<T>
#if defined(MPPP_WITH_QUADMATH)
                                                     ,
                                                     std::is_same<T, complex128>
#endif
                                                     >;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL complex_interoperable = is_complex_interoperable<T>::value;

#endif

// Fwd declare swap.
void swap(complex &, complex &) noexcept;

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

private:
    // A tag for private generic ctors.
    struct gtag {
    };
    // From real-valued interoperable types.
    template <typename T>
    explicit complex(gtag, const T &x, std::true_type)
    {
        real re{x}, im{0, re.get_prec()};

        // Shallow-copy into this.
        m_mpc.re[0] = *re.get_mpfr_t();
        m_mpc.im[0] = *im.get_mpfr_t();

        // Deactivate the temporaries.
        re._get_mpfr_t()->_mpfr_d = nullptr;
        im._get_mpfr_t()->_mpfr_d = nullptr;
    }
    // From complex-valued interoperable types.
    template <typename T>
    explicit complex(gtag, const T &c, std::false_type)
    {
        real re{c.real()}, im{c.imag()};

        // Shallow-copy into this.
        m_mpc.re[0] = *re.get_mpfr_t();
        m_mpc.im[0] = *im.get_mpfr_t();

        // Deactivate the temporaries.
        re._get_mpfr_t()->_mpfr_d = nullptr;
        im._get_mpfr_t()->_mpfr_d = nullptr;
    }

public:
    // Ctor from interoperable types.
    template <typename T, detail::enable_if_t<is_complex_interoperable<T>::value, int> = 0>
    explicit complex(const T &x) : complex(gtag{}, x, detail::is_rv_complex_interoperable<T>{})
    {
    }

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

    private:
        real m_value;
    };

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
    // mpfr_set_prec() wrapper, with or without prec checking.
    template <bool Check>
    void set_prec_impl(::mpfr_prec_t p)
    {
        ::mpc_set_prec(&m_mpc, Check ? check_set_prec(p) : p);
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
    detail::conjunction<std::is_same<complex, detail::uncvref_t<T>>, is_complex_interoperable<detail::uncvref_t<U>>>,
    detail::conjunction<std::is_same<complex, detail::uncvref_t<U>>, is_complex_interoperable<detail::uncvref_t<T>>>>;

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

    return mpfr_zero_p(mpc_imagref(&c.m_mpc)) != 0 && rex.get() == x;
}

template <typename T, enable_if_t<!is_rv_complex_interoperable<T>::value, int> = 0>
inline bool dispatch_complex_equality(const complex &c1, const T &c2)
{
    complex::const_re_extractor rex{c1};
    complex::const_im_extractor iex{c1};

    return rex.get() == c2.real() && iex.get() == c2.imag();
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
