// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
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

#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <mp++/concepts.hpp>
#include <mp++/detail/fwd_decl.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

namespace mppp
{

inline namespace detail
{

// constexpr max/min implementations.
template <typename T>
constexpr T c_max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
constexpr T c_min(T a, T b)
{
    return a < b ? a : b;
}

// Compute a power of 2 value of the signed integer type T that can be safely negated.
template <typename T>
constexpr T safe_abs(T n = T(1))
{
    static_assert(std::is_integral<T>::value && std::is_signed<T>::value, "Invalid type.");
    return (n <= std::numeric_limits<T>::max() / T(2) && n >= std::numeric_limits<T>::min() / T(2))
               ? safe_abs(T(n * T(2)))
               : n;
}

// Min prec that will be allowed for reals. It should be MPFR_PREC_MIN, as that value is currently 2.
// We just put an extra constraint for paranoia, so that we ensure 100% that the min prec is not zero
// (we rely on that for distinguishing static vs dynamic storage).
constexpr ::mpfr_prec_t real_prec_min()
{
    return c_max(::mpfr_prec_t(1), ::mpfr_prec_t(MPFR_PREC_MIN));
}

// For the max precision, we first remove 7 bits from the MPFR_PREC_MAX value (as the MPFR docs warn
// to never set the precision "close" to the max value). Then we make sure that we can negate safely.
constexpr ::mpfr_prec_t real_prec_max()
{
    return c_min(::mpfr_prec_t(MPFR_PREC_MAX / 128), safe_abs<::mpfr_prec_t>());
}

// Sanity check.
static_assert(real_prec_min() <= real_prec_max(),
              "The minimum precision for real is larger than the maximum precision.");

// TODO mpfr struct checks (see integer)

// Helper function to print an mpfr to stream in base 10.
inline void mpfr_to_stream(const ::mpfr_t r, std::ostream &os)
{
    // Special values first.
    if (mpfr_nan_p(r)) {
        os << "nan";
        return;
    }
    if (mpfr_inf_p(r)) {
        if (mpfr_sgn(r) < 0) {
            os << "-";
        }
        os << "inf";
        return;
    }

    // Get the string fractional representation via the MPFR function,
    // and wrap it into a smart pointer.
    ::mpfr_exp_t exp(0);
    smart_mpfr_str str(::mpfr_get_str(nullptr, &exp, 10, 0, r, MPFR_RNDN), ::mpfr_free_str);
    if (mppp_unlikely(!str)) {
        throw std::runtime_error("Error in the conversion of a real to string: the call to mpfr_get_str() failed");
    }

    // Print the string, inserting a decimal point after the first digit.
    bool dot_added = false;
    for (auto cptr = str.get(); *cptr != '\0'; ++cptr) {
        os << (*cptr);
        // NOTE: check this answer:
        // http://stackoverflow.com/questions/13827180/char-ascii-relation
        // """
        // The mapping of integer values for characters does have one guarantee given
        // by the Standard: the values of the decimal digits are continguous.
        // (i.e., '1' - '0' == 1, ... '9' - '0' == 9)
        // """
        if (!dot_added && *cptr >= '0' && *cptr <= '9') {
            os << '.';
            dot_added = true;
        }
    }
    assert(dot_added);

    // Adjust the exponent. Do it in multiprec in order to avoid potential overflow.
    integer<1> z_exp{exp};
    --z_exp;
    if (z_exp.sgn() && !mpfr_zero_p(r)) {
        // Add the exponent at the end of the string, if both the value and the exponent
        // are nonzero.
        os << "e" << z_exp;
    }
}

template <std::size_t SSize>
struct static_real {
    // Let's put a hard cap and sanity check on the static size.
    static_assert(SSize > 0u && SSize <= 64u, "Invalid static size for real.");
    // Zero-fill the limbs array on init. The reason we do this
    // is that we are not sure the mpfr api will init the whole array,
    // so we could be left with uninited values in the array, thus
    // ending up reading uninited values during copy/move. It's not
    // clear 100% if this is UB, but better safe than sorry.
    // Also, the precision will be the minimum value.
    static_real() : _mpfr_prec(-real_prec_min()), m_limbs()
    {
        // A temporary mpfr struct for use with the mpfr custom interface.
        mpfr_struct_t tmp;
        // Init the limbs first, as indicated by the mpfr docs. But first assert the static
        // storage is enough to store a real with minimum precision.
        static_assert(mpfr_custom_get_size(real_prec_min()) <= SSize * sizeof(::mp_limb_t),
                      "Not enough storage in static_real to represent a real with minimum precision.");
        mpfr_custom_init(m_limbs.data(), real_prec_min());
        // Do the custom init with a zero value, exponent 0 (unused), minimum precision (must match
        // the previous mpfr_custom_init() call), and the limbs array pointer.
        mpfr_custom_init_set(&tmp, MPFR_ZERO_KIND, 0, real_prec_min(), m_limbs.data());

        // Copy over from tmp. Precision has already been set.
        assert(tmp._mpfr_prec == real_prec_min());
        _mpfr_sign = tmp._mpfr_sign;
        _mpfr_exp = tmp._mpfr_exp;
    }
    mpfr_struct_t get_mpfr()
    {
        // NOTE: here we will assume the precision is set to a value which can
        // always be negated safely.
        return mpfr_struct_t{-_mpfr_prec, _mpfr_sign, _mpfr_exp, m_limbs.data()};
    }
    mpfr_struct_t get_mpfr_c() const
    {
        // NOTE: as usual, we are using const_cast here to cast away the constness of the pointer.
        // We need to make extra-sure the returned value is used only where const mpfr_t is expected.
        return mpfr_struct_t{-_mpfr_prec, _mpfr_sign, _mpfr_exp, const_cast<::mp_limb_t *>(m_limbs.data())};
    }
    // Helper to set this to the mpfr m. It will copy the negated precision,
    // the sign and the exponent, but *not* the limbs.
    void set_mpfr_nl(const mpfr_struct_t &m)
    {
        _mpfr_prec = -m._mpfr_prec;
        _mpfr_sign = m._mpfr_sign;
        _mpfr_exp = m._mpfr_exp;
    }
    // Maximum precision possible for a static real.
    static constexpr ::mpfr_prec_t max_prec_impl(::mpfr_prec_t n)
    {
        return n == std::numeric_limits<::mpfr_prec_t>::max()
                   // Overflow check first.
                   ? (throw std::overflow_error(
                         "Overflow in the determination of the maximum precision for a static real."))
                   // NOTE: here and elsewhere we assume overflows in SSize * sizeof(::mp_limb_t) will
                   // be caught when m_limbs is instantiated.
                   : (mpfr_custom_get_size(n) <= SSize * sizeof(::mp_limb_t)
                          // For the current precision n, we have enough storage. Try 1 bit more of precision.
                          ? max_prec_impl(static_cast<::mpfr_prec_t>(n + 1))
                          // We don't have enough storage for the current precision. Removing one bit should
                          // give us a suitable precision.
                          // NOTE: the min n possible here is real_prec_min(), and that is > 0. Hence, n - 1 is a safe
                          // computation.
                          : (mpfr_custom_get_size(static_cast<::mpfr_prec_t>(n - 1)) <= SSize * sizeof(::mp_limb_t)
                                 ? static_cast<::mpfr_prec_t>(n - 1)
                                 // This should never happen, as it would mean that 1 extra bit of precision
                                 // requires 2 bytes of storage. Let's leave it for sanity check.
                                 : (throw std::logic_error(
                                       "Could not determine the maximum precision for a static real."))));
    }
    static constexpr ::mpfr_prec_t max_prec()
    {
        // NOTE: the impl() function assumes there's enough storage for the minimum precision.
        static_assert(mpfr_custom_get_size(real_prec_min()) <= SSize * sizeof(::mp_limb_t),
                      "Not enough storage in static_real to represent a real with minimum precision.");
        // Make sure the output is never greater than real_prec_max().
        return c_min(real_prec_max(), max_prec_impl(real_prec_min()));
    }
    ::mpfr_prec_t _mpfr_prec;
    ::mpfr_sign_t _mpfr_sign;
    ::mpfr_exp_t _mpfr_exp;
    std::array<::mp_limb_t, SSize> m_limbs;
};

template <std::size_t SSize>
union real_union {
    using s_storage = static_real<SSize>;
    using d_storage = mpfr_struct_t;
    // Clear the dynamic mpfr and destroy the dynamic member.
    void destroy_dynamic()
    {
        ::mpfr_clear(&g_dy());
        g_dy().~d_storage();
    }
    real_union() : m_st()
    {
    }
    real_union(const real_union &r)
    {
        if (r.is_static()) {
            // Activate and init the static member with a copy.
            ::new (static_cast<void *>(&m_st)) s_storage(r.g_st());
        } else {
            // Activate dynamic storage.
            ::new (static_cast<void *>(&m_dy)) d_storage;
            // Init with same precision as r, then set.
            ::mpfr_init2(&m_dy, mpfr_get_prec(&r.g_dy()));
            ::mpfr_set(&m_dy, &r.g_dy(), MPFR_RNDN);
        }
    }
    real_union(real_union &&r) noexcept
    {
        if (r.is_static()) {
            // Activate and init the static member with a copy.
            ::new (static_cast<void *>(&m_st)) s_storage(r.g_st());
        } else {
            // Activate dynamic storage, and shallow copy r.
            ::new (static_cast<void *>(&m_dy)) d_storage(r.g_dy());

            // Deactivate the static storage in r, and default-init to static.
            r.g_dy().~d_storage();
            ::new (static_cast<void *>(&r.m_st)) s_storage();
        }
    }
    real_union &operator=(const real_union &other)
    {
        const bool s1 = is_static(), s2 = other.is_static();
        if (s1 && s2) {
            // Self assignment of the array in static_real *should* be fine.
            g_st() = other.g_st();
        } else if (s1 && !s2) {
            // Destroy static.
            g_st().~s_storage();

            // Construct the dynamic struct.
            ::new (static_cast<void *>(&m_dy)) d_storage;
            // Init with same precision as other, then set.
            ::mpfr_init2(&m_dy, mpfr_get_prec(&other.g_dy()));
            ::mpfr_set(&m_dy, &other.g_dy(), MPFR_RNDN);
        } else if (!s1 && s2) {
            // Destroy the dynamic member.
            destroy_dynamic();

            // Init-copy the static from other.
            ::new (static_cast<void *>(&m_st)) s_storage(other.g_st());
        } else {
            // Set the precision of this to the precision of other.
            // NOTE: mpfr_set_prec() also sets to nan.
            ::mpfr_set_prec(&g_dy(), mpfr_get_prec(&other.g_dy()));
            // Set the value.
            ::mpfr_set(&g_dy(), &other.g_dy(), MPFR_RNDN);
        }
        return *this;
    }
    real_union &operator=(real_union &&other) noexcept
    {
        const bool s1 = is_static(), s2 = other.is_static();
        if (s1 && s2) {
            // Same as copy here.
            g_st() = other.g_st();
        } else if (s1 && !s2) {
            // Destroy static.
            g_st().~s_storage();

            // Construct the dynamic struct, shallow-copying from other.
            ::new (static_cast<void *>(&m_dy)) d_storage(other.g_dy());
            // Downgrade the other to an empty static.
            other.g_dy().~d_storage();
            ::new (static_cast<void *>(&other.m_st)) s_storage();
        } else if (!s1 && s2) {
            // Same as copy assignment: destroy and copy-construct.
            destroy_dynamic();
            ::new (static_cast<void *>(&m_st)) s_storage(other.g_st());
        } else {
            // Swap with other. Self-assignment is fine, mpfr_swap() can have
            // aliasing arguments.
            ::mpfr_swap(&g_dy(), &other.g_dy());
        }
        return *this;
    }
    ~real_union()
    {
        assert(m_st._mpfr_prec);
        if (is_static()) {
            assert(g_st()._mpfr_prec < 0);
            g_st().~s_storage();
        } else {
            assert(g_dy()._mpfr_prec > 0);
            destroy_dynamic();
        }
    }
    // Check storage type.
    bool is_static() const
    {
        return m_st._mpfr_prec < 0;
    }
    bool is_dynamic() const
    {
        return m_st._mpfr_prec > 0;
    }
    // Getters for st and dy.
    const s_storage &g_st() const
    {
        assert(is_static());
        return m_st;
    }
    s_storage &g_st()
    {
        assert(is_static());
        return m_st;
    }
    const d_storage &g_dy() const
    {
        assert(is_dynamic());
        return m_dy;
    }
    d_storage &g_dy()
    {
        assert(is_dynamic());
        return m_dy;
    }
    // Data members.
    s_storage m_st;
    d_storage m_dy;
};
}

template <typename T>
using is_real_interoperable = disjunction<is_cpp_interoperable<T>, is_integer<T>, is_rational<T>>;

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RealInteroperable = is_real_interoperable<T>::value;
#else
using real_interoperable_enabler = enable_if_t<is_real_interoperable<T>::value, int>;
#endif

template <std::size_t SSize>
class real
{
    using s_storage = typename real_union<SSize>::s_storage;
    using d_storage = typename real_union<SSize>::d_storage;

public:
    real() = default;
    real(const real &) = default;
    real(real &&) = default;

private:
    template <std::size_t S>
    void dispatch_generic_ctor(const integer<S> &n, ::mpfr_prec_t prec)
    {
        if (!prec) {
            // If the precision is not given explicitly, we will infer it.
            // Compute the number of bits used by n.
            const auto ls = n.size();
            // Check that ls * GMP_NUMB_BITS <= max_prec.
            if (mppp_unlikely(ls > static_cast<std::make_unsigned<::mpfr_prec_t>::type>(real_prec_max())
                                       / unsigned(GMP_NUMB_BITS))) {
                throw std::invalid_argument(
                    "The deduced precision for a real constructed from an integer is too large");
            }
            // Compute the precision. We already know it's a non-negative value not greater
            // than the max allowed precision. We just need to make sure it's not smaller than the
            // min allowed precision.
            prec = c_max(static_cast<::mpfr_prec_t>(static_cast<::mpfr_prec_t>(ls) * int(GMP_NUMB_BITS)),
                         real_prec_min());
        }
        // Set the precision (manually specified or deduced).
        set_prec(prec);
        if (is_static()) {
            auto tmp = m_real.g_st().get_mpfr();
            ::mpfr_set_z(&tmp, n.get_mpz_view(), MPFR_RNDN);
            // Copy over.
            m_real.g_st().set_mpfr_nl(tmp);
        } else {
            ::mpfr_set_z(&m_real.g_dy(), n.get_mpz_view(), MPFR_RNDN);
        }
    }
    template <std::size_t S>
    void dispatch_generic_ctor(const rational<S> &q, ::mpfr_prec_t prec)
    {
        if (!prec) {
            // If the precision is not given explicitly, we will infer it.
            // Compute the number of bits used by q.
            const auto n_size = q.get_num().size();
            const auto d_size = q.get_den().size();
            const auto tot_size = n_size + d_size;
            // Size checks.
            if (mppp_unlikely(
                    // Overflow in tot_size.
                    (n_size > std::numeric_limits<decltype(q.get_num().size())>::max() - d_size)
                    // Check that tot_size * GMP_NUMB_BITS <= max_prec.
                    || (tot_size > static_cast<std::make_unsigned<::mpfr_prec_t>::type>(real_prec_max())
                                       / unsigned(GMP_NUMB_BITS)))) {
                throw std::invalid_argument(
                    "The deduced precision for a real constructed from a rational is too large");
            }
            // Compute the precision. We already know it's a non-negative value not greater
            // than the max allowed precision. We just need to make sure it's not smaller than the
            // min allowed precision.
            prec = c_max(static_cast<::mpfr_prec_t>(static_cast<::mpfr_prec_t>(tot_size) * int(GMP_NUMB_BITS)),
                         real_prec_min());
        }
        // Set the precision (manually specified or deduced).
        set_prec(prec);
        if (is_static()) {
            auto tmp = m_real.g_st().get_mpfr();
            ::mpfr_set_q(&tmp, q.get_mpq_view(), MPFR_RNDN);
            // Copy over.
            m_real.g_st().set_mpfr_nl(tmp);
        } else {
            ::mpfr_set_q(&m_real.g_dy(), q.get_mpq_view(), MPFR_RNDN);
        }
    }
    // NOTE: for signed/unsigned int init, there are MPFR functions working with (u)intmax_t. However, they
    // create constraints on the inclusion order, so let's just use the (u)long functions for now.
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_unsigned<T>>::value, int> = 0>
    void dispatch_generic_ctor(T n, ::mpfr_prec_t prec)
    {
        if (n <= std::numeric_limits<unsigned long>::max()) {
            if (!prec) {
                static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<::mpfr_prec_t>::max(),
                              "Overflow error.");
                prec = c_max(real_prec_min(), static_cast<::mpfr_prec_t>(std::numeric_limits<T>::digits));
            }
            set_prec(prec);
            if (is_static()) {
                auto tmp = m_real.g_st().get_mpfr();
                ::mpfr_set_ui(&tmp, static_cast<unsigned long>(n), MPFR_RNDN);
                m_real.g_st().set_mpfr_nl(tmp);
            } else {
                ::mpfr_set_ui(&m_real.g_dy(), static_cast<unsigned long>(n), MPFR_RNDN);
            }
        } else {
            dispatch_generic_ctor(integer<1>{n}, prec);
        }
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    void dispatch_generic_ctor(T n, ::mpfr_prec_t prec)
    {
        if (n <= std::numeric_limits<long>::max() && n >= std::numeric_limits<long>::min()) {
            if (!prec) {
                static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<::mpfr_prec_t>::max(),
                              "Overflow error.");
                prec = c_max(real_prec_min(), static_cast<::mpfr_prec_t>(std::numeric_limits<T>::digits));
            }
            set_prec(prec);
            if (is_static()) {
                auto tmp = m_real.g_st().get_mpfr();
                ::mpfr_set_si(&tmp, static_cast<long>(n), MPFR_RNDN);
                m_real.g_st().set_mpfr_nl(tmp);
            } else {
                ::mpfr_set_si(&m_real.g_dy(), static_cast<long>(n), MPFR_RNDN);
            }
        } else {
            dispatch_generic_ctor(integer<1>{n}, prec);
        }
    }
    template <typename T, enable_if_t<std::is_floating_point<T>::value, int> = 0>
    void dispatch_generic_ctor(T x, ::mpfr_prec_t prec)
    {
        static_assert(std::numeric_limits<T>::radix == 2, "The float/double type's radix is not 2.");
        static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<::mpfr_prec_t>::max(), "Overflow error.");
        // Check and set the precision, either autodetected or specified.
        set_prec(prec ? prec : c_max(real_prec_min(), static_cast<::mpfr_prec_t>(std::numeric_limits<T>::digits)));
        // Helper to invoke the correct mpfr function.
        auto setter = [x](::mpfr_t m) {
            if (std::is_same<T, float>::value) {
                ::mpfr_set_flt(m, static_cast<float>(x), MPFR_RNDN);
            } else if (std::is_same<T, double>::value) {
                ::mpfr_set_d(m, static_cast<double>(x), MPFR_RNDN);
            } else {
                ::mpfr_set_ld(m, static_cast<long double>(x), MPFR_RNDN);
            }
        };
        if (is_static()) {
            auto tmp = m_real.g_st().get_mpfr();
            setter(&tmp);
            m_real.g_st().set_mpfr_nl(tmp);
        } else {
            setter(&m_real.g_dy());
        }
    }

