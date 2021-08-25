// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
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
#include <complex>
#include <cstddef>
#include <functional>
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
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>

#endif

#include <mp++/concepts.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/exceptions.hpp>
#include <mp++/fwd.hpp>
#include <mp++/integer.hpp>
#include <mp++/type_name.hpp>

#if defined(MPPP_WITH_MPFR)
#include <mp++/detail/mpfr.hpp>
#endif

namespace mppp
{

template <typename T, std::size_t SSize>
using is_rational_interoperable
    = detail::disjunction<is_integer_cpp_arithmetic<T>, std::is_same<T, integer<SSize>>, is_integer_cpp_complex<T>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, std::size_t SSize>
MPPP_CONCEPT_DECL rational_interoperable = is_rational_interoperable<T, SSize>::value;

#endif

template <typename T, std::size_t SSize>
using is_rational_integral_interoperable = detail::disjunction<is_cpp_integral<T>, std::is_same<T, integer<SSize>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, std::size_t SSize>
MPPP_CONCEPT_DECL rational_integral_interoperable = is_rational_integral_interoperable<T, SSize>::value;

#endif

template <typename T, std::size_t SSize>
using is_rational_cvr_interoperable = is_rational_interoperable<detail::uncvref_t<T>, SSize>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, std::size_t SSize>
MPPP_CONCEPT_DECL rational_cvr_interoperable = is_rational_cvr_interoperable<T, SSize>::value;

#endif

template <typename T, std::size_t SSize>
using is_rational_cvr_integral_interoperable = is_rational_integral_interoperable<detail::uncvref_t<T>, SSize>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, std::size_t SSize>
MPPP_CONCEPT_DECL rational_cvr_integral_interoperable = is_rational_cvr_integral_interoperable<T, SSize>::value;

#endif

namespace detail
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

} // namespace detail

// Multiprecision rational class.
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
#if defined(MPPP_WITH_BOOST_S11N)
    // Boost serialization support.
    friend class boost::serialization::access;

    template <typename Archive>
    void save(Archive &ar, unsigned) const
    {
        ar << get_num();
        ar << get_den();
    }

    template <typename Archive>
    void load(Archive &ar, unsigned)
    {
        // NOTE: use tmp variables
        // for exception safety.
        int_t num, den;

        ar >> num;
        ar >> den;

        *this = rational{std::move(num), std::move(den)};
    }
    void load(boost::archive::binary_iarchive &ar, unsigned)
    {
        // NOTE: for the binary archive, avoid calling the constructor,
        // but still do it in 2 stages for exception safety.
        int_t num, den;

        ar >> num;
        ar >> den;

        _get_num() = std::move(num);
        _get_den() = std::move(den);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif

public:
    // Underlying integral type.
    using int_t = integer<SSize>;
    // Alias for the template parameter SSize.
    static constexpr std::size_t ssize = SSize;
    // Default constructor.
    rational() : m_den(1u) {}
    // Defaulted copy constructor.
    rational(const rational &) = default;
    // Move constructor.
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
    // A tag for private constructors.
    struct ptag {
    };
    template <typename T,
              detail::enable_if_t<detail::disjunction<std::is_same<float, T>, std::is_same<double, T>>::value, int> = 0>
    explicit rational(const ptag &, const T &x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot construct a rational from the non-finite floating-point value "
                                    + detail::to_string(x));
        }
        MPPP_MAYBE_TLS detail::mpq_raii q;
        mpq_set_d(&q.m_mpq, static_cast<double>(x));
        m_num = mpq_numref(&q.m_mpq);
        m_den = mpq_denref(&q.m_mpq);
    }
#if defined(MPPP_WITH_MPFR)
    explicit rational(const ptag &, const long double &x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot construct a rational from the non-finite floating-point value "
                                    + detail::to_string(x));
        }
        // NOTE: static checks for overflows and for the precision value are done in mpfr.hpp.
        constexpr int d2 = std::numeric_limits<long double>::max_digits10 * 4;
        MPPP_MAYBE_TLS detail::mpfr_raii mpfr(static_cast<::mpfr_prec_t>(d2));
        MPPP_MAYBE_TLS detail::mpf_raii mpf(static_cast<::mp_bitcnt_t>(d2));
        MPPP_MAYBE_TLS detail::mpq_raii mpq;
        // NOTE: we go through an mpfr->mpf->mpq conversion chain as
        // mpfr_get_q() does not exist.
        // NOTE: probably coming in MPFR 4:
        // https://lists.gforge.inria.fr/pipermail/mpfr-commits/2017-June/011186.html
        ::mpfr_set_ld(&mpfr.m_mpfr, x, MPFR_RNDN);
        ::mpfr_get_f(&mpf.m_mpf, &mpfr.m_mpfr, MPFR_RNDN);
        mpq_set_f(&mpq.m_mpq, &mpf.m_mpf);
        m_num = mpq_numref(&mpq.m_mpq);
        m_den = mpq_denref(&mpq.m_mpq);
    }
