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

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <gmp.h>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
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

// TODO mpz struct checks -> TODO check about _mp_size as well.
// TODO --> config.hpp
#define MPPP_UINT128 __uint128_t

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

// A couple of sanity checks when constructing temporary mpfrs from long double.
static_assert(std::numeric_limits<long double>::digits10 < std::numeric_limits<int>::max() / 4, "Overflow error.");
static_assert(std::numeric_limits<long double>::digits10 * 4 < std::numeric_limits<::mpfr_prec_t>::max(),
              "Overflow error.");

#endif

// Convert an mpz to a string in a specific base.
// NOTE: this function returns a pointer to thread_local storage: the return value will be overwritten
// by successive calls to mpz_to_str() from the same thread.
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
    return ::mpz_get_str(&tmp[0], base, mpz);
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
inline const mpz_struct_t *&fail_too_many_limbs()
{
    static thread_local const mpz_struct_t *m = nullptr;
    return m;
}

// Small wrapper to copy limbs. We cannot use std::copy because it requires non-overlapping ranges.
inline void copy_limbs(const ::mp_limb_t *begin, const ::mp_limb_t *end, ::mp_limb_t *out)
{
    for (; begin != end; ++begin, ++out) {
        *out = *begin;
    }
}

// The static integer class.
template <std::size_t SSize>
struct static_int {
    // Let's put a hard cap and sanity check on the static size.
    static_assert(SSize > 0u && SSize <= 64u, "Invalid static size.");
    using limbs_type = std::array<::mp_limb_t, SSize>;
    // Cast it to mpz_size_t for convenience.
    static const mpz_size_t s_size = SSize;
    // Special alloc value to signal static storage in the union.
    static const mpz_alloc_t s_alloc = -1;
    // NOTE: def ctor leaves the limbs uninited.
    static_int() : _mp_alloc(s_alloc), _mp_size(0)
    {
    }
    // NOTE: implement this manually - the default implementation would read from uninited limbs
    // in case o is not full.
    static_int(const static_int &o) : _mp_alloc(s_alloc), _mp_size(o._mp_size)
    {
        copy_limbs(o.m_limbs.data(), o.m_limbs.data() + abs_size(), m_limbs.data());
    }
    // Delegate to the copy ctor.
    static_int(static_int &&o) noexcept : static_int(o)
    {
    }
    // NOTE: implement this manually - the default implementation would read from uninited limbs
    // in case other is not full.
    static_int &operator=(const static_int &other)
    {
        if (this != &other) {
            _mp_size = other._mp_size;
            copy_limbs(other.m_limbs.data(), other.m_limbs.data() + other.abs_size(), m_limbs.data());
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
        return _mp_size >= 0 ? _mp_size : -_mp_size;
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
            copy_limbs(m._mp_d, m._mp_d + abs_size(), m_limbs.data());
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
            // NOTE: this is ok because we tested above that n is wider than unsigned long, so its bit
            // width must be larger than unsigned long's.
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
        // NOTE: this is needed when we have the variant view in the integer class: if the active view
        // is the dynamic one, we need to def construct a static view that we will never use.
        // NOTE: m_mpz needs to be zero inited because otherwise when using the move ctor we will
        // be reading from uninited memory.
        static_mpz_view() : m_mpz()
        {
        }
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

// Utility function to shallow copy "from" into "to".
void inline mpz_shallow_copy(mpz_struct_t &to, const mpz_struct_t &from)
{
    to._mp_alloc = from._mp_alloc;
    to._mp_size = from._mp_size;
    to._mp_d = from._mp_d;
}

// {static_int,mpz} union.
template <std::size_t SSize>
union integer_union {
public:
    using s_storage = static_int<SSize>;
    using d_storage = mpz_struct_t;
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
            // Reset the pointer before proceeding.
            fail_too_many_limbs() = nullptr;
            // Destroy static.
            g_st().~s_storage();
            // Init dynamic.
            ::new (static_cast<void *>(&m_dy)) d_storage;
            ::mpz_init_set(&m_dy, ptr);
        }
    }
    explicit integer_union(const char *s, int base) : m_st()
    {
        static thread_local mpz_raii mpz;
        if (::mpz_set_str(&mpz.m_mpz, s, base)) {
            throw std::invalid_argument(std::string("The string '") + s + "' is not a valid integer in base "
                                        + std::to_string(base) + ".");
        }
        g_st().ctor_from_mpz(mpz.m_mpz);
        // Check if too many limbs were generated.
        auto ptr = fail_too_many_limbs();
        if (ptr) {
            // Reset the pointer before proceeding.
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
    // Reset a dynamic int to a default constructed static.
    void reset_to_static()
    {
        assert(is_dynamic());
        // Destroy the dynamic storage.
        destroy_dynamic();
        // Init the static storage.
        ::new (static_cast<void *>(&m_st)) s_storage();
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
    // Demotion from dynamic to static.
    bool demote()
    {
        assert(!is_static());
        const auto dyn_size = ::mpz_size(&g_dy());
        // If the dynamic size is greater than the static size, we cannot demote.
        if (dyn_size > SSize) {
            return false;
        }
        // Copy over the limbs to temporary storage.
        std::array<::mp_limb_t, SSize> tmp;
        copy_limbs(g_dy()._mp_d, g_dy()._mp_d + dyn_size, tmp.data());
        const auto signed_size = g_dy()._mp_size;
        // Destroy the dynamic storage.
        destroy_dynamic();
        // Init the static storage and copy over the data.
        ::new (static_cast<void *>(&m_st)) s_storage();
        g_st()._mp_size = signed_size;
        copy_limbs(tmp.data(), tmp.data() + dyn_size, g_st().m_limbs.data());
    }
    void negate()
    {
        // NOTE: _mp_size is part of the common initial sequence.
        m_st._mp_size = -m_st._mp_size;
    }

    s_storage m_st;
    d_storage m_dy;
};
}

template <std::size_t SSize>
class mp_integer
{
    // The underlying static int.
    using s_int = static_int<SSize>;
    // TODO maybe move the enablers next to where they are used.
    // Enabler for generic ctor.
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
    static T conversion_impl(const s_int &n)
    {
        return n._mp_size != 0;
    }
    // Static conversion to unsigned ints.
    template <typename T, uint_conversion_enabler<T> = 0>
    static T conversion_impl(const s_int &n)
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
    static T conversion_impl(const s_int &n)
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
    // Static conversion to floating-point.
    template <typename T, typename std::enable_if<is_supported_float<T>::value, int>::type = 0>
    static T conversion_impl(const s_int &n)
    {
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
        // fit in ulonglong. We will try to build one operating in 32-bit chunks.
        unsigned long long retval = 0u;
        // q will be a copy of m that will be right-shifted down in chunks.
        static thread_local mpz_raii q;
        ::mpz_set(&q.m_mpz, &m);
        // Init the multiplier for use in the loop below.
        unsigned long long multiplier = 1u;
        // Handy shortcut.
        constexpr auto ull_max = std::numeric_limits<unsigned long long>::max();
        while (true) {
            // NOTE: mpz_get_ui() already gives the lower bits of q, select the first 32 bits.
            unsigned long long ull = ::mpz_get_ui(&q.m_mpz) & ((1ull << 32) - 1ull);
            // Overflow check.
            if (ull > ull_max / multiplier) {
                // TODO message.
                throw std::overflow_error("");
            }
            // Shift up the current 32 bits being considered.
            ull *= multiplier;
            // Overflow check.
            if (retval > ull_max - ull) {
                // TODO message.
                throw std::overflow_error("");
            }
            // Add the current 32 bits to the result.
            retval += ull;
            // Shift down q.
            ::mpz_tdiv_q_2exp(&q.m_mpz, &q.m_mpz, 32);
            // The iteration will stop when q becomes zero.
            if (!mpz_sgn(&q.m_mpz)) {
                break;
            }
            // Overflow check.
            if (multiplier > ull_max / (1ull << 32)) {
                // TODO message.
                throw std::overflow_error("");
            }
            // Update the multiplier.
            multiplier *= 1ull << 32;
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
            auto ll = static_cast<long long>(::mpz_get_ui(&q.m_mpz) & ((1ull << 32) - 1ull));
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
            ::mpz_tdiv_q_2exp(&q.m_mpz, &q.m_mpz, 32);
            if (!mpz_sgn(&q.m_mpz)) {
                break;
            }
            if ((sign && multiplier > ll_max / (1ll << 32)) || (!sign && multiplier < ll_min / (1ll << 32))) {
                // TODO error message.
                throw std::overflow_error("3");
            }
            multiplier *= 1ll << 32;
        }
        if (retval > std::numeric_limits<T>::max() || retval < std::numeric_limits<T>::min()) {
            // TODO error message.
            throw std::overflow_error("4");
        }
        return static_cast<T>(retval);
    }
    // Dynamic conversion to float/double.
    template <typename T,
              typename std::enable_if<std::is_same<T, float>::value || std::is_same<T, double>::value, int>::type = 0>
    static T conversion_impl(const mpz_struct_t &m)
    {
        return static_cast<T>(::mpz_get_d(&m));
    }
#if defined(MPPP_WITH_LONG_DOUBLE)
    // Dynamic conversion to long double.
    template <typename T, typename std::enable_if<std::is_same<T, long double>::value, int>::type = 0>
    static T conversion_impl(const mpz_struct_t &m)
    {
        static thread_local mpfr_raii mpfr;
        constexpr int d2 = std::numeric_limits<long double>::digits10 * 4;
        ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(d2));
        ::mpfr_set_z(&mpfr.m_mpfr, &m, MPFR_RNDN);
        return ::mpfr_get_ld(&mpfr.m_mpfr, MPFR_RNDN);
    }
#endif
    // mpz view class.
    class mpz_view
    {
        using static_mpz_view = typename s_int::static_mpz_view;

    public:
        explicit mpz_view(const mp_integer &n)
            : m_static_view(n.is_static() ? n.m_int.g_st().get_mpz_view() : static_mpz_view{}),
              m_ptr(n.is_static() ? m_static_view : &(n.m_int.g_dy()))
        {
        }
        mpz_view(const mpz_view &) = delete;
        mpz_view(mpz_view &&) = default;
        mpz_view &operator=(const mpz_view &) = delete;
        mpz_view &operator=(mpz_view &&) = delete;
        operator const mpz_struct_t *() const
        {
            return get();
        }
        const mpz_struct_t *get() const
        {
            return m_ptr;
        }

    private:
        static_mpz_view m_static_view;
        const mpz_struct_t *m_ptr;
    };

public:
    mp_integer() = default;
    mp_integer(const mp_integer &other) = default;
    mp_integer(mp_integer &&other) = default;
    template <typename T, generic_ctor_enabler<T> = 0>
    explicit mp_integer(T x) : m_int(x)
    {
    }
    explicit mp_integer(const char *s, int base = 10) : m_int(s, base)
    {
    }
    explicit mp_integer(const std::string &s, int base = 10) : mp_integer(s.c_str(), base)
    {
    }
    mp_integer &operator=(const mp_integer &other) = default;
    mp_integer &operator=(mp_integer &&other) = default;
    bool is_static() const
    {
        return m_int.is_static();
    }
    friend std::ostream &operator<<(std::ostream &os, const mp_integer &n)
    {
        return os << n.to_string();
    }
    std::string to_string(int base = 10) const
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
        if (!is_static()) {
            // TODO throw.
            throw std::invalid_argument("");
        }
        m_int.promote();
    }
    void negate()
    {
        m_int.negate();
    }
    std::size_t nbits() const
    {
        return ::mpz_sizeinbase(get_mpz_view(), 2);
    }
    mpz_view get_mpz_view() const
    {
        return mpz_view(*this);
    }

private:
    // Metaprogramming for selecting the algorithm for static addition. The selection happens via
    // an std::integral_constant with 3 possible values:
    // - 0 (default case): use the GMP mpn functions,
    // - 1: selected when there are no nail bits and the static size is 1,
    // - 2: selected when there are no nail bits and the static size is 2.
    template <typename SInt>
    using static_add_algo = std::integral_constant<int, (!GMP_NAIL_BITS && SInt::s_size == 1)
                                                            ? 1
                                                            : ((!GMP_NAIL_BITS && SInt::s_size == 2) ? 2 : 0)>;
    // General implementation via mpn.
    static int static_add_impl(s_int &rop, ::mp_limb_t *rdata, const ::mp_limb_t *data1, mpz_size_t size1,
                               mpz_size_t asize1, int sign1, const ::mp_limb_t *data2, mpz_size_t size2,
                               mpz_size_t asize2, int sign2, const std::integral_constant<int, 0> &)
    {
        // mpn functions require nonzero arguments.
        assert(asize1 > 0 && asize2 > 0);
        if (sign1 == sign2) {
            // Same sign.
            if (asize1 >= asize2) {
                // The number of limbs of op1 >= op2.
                ::mp_limb_t cy;
                if (asize2 == 1) {
                    // NOTE: we are not masking data2[0] with GMP_NUMB_MASK, I am assuming the mpn function
                    // is able to deal with a limb with a nail.
                    cy = ::mpn_add_1(rdata, data1, static_cast<::mp_size_t>(asize1), data2[0]);
                } else if (asize1 == asize2) {
                    cy = ::mpn_add_n(rdata, data1, data2, static_cast<::mp_size_t>(asize1));
                } else {
                    cy = ::mpn_add(rdata, data1, static_cast<::mp_size_t>(asize1), data2,
                                   static_cast<::mp_size_t>(asize2));
                }
                if (cy) {
                    // If there is a carry digit, we need to check if we can increase the size of the static.
                    // Otherwise, we return failure.
                    if (asize1 == s_int::s_size) {
                        rop._mp_size = size1;
                        return 1;
                    }
                    rop._mp_size = size1 + sign1;
                    // NOTE: there should be no need to use GMP_NUMB_MASK here.
                    rdata[asize1] = 1;
                } else {
                    // Without carry, the size is unchanged.
                    rop._mp_size = size1;
                }
                return 0;
            } else {
                // The number of limbs of op2 > op1.
                ::mp_limb_t cy;
                if (asize1 == 1) {
                    cy = ::mpn_add_1(rdata, data2, static_cast<::mp_size_t>(asize2), data1[0]);
                } else {
                    cy = ::mpn_add(rdata, data2, static_cast<::mp_size_t>(asize2), data1,
                                   static_cast<::mp_size_t>(asize1));
                }
                if (cy) {
                    if (asize2 == s_int::s_size) {
                        rop._mp_size = size2;
                        return 1;
                    }
                    rop._mp_size = size2 + sign2;
                    rdata[asize2] = 1;
                } else {
                    rop._mp_size = size2;
                }
                return 0;
            }
        } else {
            if (asize1 > asize2
                || (asize1 == asize2 && ::mpn_cmp(data1, data2, static_cast<::mp_size_t>(asize1)) >= 0)) {
                // abs(op1) >= abs(op2).
                ::mp_limb_t br;
                if (asize2 == 1) {
                    br = ::mpn_sub_1(rdata, data1, static_cast<::mp_size_t>(asize1), data2[0]);
                } else if (asize1 == asize2) {
                    br = ::mpn_sub_n(rdata, data1, data2, static_cast<::mp_size_t>(asize1));
                } else {
                    br = ::mpn_sub(rdata, data1, static_cast<::mp_size_t>(asize1), data2,
                                   static_cast<::mp_size_t>(asize2));
                }
                assert(!br);
                mpz_size_t s = asize1;
                // When subtracting, we need to compute the size. It could be anything from s (the larger size) to zero.
                while (s > 0) {
                    if (rdata[--s] & GMP_NUMB_MASK) {
                        break;
                    }
                }
                rop._mp_size = (sign1 == 1) ? s : -s;
            } else {
                // abs(op2) > abs(op1).
                ::mp_limb_t br;
                if (asize1 == 1) {
                    br = ::mpn_sub_1(rdata, data2, static_cast<::mp_size_t>(asize2), data1[0]);
                } else {
                    br = ::mpn_sub(rdata, data2, static_cast<::mp_size_t>(asize2), data1,
                                   static_cast<::mp_size_t>(asize1));
                }
                assert(!br);
                mpz_size_t s = asize2;
                while (s > 0) {
                    if (rdata[--s] & GMP_NUMB_MASK) {
                        break;
                    }
                }
                rop._mp_size = (sign2 == 1) ? s : -s;
            }
            return 0;
        }
        assert(false);
    }
    // Optimization for single-limb statics with no nails.
    static int static_add_impl(s_int &rop, ::mp_limb_t *rdata, const ::mp_limb_t *data1, mpz_size_t size1,
                               mpz_size_t asize1, int sign1, const ::mp_limb_t *data2, mpz_size_t size2,
                               mpz_size_t asize2, int sign2, const std::integral_constant<int, 1> &)
    {
        // NOTE: both sizes have to be 1 here.
        assert(asize1 == 1 && asize2 == 1);
        if (sign1 == sign2) {
            // When the signs are identical, we can implement addition as a true addition.
            const auto tmp = data1[0] + data2[0];
            // Detect overflow in the result.
            const int retval = tmp < data1[0];
            // Assign the output. The abs size will always be 1.
            rop._mp_size = sign1;
            rdata[0] = tmp;
            return retval;
        } else {
            // When the signs differ, we need to implement addition as a subtraction.
            if (data1[0] >= data2[0]) {
                // op1 is not smaller than op2.
                const auto tmp = data1[0] - data2[0];
                // asize is either 1 or 0 (0 iff abs(op1) == abs(op2)).
                const mpz_size_t s = (tmp != 0u);
                rop._mp_size = (sign1 == 1) ? s : -s;
                rdata[0] = tmp;
            } else {
                const auto tmp = data2[0] - data1[0];
                // NOTE: this has to be one, as data2[0] and data1[0] cannot be equal.
                rop._mp_size = sign2;
                rdata[0] = tmp;
            }
            return 0;
        }
    }
    // Optimization for two-limbs statics with no nails.
    // Small helper to compare two statics of equal asize 1 or 2.
    static int compare_limbs_2(const ::mp_limb_t *data1, const ::mp_limb_t *data2, mpz_size_t asize)
    {
        // NOTE: this requires no nail bits.
        assert(!GMP_NAIL_BITS);
        assert(asize == 1 || asize == 2);
        // Start comparing from the top.
        auto cmp_idx = asize - 1;
        if (data1[cmp_idx] != data2[cmp_idx]) {
            return data1[cmp_idx] > data2[cmp_idx] ? 1 : -1;
        }
        // The top limbs are equal, move down one limb.
        // If we are already at the bottom limb, it means the two numbers are equal.
        if (!cmp_idx) {
            return 0;
        }
        --cmp_idx;
        if (data1[cmp_idx] != data2[cmp_idx]) {
            return data1[cmp_idx] > data2[cmp_idx] ? 1 : -1;
        }
        return 0;
    }
    static int static_add_impl(s_int &rop, ::mp_limb_t *rdata, const ::mp_limb_t *data1, mpz_size_t size1,
                               mpz_size_t asize1, int sign1, const ::mp_limb_t *data2, mpz_size_t size2,
                               mpz_size_t asize2, int sign2, const std::integral_constant<int, 2> &)
    {
        // abs sizes must be 1 or 2.
        assert(asize1 <= 2 && asize2 <= 2 && asize1 > 0 && asize2 > 0);
        const unsigned size_mask = (unsigned)(asize1 - 1) + ((unsigned)(asize2 - 1) << 1u);
        if (sign1 == sign2) {
            // When the signs are identical, we can implement addition as a true addition.
            switch (size_mask) {
                case 0u: {
                    // Both sizes are 1. This can never fail, and at most bumps the asize to 2.
                    const auto lo = data1[0] + data2[0];
                    const auto hi = static_cast<::mp_limb_t>(lo < data1[0]);
                    rop._mp_size = (sign1 == 1) ? static_cast<mpz_size_t>(hi + 1u) : -static_cast<mpz_size_t>(hi + 1u);
                    rdata[0] = lo;
                    rdata[1] = hi;
                    return 0;
                }
                case 1u: {
                    // asize1 is 2, asize2 is 1. Result could have 3 limbs.
                    const auto lo = data1[0] + data2[0];
                    const auto hi = data1[1] + static_cast<::mp_limb_t>(lo < data1[0]);
                    rop._mp_size = sign1 + sign1;
                    rdata[0] = lo;
                    rdata[1] = hi;
                    return static_cast<int>(hi == 0u);
                }
                case 2u: {
                    // asize1 is 1, asize2 is 2. Result could have 3 limbs.
                    const auto lo = data1[0] + data2[0];
                    const auto hi = data2[1] + static_cast<::mp_limb_t>(lo < data1[0]);
                    rop._mp_size = sign1 + sign1;
                    rdata[0] = lo;
                    rdata[1] = hi;
                    return static_cast<int>(hi == 0u);
                }
                case 3u: {
                    // Both have asize 2.
                    // The rop hi limb might spill over either from the addition of the hi limbs
                    // of op1 and op2, or by the addition of carry coming over from the addition of
                    // the lo limbs of op1 and op2.
                    const auto lo = data1[0] + data2[0];
                    const auto hi1 = data1[1] + data2[1];
                    const auto cy_lo = static_cast<::mp_limb_t>(lo < data1[0]);
                    const auto cy_hi1 = static_cast<::mp_limb_t>(hi1 < data1[1]);
                    const auto hi2 = hi1 + cy_lo;
                    const auto cy_hi2 = static_cast<::mp_limb_t>(hi2 == 0u);
                    rop._mp_size = sign1 + sign1;
                    rdata[0] = lo;
                    rdata[1] = hi2;
                    return static_cast<int>(cy_hi1 || cy_hi2);
                }
            }
        } else {
            // When the signs differ, we need to implement addition as a subtraction.
            if (asize1 > asize2 || (asize1 == asize2 && compare_limbs_2(data1, data2, asize1) >= 0)) {
                // op1 is >= op2 in absolute value.
                switch (size_mask) {
                    case 0u: {
                        const auto tmp = data1[0] - data2[0];
                        // asize is either 1 or 0 (0 iff abs(op1) == abs(op2)).
                        const mpz_size_t s = (tmp != 0u);
                        rop._mp_size = (sign1 == 1) ? s : -s;
                        rdata[0] = tmp;
                        return 0;
                    }
                    case 1u: {
                        // asize1 is 2, asize2 is 1.
                        const auto lo = data1[0] - data2[0];
                        // Subtract the borrow, if any, from the top limb.
                        const auto hi = data1[1] - static_cast<::mp_limb_t>(data1[0] < data2[0]);
                        // The asize can be 2 or 1. It will be 1 if the new hi limb is 0.
                        rop._mp_size = (sign1 == 1) ? (2 - (hi == 0u)) : -(2 - (hi == 0u));
                        rdata[0] = lo;
                        rdata[1] = hi;
                        return 0;
                    }
                    case 2u:
                        // NOTE: this is not possible as it corresponds to asize1 1, asize2 2, but here we know
                        // that asize1 >= asize2.
                        assert(false);
                    case 3u:
                        // Both sizes are 2.
                        const auto lo = data1[0] - data2[0];
                        // If there's a borrow, then hi1 > hi2, otherwise we would have a negative result.
                        assert(data1[0] >= data2[0] || data1[1] > data2[1]);
                        // This can never wrap around, at most it goes to zero.
                        const auto hi = data1[1] - data2[1] - static_cast<::mp_limb_t>(data1[0] < data2[0]);
                        // asize can be 0, 1 or 2.
                        const mpz_size_t s = (hi == 0u) ? (lo == 0u ? 0 : 1) : 2;
                        rop._mp_size = (sign1 == 1) ? s : -s;
                        rdata[0] = lo;
                        rdata[1] = hi;
                        return 0;
                }
            } else {
                // op2 is > op1 in absolute value.
                switch (size_mask) {
                    case 0u: {
                        const auto tmp = data2[0] - data1[0];
                        // NOTE: this has to be one, as data2[0] and data1[0] cannot be equal.
                        rop._mp_size = sign2;
                        rdata[0] = tmp;
                        return 0;
                    }
                    case 1u:
                        assert(false);
                    case 2u: {
                        // asize1 is 1, asize2 is 2.
                        const auto lo = data2[0] - data1[0];
                        const auto hi = data2[1] - static_cast<::mp_limb_t>(data2[0] < data1[0]);
                        rop._mp_size = (sign2 == 1) ? (2 - (hi == 0u)) : -(2 - (hi == 0u));
                        rdata[0] = lo;
                        rdata[1] = hi;
                        return 0;
                    }
                    case 3u:
                        // Both sizes are 2.
                        const auto lo = data2[0] - data1[0];
                        assert(data2[0] >= data1[0] || data2[1] > data1[1]);
                        const auto hi = data2[1] - data1[1] - static_cast<::mp_limb_t>(data2[0] < data1[0]);
                        // asize can be 1 or 2, but not zero as we know abs(op1) != abs(op2).
                        rop._mp_size = (sign2 == 1) ? (1 + (hi != 0u)) : -(1 + (hi != 0u));
                        rdata[0] = lo;
                        rdata[1] = hi;
                        return 0;
                }
            }
        }
        // We should never get here.
        assert(false);
    }
    template <bool AddOrSub>
    static int static_addsub(s_int &rop, const s_int &op1, const s_int &op2)
    {
        // Cache a few quantities.
        const auto size1 = op1._mp_size, size2 = op2._mp_size;
        // NOTE: effectively negate op2 if we are subtracting.
        mpz_size_t asize1 = size1, asize2 = AddOrSub ? size2 : -size2;
        int sign1 = 1, sign2 = 1;
        if (asize1 < 0) {
            asize1 = -asize1;
            sign1 = -1;
        }
        if (asize2 < 0) {
            asize2 = -asize2;
            sign2 = -1;
        }
        ::mp_limb_t *rdata = rop.m_limbs.data();
        const ::mp_limb_t *data1 = op1.m_limbs.data(), *data2 = op2.m_limbs.data();
        // Handle the case in which at least one operand is zero.
        if (!size2) {
            // Second op is zero, copy over the first op.
            rop._mp_size = size1;
            copy_limbs(data1, data1 + asize1, rdata);
            return 0;
        }
        if (!size1) {
            // First op is zero, copy over the second op, flipping sign in case of subtraction.
            rop._mp_size = AddOrSub ? size2 : -size2;
            copy_limbs(data2, data2 + asize2, rdata);
            return 0;
        }
        return static_add_impl(rop, rdata, data1, size1, asize1, sign1, data2, size2, asize2, sign2,
                               static_add_algo<s_int>{});
    }

public:
    friend void add(mp_integer &rop, const mp_integer &op1, const mp_integer &op2)
    {
        const unsigned mask
            = (unsigned)!rop.is_static() + ((unsigned)!op1.is_static() << 1u) + ((unsigned)!op2.is_static() << 2u);
        // Implementation of the static addition (all 3 operands are static). We write it here as a functor because
        // we use it more than once in the switch below.
        auto static_adder = [&]() {
            auto fail = static_addsub<true>(rop.m_int.g_st(), op1.m_int.g_st(), op2.m_int.g_st());
            if (fail) {
                // Now rop contains the lower limbs of the result. We need to promote it and add the carry.
                // Init tmp mpz with an adequate number of limbs.
                mpz_struct_t tmp;
                // TODO overflow check, static.
                ::mpz_init2(&tmp,GMP_NUMB_BITS * (SSize + 1u));
                // Copy over from rop.
                tmp._mp_size = rop.m_int.g_st()._mp_size > 0 ? static_cast<mpz_size_t>(SSize + 1) :
                        -static_cast<mpz_size_t>(SSize + 1);
                copy_limbs(rop.m_int.g_st().m_limbs.data(), rop.m_int.g_st().m_limbs.data() + SSize + 1u,
                    tmp._mp_d);
                // Write the carry.
                tmp._mp_d[SSize] = 1;
                // Destroy the static, init dynamic and copy it.
                rop.m_int.g_st().~s_int();
                ::new (static_cast<void *>(&rop.m_int.m_dy)) mpz_struct_t;
                mpz_shallow_copy(rop.m_int.m_dy, tmp);
            }
        };
        switch (mask) {
            case 0u:
                // All 3 static.
                static_adder();
                break;
            case 1u:
                // rop is dynamic, the others are static. Demote rop and do the static add.
                rop.m_int.reset_to_static();
                static_adder();
                break;
            case 2u:
                // rop static, op1 dynamic, op2 static. We fall through the case 3u after promoting rop.
                rop.m_int.promote();
            case 3u:
                // rop and op1 dynamic, op2 static.
                ::mpz_add(&rop.m_int.g_dy(),&op1.m_int.g_dy(),op2.m_int.g_st().get_mpz_view());
                break;
            case 4u:
                // rop and op1 static, op2 dynamic.
                rop.m_int.promote();
            case 5u:
                // rop dynamic, op1 static, op2 dynamic.
                ::mpz_add(&rop.m_int.g_dy(),op1.m_int.g_st().get_mpz_view(),&op2.m_int.g_dy());
                break;
            case 6u:
                // rop static, op1 and op2 dynamic.
                rop.m_int.promote();
            case 7u:
                // All 3 dynamic.
                ::mpz_add(&rop.m_int.g_dy(),&op1.m_int.g_dy(),&op2.m_int.g_dy());
        }
    }
#if 0
    static int static_mul(s_int &rop, const s_int &op1, const s_int &op2)
    {
        // Cache a few quantities.
        const auto size1 = op1._mp_size, size2 = op2._mp_size;
        auto asize1 = size1, asize2 = size2;
        bool sign1 = true, sign2 = true;
        if (asize1 < 0) {
            asize1 = -asize1;
            sign1 = false;
        }
        if (asize2 < 0) {
            asize2 = -asize2;
            sign2 = false;
        }
        ::mp_limb_t *rdata = rop.m_limbs.data();
        const ::mp_limb_t *data1 = op1.m_limbs.data(), *data2 = op2.m_limbs.data();
        // Handle the case in which at least one operand is zero.
        if (!size1 || !size2) {
            rop._mp_size = 0;
            return 0;
        }
        using dlimb_t = dlimb_available<::mp_limb_t>::type;
        const unsigned size_mask = (unsigned)(asize1 - 1) + ((unsigned)(asize2 - 1) << 1u);
        switch (size_mask) {
            case 0u: {
                const auto lo = static_cast<dlimb_t>(static_cast<dlimb_t>(data1[0u]) * data2[0u]);
                rdata[0u] = static_cast<::mp_limb_t>(lo);
                const auto cy_limb = static_cast<::mp_limb_t>(lo >> GMP_LIMB_BITS);
                rdata[1u] = cy_limb;
                auto new_size = static_cast<mpz_size_t>((asize1 + asize2) - mpz_size_t(cy_limb == 0u));
                if (sign1 != sign2) {
                    new_size = -new_size;
                }
                rop._mp_size = new_size;
                return 0;
            }
        }
        throw;
    }
    friend void mul(mp_integer &rop, const mp_integer &op1, const mp_integer &op2)
    {
        const unsigned mask
            = (unsigned)!rop.is_static() + ((unsigned)!op1.is_static() << 1u) + ((unsigned)!op2.is_static() << 2u);
        switch (mask) {
            case 0u:
                // All 3 statics.
                auto fail = static_mul(rop.m_int.g_st(), op1.m_int.g_st(), op2.m_int.g_st());
                if (fail) {
                    std::cout << "Adadsa\n";
                }
                return;
        }
        throw;
    }
#endif

private:
    integer_union<SSize> m_int;
};
}

#endif
