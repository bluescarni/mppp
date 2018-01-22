// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_RATIONAL_HPP
#define MPPP_RATIONAL_HPP

#include <mp++/config.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#if MPPP_CPLUSPLUS >= 201703L
#include <string_view>
#endif
#include <type_traits>
#include <utility>
#include <vector>

#include <mp++/concepts.hpp>
#include <mp++/detail/fwd_decl.hpp>
#include <mp++/detail/gmp.hpp>
#if defined(MPPP_WITH_MPFR)
#include <mp++/detail/mpfr.hpp>
#endif
#include <mp++/detail/type_traits.hpp>
#include <mp++/exceptions.hpp>
#include <mp++/integer.hpp>

namespace mppp
{

template <typename T, std::size_t SSize>
using is_rational_interoperable = disjunction<is_cpp_interoperable<T>, std::is_same<T, integer<SSize>>>;

template <typename T, std::size_t SSize>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RationalInteroperable = is_rational_interoperable<T, SSize>::value;
#else
using rational_interoperable_enabler = enable_if_t<is_rational_interoperable<T, SSize>::value, int>;
#endif

template <typename T, std::size_t SSize>
using is_rational_integral_interoperable
    = disjunction<is_cpp_integral_interoperable<T>, std::is_same<T, integer<SSize>>>;

template <typename T, std::size_t SSize>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RationalIntegralInteroperable = is_rational_integral_interoperable<T, SSize>::value;
#else
using rational_integral_interoperable_enabler = enable_if_t<is_rational_integral_interoperable<T, SSize>::value, int>;
#endif

template <typename T, std::size_t SSize>
using is_rational_cvr_interoperable = is_rational_interoperable<uncvref_t<T>, SSize>;

template <typename T, std::size_t SSize>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RationalCvrInteroperable = is_rational_cvr_interoperable<T, SSize>::value;
#else
using rational_cvr_interoperable_enabler = enable_if_t<is_rational_cvr_interoperable<T, SSize>::value, int>;
#endif

template <typename T, std::size_t SSize>
using is_rational_cvr_integral_interoperable = is_rational_integral_interoperable<uncvref_t<T>, SSize>;

template <typename T, std::size_t SSize>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RationalCvrIntegralInteroperable = is_rational_cvr_integral_interoperable<T, SSize>::value;
#else
using rational_cvr_integral_interoperable_enabler
    = enable_if_t<is_rational_cvr_integral_interoperable<T, SSize>::value, int>;
#endif

inline namespace detail
{

// This is useful in various bits below.
template <std::size_t SSize>
inline void fix_den_sign(rational<SSize> &q)
{
    if (q.get_den().sgn() == -1) {
        q._get_num().neg();
        q._get_den().neg();
    }
}

// Detect rational.
template <typename T>
struct is_rational : std::false_type {
};

template <std::size_t SSize>
struct is_rational<rational<SSize>> : std::true_type {
};

// Detect rationals with the same static size.
template <typename T, typename U>
struct is_same_ssize_rational : std::false_type {
};

template <std::size_t SSize>
struct is_same_ssize_rational<rational<SSize>, rational<SSize>> : std::true_type {
};

// mpq_view getter fwd declaration.
// NOTE: the returned mpq_struct_t should always be marked as const, as we cannot modify its content
// (due to the use of const_cast() within the mpz view machinery).
template <std::size_t SSize>
mpq_struct_t get_mpq_view(const rational<SSize> &);
}

/// Multiprecision rational class.
/**
 * \rststar
 * This class represents arbitrary-precision rationals. Internally, the class stores a pair of
 * :cpp:class:`integers <mppp::integer>` with static size ``SSize`` as the numerator and denominator.
 * Rational numbers are represented in the usual canonical form:
 *
 * * numerator and denominator are coprime,
 * * the denominator is always strictly positive.
 *
 * This class has the look and feel of a C++ builtin type: it can interact with most of C++'s integral and
 * floating-point primitive types (see the :cpp:concept:`~mppp::CppInteroperable` concept for the full list)
 * and with :cpp:class:`integers <mppp::integer>` with static size ``SSize``,
 * and it provides overloaded :ref:`operators <rational_operators>`. Differently from the builtin types,
 * however, this class does not allow any implicit conversion to/from other types (apart from ``bool``): construction
 * from and conversion to primitive types must always be requested explicitly. As a side effect, syntax such as
 *
 * .. code-block:: c++
 *
 *    rational<1> q = 5;
 *    int m = q;
 *
 * will not work, and direct initialization should be used instead:
 *
 * .. code-block:: c++
 *
 *    rational<1> q{5};
 *    int m{q};
 *
 * Most of the functionality is exposed via plain :ref:`functions <rational_functions>`, with the
 * general convention that the functions are named after the corresponding GMP functions minus the leading ``mpq_``
 * prefix. For instance, the GMP call
 *
 * .. code-block:: c++
 *
 *    mpq_add(rop,a,b);
 *
 * that writes the result of ``a + b`` into ``rop`` becomes simply
 *
 * .. code-block:: c++
 *
 *    add(rop,a,b);
 *
 * where the ``add()`` function is resolved via argument-dependent lookup. Function calls with overlapping arguments
 * are allowed, unless noted otherwise. In a similar way to the :cpp:class:`~mppp::integer` class, various convenience
 * overloads are provided for the same functionality. For instance, here are three possible ways of computing the
 * absolute value:
 *
 * .. code-block:: c++
 *
 *    rational<1> q1, q2, q{-5};
 *    abs(q1,q);   // Binary abs(): computes and stores the absolute value
 *                 // of q into q1.
 *    q2 = abs(q); // Unary abs(): returns the absolute value of q, which is
 *                 // then assigned to q2.
 *    q.abs();     // Member function abs(): replaces the value of q with its
 *                 // absolute value.
 *
 * Various :ref:`overloaded operators <rational_operators>` are provided.
 * For the common arithmetic operations (``+``, ``-``, ``*`` and ``/``), the type promotion
 * rules are a natural extension of the corresponding rules for native C++ types: if the other argument
 * is a C++ integral or an :cpp:class:`~mppp::integer`, the result will be of type :cpp:class:`~mppp::rational`, if the
 * other argument is a C++ floating-point the result will be of the same floating-point type. For example:
 *
 * .. code-block:: c++
 *
 *    rational<1> q1{1}, q2{2};
 *    integer<1> n{2};
 *    auto res1 = q1 + q2; // res1 is a rational
 *    auto res2 = q1 * 2; // res2 is a rational
 *    auto res3 = q1 * n; // res3 is a rational
 *    auto res4 = 2 - q2; // res4 is a rational
 *    auto res5 = q1 / 2.f; // res5 is a float
 *    auto res6 = 12. / q1; // res6 is a double
 *
 * The relational operators, ``==``, ``!=``, ``<``, ``>``, ``<=`` and ``>=`` will promote the arguments to a common type
 * before comparing them. The promotion rules are the same as in the arithmetic operators (that is, both arguments are
 * promoted to :cpp:class:`~mppp::rational` if no floating-point types are involved, otherwise they are promoted to the
 * type of the floating-point argument).
 *
 * The :cpp:class:`~mppp::rational` class allows to access and manipulate directly the numerator and denominator
 * via the :cpp:func:`~mppp::rational::get_num()`, :cpp:func:`~mppp::rational::get_den()`,
 * :cpp:func:`~mppp::rational::_get_num()` and :cpp:func:`~mppp::rational::_get_den()` methods, so that it is possible
 * to use :cpp:class:`~mppp::integer` functions directly on numerator and denominator. The mutable getters' names
 * :cpp:func:`~mppp::rational::_get_num()` and :cpp:func:`~mppp::rational::_get_den()` are prefixed with an underscore
 * ``_`` to highlight their potentially dangerous nature: it is the user's responsibility to ensure that the canonical
 * form of the rational is preserved after altering the numerator and/or the denominator via the mutable getters.
 * \endrststar
 */
// NOTEs:
// - not clear if the NewRop flag helps at all. Needs to be benchmarked. If it does, its usage could
//   be expanded in mul/div.
// - we might be paying a perf penalty for dynamic storage values due to the lack of pre-allocation for
//   temporary variables in algorithms such as addsub, mul, div, etc. Needs to be investigated.
// - in the algorithm bits derived from GMP ones, we don't check for unitary GCDs because the original
//   algos do not. We should investigate if there's perf benefits to be had there.
// - cmp() can be improved (see comments there).
template <std::size_t SSize>
class rational
{
public:
/// Underlying integral type.
/**
 * \rststar
 * This is the :cpp:class:`~mppp::integer` type used for the representation of numerator and
 * denominator.
 * \endrststar
 */
#if defined(MPPP_DOXYGEN_INVOKED)
    typedef integer<SSize> int_t;
#else
    using int_t = integer<SSize>;
#endif
    /// Alias for the template parameter \p SSize.
    static constexpr std::size_t ssize = SSize;
    /// Default constructor.
    /**
     * The default constructor will initialize ``this`` to 0 (represented as 0/1).
     */
    rational() : m_den(1u) {}
    /// Defaulted copy constructor.
    rational(const rational &) = default;
    /// Move constructor.
    /**
     * The move constructor will leave \p other in an unspecified but valid state.
     *
     * @param other the construction argument.
     */
    rational(rational &&other) noexcept : m_num(std::move(other.m_num)), m_den(std::move(other.m_den))
    {
        // NOTE: the aim of this is to have other in a valid state. One reason is that,
        // according to the standard, for use in std::sort() (and possibly other algorithms)
        // it is required that a moved-from rational is still comparable, but if we simply move
        // construct num and den we could end up with other in noncanonical form or zero den.
        // http://stackoverflow.com/questions/26579132/what-is-the-post-condition-of-a-move-constructor
        //
        // NOTE: after move construction, other's data members are guaranteed to be static.
        assert(other.m_num.is_static());
        assert(other.m_den.is_static());
        // Set other's numerator to 0 (see integer::set_zero()).
        other.m_num.m_int.g_st()._mp_size = 0;
        other.m_num.m_int.g_st().zero_upper_limbs(0);
        // Set other's denominator to 1 (see integer::set_one()).
        other.m_den.m_int.g_st()._mp_size = 1;
        other.m_den.m_int.g_st().m_limbs[0] = 1;
        other.m_den.m_int.g_st().zero_upper_limbs(1);
    }

private:
    // A tag for private constrcutors.
    struct ptag {
    };
    template <typename T, enable_if_t<disjunction<std::is_same<float, T>, std::is_same<double, T>>::value, int> = 0>
    explicit rational(const ptag &, const T &x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot construct a rational from the non-finite floating-point value "
                                    + mppp::to_string(x));
        }
        MPPP_MAYBE_TLS mpq_raii q;
        ::mpq_set_d(&q.m_mpq, static_cast<double>(x));
        m_num = mpq_numref(&q.m_mpq);
        m_den = mpq_denref(&q.m_mpq);
    }
