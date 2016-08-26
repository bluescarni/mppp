/* Copyright 2009-2016 Francesco Biscani (bluescarni@gmail.com)

This file is part of the mp++ library.

The mp++ library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The mp++ library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the mp++ library.  If not,
see https://www.gnu.org/licenses/. */

#ifndef MPPP_MPPP_HPP
#define MPPP_MPPP_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <gmp.h>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(MPPP_WITH_LONG_DOUBLE)

#include <mpfr.h>

#endif

namespace mppp
{

inline namespace detail
{

// TODO mpz struct checks.

// mpz_t is an array of some struct.
using mpz_struct_t = std::remove_extent<::mpz_t>::type;
// Integral types used for allocation size and number of limbs.
using mpz_alloc_t = decltype(std::declval<mpz_struct_t>()._mp_alloc);
using mpz_size_t = decltype(std::declval<mpz_struct_t>()._mp_size);

// Simple RAII holder for GMP integers.
struct mpz_raii {
    mpz_raii()
    {
        ::mpz_init(&m_mpz);
        assert(m_mpz._mp_alloc >= 0);
    }
    mpz_raii(const mpz_raii &) = delete;
    mpz_raii(mpz_raii &&) = delete;
    mpz_raii &operator=(const mpz_raii &) = delete;
    mpz_raii &operator=(mpz_raii &&) = delete;
    ~mpz_raii()
    {
        // NOTE: even in recent GMP versions, with lazy allocation,
        // it seems like the pointer always points to something:
        // https://gmplib.org/repo/gmp/file/835f8974ff6e/mpz/init.c
        assert(m_mpz._mp_d != nullptr);
        ::mpz_clear(&m_mpz);
    }
    mpz_struct_t m_mpz;
};

#if defined(MPPP_WITH_LONG_DOUBLE)

// mpfr_t is an array of some struct.
using mpfr_struct_t = std::remove_extent<::mpfr_t>::type;

// Simple RAII holder for MPFR floats.
struct mpfr_raii {
    mpfr_raii()
    {
        ::mpfr_init2(&m_mpfr, 53);
    }
    ~mpfr_raii()
    {
        ::mpfr_clear(&m_mpfr);
    }
    mpfr_struct_t m_mpfr;
};

#endif

// Convert an mpz to a string in a specific base.
inline const char *mpz_to_str(const mpz_struct_t *mpz, int base = 10)
{
    assert(base >= 2 && base <= 62);
    const auto size_base = ::mpz_sizeinbase(mpz, base);
    if (size_base > std::numeric_limits<std::size_t>::max() - 2u) {
        throw std::overflow_error("Too many digits in the conversion of mpz_t to string.");
    }
    // Total max size is the size in base plus an optional sign and the null terminator.
    const auto total_size = size_base + 2u;
    // NOTE: possible improvement: use a null allocator to avoid initing the chars each time
    // we resize up.
    static thread_local std::vector<char> tmp;
    tmp.resize(static_cast<std::vector<char>::size_type>(total_size));
    if (tmp.size() != total_size) {
        throw std::overflow_error("Too many digits in the conversion of mpz_t to string.");
    }
    return ::mpz_get_str(&tmp[0u], base, mpz);
}

// Type trait to check if T is a supported integral type.
template <typename T>
using is_supported_integral
    = std::integral_constant<bool, std::is_same<T, bool>::value || std::is_same<T, char>::value
                                       || std::is_same<T, signed char>::value || std::is_same<T, unsigned char>::value
                                       || std::is_same<T, short>::value || std::is_same<T, unsigned short>::value
                                       || std::is_same<T, int>::value || std::is_same<T, unsigned>::value
                                       || std::is_same<T, long>::value || std::is_same<T, unsigned long>::value
                                       || std::is_same<T, long long>::value
                                       || std::is_same<T, unsigned long long>::value>;

// Type trait to check if T is a supported floating-point type.
template <typename T>
using is_supported_float = std::integral_constant<bool, std::is_same<T, float>::value || std::is_same<T, double>::value
#if defined(MPPP_WITH_LONG_DOUBLE)
                                                            || std::is_same<T, long double>::value
#endif
                                                  >;

template <typename T>
using is_supported_interop
    = std::integral_constant<bool, is_supported_integral<T>::value || is_supported_float<T>::value>;

// A global thread local pointer, initially set to null, that signals if a static int construction via mpz
// fails due to too many limbs required. The pointer is set by static_int's constructors to a thread local
// mpz variable that contains the constructed object.
inline const mpz_struct_t * &fail_too_many_limbs()
{
    static thread_local const mpz_struct_t *m = nullptr;
    return m;
}

// The static integer class.
struct static_int {
    // Special alloc value to signal static storage in the union.
    static const mpz_alloc_t s_alloc = -1;
    // Number of limbs in static storage.
    static const int s_size = 2;
    // Let's put a hard cap and sanity check.
    static_assert(s_size > 0 && s_size < 512, "Invalid static size.");
    using limbs_type = std::array<::mp_limb_t, s_size>;
    // NOTE: def ctor leaves the limbs uninited.
    static_int() : _mp_alloc(s_alloc), _mp_size(0)
    {
    }
    static_int(const static_int &o) : _mp_alloc(s_alloc), _mp_size(o._mp_size)
    {
        std::copy(o.m_limbs.begin(), o.m_limbs.begin() + abs_size(), m_limbs.begin());
    }
    // Delegate to the copy ctor.
    static_int(static_int &&o) noexcept : static_int(o)
    {
    }
    // Copy assignment.
    static_int &operator=(const static_int &other)
    {
        if (this != &other) {
            _mp_size = other._mp_size;
            std::copy(other.m_limbs.begin(), other.m_limbs.begin() + other.abs_size(), m_limbs.begin());
        }
        return *this;
    }
    // Move assignment (same as copy assignment).
    static_int &operator=(static_int &&other) noexcept
    {
        return operator=(other);
    }
    ~static_int()
    {
        assert(_mp_alloc == s_alloc);
        assert(_mp_size >= -s_size && _mp_size <= s_size);
    }
    // Size in limbs (absolute value of the _mp_size member).
    mpz_size_t abs_size() const
    {
        return static_cast<mpz_size_t>(_mp_size >= 0 ? _mp_size : -_mp_size);
    }
    // Construct from mpz. If the size in limbs is too large, it will set a global
    // thread local pointer referring to m, so that the constructed mpz can be re-used.
    void ctor_from_mpz(const mpz_struct_t &m)
    {
        if (m._mp_size > s_size || m._mp_size < -s_size) {
            _mp_size = 0;
            fail_too_many_limbs() = &m;
        } else {
            // All this is noexcept.
            _mp_size = m._mp_size;
            std::copy(m._mp_d, m._mp_d + abs_size(), m_limbs.begin());
        }
    }
    template <typename Int,
              typename std::enable_if<is_supported_integral<Int>::value && std::is_unsigned<Int>::value, int>::type = 0>
    bool attempt_1limb_ctor(Int n)
    {
        if (!n) {
            _mp_size = 0;
            return true;
        }
        // This contraption is to avoid a compiler warning when Int is bool: in that case cast it to unsigned,
        // otherwise use the original value.
        if ((std::is_same<bool, Int>::value ? unsigned(n) : n) <= GMP_NUMB_MAX) {
            _mp_size = 1;
            m_limbs[0] = static_cast<::mp_limb_t>(n);
            return true;
        }
        return false;
    }
    template <typename Int,
              typename std::enable_if<is_supported_integral<Int>::value && std::is_signed<Int>::value, int>::type = 0>
    bool attempt_1limb_ctor(Int n)
    {
        using uint_t = typename std::make_unsigned<Int>::type;
        if (!n) {
            _mp_size = 0;
            return true;
        }
        if (n > 0 && uint_t(n) <= GMP_NUMB_MAX) {
            // If n is positive and fits in a limb, just cast it.
            _mp_size = 1;
            m_limbs[0] = static_cast<::mp_limb_t>(n);
            return true;
        }
        // For the negative case, we cast to long long and we check against
        // guaranteed limits for taking the absolute value of n.
        const long long lln = n;
        if (lln < 0 && lln >= -9223372036854775807ll && (unsigned long long)(-lln) <= GMP_NUMB_MAX) {
            _mp_size = -1;
            m_limbs[0] = static_cast<::mp_limb_t>(-lln);
            return true;
        }
        return false;
    }
    // Ctor from unsigned integral types that are wider than unsigned long.
    // This requires special handling as the GMP api does not support unsigned long long natively.
    template <typename Uint, typename std::enable_if<is_supported_integral<Uint>::value && std::is_unsigned<Uint>::value
                                                         && (std::numeric_limits<Uint>::max()
                                                             > std::numeric_limits<unsigned long>::max()),
                                                     int>::type
                             = 0>
    explicit static_int(Uint n) : _mp_alloc(s_alloc)
    {
        if (attempt_1limb_ctor(n)) {
            return;
        }
        static thread_local mpz_raii mpz;
        constexpr auto ulmax = std::numeric_limits<unsigned long>::max();
        if (n <= ulmax) {
            // The value fits unsigned long, just cast it.
            ::mpz_set_ui(&mpz.m_mpz, static_cast<unsigned long>(n));
        } else {
            // Init the shifter.
            static thread_local mpz_raii shifter;
            ::mpz_set_ui(&shifter.m_mpz, 1u);
            // Set output to the lowest UL limb of n.
            ::mpz_set_ui(&mpz.m_mpz, static_cast<unsigned long>(n & ulmax));
            // Move the limbs of n to the right.
            n >>= std::numeric_limits<unsigned long>::digits;
            while (n) {
                // Increase the shifter.
                ::mpz_mul_2exp(&shifter.m_mpz, &shifter.m_mpz, std::numeric_limits<unsigned long>::digits);
                // Add the current lowest UL limb of n to the output, after having multiplied it
                // by the shitfer.
                ::mpz_addmul_ui(&mpz.m_mpz, &shifter.m_mpz, static_cast<unsigned long>(n & ulmax));
                n >>= std::numeric_limits<unsigned long>::digits;
            }
        }
        ctor_from_mpz(mpz.m_mpz);
    }
    // Ctor from unsigned integral types that are not wider than unsigned long.
    template <typename Uint, typename std::enable_if<is_supported_integral<Uint>::value && std::is_unsigned<Uint>::value
                                                         && (std::numeric_limits<Uint>::max()
                                                             <= std::numeric_limits<unsigned long>::max()),
                                                     int>::type
                             = 0>
    explicit static_int(Uint n) : _mp_alloc(s_alloc)
    {
        if (attempt_1limb_ctor(n)) {
            return;
        }
        static thread_local mpz_raii mpz;
        ::mpz_set_ui(&mpz.m_mpz, static_cast<unsigned long>(n));
        ctor_from_mpz(mpz.m_mpz);
    }
    // Ctor from signed integral types that are wider than long.
    template <typename Int,
              typename std::enable_if<std::is_signed<Int>::value && is_supported_integral<Int>::value
                                          && (std::numeric_limits<Int>::max() > std::numeric_limits<long>::max()
                                              || std::numeric_limits<Int>::min() < std::numeric_limits<long>::min()),
                                      int>::type
              = 0>
    explicit static_int(Int n) : _mp_alloc(s_alloc)
    {
        if (attempt_1limb_ctor(n)) {
            return;
        }
        static thread_local mpz_raii mpz;
        constexpr auto lmax = std::numeric_limits<long>::max(), lmin = std::numeric_limits<long>::min();
        if (n <= lmax && n >= lmin) {
            // The value fits long, just cast it.
            ::mpz_set_si(&mpz.m_mpz, static_cast<long>(n));
        } else {
            // A temporary variable for the accumulation of the result in the loop below.
            // Needed because GMP does not have mpz_addmul_si().
            static thread_local mpz_raii tmp;
            // The rest is as above, with the following differences:
            // - use % instead of bit masking and division instead of bit shift,
            // - proceed by chunks of 30 bits, as that's the highest power of 2 portably
            //   representable by long.
            static thread_local mpz_raii shifter;
            ::mpz_set_ui(&shifter.m_mpz, 1u);
            ::mpz_set_si(&mpz.m_mpz, static_cast<long>(n % (1l << 30)));
            n /= (1l << 30);
            while (n) {
                ::mpz_mul_2exp(&shifter.m_mpz, &shifter.m_mpz, 30);
                ::mpz_set_si(&tmp.m_mpz, static_cast<long>(n % (1l << 30)));
                ::mpz_addmul(&mpz.m_mpz, &shifter.m_mpz, &tmp.m_mpz);
                n /= (1l << 30);
            }
        }
        ctor_from_mpz(mpz.m_mpz);
    }
    // Ctor from signed integral types that are not wider than long.
    template <typename Int,
              typename std::enable_if<std::is_signed<Int>::value && is_supported_integral<Int>::value
                                          && (std::numeric_limits<Int>::max() <= std::numeric_limits<long>::max()
                                              && std::numeric_limits<Int>::min() >= std::numeric_limits<long>::min()),
                                      int>::type
              = 0>
    explicit static_int(Int n) : _mp_alloc(s_alloc)
    {
        if (attempt_1limb_ctor(n)) {
            return;
        }
        static thread_local mpz_raii mpz;
        ::mpz_set_si(&mpz.m_mpz, static_cast<long>(n));
        ctor_from_mpz(mpz.m_mpz);
    }
    // Ctor from float or double.
    template <
        typename Float,
        typename std::enable_if<std::is_same<Float, float>::value || std::is_same<Float, double>::value, int>::type = 0>
    explicit static_int(Float f) : _mp_alloc(s_alloc)
    {
        if (!std::isfinite(f)) {
            throw std::invalid_argument("Cannot init integer from non-finite floating-point value.");
        }
        static thread_local mpz_raii mpz;
        ::mpz_set_d(&mpz.m_mpz, static_cast<double>(f));
        ctor_from_mpz(mpz.m_mpz);
    }
#if defined(MPPP_WITH_LONG_DOUBLE)
    // Ctor from long double.
    explicit static_int(long double x) : _mp_alloc(s_alloc)
    {
        if (!std::isfinite(x)) {
            throw std::invalid_argument("Cannot init integer from non-finite floating-point value.");
        }
        static thread_local mpfr_raii mpfr;
        static_assert(std::numeric_limits<long double>::digits10 < std::numeric_limits<int>::max() / 4,
                      "Overflow error.");
        static_assert(std::numeric_limits<long double>::digits10 * 4 < std::numeric_limits<::mpfr_prec_t>::max(),
                      "Overflow error.");
        constexpr int d2 = std::numeric_limits<long double>::digits10 * 4;
        ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(d2));
        ::mpfr_set_ld(&mpfr.m_mpfr, x, MPFR_RNDN);
        static thread_local mpz_raii mpz;
        ::mpfr_get_z(&mpz.m_mpz, &mpfr.m_mpfr, MPFR_RNDZ);
        ctor_from_mpz(mpz.m_mpz);
    }
#endif

    class static_mpz_view
    {
    public:
        // NOTE: we use the const_cast to cast away the constness from the pointer to the limbs
        // in n. This is valid as we are never going to use this pointer for writing.
        explicit static_mpz_view(const static_int &n)
            : m_mpz{s_size, n._mp_size, const_cast<::mp_limb_t *>(n.m_limbs.data())}
        {
        }
        static_mpz_view(const static_mpz_view &) = delete;
        static_mpz_view(static_mpz_view &&) = default;
        static_mpz_view &operator=(const static_mpz_view &) = delete;
        static_mpz_view &operator=(static_mpz_view &&) = delete;
        operator const mpz_struct_t *() const
        {
            return &m_mpz;
        }

    private:
        mpz_struct_t m_mpz;
    };
    static_mpz_view get_mpz_view() const
    {
        return static_mpz_view{*this};
    }
    mpz_alloc_t _mp_alloc;
    mpz_size_t _mp_size;
    limbs_type m_limbs;
};

// {static_int,mpz} union.
union integer_union {
public:
    using s_storage = static_int;
    using d_storage = mpz_struct_t;
    // Utility function to shallow copy "from" into "to".
    static void mpz_shallow_copy(mpz_struct_t &to, const mpz_struct_t &from)
    {
        to._mp_alloc = from._mp_alloc;
        to._mp_size = from._mp_size;
        to._mp_d = from._mp_d;
    }
    // Def ctor, will init to static.
    integer_union() : m_st()
    {
    }
    // Copy constructor, does a deep copy maintaining the storage class of other.
    integer_union(const integer_union &other)
    {
        if (other.is_static()) {
            ::new (static_cast<void *>(&m_st)) s_storage(other.g_st());
        } else {
            ::new (static_cast<void *>(&m_dy)) d_storage;
            ::mpz_init_set(&m_dy, &other.g_dy());
            assert(m_dy._mp_alloc >= 0);
        }
    }
    // Move constructor. Will downgrade other to a static zero integer if other is dynamic.
    integer_union(integer_union &&other) noexcept
    {
        if (other.is_static()) {
            ::new (static_cast<void *>(&m_st)) s_storage(std::move(other.g_st()));
        } else {
            ::new (static_cast<void *>(&m_dy)) d_storage;
            mpz_shallow_copy(m_dy, other.g_dy());
            // Downgrade the other to an empty static.
            other.g_dy().~d_storage();
            ::new (static_cast<void *>(&other.m_st)) s_storage();
        }
    }
    // Generic constructor from the interoperable basic C++ types. It will first try to construct
    // a static, if too many limbs are needed it will construct a dynamic instead.
    template <typename T, typename std::enable_if<is_supported_interop<T>::value, int>::type = 0>
    explicit integer_union(T x)
    {
        // Attempt static storage construction.
        ::new (static_cast<void *>(&m_st)) s_storage(x);
        // Check if too many limbs were generated.
        auto ptr = fail_too_many_limbs();
        if (ptr) {
            // Get the pointer to the mpz storing the constructed value, then reset it.
            fail_too_many_limbs() = nullptr;
            // Destroy static.
            g_st().~s_storage();
            // Init dynamic.
            ::new (static_cast<void *>(&m_dy)) d_storage;
            ::mpz_init_set(&m_dy, ptr);
        }
    }
    // Copy assignment operator, performs a deep copy maintaining the storage class.
    integer_union &operator=(const integer_union &other)
    {
        if (this == &other) {
            return *this;
        }
        const bool s1 = is_static(), s2 = other.is_static();
        if (s1 && s2) {
            g_st() = other.g_st();
        } else if (s1 && !s2) {
            // Destroy static.
            g_st().~s_storage();
            // Construct the dynamic struct.
            ::new (static_cast<void *>(&m_dy)) d_storage;
            // Init + assign the mpz.
            ::mpz_init_set(&m_dy, &other.g_dy());
            assert(m_dy._mp_alloc >= 0);
        } else if (!s1 && s2) {
            // Destroy the dynamic this.
            destroy_dynamic();
            // Init-copy the static from other.
            ::new (static_cast<void *>(&m_st)) s_storage(other.g_st());
        } else {
            ::mpz_set(&g_dy(), &other.g_dy());
        }
        return *this;
    }
    // Move assignment, same as above plus possibly steals resources. If this is static
    // and other is dynamic, other is downgraded to a zero static.
    integer_union &operator=(integer_union &&other) noexcept
    {
        if (this == &other) {
            return *this;
        }
        const bool s1 = is_static(), s2 = other.is_static();
        if (s1 && s2) {
            g_st() = std::move(other.g_st());
        } else if (s1 && !s2) {
            // Destroy static.
            g_st().~s_storage();
            // Construct the dynamic struct.
            ::new (static_cast<void *>(&m_dy)) d_storage;
            mpz_shallow_copy(m_dy, other.g_dy());
            // Downgrade the other to an empty static.
            other.g_dy().~d_storage();
            ::new (static_cast<void *>(&other.m_st)) s_storage();
        } else if (!s1 && s2) {
            // Same as copy assignment: destroy and copy-construct.
            destroy_dynamic();
            ::new (static_cast<void *>(&m_st)) s_storage(other.g_st());
        } else {
            // Swap with other.
            ::mpz_swap(&g_dy(), &other.g_dy());
        }
        return *this;
    }
    ~integer_union()
    {
        if (is_static()) {
            g_st().~s_storage();
        } else {
            destroy_dynamic();
        }
    }
    void destroy_dynamic()
    {
        assert(!is_static());
        assert(g_dy()._mp_alloc >= 0);
        assert(g_dy()._mp_d != nullptr);
        ::mpz_clear(&g_dy());
        m_dy.~d_storage();
    }
    // Check static flag.
    bool is_static() const
    {
        return m_st._mp_alloc == s_storage::s_alloc;
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
        assert(!is_static());
        return m_dy;
    }
    d_storage &g_dy()
    {
        assert(!is_static());
        return m_dy;
    }
    // Promotion from static to dynamic.
    void promote()
    {
        assert(is_static());
        // Construct an mpz from the static.
        mpz_struct_t tmp_mpz;
        auto v = g_st().get_mpz_view();
        ::mpz_init_set(&tmp_mpz, v);
        // Destroy static.
        g_st().~s_storage();
        // Construct the dynamic struct.
        ::new (static_cast<void *>(&m_dy)) d_storage;
        mpz_shallow_copy(m_dy, tmp_mpz);
    }

private:
    s_storage m_st;
    d_storage m_dy;
};

// Utility function to detect the highest bit set in the data part of a limb. Adapted from:
// https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup
// The data part of n must not be zero.
inline unsigned msb_index(::mp_limb_t n)
{
    // This is a table that stores the MSB index for the values from 0 to 255.
    constexpr std::array<unsigned char, 256> lt_256 = {{
#define MPP_LT256(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
        0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, MPP_LT256(4), MPP_LT256(5), MPP_LT256(5), MPP_LT256(6),
        MPP_LT256(6), MPP_LT256(6), MPP_LT256(6), MPP_LT256(7), MPP_LT256(7), MPP_LT256(7), MPP_LT256(7), MPP_LT256(7),
        MPP_LT256(7), MPP_LT256(7), MPP_LT256(7)
#undef MPP_LT256
    }};
    // Remove non-data bits.
    n = n & GMP_NUMB_MASK;
    assert(n);
    static_assert(GMP_NUMB_BITS >= 8, "Invalid number of bits.");
    auto shift = static_cast<unsigned>(GMP_NUMB_BITS - 8);
    // NOTE: the idea here is that we keep decreasing the shift amount 8 bits at a time until
    // we remain with some nonzero value when doing n >> shift.
    while (shift) {
        const ::mp_limb_t tmp = n >> shift;
        if (tmp) {
            return shift + lt_256[std::size_t(tmp)];
        }
        if (GMP_NUMB_BITS % 8) {
            // If GMP_NUMB_BITS is not divided exactly by 8 (a nail build perhaps), we need to make sure
            // that we don't decrease shift too much.
            shift = shift >= 8u ? (shift - 8u) : 0u;
        } else {
            // If GMP_NUMB_BITS is divided exactly by 8 (as usual) we can keep on subtracting
            // without fear of shift wrapping over.
            shift -= 8u;
        }
    }
    // The number must be smaller 256, use the table directly.
    return lt_256[std::size_t(n)];
}
}

