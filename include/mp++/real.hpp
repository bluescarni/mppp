// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_REAL_HPP
#define MPPP_REAL_HPP

#include <mp++/config.hpp>

#if defined(MPPP_WITH_MPFR)

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#if defined(MPPP_WITH_BOOST_S11N)

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/tracking.hpp>

#endif

#include <mp++/concepts.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/fwd.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/type_name.hpp>

#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif

namespace mppp
{

namespace detail
{

// Clamp the precision between the min and max allowed values. This is used in the generic constructor/assignment
// operator.
constexpr ::mpfr_prec_t clamp_mpfr_prec(::mpfr_prec_t p)
{
    return real_prec_check(p) ? p : (p < real_prec_min() ? real_prec_min() : real_prec_max());
}

// Helper function to print an mpfr to stream in a given base.
MPPP_DLL_PUBLIC void mpfr_to_stream(const ::mpfr_t, std::ostream &, int);

// Helpers to deduce the precision when constructing/assigning a real via another type.
// NOTE: it is important that these helpers return a valid (i.e., clamped) precision
// value, because they are used in contexts in which the output precision
// is not checked for validity.
template <typename T, enable_if_t<is_integral<T>::value, int> = 0>
inline ::mpfr_prec_t real_deduce_precision(const T &)
{
    static_assert(nl_digits<T>() < nl_max<::mpfr_prec_t>(), "Overflow error.");
    // NOTE: for signed integers, include the sign bit as well.
    return clamp_mpfr_prec(static_cast<::mpfr_prec_t>(nl_digits<T>()) + is_signed<T>::value);
}

// Utility function to determine the number of base-2 digits of the significand
// of a floating-point type which is not in base-2.
template <typename T>
inline ::mpfr_prec_t dig2mpfr_prec()
{
    static_assert(std::is_floating_point<T>::value, "Invalid type.");
    // NOTE: just do a raw cast for the time being, it's not like we have ways of testing
    // this in any case. In the future we could consider switching to a compile-time implementation
    // of the integral log2, and do everything as compile-time integral computations.
    return static_cast<::mpfr_prec_t>(std::ceil(nl_digits<T>() * std::log2(std::numeric_limits<T>::radix)));
}

template <typename T, enable_if_t<std::is_floating_point<T>::value, int> = 0>
inline ::mpfr_prec_t real_deduce_precision(const T &)
{
    static_assert(nl_digits<T>() <= nl_max<::mpfr_prec_t>(), "Overflow error.");
    return clamp_mpfr_prec(std::numeric_limits<T>::radix == 2 ? static_cast<::mpfr_prec_t>(nl_digits<T>())
                                                              : dig2mpfr_prec<T>());
}

template <std::size_t SSize>
inline ::mpfr_prec_t real_deduce_precision(const integer<SSize> &n)
{
    // Infer the precision from the bit size of n.
    const auto ls = n.size();
    // Check that ls * GMP_NUMB_BITS is representable by mpfr_prec_t.
    // LCOV_EXCL_START
    if (mppp_unlikely(ls > make_unsigned(nl_max<::mpfr_prec_t>()) / unsigned(GMP_NUMB_BITS))) {
        throw std::overflow_error("The deduced precision for a real from an integer is too large");
    }
    // LCOV_EXCL_STOP
    return clamp_mpfr_prec(static_cast<::mpfr_prec_t>(static_cast<::mpfr_prec_t>(ls) * GMP_NUMB_BITS));
}

template <std::size_t SSize>
inline ::mpfr_prec_t real_deduce_precision(const rational<SSize> &q)
{
    // Infer the precision from the bit size of num/den.
    const auto n_size = q.get_num().size();
    const auto d_size = q.get_den().size();
    // Overflow checks.
    // LCOV_EXCL_START
    if (mppp_unlikely(
            // Overflow in total size.
            (n_size > nl_max<decltype(q.get_num().size())>() - d_size)
            // Check that tot_size * GMP_NUMB_BITS is representable by mpfr_prec_t.
            || ((n_size + d_size) > make_unsigned(nl_max<::mpfr_prec_t>()) / unsigned(GMP_NUMB_BITS)))) {
        throw std::overflow_error("The deduced precision for a real from a rational is too large");
    }
    // LCOV_EXCL_STOP
    return clamp_mpfr_prec(static_cast<::mpfr_prec_t>(static_cast<::mpfr_prec_t>(n_size + d_size) * GMP_NUMB_BITS));
}

#if defined(MPPP_WITH_QUADMATH)

inline ::mpfr_prec_t real_deduce_precision(const real128 &)
{
    // The significand precision in bits is 113 for real128. Let's double-check it.
    static_assert(real128_sig_digits() == 113u, "Invalid number of digits.");
    return clamp_mpfr_prec(113);
}

#endif

// Fwd declare for friendship.
template <bool, typename F, typename Arg0, typename... Args>
real &mpfr_nary_op_impl(::mpfr_prec_t, const F &, real &, Arg0 &&, Args &&...);

template <bool, typename F, typename Arg0, typename... Args>
real mpfr_nary_op_return_impl(::mpfr_prec_t, const F &, Arg0 &&, Args &&...);

template <typename F>
real real_constant(const F &, ::mpfr_prec_t);

// Wrapper for calling mpfr_lgamma().
MPPP_DLL_PUBLIC void real_lgamma_wrapper(::mpfr_t, const ::mpfr_t, ::mpfr_rnd_t);

// Wrapper for calling mpfr_li2().
MPPP_DLL_PUBLIC void real_li2_wrapper(::mpfr_t, const ::mpfr_t, ::mpfr_rnd_t);

// Wrappers for calling integer and remainder-related functions
// with NaN checking.
MPPP_DLL_PUBLIC void real_ceil_wrapper(::mpfr_t, const ::mpfr_t);
MPPP_DLL_PUBLIC void real_floor_wrapper(::mpfr_t, const ::mpfr_t);
MPPP_DLL_PUBLIC void real_round_wrapper(::mpfr_t, const ::mpfr_t);
#if defined(MPPP_MPFR_HAVE_MPFR_ROUNDEVEN)
MPPP_DLL_PUBLIC void real_roundeven_wrapper(::mpfr_t, const ::mpfr_t);
#endif
MPPP_DLL_PUBLIC void real_trunc_wrapper(::mpfr_t, const ::mpfr_t);
MPPP_DLL_PUBLIC void real_frac_wrapper(::mpfr_t, const ::mpfr_t);

#if defined(MPPP_WITH_ARB)

// The Arb MPFR wrappers.
MPPP_DLL_PUBLIC void arb_sqrt1pm1(::mpfr_t, const ::mpfr_t);

MPPP_DLL_PUBLIC void arb_log_hypot(::mpfr_t, const ::mpfr_t, const ::mpfr_t);
MPPP_DLL_PUBLIC void arb_log_base_ui(::mpfr_t, const ::mpfr_t, unsigned long);

MPPP_DLL_PUBLIC void arb_sin_pi(::mpfr_t, const ::mpfr_t);
MPPP_DLL_PUBLIC void arb_cos_pi(::mpfr_t, const ::mpfr_t);
MPPP_DLL_PUBLIC void arb_tan_pi(::mpfr_t, const ::mpfr_t);
MPPP_DLL_PUBLIC void arb_cot_pi(::mpfr_t, const ::mpfr_t);
MPPP_DLL_PUBLIC void arb_sinc(::mpfr_t, const ::mpfr_t);
MPPP_DLL_PUBLIC void arb_sinc_pi(::mpfr_t, const ::mpfr_t);

MPPP_DLL_PUBLIC void arb_hypgeom_bessel_j(::mpfr_t, const ::mpfr_t, const ::mpfr_t);
MPPP_DLL_PUBLIC void arb_hypgeom_bessel_y(::mpfr_t, const ::mpfr_t, const ::mpfr_t);

MPPP_DLL_PUBLIC void arb_lambert_w0(::mpfr_t, const ::mpfr_t);
MPPP_DLL_PUBLIC void arb_lambert_wm1(::mpfr_t, const ::mpfr_t);

MPPP_DLL_PUBLIC void arb_polylog_si(::mpfr_t, long, const ::mpfr_t);
MPPP_DLL_PUBLIC void arb_polylog(::mpfr_t, const ::mpfr_t, const ::mpfr_t);

#endif

} // namespace detail

// Fwd declare swap.
void swap(real &, real &) noexcept;

template <typename T>
using is_real_interoperable = detail::disjunction<is_cpp_arithmetic<T>, detail::is_integer<T>, detail::is_rational<T>
#if defined(MPPP_WITH_QUADMATH)
                                                  ,
                                                  std::is_same<T, real128>
#endif
                                                  >;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL real_interoperable = is_real_interoperable<T>::value;

#endif

template <typename T>
using is_cvr_real = std::is_same<detail::uncvref_t<T>, real>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL cvr_real = is_cvr_real<T>::value;

#endif

template <typename... Args>
using cvr_real_enabler = detail::enable_if_t<detail::conjunction<is_cvr_real<Args>...>::value, int>;

// Special initialisation tags for real.
enum class real_kind : std::underlying_type<::mpfr_kind_t>::type {
    nan = MPFR_NAN_KIND,
    inf = MPFR_INF_KIND,
    zero = MPFR_ZERO_KIND
};

// For the future:
// - construction from/conversion to interoperables can probably be improved performance wise, especially
//   if we exploit the mpfr_t internals.
// - probably we should have a build in the CI against the latest MPFR, built with sanitizers on.
// - probably we should have MPFR as well in the 32bit coverage build.
// - it seems like we might be doing multiple roundings when cting from real128.
//   See if we can implement with only a single rounding, perhaps via an integer
//   and mpfr_set_z_2exp()?
// - Do we need real_equal_to() to work also on invalid reals, the way
//   real_lt/gt() do?
// - Not sure what the caching situation is currently. The MPFR 4 changelog mentions it:
//   https://www.mpfr.org/mpfr-4.0.0/#changes
//   But experiments with valgrind and the real_alloc benchmark seem to indicate
//   that no caching is done. Need to revisit this.

// Multiprecision floating-point class.
class MPPP_DLL_PUBLIC real
{
#if defined(MPPP_WITH_BOOST_S11N)
    friend class boost::serialization::access;

    template <typename Archive>
    void save(Archive &ar, unsigned) const
    {
        ar << get_prec();
        ar << to_string();
    }

    template <typename Archive>
    void load(Archive &ar, unsigned)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
        ::mpfr_prec_t p;
        ar >> p;
        std::string tmp;
        ar >> tmp;

        *this = real{tmp, p};
    }

    // Overloads for binary archives.
    void save(boost::archive::binary_oarchive &, unsigned) const;
    void load(boost::archive::binary_iarchive &, unsigned);

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif

    // Make friends, for accessing the non-checking prec setting funcs.
    template <bool, typename F, typename Arg0, typename... Args>
    // NOLINTNEXTLINE(readability-redundant-declaration)
    friend real &detail::mpfr_nary_op_impl(::mpfr_prec_t, const F &, real &, Arg0 &&, Args &&...);
    template <bool, typename F, typename Arg0, typename... Args>
    // NOLINTNEXTLINE(readability-redundant-declaration)
    friend real detail::mpfr_nary_op_return_impl(::mpfr_prec_t, const F &, Arg0 &&, Args &&...);
    template <typename F>
    // NOLINTNEXTLINE(readability-redundant-declaration)
    friend real detail::real_constant(const F &, ::mpfr_prec_t);
    // Utility function to check the precision upon init.
    static ::mpfr_prec_t check_init_prec(::mpfr_prec_t p)
    {
        if (mppp_unlikely(!detail::real_prec_check(p))) {
            throw std::invalid_argument("Cannot init a real with a precision of " + detail::to_string(p)
                                        + ": the maximum allowed precision is " + detail::to_string(real_prec_max())
                                        + ", the minimum allowed precision is " + detail::to_string(real_prec_min()));
        }
        return p;
    }

#if defined(MPPP_WITH_MPC)
    // NOTE: the complex class needs access to some
    // private bits of real.
    friend class complex;

    // Shallow copy constructor from mpfr_t, used
    // only by the complex class.
    struct shallow_copy_t {
    };
    explicit real(shallow_copy_t, const ::mpfr_t r) : m_mpfr(r[0]) {}
#endif

public:
    // Default constructor.
    real();

private:
    // A tag to call private ctors.
    struct ptag {
    };
    // Private ctor that sets to NaN with a certain precision,
    // without checking the input precision value.
    explicit real(const ptag &, ::mpfr_prec_t, bool);

public:
    // Copy constructor.
    real(const real &);
    // Move constructor.
    real(real &&other) noexcept
        : // Shallow copy other.
          m_mpfr(other.m_mpfr)
    {
        // Mark the other as moved-from.
        other.m_mpfr._mpfr_d = nullptr;
    }

    // Copy constructor with custom precision.
    explicit real(const real &, ::mpfr_prec_t);
    // Move constructor with custom precision.
    explicit real(real &&, ::mpfr_prec_t);

    // Constructor from a special value, sign and precision.
    explicit real(real_kind, int, ::mpfr_prec_t);
    // Constructor from a special value and precision.
    explicit real(real_kind, ::mpfr_prec_t);

    // Constructors from n*2**e.
    template <std::size_t SSize>
    explicit real(const integer<SSize> &, ::mpfr_exp_t, ::mpfr_prec_t);
    explicit real(unsigned long, ::mpfr_exp_t, ::mpfr_prec_t);
    explicit real(long, ::mpfr_exp_t, ::mpfr_prec_t);

private:
    // Construction from FPs.
    template <typename Func, typename T>
    MPPP_DLL_LOCAL void dispatch_fp_construction(const Func &, const T &);
    void dispatch_construction(const float &);
    void dispatch_construction(const double &);
    void dispatch_construction(const long double &);

    // Construction from integral types.
    // Special casing for bool, otherwise MSVC warns if we fold this into the
    // constructor from unsigned.
    void dispatch_construction(const bool &);
    template <typename T,
              detail::enable_if_t<detail::conjunction<detail::is_integral<T>, detail::is_unsigned<T>>::value, int> = 0>
    void dispatch_construction(const T &n)
    {
        if (n <= detail::nl_max<unsigned long>()) {
            mpfr_set_ui(&m_mpfr, static_cast<unsigned long>(n), MPFR_RNDN);
        } else {
            // NOTE: here and elsewhere let's use a 2-limb integer, in the hope
            // of avoiding dynamic memory allocation.
            ::mpfr_set_z(&m_mpfr, integer<2>(n).get_mpz_view(), MPFR_RNDN);
        }
    }
    template <typename T,
              detail::enable_if_t<detail::conjunction<detail::is_integral<T>, detail::is_signed<T>>::value, int> = 0>
    void dispatch_construction(const T &n)
    {
        if (n <= detail::nl_max<long>() && n >= detail::nl_min<long>()) {
            mpfr_set_si(&m_mpfr, static_cast<long>(n), MPFR_RNDN);
        } else {
            ::mpfr_set_z(&m_mpfr, integer<2>(n).get_mpz_view(), MPFR_RNDN);
        }
    }

    // Construction from mppp::integer.
    void dispatch_mpz_construction(const ::mpz_t);
    template <std::size_t SSize>
    void dispatch_construction(const integer<SSize> &n)
    {
        dispatch_mpz_construction(n.get_mpz_view());
    }

    // Construction from mppp::rational.
    void dispatch_mpq_construction(const ::mpq_t);
    template <std::size_t SSize>
    void dispatch_construction(const rational<SSize> &q)
    {
        // NOTE: get_mpq_view() returns an mpq_struct, whose
        // address we then need to use.
        const auto v = detail::get_mpq_view(q);
        dispatch_mpq_construction(&v);
    }

#if defined(MPPP_WITH_QUADMATH)
    void dispatch_construction(const real128 &);
    // NOTE: split this off from the dispatch_construction() overload, so we can re-use it in the
    // generic assignment.
    void assign_real128(const real128 &);
#endif

public:
    // Generic constructors.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_real_interoperable<T>::value, int> = 0>
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    real(const T &x) : real(ptag{}, detail::real_deduce_precision(x), true)
    {
        dispatch_construction(x);
    }
#if defined(MPPP_HAVE_CONCEPTS)
    template <real_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_real_interoperable<T>::value, int> = 0>
#endif
    explicit real(const T &x, ::mpfr_prec_t p) : real(ptag{}, check_init_prec(p), true)
    {
        dispatch_construction(x);
    }