#if defined(MPPP_WITH_MPFR)
    explicit rational(const ptag &, const long double &x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot construct a rational from the non-finite floating-point value "
                                    + mppp::to_string(x));
        }
        // NOTE: static checks for overflows and for the precision value are done in mpfr.hpp.
        constexpr int d2 = std::numeric_limits<long double>::max_digits10 * 4;
        MPPP_MAYBE_TLS mpfr_raii mpfr(static_cast<::mpfr_prec_t>(d2));
        MPPP_MAYBE_TLS mpf_raii mpf(static_cast<::mp_bitcnt_t>(d2));
        MPPP_MAYBE_TLS mpq_raii mpq;
        // NOTE: we go through an mpfr->mpf->mpq conversion chain as
        // mpfr_get_q() does not exist.
        // NOTE: probably coming in MPFR 4:
        // https://lists.gforge.inria.fr/pipermail/mpfr-commits/2017-June/011186.html
        ::mpfr_set_ld(&mpfr.m_mpfr, x, MPFR_RNDN);
        ::mpfr_get_f(&mpf.m_mpf, &mpfr.m_mpfr, MPFR_RNDN);
        ::mpq_set_f(&mpq.m_mpq, &mpf.m_mpf);
        m_num = mpq_numref(&mpq.m_mpq);
        m_den = mpq_denref(&mpq.m_mpq);
    }
#endif
    template <typename T, enable_if_t<is_rational_cvr_integral_interoperable<T, SSize>::value, int> = 0>
    explicit rational(const ptag &, T &&n) : m_num(std::forward<T>(n)), m_den(1u)
    {
    }

public:
    /// Generic constructor.
    /**
     * \rststar
     * This constructor will initialize a rational with the value ``x``. The construction will fail if ``x``
     * is a non-finite floating-point value.
     * \endrststar
     *
     * @param x the value that will be used to initialize \p this.
     *
     * @throws std::domain_error if \p x is a non-finite floating-point value.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    explicit rational(RationalCvrInteroperable<SSize> &&x)
#else
    template <typename T, rational_cvr_interoperable_enabler<T, SSize> = 0>
    explicit rational(T &&x)
#endif
        : rational(ptag{}, std::forward<decltype(x)>(x))
    {
    }
/// Constructor from numerator and denominator.
/**
 * \rststar
 * This constructor is enabled only if both ``T`` and ``U`` satisfy the
 * :cpp:concept:`~mppp::RationalCvrIntegralInteroperable` concept. The input value ``n`` will be used to initialise the
 * numerator, while ``d`` will be used to initialise the denominator. If ``make_canonical`` is ``true``
 * (the default), then :cpp:func:`~mppp::rational::canonicalise()` will be called after the construction
 * of numerator and denominator.
 *
 * .. warning::
 *
 *    If this constructor is called with ``make_canonical`` set to ``false``, it will be the user's responsibility
 *    to ensure that ``this`` is canonicalised before using it with other mp++ functions. Failure to do
 *    so will result in undefined behaviour.
 * \endrststar
 *
 * @param n the numerator.
 * @param d the denominator.
 * @param make_canonical if \p true, the rational will be canonicalised after the construction
 * of numerator and denominator.
 *
 * @throws zero_division_error if the denominator is zero.
 */
#if defined(MPPP_HAVE_CONCEPTS)
    template <RationalCvrIntegralInteroperable<SSize> T, RationalCvrIntegralInteroperable<SSize> U>
#else
    template <typename T, typename U,
              enable_if_t<conjunction<is_rational_cvr_integral_interoperable<T, SSize>,
                                      is_rational_cvr_integral_interoperable<U, SSize>>::value,
                          int> = 0>
#endif
    explicit rational(T &&n, U &&d, bool make_canonical = true) : m_num(std::forward<T>(n)), m_den(std::forward<U>(d))
    {
        if (mppp_unlikely(m_den.is_zero())) {
            throw zero_division_error("Cannot construct a rational with zero as denominator");
        }
        if (make_canonical) {
            canonicalise();
        }
    }

private:
    // Implementation of the constructor from C string. Requires a def-cted object.
    void dispatch_c_string_ctor(const char *s, int base)
    {
        MPPP_MAYBE_TLS std::string tmp_str;
        auto ptr = s;
        for (; *ptr != '\0' && *ptr != '/'; ++ptr) {
        }
        tmp_str.assign(s, ptr);
        m_num = int_t{tmp_str, base};
        if (*ptr != '\0') {
            tmp_str.assign(ptr + 1);
            m_den = int_t{tmp_str, base};
            if (mppp_unlikely(m_den.is_zero())) {
                throw zero_division_error(
                    "A zero denominator was detected in the constructor of a rational from string");
            }
            canonicalise();
        }
    }
    explicit rational(const ptag &, const char *s, int base) : m_den(1u)
    {
        dispatch_c_string_ctor(s, base);
    }
    explicit rational(const ptag &, const std::string &s, int base) : rational(s.c_str(), base) {}
#if MPPP_CPLUSPLUS >= 201703L
    explicit rational(const ptag &, const std::string_view &s, int base) : rational(s.data(), s.data() + s.size(), base)
    {
    }
#endif

public:
    /// Constructor from string.
    /**
     * \rststar
     * This constructor will initialize ``this`` from the :cpp:concept:`~mppp::StringType` ``s``, which must represent
     * a rational value in base ``base``. The expected format is either a numerator-denominator pair separated
     * by the division operator ``/``, or just a numerator (in which case the denominator will be set to one).
     * The format for numerator and denominator is described in the documentation of the constructor from string
     * of :cpp:class:`~mppp::integer`.
     * \endrststar
     *
     * @param s the input string.
     * @param base the base used in the string representation.
     *
     * @throws zero_division_error if the denominator is zero.
     * @throws unspecified any exception thrown by the string constructor of mppp::integer, or by
     * memory errors in standard containers.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    explicit rational(const StringType &s,
#else
    template <typename T, string_type_enabler<T> = 0>
    explicit rational(const T &s,
#endif
                      int base = 10)
        : rational(ptag{}, s, base)
    {
    }
    /// Constructor from range of characters.
    /**
     * This constructor will initialise \p this from the content of the input half-open range,
     * which is interpreted as the string representation of a rational in base \p base.
     *
     * Internally, the constructor will copy the content of the range to a local buffer, add a
     * string terminator, and invoke the constructor from string.
     *
     * @param begin the begin of the input range.
     * @param end the end of the input range.
     * @param base the base used in the string representation.
     *
     * @throws unspecified any exception thrown by the constructor from string, or by
     * memory errors in standard containers.
     */
    explicit rational(const char *begin, const char *end, int base = 10) : m_den(1u)
    {
        // Copy the range into a local buffer.
        MPPP_MAYBE_TLS std::vector<char> buffer;
        buffer.assign(begin, end);
        buffer.emplace_back('\0');
        dispatch_c_string_ctor(buffer.data(), base);
    }
    /// Constructor from \p mpz_t.
    /**
     * This constructor will initialise the numerator of \p this with the input GMP integer \p n,
     * and the denominator to 1.
     *
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that ``n`` has been correctly initialized. Calling this constructor
     *    with an uninitialized ``n`` results in undefined behaviour.
     * \endrststar
     *
     * @param n the input GMP integer.
     */
    explicit rational(const ::mpz_t n) : m_num(n), m_den(1u) {}
    /// Copy constructor from \p mpq_t.
    /**
     * This constructor will initialise the numerator and denominator of \p this with those of the GMP rational \p q.
     *
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that ``q`` has been correctly initialized. Calling this constructor
     *    with an uninitialized ``q`` results in undefined behaviour.
     *
     *    This constructor will **not**
     *    canonicalise ``this``: numerator and denominator are constructed as-is from ``q``.
     * \endrststar
     *
     * @param q the input GMP rational.
     */
    explicit rational(const ::mpq_t q) : m_num(mpq_numref(q)), m_den(mpq_denref(q)) {}
#if !defined(_MSC_VER)
    /// Move constructor from \p mpq_t.
    /**
     * This constructor will move the numerator and denominator of the GMP rational \p q into \p this.
     *
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that ``q`` has been correctly initialized. Calling this constructor
     *    with an uninitialized ``q`` results in undefined behaviour.
     *
     *    This constructor will **not**
     *    canonicalise ``this``: numerator and denominator are constructed as-is from ``q``.
     *
     *    The user must ensure that, after construction, ``mpq_clear()`` is never
     *    called on ``q``: the resources previously owned by ``q`` are now owned by ``this``, which
     *    will take care of releasing them when the destructor is called.
     *
     * .. note::
     *
     *    Due to a compiler bug, this constructor is not available on Microsoft Visual Studio.
     * \endrststar
     *
     * @param q the input GMP rational.
     */
    explicit rational(::mpq_t &&q) : m_num(::mpz_t{*mpq_numref(q)}), m_den(::mpz_t{*mpq_denref(q)}) {}
#endif
    /// Defaulted copy-assignment operator.
    /**
     * @return a reference to ``this``.
     */
    // NOTE: as long as copy assignment of integer cannot
    // throw, the default is good.
    rational &operator=(const rational &) = default;
    /// Defaulted move assignment operator.
    /**
     * After the assignment, \p other is left in an unspecified but valid state.
     *
     * @param other the assignment argument.
     *
     * @return a reference to ``this``.
     */
    rational &operator=(rational &&other) noexcept
    {
        // NOTE: see the rationale in the move ctor about why we don't want
        // to default this.
        //
        // NOTE: we need the self check here because otherwise we will end
        // up setting this to 0/1, in case of self assignment.
        if (mppp_likely(this != &other)) {
            m_num = std::move(other.m_num);
            m_den = std::move(other.m_den);
            other.m_num.set_zero();
            other.m_den.set_one();
        }
        return *this;
    }

private:
    // Implementation of the generic assignment operator.
    template <typename T>
    void dispatch_assignment(T &&n, const std::true_type &)
    {
        m_num = std::forward<T>(n);
        m_den.set_one();
    }
    template <typename T>
    void dispatch_assignment(const T &x, const std::false_type &)
    {
        *this = rational{x};
    }

public:
    /// Generic assignment operator.
    /**
     * \rststar
     * This operator will assign ``x`` to ``this``.
     * \endrststar
     *
     * @param x the assignment argument.
     *
     * @return a reference to ``this``.
     *
     * @throws unspecified any exception thrown by the generic constructor of \link mppp::rational\endlink.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    rational &operator=(RationalCvrInteroperable<SSize> &&x)
#else
    template <typename T, rational_cvr_interoperable_enabler<T, SSize> = 0>
    rational &operator=(T &&x)
#endif
    {
        dispatch_assignment(
            std::forward<decltype(x)>(x),
            std::integral_constant<bool, is_rational_cvr_integral_interoperable<decltype(x), SSize>::value>{});
        return *this;
    }
        /// Assignment from string.
        /**
         * \rststar
         * The body of this operator is equivalent to:
         *
         * .. code-block:: c++
         *
         *    return *this = rational{s};
         *
         * That is, a temporary rational is constructed from ``s`` and it is then move-assigned to ``this``.
         * \endrststar
         *
         * @param s the string that will be used for the assignment.
         *
         * @return a reference to \p this.
         *
         * @throws unspecified any exception thrown by the constructor from string.
         */
#if defined(MPPP_HAVE_CONCEPTS)
    rational &operator=(const StringType &s)
#else
    template <typename T, string_type_enabler<T> = 0>
    rational &operator=(const T &s)