class integer
{
    template <typename T>
    using generic_ctor_enabler = typename std::enable_if<is_supported_interop<T>::value, int>::type;
    // Conversion operator.
    template <typename T>
    using generic_conversion_enabler = generic_ctor_enabler<T>;
    template <typename T>
    using uint_conversion_enabler =
        typename std::enable_if<is_supported_integral<T>::value && std::is_unsigned<T>::value
                                    && !std::is_same<bool, T>::value,
                                int>::type;
    template <typename T>
    using int_conversion_enabler =
        typename std::enable_if<is_supported_integral<T>::value && std::is_signed<T>::value, int>::type;
    // Static conversion to bool.
    template <typename T, typename std::enable_if<std::is_same<T, bool>::value, int>::type = 0>
    static T conversion_impl(const static_int &n)
    {
        return n._mp_size != 0;
    }
    // Static conversion to unsigned ints.
    template <typename T, uint_conversion_enabler<T> = 0>
    static T conversion_impl(const static_int &n)
    {
        // Handle zero.
        if (!n._mp_size) {
            return T(0);
        }
        if (n._mp_size == 1) {
            // Single-limb, positive value case.
            if ((n.m_limbs[0] & GMP_NUMB_MASK) > std::numeric_limits<T>::max()) {
                // TODO error message.
                throw std::overflow_error("");
            }
            return static_cast<T>(n.m_limbs[0] & GMP_NUMB_MASK);
        }
        if (n._mp_size < 0) {
            // Negative values cannot be converted to unsigned ints.
            // TODO error message.
            throw std::overflow_error("");
        }
        // In multilimb case, forward to mpz.
        return conversion_impl<T>(*static_cast<const mpz_struct_t *>(n.get_mpz_view()));
    }
    // Static conversion to signed ints.
    template <typename T, int_conversion_enabler<T> = 0>
    static T conversion_impl(const static_int &n)
    {
        using uint_t = typename std::make_unsigned<T>::type;
        // Handle zero.
        if (!n._mp_size) {
            return T(0);
        }
        if (n._mp_size == 1) {
            // Single-limb, positive value case.
            if ((n.m_limbs[0] & GMP_NUMB_MASK) > uint_t(std::numeric_limits<T>::max())) {
                // TODO error message.
                throw std::overflow_error("");
            }
            return static_cast<T>(n.m_limbs[0] & GMP_NUMB_MASK);
        }
        if (n._mp_size == -1 && (n.m_limbs[0] & GMP_NUMB_MASK) <= 9223372036854775807ull) {
            // Handle negative single limb only if we are in the safe range of long long.
            const auto candidate = -static_cast<long long>(n.m_limbs[0] & GMP_NUMB_MASK);
            if (candidate < std::numeric_limits<T>::min()) {
                // TODO error message.
                throw std::overflow_error("");
            }
            return static_cast<T>(candidate);
        }
        // Forward to mpz.
        return conversion_impl<T>(*static_cast<const mpz_struct_t *>(n.get_mpz_view()));
    }
    // Dynamic conversion to bool.
    template <typename T, typename std::enable_if<std::is_same<T, bool>::value, int>::type = 0>
    static T conversion_impl(const mpz_struct_t &m)
    {
        return mpz_sgn(&m);
    }
    // Dynamic conversion to unsigned ints.
    template <typename T, uint_conversion_enabler<T> = 0>
    static T conversion_impl(const mpz_struct_t &m)
    {
        if (mpz_sgn(&m) < 0) {
            // Cannot convert negative values into unsigned ints.
            // TODO error message.
            throw std::overflow_error("");
        }
        if (::mpz_fits_ulong_p(&m)) {
            // Go through GMP if possible.
            const auto ul = ::mpz_get_ui(&m);
            if (ul <= std::numeric_limits<T>::max()) {
                return static_cast<T>(ul);
            }
            // TODO error message.
            throw std::overflow_error("");
        }
        // We are now in a situation in which m does not fit in ulong. The only hope is that it does
        // fit in ulonglong. We will try to build one operating in 31-bit chunks (2**31 is the highest
        // power of 2 guaranteed to be representable by unsigned long - we need to use unsigned long
        // instead of unsigned long long because of the GMP API).
        unsigned long long retval = 0u;
        // q will be a copy of m that will be right-shifted down in chunks.
        static thread_local mpz_raii q;
        ::mpz_set(&q.m_mpz, &m);
        // Init the multiplier for use in the loop below.
        unsigned long long multiplier = 1u;
        // Handy shortcut.
        constexpr auto ull_max = std::numeric_limits<unsigned long long>::max();
        while (true) {
            // NOTE: mpz_get_ui() already gives the lower bits of q, select the first 31 bits.
            unsigned long long ull = ::mpz_get_ui(&q.m_mpz) & ((1ul << 31) - 1ul);
            // Overflow check.
            if (ull > ull_max / multiplier) {
                // TODO message.
                throw std::overflow_error("");
            }
            // Shift up the current 31 bits being considered.
            ull *= multiplier;
            // Overflow check.
            if (retval > ull_max - ull) {
                // TODO message.
                throw std::overflow_error("");
            }
            // Add the current 31 bits to the result.
            retval += ull;
            // Shift down q.
            ::mpz_tdiv_q_2exp(&q.m_mpz, &q.m_mpz, 31);
            // The iteration will stop when q becomes zero.
            if (!mpz_sgn(&q.m_mpz)) {
                break;
            }
            // Overflow check.
            if (multiplier > ull_max / (1ul << 31)) {
                // TODO message.
                throw std::overflow_error("");
            }
            // Update the multiplier.
            multiplier *= (1ul << 31);
        }
        if (retval > std::numeric_limits<T>::max()) {
            // TODO error message.
            throw std::overflow_error("");
        }
        return static_cast<T>(retval);
    }
    // Dynamic conversion to signed ints.
    template <typename T, int_conversion_enabler<T> = 0>
    static T conversion_impl(const mpz_struct_t &m)
    {
        if (::mpz_fits_slong_p(&m)) {
            const auto sl = ::mpz_get_si(&m);
            if (sl >= std::numeric_limits<T>::min() && sl <= std::numeric_limits<T>::max()) {
                return static_cast<T>(sl);
            }
            // TODO error message.
            throw std::overflow_error("");
        }
        // The same approach as for the unsigned case, just slightly more complicated because
        // of the presence of the sign.
        long long retval = 0;
        static thread_local mpz_raii q;
        ::mpz_set(&q.m_mpz, &m);
        const bool sign = mpz_sgn(&q.m_mpz) > 0;
        long long multiplier = sign ? 1 : -1;
        // Shortcuts.
        constexpr auto ll_max = std::numeric_limits<long long>::max();
        constexpr auto ll_min = std::numeric_limits<long long>::min();
        while (true) {
            auto ll = static_cast<long long>(::mpz_get_ui(&q.m_mpz) & ((1ul << 31) - 1ul));
            if ((sign && ll && multiplier > ll_max / ll) || (!sign && ll && multiplier < ll_min / ll)) {
                // TODO error message.
                throw std::overflow_error("1");
            }
            ll *= multiplier;
            if ((sign && retval > ll_max - ll) || (!sign && retval < ll_min - ll)) {
                // TODO error message.
                throw std::overflow_error("2");
            }
            retval += ll;
            ::mpz_tdiv_q_2exp(&q.m_mpz, &q.m_mpz, 31);
            if (!mpz_sgn(&q.m_mpz)) {
                break;
            }
            if ((sign && multiplier > ll_max / (1ll << 31)) || (!sign && multiplier < ll_min / (1ll << 31))) {
                // TODO error message.
                throw std::overflow_error("3");
            }
            multiplier *= (1ll << 31);
        }
        if (retval > std::numeric_limits<T>::max() || retval < std::numeric_limits<T>::min()) {
            // TODO error message.
            throw std::overflow_error("4");
        }
        return static_cast<T>(retval);
    }

public:
    integer() = default;
    integer(const integer &other) = default;
    integer(integer &&other) = default;
    template <typename T, generic_ctor_enabler<T> = 0>
    explicit integer(T x) : m_int(x)
    {
    }
    integer &operator=(const integer &other) = default;
    integer &operator=(integer &&other) = default;
    bool is_static() const
    {
        return m_int.is_static();
    }
    friend std::ostream &operator<<(std::ostream &os, const integer &n)
    {
        return os << n.to_string();
    }
    const char *to_string(int base = 10) const
    {
        if (base < 2 || base > 62) {
            throw std::invalid_argument("Invalid base for string conversion: the base must be between "
                                        "2 and 62, but a value of "
                                        + std::to_string(base) + " was provided instead.");
        }
        if (is_static()) {
            return mpz_to_str(m_int.g_st().get_mpz_view());
        }
        return mpz_to_str(&m_int.g_dy());
    }
    template <typename T, generic_conversion_enabler<T> = 0>
    explicit operator T() const
    {
        if (is_static()) {
            return conversion_impl<T>(m_int.g_st());
        }
        return conversion_impl<T>(m_int.g_dy());
    }
    void promote()
    {
        if (!m_int.is_static()) {
            // TODO throw.
            throw std::invalid_argument("");
        }
        m_int.promote();
    }

private:
    integer_union m_int;
};
}

#endif