    // Constructors from std::complex.
#if defined(MPPP_HAVE_CONCEPTS)
    template <cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_cpp_complex<T>::value, int> = 0>
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit real(const T &c)
        : real(c.imag() == 0 ? c.real()
                             : throw std::domain_error(
                                 "Cannot construct a real from a complex C++ value with a non-zero imaginary part of "
                                 + detail::to_string(c.imag())))
    {
    }
#if defined(MPPP_HAVE_CONCEPTS)
    template <cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_cpp_complex<T>::value, int> = 0>
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit real(const T &c, ::mpfr_prec_t p)
        : real(c.imag() == 0 ? c.real()
                             : throw std::domain_error(
                                 "Cannot construct a real from a complex C++ value with a non-zero imaginary part of "
                                 + detail::to_string(c.imag())),
               p)
    {
    }

private:
    MPPP_DLL_LOCAL void construct_from_c_string(const char *, int, ::mpfr_prec_t);
    explicit real(const ptag &, const char *, int, ::mpfr_prec_t);
    explicit real(const ptag &, const std::string &, int, ::mpfr_prec_t);
#if defined(MPPP_HAVE_STRING_VIEW)
    explicit real(const ptag &, const std::string_view &, int, ::mpfr_prec_t);
#endif

public:
    // Constructor from string, base and precision.
#if defined(MPPP_HAVE_CONCEPTS)
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit real(const T &s, int base, ::mpfr_prec_t p) : real(ptag{}, s, base, p)
    {
    }
    // Constructor from string and precision.
#if defined(MPPP_HAVE_CONCEPTS)
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit real(const T &s, ::mpfr_prec_t p) : real(s, 10, p)
    {
    }
    // Constructor from range of characters, base and precision.
    explicit real(const char *, const char *, int, ::mpfr_prec_t);
    // Constructor from range of characters and precision.
    explicit real(const char *, const char *, ::mpfr_prec_t);

    // Copy constructor from mpfr_t.
    explicit real(const ::mpfr_t);
#if !defined(_MSC_VER) || defined(__clang__)
    // Move constructor from mpfr_t.
    explicit real(::mpfr_t &&x) : m_mpfr(*x) {}
#endif

    // Destructor.
    ~real();

    // Copy assignment operator.
    real &operator=(const real &);

    // Move assignment operator.
    real &operator=(real &&other) noexcept
    {
        // NOTE: for generic code, std::swap() is not a particularly good way of implementing
        // the move assignment:
        //
        // https://stackoverflow.com/questions/6687388/why-do-some-people-use-swap-for-move-assignments
        //
        // Here however it is fine, as we know there are no side effects we need to maintain.
        //
        // NOTE: we use a raw std::swap() here (instead of mpfr_swap()) because we don't know in principle
        // if mpfr_swap() relies on the operands not to be in a moved-from state (although it's unlikely).
        std::swap(m_mpfr, other.m_mpfr);
        return *this;
    }

private:
    // Assignment from FPs.
    template <bool SetPrec, typename Func, typename T>
    void dispatch_fp_assignment(const Func &func, const T &x)
    {
        if (SetPrec) {
            set_prec_impl<false>(detail::real_deduce_precision(x));
        }
        func(&m_mpfr, x, MPFR_RNDN);
    }
    template <bool SetPrec>
    void dispatch_assignment(const float &x)
    {
        dispatch_fp_assignment<SetPrec>(::mpfr_set_flt, x);
    }
    template <bool SetPrec>
    void dispatch_assignment(const double &x)
    {
        dispatch_fp_assignment<SetPrec>(::mpfr_set_d, x);
    }
    template <bool SetPrec>
    void dispatch_assignment(const long double &x)
    {
        dispatch_fp_assignment<SetPrec>(::mpfr_set_ld, x);
    }

    // Assignment from integral types.
    template <bool SetPrec, typename T>
    void dispatch_integral_ass_prec(const T &n)
    {
        if (SetPrec) {
            set_prec_impl<false>(detail::real_deduce_precision(n));
        }
    }
    // Special casing for bool.
    template <bool SetPrec>
    void dispatch_assignment(const bool &b)
    {
        dispatch_integral_ass_prec<SetPrec>(b);
        mpfr_set_ui(&m_mpfr, static_cast<unsigned long>(b), MPFR_RNDN);
    }
    template <bool SetPrec, typename T,
              detail::enable_if_t<detail::conjunction<detail::is_integral<T>, detail::is_unsigned<T>>::value, int> = 0>
    void dispatch_assignment(const T &n)
    {
        dispatch_integral_ass_prec<SetPrec>(n);
        if (n <= detail::nl_max<unsigned long>()) {
            mpfr_set_ui(&m_mpfr, static_cast<unsigned long>(n), MPFR_RNDN);
        } else {
            ::mpfr_set_z(&m_mpfr, integer<2>(n).get_mpz_view(), MPFR_RNDN);
        }
    }
    template <bool SetPrec, typename T,
              detail::enable_if_t<detail::conjunction<detail::is_integral<T>, detail::is_signed<T>>::value, int> = 0>
    void dispatch_assignment(const T &n)
    {
        dispatch_integral_ass_prec<SetPrec>(n);
        if (n <= detail::nl_max<long>() && n >= detail::nl_min<long>()) {
            mpfr_set_si(&m_mpfr, static_cast<long>(n), MPFR_RNDN);
        } else {
            ::mpfr_set_z(&m_mpfr, integer<2>(n).get_mpz_view(), MPFR_RNDN);
        }
    }

    // Assignment from integer.
    template <bool SetPrec, std::size_t SSize>
    void dispatch_assignment(const integer<SSize> &n)
    {
        if (SetPrec) {
            set_prec_impl<false>(detail::real_deduce_precision(n));
        }
        ::mpfr_set_z(&m_mpfr, n.get_mpz_view(), MPFR_RNDN);
    }

    // Assignment from rational.
    template <bool SetPrec, std::size_t SSize>
    void dispatch_assignment(const rational<SSize> &q)
    {
        if (SetPrec) {
            set_prec_impl<false>(detail::real_deduce_precision(q));
        }
        const auto v = detail::get_mpq_view(q);
        ::mpfr_set_q(&m_mpfr, &v, MPFR_RNDN);
    }

#if defined(MPPP_WITH_QUADMATH)
    // Assignment from real128.
    template <bool SetPrec>
    void dispatch_assignment(const real128 &x)
    {
        if (SetPrec) {
            set_prec_impl<false>(detail::real_deduce_precision(x));
        }
        assign_real128(x);
    }
#endif

public:
    // Generic assignment operator.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_real_interoperable<T>::value, int> = 0>
#endif
    real &operator=(const T &x)
    {
        dispatch_assignment<true>(x);
        return *this;
    }
    // Assignment from std::complex.
#if defined(MPPP_HAVE_CONCEPTS)
    template <cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_cpp_complex<T>::value, int> = 0>
#endif
    real &operator=(const T &c)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
        return *this = static_cast<real>(c);
    }
#if defined(MPPP_WITH_QUADMATH)
    real &operator=(const complex128 &);
#endif
#if defined(MPPP_WITH_MPC)
    real &operator=(const complex &);
#endif

    // Copy assignment from mpfr_t.
    real &operator=(const ::mpfr_t);

#if !defined(_MSC_VER) || defined(__clang__)
    // Move assignment from mpfr_t.
    real &operator=(::mpfr_t &&);
#endif

    // Check validity.
    MPPP_NODISCARD bool is_valid() const noexcept
    {
        return m_mpfr._mpfr_d != nullptr;
    }

    // Set to another real.
    real &set(const real &);

    // Generic setter.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_real_interoperable<T>::value, int> = 0>
#endif
    real &set(const T &x)
    {
        dispatch_assignment<false>(x);
        return *this;
    }
    // Set to std::complex.
#if defined(MPPP_HAVE_CONCEPTS)
    template <cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_cpp_complex<T>::value, int> = 0>
#endif
    real &set(const T &c)
    {
        if (mppp_unlikely(c.imag() != 0)) {
            throw std::domain_error("Cannot set a real to a complex C++ value with a non-zero imaginary part of "
                                    + detail::to_string(c.imag()));
        }
        return set(c.real());
    }

private:
    // Implementation of string setters.
    MPPP_DLL_LOCAL void string_assignment_impl(const char *, int);
    real &set_impl(const char *, int);
    real &set_impl(const std::string &, int);
#if defined(MPPP_HAVE_STRING_VIEW)
    real &set_impl(const std::string_view &, int);
#endif

public:
    // Setter to string.
#if defined(MPPP_HAVE_CONCEPTS)
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    real &set(const T &s, int base = 10)
    {
        return set_impl(s, base);
    }
    // Set to character range.
    real &set(const char *begin, const char *end, int base = 10);

    // Set to an mpfr_t.
    real &set(const ::mpfr_t);

    // Set to NaN.
    real &set_nan();
    // Set to infinity.
    real &set_inf(int sign = 0);
    // Set to zero.
    real &set_zero(int sign = 0);

    // Const reference to the internal mpfr_t.
    MPPP_NODISCARD const mpfr_struct_t *get_mpfr_t() const
    {
        return &m_mpfr;
    }
    // Mutable reference to the internal mpfr_t.
    mpfr_struct_t *_get_mpfr_t()
    {
        return &m_mpfr;
    }

    // Detect NaN.
    MPPP_NODISCARD bool nan_p() const
    {
        return mpfr_nan_p(&m_mpfr) != 0;
    }
    // Detect infinity.
    MPPP_NODISCARD bool inf_p() const
    {
        return mpfr_inf_p(&m_mpfr) != 0;
    }
    // Detect finite number.
    MPPP_NODISCARD bool number_p() const
    {
        return mpfr_number_p(&m_mpfr) != 0;
    }
    // Detect zero.
    MPPP_NODISCARD bool zero_p() const
    {
        return mpfr_zero_p(&m_mpfr) != 0;
    }
    // Detect regular number.
    MPPP_NODISCARD bool regular_p() const
    {
        return mpfr_regular_p(&m_mpfr) != 0;
    }
    // Detect integer.
    MPPP_NODISCARD bool integer_p() const;
    // Detect one.
    MPPP_NODISCARD bool is_one() const;

    // Detect sign.
    MPPP_NODISCARD int sgn() const
    {
        if (mppp_unlikely(nan_p())) {
            // NOTE: mpfr_sgn() in this case would set an error flag, and we generally
            // handle error flags as exceptions.
            throw std::domain_error("Cannot determine the sign of a real NaN");
        }
        return mpfr_sgn(&m_mpfr);
    }
    // Get the sign bit.
    MPPP_NODISCARD bool signbit() const
    {
        return mpfr_signbit(&m_mpfr) != 0;
    }

    // Get the precision of this.
    MPPP_NODISCARD ::mpfr_prec_t get_prec() const
    {
        return mpfr_get_prec(&m_mpfr);
    }

private:
    // Utility function to check precision in set_prec().
    static ::mpfr_prec_t check_set_prec(::mpfr_prec_t p)
    {
        if (mppp_unlikely(!detail::real_prec_check(p))) {
            throw std::invalid_argument("Cannot set the precision of a real to the value " + detail::to_string(p)
                                        + ": the maximum allowed precision is " + detail::to_string(real_prec_max())
                                        + ", the minimum allowed precision is " + detail::to_string(real_prec_min()));
        }
        return p;
    }
    // mpfr_set_prec() wrapper, with or without prec checking.
    template <bool Check>
    void set_prec_impl(::mpfr_prec_t p)
    {
        ::mpfr_set_prec(&m_mpfr, Check ? check_set_prec(p) : p);
    }
    // mpfr_prec_round() wrapper, with or without prec checking.
    template <bool Check>
    void prec_round_impl(::mpfr_prec_t p)
    {
        ::mpfr_prec_round(&m_mpfr, Check ? check_set_prec(p) : p, MPFR_RNDN);
    }

public:
    // Destructively set the precision.
    real &set_prec(::mpfr_prec_t);
    // Set the precision maintaining the current value.
    real &prec_round(::mpfr_prec_t);

private:
    // Generic conversion.
    // integer.
    template <typename T, detail::enable_if_t<detail::is_integer<T>::value, int> = 0>
    MPPP_NODISCARD T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to an integer");
        }
        MPPP_MAYBE_TLS detail::mpz_raii mpz;
        // Truncate the value when converting to integer.
        ::mpfr_get_z(&mpz.m_mpz, &m_mpfr, MPFR_RNDZ);
        return T{&mpz.m_mpz};
    }
    // rational.
    template <std::size_t SSize>
    bool rational_conversion(rational<SSize> &rop) const
    {
#if defined(MPPP_MPFR_HAVE_MPFR_GET_Q)
        MPPP_MAYBE_TLS detail::mpq_raii mpq;
        // NOTE: we already checked outside
        // that rop is a finite number, hence
        // this function cannot fail.
        ::mpfr_get_q(&mpq.m_mpq, &m_mpfr);
        rop = &mpq.m_mpq;
        return true;
#else
        // Clear the range error flag before attempting the conversion.
        ::mpfr_clear_erangeflag();
        // NOTE: this call can fail if the exponent of this is very close to the upper/lower limits of the exponent
        // type. If the call fails (signalled by a range flag being set), we will return error.
        MPPP_MAYBE_TLS detail::mpz_raii mpz;
        const ::mpfr_exp_t exp2 = ::mpfr_get_z_2exp(&mpz.m_mpz, &m_mpfr);
        // NOTE: not sure at the moment how to trigger this, let's leave it for now.
        // LCOV_EXCL_START
        if (mppp_unlikely(::mpfr_erangeflag_p())) {
            // Let's first reset the error flag.
            ::mpfr_clear_erangeflag();
            return false;
        }
        // LCOV_EXCL_STOP
        // The conversion to n * 2**exp succeeded. We will build a rational
        // from n and exp.
        rop._get_num() = &mpz.m_mpz;
        rop._get_den().set_one();
        if (exp2 >= ::mpfr_exp_t(0)) {
            // The output value will be an integer.
            rop._get_num() <<= detail::make_unsigned(exp2);
        } else {
            // The output value will be a rational. Canonicalisation will be needed.
            rop._get_den() <<= detail::nint_abs(exp2);
            canonicalise(rop);
        }
        return true;
#endif
    }
    template <typename T, detail::enable_if_t<detail::is_rational<T>::value, int> = 0>
    MPPP_NODISCARD T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to a rational");
        }
        T rop;
        // LCOV_EXCL_START
        if (mppp_unlikely(!rational_conversion(rop))) {
            throw std::overflow_error("The exponent of a real is too large for conversion to rational");
        }
        // LCOV_EXCL_STOP
        return rop;
    }
    // C++ floating-point.
    template <typename T, detail::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    MPPP_NODISCARD T dispatch_conversion() const
    {
        if (std::is_same<T, float>::value) {
            return static_cast<T>(::mpfr_get_flt(&m_mpfr, MPFR_RNDN));
        }
        if (std::is_same<T, double>::value) {
            return static_cast<T>(::mpfr_get_d(&m_mpfr, MPFR_RNDN));
        }
        assert((std::is_same<T, long double>::value));
        return static_cast<T>(::mpfr_get_ld(&m_mpfr, MPFR_RNDN));
    }
    // Small helper to raise an exception when converting to C++ integrals.
    template <typename T>
    [[noreturn]] void raise_overflow_error() const
    {
        throw std::overflow_error("Conversion of the real " + to_string() + " to the type '" + type_name<T>()
                                  + "' results in overflow");
    }
    // Unsigned integrals, excluding bool.
    template <typename T>
    bool uint_conversion(T &rop) const
    {
        // Clear the range error flag before attempting the conversion.
        ::mpfr_clear_erangeflag();
        // NOTE: this will handle correctly the case in which this is negative but greater than -1.
        const unsigned long candidate = ::mpfr_get_ui(&m_mpfr, MPFR_RNDZ);
        if (::mpfr_erangeflag_p()) {
            // If the range error flag is set, it means the conversion failed because this is outside
            // the range of unsigned long. Let's clear the error flag first.
            ::mpfr_clear_erangeflag();
            // If the value is positive, and the target type has a range greater than unsigned long,
            // we will attempt the conversion again via integer.
            if (detail::nl_max<T>() > detail::nl_max<unsigned long>() && sgn() > 0) {
                return mppp::get(rop, static_cast<integer<2>>(*this));
            }
            return false;
        }
        if (candidate <= detail::nl_max<T>()) {
            rop = static_cast<T>(candidate);
            return true;
        }
        return false;
    }
    template <typename T,
              detail::enable_if_t<detail::conjunction<detail::negation<std::is_same<bool, T>>, detail::is_integral<T>,
                                                      detail::is_unsigned<T>>::value,
                                  int> = 0>
    MPPP_NODISCARD T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to a C++ unsigned integral type");
        }
        T rop;
        if (mppp_unlikely(!uint_conversion(rop))) {
            raise_overflow_error<T>();
        }
        return rop;
    }
    // bool.
    template <typename T, detail::enable_if_t<std::is_same<bool, T>::value, int> = 0>
    MPPP_NODISCARD T dispatch_conversion() const
    {
        // NOTE: in C/C++ the conversion of NaN to bool gives true:
        // https://stackoverflow.com/questions/9158567/nan-to-bool-conversion-true-or-false
        return !zero_p();
    }
    // Signed integrals.
    template <typename T>
    bool sint_conversion(T &rop) const
    {
        ::mpfr_clear_erangeflag();
        const long candidate = ::mpfr_get_si(&m_mpfr, MPFR_RNDZ);
        if (::mpfr_erangeflag_p()) {
            // If the range error flag is set, it means the conversion failed because this is outside
            // the range of long. Let's clear the error flag first.
            ::mpfr_clear_erangeflag();
            // If the target type has a range greater than long,
            // we will attempt the conversion again via integer.
            if (detail::nl_min<T>() < detail::nl_min<long>() && detail::nl_max<T>() > detail::nl_max<long>()) {
                return mppp::get(rop, static_cast<integer<2>>(*this));
            }
            return false;
        }
        if (candidate >= detail::nl_min<T>() && candidate <= detail::nl_max<T>()) {
            rop = static_cast<T>(candidate);
            return true;
        }
        return false;
    }
    template <typename T,
              detail::enable_if_t<detail::conjunction<detail::is_integral<T>, detail::is_signed<T>>::value, int> = 0>
    MPPP_NODISCARD T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to a C++ signed integral type");
        }
        T rop;
        if (mppp_unlikely(!sint_conversion(rop))) {
            raise_overflow_error<T>();
        }
        return rop;
    }