#endif
    {
        return *this = rational{s};
    }
    /// Assignment from \p mpz_t.
    /**
     * \rststar
     * This assignment operator will copy into the numerator of ``this`` the value of the GMP integer ``n``,
     * and it will set the denominator to 1 via :cpp:func:`mppp::integer::set_one()`.
     *
     * .. warning::
     *
     *    It is the user's responsibility to ensure that ``n`` has been correctly initialized. Calling this operator
     *    with an uninitialized ``n`` results in undefined behaviour.
     *
     *    No aliasing is allowed:
     *    the data in ``n`` must be completely distinct from the data in ``this`` (e.g., if ``n`` is an ``mpz_view`` of
     *    the numerator of ``this`` then it might point to internal data of ``this``, and the behaviour of this operator
     *    will thus be undefined).
     * \endrststar
     *
     * @param n the input GMP integer.
     *
     * @return a reference to \p this.
     */
    rational &operator=(const ::mpz_t n)
    {
        m_num = n;
        m_den.set_one();
        return *this;
    }
    /// Copy assignment from \p mpq_t.
    /**
     * This assignment operator will copy into \p this the value of the GMP rational \p q.
     *
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that ``q`` has been correctly initialized. Calling this operator
     *    with an uninitialized ``q`` results in undefined behaviour.
     *
     *    This operator will **not** canonicalise
     *    the assigned value: numerator and denominator are assigned as-is from ``q``.
     *
     *    No aliasing is allowed:
     *    the data in ``q`` must be completely distinct from the data in ``this``.
     * \endrststar
     *
     * @param q the input GMP rational.
     *
     * @return a reference to \p this.
     */
    rational &operator=(const ::mpq_t q)
    {
        m_num = mpq_numref(q);
        m_den = mpq_denref(q);
        return *this;
    }
#if !defined(_MSC_VER)
    /// Move assignment from \p mpq_t.
    /**
     * This assignment operator will move the GMP rational \p q into \p this.
     *
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that ``q`` has been correctly initialized. Calling this operator
     *    with an uninitialized ``q`` results in undefined behaviour.
     *
     *    This operator will **not** canonicalise
     *    the assigned value: numerator and denominator are assigned as-is from ``q``.
     *
     *    No aliasing is allowed:
     *    the data in ``q`` must be completely distinct from the data in ``this``.
     *
     *    The user must ensure that, after the assignment, ``mpq_clear()`` is never
     *    called on ``q``: the resources previously owned by ``q`` are now owned by ``this``, which
     *    will take care of releasing them when the destructor is called.
     *
     * .. note::
     *
     *    Due to a compiler bug, this operator is not available on Microsoft Visual Studio.
     * \endrststar
     *
     * @param q the input GMP rational.
     *
     * @return a reference to \p this.
     */
    rational &operator=(::mpq_t &&q)
    {
        m_num = ::mpz_t{*mpq_numref(q)};
        m_den = ::mpz_t{*mpq_denref(q)};
        return *this;
    }
#endif
    /// Convert to string.
    /**
     * \rststar
     * This operator will return a string representation of ``this`` in base ``base``.
     * The string format consists of the numerator, followed by the division operator ``/`` and the
     * denominator, but only if the denominator is not unitary. Otherwise, only the numerator will be
     * represented in the returned string.
     * \endrststar
     *
     * @param base the desired base for the string representation.
     *
     * @return a string representation for ``this``.
     *
     * @throws unspecified any exception thrown by mppp::integer::to_string().
     */
    std::string to_string(int base = 10) const
    {
        if (m_den.is_one()) {
            return m_num.to_string(base);
        }
        return m_num.to_string(base) + "/" + m_den.to_string(base);
    }

private:
    // Conversion to int_t.
    template <typename T, enable_if_t<std::is_same<int_t, T>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        return std::make_pair(true, m_num / m_den);
    }
    // Conversion to bool.
    template <typename T, enable_if_t<std::is_same<bool, T>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        return std::make_pair(true, m_num.m_int.m_st._mp_size != 0);
    }
    // Conversion to integral types other than bool.
    template <typename T, enable_if_t<conjunction<is_integral<T>, negation<std::is_same<bool, T>>>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        return static_cast<int_t>(*this).template dispatch_conversion<T>();
    }
    // Conversion to float/double.
    template <typename T, enable_if_t<disjunction<std::is_same<T, float>, std::is_same<T, double>>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        const auto v = get_mpq_view(*this);
        return std::make_pair(true, static_cast<T>(::mpq_get_d(&v)));
    }
#if defined(MPPP_WITH_MPFR)
    // Conversion to long double.
    template <typename T, enable_if_t<std::is_same<T, long double>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        constexpr int d2 = std::numeric_limits<long double>::max_digits10 * 4;
        MPPP_MAYBE_TLS mpfr_raii mpfr(static_cast<::mpfr_prec_t>(d2));
        const auto v = get_mpq_view(*this);
        ::mpfr_set_q(&mpfr.m_mpfr, &v, MPFR_RNDN);
        return std::make_pair(true, ::mpfr_get_ld(&mpfr.m_mpfr, MPFR_RNDN));
    }
#endif

public:
/// Generic conversion operator.
/**
 * \rststar
 * This operator will convert ``this`` to a :cpp:concept:`~mppp::RationalInteroperable` type.
 * Conversion to ``bool`` yields ``false`` if ``this`` is zero,
 * ``true`` otherwise. Conversion to other integral types and to :cpp:type:`~mppp::rational::int_t`
 * yields the result of the truncated division of the numerator by the denominator, if representable by the target
 * :cpp:concept:`~mppp::RationalInteroperable` type. Conversion to floating-point types might yield inexact values and
 * infinities.
 * \endrststar
 *
 * @return \p this converted to the target type.
 *
 * @throws std::overflow_error if the target type is an integral type and the value of ``this`` cannot be
 * represented by it.
 */
#if defined(MPPP_HAVE_CONCEPTS)
    template <RationalInteroperable<SSize> T>
#else
    template <typename T, rational_interoperable_enabler<T, SSize> = 0>
#endif
    explicit operator T() const
    {
        auto retval = dispatch_conversion<T>();
        if (mppp_unlikely(!retval.first)) {
            throw std::overflow_error("Conversion of the rational " + to_string() + " to the type '" + type_string<T>()
                                      + "' results in overflow");
        }
        return std::move(retval.second);
    }

private:
    // int_t getter.
    bool dispatch_get(int_t &rop) const
    {
        return tdiv_q(rop, m_num, m_den), true;
    }
    // The other getters.
    template <typename T>
    bool dispatch_get(T &rop) const
    {
        auto retval = dispatch_conversion<T>();
        if (retval.first) {
            rop = std::move(retval.second);
            return true;
        }
        return false;
    }

public:
    /// Generic conversion method.
    /**
     * \rststar
     * This method, similarly to the conversion operator, will convert ``this`` to a
     * :cpp:concept:`~mppp::RationalInteroperable` type, storing the result of the conversion into ``rop``. Differently
     * from the conversion operator, this method does not raise any exception: if the conversion is successful, the
     * method will return ``true``, otherwise the method will return ``false``. If the conversion fails,
     * ``rop`` will not be altered.
     * \endrststar
     *
     * @param rop the variable which will store the result of the conversion.
     *
     * @return ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail only if ``rop`` is
     * a C++ integral which cannot represent the truncated value of ``this``.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    template <RationalInteroperable<SSize> T>
#else
    template <typename T, rational_interoperable_enabler<T, SSize> = 0>
#endif
    bool get(T &rop) const
    {
        return dispatch_get(rop);
    }
    /// Const numerator getter.
    /**
     * @return a const reference to the numerator.
     */
    const int_t &get_num() const
    {
        return m_num;
    }
    /// Const denominator getter.
    /**
     * @return a const reference to the denominator.
     */
    const int_t &get_den() const
    {
        return m_den;
    }
    /// Mutable numerator getter.
    /**
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that, after changing the numerator
     *    via this getter, the rational is kept in canonical form.
     * \endrststar
     *
     * @return a mutable reference to the numerator.
     */
    int_t &_get_num()
    {
        return m_num;
    }
    /// Mutable denominator getter.
    /**
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that, after changing the denominator
     *    via this getter, the rational is kept in canonical form.
     * \endrststar
     *
     * @return a mutable reference to the denominator.
     */
    int_t &_get_den()
    {
        return m_den;
    }
    /// Canonicalise.
    /**
     * \rststar
     * This method will put ``this`` in canonical form. In particular, this method
     * will make sure that:
     *
     * * if the numerator is zero, the denominator is set to 1,
     * * the numerator and denominator are coprime (dividing them by their GCD,
     *   if necessary),
     * * the denominator is strictly positive.
     *
     * In general, it is not necessary to call explicitly this method, as the public
     * API of :cpp:class:`~mppp::rational` ensures that rationals are kept in canonical
     * form. Calling this method, however, might be necessary if the numerator and/or denominator
     * are modified manually, or when constructing/assigning from non-canonical ``mpq_t``
     * values.
     *
     * .. warning::
     *
     *    Calling this method with on a rational with null denominator will result in undefined
     *    behaviour.
     * \endrststar
     *
     * @return a reference to \p this.
     */
    rational &canonicalise()
    {
        if (m_num.is_zero()) {
            m_den = 1;
            return *this;
        }
        // NOTE: this is best in case of small m_num/m_den.
        // For dynamically allocated num/den, it would be better
        // to have a TLS integer and re-use that accross calls to
        // canonicalise. Eventually, we could consider branching
        // this bit out depending on whether num/den are static
        // or not. Let's keep it simple for now.
        // NOTE: gcd() always gets a positive value.
        const auto g = gcd(m_num, m_den);
        // This can be zero only if both num and den are zero.
        assert(!g.is_zero());
        if (!g.is_one()) {
            divexact_gcd(m_num, m_num, g);
            divexact_gcd(m_den, m_den, g);
        }
        // Fix mismatch in signs.
        fix_den_sign(*this);
        // NOTE: consider attempting demoting num/den. Let's KIS for now.
        return *this;
    }
    /// Check canonical form.
    /**
     * @return \p true if \p this is the canonical form for rational numbers, \p false otherwise.
     */
    bool is_canonical() const
    {
        if (m_num.is_zero()) {
            // If num is zero, den must be one.
            return m_den.is_one();
        }
        if (m_den.sgn() != 1) {
            // Den must be strictly positive.
            return false;
        }
        if (m_den.is_one()) {
            // The rational is an integer.
            return true;
        }
        // Num and den must be coprime.
        MPPP_MAYBE_TLS int_t g;
        gcd(g, m_num, m_den);
        return g.is_one();
    }
    /// Sign.
    /**
     * @return 0 if \p this is zero, 1 if \p this is positive, -1 if \p this is negative.
     */
    int sgn() const
    {
        return mppp::sgn(m_num);
    }
    /// Negate in-place.
    /**
     * This method will set \p this to <tt>-this</tt>.
     *
     * @return a reference to \p this.
     */
    rational &neg()
    {
        m_num.neg();
        return *this;
    }
    /// In-place absolute value.
    /**
     * This method will set \p this to its absolute value.
     *
     * @return a reference to \p this.
     */
    rational &abs()
    {
        m_num.abs();
        return *this;
    }
    /// In-place inversion.
    /**
     * This method will set \p this to its inverse.
     *
     * @return a reference to \p this.
     *
     * @throws zero_division_error if \p this is zero.
     */
    rational &inv()
    {
        if (mppp_unlikely(is_zero())) {
            throw zero_division_error("Cannot invert a zero rational");
        }
        std::swap(m_num, m_den);
        fix_den_sign(*this);
        return *this;
    }
    /// Test if the value is zero.
    /**
     * @return \p true if the value represented by \p this is 0, \p false otherwise.
     */
    bool is_zero() const
    {
        return mppp::is_zero(m_num);
    }
    /// Test if the value is one.
    /**
     * @return \p true if the value represented by \p this is 1, \p false otherwise.
     */
    bool is_one() const
    {
        return mppp::is_one(m_num) && mppp::is_one(m_den);
    }
    /// Test if the value is minus one.
    /**
     * @return \p true if the value represented by \p this is -1, \p false otherwise.
     */
    bool is_negative_one() const
    {
        return mppp::is_negative_one(m_num) && mppp::is_one(m_den);
    }