#endif
    template <typename T, detail::enable_if_t<is_rational_cvr_integral_interoperable<T, SSize>::value, int> = 0>
    explicit rational(const ptag &, T &&n) : m_num(std::forward<T>(n)), m_den(1u)
    {
    }
    // Constructor from std::complex.
    template <typename T>
    explicit rational(const ptag &p, const std::complex<T> &c)
        : rational(p, c.imag() == 0
                          ? c.real()
                          : throw std::domain_error(
                              "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(c.imag())))
    {
    }

public:
    // Generic constructor.
#if defined(MPPP_HAVE_CONCEPTS)
    template <typename T>
    requires rational_cvr_interoperable<T, SSize> &&(!rational_integral_interoperable<T, SSize>)
#else
    template <
        typename T,
        detail::enable_if_t<detail::conjunction<is_rational_cvr_interoperable<T, SSize>,
                                                detail::negation<is_rational_integral_interoperable<T, SSize>>>::value,
                            int> = 0>
#endif
        // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
        explicit rational(T &&x)
        : rational(ptag{}, std::forward<T>(x))
    {
    }
#if defined(MPPP_HAVE_CONCEPTS)
    template <typename T>
    requires rational_cvr_interoperable<T, SSize> && rational_integral_interoperable<T, SSize>
#else
    template <typename T, detail::enable_if_t<detail::conjunction<is_rational_cvr_interoperable<T, SSize>,
                                                                  is_rational_integral_interoperable<T, SSize>>::value,
                                              int> = 0>
#endif
    // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    rational(T &&x) : rational(ptag{}, std::forward<T>(x))
    {
    }
    // Constructor from numerator and denominator.
#if defined(MPPP_HAVE_CONCEPTS)
    template <rational_cvr_integral_interoperable<SSize> T, rational_cvr_integral_interoperable<SSize> U>
#else
    template <typename T, typename U,
              detail::enable_if_t<detail::conjunction<is_rational_cvr_integral_interoperable<T, SSize>,
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
        // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
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
#if defined(MPPP_HAVE_STRING_VIEW)
    explicit rational(const ptag &, const std::string_view &s, int base) : rational(s.data(), s.data() + s.size(), base)
    {
    }
#endif

public:
    // Constructor from string.
#if defined(MPPP_HAVE_CONCEPTS)
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    explicit rational(const T &s, int base = 10) : rational(ptag{}, s, base)
    {
    }
    // Constructor from range of characters.
    explicit rational(const char *begin, const char *end, int base = 10) : m_den(1u)
    {
        // Copy the range into a local buffer.
        MPPP_MAYBE_TLS std::vector<char> buffer;
        buffer.assign(begin, end);
        buffer.emplace_back('\0');
        dispatch_c_string_ctor(buffer.data(), base);
    }
    // Constructor from mpz_t.
    explicit rational(const ::mpz_t n) : m_num(n), m_den(1u) {}
    // Constructor from mpq_t.
    explicit rational(const ::mpq_t q) : m_num(mpq_numref(q)), m_den(mpq_denref(q)) {}
#if !defined(_MSC_VER)
    // Move constructor from mpq_t.
    explicit rational(::mpq_t &&q) : m_num(::mpz_t{*mpq_numref(q)}), m_den(::mpz_t{*mpq_denref(q)}) {}
#endif

    ~rational() = default;

    // Copy-assignment operator.
    // NOTE: as long as copy assignment of integer cannot
    // throw, the default is good.
    rational &operator=(const rational &) = default;
    // Move assignment operator.
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
    // Generic assignment operator.
#if defined(MPPP_HAVE_CONCEPTS)
    template <rational_cvr_interoperable<SSize> T>
#else
    template <typename T, detail::enable_if_t<is_rational_cvr_interoperable<T, SSize>::value, int> = 0>
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
    rational &operator=(T &&x)
    {
        dispatch_assignment(std::forward<T>(x),
                            std::integral_constant<bool, is_rational_cvr_integral_interoperable<T, SSize>::value>{});
        return *this;
    }

    // Declaration of the assignments from
    // other mp++ classes.
#if defined(MPPP_WITH_QUADMATH)
    rational &operator=(const real128 &);
    rational &operator=(const complex128 &);
#endif
#if defined(MPPP_WITH_MPFR)
    rational &operator=(const real &);
#endif
#if defined(MPPP_WITH_MPC)
    rational &operator=(const complex &);
#endif

    // Assignment from string.
#if defined(MPPP_HAVE_CONCEPTS)
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    rational &operator=(const T &s)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
        return *this = rational{s};
    }
    // Assignment from mpz_t.
    rational &operator=(const ::mpz_t n)
    {
        m_num = n;
        m_den.set_one();
        return *this;
    }
    // Assignment from mpq_t.
    rational &operator=(const ::mpq_t q)
    {
        m_num = mpq_numref(q);
        m_den = mpq_denref(q);
        return *this;
    }
#if !defined(_MSC_VER)
    // Move assignment from mpq_t.
    rational &operator=(::mpq_t &&q)
    {
        m_num = ::mpz_t{*mpq_numref(q)};
        m_den = ::mpz_t{*mpq_denref(q)};
        return *this;
    }
#endif
    // Convert to string.
    MPPP_NODISCARD std::string to_string(int base = 10) const
    {
        if (m_den.is_one()) {
            return m_num.to_string(base);
        }
        return m_num.to_string(base) + "/" + m_den.to_string(base);
    }

private:
    // Conversion to int_t.
    template <typename T, detail::enable_if_t<std::is_same<int_t, T>::value, int> = 0>
    MPPP_NODISCARD std::pair<bool, T> dispatch_conversion() const
    {
        return std::make_pair(true, m_num / m_den);
    }
    // Conversion to bool.
    template <typename T, detail::enable_if_t<std::is_same<bool, T>::value, int> = 0>
    MPPP_NODISCARD std::pair<bool, T> dispatch_conversion() const
    {
        return std::make_pair(true, m_num.m_int.m_st._mp_size != 0);
    }
    // Conversion to integral types other than bool.
    template <typename T,
              detail::enable_if_t<
                  detail::conjunction<detail::is_integral<T>, detail::negation<std::is_same<bool, T>>>::value, int> = 0>
    MPPP_NODISCARD std::pair<bool, T> dispatch_conversion() const
    {
        return static_cast<int_t>(*this).template dispatch_conversion<T>();
    }
    // Conversion to float/double.
    template <typename T,
              detail::enable_if_t<detail::disjunction<std::is_same<T, float>, std::is_same<T, double>>::value, int> = 0>
    MPPP_NODISCARD std::pair<bool, T> dispatch_conversion() const
    {
        const auto v = detail::get_mpq_view(*this);
        return std::make_pair(true, static_cast<T>(mpq_get_d(&v)));
    }
#if defined(MPPP_WITH_MPFR)
    // Conversion to long double.
    template <typename T, detail::enable_if_t<std::is_same<T, long double>::value, int> = 0>
    MPPP_NODISCARD std::pair<bool, T> dispatch_conversion() const
    {
        constexpr int d2 = std::numeric_limits<long double>::max_digits10 * 4;
        MPPP_MAYBE_TLS detail::mpfr_raii mpfr(static_cast<::mpfr_prec_t>(d2));
        const auto v = detail::get_mpq_view(*this);
        ::mpfr_set_q(&mpfr.m_mpfr, &v, MPFR_RNDN);
        return std::make_pair(true, ::mpfr_get_ld(&mpfr.m_mpfr, MPFR_RNDN));
    }
#endif
    // Conversion to std::complex.
    template <typename T, detail::enable_if_t<is_cpp_complex<T>::value, int> = 0>
    MPPP_NODISCARD std::pair<bool, T> dispatch_conversion() const
    {
        return std::make_pair(true, T(static_cast<typename T::value_type>(*this)));
    }

public:
    // Generic conversion operator.
#if defined(MPPP_HAVE_CONCEPTS)
    template <rational_interoperable<SSize> T>
#else
    template <typename T, detail::enable_if_t<is_rational_interoperable<T, SSize>::value, int> = 0>
#endif
    explicit operator T() const
    {
        auto retval = dispatch_conversion<T>();
        if (mppp_unlikely(!retval.first)) {
            throw std::overflow_error("Conversion of the rational " + to_string() + " to the type '" + type_name<T>()
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
    // Generic conversion member function.
#if defined(MPPP_HAVE_CONCEPTS)
    template <rational_interoperable<SSize> T>
#else
    template <typename T, detail::enable_if_t<is_rational_interoperable<T, SSize>::value, int> = 0>
#endif
    bool get(T &rop) const
    {
        return dispatch_get(rop);
    }
    // Const numerator getter.
    MPPP_NODISCARD const int_t &get_num() const
    {
        return m_num;
    }
    // Const denominator getter.
    MPPP_NODISCARD const int_t &get_den() const
    {
        return m_den;
    }
    // Mutable numerator getter.
    int_t &_get_num()
    {
        return m_num;
    }
    // Mutable denominator getter.
    int_t &_get_den()
    {
        return m_den;
    }
    // Canonicalise.
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
        detail::fix_den_sign(*this);
        // NOTE: consider attempting demoting num/den. Let's KIS for now.
        return *this;
    }
    // Check canonical form.
    MPPP_NODISCARD bool is_canonical() const
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
    // Sign.
    MPPP_NODISCARD int sgn() const
    {
        return mppp::sgn(m_num);
    }
    // Negate in-place.
    rational &neg()
    {
        m_num.neg();
        return *this;
    }
    // In-place absolute value.
    rational &abs()
    {
        m_num.abs();
        return *this;
    }
    // In-place inversion.
    rational &inv()
    {
        if (mppp_unlikely(is_zero())) {
            throw zero_division_error("Cannot invert a zero rational");
        }
        swap(m_num, m_den);
        detail::fix_den_sign(*this);
        return *this;
    }
    // Test if the value is zero.
    MPPP_NODISCARD bool is_zero() const
    {
        return mppp::is_zero(m_num);
    }
    // Test if the value is one.
    MPPP_NODISCARD bool is_one() const
    {
        return mppp::is_one(m_num) && mppp::is_one(m_den);
    }
    // Test if the value is minus one.
    MPPP_NODISCARD bool is_negative_one() const
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

// Swap.
template <std::size_t SSize>
inline void swap(rational<SSize> &q1, rational<SSize> &q2) noexcept
{
    swap(q1._get_num(), q2._get_num());
    swap(q1._get_den(), q2._get_den());
}

namespace detail
{

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
struct rational_common_type<rational<SSize>, U, enable_if_t<is_cpp_integral<U>::value>> {
    using type = rational<SSize>;
};

template <std::size_t SSize, typename T>
struct rational_common_type<T, rational<SSize>, enable_if_t<is_cpp_integral<T>::value>> {
    using type = rational<SSize>;
};

template <std::size_t SSize, typename U>
struct rational_common_type<
    rational<SSize>, U, enable_if_t<disjunction<is_integer_cpp_floating_point<U>, is_integer_cpp_complex<U>>::value>> {
    using type = U;
};

template <std::size_t SSize, typename T>
struct rational_common_type<
    T, rational<SSize>, enable_if_t<disjunction<is_integer_cpp_floating_point<T>, is_integer_cpp_complex<T>>::value>> {
    using type = T;
};

template <typename T, typename U>
using rational_common_t = typename rational_common_type<T, U>::type;

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
} // namespace detail

// Implementation of the rational op types concept, used in various operators.
template <typename T, typename U>
using are_rational_op_types = detail::is_detected<detail::rational_common_t, T, U>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL rational_op_types = are_rational_op_types<T, U>::value;

#endif

template <typename T, typename U>
using are_rational_real_op_types
    = detail::conjunction<are_rational_op_types<T, U>, detail::negation<is_integer_cpp_complex<T>>,
                          detail::negation<is_integer_cpp_complex<U>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL rational_real_op_types = are_rational_real_op_types<T, U>::value;

#endif

// Generic conversion function.
#if defined(MPPP_HAVE_CONCEPTS)
template <std::size_t SSize, rational_interoperable<SSize> T>
#else
template <typename T, std::size_t SSize, detail::enable_if_t<is_rational_interoperable<T, SSize>::value, int> = 0>
#endif
inline bool get(T &rop, const rational<SSize> &q)
{
    return q.get(rop);
}

// Ternary addition.
template <std::size_t SSize>
inline rational<SSize> &add(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    detail::addsub_impl<true, false>(rop, op1, op2);
    return rop;
}

// Ternary subtraction.
template <std::size_t SSize>
inline rational<SSize> &sub(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    detail::addsub_impl<false, false>(rop, op1, op2);
    return rop;
}

namespace detail
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
} // namespace detail

// Ternary multiplication.
template <std::size_t SSize>
inline rational<SSize> &mul(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    detail::mul_impl<false>(rop, op1, op2);
    return rop;
}

// Ternary division.
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
        swap(rop._get_num(), rop._get_den());
        // Fix den sign.
        detail::fix_den_sign(rop);
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
    detail::fix_den_sign(rop);
    return rop;
}

// Binary negation.
template <std::size_t SSize>
inline rational<SSize> &neg(rational<SSize> &rop, const rational<SSize> &q)
{
    rop = q;
    return rop.neg();
}

// Unary negation.
template <std::size_t SSize>
inline rational<SSize> neg(const rational<SSize> &q)
{
    rational<SSize> ret(q);
    ret.neg();
    return ret;
}

// Binary absolute value.
template <std::size_t SSize>
inline rational<SSize> &abs(rational<SSize> &rop, const rational<SSize> &q)
{
    rop = q;
    return rop.abs();
}

// Unary absolute value.
template <std::size_t SSize>
inline rational<SSize> abs(const rational<SSize> &q)
{
    rational<SSize> ret(q);
    ret.abs();
    return ret;
}

// Binary inversion.
template <std::size_t SSize>
inline rational<SSize> &inv(rational<SSize> &rop, const rational<SSize> &q)
{
    rop = q;
    return rop.inv();
}

// Unary inversion.
template <std::size_t SSize>
inline rational<SSize> inv(const rational<SSize> &q)
{
    rational<SSize> ret(q);
    ret.inv();
    return ret;
}

namespace detail
{

MPPP_DLL_PUBLIC std::ostream &rational_stream_operator_impl(std::ostream &, const mpz_struct_t *, const mpz_struct_t *,
                                                            int, bool);

} // namespace detail

// Output stream operator.
template <std::size_t SSize>
inline std::ostream &operator<<(std::ostream &os, const rational<SSize> &q)
{
    return detail::rational_stream_operator_impl(os, q.get_num().get_mpz_view(), q.get_den().get_mpz_view(), q.sgn(),
                                                 q.get_den().is_one());
}

// Identity operator.
template <std::size_t SSize>
inline rational<SSize> operator+(const rational<SSize> &q)
{
    return q;
}

namespace detail
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

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_add(const rational<SSize> &op1, T n)
{
    return dispatch_binary_add(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_add(T n, const rational<SSize> &op2)
{
    return dispatch_binary_add(op2, n);
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_add(const rational<SSize> &op1, const T &x)
{
    return static_cast<T>(op1) + x;
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_add(const T &x, const rational<SSize> &op2)
{
    return dispatch_binary_add(op2, x);
}

} // namespace detail

// Binary addition operator.
template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
requires rational_op_types<T, U>
inline auto
#else
inline detail::rational_common_t<T, U>
#endif
operator+(const T &op1, const U &op2)
{
    return detail::dispatch_binary_add(op1, op2);
}

namespace detail
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

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_add(rational<SSize> &retval, const T &n)
{
    dispatch_in_place_add(retval, integer<SSize>{n});
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_add(rational<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) + x;
}

template <typename T, std::size_t SSize, enable_if_t<is_rational_interoperable<T, SSize>::value, int> = 0>
inline void dispatch_in_place_add(T &rop, const rational<SSize> &op)
{
    rop = static_cast<T>(rop + op);
}

} // namespace detail

// In-place addition operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires rational_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_rational_op_types<T, U>::value, int> = 0>
#endif
inline T &operator+=(T &rop, const U &op)
{
    detail::dispatch_in_place_add(rop, op);
    return rop;
}

// Prefix increment.
template <std::size_t SSize>
inline rational<SSize> &operator++(rational<SSize> &q)
{
    add(q._get_num(), q._get_num(), q.get_den());
    return q;
}

// Suffix increment.
template <std::size_t SSize>
inline rational<SSize> operator++(rational<SSize> &q, int)
{
    auto retval(q);
    ++q;
    return retval;
}

// Negated copy.
template <std::size_t SSize>
inline rational<SSize> operator-(const rational<SSize> &q)
{
    auto retval(q);
    retval.neg();
    return retval;
}

namespace detail
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

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_sub(const rational<SSize> &op1, T n)
{
    return dispatch_binary_sub(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_sub(T n, const rational<SSize> &op2)
{
    auto retval = dispatch_binary_sub(op2, n);
    retval.neg();
    return retval;
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_sub(const rational<SSize> &op1, const T &x)
{
    return static_cast<T>(op1) - x;
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_sub(const T &x, const rational<SSize> &op2)
{
    return -dispatch_binary_sub(op2, x);
}

} // namespace detail

// Binary subtraction operator.
template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
requires rational_op_types<T, U>
inline auto
#else
inline detail::rational_common_t<T, U>
#endif
operator-(const T &op1, const U &op2)
{
    return detail::dispatch_binary_sub(op1, op2);
}

namespace detail
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

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_sub(rational<SSize> &retval, const T &n)
{
    dispatch_in_place_sub(retval, integer<SSize>{n});
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_sub(rational<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) - x;
}

template <typename T, std::size_t SSize, enable_if_t<is_rational_interoperable<T, SSize>::value, int> = 0>
inline void dispatch_in_place_sub(T &rop, const rational<SSize> &op)
{
    rop = static_cast<T>(rop - op);
}

} // namespace detail

// In-place subtraction operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires rational_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_rational_op_types<T, U>::value, int> = 0>
#endif
inline T &operator-=(T &rop, const U &op)
{
    detail::dispatch_in_place_sub(rop, op);
    return rop;
}

// Prefix decrement.
template <std::size_t SSize>
inline rational<SSize> &operator--(rational<SSize> &q)
{
    sub(q._get_num(), q._get_num(), q.get_den());
    return q;
}

// Suffix decrement.
template <std::size_t SSize>
inline rational<SSize> operator--(rational<SSize> &q, int)
{
    auto retval(q);
    --q;
    return retval;
}

namespace detail
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

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_mul(const rational<SSize> &op1, T n)
{
    return dispatch_binary_mul(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_mul(T n, const rational<SSize> &op2)
{
    return dispatch_binary_mul(op2, n);
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_mul(const rational<SSize> &op1, const T &x)
{
    return static_cast<T>(op1) * x;
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_mul(const T &x, const rational<SSize> &op2)
{
    return dispatch_binary_mul(op2, x);
}

} // namespace detail

// Binary multiplication operator.
template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
requires rational_op_types<T, U>
inline auto
#else
inline detail::rational_common_t<T, U>
#endif
operator*(const T &op1, const U &op2)
{
    return detail::dispatch_binary_mul(op1, op2);
}

namespace detail
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

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_mul(rational<SSize> &retval, const T &n)
{
    dispatch_in_place_mul(retval, integer<SSize>{n});
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_mul(rational<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) * x;
}

template <typename T, std::size_t SSize, enable_if_t<is_rational_interoperable<T, SSize>::value, int> = 0>
inline void dispatch_in_place_mul(T &rop, const rational<SSize> &op)
{
    rop = static_cast<T>(rop * op);
}

} // namespace detail

// In-place multiplication operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires rational_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_rational_op_types<T, U>::value, int> = 0>
#endif
inline T &operator*=(T &rop, const U &op)
{
    detail::dispatch_in_place_mul(rop, op);
    return rop;
}

namespace detail
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

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_div(const rational<SSize> &op1, T n)
{
    return dispatch_binary_div(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_div(T n, const rational<SSize> &op2)
{
    return dispatch_binary_div(integer<SSize>{n}, op2);
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_div(const rational<SSize> &op1, const T &x)
{
    return static_cast<T>(op1) / x;
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_div(const T &x, const rational<SSize> &op2)
{
    return x / static_cast<T>(op2);
}

} // namespace detail

// Binary division operator.
template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
requires rational_op_types<T, U>
inline auto
#else
inline detail::rational_common_t<T, U>
#endif
operator/(const T &op1, const U &op2)
{
    return detail::dispatch_binary_div(op1, op2);
}

namespace detail
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

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_div(rational<SSize> &retval, const T &n)
{
    dispatch_in_place_div(retval, integer<SSize>{n});
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_div(rational<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) / x;
}

template <typename T, std::size_t SSize, enable_if_t<is_rational_interoperable<T, SSize>::value, int> = 0>
inline void dispatch_in_place_div(T &rop, const rational<SSize> &op)
{
    rop = static_cast<T>(rop / op);
}

} // namespace detail

// In-place division operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires rational_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_rational_op_types<T, U>::value, int> = 0>
#endif
inline T &operator/=(T &rop, const U &op)
{
    detail::dispatch_in_place_div(rop, op);
    return rop;
}

namespace detail
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

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline bool dispatch_equality(const rational<SSize> &op1, const T &op2)
{
    return static_cast<T>(op1) == op2;
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline bool dispatch_equality(const T &op1, const rational<SSize> &op2)
{
    return dispatch_equality(op2, op1);
}

} // namespace detail

// Equality operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires rational_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_rational_op_types<T, U>::value, int> = 0>
#endif
inline bool operator==(const T &op1, const U &op2)
{
    return detail::dispatch_equality(op1, op2);
}

// Inequality operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires rational_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_rational_op_types<T, U>::value, int> = 0>
#endif
inline bool operator!=(const T &op1, const U &op2)
{
    return !(op1 == op2);
}

namespace detail
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

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline bool dispatch_less_than(const rational<SSize> &a, T n)
{
    return dispatch_less_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
bool dispatch_less_than(T, const rational<SSize> &);

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point<T>::value, int> = 0>
inline bool dispatch_less_than(const rational<SSize> &a, T x)
{
    return static_cast<T>(a) < x;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point<T>::value, int> = 0>
inline bool dispatch_less_than(T, const rational<SSize> &);

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

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline bool dispatch_greater_than(const rational<SSize> &a, T n)
{
    return dispatch_greater_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline bool dispatch_greater_than(T n, const rational<SSize> &a)
{
    return dispatch_less_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point<T>::value, int> = 0>
inline bool dispatch_greater_than(const rational<SSize> &a, T x)
{
    return static_cast<T>(a) > x;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point<T>::value, int> = 0>
inline bool dispatch_greater_than(T x, const rational<SSize> &a)
{
    return dispatch_less_than(a, x);
}

// Implement them here, as we need visibility of dispatch_greater_than().
template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int>>
inline bool dispatch_less_than(T n, const rational<SSize> &a)
{
    return dispatch_greater_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point<T>::value, int>>
inline bool dispatch_less_than(T x, const rational<SSize> &a)
{
    return dispatch_greater_than(a, x);
}
} // namespace detail

// Less-than operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires rational_real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_rational_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator<(const T &op1, const U &op2)
{
    return detail::dispatch_less_than(op1, op2);
}

// Less-than or equal operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires rational_real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_rational_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator<=(const T &op1, const U &op2)
{
    return !(op1 > op2);
}

// Greater-than operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires rational_real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_rational_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator>(const T &op1, const U &op2)
{
    return detail::dispatch_greater_than(op1, op2);
}

// Greater-than or equal operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires rational_real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_rational_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator>=(const T &op1, const U &op2)
{
    return !(op1 < op2);
}

// Comparison function for rationals.
template <std::size_t SSize>
inline int cmp(const rational<SSize> &op1, const rational<SSize> &op2)
{
    // NOTE: here we have potential for 2 views referring to the same underlying
    // object. The same potential issues as described in the mpz_view class may arise.
    // Keep an eye on it.
    // NOTE: this can probably be improved by implementing the same strategy as in mpq_cmp()
    // but based on our primitives:
    // - if op1 and op2 are integers, compare the nums,
    // - try to see if the limb/bit sizes of nums and dens can tell use immediately which
    //   number is larger,
    // - otherwise, do the two multiplications and compare.
    const auto v1 = detail::get_mpq_view(op1);
    const auto v2 = detail::get_mpq_view(op2);
    return mpq_cmp(&v1, &v2);
}

// Comparison function for rational/integer arguments.
template <std::size_t SSize>
inline int cmp(const rational<SSize> &op1, const integer<SSize> &op2)
{
    // NOTE: mpq_cmp_z() is a macro or a function, depending on the GMP version. Don't
    // call it with "::".
    const auto v1 = detail::get_mpq_view(op1);
    return mpq_cmp_z(&v1, op2.get_mpz_view());
}

// Comparison function for integer/rational arguments.
template <std::size_t SSize>
inline int cmp(const integer<SSize> &op1, const rational<SSize> &op2)
{
    // NOTE: this will clamp the return value in the [-1,1] range,
    // so not exactly identical to the GMP return value. It's still consistent
    // with the rest of GMP/mp++ rational comparison functions.
    // NOTE: we don't take directly the negative because it's not clear
    // how large the returned value could be. Like this, we prevent any
    // possible integral overflow shenanigans.
    return -detail::integral_sign(cmp(op2, op1));
}

// Sign function.
template <std::size_t SSize>
int sgn(const rational<SSize> &q)
{
    return q.sgn();
}

// Test if a rational is one.
template <std::size_t SSize>
inline bool is_one(const rational<SSize> &q)
{
    return q.is_one();
}

// Test if a rational is minus one.
template <std::size_t SSize>
inline bool is_negative_one(const rational<SSize> &q)
{
    return q.is_negative_one();
}

// Test if a rational is zero.
template <std::size_t SSize>
inline bool is_zero(const rational<SSize> &q)
{
    return q.is_zero();
}

namespace detail
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

} // namespace detail

// Binomial coefficient.
#if defined(MPPP_HAVE_CONCEPTS)
template <std::size_t SSize, rational_integral_interoperable<SSize> T>
#else
template <std::size_t SSize, typename T,
          detail::enable_if_t<is_rational_integral_interoperable<T, SSize>::value, int> = 0>
#endif
inline rational<SSize> binomial(const rational<SSize> &x, const T &y)
{
    return detail::rational_binomial_impl(x, y);
}

namespace detail
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

// Rational base, fp/complex exponent.
template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T pow_impl(const rational<SSize> &base, const T &exp)
{
    return std::pow(static_cast<T>(base), exp);
}

// Fp/complex base, rational exponent.
template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T pow_impl(const T &base, const rational<SSize> &exp)
{
    return std::pow(base, static_cast<T>(exp));
}

} // namespace detail

// Binary exponentiation.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires rational_op_types<T, U>
inline auto
#else
template <typename T, typename U>
inline detail::rational_common_t<T, U>
#endif
pow(const T &base, const U &exp)
{
    return detail::pow_impl(base, exp);
}

// Canonicalise.
template <std::size_t SSize>
inline rational<SSize> &canonicalise(rational<SSize> &rop)
{
    return rop.canonicalise();
}

// Hash value.
template <std::size_t SSize>
inline std::size_t hash(const rational<SSize> &q)
{
    // NOTE: just return the sum of the hashes. We are already doing
    // some hashing in the integers, hopefully this is enough to obtain
    // decent hashing on the rational as well.
    return hash(q.get_num()) + hash(q.get_den());
}

// Implementation of integer's assignment
// from rational.
template <std::size_t SSize>
inline integer<SSize> &integer<SSize>::operator=(const rational<SSize> &q)
{
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
    return *this = static_cast<integer<SSize>>(q);
}

} // namespace mppp

#if defined(MPPP_WITH_BOOST_S11N)

// Disable tracking for mppp::rational.
// NOTE: this code has been lifted from the Boost
// macro, which does not support directly
// class templates.

namespace boost
{

namespace serialization
{

template <std::size_t SSize>
struct tracking_level<mppp::rational<SSize>> {
    typedef mpl::integral_c_tag tag;
    typedef mpl::int_<track_never> type;
    BOOST_STATIC_CONSTANT(int, value = tracking_level::type::value);
    BOOST_STATIC_ASSERT((mpl::greater<implementation_level<mppp::rational<SSize>>, mpl::int_<primitive_type>>::value));
};

} // namespace serialization

} // namespace boost

#endif

namespace std
{

// Specialisation of std::hash for mppp::rational.
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
} // namespace std

#include <mp++/detail/rational_literals.hpp>

// Support for pretty printing in xeus-cling.
#if defined(__CLING__)

#if __has_include(<nlohmann/json.hpp>)

#include <nlohmann/json.hpp>

namespace mppp
{

template <std::size_t SSize>
inline nlohmann::json mime_bundle_repr(const rational<SSize> &q)
{
    auto bundle = nlohmann::json::object();

    bundle["text/plain"] = q.to_string();
    if (q.get_den().is_one()) {
        bundle["text/latex"] = "$" + q.get_num().to_string() + "$";
    } else {
        bundle["text/latex"] = "$\\frac{" + q.get_num().to_string() + "}{" + q.get_den().to_string() + "}$";
    }

    return bundle;
}

} // namespace mppp

#endif

#endif

#endif