#if defined(MPPP_WITH_QUADMATH)
    template <typename T, detail::enable_if_t<std::is_same<real128, T>::value, int> = 0>
    MPPP_NODISCARD T dispatch_conversion() const
    {
        return convert_to_real128();
    }
    MPPP_NODISCARD real128 convert_to_real128() const;
#endif

public:
    // Generic conversion operator.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_real_interoperable<T>::value, int> = 0>
#endif
    explicit operator T() const
    {
        return dispatch_conversion<T>();
    }
    // Conversion to std::complex.
#if defined(MPPP_HAVE_CONCEPTS)
    template <cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_cpp_complex<T>::value, int> = 0>
#endif
    explicit operator T() const
    {
        using value_type = typename T::value_type;

        return T{static_cast<value_type>(*this), value_type(0)};
    }

private:
    template <std::size_t SSize>
    bool dispatch_get(integer<SSize> &rop) const
    {
        if (!number_p()) {
            return false;
        }
        MPPP_MAYBE_TLS detail::mpz_raii mpz;
        // Truncate the value when converting to integer.
        ::mpfr_get_z(&mpz.m_mpz, &m_mpfr, MPFR_RNDZ);
        rop = &mpz.m_mpz;
        return true;
    }
    template <std::size_t SSize>
    bool dispatch_get(rational<SSize> &rop) const
    {
        if (!number_p()) {
            return false;
        }
        return rational_conversion(rop);
    }
    bool dispatch_get(bool &b) const
    {
        b = !zero_p();
        return true;
    }
    template <typename T,
              detail::enable_if_t<detail::conjunction<detail::is_integral<T>, detail::is_unsigned<T>>::value, int> = 0>
    bool dispatch_get(T &n) const
    {
        if (!number_p()) {
            return false;
        }
        return uint_conversion(n);
    }
    template <typename T,
              detail::enable_if_t<detail::conjunction<detail::is_integral<T>, detail::is_signed<T>>::value, int> = 0>
    bool dispatch_get(T &n) const
    {
        if (!number_p()) {
            return false;
        }
        return sint_conversion(n);
    }
    template <typename T, detail::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    bool dispatch_get(T &x) const
    {
        x = static_cast<T>(*this);
        return true;
    }
#if defined(MPPP_WITH_QUADMATH)
    bool dispatch_get(real128 &) const;
#endif

public:
    // Generic conversion function.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_real_interoperable<T>::value, int> = 0>
#endif
    bool get(T &rop) const
    {
        return dispatch_get(rop);
    }
    // Conversion function to std::complex.
#if defined(MPPP_HAVE_CONCEPTS)
    template <cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_cpp_complex<T>::value, int> = 0>
#endif
    bool get(T &rop) const
    {
        rop = static_cast<T>(*this);
        return true;
    }

    // Convert to string.
    MPPP_NODISCARD std::string to_string(int base = 10) const;

private:
    template <typename T>
    MPPP_DLL_LOCAL real &self_mpfr_unary(T &&);
    // Wrapper to apply the input unary MPFR function to this.
    // f must not need a rounding mode. Returns a reference to this.
    template <typename T>
    MPPP_DLL_LOCAL real &self_mpfr_unary_nornd(T &&f)
    {
        std::forward<T>(f)(&m_mpfr, &m_mpfr);
        return *this;
    }

public:
    // Negate in-place.
    real &neg();
    // In-place absolute value.
    real &abs();

    // In-place square root.
    real &sqrt();
    // In-place reciprocal square root.
    real &rec_sqrt();
#if defined(MPPP_WITH_ARB)
    // In-place sqrt1pm1.
    real &sqrt1pm1();
#endif
    // In-place cubic root.
    real &cbrt();

    // In-place squaring.
    real &sqr();

    // In-place sine.
    real &sin();
    // In-place cosine.
    real &cos();
    // In-place tangent.
    real &tan();
    // In-place secant.
    real &sec();
    // In-place cosecant.
    real &csc();
    // In-place cotangent.
    real &cot();
#if defined(MPPP_WITH_ARB)
    // Trig functions from Arb.
    real &sin_pi();
    real &cos_pi();
    real &tan_pi();
    real &cot_pi();
    real &sinc();
    real &sinc_pi();
#endif

    // In-place arccosine.
    real &acos();
    // In-place arcsine.
    real &asin();
    // In-place arctangent.
    real &atan();

    // In-place hyperbolic cosine.
    real &cosh();
    // In-place hyperbolic sine.
    real &sinh();
    // In-place hyperbolic tangent.
    real &tanh();
    // In-place hyperbolic secant.
    real &sech();
    // In-place hyperbolic cosecant.
    real &csch();
    // In-place hyperbolic cotangent.
    real &coth();

    // In-place inverse hyperbolic cosine.
    real &acosh();
    // In-place inverse hyperbolic sine.
    real &asinh();
    // In-place inverse hyperbolic tangent.
    real &atanh();

    // In-place exponential.
    real &exp();
    // In-place base-2 exponential.
    real &exp2();
    // In-place base-10 exponential.
    real &exp10();
    // In-place exponential minus 1.
    real &expm1();
    // In-place logarithm.
    real &log();
    // In-place base-2 logarithm.
    real &log2();
    // In-place base-10 logarithm.
    real &log10();
    // In-place augmented logarithm.
    real &log1p();

    // In-place Gamma function.
    real &gamma();
    // In-place logarithm of the Gamma function.
    real &lngamma();
    // In-place logarithm of the absolute value of the Gamma function.
    real &lgamma();
    // In-place Digamma function.
    real &digamma();

    // In-place Bessel function of the first kind of order 0.
    real &j0();
    // In-place Bessel function of the first kind of order 1.
    real &j1();
    // In-place Bessel function of the second kind of order 0.
    real &y0();
    // In-place Bessel function of the second kind of order 1.
    real &y1();

    // In-place exponential integral.
    real &eint();
    // In-place dilogarithm.
    real &li2();
    // In-place Riemann Zeta function.
    real &zeta();
    // In-place error function.
    real &erf();
    // In-place complementary error function.
    real &erfc();
    // In-place Airy function.
    real &ai();
#if defined(MPPP_WITH_ARB)
    // In-place Lambert W function.
    real &lambert_w0();
    real &lambert_wm1();
#endif

    // In-place integer and remainder-related functions.
    real &ceil();
    real &floor();
    real &round();
#if defined(MPPP_MPFR_HAVE_MPFR_ROUNDEVEN)
    real &roundeven();
#endif
    real &trunc();
    real &frac();

#if defined(MPPP_MPFR_HAVE_MPFR_GET_STR_NDIGITS)
    // Get the number of significant digits required for a round-tripping representation.
    MPPP_NODISCARD std::size_t get_str_ndigits(int = 10) const;
#endif

    // Size of the serialised binary representation.
    MPPP_NODISCARD std::size_t binary_size() const;

private:
    void binary_save_impl(char *, std::size_t) const;

    MPPP_DLL_LOCAL std::size_t binary_load_impl(const char *);
    std::size_t binary_load_impl(const char *, std::size_t, const char *);

public:
    std::size_t binary_save(char *) const;
    std::size_t binary_save(std::vector<char> &) const;
    template <std::size_t S>
    std::size_t binary_save(std::array<char, S> &dest) const
    {
        const auto bs = binary_size();
        if (bs > S) {
            return 0;
        }
        binary_save_impl(dest.data(), bs);
        return bs;
    }
    std::size_t binary_save(std::ostream &) const;

    std::size_t binary_load(const char *);
    std::size_t binary_load(const std::vector<char> &);
    template <std::size_t S>
    std::size_t binary_load(const std::array<char, S> &src)
    {
        return binary_load_impl(src.data(), detail::safe_cast<std::size_t>(src.size()), "std::array");
    }
    std::size_t binary_load(std::istream &);

private:
    mpfr_struct_t m_mpfr;
};

template <typename T, typename U>
using are_real_op_types
    = detail::disjunction<detail::conjunction<is_cvr_real<T>, is_cvr_real<U>>,
                          detail::conjunction<is_cvr_real<T>, is_real_interoperable<detail::uncvref_t<U>>>,
                          detail::conjunction<is_cvr_real<U>, is_real_interoperable<detail::uncvref_t<T>>>>;

template <typename T, typename U>
using are_real_in_place_op_types
    = detail::conjunction<detail::negation<std::is_const<detail::unref_t<T>>>, are_real_op_types<T, U>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL real_op_types = are_real_op_types<T, U>::value;

template <typename T, typename U>
MPPP_CONCEPT_DECL real_in_place_op_types = are_real_in_place_op_types<T, U>::value;

#endif

// Destructively set the precision.
inline void set_prec(real &r, ::mpfr_prec_t p)
{
    r.set_prec(p);
}

// Set the precision.
inline void prec_round(real &r, ::mpfr_prec_t p)
{
    r.prec_round(p);
}

// Get the precision.
inline mpfr_prec_t get_prec(const real &r)
{
    return r.get_prec();
}

namespace detail
{

template <typename... Args>
using real_set_t = decltype(std::declval<real &>().set(std::declval<const Args &>()...));

} // namespace detail

#if defined(MPPP_HAVE_CONCEPTS)

template <typename... Args>
MPPP_CONCEPT_DECL real_set_args = detail::is_detected<detail::real_set_t, Args...>::value;

#endif

// Generic setter.
#if defined(MPPP_HAVE_CONCEPTS)
template <real_set_args... Args>
#else
template <typename... Args, detail::enable_if_t<detail::is_detected<detail::real_set_t, Args...>::value, int> = 0>
#endif
inline real &set(real &r, const Args &...args)
{
    return r.set(args...);
}

// Set to n*2**e.
template <std::size_t SSize>
inline real &set_z_2exp(real &r, const integer<SSize> &n, ::mpfr_exp_t e)
{
    ::mpfr_set_z_2exp(r._get_mpfr_t(), n.get_mpz_view(), e, MPFR_RNDN);
    return r;
}

MPPP_DLL_PUBLIC real &set_ui_2exp(real &, unsigned long, ::mpfr_exp_t);
MPPP_DLL_PUBLIC real &set_si_2exp(real &, long, ::mpfr_exp_t);

// Implementation of the constructor from n*2**e, integer overload.
// Place it here so that set_z_2exp() is visible.
template <std::size_t SSize>
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
inline real::real(const integer<SSize> &n, ::mpfr_exp_t e, ::mpfr_prec_t p)
{
    ::mpfr_init2(&m_mpfr, check_init_prec(p));
    set_z_2exp(*this, n, e);
}

// Set to NaN.
inline real &set_nan(real &r)
{
    return r.set_nan();
}

// Set to infinity.
inline real &set_inf(real &r, int sign = 0)
{
    return r.set_inf(sign);
}

// Set to zero.
inline real &set_zero(real &r, int sign = 0)
{
    return r.set_zero(sign);
}

// Swap.
inline void swap(real &a, real &b) noexcept
{
    ::mpfr_swap(a._get_mpfr_t(), b._get_mpfr_t());
}