private:
    int_t m_num;
    int_t m_den;
};

#if MPPP_CPLUSPLUS < 201703L

// NOTE: see the explanation in integer.hpp regarding static constexpr variables in C++17.

template <std::size_t SSize>
constexpr std::size_t rational<SSize>::ssize;

#endif

inline namespace detail
{

// Some misc tests to check that the mpq struct conforms to our expectations.
struct expected_mpq_struct_t {
    mpz_struct_t _mp_num;
    mpz_struct_t _mp_den;
};

static_assert(sizeof(expected_mpq_struct_t) == sizeof(mpq_struct_t)
                  && offsetof(mpq_struct_t, _mp_num) == offsetof(expected_mpq_struct_t, _mp_num)
                  && offsetof(mpq_struct_t, _mp_den) == offsetof(expected_mpq_struct_t, _mp_den),
              "Invalid mpq_t struct layout.");

#if MPPP_CPLUSPLUS >= 201703L

constexpr bool test_mpq_struct_t()
{
    auto[num, den] = mpq_struct_t{};
    (void)num;
    (void)den;
    return std::is_same<decltype(num), mpz_struct_t>::value && std::is_same<decltype(den), mpz_struct_t>::value;
}

static_assert(test_mpq_struct_t(), "The mpq_struct_t does not have the expected layout.");

#endif

// Let's keep the view machinery private for now, as it suffers from the potential aliasing
// issues described in the mpz_view documentation. In this case, we have to fill in a shallow mpq_struct
// in any case, as the rational class is not backed by a regular mpq_t. We could then have aliasing
// between a real mpz_t and, say, the numerator extracted from a view which is internally pointing
// to the same mpz_t. Example:
//
// rational q;
// auto &n = q._get_num();
// n.promote();
// const auto q_view = get_mpq_view(q);
// mpz_add_ui(n.get_mpz_t(), mpq_numref(&q_view), 1);
//
// In the last line, mpq_numref() is extracting an mpz_struct from the view which internally
// points to the same limbs as the mpz_t inside n. The mpz_add_ui code will try to realloc()
// the limb pointer inside n, thus invalidating the limb pointer in the view (this would work
// if the view's mpz_struct were the same object as n's mpz_t, as the GMP code would update
// the new pointer after the realloc for both structs).
template <std::size_t SSize>
inline mpq_struct_t get_mpq_view(const rational<SSize> &q)
{
    return mpq_struct_t{*q.get_num().get_mpz_view().get(), *q.get_den().get_mpz_view().get()};
}

// Machinery for the determination of the result of a binary operation involving rational.
// Default is empty for SFINAE.
template <typename, typename, typename = void>
struct rational_common_type {
};

template <std::size_t SSize>
struct rational_common_type<rational<SSize>, rational<SSize>> {
    using type = rational<SSize>;
};

template <std::size_t SSize>
struct rational_common_type<rational<SSize>, integer<SSize>> {
    using type = rational<SSize>;
};

template <std::size_t SSize>
struct rational_common_type<integer<SSize>, rational<SSize>> {
    using type = rational<SSize>;
};

template <std::size_t SSize, typename U>
struct rational_common_type<rational<SSize>, U, enable_if_t<is_cpp_integral_interoperable<U>::value>> {
    using type = rational<SSize>;
};

template <std::size_t SSize, typename T>
struct rational_common_type<T, rational<SSize>, enable_if_t<is_cpp_integral_interoperable<T>::value>> {
    using type = rational<SSize>;
};

template <std::size_t SSize, typename U>
struct rational_common_type<rational<SSize>, U, enable_if_t<is_cpp_floating_point_interoperable<U>::value>> {
    using type = U;
};

template <std::size_t SSize, typename T>
struct rational_common_type<T, rational<SSize>, enable_if_t<is_cpp_floating_point_interoperable<T>::value>> {
    using type = T;
};

template <typename T, typename U>
using rational_common_t = typename rational_common_type<T, U>::type;

// Implementation of the rational op types concept, used in various operators.
template <typename T, typename U>
using are_rational_op_types = is_detected<rational_common_t, T, U>;

template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RationalOpTypes = are_rational_op_types<T, U>::value;
#else
using rational_op_types_enabler = enable_if_t<are_rational_op_types<T, U>::value, int>;
#endif

// Implementation of binary add/sub. The NewRop flag indicates that
// rop is a def-cted rational distinct from op1 and op2.
template <bool AddOrSub, bool NewRop, std::size_t SSize>
inline void addsub_impl(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    assert(!NewRop || (rop.is_zero() && &rop != &op1 && &rop != &op2));
    const bool u1 = op1.get_den().is_one(), u2 = op2.get_den().is_one();
    // NOTE: it's important here to take care about overlapping arguments: we cannot use
    // rop as a "temporary" storage space, because if it overlaps with op1/op2 we will be
    // altering op1/op2 as well.
    if (u1 && u2) {
        // add/sub() are fine with overlapping args.
        AddOrSub ? add(rop._get_num(), op1.get_num(), op2.get_num())
                 : sub(rop._get_num(), op1.get_num(), op2.get_num());
        if (!NewRop) {
            // Set rop's den to 1, if rop is not new (otherwise it's 1 already).
            rop._get_den().set_one();
        }
    } else if (u1) {
        if (NewRop) {
            // rop is separate, can write into it directly.
            rop._get_num() = op2.get_num();
            if (AddOrSub) {
                addmul(rop._get_num(), op1.get_num(), op2.get_den());
            } else {
                submul(rop._get_num(), op1.get_num(), op2.get_den());
                rop._get_num().neg();
            }
            // NOTE: gcd(a+m*b,b) == gcd(a,b) for every integer m, no need to canonicalise the result.
            rop._get_den() = op2.get_den();
        } else {
            integer<SSize> tmp{op2.get_num()};
            // Ok, tmp is a separate variable, won't modify ops.
            if (AddOrSub) {
                addmul(tmp, op1.get_num(), op2.get_den());
            } else {
                submul(tmp, op1.get_num(), op2.get_den());
                tmp.neg();
            }
            // Final assignments, potential for self-assignment
            // in the second one.
            rop._get_num() = std::move(tmp);
            rop._get_den() = op2.get_den();
        }
    } else if (u2) {
        // Mirror of the above.
        if (NewRop) {
            rop._get_num() = op1.get_num();
            AddOrSub ? addmul(rop._get_num(), op2.get_num(), op1.get_den())
                     : submul(rop._get_num(), op2.get_num(), op1.get_den());
            rop._get_den() = op1.get_den();
        } else {
            integer<SSize> tmp{op1.get_num()};
            AddOrSub ? addmul(tmp, op2.get_num(), op1.get_den()) : submul(tmp, op2.get_num(), op1.get_den());
            rop._get_num() = std::move(tmp);
            rop._get_den() = op1.get_den();
        }
    } else if (op1.get_den() == op2.get_den()) {
        // add()/sub() are fine with overlapping args.
        AddOrSub ? add(rop._get_num(), op1.get_num(), op2.get_num())
                 : sub(rop._get_num(), op1.get_num(), op2.get_num());
        // Set rop's den to the common den.
        rop._get_den() = op1.get_den();
        rop.canonicalise();
    } else {
        // NOTE: the algorithm here is taken from GMP's aors.c for mpq. The idea
        // is, as usual, to avoid large canonicalisations and to try to keep
        // the values as small as possible at every step. We need to do some
        // testing about this, as I am not 100% sure that this is going to be
        // a win for our small-operands focus. Let's bookmark here the previous
        // implementation, at git commit:
        // a8a397d67d6e2af43592aa99061016398a1457ad
        auto g = gcd(op1.get_den(), op2.get_den());
        if (g.is_one()) {
            // This is the case in which the two dens are coprime.
            AddOrSub ? add(rop._get_num(), op1.get_num() * op2.get_den(), op2.get_num() * op1.get_den())
                     : sub(rop._get_num(), op1.get_num() * op2.get_den(), op2.get_num() * op1.get_den());
            mul(rop._get_den(), op1.get_den(), op2.get_den());
        } else {
            // Eliminate common factors between the dens.
            auto t = divexact_gcd(op2.get_den(), g);
            auto tmp2 = divexact_gcd(op1.get_den(), g);

            // Compute the numerator (will be t).
            auto tmp1 = op1.get_num() * t;
            mul(t, op2.get_num(), tmp2);
            AddOrSub ? add(t, tmp1, t) : sub(t, tmp1, t);

            // Check if the numerator and the den GCD are coprime.
            gcd(g, t, g);
            if (g.is_one()) {
                // They are coprime: assign the num and compute the final den.
                rop._get_num() = std::move(t);
                mul(rop._get_den(), op2.get_den(), tmp2);
            } else {
                // Assign numerator, reduced by the new gcd.
                divexact_gcd(rop._get_num(), t, g);
                // Reduced version of the second den.
                divexact_gcd(tmp1, op2.get_den(), g);
                // Assign final den: tmp1 x the reduced den1.
                mul(rop._get_den(), tmp1, tmp2);
            }
        }
    }
}
}

/** @defgroup rational_conversion rational_conversion
 *  @{
 */

/// Generic conversion function for \link mppp::rational rational\endlink.
/**
 * \rststar
 * This function will convert the input :cpp:class:`~mppp::rational` ``q`` to a
 * :cpp:concept:`~mppp::RationalInteroperable` type, storing the result of the conversion into ``rop``.
 * If the conversion is successful, the function
 * will return ``true``, otherwise the function will return ``false``. If the conversion fails, ``rop`` will
 * not be altered.
 * \endrststar
 *
 * @param rop the variable which will store the result of the conversion.
 * @param q the input \link mppp::rational rational\endlink.
 *
 * @return ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail only if ``rop`` is
 * a C++ integral which cannot represent the truncated value of ``q``.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <std::size_t SSize>
inline bool get(RationalInteroperable<SSize> &rop, const rational<SSize> &q)
#else
template <typename T, std::size_t SSize, rational_interoperable_enabler<T, SSize> = 0>
inline bool get(T &rop, const rational<SSize> &q)
#endif
{
    return q.get(rop);
}

/** @} */

/** @defgroup rational_arithmetic rational_arithmetic
 *  @{
 */

/// Ternary addition.
/**
 * This function will set \p rop to <tt>op1 + op2</tt>.
 *
 * @param rop the return value.
 * @param op1 the first argument.
 * @param op2 the second argument.
 *
 * @return a reference to \p rop.
 */
template <std::size_t SSize>
inline rational<SSize> &add(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    addsub_impl<true, false>(rop, op1, op2);
    return rop;
}

/// Ternary subtraction.
/**
 * This function will set \p rop to <tt>op1 - op2</tt>.
 *
 * @param rop the return value.
 * @param op1 the first argument.
 * @param op2 the second argument.
 *
 * @return a reference to \p rop.
 */
template <std::size_t SSize>
inline rational<SSize> &sub(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    addsub_impl<false, false>(rop, op1, op2);
    return rop;
}