public:
/// Generic constructor.
/**
 * \rststar
 * This constructor will initialise the value of ``this`` with ``x``. The precision of ``this``
 * is either automatically deduced (if ``prec`` is zero), or explicitly specified by the user
 * (if ``prec`` is nonzero).
 *
 * If ``prec`` is zero, then the precision of ``this`` will be set according to the following heuristic:
 *
 * * if the type of ``x`` is a C++ integral type ``I``, then the precision will be set to the bit
 *   width of ``I``;
 * * if the type of ``x`` is a C++ floating-point type ``F``, then the precision will be set to the bit
 *   width of the significand of ``F``;
 * * if ``x`` is an instance of :cpp:class:`~mppp::integer`, then the precision will be set to the number
 *   of bits used by the representation of ``x`` (rounded up to next multiple of the limb size);
 * * if ``x`` is an instance of :cpp:class:`~mppp::rational`, then the precision will be set to the sum of the number
 *   of bits used by the representations of the numerator and denominator of ``x`` (both rounded up to next multiple of
 *   the limb size).
 *
 * If ``x`` is *not* a :cpp:class:`~mppp::rational`, then ``this`` will be set to the exact value of ``x``, and no
 * rounding takes place during construction. If ``x`` is a :cpp:class:`~mppp::rational`, then the value of ``this``
 * will be rounded to the closest value to ``x`` representable with the automatically-deduced precision.
 *
 * If ``prec`` is nonzero, then the precision of ``this`` is set to ``prec``, and the value of ``this`` will be
 * rounded the closest value to ``x`` representable with the specified precision.
 * \endrststar
 *
 * @param x the construction argument.
 * @param prec the desired precision.
 *
 * @throws std::invalid_argument if the deduced precision when constructing from an mppp::integer or an mppp::rational
 * is larger than an implementation-defined value.
 * @throws unspecified any exception thrown by set_prec().
 */