// Generic conversion functions.
#if defined(MPPP_HAVE_CONCEPTS)
template <real_interoperable T>
#else
template <typename T, detail::enable_if_t<is_real_interoperable<T>::value, int> = 0>
#endif
inline bool get(T &rop, const real &x)
{
    return x.get(rop);
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cpp_complex T>
#else
template <typename T, detail::enable_if_t<is_cpp_complex<T>::value, int> = 0>
#endif
inline bool get(T &rop, const real &x)
{
    return x.get(rop);
}

// Extract significand and exponent.
template <std::size_t SSize>
inline mpfr_exp_t get_z_2exp(integer<SSize> &n, const real &r)
{
    if (mppp_unlikely(!r.number_p())) {
        throw std::domain_error("Cannot extract the significand and the exponent of a non-finite real");
    }
    MPPP_MAYBE_TLS detail::mpz_raii m;
    ::mpfr_clear_erangeflag();
    auto retval = ::mpfr_get_z_2exp(&m.m_mpz, r.get_mpfr_t());
    // LCOV_EXCL_START
    if (mppp_unlikely(::mpfr_erangeflag_p())) {
        ::mpfr_clear_erangeflag();
        throw std::overflow_error("Cannot extract the exponent of the real value " + r.to_string()
                                  + ": the exponent's magnitude is too large");
    }
    // LCOV_EXCL_STOP
    n = &m.m_mpz;
    return retval;
}

namespace detail
{

// A small helper to init the pairs in the functions below. We need this because
// we cannot take the address of a const real as a real *.
template <typename Arg, enable_if_t<!is_ncrvr<Arg &&>::value, int> = 0>
inline std::pair<real *, ::mpfr_prec_t> mpfr_nary_op_init_pair(::mpfr_prec_t min_prec, Arg &&arg)
{
    // arg is not a non-const rvalue ref, we cannot steal from it. Init with nullptr.
    return std::make_pair(static_cast<real *>(nullptr), c_max(arg.get_prec(), min_prec));
}

template <typename Arg, enable_if_t<is_ncrvr<Arg &&>::value, int> = 0>
inline std::pair<real *, ::mpfr_prec_t> mpfr_nary_op_init_pair(::mpfr_prec_t min_prec, Arg &&arg)
{
    // arg is a non-const rvalue ref, and a candidate for stealing resources.
    return std::make_pair(&arg, c_max(arg.get_prec(), min_prec));
}

// A recursive function to determine, in an MPFR function call,
// the largest argument we can steal resources from, and the max precision among
// all the arguments.
inline void mpfr_nary_op_check_steal(std::pair<real *, ::mpfr_prec_t> &) {}

// NOTE: we need 2 overloads for this, as we cannot extract a non-const pointer from
// arg0 if arg0 is a const ref.
template <typename Arg0, typename... Args, enable_if_t<!is_ncrvr<Arg0 &&>::value, int> = 0>
void mpfr_nary_op_check_steal(std::pair<real *, ::mpfr_prec_t> &, Arg0 &&, Args &&...);

template <typename Arg0, typename... Args, enable_if_t<is_ncrvr<Arg0 &&>::value, int> = 0>
void mpfr_nary_op_check_steal(std::pair<real *, ::mpfr_prec_t> &, Arg0 &&, Args &&...);

template <typename Arg0, typename... Args, enable_if_t<!is_ncrvr<Arg0 &&>::value, int>>
inline void mpfr_nary_op_check_steal(std::pair<real *, ::mpfr_prec_t> &p, Arg0 &&arg0, Args &&...args)
{
    // arg0 is not a non-const rvalue ref, we won't be able to steal from it regardless. Just
    // update the max prec.
    p.second = c_max(arg0.get_prec(), p.second);
    mpfr_nary_op_check_steal(p, std::forward<Args>(args)...);
}

template <typename Arg0, typename... Args, enable_if_t<is_ncrvr<Arg0 &&>::value, int>>
inline void mpfr_nary_op_check_steal(std::pair<real *, ::mpfr_prec_t> &p, Arg0 &&arg0, Args &&...args)
{
    const auto prec0 = arg0.get_prec();
    if (!p.first || prec0 > p.first->get_prec()) {
        // The current argument arg0 is a non-const rvalue reference, and either it's
        // the first argument we encounter we can steal from, or it has a precision
        // larger than the current candidate for stealing resources from. This means that
        // arg0 is the new candidate.
        p.first = &arg0;
    }
    // Update the max precision among the arguments, if necessary.
    p.second = c_max(prec0, p.second);
    mpfr_nary_op_check_steal(p, std::forward<Args>(args)...);
}

// A small wrapper to call an MPFR function f with arguments args. If the first param is true_type,
// the rounding mode MPFR_RNDN will be appended at the end of the function arguments list.
template <typename F, typename... Args>
inline void mpfr_nary_func_wrapper(const std::true_type &, const F &f, Args &&...args)
{
    f(std::forward<Args>(args)..., MPFR_RNDN);
}

template <typename F, typename... Args>
inline void mpfr_nary_func_wrapper(const std::false_type &, const F &f, Args &&...args)
{
    f(std::forward<Args>(args)...);
}

// The goal of this helper is to invoke the MPFR-like function object f with signature
//
// void f(mpfr_t out, const mpfr_t x0, const mpfr_t x1, ...)
//
// on the mpfr_t instances contained in the input real objects,
//
// f(rop._get_mpfr_t(), arg0.get_mpfr_t(), arg1.get_mpfr_t(), ...)
//
// The helper will ensure that, before the invocation, the precision
// of rop is set to max(min_prec, arg0.get_prec(), arg1.get_prec(), ...).
//
// One of the input arguments may be used as return value in the invocation
// instead of rop if it provides enough precision and it is passed as a non-const
// rvalue reference. In such a case, the selected input argument will be swapped
// into rop after the invocation and before the function returns.
//
// The Rnd flag controls whether to add the rounding mode (MPFR_RNDN) at the end
// of the MPFR-like function object arguments list or not.
//
// This function requires that the MPFR-like function object being called supports
// overlapping arguments (both input and output).
template <bool Rnd, typename F, typename Arg0, typename... Args>
inline real &mpfr_nary_op_impl(::mpfr_prec_t min_prec, const F &f, real &rop, Arg0 &&arg0, Args &&...args)
{
    // Make sure min_prec is valid.
    // NOTE: min_prec == 0 is ok, it just means
    // p below will be inited with arg0's precision
    // rather than min_prec.
    assert(min_prec == 0 || real_prec_check(min_prec));

    // This pair will contain:
    //
    // - a pointer to the largest-precision arg from which we can steal resources (may be nullptr),
    // - the largest precision among all args and min_prec (i.e., the target precision
    //   for rop).
    //
    // It is inited with arg0's precision (but no less than min_prec), and a pointer to arg0, if arg0 is a nonconst
    // rvalue ref (a nullptr otherwise).
    auto p = mpfr_nary_op_init_pair(min_prec, std::forward<Arg0>(arg0));
    // Finish setting up p by examining the remaining arguments.
    mpfr_nary_op_check_steal(p, std::forward<Args>(args)...);

    // Cache for convenience.
    const auto r_prec = rop.get_prec();

    if (p.second == r_prec) {
        // The target precision and the precision of the return value
        // match. No need to steal, just execute the function.
        mpfr_nary_func_wrapper(std::integral_constant<bool, Rnd>{}, f, rop._get_mpfr_t(), arg0.get_mpfr_t(),
                               args.get_mpfr_t()...);
    } else {
        if (r_prec > p.second) {
            // The precision of the return value is larger than the target precision.
            // We can reset its precision destructively
            // because we know it does not overlap with any operand.
            rop.set_prec_impl<false>(p.second);
            mpfr_nary_func_wrapper(std::integral_constant<bool, Rnd>{}, f, rop._get_mpfr_t(), arg0.get_mpfr_t(),
                                   args.get_mpfr_t()...);
        } else if (p.first && p.first->get_prec() == p.second) {
            // The precision of the return value is smaller than the target precision,
            // and we have a candidate for stealing with enough precision: we will use it as return
            // value and then swap out the result to rop.
            mpfr_nary_func_wrapper(std::integral_constant<bool, Rnd>{}, f, p.first->_get_mpfr_t(), arg0.get_mpfr_t(),
                                   args.get_mpfr_t()...);
            swap(*p.first, rop);
        } else {
            // The precision of the return value is smaller than the target precision,
            // and either:
            //
            // - we cannot steal from any argument, or
            // - we can steal from an argument but the selected argument
            //   does not have enough precision.
            //
            // In these cases, we will just set the precision of rop and call the function.
            //
            // NOTE: we need to set the precision without destroying the rop, as rop might
            // overlap with one of the arguments. Since this will be an increase in precision,
            // it should not entail a rounding operation.
            //
            // NOTE: we assume all the precs in the operands and min_prec are valid, so
            // we will not need to check them.
            rop.prec_round_impl<false>(p.second);
            mpfr_nary_func_wrapper(std::integral_constant<bool, Rnd>{}, f, rop._get_mpfr_t(), arg0.get_mpfr_t(),
                                   args.get_mpfr_t()...);
        }
    }

    return rop;
}

// The goal of this helper is to invoke the MPFR-like function object f with signature
//
// void f(mpfr_t out, const mpfr_t x0, const mpfr_t x1, ...)
//
// on the mpfr_t instances contained in the input real objects,
//
// f(rop._get_mpfr_t(), arg0.get_mpfr_t(), arg1.get_mpfr_t(), ...)
//
// and then return rop.
//
// The rop object will either be created within the helper with a precision
// set to max(min_prec, arg0.get_prec(), arg1.get_prec(), ...),
// or it will be one of the input arguments if it provides enough precision and
// it is passed as a non-const rvalue reference.
//
// The Rnd flag controls whether to add the rounding mode (MPFR_RNDN) at the end
// of the MPFR-like function object arguments list or not.
//
// This function requires that the MPFR-like function object being called supports
// overlapping arguments (both input and output).
template <bool Rnd, typename F, typename Arg0, typename... Args>
inline real mpfr_nary_op_return_impl(::mpfr_prec_t min_prec, const F &f, Arg0 &&arg0, Args &&...args)
{
    // Make sure min_prec is valid.
    // NOTE: min_prec == 0 is ok, it just means
    // p below will be inited with arg0's precision
    // rather than min_prec.
    assert(min_prec == 0 || real_prec_check(min_prec));

    // This pair will contain:
    //
    // - a pointer to the largest-precision arg from which we can steal resources (may be nullptr),
    // - the largest precision among all args and min_prec (i.e., the target precision
    //   for the return value).
    //
    // It is inited with arg0's precision (but no less than min_prec), and a pointer to arg0, if arg0 is a nonconst
    // rvalue ref (a nullptr otherwise).
    auto p = mpfr_nary_op_init_pair(min_prec, std::forward<Arg0>(arg0));
    // Finish setting up p by examining the remaining arguments.
    mpfr_nary_op_check_steal(p, std::forward<Args>(args)...);

    if (p.first && p.first->get_prec() == p.second) {
        // We can steal from one or more args, and the precision of
        // the largest-precision arg we can steal from matches
        // the target precision. Use it.
        mpfr_nary_func_wrapper(std::integral_constant<bool, Rnd>{}, f, p.first->_get_mpfr_t(), arg0.get_mpfr_t(),
                               args.get_mpfr_t()...);
        return std::move(*p.first);
    } else {
        // Either we cannot steal from any arg, or the candidate does not have
        // enough precision. Init a new value and use it instead.
        real retval{real::ptag{}, p.second, true};
        mpfr_nary_func_wrapper(std::integral_constant<bool, Rnd>{}, f, retval._get_mpfr_t(), arg0.get_mpfr_t(),
                               args.get_mpfr_t()...);
        return retval;
    }
}

} // namespace detail

// Ternary addition.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T, cvr_real U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &add(real &rop, T &&a, U &&b)
{
    return detail::mpfr_nary_op_impl<true>(0, ::mpfr_add, rop, std::forward<T>(a), std::forward<U>(b));
}

// Ternary subtraction.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T, cvr_real U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &sub(real &rop, T &&a, U &&b)
{
    return detail::mpfr_nary_op_impl<true>(0, ::mpfr_sub, rop, std::forward<T>(a), std::forward<U>(b));
}

// Ternary multiplication.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T, cvr_real U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &mul(real &rop, T &&a, U &&b)
{
    return detail::mpfr_nary_op_impl<true>(0, ::mpfr_mul, rop, std::forward<T>(a), std::forward<U>(b));
}

// Ternary division.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T, cvr_real U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &div(real &rop, T &&a, U &&b)
{
    return detail::mpfr_nary_op_impl<true>(0, ::mpfr_div, rop, std::forward<T>(a), std::forward<U>(b));
}

// Quaternary fused multiply–add.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T, cvr_real U, cvr_real V>
#else
template <typename T, typename U, typename V, cvr_real_enabler<T, U, V> = 0>
#endif
inline real &fma(real &rop, T &&a, U &&b, V &&c)
{
    return detail::mpfr_nary_op_impl<true>(0, ::mpfr_fma, rop, std::forward<T>(a), std::forward<U>(b),
                                           std::forward<V>(c));
}

// Quaternary fused multiply–sub.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T, cvr_real U, cvr_real V>
#else
template <typename T, typename U, typename V, cvr_real_enabler<T, U, V> = 0>
#endif
inline real &fms(real &rop, T &&a, U &&b, V &&c)
{
    return detail::mpfr_nary_op_impl<true>(0, ::mpfr_fms, rop, std::forward<T>(a), std::forward<U>(b),
                                           std::forward<V>(c));
}

// Ternary fused multiply–add.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T, cvr_real U, cvr_real V>
#else
template <typename T, typename U, typename V, cvr_real_enabler<T, U, V> = 0>
#endif
inline real fma(T &&a, U &&b, V &&c)
{
    return detail::mpfr_nary_op_return_impl<true>(0, ::mpfr_fma, std::forward<T>(a), std::forward<U>(b),
                                                  std::forward<V>(c));
}

// Ternary fused multiply–sub.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T, cvr_real U, cvr_real V>
#else
template <typename T, typename U, typename V, cvr_real_enabler<T, U, V> = 0>
#endif
inline real fms(T &&a, U &&b, V &&c)
{
    return detail::mpfr_nary_op_return_impl<true>(0, ::mpfr_fms, std::forward<T>(a), std::forward<U>(b),
                                                  std::forward<V>(c));
}

// mul2/div2 primitives.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real mul_2ui(T &&x, unsigned long n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_mul_2ui(r, o, n, MPFR_RNDN); };
    return detail::mpfr_nary_op_return_impl<false>(0, wrapper, std::forward<T>(x));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real &mul_2ui(real &rop, T &&x, unsigned long n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_mul_2ui(r, o, n, MPFR_RNDN); };
    return detail::mpfr_nary_op_impl<false>(0, wrapper, rop, std::forward<T>(x));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real mul_2si(T &&x, long n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_mul_2si(r, o, n, MPFR_RNDN); };
    return detail::mpfr_nary_op_return_impl<false>(0, wrapper, std::forward<T>(x));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real &mul_2si(real &rop, T &&x, long n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_mul_2si(r, o, n, MPFR_RNDN); };
    return detail::mpfr_nary_op_impl<false>(0, wrapper, rop, std::forward<T>(x));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real div_2ui(T &&x, unsigned long n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_div_2ui(r, o, n, MPFR_RNDN); };
    return detail::mpfr_nary_op_return_impl<false>(0, wrapper, std::forward<T>(x));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real &div_2ui(real &rop, T &&x, unsigned long n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_div_2ui(r, o, n, MPFR_RNDN); };
    return detail::mpfr_nary_op_impl<false>(0, wrapper, rop, std::forward<T>(x));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real div_2si(T &&x, long n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_div_2si(r, o, n, MPFR_RNDN); };
    return detail::mpfr_nary_op_return_impl<false>(0, wrapper, std::forward<T>(x));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real &div_2si(real &rop, T &&x, long n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_div_2si(r, o, n, MPFR_RNDN); };
    return detail::mpfr_nary_op_impl<false>(0, wrapper, rop, std::forward<T>(x));
}

// Detect NaN.
inline bool nan_p(const real &r)
{
    return r.nan_p();
}

// Detect inf.
inline bool inf_p(const real &r)
{
    return r.inf_p();
}

// Detect finite.
inline bool number_p(const real &r)
{
    return r.number_p();
}

// Detect zero.
inline bool zero_p(const real &r)
{
    return r.zero_p();
}

// Detect regular number.
inline bool regular_p(const real &r)
{
    return r.regular_p();
}

// Detect integral value.
inline bool integer_p(const real &r)
{
    return r.integer_p();
}

// Detect one.
inline bool is_one(const real &r)
{
    return r.is_one();
}

// Detect the sign.
inline int sgn(const real &r)
{
    return r.sgn();
}

// Get the sign bit.
inline bool signbit(const real &r)
{
    return r.signbit();
}

// Comparison.
MPPP_DLL_PUBLIC int cmp(const real &, const real &);

// Comparison of absolute values.
MPPP_DLL_PUBLIC int cmpabs(const real &, const real &);

// Comparison with integral multiples of powers of 2.
MPPP_DLL_PUBLIC int cmp_ui_2exp(const real &, unsigned long, ::mpfr_exp_t);
MPPP_DLL_PUBLIC int cmp_si_2exp(const real &, long, ::mpfr_exp_t);

// Equality predicate with special NaN handling.
MPPP_DLL_PUBLIC bool real_equal_to(const real &, const real &);

// Less-than predicate with special NaN and moved-from handling.
MPPP_DLL_PUBLIC bool real_lt(const real &, const real &);

// Greater-than predicate with special NaN and moved-from handling.
MPPP_DLL_PUBLIC bool real_gt(const real &, const real &);

// These are helper macros to reduce typing when dealing with the common case
// of exposing MPFR-like functions with a single argument (both variants with retval
// and with return). "name" will be the name of the mppp function, "fname" is
// the name of the MPFR-like function and "rnd" is a boolean flag that signals whether
// fname requires a rounding mode argument or not.
// The fname function must accept only mpfr_t arguments in input (plus the rounding mode if
// rnd is true).

// These are the headers of the overloads that will be produced. They are different depending
// on whether concepts are available or not.
#if defined(MPPP_HAVE_CONCEPTS)
#define MPPP_REAL_MPFR_UNARY_HEADER template <cvr_real T>
#else
#define MPPP_REAL_MPFR_UNARY_HEADER template <typename T, cvr_real_enabler<T> = 0>
#endif

#define MPPP_REAL_MPFR_UNARY_IMPL(name, fname, rnd)                                                                    \
    MPPP_REAL_MPFR_UNARY_HEADER inline real &name(real &rop, T &&op)                                                   \
    {                                                                                                                  \
        return detail::mpfr_nary_op_impl<rnd>(0, fname, rop, std::forward<T>(op));                                     \
    }                                                                                                                  \
    MPPP_REAL_MPFR_UNARY_HEADER inline real name(T &&r)                                                                \
    {                                                                                                                  \
        return detail::mpfr_nary_op_return_impl<rnd>(0, fname, std::forward<T>(r));                                    \
    }

// Machinery to expose the binary MPFR-like function fname as an mppp function called "name".
//
// Two overloads of "name" will be provided:
//
// - an overload in which the return value is passed by
//   reference as the first argument,
// - an overload which returns the result.
//
// The first overload accepts only real arguments.
//
// The second overload is generic and it accepts 2 input arguments, at least one of which must be a
// real. The other argument, if not real, will be converted to real in the usual way.
//
// The rnd param (a boolean) indicates if fname requires a rounding mode as last argument or not.
//
// The fname function must accept only mpfr_t arguments in input (plus the rounding mode if
// rnd is true).

// These are the headers of the overloads that will be produced. They are different depending
// on whether concepts are available or not.
#if defined(MPPP_HAVE_CONCEPTS)
#define MPPP_REAL_MPFR_BINARY_HEADER1 template <cvr_real T, cvr_real U>
#define MPPP_REAL_MPFR_BINARY_HEADER2 template <typename T, real_op_types<T> U>
#else
#define MPPP_REAL_MPFR_BINARY_HEADER1 template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#define MPPP_REAL_MPFR_BINARY_HEADER2                                                                                  \
    template <typename T, typename U, detail::enable_if_t<are_real_op_types<U, T>::value, int> = 0>
#endif

// The actual macro.
#define MPPP_REAL_MPFR_BINARY_IMPL(name, fname, rnd)                                                                   \
    /* The overload which accepts the return value in input. */                                                        \
    MPPP_REAL_MPFR_BINARY_HEADER1 inline real &name(real &rop, T &&y, U &&x)                                           \
    {                                                                                                                  \
        return detail::mpfr_nary_op_impl<rnd>(0, fname, rop, std::forward<T>(y), std::forward<U>(x));                  \
    }                                                                                                                  \
    /* Implementation details of the other overload. */                                                                \
    namespace detail                                                                                                   \
    {                                                                                                                  \
    /* Both arguments are real. */                                                                                     \
    template <typename T, typename U, cvr_real_enabler<T, U> = 0>                                                      \
    inline real dispatch_##name(T &&y, U &&x)                                                                          \
    {                                                                                                                  \
        return mpfr_nary_op_return_impl<rnd>(0, fname, std::forward<T>(y), std::forward<U>(x));                        \
    }                                                                                                                  \
    /* Only the first argument is real. */                                                                             \
    template <typename T, typename U,                                                                                  \
              enable_if_t<conjunction<is_cvr_real<T>, is_real_interoperable<U>>::value, int> = 0>                      \
    inline real dispatch_##name(T &&a, const U &x)                                                                     \
    {                                                                                                                  \
        MPPP_MAYBE_TLS real tmp;                                                                                       \
        tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));                                                   \
        tmp.set(x);                                                                                                    \
        return dispatch_##name(std::forward<T>(a), tmp);                                                               \
    }                                                                                                                  \
    /* Only the second argument is real. */                                                                            \
    template <typename T, typename U,                                                                                  \
              enable_if_t<conjunction<is_real_interoperable<T>, is_cvr_real<U>>::value, int> = 0>                      \
    inline real dispatch_##name(const T &x, U &&a)                                                                     \
    {                                                                                                                  \
        MPPP_MAYBE_TLS real tmp;                                                                                       \
        tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));                                                   \
        tmp.set(x);                                                                                                    \
        return dispatch_##name(tmp, std::forward<U>(a));                                                               \
    }                                                                                                                  \
    }                                                                                                                  \
    /* The overload which returns the result. */                                                                       \
    MPPP_REAL_MPFR_BINARY_HEADER2 inline real name(T &&y, U &&x)                                                       \
    {                                                                                                                  \
        return detail::dispatch_##name(std::forward<T>(y), std::forward<U>(x));                                        \
    }

// Neg and abs.
MPPP_REAL_MPFR_UNARY_IMPL(neg, ::mpfr_neg, true)
MPPP_REAL_MPFR_UNARY_IMPL(abs, ::mpfr_abs, true)

// Positive difference.
MPPP_REAL_MPFR_BINARY_IMPL(dim, ::mpfr_dim, true)

// Square root.
MPPP_REAL_MPFR_UNARY_IMPL(sqrt, ::mpfr_sqrt, true)

#if defined(MPPP_WITH_ARB)