inline namespace detail
{
template <bool NewRop, std::size_t SSize>
inline void mul_impl(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    assert(!NewRop || rop.is_zero());
    const bool u1 = op1.get_den().is_one(), u2 = op2.get_den().is_one();
    // NOTE: it's important here to take care about overlapping arguments: we cannot use
    // rop as a "temporary" storage space, because if it overlaps with op1/op2 we will be
    // altering op1/op2 as well.
    if (u1 && u2) {
        // mul() is fine with overlapping args.
        mul(rop._get_num(), op1.get_num(), op2.get_num());
        if (!NewRop) {
            // Set rop's den to 1, if rop is not a new value (in that case, den is 1 already).
            rop._get_den().set_one();
        }
    } else if (op1.get_den() == op2.get_den()) {
        // Special case: equal dens do not require canonicalisation.
        mul(rop._get_num(), op1.get_num(), op2.get_num());
        // NOTE: we can use a squaring function here, once implemented in integer.
        mul(rop._get_den(), op1.get_den(), op2.get_den());
    } else if (u1) {
        // This is a * (b/c). Instead of doing (ab)/c and then canonicalise,
        // we remove the common factors from a and c and we perform
        // a normal multiplication (without canonicalisation). This allows us to
        // perform a gcd with smaller operands.
        auto g = gcd(op1.get_num(), op2.get_den());
        if (g.is_one()) {
            // NOTE: after this line, all nums are tainted.
            mul(rop._get_num(), op2.get_num(), op1.get_num());
            rop._get_den() = op2.get_den();
        } else {
            // NOTE: after this line, all dens are tainted.
            divexact_gcd(rop._get_den(), op2.get_den(), g);
            // Re-use g.
            divexact_gcd(g, op1.get_num(), g);
            mul(rop._get_num(), op2.get_num(), g);
        }
    } else if (u2) {
        // Mirror of the above.
        auto g = gcd(op2.get_num(), op1.get_den());
        if (g.is_one()) {
            mul(rop._get_num(), op1.get_num(), op2.get_num());
            rop._get_den() = op1.get_den();
        } else {
            divexact_gcd(rop._get_den(), op1.get_den(), g);
            divexact_gcd(g, op2.get_num(), g);
            mul(rop._get_num(), op1.get_num(), g);
        }
    } else {
        // General case: a/b * c/d
        // NOTE: like above, we don't want to canonicalise (ac)/(bd),
        // and we trade one big gcd with two smaller gcds.
        // Compute gcd(a,d) and gcd(b,c).
        const auto g1 = gcd(op1.get_num(), op2.get_den());
        const auto g2 = gcd(op1.get_den(), op2.get_num());
        // Remove common factors from the nums.
        auto tmp1 = divexact_gcd(op1.get_num(), g1);
        auto tmp2 = divexact_gcd(op2.get_num(), g2);
        // Compute rop's numerator.
        // NOTE: after this line, all nums are tainted.
        mul(rop._get_num(), tmp1, tmp2);
        // Remove common factors from the dens.
        divexact_gcd(tmp1, op2.get_den(), g1);
        divexact_gcd(tmp2, op1.get_den(), g2);
        // Compute rop's denominator.
        mul(rop._get_den(), tmp1, tmp2);
    }
}
}

/// Ternary multiplication.
/**
 * This function will set \p rop to <tt>op1 * op2</tt>.
 *
 * @param rop the return value.
 * @param op1 the first argument.
 * @param op2 the second argument.
 *
 * @return a reference to \p rop.
 */
template <std::size_t SSize>
inline rational<SSize> &mul(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    mul_impl<false>(rop, op1, op2);
    return rop;
}

/// Ternary division.
/**
 * This function will set \p rop to <tt>op1 / op2</tt>.
 *
 * @param rop the return value.
 * @param op1 the first argument.
 * @param op2 the second argument.
 *
 * @return a reference to \p rop.
 *
 * @throws zero_division_error if \p op2 is zero.
 */
template <std::size_t SSize>
inline rational<SSize> &div(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    if (mppp_unlikely(op2.is_zero())) {
        throw zero_division_error("Zero divisor in rational division");
    }
    if (mppp_unlikely(&rop == &op2)) {
        // Following the GMP algorithm, special case in which rop and op2 are the same object.
        // This allows us to use op2.get_num() safely later, even after setting rop's num, as
        // we will be sure that rop and op2 do not overlap.
        if (mppp_unlikely(&rop == &op1)) {
            // x = x/x = 1.
            rop._get_num().set_one();
            rop._get_den().set_one();
            return rop;
        }
        // Set rop to 1/rop by swapping num/den.
        // NOTE: we already checked that op2 is nonzero, so inverting it
        // does not yield division by zero.
        std::swap(rop._get_num(), rop._get_den());
        // Fix den sign.
        fix_den_sign(rop);
        // Multiply by op1.
        mul(rop, rop, op1);
        return rop;
    }
    const bool u1 = op1.get_den().is_one(), u2 = op2.get_den().is_one();
    if ((u1 && u2) || (op1.get_den() == op2.get_den())) {
        const auto g = gcd(op1.get_num(), op2.get_num());
        if (g.is_one()) {
            rop._get_num() = op1.get_num();
            // NOTE: op2 not tainted, as it's separate from rop.
            rop._get_den() = op2.get_num();
        } else {
            divexact_gcd(rop._get_num(), op1.get_num(), g);
            divexact_gcd(rop._get_den(), op2.get_num(), g);
        }
    } else if (u1) {
        // Same idea as in the mul().
        auto g = gcd(op1.get_num(), op2.get_num());
        if (g.is_one()) {
            mul(rop._get_num(), op2.get_den(), op1.get_num());
            // NOTE: op2's num is not tainted, as op2 is distinct
            // from rop.
            rop._get_den() = op2.get_num();
        } else {
            // NOTE: dens tainted after this, apart from op2's
            // as we have established earlier that rop and op2
            // are distinct.
            divexact_gcd(rop._get_den(), op2.get_num(), g);
            divexact_gcd(g, op1.get_num(), g);
            mul(rop._get_num(), op2.get_den(), g);
        }
    } else if (u2) {
        auto g = gcd(op1.get_num(), op2.get_num());
        if (g.is_one()) {
            rop._get_num() = op1.get_num();
            mul(rop._get_den(), op1.get_den(), op2.get_num());
        } else {
            divexact_gcd(rop._get_num(), op1.get_num(), g);
            divexact_gcd(g, op2.get_num(), g);
            mul(rop._get_den(), op1.get_den(), g);
        }
    } else {
        // (a/b) / (c/d) -> a/b * d/c
        // The two gcds.
        const auto g1 = gcd(op1.get_num(), op2.get_num());
        const auto g2 = gcd(op1.get_den(), op2.get_den());
        // Remove common factors.
        auto tmp1 = divexact_gcd(op1.get_num(), g1);
        auto tmp2 = divexact_gcd(op2.get_den(), g2);
        // Compute the numerator.
        mul(rop._get_num(), tmp1, tmp2);
        // Remove common factors.
        divexact_gcd(tmp1, op2.get_num(), g1);
        divexact_gcd(tmp2, op1.get_den(), g2);
        // Denominator.
        mul(rop._get_den(), tmp1, tmp2);
    }
    // Fix wrong sign in the den.
    fix_den_sign(rop);
    return rop;
}

/// Binary negation.
/**
 * This method will set \p rop to <tt>-q</tt>.
 *
 * @param rop the return value.
 * @param q the rational that will be negated.
 *
 * @return a reference to \p rop.
 */
template <std::size_t SSize>
inline rational<SSize> &neg(rational<SSize> &rop, const rational<SSize> &q)
{
    rop = q;
    return rop.neg();
}

/// Unary negation.
/**
 * @param q the rational that will be negated.
 *
 * @return <tt>-q</tt>.
 */
template <std::size_t SSize>
inline rational<SSize> neg(const rational<SSize> &q)
{
    rational<SSize> ret(q);
    ret.neg();
    return ret;
}

/// Binary absolute value.
/**
 * This function will set \p rop to the absolute value of \p q.
 *
 * @param rop the return value.
 * @param q the argument.
 *
 * @return a reference to \p rop.
 */
template <std::size_t SSize>
inline rational<SSize> &abs(rational<SSize> &rop, const rational<SSize> &q)
{
    rop = q;
    return rop.abs();
}

/// Unary absolute value.
/**
 * @param q the argument.
 *
 * @return the absolute value of \p q.
 */
template <std::size_t SSize>
inline rational<SSize> abs(const rational<SSize> &q)
{
    rational<SSize> ret(q);
    ret.abs();
    return ret;
}

/// Binary inversion.
/**
 * This function will set \p rop to the inverse of \p q.
 *
 * @param rop the return value.
 * @param q the argument.
 *
 * @return a reference to \p rop.
 *
 * @throws unspecified any exception thrown by mppp::rational::inv().
 */
template <std::size_t SSize>
inline rational<SSize> &inv(rational<SSize> &rop, const rational<SSize> &q)
{
    rop = q;
    return rop.inv();
}

/// Unary inversion.
/**
 * @param q the argument.
 *
 * @return the inverse of \p q.
 *
 * @throws unspecified any exception thrown by mppp::rational::inv().
 */
template <std::size_t SSize>
inline rational<SSize> inv(const rational<SSize> &q)
{
    rational<SSize> ret(q);
    ret.inv();
    return ret;
}

/** @} */

/** @defgroup rational_io rational_io
 *  @{
 */

/// Output stream operator.
/**
 * \rststar
 * This operator will print to the stream ``os`` the :cpp:class:`~mppp::rational` ``q`` in base 10.
 * The printing format is described in :cpp:func:`mppp::rational::to_string()`.
 * \endrststar
 *
 * @param os the target stream.
 * @param q the input rational.
 *
 * @return a reference to \p os.
 *
 * @throws unspecified any exception thrown by mppp::rational::to_string().
 */
template <std::size_t SSize>
inline std::ostream &operator<<(std::ostream &os, const rational<SSize> &q)
{
    return os << q.to_string();
}

/// Input stream operator.
/**
 * \rststar
 * This operator is equivalent to extracting a line from the stream and assigning it to ``q``.
 * \endrststar
 *
 * @param is the input stream.
 * @param q the rational to which the string extracted from the stream will be assigned.
 *
 * @return a reference to \p is.
 *
 * @throws unspecified any exception thrown by \link mppp::rational rational\endlink's assignment operator from string.
 */
template <std::size_t SSize>
inline std::istream &operator>>(std::istream &is, rational<SSize> &q)
{
    MPPP_MAYBE_TLS std::string tmp_str;
    std::getline(is, tmp_str);
    q = tmp_str;
    return is;
}

/** @} */

/** @defgroup rational_operators rational_operators
 *  @{
 */

/// Identity operator.
/**
 * @param q the rational that will be copied.
 *
 * @return a copy of \p q.
 */
template <std::size_t SSize>
inline rational<SSize> operator+(const rational<SSize> &q)
{
    return q;
}