#if defined(MPPP_HAVE_CONCEPTS)
    explicit real(const RealInteroperable &x
#else
    template <typename T, real_interoperable_enabler<T> = 0>
    explicit real(const T &x
#endif
                  ,
                  ::mpfr_prec_t prec = 0)
    {
        dispatch_generic_ctor(x, prec);
    }
    real &operator=(const real &) = default;
    real &operator=(real &&) = default;
    bool is_static() const
    {
        return m_real.is_static();
    }
    bool is_dynamic() const
    {
        return m_real.is_dynamic();
    }
    ::mpfr_prec_t get_prec() const
    {
        return static_cast<::mpfr_prec_t>(m_real.m_st._mpfr_prec > 0 ? m_real.m_st._mpfr_prec
                                                                     : -m_real.m_st._mpfr_prec);
    }

private:
    static void check_prec(::mpfr_prec_t prec)
    {
        if (mppp_unlikely(prec > real_prec_max() || prec < real_prec_min())) {
            throw std::invalid_argument("An invalid precision of " + std::to_string(prec)
                                        + " was specified for a real object (the minimum allowed precision is "
                                        + std::to_string(real_prec_min()) + ", while the maximum allowed precision is "
                                        + std::to_string(real_prec_max()) + ")");
        }
    }

public:
    real &set_prec(::mpfr_prec_t prec)
    {
        // Determine storage type and current prec.
        bool st = false;
        ::mpfr_prec_t cur_prec = m_real.m_st._mpfr_prec;
        if (cur_prec < 0) {
            st = true;
            cur_prec = -cur_prec;
        }

        // Shortcut if there's nothing to do.
        if (cur_prec == prec) {
            return *this;
        }

        check_prec(prec);
        mpfr_struct_t tmp;
        if (st) {
            if (prec <= static_real<SSize>::max_prec()) {
                // The new precision still fits in static storage. We will first create a new MPFR
                // with the custom interface, using the old value and the new precision. We will then copy
                // it to this.
                // NOTE: zero init the limbs array, as usual.
                // NOTE: use the double nested brackets syntax to work around a GCC<5 warning bug. This should
                // be ok, as std::array is guaranteed to have the same semantics as a class with an array member,
                // and the inner {} value-inits that member.
                std::array<::mp_limb_t, SSize> limbs{{}};
                mpfr_custom_init(limbs.data(), prec);
                mpfr_custom_init_set(&tmp, MPFR_NAN_KIND, 0, prec, limbs.data());
                // Now assign the existing static to tmp, using the new precision.
                const auto cur = m_real.g_st().get_mpfr_c();
                ::mpfr_set(&tmp, &cur, MPFR_RNDN);
                // Finally, copy over from tmp to the static.
                assert(tmp._mpfr_prec == prec);
                m_real.g_st().set_mpfr_nl(tmp);
                m_real.g_st().m_limbs = limbs;
            } else {
                // The desired precision exceeds the maximum static precision. We will build a normal
                // mpfr_t and then move it into the dynamic storage.
                ::mpfr_init2(&tmp, prec);
                // NOTE: everything is no except from now on, no risk of memory leaks.
                // Now assign the existing static to tmp.
                const auto cur = m_real.g_st().get_mpfr_c();
                ::mpfr_set(&tmp, &cur, MPFR_RNDN);
                // Destroy the static, shallow init the dynamic.
                m_real.g_st().~s_storage();
                ::new (static_cast<void *>(&m_real.m_dy)) d_storage(tmp);
            }
        } else {
            if (prec <= static_real<SSize>::max_prec()) {
                // We can demote from dynamic to static. First we create a new MPFR with the custom
                // interface, using the old value and the new precision. We will then copy
                // it to this.
                std::array<::mp_limb_t, SSize> limbs{{}};
                mpfr_custom_init(limbs.data(), prec);
                mpfr_custom_init_set(&tmp, MPFR_NAN_KIND, 0, prec, limbs.data());
                ::mpfr_set(&tmp, &m_real.g_dy(), MPFR_RNDN);
                // Now we can destroy the dynamic member of this, and init the static one.
                m_real.destroy_dynamic();
                // NOTE: there is some overhead here, as we are spending time initing
                // a static member with min prec and zero value. Consider in the future
                // providing an alternate init form which does less work, to be used here.
                ::new (static_cast<void *>(&m_real.m_st)) s_storage();
                // Finally, copy over from tmp to the static.
                assert(tmp._mpfr_prec == prec);
                m_real.g_st().set_mpfr_nl(tmp);
                m_real.g_st().m_limbs = limbs;
            } else {
                // Store a copy of this, as mpfr_set_prec() will erase the value.
                MPPP_MAYBE_TLS mpfr_raii mpfr(real_prec_min());
                ::mpfr_set_prec(&mpfr.m_mpfr, mpfr_get_prec(&m_real.g_dy()));
                ::mpfr_set(&mpfr.m_mpfr, &m_real.g_dy(), MPFR_RNDN);
                // Set the new precision, recover old value from the tmp copy.
                ::mpfr_set_prec(&m_real.g_dy(), prec);
                ::mpfr_set(&m_real.g_dy(), &mpfr.m_mpfr, MPFR_RNDN);
            }
        }
        return *this;
    }
    const real_union<SSize> &_get_union() const
    {
        return m_real;
    }
    real_union<SSize> &_get_union()
    {
        return m_real;
    }

private:
    real_union<SSize> m_real;
};