// sqrt1pm1.
MPPP_REAL_MPFR_UNARY_IMPL(sqrt1pm1, detail::arb_sqrt1pm1, false)

#endif

// Reciprocal square root.
MPPP_REAL_MPFR_UNARY_IMPL(rec_sqrt, ::mpfr_rec_sqrt, true)

// Cubic root.
MPPP_REAL_MPFR_UNARY_IMPL(cbrt, ::mpfr_cbrt, true)

#if defined(MPPP_MPFR_HAVE_MPFR_ROOTN_UI)

// K-th root.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real &rootn_ui(real &rop, T &&op, unsigned long k)
{
    auto wrapper = [k](::mpfr_t r, const ::mpfr_t o) { ::mpfr_rootn_ui(r, o, k, MPFR_RNDN); };
    return detail::mpfr_nary_op_impl<false>(0, wrapper, rop, std::forward<T>(op));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real rootn_ui(T &&r, unsigned long k)
{
    auto wrapper = [k](::mpfr_t rop, const ::mpfr_t op) { ::mpfr_rootn_ui(rop, op, k, MPFR_RNDN); };
    return detail::mpfr_nary_op_return_impl<false>(0, wrapper, std::forward<T>(r));
}

#endif

// Ternary exponentiation.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T, cvr_real U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &pow(real &rop, T &&op1, U &&op2)
{
    return detail::mpfr_nary_op_impl<true>(0, ::mpfr_pow, rop, std::forward<T>(op1), std::forward<U>(op2));
}

namespace detail
{

// real-real.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cvr_real<U>>::value, int> = 0>
inline real dispatch_real_pow(T &&op1, U &&op2)
{
    return mpfr_nary_op_return_impl<true>(0, ::mpfr_pow, std::forward<T>(op1), std::forward<U>(op2));
}

// real-integer.
template <typename T, std::size_t SSize, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline real dispatch_real_pow(T &&a, const integer<SSize> &n)
{
    auto wrapper = [&n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_pow_z(r, o, n.get_mpz_view(), MPFR_RNDN); };

    // NOTE: in these mpfr_nary_op_return_impl() invocations, we are passing a min_prec
    // which is by definition valid because it is produced by an invocation of
    // real_deduce_precision() (which does clamping).
    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// real-unsigned integral.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_unsigned_integral<U>>::value, int> = 0>
inline real dispatch_real_pow(T &&a, const U &n)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_pow_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_pow(std::forward<T>(a), integer<2>{n});
    }
}

// Special casing for bool
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline real dispatch_real_pow(T &&a, const bool &n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_pow_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// real-signed integral.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_signed_integral<U>>::value, int> = 0>
inline real dispatch_real_pow(T &&a, const U &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_pow_si(r, o, static_cast<long>(n), MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_pow(std::forward<T>(a), integer<2>{n});
    }
}

// real-(floating point, rational, real128).
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<is_cpp_floating_point<U>, is_rational<U>
#if defined(MPPP_WITH_QUADMATH)
                                                              ,
                                                              std::is_same<U, real128>
#endif
                                                              >>::value,
                      int> = 0>
inline real dispatch_real_pow(T &&a, const U &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    return dispatch_real_pow(std::forward<T>(a), tmp);
}

// (everything but unsigned integral)-real.
template <
    typename T, typename U,
    enable_if_t<conjunction<is_real_interoperable<T>, negation<is_cpp_unsigned_integral<T>>, is_cvr_real<U>>::value,
                int> = 0>
inline real dispatch_real_pow(const T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    return dispatch_real_pow(tmp, std::forward<U>(a));
}

// unsigned integral-real.
template <
    typename T, typename U,
    enable_if_t<conjunction<is_real_interoperable<T>, is_cpp_unsigned_integral<T>, is_cvr_real<U>>::value, int> = 0>
inline real dispatch_real_pow(const T &n, U &&a)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_ui_pow(r, static_cast<unsigned long>(n), o, MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<U>(a));
    } else {
        return dispatch_real_pow(integer<2>{n}, std::forward<U>(a));
    }
}

// Special casing for bool.
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline real dispatch_real_pow(const bool &n, T &&a)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_ui_pow(r, static_cast<unsigned long>(n), o, MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

} // namespace detail

// Binary exponentiation.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_op_types<T, U>::value, int> = 0>
#endif
inline real pow(T &&op1, U &&op2)
{
    return detail::dispatch_real_pow(std::forward<T>(op1), std::forward<U>(op2));
}

// Squaring.
MPPP_REAL_MPFR_UNARY_IMPL(sqr, ::mpfr_sqr, true)

// Trigonometric functions.
MPPP_REAL_MPFR_UNARY_IMPL(sin, ::mpfr_sin, true)
MPPP_REAL_MPFR_UNARY_IMPL(cos, ::mpfr_cos, true)
MPPP_REAL_MPFR_UNARY_IMPL(tan, ::mpfr_tan, true)
MPPP_REAL_MPFR_UNARY_IMPL(sec, ::mpfr_sec, true)
MPPP_REAL_MPFR_UNARY_IMPL(csc, ::mpfr_csc, true)
MPPP_REAL_MPFR_UNARY_IMPL(cot, ::mpfr_cot, true)

#if defined(MPPP_WITH_ARB)

MPPP_REAL_MPFR_UNARY_IMPL(sin_pi, detail::arb_sin_pi, false)
MPPP_REAL_MPFR_UNARY_IMPL(cos_pi, detail::arb_cos_pi, false)
MPPP_REAL_MPFR_UNARY_IMPL(tan_pi, detail::arb_tan_pi, false)
MPPP_REAL_MPFR_UNARY_IMPL(cot_pi, detail::arb_cot_pi, false)
MPPP_REAL_MPFR_UNARY_IMPL(sinc, detail::arb_sinc, false)
MPPP_REAL_MPFR_UNARY_IMPL(sinc_pi, detail::arb_sinc_pi, false)

#endif

MPPP_REAL_MPFR_UNARY_IMPL(asin, ::mpfr_asin, true)
MPPP_REAL_MPFR_UNARY_IMPL(acos, ::mpfr_acos, true)
MPPP_REAL_MPFR_UNARY_IMPL(atan, ::mpfr_atan, true)

// sin and cos at the same time.
// NOTE: we don't have the machinery to steal resources
// for multiple retvals, thus we do a manual implementation
// of this function. We keep the signature with cvr_real
// for consistency with the other functions.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline void sin_cos(real &sop, real &cop, T &&op)
{
    if (mppp_unlikely(&sop == &cop)) {
        throw std::invalid_argument(
            "In the real sin_cos() function, the return values 'sop' and 'cop' must be distinct objects");
    }

    // Set the precision of sop and cop to the
    // precision of op.
    const auto op_prec = op.get_prec();
    // NOTE: use prec_round() to avoid issues in case
    // sop/cop overlap with op.
    sop.prec_round(op_prec);
    cop.prec_round(op_prec);

    // Run the mpfr function.
    ::mpfr_sin_cos(sop._get_mpfr_t(), cop._get_mpfr_t(), op.get_mpfr_t(), MPFR_RNDN);
}

MPPP_REAL_MPFR_BINARY_IMPL(atan2, ::mpfr_atan2, true)

// Hyperbolic functions.
MPPP_REAL_MPFR_UNARY_IMPL(sinh, ::mpfr_sinh, true)
MPPP_REAL_MPFR_UNARY_IMPL(cosh, ::mpfr_cosh, true)
MPPP_REAL_MPFR_UNARY_IMPL(tanh, ::mpfr_tanh, true)
MPPP_REAL_MPFR_UNARY_IMPL(sech, ::mpfr_sech, true)
MPPP_REAL_MPFR_UNARY_IMPL(csch, ::mpfr_csch, true)
MPPP_REAL_MPFR_UNARY_IMPL(coth, ::mpfr_coth, true)
MPPP_REAL_MPFR_UNARY_IMPL(asinh, ::mpfr_asinh, true)
MPPP_REAL_MPFR_UNARY_IMPL(acosh, ::mpfr_acosh, true)
MPPP_REAL_MPFR_UNARY_IMPL(atanh, ::mpfr_atanh, true)

// sinh and cosh at the same time.
// NOTE: we don't have the machinery to steal resources
// for multiple retvals, thus we do a manual implementation
// of this function. We keep the signature with cvr_real
// for consistency with the other functions.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline void sinh_cosh(real &sop, real &cop, T &&op)
{
    if (mppp_unlikely(&sop == &cop)) {
        throw std::invalid_argument(
            "In the real sinh_cosh() function, the return values 'sop' and 'cop' must be distinct objects");
    }

    // Set the precision of sop and cop to the
    // precision of op.
    const auto op_prec = op.get_prec();
    // NOTE: use prec_round() to avoid issues in case
    // sop/cop overlap with op.
    sop.prec_round(op_prec);
    cop.prec_round(op_prec);

    // Run the mpfr function.
    ::mpfr_sinh_cosh(sop._get_mpfr_t(), cop._get_mpfr_t(), op.get_mpfr_t(), MPFR_RNDN);
}

// Exponentials and logarithms.
MPPP_REAL_MPFR_UNARY_IMPL(exp, ::mpfr_exp, true)
MPPP_REAL_MPFR_UNARY_IMPL(exp2, ::mpfr_exp2, true)
MPPP_REAL_MPFR_UNARY_IMPL(exp10, ::mpfr_exp10, true)
MPPP_REAL_MPFR_UNARY_IMPL(expm1, ::mpfr_expm1, true)
MPPP_REAL_MPFR_UNARY_IMPL(log, ::mpfr_log, true)
MPPP_REAL_MPFR_UNARY_IMPL(log2, ::mpfr_log2, true)
MPPP_REAL_MPFR_UNARY_IMPL(log10, ::mpfr_log10, true)
MPPP_REAL_MPFR_UNARY_IMPL(log1p, ::mpfr_log1p, true)

#if defined(MPPP_WITH_ARB)

// log_hypot.
MPPP_REAL_MPFR_BINARY_IMPL(log_hypot, detail::arb_log_hypot, false)

// log_base_ui.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real &log_base_ui(real &rop, T &&op, unsigned long b)
{
    auto wrapper = [b](::mpfr_t r, const ::mpfr_t o) { detail::arb_log_base_ui(r, o, b); };
    return detail::mpfr_nary_op_impl<false>(0, wrapper, rop, std::forward<T>(op));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real log_base_ui(T &&r, unsigned long b)
{
    auto wrapper = [b](::mpfr_t rop, const ::mpfr_t op) { detail::arb_log_base_ui(rop, op, b); };
    return detail::mpfr_nary_op_return_impl<false>(0, wrapper, std::forward<T>(r));
}

#endif

// Gamma functions.
MPPP_REAL_MPFR_UNARY_IMPL(gamma, ::mpfr_gamma, true)
MPPP_REAL_MPFR_UNARY_IMPL(lngamma, ::mpfr_lngamma, true)
MPPP_REAL_MPFR_UNARY_IMPL(lgamma, detail::real_lgamma_wrapper, true)
MPPP_REAL_MPFR_UNARY_IMPL(digamma, ::mpfr_digamma, true)

#if defined(MPPP_MPFR_HAVE_MPFR_GAMMA_INC)

// gamma_inc.
MPPP_REAL_MPFR_BINARY_IMPL(gamma_inc, ::mpfr_gamma_inc, true)

#endif

// Bessel functions.
MPPP_REAL_MPFR_UNARY_IMPL(j0, ::mpfr_j0, true)
MPPP_REAL_MPFR_UNARY_IMPL(j1, ::mpfr_j1, true)

// Bessel function of the first kind of order n.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real &jn(real &rop, long n, T &&op)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_jn(r, n, o, MPFR_RNDN); };
    return detail::mpfr_nary_op_impl<false>(0, wrapper, rop, std::forward<T>(op));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real jn(long n, T &&r)
{
    auto wrapper = [n](::mpfr_t rop, const ::mpfr_t op) { ::mpfr_jn(rop, n, op, MPFR_RNDN); };
    return detail::mpfr_nary_op_return_impl<false>(0, wrapper, std::forward<T>(r));
}

MPPP_REAL_MPFR_UNARY_IMPL(y0, ::mpfr_y0, true)
MPPP_REAL_MPFR_UNARY_IMPL(y1, ::mpfr_y1, true)

// Bessel function of the second kind of order n.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real &yn(real &rop, long n, T &&op)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_yn(r, n, o, MPFR_RNDN); };
    return detail::mpfr_nary_op_impl<false>(0, wrapper, rop, std::forward<T>(op));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real yn(long n, T &&r)
{
    auto wrapper = [n](::mpfr_t rop, const ::mpfr_t op) { ::mpfr_yn(rop, n, op, MPFR_RNDN); };
    return detail::mpfr_nary_op_return_impl<false>(0, wrapper, std::forward<T>(r));
}

#if defined(MPPP_WITH_ARB)

MPPP_REAL_MPFR_BINARY_IMPL(jx, detail::arb_hypgeom_bessel_j, false)
MPPP_REAL_MPFR_BINARY_IMPL(yx, detail::arb_hypgeom_bessel_y, false)

#endif

// Polylogarithms.
MPPP_REAL_MPFR_UNARY_IMPL(li2, detail::real_li2_wrapper, true)

#if defined(MPPP_WITH_ARB)

// Polylogarithm, integer order.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real &polylog_si(real &rop, long n, T &&op)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { detail::arb_polylog_si(r, n, o); };
    return detail::mpfr_nary_op_impl<false>(0, wrapper, rop, std::forward<T>(op));
}

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real polylog_si(long n, T &&r)
{
    auto wrapper = [n](::mpfr_t rop, const ::mpfr_t op) { detail::arb_polylog_si(rop, n, op); };
    return detail::mpfr_nary_op_return_impl<false>(0, wrapper, std::forward<T>(r));
}

// Polylogarithm, real order.
MPPP_REAL_MPFR_BINARY_IMPL(polylog, detail::arb_polylog, false)

#endif

// Other special functions.
MPPP_REAL_MPFR_UNARY_IMPL(eint, ::mpfr_eint, true)
MPPP_REAL_MPFR_UNARY_IMPL(zeta, ::mpfr_zeta, true)
MPPP_REAL_MPFR_UNARY_IMPL(erf, ::mpfr_erf, true)
MPPP_REAL_MPFR_UNARY_IMPL(erfc, ::mpfr_erfc, true)
MPPP_REAL_MPFR_UNARY_IMPL(ai, ::mpfr_ai, true)

#if defined(MPPP_WITH_ARB)

MPPP_REAL_MPFR_UNARY_IMPL(lambert_w0, detail::arb_lambert_w0, false)
MPPP_REAL_MPFR_UNARY_IMPL(lambert_wm1, detail::arb_lambert_wm1, false)

#endif

#if defined(MPPP_MPFR_HAVE_MPFR_BETA)

// beta.
MPPP_REAL_MPFR_BINARY_IMPL(beta, ::mpfr_beta, true)

#endif

// hypot.
MPPP_REAL_MPFR_BINARY_IMPL(hypot, ::mpfr_hypot, true)

// agm.
MPPP_REAL_MPFR_BINARY_IMPL(agm, ::mpfr_agm, true)

// Integer and remainder-related functions.
MPPP_REAL_MPFR_UNARY_IMPL(ceil, detail::real_ceil_wrapper, false)
MPPP_REAL_MPFR_UNARY_IMPL(floor, detail::real_floor_wrapper, false)
MPPP_REAL_MPFR_UNARY_IMPL(round, detail::real_round_wrapper, false)
#if defined(MPPP_MPFR_HAVE_MPFR_ROUNDEVEN)
MPPP_REAL_MPFR_UNARY_IMPL(roundeven, detail::real_roundeven_wrapper, false)
#endif
MPPP_REAL_MPFR_UNARY_IMPL(trunc, detail::real_trunc_wrapper, false)
MPPP_REAL_MPFR_UNARY_IMPL(frac, detail::real_frac_wrapper, false)

// modf.
// NOTE: we don't have the machinery to steal resources
// for multiple retvals, thus we do a manual implementation
// of this function. We keep the signature with cvr_real
// for consistency with the other functions.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline void modf(real &iop, real &fop, T &&op)
{
    if (mppp_unlikely(&iop == &fop)) {
        throw std::invalid_argument(
            "In the real modf() function, the return values 'iop' and 'fop' must be distinct objects");
    }
    if (mppp_unlikely(op.nan_p())) {
        throw std::domain_error("In the real modf() function, the input argument cannot be NaN");
    }

    // Set the precision of iop and fop to the
    // precision of op.
    const auto op_prec = op.get_prec();
    // NOTE: use prec_round() to avoid issues in case
    // iop/fop overlap with op.
    iop.prec_round(op_prec);
    fop.prec_round(op_prec);

    // Run the mpfr function.
    ::mpfr_modf(iop._get_mpfr_t(), fop._get_mpfr_t(), op.get_mpfr_t(), MPFR_RNDN);
}

MPPP_REAL_MPFR_BINARY_IMPL(fmod, ::mpfr_fmod, true)
MPPP_REAL_MPFR_BINARY_IMPL(remainder, ::mpfr_remainder, true)

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T, cvr_real U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &remquo(real &rop, long *q, T &&x, U &&y)
{
    auto wrapper = [q](::mpfr_t r, const ::mpfr_t o1, const ::mpfr_t o2) { ::mpfr_remquo(r, q, o1, o2, MPFR_RNDN); };
    return detail::mpfr_nary_op_impl<false>(0, wrapper, rop, std::forward<T>(x), std::forward<U>(y));
}