inline namespace detail
{

// Dispatching for the binary addition operator.
template <std::size_t SSize>
inline rational<SSize> dispatch_binary_add(const rational<SSize> &op1, const rational<SSize> &op2)
{
    rational<SSize> retval;
    addsub_impl<true, true>(retval, op1, op2);
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_add(const rational<SSize> &op1, const integer<SSize> &op2)
{
    rational<SSize> retval{op1};
    if (op1.get_den().is_one()) {
        add(retval._get_num(), retval._get_num(), op2);
    } else {
        addmul(retval._get_num(), retval.get_den(), op2);
    }
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_add(const integer<SSize> &op1, const rational<SSize> &op2)
{
    return dispatch_binary_add(op2, op1);
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_add(const rational<SSize> &op1, T n)
{
    return dispatch_binary_add(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_add(T n, const rational<SSize> &op2)
{
    return dispatch_binary_add(op2, n);
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline T dispatch_binary_add(const rational<SSize> &op1, T x)
{
    return static_cast<T>(op1) + x;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline T dispatch_binary_add(T x, const rational<SSize> &op2)
{
    return dispatch_binary_add(op2, x);
}
}

/// Binary addition operator.
/**
 * \rststar
 * The return type is determined as follows:
 *
 * * if the non-:cpp:class:`~mppp::rational` argument is a floating-point type ``F``, then the
 *   type of the result is ``F``; otherwise,
 * * the type of the result is a :cpp:class:`~mppp::rational`.
 *
 * \endrststar
 *
 * @param op1 the first summand.
 * @param op2 the second summand.
 *
 * @return <tt>op1 + op2</tt>.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline auto operator+(const RationalOpTypes<T> &op1, const T &op2)
#else
template <typename T, typename U>
inline rational_common_t<T, U> operator+(const T &op1, const U &op2)
#endif
{
    return dispatch_binary_add(op1, op2);
}

inline namespace detail
{

// Dispatching for in-place add.
template <std::size_t SSize>
inline void dispatch_in_place_add(rational<SSize> &retval, const rational<SSize> &q)
{
    add(retval, retval, q);
}

template <std::size_t SSize>
inline void dispatch_in_place_add(rational<SSize> &retval, const integer<SSize> &n)
{
    if (retval.get_den().is_one()) {
        add(retval._get_num(), retval._get_num(), n);
    } else {
        addmul(retval._get_num(), retval.get_den(), n);
    }
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_add(rational<SSize> &retval, const T &n)
{
    dispatch_in_place_add(retval, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_add(rational<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) + x;
}

template <typename T, std::size_t SSize, enable_if_t<is_rational_interoperable<T, SSize>::value, int> = 0>
inline void dispatch_in_place_add(T &rop, const rational<SSize> &op)
{
    rop = static_cast<T>(rop + op);
}
}

/// In-place addition operator.
/**
 * @param rop the augend.
 * @param op the addend.
 *
 * @return a reference to \p rop.
 *
 * @throws unspecified any exception thrown by the assignment of a floating-point value to \p rop or
 * by the conversion operator of \link mppp::rational rational\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline auto &operator+=(RationalOpTypes<T> &rop, const T &op)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline T &operator+=(T &rop, const U &op)
#endif
{
    dispatch_in_place_add(rop, op);
    return rop;
}

/// Prefix increment for \link mppp::rational rational\endlink.
/**
 * This operator will increment \p q by one.
 *
 * @param q the \link mppp::rational rational\endlink that will be increased.
 *
 * @return a reference to \p q after the increment.
 */
template <std::size_t SSize>
inline rational<SSize> &operator++(rational<SSize> &q)
{
    add(q._get_num(), q._get_num(), q.get_den());
    return q;
}

/// Suffix increment for \link mppp::rational rational\endlink.
/**
 * This operator will increment \p q by one and return a copy of \p q as it was before the increment.
 *
 * @param q the \link mppp::rational rational\endlink that will be increased.
 *
 * @return a copy of \p q before the increment.
 */
template <std::size_t SSize>
inline rational<SSize> operator++(rational<SSize> &q, int)
{
    auto retval(q);
    ++q;
    return retval;
}

/// Negated copy.
/**
 * @param q the rational that will be negated.
 *
 * @return a negated copy of \p q.
 */
template <std::size_t SSize>
inline rational<SSize> operator-(const rational<SSize> &q)
{
    auto retval(q);
    retval.neg();
    return retval;
}

inline namespace detail
{

// Dispatching for the binary subtraction operator.
template <std::size_t SSize>
inline rational<SSize> dispatch_binary_sub(const rational<SSize> &op1, const rational<SSize> &op2)
{
    rational<SSize> retval;
    addsub_impl<false, true>(retval, op1, op2);
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_sub(const rational<SSize> &op1, const integer<SSize> &op2)
{
    rational<SSize> retval{op1};
    if (op1.get_den().is_one()) {
        sub(retval._get_num(), retval._get_num(), op2);
    } else {
        submul(retval._get_num(), retval.get_den(), op2);
    }
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_sub(const integer<SSize> &op1, const rational<SSize> &op2)
{
    auto retval = dispatch_binary_sub(op2, op1);
    retval.neg();
    return retval;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_sub(const rational<SSize> &op1, T n)
{
    return dispatch_binary_sub(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_sub(T n, const rational<SSize> &op2)
{
    auto retval = dispatch_binary_sub(op2, n);
    retval.neg();
    return retval;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline T dispatch_binary_sub(const rational<SSize> &op1, T x)
{
    return static_cast<T>(op1) - x;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline T dispatch_binary_sub(T x, const rational<SSize> &op2)
{
    return -dispatch_binary_sub(op2, x);
}
}

/// Binary subtraction operator.
/**
 * \rststar
 * The return type is determined as follows:
 *
 * * if the non-:cpp:class:`~mppp::rational` argument is a floating-point type ``F``, then the
 *   type of the result is ``F``; otherwise,
 * * the type of the result is a :cpp:class:`~mppp::rational`.
 *
 * \endrststar
 *
 * @param op1 the first operand.
 * @param op2 the second operand.
 *
 * @return <tt>op1 - op2</tt>.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline auto operator-(const RationalOpTypes<T> &op1, const T &op2)
#else
template <typename T, typename U>
inline rational_common_t<T, U> operator-(const T &op1, const U &op2)
#endif
{
    return dispatch_binary_sub(op1, op2);
}

inline namespace detail
{

// Dispatching for in-place sub.
template <std::size_t SSize>
inline void dispatch_in_place_sub(rational<SSize> &retval, const rational<SSize> &q)
{
    sub(retval, retval, q);
}

template <std::size_t SSize>
inline void dispatch_in_place_sub(rational<SSize> &retval, const integer<SSize> &n)
{
    if (retval.get_den().is_one()) {
        sub(retval._get_num(), retval._get_num(), n);
    } else {
        submul(retval._get_num(), retval.get_den(), n);
    }
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_sub(rational<SSize> &retval, const T &n)
{
    dispatch_in_place_sub(retval, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_sub(rational<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) - x;
}

template <typename T, std::size_t SSize, enable_if_t<is_rational_interoperable<T, SSize>::value, int> = 0>
inline void dispatch_in_place_sub(T &rop, const rational<SSize> &op)
{
    rop = static_cast<T>(rop - op);
}
}

/// In-place subtraction operator.
/**
 * @param rop the minuend.
 * @param op the subtrahend.
 *
 * @return a reference to \p rop.
 *
 * @throws unspecified any exception thrown by the assignment of a floating-point value to \p rop or
 * by the conversion operator of \link mppp::rational rational\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline auto &operator-=(RationalOpTypes<T> &rop, const T &op)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline T &operator-=(T &rop, const U &op)
#endif
{
    dispatch_in_place_sub(rop, op);
    return rop;
}

/// Prefix decrement for \link mppp::rational rational\endlink.
/**
 * This operator will decrement \p q by one.
 *
 * @param q the \link mppp::rational rational\endlink that will be decreased.
 *
 * @return a reference to \p q after the decrement.
 */
template <std::size_t SSize>
inline rational<SSize> &operator--(rational<SSize> &q)
{
    sub(q._get_num(), q._get_num(), q.get_den());
    return q;
}

/// Suffix decrement for \link mppp::rational rational\endlink.
/**
 * This operator will decrement \p q by one and return a copy of \p q as it was before the decrement.
 *
 * @param q the \link mppp::rational rational\endlink that will be decreased.
 *
 * @return a copy of \p q before the decrement.
 */
template <std::size_t SSize>
inline rational<SSize> operator--(rational<SSize> &q, int)
{
    auto retval(q);
    --q;
    return retval;
}

inline namespace detail
{

// Dispatching for the binary multiplication operator.
template <std::size_t SSize>
inline rational<SSize> dispatch_binary_mul(const rational<SSize> &op1, const rational<SSize> &op2)
{
    rational<SSize> retval;
    mul_impl<true>(retval, op1, op2);
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_mul(const rational<SSize> &op1, const integer<SSize> &op2)
{
    rational<SSize> retval;
    if (op1.get_den().is_one()) {
        mul(retval._get_num(), op1.get_num(), op2);
    } else {
        auto g = gcd(op1.get_den(), op2);
        if (g.is_one()) {
            // Nums will be tainted after this.
            mul(retval._get_num(), op1.get_num(), op2);
            retval._get_den() = op1.get_den();
        } else {
            // Set the den first. Dens tainted after this.
            divexact_gcd(retval._get_den(), op1.get_den(), g);
            // Re-use the g variable as tmp storage.
            divexact_gcd(g, op2, g);
            mul(retval._get_num(), op1.get_num(), g);
        }
    }
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_mul(const integer<SSize> &op1, const rational<SSize> &op2)
{
    return dispatch_binary_mul(op2, op1);
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_mul(const rational<SSize> &op1, T n)
{
    return dispatch_binary_mul(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_mul(T n, const rational<SSize> &op2)
{
    return dispatch_binary_mul(op2, n);
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline T dispatch_binary_mul(const rational<SSize> &op1, T x)
{
    return static_cast<T>(op1) * x;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline T dispatch_binary_mul(T x, const rational<SSize> &op2)
{
    return dispatch_binary_mul(op2, x);
}
}

/// Binary multiplication operator.
/**
 * \rststar
 * The return type is determined as follows:
 *
 * * if the non-:cpp:class:`~mppp::rational` argument is a floating-point type ``F``, then the
 *   type of the result is ``F``; otherwise,
 * * the type of the result is a :cpp:class:`~mppp::rational`.
 *
 * \endrststar
 *
 * @param op1 the first factor.
 * @param op2 the second factor.
 *
 * @return <tt>op1 * op2</tt>.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline auto operator*(const RationalOpTypes<T> &op1, const T &op2)
#else
template <typename T, typename U>
inline rational_common_t<T, U> operator*(const T &op1, const U &op2)
#endif
{
    return dispatch_binary_mul(op1, op2);
}

inline namespace detail
{

// Dispatching for in-place mul.
template <std::size_t SSize>
inline void dispatch_in_place_mul(rational<SSize> &retval, const rational<SSize> &q)
{
    mul(retval, retval, q);
}

template <std::size_t SSize>
inline void dispatch_in_place_mul(rational<SSize> &retval, const integer<SSize> &n)
{
    if (retval.get_den().is_one()) {
        // Integer multiplication.
        mul(retval._get_num(), retval.get_num(), n);
    } else {
        auto g = gcd(retval.get_den(), n);
        if (g.is_one()) {
            // No common factors, just multiply the numerators. Den is already
            // assigned.
            mul(retval._get_num(), retval.get_num(), n);
        } else {
            // Set the den first. Dens tainted after this.
            divexact_gcd(retval._get_den(), retval.get_den(), g);
            // Re-use the g variable as tmp storage.
            divexact_gcd(g, n, g);
            mul(retval._get_num(), retval.get_num(), g);
        }
    }
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_mul(rational<SSize> &retval, const T &n)
{
    dispatch_in_place_mul(retval, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_mul(rational<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) * x;
}

template <typename T, std::size_t SSize, enable_if_t<is_rational_interoperable<T, SSize>::value, int> = 0>
inline void dispatch_in_place_mul(T &rop, const rational<SSize> &op)
{
    rop = static_cast<T>(rop * op);
}
}

/// In-place multiplication operator.
/**
 * @param rop the multiplicand.
 * @param op the multiplicator.
 *
 * @return a reference to \p rop.
 *
 * @throws unspecified any exception thrown by the assignment of a floating-point value to \p rop or
 * by the conversion operator of \link mppp::rational rational\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline auto &operator*=(RationalOpTypes<T> &rop, const T &op)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline T &operator*=(T &rop, const U &op)
#endif
{
    dispatch_in_place_mul(rop, op);
    return rop;
}

inline namespace detail
{

// Dispatching for the binary division operator.
template <std::size_t SSize>
inline rational<SSize> dispatch_binary_div(const rational<SSize> &op1, const rational<SSize> &op2)
{
    rational<SSize> retval;
    div(retval, op1, op2);
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_div(const rational<SSize> &op1, const integer<SSize> &op2)
{
    if (mppp_unlikely(op2.is_zero())) {
        throw zero_division_error("Zero divisor in rational division");
    }
    rational<SSize> retval;
    auto g = gcd(op1.get_num(), op2);
    if (op1.get_den().is_one()) {
        if (g.is_one()) {
            retval._get_num() = op1.get_num();
            retval._get_den() = op2;
        } else {
            divexact_gcd(retval._get_num(), op1.get_num(), g);
            divexact_gcd(retval._get_den(), op2, g);
        }
    } else {
        if (g.is_one()) {
            retval._get_num() = op1.get_num();
            mul(retval._get_den(), op1.get_den(), op2);
        } else {
            // Set the num first.
            divexact_gcd(retval._get_num(), op1.get_num(), g);
            // Re-use the g variable as tmp storage.
            divexact_gcd(g, op2, g);
            mul(retval._get_den(), op1.get_den(), g);
        }
    }
    // Fix den sign.
    fix_den_sign(retval);
    return retval;
}

// NOTE: could not find a way to easily share the implementation above, so there's some repetition here.
template <std::size_t SSize>
inline rational<SSize> dispatch_binary_div(const integer<SSize> &op1, const rational<SSize> &op2)
{
    if (mppp_unlikely(op2.is_zero())) {
        throw zero_division_error("Zero divisor in rational division");
    }
    rational<SSize> retval;
    auto g = gcd(op1, op2.get_num());
    if (op2.get_den().is_one()) {
        if (g.is_one()) {
            retval._get_num() = op1;
            retval._get_den() = op2.get_num();
        } else {
            divexact_gcd(retval._get_num(), op1, g);
            divexact_gcd(retval._get_den(), op2.get_num(), g);
        }
    } else {
        if (g.is_one()) {
            mul(retval._get_num(), op1, op2.get_den());
            retval._get_den() = op2.get_num();
        } else {
            divexact_gcd(retval._get_den(), op2.get_num(), g);
            divexact_gcd(g, op1, g);
            mul(retval._get_num(), op2.get_den(), g);
        }
    }
    // Fix den sign.
    fix_den_sign(retval);
    return retval;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_div(const rational<SSize> &op1, T n)
{
    return dispatch_binary_div(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_div(T n, const rational<SSize> &op2)
{
    return dispatch_binary_div(integer<SSize>{n}, op2);
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline T dispatch_binary_div(const rational<SSize> &op1, T x)
{
    return static_cast<T>(op1) / x;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline T dispatch_binary_div(T x, const rational<SSize> &op2)
{
    return x / static_cast<T>(op2);
}
}

/// Binary division operator.
/**
 * \rststar
 * The return type is determined as follows:
 *
 * * if the non-:cpp:class:`~mppp::rational` argument is a floating-point type ``F``, then the
 *   type of the result is ``F``; otherwise,
 * * the type of the result is a :cpp:class:`~mppp::rational`.
 *
 * \endrststar
 *
 * @param op1 the dividend.
 * @param op2 the divisor.
 *
 * @return <tt>op1 / op2</tt>.
 *
 * @throws zero_division_error if the division does not involve floating-point types and \p op2 is zero.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline auto operator/(const RationalOpTypes<T> &op1, const T &op2)
#else
template <typename T, typename U>
inline rational_common_t<T, U> operator/(const T &op1, const U &op2)
#endif
{
    return dispatch_binary_div(op1, op2);
}

inline namespace detail
{

// Dispatching for in-place div.
template <std::size_t SSize>
inline void dispatch_in_place_div(rational<SSize> &retval, const rational<SSize> &q)
{
    div(retval, retval, q);
}

template <std::size_t SSize>
inline void dispatch_in_place_div(rational<SSize> &retval, const integer<SSize> &n)
{
    if (mppp_unlikely(n.is_zero())) {
        throw zero_division_error("Zero divisor in rational division");
    }
    auto g = gcd(retval.get_num(), n);
    if (retval.get_den().is_one()) {
        if (g.is_one()) {
            retval._get_den() = n;
        } else {
            divexact_gcd(retval._get_num(), retval.get_num(), g);
            divexact_gcd(retval._get_den(), n, g);
        }
    } else {
        if (g.is_one()) {
            mul(retval._get_den(), retval.get_den(), n);
        } else {
            // Set the num first.
            divexact_gcd(retval._get_num(), retval.get_num(), g);
            // Re-use the g variable as tmp storage.
            divexact_gcd(g, n, g);
            mul(retval._get_den(), retval.get_den(), g);
        }
    }
    // Fix den sign.
    fix_den_sign(retval);
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_div(rational<SSize> &retval, const T &n)
{
    dispatch_in_place_div(retval, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_div(rational<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) / x;
}

template <typename T, std::size_t SSize, enable_if_t<is_rational_interoperable<T, SSize>::value, int> = 0>
inline void dispatch_in_place_div(T &rop, const rational<SSize> &op)
{
    rop = static_cast<T>(rop / op);
}
}

/// In-place division operator.
/**
 * @param rop the dividend.
 * @param op the divisor.
 *
 * @return a reference to \p rop.
 *
 * @throws zero_division_error if \p op is zero and only integral types are involved in the division.
 * @throws unspecified any exception thrown by the assignment of a floating-point value to \p rop or
 * by the conversion operator of \link mppp::rational rational\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline auto &operator/=(RationalOpTypes<T> &rop, const T &op)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline T &operator/=(T &rop, const U &op)
#endif
{
    dispatch_in_place_div(rop, op);
    return rop;
}

inline namespace detail
{

template <std::size_t SSize>
inline bool dispatch_equality(const rational<SSize> &op1, const rational<SSize> &op2)
{
    return op1.get_num() == op2.get_num() && op1.get_den() == op2.get_den();
}

template <std::size_t SSize, typename T, enable_if_t<is_rational_integral_interoperable<T, SSize>::value, int> = 0>
inline bool dispatch_equality(const rational<SSize> &op1, const T &op2)
{
    return op1.get_den().is_one() && op1.get_num() == op2;
}

template <std::size_t SSize, typename T, enable_if_t<is_rational_integral_interoperable<T, SSize>::value, int> = 0>
inline bool dispatch_equality(const T &op1, const rational<SSize> &op2)
{
    return dispatch_equality(op2, op1);
}

template <std::size_t SSize, typename T, enable_if_t<!is_rational_integral_interoperable<T, SSize>::value, int> = 0>
inline bool dispatch_equality(const rational<SSize> &op1, const T &op2)
{
    return static_cast<T>(op1) == op2;
}

template <std::size_t SSize, typename T, enable_if_t<!is_rational_integral_interoperable<T, SSize>::value, int> = 0>
inline bool dispatch_equality(const T &op1, const rational<SSize> &op2)
{
    return dispatch_equality(op2, op1);
}
}

/// Equality operator.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p true if <tt>op1 == op2</tt>, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator==(const RationalOpTypes<T> &op1, const T &op2)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline bool operator==(const T &op1, const U &op2)
#endif
{
    return dispatch_equality(op1, op2);
}

/// Inequality operator.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p true if <tt>op1 != op2</tt>, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator!=(const RationalOpTypes<T> &op1, const T &op2)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline bool operator!=(const T &op1, const U &op2)
#endif
{
    return !(op1 == op2);
}

inline namespace detail
{

// Less-than operator.
template <std::size_t SSize>
inline bool dispatch_less_than(const rational<SSize> &a, const rational<SSize> &b)
{
    return cmp(a, b) < 0;
}

template <std::size_t SSize>
inline bool dispatch_less_than(const rational<SSize> &a, const integer<SSize> &b)
{
    return cmp(a, b) < 0;
}

template <std::size_t SSize>
inline bool dispatch_less_than(const integer<SSize> &a, const rational<SSize> &b)
{
    return cmp(a, b) < 0;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline bool dispatch_less_than(const rational<SSize> &a, T n)
{
    return dispatch_less_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline bool dispatch_less_than(T n, const rational<SSize> &a)
{
    return dispatch_greater_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline bool dispatch_less_than(const rational<SSize> &a, T x)
{
    return static_cast<T>(a) < x;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline bool dispatch_less_than(T x, const rational<SSize> &a)
{
    return dispatch_greater_than(a, x);
}

// Greater-than operator.
template <std::size_t SSize>
inline bool dispatch_greater_than(const rational<SSize> &a, const rational<SSize> &b)
{
    return cmp(a, b) > 0;
}

template <std::size_t SSize>
inline bool dispatch_greater_than(const rational<SSize> &a, const integer<SSize> &b)
{
    return cmp(a, b) > 0;
}

template <std::size_t SSize>
inline bool dispatch_greater_than(const integer<SSize> &a, const rational<SSize> &b)
{
    return cmp(a, b) > 0;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline bool dispatch_greater_than(const rational<SSize> &a, T n)
{
    return dispatch_greater_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral_interoperable<T>::value, int> = 0>
inline bool dispatch_greater_than(T n, const rational<SSize> &a)
{
    return dispatch_less_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline bool dispatch_greater_than(const rational<SSize> &a, T x)
{
    return static_cast<T>(a) > x;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline bool dispatch_greater_than(T x, const rational<SSize> &a)
{
    return dispatch_less_than(a, x);
}
}

/// Less-than operator.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p true if <tt>op1 < op2</tt>, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator<(const RationalOpTypes<T> &op1, const T &op2)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline bool operator<(const T &op1, const U &op2)
#endif
{
    return dispatch_less_than(op1, op2);
}

/// Less-than or equal operator.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p true if <tt>op1 <= op2</tt>, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator<=(const RationalOpTypes<T> &op1, const T &op2)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline bool operator<=(const T &op1, const U &op2)
#endif
{
    return !(op1 > op2);
}

/// Greater-than operator.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p true if <tt>op1 > op2</tt>, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator>(const RationalOpTypes<T> &op1, const T &op2)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline bool operator>(const T &op1, const U &op2)
#endif
{
    return dispatch_greater_than(op1, op2);
}

/// Greater-than or equal operator.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p true if <tt>op1 >= op2</tt>, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator>=(const RationalOpTypes<T> &op1, const T &op2)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline bool operator>=(const T &op1, const U &op2)
#endif
{
    return !(op1 < op2);
}

/** @} */

/** @defgroup rational_comparison rational_comparison
 *  @{
 */

/// Comparison function for rationals.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p 0 if <tt>op1 == op2</tt>, a negative value if <tt>op1 < op2</tt>, a positive value if
 * <tt>op1 > op2</tt>.
 */
template <std::size_t SSize>
inline int cmp(const rational<SSize> &op1, const rational<SSize> &op2)
{
    // NOTE: here we have potential for 2 views referring to the same underlying
    // object. The same potential issues as described in the mpz_view class may arise.
    // Keep an eye on it.
    // NOTE: this can probably be improved by implementing the same strategy as in ::mpq_cmp()
    // but based on our primitives:
    // - if op1 and op2 are integers, compare the nums,
    // - try to see if the limb/bit sizes of nums and dens can tell use immediately which
    //   number is larger,
    // - otherwise, do the two multiplications and compare.
    const auto v1 = get_mpq_view(op1);
    const auto v2 = get_mpq_view(op2);
    return ::mpq_cmp(&v1, &v2);
}

/// Comparison function for rational/integer arguments.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p 0 if <tt>op1 == op2</tt>, a negative value if <tt>op1 < op2</tt>, a positive value if
 * <tt>op1 > op2</tt>.
 */
template <std::size_t SSize>
inline int cmp(const rational<SSize> &op1, const integer<SSize> &op2)
{
    // NOTE: mpq_cmp_z() is a macro or a function, depending on the GMP version. Don't
    // call it with "::".
    const auto v1 = get_mpq_view(op1);
    return mpq_cmp_z(&v1, op2.get_mpz_view());
}

/// Comparison function for integer/rational arguments.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p 0 if <tt>op1 == op2</tt>, a negative value if <tt>op1 < op2</tt>, a positive value if
 * <tt>op1 > op2</tt>.
 */
template <std::size_t SSize>
inline int cmp(const integer<SSize> &op1, const rational<SSize> &op2)
{
    // NOTE: this will clamp the return value in the [-1,1] range,
    // so not exactly identical to the GMP return value. It's still consistent
    // with the rest of GMP/mp++ rational comparison functions.
    // NOTE: we don't take directly the negative because it's not clear
    // how large the returned value could be. Like this, we prevent any
    // possible integral overflow shenanigans.
    return -integral_sign(cmp(op2, op1));
}

/// Sign function.
/**
 * @param q the rational whose sign will be computed.
 *
 * @return 0 if \p q is zero, 1 if \p q is positive, -1 if \p q is negative.
 */
template <std::size_t SSize>
int sgn(const rational<SSize> &q)
{
    return q.sgn();
}

/// Test if a rational is one.
/**
 * @param q the rational to be tested.
 *
 * @return \p true if \p q is 1, \p false otherwise.
 */
template <std::size_t SSize>
inline bool is_one(const rational<SSize> &q)
{
    return q.is_one();
}

/// Test if a rational is minus one.
/**
 * @param q the rational to be tested.
 *
 * @return \p true if \p q is -1, \p false otherwise.
 */
template <std::size_t SSize>
inline bool is_negative_one(const rational<SSize> &q)
{
    return q.is_negative_one();
}

/// Test if a rational is zero.
/**
 * @param q the rational to be tested.
 *
 * @return \p true if \p q is 0, \p false otherwise.
 */
template <std::size_t SSize>
inline bool is_zero(const rational<SSize> &q)
{
    return q.is_zero();
}

/** @} */

/** @defgroup rational_ntheory rational_ntheory
 *  @{
 */

inline namespace detail
{

// binomial() implementation.
// rational - integer overload.
// NOTE: this is very simplistic and probably really slow. Need to investigate
// better ways of doing this.
template <std::size_t SSize>
inline rational<SSize> rational_binomial_impl(const rational<SSize> &t, const integer<SSize> &b)
{
    if (t.get_den().is_one()) {
        // If top is an integer, forward to the integer overload of binomial().
        return rational<SSize>{binomial(t.get_num(), b)};
    }
    const auto b_sgn = b.sgn();
    if (b_sgn == -1) {
        // (rational negative-int) will always give zero.
        return rational<SSize>{};
    }
    if (b_sgn == 0) {
        // Zero at bottom results always in 1.
        return rational<SSize>{1};
    }
    // Falling factorial implementation.
    rational<SSize> tmp{t}, retval = t / b;
    integer<SSize> i{b};
    for (--i, --tmp; i.sgn() == 1; --i, --tmp) {
        retval *= tmp;
        retval /= i;
    }
    return retval;
}

// rational - integral overload.
template <std::size_t SSize, typename T>
inline rational<SSize> rational_binomial_impl(const rational<SSize> &t, const T &b)
{
    return rational_binomial_impl(t, integer<SSize>{b});
}
}

/// Binomial coefficient for \link mppp::rational rational\endlink.
/**
 * \rststar
 * This function will compute the binomial coefficient :math:`{x \choose y}`. If ``x``
 * represents an integral value, the calculation is forwarded to the implementation of
 * the binomial coefficient for :cpp:class:`~mppp::integer`. Otherwise, an implementation
 * based on the falling factorial is employed.
 * \endrststar
 *
 * @param x the top value.
 * @param y the bottom value.
 *
 * @return \f$ {x \choose y} \f$.
 *
 * @throws unspecified any exception thrown by the implementation of
 * the binomial coefficient for \link mppp::integer integer\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <std::size_t SSize>
inline rational<SSize> binomial(const rational<SSize> &x, const RationalIntegralInteroperable<SSize> &y)
#else
template <std::size_t SSize, typename T, rational_integral_interoperable_enabler<T, SSize> = 0>
inline rational<SSize> binomial(const rational<SSize> &x, const T &y)
#endif
{
    return rational_binomial_impl(x, y);
}

/** @} */

/** @defgroup rational_exponentiation rational_exponentiation
 *  @{
 */

inline namespace detail
{

// rational base, integral exponent implementation. Assumes exp is non-null.
template <std::size_t SSize, typename T>
inline rational<SSize> pow_impl_impl(const rational<SSize> &base, const T &exp, int exp_sign)
{
    assert(exp_sign != 0);
    rational<SSize> retval;
    if (exp_sign == 1) {
        retval._get_num() = pow(base.get_num(), exp);
        retval._get_den() = pow(base.get_den(), exp);
    } else {
        // Make integer copy of exp for safe negation.
        typename rational<SSize>::int_t exp_copy(exp);
        exp_copy.neg();
        retval._get_num() = pow(base.get_den(), exp_copy);
        retval._get_den() = pow(base.get_num(), exp_copy);
        fix_den_sign(retval);
    }
    return retval;
}

// rational base, rational exponent implementation. Works only if exponent is an integral value, will call the other
// impl_impl overload or error out.
template <std::size_t SSize>
inline rational<SSize> pow_impl_impl(const rational<SSize> &base, const rational<SSize> &exp, int exp_sign)
{
    if (mppp_unlikely(!is_one(exp.get_den()))) {
        throw std::domain_error("Cannot raise the rational base " + base.to_string() + " to the non-integral exponent "
                                + exp.to_string());
    }
    return pow_impl_impl(base, exp.get_num(), exp_sign);
}

// Rational base, integral or rational exp.
template <
    std::size_t SSize, typename T,
    enable_if_t<disjunction<is_rational_integral_interoperable<T, SSize>, std::is_same<T, rational<SSize>>>::value,
                int> = 0>
inline rational<SSize> pow_impl(const rational<SSize> &base, const T &exp)
{
    const auto exp_sign = sgn(exp);
    // Handle special cases first.
    if (is_one(base) || exp_sign == 0) {
        // 1**q == 1 ∀ q, q**0 == 1 ∀ q.
        return rational<SSize>{1};
    }
    if (is_zero(base)) {
        if (exp_sign == 1) {
            // 0**q == 0 ∀ q > 0.
            return rational<SSize>{};
        }
        // 0**q with q < 0 -> division by zero.
        throw zero_division_error("Cannot raise rational zero to the negative exponent " + to_string(exp));
    }
    return pow_impl_impl(base, exp, exp_sign);
}

// Integral base, rational exponent.
template <std::size_t SSize, typename T, enable_if_t<is_rational_integral_interoperable<T, SSize>::value, int> = 0>
inline rational<SSize> pow_impl(const T &base, const rational<SSize> &exp)
{
    return pow_impl(rational<SSize>{base}, exp);
}

// Fp base, rational exponent.
template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline T pow_impl(const rational<SSize> &base, const T &exp)
{
    return std::pow(static_cast<T>(base), exp);
}

// Rational base, fp exponent.
template <std::size_t SSize, typename T, enable_if_t<is_cpp_floating_point_interoperable<T>::value, int> = 0>
inline T pow_impl(const T &base, const rational<SSize> &exp)
{
    return std::pow(base, static_cast<T>(exp));
}
}

/// Binary exponentiation.
/**
 * \rststar
 * This function will raise ``base`` to the power ``exp``, and return the result. If one of the arguments
 * is a floating-point value, then the result will be computed via ``std::pow()`` and it will also be a
 * floating-point value. Otherwise, the result will be a :cpp:class:`~mppp::rational`.
 *
 * When floating-point types are not involved, the implementation is based on the integral exponentiation
 * of numerator and denominator. Thus, if ``exp`` is a rational value, the exponentiation will be successful
 * only in a few special cases (e.g., unitary base, zero exponent, etc.).
 * \endrststar
 *
 * @param base the base.
 * @param exp the exponent.
 *
 * @return <tt>base**exp</tt>.
 *
 * @throws zero_division_error if floating-point types are not involved, \p base is zero and \p exp is negative.
 * @throws std::domain_error if floating-point types are not involved and \p exp is a rational value (except
 * in a handful of special cases).
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline auto pow(const RationalOpTypes<T> &base, const T &exp)
#else
template <typename T, typename U>
inline rational_common_t<T, U> pow(const T &base, const U &exp)
#endif
{
    return pow_impl(base, exp);
}

/** @} */

/** @defgroup rational_other rational_other
 *  @{
 */

/// Canonicalise.
/**
 * \rststar
 * This function will put ``rop`` in canonical form. Internally, this function will employ
 * :cpp:func:`mppp::rational::canonicalise()`.
 * \endrststar
 *
 * @param rop the rational that will be canonicalised.
 *
 * @return a reference to \p rop.
 */
template <std::size_t SSize>
inline rational<SSize> &canonicalise(rational<SSize> &rop)
{
    return rop.canonicalise();
}

/// Hash value.
/**
 * \rststar
 * This function will return a hash value for ``q``.
 *
 * A specialisation of the standard ``std::hash`` functor is also provided, so that it is possible to use
 * :cpp:class:`~mppp::rational` in standard unordered associative containers out of the box.
 * \endrststar
 *
 * @param q the rational whose hash value will be computed.
 *
 * @return a hash value for \p q.
 */
template <std::size_t SSize>
inline std::size_t hash(const rational<SSize> &q)
{
    // NOTE: just return the sum of the hashes. We are already doing
    // some hashing in the integers, hopefully this is enough to obtain
    // decent hashing on the rational as well.
    return hash(q.get_num()) + hash(q.get_den());
}

/** @} */
}

namespace std
{

template <size_t SSize>
struct hash<mppp::rational<SSize>> {
#if MPPP_CPLUSPLUS < 201703L
    using argument_type = mppp::rational<SSize>;
    using result_type = size_t;
#endif
    size_t operator()(const mppp::rational<SSize> &q) const
    {
        return mppp::hash(q);
    }
};
}

#endif