template <std::size_t SSize>
inline std::ostream &operator<<(std::ostream &os, const real<SSize> &r)
{
    if (r.is_static()) {
        const auto m = r._get_union().g_st().get_mpfr_c();
        mpfr_to_stream(&m, os);
    } else {
        mpfr_to_stream(&r._get_union().g_dy(), os);
    }
    return os;
}

inline namespace detail
{

template <int (*FPtr)(::mpfr_ptr, ::mpfr_srcptr, ::mpfr_srcptr, ::mpfr_rnd_t), std::size_t SSize>
inline void mpfr_binary_op(real<SSize> &rop, const real<SSize> &op1, const real<SSize> &op2)
{
    // Determine storage type of the two operands, and the precision of rop.
    bool s1 = false, s2 = false;
    ::mpfr_prec_t p1 = op1._get_union().m_st._mpfr_prec, p2 = op1._get_union().m_st._mpfr_prec;
    if (p1 < 0) {
        s1 = true;
        p1 = -p1;
    }
    if (p2 < 0) {
        s2 = true;
        p2 = -p2;
    }
    // Set the output precision.
    // TODO preserve?
    rop.set_prec(c_max(p1, p2));

    if (s1 && s2) {
        // Both are static, rp also static as a consequence.
        const auto m1 = op1._get_union().g_st().get_mpfr_c();
        const auto m2 = op2._get_union().g_st().get_mpfr_c();
        auto mr = rop._get_union().g_st().get_mpfr();
        FPtr(&mr, &m1, &m2, MPFR_RNDN);
        rop._get_union().g_st().set_mpfr_nl(mr);
    } else if (s1 && !s2) {
        const auto m1 = op1._get_union().g_st().get_mpfr_c();
        FPtr(&rop._get_union().g_dy(), &m1, &op2._get_union().g_dy(), MPFR_RNDN);
    } else if (!s1 && s2) {
        const auto m2 = op2._get_union().g_st().get_mpfr_c();
        FPtr(&rop._get_union().g_dy(), &op1._get_union().g_dy(), &m2, MPFR_RNDN);
    } else {
        FPtr(&rop._get_union().g_dy(), &op1._get_union().g_dy(), &op2._get_union().g_dy(), MPFR_RNDN);
    }
}
}

template <std::size_t SSize>
inline void add(real<SSize> &rop, const real<SSize> &op1, const real<SSize> &op2)
{
    mpfr_binary_op<::mpfr_add>(rop, op1, op2);
}

template <std::size_t SSize>
inline void mul(real<SSize> &rop, const real<SSize> &op1, const real<SSize> &op2)
{
    mpfr_binary_op<::mpfr_mul>(rop, op1, op2);
}
}

#else

#error The real.hpp header was included but mp++ was not configured with the MPPP_WITH_MPFR option.

#endif

#endif