#if defined(MPPP_MPFR_HAVE_MPFR_FMODQUO)

#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T, cvr_real U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &fmodquo(real &rop, long *q, T &&x, U &&y)
{
    auto wrapper = [q](::mpfr_t r, const ::mpfr_t o1, const ::mpfr_t o2) { ::mpfr_fmodquo(r, q, o1, o2, MPFR_RNDN); };
    return detail::mpfr_nary_op_impl<false>(0, wrapper, rop, std::forward<T>(x), std::forward<U>(y));
}

#endif

#undef MPPP_REAL_MPFR_UNARY_HEADER
#undef MPPP_REAL_MPFR_UNARY_IMPL
#undef MPPP_REAL_MPFR_BINARY_HEADER1
#undef MPPP_REAL_MPFR_BINARY_HEADER2
#undef MPPP_REAL_MPFR_BINARY_IMPL

// Output stream operator.
MPPP_DLL_PUBLIC std::ostream &operator<<(std::ostream &, const real &);

#if defined(MPPP_MPFR_HAVE_MPFR_GET_STR_NDIGITS)

// Get the number of significant digits required for a round-tripping representation.
MPPP_DLL_PUBLIC std::size_t get_str_ndigits(const real &, int = 10);

#endif

// Binary serialization.
MPPP_DLL_PUBLIC std::size_t binary_size(const real &);

// Save in binary format.
template <typename T>
inline auto binary_save(const real &x, T &&dest) -> decltype(x.binary_save(std::forward<T>(dest)))
{
    return x.binary_save(std::forward<T>(dest));
}

// Load in binary format.
template <typename T>
inline auto binary_load(real &x, T &&src) -> decltype(x.binary_load(std::forward<T>(src)))
{
    return x.binary_load(std::forward<T>(src));
}

// Constants.
MPPP_DLL_PUBLIC real real_pi(::mpfr_prec_t);
MPPP_DLL_PUBLIC real &real_pi(real &);
MPPP_DLL_PUBLIC real real_log2(::mpfr_prec_t);
MPPP_DLL_PUBLIC real &real_log2(real &);
MPPP_DLL_PUBLIC real real_euler(::mpfr_prec_t);
MPPP_DLL_PUBLIC real &real_euler(real &);
MPPP_DLL_PUBLIC real real_catalan(::mpfr_prec_t);
MPPP_DLL_PUBLIC real &real_catalan(real &);

// Identity operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real operator+(T &&r)
{
    return std::forward<T>(r);
}

namespace detail
{

// real-real.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cvr_real<U>>::value, int> = 0>
inline real dispatch_real_binary_add(T &&a, U &&b)
{
    return mpfr_nary_op_return_impl<true>(0, ::mpfr_add, std::forward<T>(a), std::forward<U>(b));
}

// real-integer.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_add(T &&a, const integer<SSize> &n)
{
    auto wrapper = [&n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_z(r, o, n.get_mpz_view(), MPFR_RNDN); };

    // NOTE: in these mpfr_nary_op_return_impl() invocations, we are passing a min_prec
    // which is by definition valid because it is produced by an invocation of
    // real_deduce_precision() (which does clamping).
    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// integer-real.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_add(const integer<SSize> &n, T &&a)
{
    return dispatch_real_binary_add(std::forward<T>(a), n);
}

// real-unsigned integral.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_unsigned_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_add(T &&a, const U &n)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_add(std::forward<T>(a), integer<2>{n});
    }
}

// real-bool.
// NOTE: make this explicit (rather than letting bool fold into
// the unsigned integrals overload) in order to avoid MSVC warnings.
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline real dispatch_real_binary_add(T &&a, const bool &n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };
    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// unsigned integral-real.
template <typename T, typename U, enable_if_t<conjunction<is_cpp_unsigned_integral<T>, is_cvr_real<U>>::value, int> = 0>
inline real dispatch_real_binary_add(const T &n, U &&a)
{
    return dispatch_real_binary_add(std::forward<U>(a), n);
}

// real-signed integral.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_signed_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_add(T &&a, const U &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_si(r, o, static_cast<long>(n), MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_add(std::forward<T>(a), integer<2>{n});
    }
}

// signed integral-real.
template <typename T, typename U, enable_if_t<conjunction<is_cpp_signed_integral<T>, is_cvr_real<U>>::value, int> = 0>
inline real dispatch_real_binary_add(const T &n, U &&a)
{
    return dispatch_real_binary_add(std::forward<U>(a), n);
}

// real-rational.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_add(T &&a, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);

    auto wrapper = [&qv](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_q(r, o, &qv, MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(q), wrapper, std::forward<T>(a));
}

// rational-real.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_add(const rational<SSize> &q, T &&a)
{
    return dispatch_real_binary_add(std::forward<T>(a), q);
}

// real-(float, double).
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, float>, std::is_same<U, double>>>::value,
                      int> = 0>
inline real dispatch_real_binary_add(T &&a, const U &x)
{
    // NOTE: the MPFR docs state that mpfr_add_d() assumes that
    // the radix of double is a power of 2. If we ever run into platforms
    // for which this is not true, we can add a compile-time dispatch
    // that uses the long double implementation instead.
    constexpr auto dradix = static_cast<unsigned>(std::numeric_limits<double>::radix);
    static_assert(!(dradix & (dradix - 1)), "mpfr_add_d() requires the radix of the 'double' type to be a power of 2.");

    auto wrapper = [x](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_d(r, o, static_cast<double>(x), MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(x), wrapper, std::forward<T>(a));
}

// (float, double)-real.
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<U>, disjunction<std::is_same<T, float>, std::is_same<T, double>>>::value,
                      int> = 0>
inline real dispatch_real_binary_add(const T &x, U &&a)
{
    return dispatch_real_binary_add(std::forward<U>(a), x);
}

// real-(long double, real128).
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, long double>
#if defined(MPPP_WITH_QUADMATH)
                                                              ,
                                                              std::is_same<U, real128>
#endif
                                                              >>::value,
                      int> = 0>
inline real dispatch_real_binary_add(T &&a, const U &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    return dispatch_real_binary_add(std::forward<T>(a), tmp);
}

// (long double, real128)-real.
template <typename T, typename U,
          enable_if_t<conjunction<disjunction<std::is_same<T, long double>
#if defined(MPPP_WITH_QUADMATH)
                                              ,
                                              std::is_same<T, real128>
#endif
                                              >,
                                  is_cvr_real<U>>::value,
                      int> = 0>
inline real dispatch_real_binary_add(const T &x, U &&a)
{
    return dispatch_real_binary_add(std::forward<U>(a), x);
}

} // namespace detail

// Binary addition.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_op_types<T, U>::value, int> = 0>
#endif
inline real operator+(T &&a, U &&b)
{
    return detail::dispatch_real_binary_add(std::forward<T>(a), std::forward<U>(b));
}

namespace detail
{

// real-real.
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline void dispatch_real_in_place_add(real &a, T &&b)
{
    add(a, a, std::forward<T>(b));
}

MPPP_DLL_PUBLIC void dispatch_real_in_place_add_integer_impl(real &, const ::mpz_t, ::mpfr_prec_t);

// real-integer.
template <std::size_t SSize>
inline void dispatch_real_in_place_add(real &a, const integer<SSize> &n)
{
    dispatch_real_in_place_add_integer_impl(a, n.get_mpz_view(), real_deduce_precision(n));
}

// real-unsigned C++ integral.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline void dispatch_real_in_place_add(real &a, const T &n)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

        mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
    } else {
        dispatch_real_in_place_add(a, integer<2>{n});
    }
}

// real-bool.
// NOTE: make this explicit (rather than letting bool fold into
// the unsigned integrals overload) in order to avoid MSVC warnings.
MPPP_DLL_PUBLIC void dispatch_real_in_place_add(real &, bool);

// real-signed C++ integral.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline void dispatch_real_in_place_add(real &a, const T &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_si(r, o, static_cast<long>(n), MPFR_RNDN); };

        mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
    } else {
        dispatch_real_in_place_add(a, integer<2>{n});
    }
}

MPPP_DLL_PUBLIC void dispatch_real_in_place_add_rational_impl(real &, const ::mpq_t, ::mpfr_prec_t);

// real-rational.
template <std::size_t SSize>
inline void dispatch_real_in_place_add(real &a, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);
    dispatch_real_in_place_add_rational_impl(a, &qv, real_deduce_precision(q));
}

// real-(float, double).
MPPP_DLL_PUBLIC void dispatch_real_in_place_add(real &, const float &);
MPPP_DLL_PUBLIC void dispatch_real_in_place_add(real &, const double &);

// real-(long double, real128).
MPPP_DLL_PUBLIC void dispatch_real_in_place_add(real &, const long double &);
#if defined(MPPP_WITH_QUADMATH)
MPPP_DLL_PUBLIC void dispatch_real_in_place_add(real &, const real128 &);
#endif

// (real interoperable)-real.
template <typename T, typename U, enable_if_t<is_real_interoperable<T>::value, int> = 0>
inline void dispatch_real_in_place_add(T &x, U &&a)
{
    x = static_cast<T>(x + std::forward<U>(a));
}

} // namespace detail

// In-place addition.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_in_place_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_in_place_op_types<T, U>::value, int> = 0>
#endif
inline T &operator+=(T &a, U &&b)
{
    detail::dispatch_real_in_place_add(a, std::forward<U>(b));
    return a;
}

// Prefix increment.
MPPP_DLL_PUBLIC real &operator++(real &);

// Suffix increment.
MPPP_DLL_PUBLIC real operator++(real &, int);

// Negated copy.
#if defined(MPPP_HAVE_CONCEPTS)
template <cvr_real T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real operator-(T &&r)
{
    real retval{std::forward<T>(r)};
    retval.neg();
    return retval;
}

namespace detail
{

// real-real.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cvr_real<U>>::value, int> = 0>
inline real dispatch_real_binary_sub(T &&a, U &&b)
{
    return mpfr_nary_op_return_impl<true>(0, ::mpfr_sub, std::forward<T>(a), std::forward<U>(b));
}

// real-integer.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_sub(T &&a, const integer<SSize> &n)
{
    auto wrapper = [&n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_z(r, o, n.get_mpz_view(), MPFR_RNDN); };

    // NOTE: in these mpfr_nary_op_return_impl() invocations, we are passing a min_prec
    // which is by definition valid because it is produced by an invocation of
    // real_deduce_precision() (which does clamping).
    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// integer-real.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_sub(const integer<SSize> &n, T &&a)
{
    auto wrapper = [&n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_z_sub(r, n.get_mpz_view(), o, MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// real-unsigned integral.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_unsigned_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_sub(T &&a, const U &n)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_sub(std::forward<T>(a), integer<2>{n});
    }
}

// unsigned integral-real.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_unsigned_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_sub(const U &n, T &&a)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_ui_sub(r, static_cast<unsigned long>(n), o, MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_sub(integer<2>{n}, std::forward<T>(a));
    }
}

// real-bool.
// NOTE: make this explicit (rather than letting bool fold into
// the unsigned integrals overload) in order to avoid MSVC warnings.
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline real dispatch_real_binary_sub(T &&a, const bool &n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };
    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// bool-real.
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline real dispatch_real_binary_sub(const bool &n, T &&a)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_ui_sub(r, static_cast<unsigned long>(n), o, MPFR_RNDN); };
    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// real-signed integral.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_signed_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_sub(T &&a, const U &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_si(r, o, static_cast<long>(n), MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_sub(std::forward<T>(a), integer<2>{n});
    }
}

// signed integral-real.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_signed_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_sub(const U &n, T &&a)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_si_sub(r, static_cast<long>(n), o, MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_sub(integer<2>{n}, std::forward<T>(a));
    }
}

// real-rational.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_sub(T &&a, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);

    auto wrapper = [&qv](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_q(r, o, &qv, MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(q), wrapper, std::forward<T>(a));
}

// rational-real.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_sub(const rational<SSize> &q, T &&a)
{
    // NOTE: apparently there's no mpfr_q_sub() primitive.
    return -dispatch_real_binary_sub(std::forward<T>(a), q);
}

// real-(float, double).
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, float>, std::is_same<U, double>>>::value,
                      int> = 0>
inline real dispatch_real_binary_sub(T &&a, const U &x)
{
    // NOTE: the MPFR docs state that mpfr_sub_d() assumes that
    // the radix of double is a power of 2. If we ever run into platforms
    // for which this is not true, we can add a compile-time dispatch
    // that uses the long double implementation instead.
    constexpr auto dradix = static_cast<unsigned>(std::numeric_limits<double>::radix);
    static_assert(!(dradix & (dradix - 1)), "mpfr_sub_d() requires the radix of the 'double' type to be a power of 2.");

    auto wrapper = [x](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_d(r, o, static_cast<double>(x), MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(x), wrapper, std::forward<T>(a));
}

// (float, double)-real.
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, float>, std::is_same<U, double>>>::value,
                      int> = 0>
inline real dispatch_real_binary_sub(const U &x, T &&a)
{
    // NOTE: the MPFR docs state that mpfr_d_sub() assumes that
    // the radix of double is a power of 2. If we ever run into platforms
    // for which this is not true, we can add a compile-time dispatch
    // that uses the long double implementation instead.
    constexpr auto dradix = static_cast<unsigned>(std::numeric_limits<double>::radix);
    static_assert(!(dradix & (dradix - 1)), "mpfr_d_sub() requires the radix of the 'double' type to be a power of 2.");

    auto wrapper = [x](::mpfr_t r, const ::mpfr_t o) { ::mpfr_d_sub(r, static_cast<double>(x), o, MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(x), wrapper, std::forward<T>(a));
}

// real-(long double, real128).
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, long double>
#if defined(MPPP_WITH_QUADMATH)
                                                              ,
                                                              std::is_same<U, real128>
#endif
                                                              >>::value,
                      int> = 0>
inline real dispatch_real_binary_sub(T &&a, const U &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    return dispatch_real_binary_sub(std::forward<T>(a), tmp);
}

// (long double, real128)-real.
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, long double>
#if defined(MPPP_WITH_QUADMATH)
                                                              ,
                                                              std::is_same<U, real128>
#endif
                                                              >>::value,
                      int> = 0>
inline real dispatch_real_binary_sub(const U &x, T &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    return dispatch_real_binary_sub(tmp, std::forward<T>(a));
}

} // namespace detail

// Binary subtraction.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_op_types<T, U>::value, int> = 0>
#endif
inline real operator-(T &&a, U &&b)
{
    return detail::dispatch_real_binary_sub(std::forward<T>(a), std::forward<U>(b));
}

namespace detail
{

// real-real.
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline void dispatch_real_in_place_sub(real &a, T &&b)
{
    sub(a, a, std::forward<T>(b));
}

MPPP_DLL_PUBLIC void dispatch_real_in_place_sub_integer_impl(real &, const ::mpz_t, ::mpfr_prec_t);

// real-integer.
template <std::size_t SSize>
inline void dispatch_real_in_place_sub(real &a, const integer<SSize> &n)
{
    dispatch_real_in_place_sub_integer_impl(a, n.get_mpz_view(), real_deduce_precision(n));
}

// real-unsigned C++ integral.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline void dispatch_real_in_place_sub(real &a, const T &n)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

        mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
    } else {
        dispatch_real_in_place_sub(a, integer<2>{n});
    }
}

// real-bool.
// NOTE: make this explicit (rather than letting bool fold into
// the unsigned integrals overload) in order to avoid MSVC warnings.
MPPP_DLL_PUBLIC void dispatch_real_in_place_sub(real &, bool);

// real-signed C++ integral.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline void dispatch_real_in_place_sub(real &a, const T &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_si(r, o, static_cast<long>(n), MPFR_RNDN); };

        mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
    } else {
        dispatch_real_in_place_sub(a, integer<2>{n});
    }
}

MPPP_DLL_PUBLIC void dispatch_real_in_place_sub_rational_impl(real &, const ::mpq_t, ::mpfr_prec_t);

// real-rational.
template <std::size_t SSize>
inline void dispatch_real_in_place_sub(real &a, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);
    dispatch_real_in_place_sub_rational_impl(a, &qv, real_deduce_precision(q));
}

// real-(float, double).
MPPP_DLL_PUBLIC void dispatch_real_in_place_sub(real &, const float &);
MPPP_DLL_PUBLIC void dispatch_real_in_place_sub(real &, const double &);

// real-(long double, real128).
MPPP_DLL_PUBLIC void dispatch_real_in_place_sub(real &, const long double &);
#if defined(MPPP_WITH_QUADMATH)
MPPP_DLL_PUBLIC void dispatch_real_in_place_sub(real &, const real128 &);
#endif

// (real interoperable)-real.
template <typename T, typename U, enable_if_t<is_real_interoperable<T>::value, int> = 0>
inline void dispatch_real_in_place_sub(T &x, U &&a)
{
    x = static_cast<T>(x - std::forward<U>(a));
}

} // namespace detail

// In-place subtraction.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_in_place_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_in_place_op_types<T, U>::value, int> = 0>
#endif
inline T &operator-=(T &a, U &&b)
{
    detail::dispatch_real_in_place_sub(a, std::forward<U>(b));
    return a;
}

// Prefix decrement.
MPPP_DLL_PUBLIC real &operator--(real &);

// Suffix decrement.
MPPP_DLL_PUBLIC real operator--(real &, int);

namespace detail
{

// real-real.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cvr_real<U>>::value, int> = 0>
inline real dispatch_real_binary_mul(T &&a, U &&b)
{
    return mpfr_nary_op_return_impl<true>(0, ::mpfr_mul, std::forward<T>(a), std::forward<U>(b));
}

// real-integer.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_mul(T &&a, const integer<SSize> &n)
{
    auto wrapper = [&n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_mul_z(r, o, n.get_mpz_view(), MPFR_RNDN); };

    // NOTE: in these mpfr_nary_op_return_impl() invocations, we are passing a min_prec
    // which is by definition valid because it is produced by an invocation of
    // real_deduce_precision() (which does clamping).
    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// integer-real.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_mul(const integer<SSize> &n, T &&a)
{
    return dispatch_real_binary_mul(std::forward<T>(a), n);
}

// real-unsigned integral.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_unsigned_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_mul(T &&a, const U &n)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_mul_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_mul(std::forward<T>(a), integer<2>{n});
    }
}

// real-bool.
// NOTE: make this explicit (rather than letting bool fold into
// the unsigned integrals overload) in order to avoid MSVC warnings.
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline real dispatch_real_binary_mul(T &&a, const bool &n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_mul_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };
    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// unsigned integral-real.
template <typename T, typename U, enable_if_t<conjunction<is_cpp_unsigned_integral<T>, is_cvr_real<U>>::value, int> = 0>
inline real dispatch_real_binary_mul(const T &n, U &&a)
{
    return dispatch_real_binary_mul(std::forward<U>(a), n);
}

// real-signed integral.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_signed_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_mul(T &&a, const U &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_mul_si(r, o, static_cast<long>(n), MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_mul(std::forward<T>(a), integer<2>{n});
    }
}

// signed integral-real.
template <typename T, typename U, enable_if_t<conjunction<is_cpp_signed_integral<T>, is_cvr_real<U>>::value, int> = 0>
inline real dispatch_real_binary_mul(const T &n, U &&a)
{
    return dispatch_real_binary_mul(std::forward<U>(a), n);
}

// real-rational.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_mul(T &&a, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);

    auto wrapper = [&qv](::mpfr_t r, const ::mpfr_t o) { ::mpfr_mul_q(r, o, &qv, MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(q), wrapper, std::forward<T>(a));
}

// rational-real.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_mul(const rational<SSize> &q, T &&a)
{
    return dispatch_real_binary_mul(std::forward<T>(a), q);
}

// real-(float, double).
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, float>, std::is_same<U, double>>>::value,
                      int> = 0>
inline real dispatch_real_binary_mul(T &&a, const U &x)
{
    // NOTE: the MPFR docs state that mpfr_mul_d() assumes that
    // the radix of double is a power of 2. If we ever run into platforms
    // for which this is not true, we can add a compile-time dispatch
    // that uses the long double implementation instead.
    constexpr auto dradix = static_cast<unsigned>(std::numeric_limits<double>::radix);
    static_assert(!(dradix & (dradix - 1)), "mpfr_mul_d() requires the radix of the 'double' type to be a power of 2.");

    auto wrapper = [x](::mpfr_t r, const ::mpfr_t o) { ::mpfr_mul_d(r, o, static_cast<double>(x), MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(x), wrapper, std::forward<T>(a));
}

// (float, double)-real.
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<U>, disjunction<std::is_same<T, float>, std::is_same<T, double>>>::value,
                      int> = 0>
inline real dispatch_real_binary_mul(const T &x, U &&a)
{
    return dispatch_real_binary_mul(std::forward<U>(a), x);
}

// real-(long double, real128).
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, long double>
#if defined(MPPP_WITH_QUADMATH)
                                                              ,
                                                              std::is_same<U, real128>
#endif
                                                              >>::value,
                      int> = 0>
inline real dispatch_real_binary_mul(T &&a, const U &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    return dispatch_real_binary_mul(std::forward<T>(a), tmp);
}

// (long double, real128)-real.
template <typename T, typename U,
          enable_if_t<conjunction<disjunction<std::is_same<T, long double>
#if defined(MPPP_WITH_QUADMATH)
                                              ,
                                              std::is_same<T, real128>
#endif
                                              >,
                                  is_cvr_real<U>>::value,
                      int> = 0>
inline real dispatch_real_binary_mul(const T &x, U &&a)
{
    return dispatch_real_binary_mul(std::forward<U>(a), x);
}

} // namespace detail

// Binary multiplication.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_op_types<T, U>::value, int> = 0>
#endif
inline real operator*(T &&a, U &&b)
{
    return detail::dispatch_real_binary_mul(std::forward<T>(a), std::forward<U>(b));
}

namespace detail
{

// real-real.
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline void dispatch_real_in_place_mul(real &a, T &&b)
{
    mul(a, a, std::forward<T>(b));
}

MPPP_DLL_PUBLIC void dispatch_real_in_place_mul_integer_impl(real &, const ::mpz_t, ::mpfr_prec_t);

// real-integer.
template <std::size_t SSize>
inline void dispatch_real_in_place_mul(real &a, const integer<SSize> &n)
{
    dispatch_real_in_place_mul_integer_impl(a, n.get_mpz_view(), real_deduce_precision(n));
}

// real-unsigned C++ integral.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline void dispatch_real_in_place_mul(real &a, const T &n)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_mul_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

        mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
    } else {
        dispatch_real_in_place_mul(a, integer<2>{n});
    }
}

// real-bool.
// NOTE: make this explicit (rather than letting bool fold into
// the unsigned integrals overload) in order to avoid MSVC warnings.
MPPP_DLL_PUBLIC void dispatch_real_in_place_mul(real &, bool);

// real-signed C++ integral.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline void dispatch_real_in_place_mul(real &a, const T &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_mul_si(r, o, static_cast<long>(n), MPFR_RNDN); };

        mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
    } else {
        dispatch_real_in_place_mul(a, integer<2>{n});
    }
}

MPPP_DLL_PUBLIC void dispatch_real_in_place_mul_rational_impl(real &, const ::mpq_t, ::mpfr_prec_t);

// real-rational.
template <std::size_t SSize>
inline void dispatch_real_in_place_mul(real &a, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);
    dispatch_real_in_place_mul_rational_impl(a, &qv, real_deduce_precision(q));
}

// real-(float, double).
MPPP_DLL_PUBLIC void dispatch_real_in_place_mul(real &, const float &);
MPPP_DLL_PUBLIC void dispatch_real_in_place_mul(real &, const double &);

// real-(long double, real128).
MPPP_DLL_PUBLIC void dispatch_real_in_place_mul(real &, const long double &);
#if defined(MPPP_WITH_QUADMATH)
MPPP_DLL_PUBLIC void dispatch_real_in_place_mul(real &, const real128 &);
#endif

// (real interoperable)-real.
template <typename T, typename U, enable_if_t<is_real_interoperable<T>::value, int> = 0>
inline void dispatch_real_in_place_mul(T &x, U &&a)
{
    x = static_cast<T>(x * std::forward<U>(a));
}

} // namespace detail

// In-place multiplication.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_in_place_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_in_place_op_types<T, U>::value, int> = 0>
#endif
inline T &operator*=(T &a, U &&b)
{
    detail::dispatch_real_in_place_mul(a, std::forward<U>(b));
    return a;
}

namespace detail
{

// real-real.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cvr_real<U>>::value, int> = 0>
inline real dispatch_real_binary_div(T &&a, U &&b)
{
    return mpfr_nary_op_return_impl<true>(0, ::mpfr_div, std::forward<T>(a), std::forward<U>(b));
}

// (long double, real128, integer, rational)-real.
// NOTE: place it here because it is used in the
// implementations below.
template <
    typename T, typename U,
    enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, long double>, is_integer<U>, is_rational<U>
#if defined(MPPP_WITH_QUADMATH)
                                                        ,
                                                        std::is_same<U, real128>
#endif
                                                        >>::value,
                int> = 0>
inline real dispatch_real_binary_div(const U &x, T &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    return dispatch_real_binary_div(tmp, std::forward<T>(a));
}

// real-integer.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_div(T &&a, const integer<SSize> &n)
{
    auto wrapper = [&n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_div_z(r, o, n.get_mpz_view(), MPFR_RNDN); };

    // NOTE: in these mpfr_nary_op_return_impl() invocations, we are passing a min_prec
    // which is by definition valid because it is produced by an invocation of
    // real_deduce_precision() (which does clamping).
    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// real-unsigned integral.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_unsigned_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_div(T &&a, const U &n)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_div_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_div(std::forward<T>(a), integer<2>{n});
    }
}

// unsigned integral-real.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_unsigned_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_div(const U &n, T &&a)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_ui_div(r, static_cast<unsigned long>(n), o, MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_div(integer<2>{n}, std::forward<T>(a));
    }
}

// real-bool.
// NOTE: make this explicit (rather than letting bool fold into
// the unsigned integrals overload) in order to avoid MSVC warnings.
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline real dispatch_real_binary_div(T &&a, const bool &n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_div_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };
    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// bool-real.
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline real dispatch_real_binary_div(const bool &n, T &&a)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_ui_div(r, static_cast<unsigned long>(n), o, MPFR_RNDN); };
    return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
}

// real-signed integral.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_signed_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_div(T &&a, const U &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_div_si(r, o, static_cast<long>(n), MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_div(std::forward<T>(a), integer<2>{n});
    }
}

// signed integral-real.
template <typename T, typename U, enable_if_t<conjunction<is_cvr_real<T>, is_cpp_signed_integral<U>>::value, int> = 0>
inline real dispatch_real_binary_div(const U &n, T &&a)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_si_div(r, static_cast<long>(n), o, MPFR_RNDN); };

        return mpfr_nary_op_return_impl<false>(real_deduce_precision(n), wrapper, std::forward<T>(a));
    } else {
        return dispatch_real_binary_div(integer<2>{n}, std::forward<T>(a));
    }
}

// real-rational.
template <typename T, std::size_t SSize>
inline real dispatch_real_binary_div(T &&a, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);

    auto wrapper = [&qv](::mpfr_t r, const ::mpfr_t o) { ::mpfr_div_q(r, o, &qv, MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(q), wrapper, std::forward<T>(a));
}

// real-(float, double).
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, float>, std::is_same<U, double>>>::value,
                      int> = 0>
inline real dispatch_real_binary_div(T &&a, const U &x)
{
    // NOTE: the MPFR docs state that mpfr_div_d() assumes that
    // the radix of double is a power of 2. If we ever run into platforms
    // for which this is not true, we can add a compile-time dispatch
    // that uses the long double implementation instead.
    constexpr auto dradix = static_cast<unsigned>(std::numeric_limits<double>::radix);
    static_assert(!(dradix & (dradix - 1)), "mpfr_div_d() requires the radix of the 'double' type to be a power of 2.");

    auto wrapper = [x](::mpfr_t r, const ::mpfr_t o) { ::mpfr_div_d(r, o, static_cast<double>(x), MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(x), wrapper, std::forward<T>(a));
}

// (float, double)-real.
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, float>, std::is_same<U, double>>>::value,
                      int> = 0>
inline real dispatch_real_binary_div(const U &x, T &&a)
{
    // NOTE: the MPFR docs state that mpfr_d_div() assumes that
    // the radix of double is a power of 2. If we ever run into platforms
    // for which this is not true, we can add a compile-time dispatch
    // that uses the long double implementation instead.
    constexpr auto dradix = static_cast<unsigned>(std::numeric_limits<double>::radix);
    static_assert(!(dradix & (dradix - 1)), "mpfr_d_div() requires the radix of the 'double' type to be a power of 2.");

    auto wrapper = [x](::mpfr_t r, const ::mpfr_t o) { ::mpfr_d_div(r, static_cast<double>(x), o, MPFR_RNDN); };

    return mpfr_nary_op_return_impl<false>(real_deduce_precision(x), wrapper, std::forward<T>(a));
}

// real-(long double, real128).
template <typename T, typename U,
          enable_if_t<conjunction<is_cvr_real<T>, disjunction<std::is_same<U, long double>
#if defined(MPPP_WITH_QUADMATH)
                                                              ,
                                                              std::is_same<U, real128>
#endif
                                                              >>::value,
                      int> = 0>
inline real dispatch_real_binary_div(T &&a, const U &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    return dispatch_real_binary_div(std::forward<T>(a), tmp);
}

} // namespace detail

// Binary division.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename U, real_op_types<U> T>
#else
template <typename T, typename U, detail::enable_if_t<are_real_op_types<T, U>::value, int> = 0>
#endif
inline real operator/(T &&a, U &&b)
{
    return detail::dispatch_real_binary_div(std::forward<T>(a), std::forward<U>(b));
}

namespace detail
{

// real-real.
template <typename T, enable_if_t<is_cvr_real<T>::value, int> = 0>
inline void dispatch_real_in_place_div(real &a, T &&b)
{
    div(a, a, std::forward<T>(b));
}

MPPP_DLL_PUBLIC void dispatch_real_in_place_div_integer_impl(real &, const ::mpz_t, ::mpfr_prec_t);

// real-integer.
template <std::size_t SSize>
inline void dispatch_real_in_place_div(real &a, const integer<SSize> &n)
{
    dispatch_real_in_place_div_integer_impl(a, n.get_mpz_view(), real_deduce_precision(n));
}

// real-unsigned C++ integral.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline void dispatch_real_in_place_div(real &a, const T &n)
{
    if (n <= nl_max<unsigned long>()) {
        auto wrapper
            = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_div_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

        mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
    } else {
        dispatch_real_in_place_div(a, integer<2>{n});
    }
}

// real-bool.
// NOTE: make this explicit (rather than letting bool fold into
// the unsigned integrals overload) in order to avoid MSVC warnings.
MPPP_DLL_PUBLIC void dispatch_real_in_place_div(real &, bool);

// real-signed C++ integral.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline void dispatch_real_in_place_div(real &a, const T &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_div_si(r, o, static_cast<long>(n), MPFR_RNDN); };

        mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
    } else {
        dispatch_real_in_place_div(a, integer<2>{n});
    }
}

MPPP_DLL_PUBLIC void dispatch_real_in_place_div_rational_impl(real &, const ::mpq_t, ::mpfr_prec_t);

// real-rational.
template <std::size_t SSize>
inline void dispatch_real_in_place_div(real &a, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);
    dispatch_real_in_place_div_rational_impl(a, &qv, real_deduce_precision(q));
}

// real-(float, double).
MPPP_DLL_PUBLIC void dispatch_real_in_place_div(real &, const float &);
MPPP_DLL_PUBLIC void dispatch_real_in_place_div(real &, const double &);

// real-(long double, real128).
MPPP_DLL_PUBLIC void dispatch_real_in_place_div(real &, const long double &);
#if defined(MPPP_WITH_QUADMATH)
MPPP_DLL_PUBLIC void dispatch_real_in_place_div(real &, const real128 &);
#endif

// (real interoperable)-real.
template <typename T, typename U, enable_if_t<is_real_interoperable<T>::value, int> = 0>
inline void dispatch_real_in_place_div(T &x, U &&a)
{
    x = static_cast<T>(x / std::forward<U>(a));
}

} // namespace detail

// In-place division.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_in_place_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_in_place_op_types<T, U>::value, int> = 0>
#endif
inline T &operator/=(T &a, U &&b)
{
    detail::dispatch_real_in_place_div(a, std::forward<U>(b));
    return a;
}

template <typename T, typename U>
using are_real_eq_op_types
    = detail::disjunction<are_real_op_types<T, U>, detail::conjunction<std::is_same<T, real>, is_cpp_complex<U>>,
                          detail::conjunction<std::is_same<U, real>, is_cpp_complex<T>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL real_eq_op_types = are_real_eq_op_types<T, U>::value;

#endif

namespace detail
{

// real-real.
MPPP_DLL_PUBLIC bool dispatch_real_equality(const real &, const real &);

MPPP_DLL_PUBLIC bool dispatch_real_equality_integer_impl(const real &, const ::mpz_t);

// real-integer.
template <std::size_t SSize>
inline bool dispatch_real_equality(const real &r, const integer<SSize> &n)
{
    return dispatch_real_equality_integer_impl(r, n.get_mpz_view());
}

// real-unsigned c++ integral.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline bool dispatch_real_equality(const real &r, const T &n)
{
    if (n <= nl_max<unsigned long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(n)) == 0;
        }
    } else {
        return dispatch_real_equality(r, integer<2>{n});
    }
}

// NOTE: treat bool explicitly in order to avoid MSVC warnings.
MPPP_DLL_PUBLIC bool dispatch_real_equality(const real &, bool);

// real-signed c++ integral.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline bool dispatch_real_equality(const real &r, const T &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_si(r.get_mpfr_t(), static_cast<long>(n)) == 0;
        }
    } else {
        return dispatch_real_equality(r, integer<2>{n});
    }
}

MPPP_DLL_PUBLIC bool dispatch_real_equality_rational_impl(const real &, const ::mpq_t);

// real-rational.
template <std::size_t SSize>
inline bool dispatch_real_equality(const real &r, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);

    return dispatch_real_equality_rational_impl(r, &qv);
}

// real-C++ floating point.
MPPP_DLL_PUBLIC bool dispatch_real_equality(const real &, const float &);
MPPP_DLL_PUBLIC bool dispatch_real_equality(const real &, const double &);
MPPP_DLL_PUBLIC bool dispatch_real_equality(const real &, const long double &);

#if defined(MPPP_WITH_QUADMATH)

// real-real128.
MPPP_DLL_PUBLIC bool dispatch_real_equality(const real &, const real128 &);

#endif

// real-std::complex.
template <typename T>
inline bool dispatch_real_equality(const real &r, const std::complex<T> &c)
{
    return c.imag() == T(0) && dispatch_real_equality(r, c.real());
}

// (anything)-real.
template <typename T>
inline bool dispatch_real_equality(const T &x, const real &r)
{
    return dispatch_real_equality(r, x);
}

} // namespace detail

// Equality operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_eq_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_eq_op_types<T, U>::value, int> = 0>
#endif
inline bool operator==(const T &a, const U &b)
{
    return detail::dispatch_real_equality(a, b);
}

// Inequality operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_eq_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_eq_op_types<T, U>::value, int> = 0>
#endif
inline bool operator!=(const T &a, const U &b)
{
    return !(a == b);
}

namespace detail
{

// real-real.
MPPP_DLL_PUBLIC bool dispatch_real_gt(const real &, const real &);

MPPP_DLL_PUBLIC bool dispatch_real_gt_integer_impl(const real &, const ::mpz_t);

// real-integer.
template <std::size_t SSize>
inline bool dispatch_real_gt(const real &r, const integer<SSize> &n)
{
    return dispatch_real_gt_integer_impl(r, n.get_mpz_view());
}

MPPP_DLL_PUBLIC bool dispatch_real_gt_integer_impl(const ::mpz_t, const real &);

// integer-real.
template <std::size_t SSize>
inline bool dispatch_real_gt(const integer<SSize> &n, const real &r)
{
    return dispatch_real_gt_integer_impl(n.get_mpz_view(), r);
}

// real-unsigned c++ integral.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline bool dispatch_real_gt(const real &r, const T &n)
{
    if (n <= nl_max<unsigned long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(n)) > 0;
        }
    } else {
        return dispatch_real_gt(r, integer<2>{n});
    }
}

// unsigned c++ integral-real.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline bool dispatch_real_gt(const T &n, const real &r)
{
    if (n <= nl_max<unsigned long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(n)) < 0;
        }
    } else {
        return dispatch_real_gt(integer<2>{n}, r);
    }
}

// NOTE: treat bool explicitly in order to avoid MSVC warnings.
MPPP_DLL_PUBLIC bool dispatch_real_gt(const real &, bool);
MPPP_DLL_PUBLIC bool dispatch_real_gt(bool, const real &);

// real-signed c++ integral.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline bool dispatch_real_gt(const real &r, const T &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_si(r.get_mpfr_t(), static_cast<long>(n)) > 0;
        }
    } else {
        return dispatch_real_gt(r, integer<2>{n});
    }
}

// signed c++ integral-real.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline bool dispatch_real_gt(const T &n, const real &r)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_si(r.get_mpfr_t(), static_cast<long>(n)) < 0;
        }
    } else {
        return dispatch_real_gt(integer<2>{n}, r);
    }
}

MPPP_DLL_PUBLIC bool dispatch_real_gt_rational_impl(const real &, const ::mpq_t);

// real-rational.
template <std::size_t SSize>
inline bool dispatch_real_gt(const real &r, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);

    return dispatch_real_gt_rational_impl(r, &qv);
}

MPPP_DLL_PUBLIC bool dispatch_real_gt_rational_impl(const ::mpq_t, const real &);

// rational-real.
template <std::size_t SSize>
inline bool dispatch_real_gt(const rational<SSize> &q, const real &r)
{
    const auto qv = detail::get_mpq_view(q);

    return dispatch_real_gt_rational_impl(&qv, r);
}

// real-C++ floating point.
MPPP_DLL_PUBLIC bool dispatch_real_gt(const real &, const float &);
MPPP_DLL_PUBLIC bool dispatch_real_gt(const real &, const double &);
MPPP_DLL_PUBLIC bool dispatch_real_gt(const real &, const long double &);

// C++ floating point-real.
MPPP_DLL_PUBLIC bool dispatch_real_gt(const float &, const real &);
MPPP_DLL_PUBLIC bool dispatch_real_gt(const double &, const real &);
MPPP_DLL_PUBLIC bool dispatch_real_gt(const long double &, const real &);

#if defined(MPPP_WITH_QUADMATH)

// real-real128.
MPPP_DLL_PUBLIC bool dispatch_real_gt(const real &, const real128 &);

// real128-real.
MPPP_DLL_PUBLIC bool dispatch_real_gt(const real128 &, const real &);

#endif

} // namespace detail

// Greater-than operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator>(const T &a, const U &b)
{
    return detail::dispatch_real_gt(a, b);
}

namespace detail
{

// real-real.
MPPP_DLL_PUBLIC bool dispatch_real_gte(const real &, const real &);

MPPP_DLL_PUBLIC bool dispatch_real_gte_integer_impl(const real &, const ::mpz_t);

// real-integer.
template <std::size_t SSize>
inline bool dispatch_real_gte(const real &r, const integer<SSize> &n)
{
    return dispatch_real_gte_integer_impl(r, n.get_mpz_view());
}

MPPP_DLL_PUBLIC bool dispatch_real_gte_integer_impl(const ::mpz_t, const real &);

// integer-real.
template <std::size_t SSize>
inline bool dispatch_real_gte(const integer<SSize> &n, const real &r)
{
    return dispatch_real_gte_integer_impl(n.get_mpz_view(), r);
}

// real-unsigned c++ integral.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline bool dispatch_real_gte(const real &r, const T &n)
{
    if (n <= nl_max<unsigned long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(n)) >= 0;
        }
    } else {
        return dispatch_real_gte(r, integer<2>{n});
    }
}

// unsigned c++ integral-real.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline bool dispatch_real_gte(const T &n, const real &r)
{
    if (n <= nl_max<unsigned long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(n)) <= 0;
        }
    } else {
        return dispatch_real_gte(integer<2>{n}, r);
    }
}

// NOTE: treat bool explicitly in order to avoid MSVC warnings.
MPPP_DLL_PUBLIC bool dispatch_real_gte(const real &, bool);
MPPP_DLL_PUBLIC bool dispatch_real_gte(bool, const real &);

// real-signed c++ integral.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline bool dispatch_real_gte(const real &r, const T &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_si(r.get_mpfr_t(), static_cast<long>(n)) >= 0;
        }
    } else {
        return dispatch_real_gte(r, integer<2>{n});
    }
}

// signed c++ integral-real.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline bool dispatch_real_gte(const T &n, const real &r)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_si(r.get_mpfr_t(), static_cast<long>(n)) <= 0;
        }
    } else {
        return dispatch_real_gte(integer<2>{n}, r);
    }
}

MPPP_DLL_PUBLIC bool dispatch_real_gte_rational_impl(const real &, const ::mpq_t);

// real-rational.
template <std::size_t SSize>
inline bool dispatch_real_gte(const real &r, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);

    return dispatch_real_gte_rational_impl(r, &qv);
}

MPPP_DLL_PUBLIC bool dispatch_real_gte_rational_impl(const ::mpq_t, const real &);

// rational-real.
template <std::size_t SSize>
inline bool dispatch_real_gte(const rational<SSize> &q, const real &r)
{
    const auto qv = detail::get_mpq_view(q);

    return dispatch_real_gte_rational_impl(&qv, r);
}

// real-C++ floating point.
MPPP_DLL_PUBLIC bool dispatch_real_gte(const real &, const float &);
MPPP_DLL_PUBLIC bool dispatch_real_gte(const real &, const double &);
MPPP_DLL_PUBLIC bool dispatch_real_gte(const real &, const long double &);

// C++ floating point-real.
MPPP_DLL_PUBLIC bool dispatch_real_gte(const float &, const real &);
MPPP_DLL_PUBLIC bool dispatch_real_gte(const double &, const real &);
MPPP_DLL_PUBLIC bool dispatch_real_gte(const long double &, const real &);

#if defined(MPPP_WITH_QUADMATH)

// real-real128.
MPPP_DLL_PUBLIC bool dispatch_real_gte(const real &, const real128 &);

// real128-real.
MPPP_DLL_PUBLIC bool dispatch_real_gte(const real128 &, const real &);

#endif

} // namespace detail

// Greater-than or equal operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator>=(const T &a, const U &b)
{
    return detail::dispatch_real_gte(a, b);
}

namespace detail
{

// real-real.
MPPP_DLL_PUBLIC bool dispatch_real_lt(const real &, const real &);

MPPP_DLL_PUBLIC bool dispatch_real_lt_integer_impl(const real &, const ::mpz_t);

// real-integer.
template <std::size_t SSize>
inline bool dispatch_real_lt(const real &r, const integer<SSize> &n)
{
    return dispatch_real_lt_integer_impl(r, n.get_mpz_view());
}

MPPP_DLL_PUBLIC bool dispatch_real_lt_integer_impl(const ::mpz_t, const real &);

// integer-real.
template <std::size_t SSize>
inline bool dispatch_real_lt(const integer<SSize> &n, const real &r)
{
    return dispatch_real_lt_integer_impl(n.get_mpz_view(), r);
}

// real-unsigned c++ integral.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline bool dispatch_real_lt(const real &r, const T &n)
{
    if (n <= nl_max<unsigned long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(n)) < 0;
        }
    } else {
        return dispatch_real_lt(r, integer<2>{n});
    }
}

// unsigned c++ integral-real.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline bool dispatch_real_lt(const T &n, const real &r)
{
    if (n <= nl_max<unsigned long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(n)) > 0;
        }
    } else {
        return dispatch_real_lt(integer<2>{n}, r);
    }
}

// NOTE: treat bool explicitly in order to avoid MSVC warnings.
MPPP_DLL_PUBLIC bool dispatch_real_lt(const real &, bool);
MPPP_DLL_PUBLIC bool dispatch_real_lt(bool, const real &);

// real-signed c++ integral.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline bool dispatch_real_lt(const real &r, const T &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_si(r.get_mpfr_t(), static_cast<long>(n)) < 0;
        }
    } else {
        return dispatch_real_lt(r, integer<2>{n});
    }
}

// signed c++ integral-real.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline bool dispatch_real_lt(const T &n, const real &r)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_si(r.get_mpfr_t(), static_cast<long>(n)) > 0;
        }
    } else {
        return dispatch_real_lt(integer<2>{n}, r);
    }
}

MPPP_DLL_PUBLIC bool dispatch_real_lt_rational_impl(const real &, const ::mpq_t);

// real-rational.
template <std::size_t SSize>
inline bool dispatch_real_lt(const real &r, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);

    return dispatch_real_lt_rational_impl(r, &qv);
}

MPPP_DLL_PUBLIC bool dispatch_real_lt_rational_impl(const ::mpq_t, const real &);

// rational-real.
template <std::size_t SSize>
inline bool dispatch_real_lt(const rational<SSize> &q, const real &r)
{
    const auto qv = detail::get_mpq_view(q);

    return dispatch_real_lt_rational_impl(&qv, r);
}

// real-C++ floating point.
MPPP_DLL_PUBLIC bool dispatch_real_lt(const real &, const float &);
MPPP_DLL_PUBLIC bool dispatch_real_lt(const real &, const double &);
MPPP_DLL_PUBLIC bool dispatch_real_lt(const real &, const long double &);

// C++ floating point-real.
MPPP_DLL_PUBLIC bool dispatch_real_lt(const float &, const real &);
MPPP_DLL_PUBLIC bool dispatch_real_lt(const double &, const real &);
MPPP_DLL_PUBLIC bool dispatch_real_lt(const long double &, const real &);

#if defined(MPPP_WITH_QUADMATH)

// real-real128.
MPPP_DLL_PUBLIC bool dispatch_real_lt(const real &, const real128 &);

// real128-real.
MPPP_DLL_PUBLIC bool dispatch_real_lt(const real128 &, const real &);

#endif

} // namespace detail

// Less-than operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator<(const T &a, const U &b)
{
    return detail::dispatch_real_lt(a, b);
}

namespace detail
{

// real-real.
MPPP_DLL_PUBLIC bool dispatch_real_lte(const real &, const real &);

MPPP_DLL_PUBLIC bool dispatch_real_lte_integer_impl(const real &, const ::mpz_t);

// real-integer.
template <std::size_t SSize>
inline bool dispatch_real_lte(const real &r, const integer<SSize> &n)
{
    return dispatch_real_lte_integer_impl(r, n.get_mpz_view());
}

MPPP_DLL_PUBLIC bool dispatch_real_lte_integer_impl(const ::mpz_t, const real &);

// integer-real.
template <std::size_t SSize>
inline bool dispatch_real_lte(const integer<SSize> &n, const real &r)
{
    return dispatch_real_lte_integer_impl(n.get_mpz_view(), r);
}

// real-unsigned c++ integral.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline bool dispatch_real_lte(const real &r, const T &n)
{
    if (n <= nl_max<unsigned long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(n)) <= 0;
        }
    } else {
        return dispatch_real_lte(r, integer<2>{n});
    }
}

// unsigned c++ integral-real.
template <typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline bool dispatch_real_lte(const T &n, const real &r)
{
    if (n <= nl_max<unsigned long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(n)) >= 0;
        }
    } else {
        return dispatch_real_lte(integer<2>{n}, r);
    }
}

// NOTE: treat bool explicitly in order to avoid MSVC warnings.
MPPP_DLL_PUBLIC bool dispatch_real_lte(const real &, bool);
MPPP_DLL_PUBLIC bool dispatch_real_lte(bool, const real &);

// real-signed c++ integral.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline bool dispatch_real_lte(const real &r, const T &n)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_si(r.get_mpfr_t(), static_cast<long>(n)) <= 0;
        }
    } else {
        return dispatch_real_lte(r, integer<2>{n});
    }
}

// signed c++ integral-real.
template <typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline bool dispatch_real_lte(const T &n, const real &r)
{
    if (n <= nl_max<long>() && n >= nl_min<long>()) {
        if (r.nan_p()) {
            return false;
        } else {
            return mpfr_cmp_si(r.get_mpfr_t(), static_cast<long>(n)) >= 0;
        }
    } else {
        return dispatch_real_lte(integer<2>{n}, r);
    }
}

MPPP_DLL_PUBLIC bool dispatch_real_lte_rational_impl(const real &, const ::mpq_t);

// real-rational.
template <std::size_t SSize>
inline bool dispatch_real_lte(const real &r, const rational<SSize> &q)
{
    const auto qv = detail::get_mpq_view(q);

    return dispatch_real_lte_rational_impl(r, &qv);
}

MPPP_DLL_PUBLIC bool dispatch_real_lte_rational_impl(const ::mpq_t, const real &);

// rational-real.
template <std::size_t SSize>
inline bool dispatch_real_lte(const rational<SSize> &q, const real &r)
{
    const auto qv = detail::get_mpq_view(q);

    return dispatch_real_lte_rational_impl(&qv, r);
}

// real-C++ floating point.
MPPP_DLL_PUBLIC bool dispatch_real_lte(const real &, const float &);
MPPP_DLL_PUBLIC bool dispatch_real_lte(const real &, const double &);
MPPP_DLL_PUBLIC bool dispatch_real_lte(const real &, const long double &);

// C++ floating point-real.
MPPP_DLL_PUBLIC bool dispatch_real_lte(const float &, const real &);
MPPP_DLL_PUBLIC bool dispatch_real_lte(const double &, const real &);
MPPP_DLL_PUBLIC bool dispatch_real_lte(const long double &, const real &);

#if defined(MPPP_WITH_QUADMATH)

// real-real128.
MPPP_DLL_PUBLIC bool dispatch_real_lte(const real &, const real128 &);

// real128-real.
MPPP_DLL_PUBLIC bool dispatch_real_lte(const real128 &, const real &);

#endif

} // namespace detail

// Less-than or equal operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator<=(const T &a, const U &b)
{
    return detail::dispatch_real_lte(a, b);
}

// Implementation of integer's assignment
// from real.
template <std::size_t SSize>
inline integer<SSize> &integer<SSize>::operator=(const real &x)
{
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
    return *this = static_cast<integer<SSize>>(x);
}

// Implementation of rational's assignment
// from real.
template <std::size_t SSize>
inline rational<SSize> &rational<SSize>::operator=(const real &x)
{
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
    return *this = static_cast<rational<SSize>>(x);
}

} // namespace mppp

#if defined(MPPP_WITH_BOOST_S11N)

// Never track the address of real objects
// during serialization.
BOOST_CLASS_TRACKING(mppp::real, boost::serialization::track_never)

#endif

#include <mp++/detail/real_literals.hpp>

// Support for pretty printing in xeus-cling.
#if defined(__CLING__)

#if __has_include(<nlohmann/json.hpp>)

#include <nlohmann/json.hpp>

namespace mppp
{

inline nlohmann::json mime_bundle_repr(const real &x)
{
    auto bundle = nlohmann::json::object();

    bundle["text/plain"] = x.to_string();

    return bundle;
}

} // namespace mppp

#endif

#endif

#else

#error The real.hpp header was included but mp++ was not configured with the MPPP_WITH_MPFR option.

#endif

#endif
