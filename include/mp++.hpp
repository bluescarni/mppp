// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_MPPP_HPP
#define MPPP_MPPP_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <gmp.h>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

#if __GNU_MP_VERSION < 5

#error Minimum supported GMP version is 5.

#endif

#if defined(MPPP_WITH_LONG_DOUBLE)

#include <mpfr.h>

#if MPFR_VERSION_MAJOR < 3

#error Minimum supported MPFR version is 3.

#endif

#endif

// Compiler configuration.
// NOTE: check for MSVC first, as clang-cl does defined both __clang__ and _MSC_VER,
// and we want to configure it as MSVC.
#if defined(_MSC_VER)

// Disable clang-format here, as the include order seems to matter.
// clang-format off
#include <windows.h>
#include <Winnt.h>
// clang-format on

// clang-cl supports __builtin_expect().
#if defined(__clang__)
#define mppp_likely(x) __builtin_expect(!!(x), 1)
#define mppp_unlikely(x) __builtin_expect(!!(x), 0)
#else
#define mppp_likely(x) (x)
#define mppp_unlikely(x) (x)
#endif
#define MPPP_RESTRICT __restrict

// Disable some warnings for MSVC.
#pragma warning(push)
#pragma warning(disable : 4127)
#pragma warning(disable : 4146)
#pragma warning(disable : 4804)

// Checked iterators functionality.
#include <iterator>

#elif defined(__clang__) || defined(__GNUC__) || defined(__INTEL_COMPILER)

#define mppp_likely(x) __builtin_expect(!!(x), 1)
#define mppp_unlikely(x) __builtin_expect(!!(x), 0)
#define MPPP_RESTRICT __restrict

// NOTE: we can check int128 on GCC/clang with __SIZEOF_INT128__ apparently:
// http://stackoverflow.com/questions/21886985/what-gcc-versions-support-the-int128-intrinsic-type
#if defined(__SIZEOF_INT128__)
#define MPPP_UINT128 __uint128_t
#endif

#else

#define mppp_likely(x) (x)
#define mppp_unlikely(x) (x)
#define MPPP_RESTRICT

#endif

// Configuration of the thread_local keyword.
#if defined(__apple_build_version__) || defined(__MINGW32__) || defined(__INTEL_COMPILER)

// - Apple clang does not support the thread_local keyword until very recent versions.
// - Testing shows that at least some MinGW versions have buggy thread_local implementations.
// - Also on Intel the thread_local keyword looks buggy.
#define MPPP_MAYBE_TLS

#else

// For the rest, we assume thread_local is available.
#define MPPP_HAVE_THREAD_LOCAL
#define MPPP_MAYBE_TLS static thread_local

#endif

/// The root mp++ namespace.
namespace mppp
{

namespace mppp_impl
{

// A bunch of useful utilities from C++14/C++17.

// http://en.cppreference.com/w/cpp/types/void_t
template <typename... Ts>
struct make_void {
    typedef void type;
};

template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

// http://en.cppreference.com/w/cpp/experimental/is_detected
template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

// http://en.cppreference.com/w/cpp/experimental/nonesuch
struct nonesuch {
    nonesuch() = delete;
    ~nonesuch() = delete;
    nonesuch(nonesuch const &) = delete;
    void operator=(nonesuch const &) = delete;
};

template <template <class...> class Op, class... Args>
using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

// http://en.cppreference.com/w/cpp/types/conjunction
template <class...>
struct conjunction : std::true_type {
};

template <class B1>
struct conjunction<B1> : B1 {
};

template <class B1, class... Bn>
struct conjunction<B1, Bn...> : std::conditional<B1::value != false, conjunction<Bn...>, B1>::type {
};

// http://en.cppreference.com/w/cpp/types/disjunction
template <class...>
struct disjunction : std::false_type {
};

template <class B1>
struct disjunction<B1> : B1 {
};

template <class B1, class... Bn>
struct disjunction<B1, Bn...> : std::conditional<B1::value != false, B1, disjunction<Bn...>>::type {
};

// http://en.cppreference.com/w/cpp/types/negation
template <class B>
struct negation : std::integral_constant<bool, !B::value> {
};

// mpz_t is an array of some struct.
using mpz_struct_t = std::remove_extent<::mpz_t>::type;
// Integral types used for allocation size and number of limbs.
using mpz_alloc_t = decltype(std::declval<mpz_struct_t>()._mp_alloc);
using mpz_size_t = decltype(std::declval<mpz_struct_t>()._mp_size);

// Some misc tests to check that the mpz struct conforms to our expectations.
// This is crucial for the implementation of the union integer type.
struct expected_mpz_struct_t {
    mpz_alloc_t _mp_alloc;
    mpz_size_t _mp_size;
    ::mp_limb_t *_mp_d;
};

static_assert(sizeof(expected_mpz_struct_t) == sizeof(mpz_struct_t) && std::is_standard_layout<mpz_struct_t>::value
                  && std::is_standard_layout<expected_mpz_struct_t>::value && offsetof(mpz_struct_t, _mp_alloc) == 0u
                  && offsetof(mpz_struct_t, _mp_size) == offsetof(expected_mpz_struct_t, _mp_size)
                  && offsetof(mpz_struct_t, _mp_d) == offsetof(expected_mpz_struct_t, _mp_d)
                  && std::is_same<::mp_limb_t *, decltype(std::declval<mpz_struct_t>()._mp_d)>::value &&
                  // mp_bitcnt_t is used in shift operators, so we double-check it is unsigned. If it is not
                  // we might end up shifting by negative values, which is UB.
                  std::is_unsigned<::mp_bitcnt_t>::value &&
                  // The mpn functions accept sizes as ::mp_size_t, but we generally represent sizes as mpz_size_t.
                  // We need then to make sure we can always cast safely mpz_size_t to ::mp_size_t. Inspection
                  // of the gmp.h header seems to indicate that mpz_size_t is never larger than ::mp_size_t.
                  std::numeric_limits<mpz_size_t>::min() >= std::numeric_limits<::mp_size_t>::min()
                  && std::numeric_limits<mpz_size_t>::max() <= std::numeric_limits<::mp_size_t>::max(),
              "Invalid mpz_t struct layout and/or GMP types.");

// Helper function to init an mpz to zero with nlimbs preallocated limbs.
inline void mpz_init_nlimbs(mpz_struct_t &rop, std::size_t nlimbs)
{
    // A bit of horrid overflow checking.
    if (mppp_unlikely(nlimbs > std::numeric_limits<std::size_t>::max() / unsigned(GMP_NUMB_BITS))) {
        // NOTE: here we are doing what GMP does in case of memory allocation errors. It does not make much sense
        // to do anything else, as long as GMP does not provide error recovery.
        std::abort(); // LCOV_EXCL_LINE
    }
    const auto nbits = static_cast<std::size_t>(unsigned(GMP_NUMB_BITS) * nlimbs);
    if (mppp_unlikely(nbits > std::numeric_limits<::mp_bitcnt_t>::max())) {
        std::abort(); // LCOV_EXCL_LINE
    }
    // NOTE: nbits == 0 is allowed.
    ::mpz_init2(&rop, static_cast<::mp_bitcnt_t>(nbits));
    assert(std::make_unsigned<mpz_size_t>::type(rop._mp_alloc) >= nlimbs);
}

// Combined init+set.
inline void mpz_init_set_nlimbs(mpz_struct_t &m0, const mpz_struct_t &m1)
{
    mpz_init_nlimbs(m0, ::mpz_size(&m1));
    ::mpz_set(&m0, &m1);
}

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

// Convert an mpz to a string in a specific base, to be written into out.
inline void mpz_to_str(std::vector<char> &out, const mpz_struct_t *mpz, int base = 10)
{
    assert(base >= 2 && base <= 62);
    const auto size_base = ::mpz_sizeinbase(mpz, base);
    if (mppp_unlikely(size_base > std::numeric_limits<std::size_t>::max() - 2u)) {
        throw std::overflow_error("Too many digits in the conversion of mpz_t to string."); // LCOV_EXCL_LINE
    }
    // Total max size is the size in base plus an optional sign and the null terminator.
    const auto total_size = size_base + 2u;
    // NOTE: possible improvement: use a null allocator to avoid initing the chars each time
    // we resize up.
    out.resize(static_cast<std::vector<char>::size_type>(total_size));
    // Overflow check.
    if (mppp_unlikely(out.size() != total_size)) {
        throw std::overflow_error("Too many digits in the conversion of mpz_t to string."); // LCOV_EXCL_LINE
    }
    ::mpz_get_str(out.data(), base, mpz);
}

// Convenience overload for the above.
inline std::string mpz_to_str(const mpz_struct_t *mpz, int base = 10)
{
    MPPP_MAYBE_TLS std::vector<char> tmp;
    mpz_to_str(tmp, mpz, base);
    return tmp.data();
}

// Type trait to check if T is a supported integral type.
template <typename T>
using is_supported_integral
    = std::integral_constant<bool, disjunction<std::is_same<T, bool>, std::is_same<T, char>,
                                               std::is_same<T, signed char>, std::is_same<T, unsigned char>,
                                               std::is_same<T, short>, std::is_same<T, unsigned short>,
                                               std::is_same<T, int>, std::is_same<T, unsigned>, std::is_same<T, long>,
                                               std::is_same<T, unsigned long>, std::is_same<T, long long>,
                                               std::is_same<T, unsigned long long>>::value>;

// Type trait to check if T is a supported floating-point type.
template <typename T>
using is_supported_float = std::integral_constant<bool, disjunction<std::is_same<T, float>, std::is_same<T, double>
#if defined(MPPP_WITH_LONG_DOUBLE)
                                                                    ,
                                                                    std::is_same<T, long double>
#endif
                                                                    >::value>;

template <typename T>
using is_supported_interop
    = std::integral_constant<bool, disjunction<is_supported_integral<T>, is_supported_float<T>>::value>;

// Small wrapper to copy limbs.
inline void copy_limbs(const ::mp_limb_t *begin, const ::mp_limb_t *end, ::mp_limb_t *out)
{
    for (; begin != end; ++begin, ++out) {
        *out = *begin;
    }
}

// The version with non-overlapping ranges.
inline void copy_limbs_no(const ::mp_limb_t *begin, const ::mp_limb_t *end, ::mp_limb_t *MPPP_RESTRICT out)
{
    assert(begin != out);
    for (; begin != end; ++begin, ++out) {
        *out = *begin;
    }
}

// Add a and b, store the result in res, and return 1 if there's unsigned overflow, 0 othersize.
// NOTE: recent GCC versions have builtins for this, but they don't seem to make much of a difference:
// https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html
inline ::mp_limb_t limb_add_overflow(::mp_limb_t a, ::mp_limb_t b, ::mp_limb_t *res)
{
    *res = a + b;
    return *res < a;
}

// The static integer class.
template <std::size_t SSize>
struct static_int {
    // Just a small helper, like C++14.
    template <bool B, typename T = void>
    using enable_if_t = typename std::enable_if<B, T>::type;
    // Let's put a hard cap and sanity check on the static size.
    static_assert(SSize > 0u && SSize <= 64u, "Invalid static size.");
    using limbs_type = std::array<::mp_limb_t, SSize>;
    // Cast it to mpz_size_t for convenience.
    static const mpz_size_t s_size = SSize;
    // Special alloc value to signal static storage in the union.
    static const mpz_alloc_t s_alloc = -1;
    // The largest number of limbs for which special optimisations are activated.
    // This is important because the arithmetic optimisations rely on the unused limbs
    // being zero, thus whenever we use mpn functions on a static int we need to
    // take care of ensuring that this invariant is respected (see dtor_checks() and
    // zero_unused_limbs(), for instance).
    static const std::size_t opt_size = 2;
    // NOTE: init limbs to zero: in some few-limbs optimisations we operate on the whole limb
    // array regardless of the integer size, for performance reasons. If we didn't init to zero,
    // we would read from uninited storage and we would have wrong results as well.
    static_int() : _mp_alloc(s_alloc), _mp_size(0), m_limbs()
    {
    }
    // The defaults here are good.
    static_int(const static_int &) = default;
    static_int(static_int &&) = default;
    static_int &operator=(const static_int &) = default;
    static_int &operator=(static_int &&) = default;
    bool dtor_checks() const
    {
        // LCOV_EXCL_START
        const auto asize = abs_size();
        // Check the value of the alloc member.
        if (_mp_alloc != s_alloc) {
            return false;
        }
        // Check the asize is not too large.
        if (asize > s_size) {
            return false;
        }
        // Check that all limbs which do not participate in the value are zero, iff the SSize is small enough.
        // For small SSize, we might be using optimisations that require unused limbs to be zero.
        if (SSize <= opt_size) {
            for (auto i = static_cast<std::size_t>(asize); i < SSize; ++i) {
                if (m_limbs[i]) {
                    return false;
                }
            }
        }
        // Check that the highest limb is not zero.
        if (asize > 0 && (m_limbs[static_cast<std::size_t>(asize - 1)] & GMP_NUMB_MASK) == 0u) {
            return false;
        }
        return true;
        // LCOV_EXCL_STOP
    }
    ~static_int()
    {
        assert(dtor_checks());
    }
    // Zero the limbs that are not used for representing the value.
    // This is normally not needed, but it is useful when using the GMP mpn api on a static int:
    // the GMP api does not clear unused limbs, but we rely on unused limbs being zero when optimizing operations
    // for few static limbs.
    void zero_unused_limbs()
    {
        // Don't do anything if the static size is larger that the maximum size for which the
        // few-limbs optimisations are activated.
        if (SSize <= opt_size) {
            for (auto i = static_cast<std::size_t>(abs_size()); i < SSize; ++i) {
                m_limbs[i] = 0u;
            }
        }
    }
    // Size in limbs (absolute value of the _mp_size member).
    mpz_size_t abs_size() const
    {
        return _mp_size >= 0 ? _mp_size : -_mp_size;
    }
    struct static_mpz_view {
        // NOTE: this is needed when we have the variant view in the integer class: if the active view
        // is the dynamic one, we need to def construct a static view that we will never use.
        // NOTE: m_mpz needs to be zero inited because otherwise when using the move ctor we will
        // be reading from uninited memory.
        // NOTE: use round parentheses here in an attempt to shut a GCC warning that happens with curly
        // braces - they should be equivalent, see:
        // http://en.cppreference.com/w/cpp/language/value_initialization
        static_mpz_view() : m_mpz()
        {
        }
        // NOTE: we use the const_cast to cast away the constness from the pointer to the limbs
        // in n. This is valid as we are never going to use this pointer for writing.
        // NOTE: in recent GMP versions there are functions to accomplish this type of read-only init
        // of an mpz:
        // https://gmplib.org/manual/Integer-Special-Functions.html#Integer-Special-Functions
        explicit static_mpz_view(const static_int &n)
            : m_mpz{s_size, n._mp_size, const_cast<::mp_limb_t *>(n.m_limbs.data())}
        {
        }
        static_mpz_view(static_mpz_view &&) = default;
        static_mpz_view(const static_mpz_view &) = delete;
        static_mpz_view &operator=(const static_mpz_view &) = delete;
        static_mpz_view &operator=(static_mpz_view &&) = delete;
        operator const mpz_struct_t *() const
        {
            return &m_mpz;
        }
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
            mpz_init_set_nlimbs(m_dy, other.g_dy());
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
            // NOTE: this copies the mpz struct members (shallow copy).
            m_dy = other.g_dy();
            // Downgrade the other to an empty static.
            other.g_dy().~d_storage();
            ::new (static_cast<void *>(&other.m_st)) s_storage();
        }
    }
    // Copy assignment operator, performs a deep copy maintaining the storage class.
    integer_union &operator=(const integer_union &other)
    {
        if (mppp_unlikely(this == &other)) {
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
            mpz_init_set_nlimbs(m_dy, other.g_dy());
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
        if (mppp_unlikely(this == &other)) {
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
            m_dy = other.g_dy();
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
        g_dy().~d_storage();
    }
    // Check storage type.
    bool is_static() const
    {
        return m_st._mp_alloc == s_storage::s_alloc;
    }
    bool is_dynamic() const
    {
        return m_st._mp_alloc != s_storage::s_alloc;
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
    // Promotion from static to dynamic. If nlimbs != 0u, allocate nlimbs limbs, otherwise
    // allocate exactly the nlimbs necessary to represent this.
    void promote(std::size_t nlimbs = 0u)
    {
        assert(is_static());
        mpz_struct_t tmp_mpz;
        auto v = g_st().get_mpz_view();
        if (nlimbs == 0u) {
            // If nlimbs is zero, we will allocate exactly the needed
            // number of limbs to represent this.
            mpz_init_set_nlimbs(tmp_mpz, *v);
        } else {
            // Otherwise, we preallocate nlimbs and then set tmp_mpz
            // to the value of this.
            mpz_init_nlimbs(tmp_mpz, nlimbs);
            ::mpz_set(&tmp_mpz, v);
        }
        // Destroy static.
        g_st().~s_storage();
        // Construct the dynamic struct.
        ::new (static_cast<void *>(&m_dy)) d_storage;
        m_dy = tmp_mpz;
    }
    // Demotion from dynamic to static.
    bool demote()
    {
        assert(is_dynamic());
        const auto dyn_size = ::mpz_size(&g_dy());
        // If the dynamic size is greater than the static size, we cannot demote.
        if (dyn_size > SSize) {
            return false;
        }
        // Copy over the limbs to temporary storage.
        std::array<::mp_limb_t, SSize> tmp;
        copy_limbs_no(g_dy()._mp_d, g_dy()._mp_d + dyn_size, tmp.data());
        const auto signed_size = g_dy()._mp_size;
        // Destroy the dynamic storage.
        destroy_dynamic();
        // Init the static storage and copy over the data.
        // NOTE: here the ctor makes sure the static limbs are zeroed
        // out and we don't get stray limbs when copying below.
        ::new (static_cast<void *>(&m_st)) s_storage();
        g_st()._mp_size = signed_size;
        copy_limbs_no(tmp.data(), tmp.data() + dyn_size, g_st().m_limbs.data());
        return true;
    }
    // NOTE: keep these public as we need them below.
    s_storage m_st;
    d_storage m_dy;
};
}

/// Exception to signal division by zero.
/**
 * This exception inherits all members (including constructors) from \p std::domain_error. It will be thrown
 * when a division by zero involving an mp_integer is attempted.
 */
struct zero_division_error final : std::domain_error {
    using std::domain_error::domain_error;
};

// NOTE: a few misc things:
// - re-visit at one point the issue of the estimators when we need to promote from static to dynamic
//   in arithmetic ops. Currently they are not 100% optimal since they rely on the information coming out
//   of the static implementation rather than computing the estimated size of rop themselves, for performance
//   reasons (see comments);
// - maybe the lshift/rshift optimisation for 2 limbs could use dlimb types if available?
// - it seems like it might be possible to re-implement a bare-bone mpz api on top of the mpn primitives,
//   with the goal of providing some form of error recovery in case of memory errors (e.g., throw instead
//   of aborting). This is probably not an immediate concern though.
// - pow() can probably benefit for some specialised static implementation, especially in conjunction with
//   mpn_sqr().
// - gcd() can be improved (see notes).
// - the pattern we follow when no mpn primitives are available is to use a static thread local instance of mpz_t
//   to perform the computation with mpz_ functions and then assign the result to rop. We could probably benefit
//   from a true assignment operator from mpz_t instead of cting an integer from mpz_t and assigning it (which
//   is what we are doing now).
/// Multiprecision integer class.
/**
 * This class represent arbitrary-precision signed integers. It acts as a wrapper around the GMP \p mpz_t type, with
 * a small value optimisation: integers whose size is up to \p SSize limbs are stored directly in the storage
 * occupied by the mp_integer object, without resorting to dynamic memory allocation. The value of \p SSize
 * must be at least 1 and less than an implementation-defined upper limit.
 *
 * When the value of an mp_integer is stored directly within the object, the <em>storage type</em> of the integer is
 * said to be <em>static</em>. When the limb size of the integer exceeds the maximum value \p SSize, the storage types
 * becomes <em>dynamic</em>. The transition from static to dynamic storage happens transparently whenever the integer
 * value becomes large enough. The demotion from dynamic to static storage usually needs to be requested explicitly.
 * For values of \p SSize of 1 and 2, optimised implementations of basic arithmetic operations are employed,
 * if supported by the target architecture and if the storage type is static. For larger values of \p SSize,
 * the \p mpn_ low-level functions of the GMP API are used if the storage type is static. If the storage type is
 * dynamic, the usual \p mpz_ functions from the GMP API are used.
 *
 * # Interoperable types #
 * This class has the look and feel of a C++ builtin type: it can interact with most of C++'s integral and
 * floating-point
 * primitive types, and it provides overloaded arithmetic operators. Differently from the builtin types, however, this
 * class does not allow any implicit conversion to/from other types (apart from \p bool): construction from and
 * conversion to primitive types must always be requested explicitly. As a side effect, syntax such as
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c++
 *
 *    mp_integer<1> n = 5;
 *    int m = n;
 * \endverbatim
 * will not work, and direct initialization and explicit casting should be used instead:
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c++
 *
 *    mp_integer<1> n{5};
 *    int m = static_cast<int>(n);
 * \endverbatim
 * The full list of interoperable builtin types is:
 * - \p bool,
 * - \p char, <tt>signed char</tt> and <tt>unsigned char</tt>,
 * - \p short and <tt>unsigned short</tt>,
 * - \p int and \p unsigned,
 * - \p long and <tt>unsigned long</tt>,
 * - <tt>long long</tt> and <tt>unsigned long long</tt>,
 * - \p float, \p double and <tt>long double</tt> (<tt>long double</tt> requires the MPFR library).
 *
 * # API #
 * Most of the functionality of this class is exposed via inline friend functions, with the general convention
 * that the functions are named after the corresponding GMP functions minus the leading \p mpz_ prefix. For instance,
 * the GMP call
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c++
 *
 *    mpz_add(rop,a,b);
 * \endverbatim
 * that writes the result of <tt>a + b</tt> into \p rop becomes simply
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c++
 *
 *    add(rop,a,b);
 * \endverbatim
 * where the add() function is resolved via argument-dependent lookup. Function calls with overlapping arguments
 * are allowed, unless noted otherwise.
 *
 * Multiple overloads of the same functionality are often available.
 * Binary functions in GMP are usually implemented via three-arguments functions, in which the first
 * argument is a reference to the return value. The exponentiation function \p mpz_pow_ui(), for instance,
 * takes three arguments: the return value, the base and the exponent. There are two overloads of the corresponding
 * pow_ui() function:
 * - a ternary overload semantically equivalent to \p mpz_pow_ui(),
 * - a binary overload taking as inputs the base and the exponent, and returning the result
 *   of the exponentiation.
 *
 * This allows to avoid having to set up a return value for one-off invocations of pow_ui() (the binary overload will
 * do it for you). For example:
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c++
 *
 *    mp_integer<1> r1, r2, n{3};
 *    pow_ui(r1,n,2);   // Ternary pow_ui(): computes n**2 and stores
 *                      // the result in r1.
 *    r2 = pow_ui(n,2); // Binary pow_ui(): returns n**2, which is then
 *                      // assigned to r2.
 * \endverbatim
 *
 * In case of unary functions, there are often three overloads available:
 * - a binary overload taking as first parameter a reference to the return value (GMP style),
 * - a unary overload returning the result of the operation,
 * - a nullary member function that modifies the calling object in-place.
 *
 * For instance, here are three possible ways of computing the absolute value:
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c++
 *
 *    mp_integer<1> r1, r2, n{-5};
 *    abs(r1,n);   // Binary abs(): computes and stores the absolute value
 *                 // of n into r1.
 *    r2 = abs(n); // Unary abs(): returns the absolute value of n, which is
 *                 // then assigned to r2.
 *    n.abs();     // Member function abs(): replaces the value of n with its
 *                 // absolute value.
 * \endverbatim
 * Note that at this time only a small subset of the GMP API has been wrapped by mp_integer.
 *
 * # Overloaded operators #
 *
 * This class provides overloaded operators for the basic arithmetic operations, including bit shifting.
 * The binary operators are implemented as inline friend functions, the in-place operators are implemented as
 * member functions. The overloaded operators are resolved via argument-dependent lookup whenever at least
 * one argument is of type mp_integer, and the other argument is either another mp_integer or an instance
 * of an interoperable type.
 *
 * For the common arithmetic operations (\p +, \p -, \p * and \p /), the type promotion
 * rules are a natural extension of the corresponding rules for native C++ types: if the other argument
 * is a C++ integral, the result will be of type mp_integer, if the other argument is a C++ floating-point the result
 * will be of the same floating-point type. For example:
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c++
 *
 *    mp_integer<1> n1{1}, n2{2};
 *    auto res1 = n1 + n2; // res1 is an mp_integer
 *    auto res2 = n1 * 2; // res2 is an mp_integer
 *    auto res3 = 2 - n2; // res3 is an mp_integer
 *    auto res4 = n1 / 2.f; // res4 is a float
 *    auto res5 = 12. / n1; // res5 is a double
 * \endverbatim
 *
 * The modulo operator \p % accepts only mp_integer and interoperable integral types as arguments, and it always returns
 * mp_integer as result. The bit shifting operators \p << and \p >> accept only interoperable integral types as
 * shift arguments, and they always return mp_integer as result.
 *
 * The relational operators, \p ==, \p !=, \p <, \p >, \p <= and \p >= will promote the arguments to a common type
 * before
 * comparing them. The promotion rules are the same as in the arithmetic operators (that is, both arguments are
 * promoted to mp_integer if they are both integral types, otherwise they are promoted to the type of the floating-point
 * argument).
 *
 * # Interfacing with GMP #
 *
 * This class provides facilities to interface with the GMP library. Specifically, mp_integer features:
 * - a constructor from the GMP integer type \p mpz_t,
 * - an mp_integer::get_mpz_t() method that promotes \p this to dynamic storage and returns a pointer to the internal
 *   \p mpz_t instance,
 * - an \p mpz_view class, an instance of which can be requested via the mp_integer::get_mpz_view() method,
 *   which allows to use mp_integer in the GMP API as a drop-in replacement for <tt>const mpz_t</tt> function
 *   arguments.
 *
 * The \p mpz_view class represent a read-only view of an mp_integer object which is implicitly convertible to the type
 * <tt>const mpz_t</tt> and which is thus usable as an argument to GMP functions. For example:
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c++
 *
 *    mpz_t m;
 *    mpz_init_set_si(m,1); // Create an mpz_t with the value 1.
 *    mp_integer<1> n{1}; // Initialize an mp_integer with the value 1.
 *    mpz_add(m,m,n.get_mpz_view()); // Compute the result of n + m and store
 *                                   // it in m using the GMP API.
 * \endverbatim
 * See the documentation of mp_integer::get_mpz_view() for more details about the \p mpz_view class.
 *
 * # Hashing #
 *
 * This class provides a hash() function to compute a hash value for an integer. A specialisation
 * of the standard \p std::hash functor is also provided, so that it is possible to use mp_integer in standard
 * unordered associative containers out of the box.
 */
template <std::size_t SSize>
class mp_integer
{
    // Just a small helper, like C++14.
    template <bool B, typename T = void>
    using enable_if_t = typename std::enable_if<B, T>::type;
    // Import these typedefs for ease of use.
    using s_storage = mppp_impl::static_int<SSize>;
    using d_storage = mppp_impl::mpz_struct_t;
    using mpz_size_t = mppp_impl::mpz_size_t;
    using mpz_raii = mppp_impl::mpz_raii;
    using mpz_struct_t = mppp_impl::mpz_struct_t;
#if defined(MPPP_WITH_LONG_DOUBLE)
    using mpfr_raii = mppp_impl::mpfr_raii;
#endif
    template <typename T>
    using is_supported_integral = mppp_impl::is_supported_integral<T>;
    template <typename T>
    using is_supported_float = mppp_impl::is_supported_float<T>;
    template <typename T>
    using is_supported_interop = mppp_impl::is_supported_interop<T>;
    template <template <class...> class Op, class... Args>
    using is_detected = mppp_impl::is_detected<Op, Args...>;
    template <typename... Args>
    using conjunction = mppp_impl::conjunction<Args...>;
    template <typename... Args>
    using disjunction = mppp_impl::disjunction<Args...>;
    template <typename T>
    using negation = mppp_impl::negation<T>;
    // The underlying static int.
    using s_int = s_storage;
    // mpz view class.
    struct mpz_view {
        using static_mpz_view = typename s_int::static_mpz_view;
        explicit mpz_view(const mp_integer &n)
            : m_static_view(n.is_static() ? n.m_int.g_st().get_mpz_view() : static_mpz_view{}),
              m_ptr(n.is_static() ? m_static_view : &(n.m_int.g_dy()))
        {
        }
        mpz_view(const mpz_view &) = delete;
        mpz_view(mpz_view &&other)
            : m_static_view(std::move(other.m_static_view)),
              // NOTE: we need to re-init ptr here if other is a static view, because in that case
              // other.m_ptr will be pointing to data in other and we want it to point to data
              // in this now.
              m_ptr((other.m_ptr == other.m_static_view) ? m_static_view : other.m_ptr)
        {
        }
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
        static_mpz_view m_static_view;
        const mpz_struct_t *m_ptr;
    };
    // Machinery for the determination of the result of a binary operation.
    // NOTE: this metaprogramming could be done more cleanly using expr. SFINAE
    // on the internal dispatching functions, but this generates an error in MSVC about
    // the operator being defined twice. My impression is that this is an MSVC problem at the
    // intersection between SFINAE and friend function templates (GCC and clang work fine).
    // We adopt this construction as a workaround.
    template <typename, typename, typename = void>
    struct common_type {
    };
    template <typename T>
    struct common_type<T, T, enable_if_t<std::is_same<T, mp_integer>::value>> {
        using type = mp_integer;
    };
    template <typename T, typename U>
    struct common_type<T, U, enable_if_t<conjunction<std::is_same<T, mp_integer>, is_supported_integral<U>>::value>> {
        using type = mp_integer;
    };
    template <typename T, typename U>
    struct common_type<T, U, enable_if_t<conjunction<std::is_same<U, mp_integer>, is_supported_integral<T>>::value>> {
        using type = mp_integer;
    };
    template <typename T, typename U>
    struct common_type<T, U, enable_if_t<conjunction<std::is_same<T, mp_integer>, is_supported_float<U>>::value>> {
        using type = U;
    };
    template <typename T, typename U>
    struct common_type<T, U, enable_if_t<conjunction<std::is_same<U, mp_integer>, is_supported_float<T>>::value>> {
        using type = T;
    };
    template <typename T, typename U>
    using common_t = typename common_type<T, U>::type;
    // Enabler for in-place arithmetic ops.
    template <typename T>
    using in_place_enabler = enable_if_t<disjunction<is_supported_interop<T>, std::is_same<T, mp_integer>>::value, int>;
#if defined(_MSC_VER)
    // Common metaprogramming for bit shifting operators.
    // NOTE: here and elsewhere we special case MSVC because we need to alter the SFINAE style, as the usual
    // approach (i.e., default template int argument) results in ICEs in MSVC. Instead, on MSCV we use SFINAE
    // on the return type.
    template <typename T>
    using shift_op_enabler = enable_if_t<is_supported_integral<T>::value, mp_integer>;
#else
    template <typename T>
    using shift_op_enabler = enable_if_t<is_supported_integral<T>::value, int>;
#endif
    template <typename T, enable_if_t<std::is_unsigned<T>::value, int> = 0>
    static ::mp_bitcnt_t cast_to_bitcnt(T n)
    {
        if (mppp_unlikely(n > std::numeric_limits<::mp_bitcnt_t>::max())) {
            throw std::domain_error("Cannot bit shift by " + std::to_string(n) + ": the value is too large");
        }
        return static_cast<::mp_bitcnt_t>(n);
    }
    template <typename T, enable_if_t<std::is_signed<T>::value, int> = 0>
    static ::mp_bitcnt_t cast_to_bitcnt(T n)
    {
        if (mppp_unlikely(n < T(0))) {
            throw std::domain_error("Cannot bit shift by " + std::to_string(n) + ": negative values are not supported");
        }
        if (mppp_unlikely(static_cast<typename std::make_unsigned<T>::type>(n)
                          > std::numeric_limits<::mp_bitcnt_t>::max())) {
            throw std::domain_error("Cannot bit shift by " + std::to_string(n) + ": the value is too large");
        }
        return static_cast<::mp_bitcnt_t>(n);
    }

public:
    /// Alias for the template parameter \p SSize.
    static constexpr std::size_t ssize = SSize;
    /// Default constructor.
    /**
     * The default constructor initialises an integer with static storage type and value 0.
     */
    mp_integer() = default;
    /// Copy constructor.
    /**
     * The copy constructor deep-copies \p other into \p this, preserving the original storage type.
     *
     * @param other the object that will be copied into \p this.
     */
    mp_integer(const mp_integer &other) = default;
    /// Move constructor.
    /**
     * The move constructor will leave \p other in an unspecified but valid state. The storage type
     * of \p this will be the same as <tt>other</tt>'s.
     *
     * @param other the object that will be moved into \p this.
     */
    mp_integer(mp_integer &&other) = default;

private:
    // Enabler for generic ctor.
    template <typename T>
    using generic_ctor_enabler = enable_if_t<is_supported_interop<T>::value, int>;
    // Enabler for generic assignment.
    template <typename T>
    using generic_assignment_enabler = generic_ctor_enabler<T>;
    // Add a limb at the top of the integer (that is, the new limb will be the new most
    // significant limb). Requires a non-negative integer. This is used only during generic construction,
    // do **NOT** use it anywhere else.
    void push_limb(::mp_limb_t l)
    {
        const auto asize = m_int.m_st._mp_size;
        assert(asize >= 0);
        if (is_static()) {
            if (std::size_t(asize) < SSize) {
                // If there's space for an extra limb, write it, update the size,
                // and return.
                ++m_int.g_st()._mp_size;
                m_int.g_st().m_limbs[std::size_t(asize)] = l;
                return;
            }
            // Store the upper limb.
            const auto limb_copy = m_int.g_st().m_limbs[SSize - 1u];
            // Make sure the upper limb contains something.
            // NOTE: This is necessary because this method is used while building an integer during generic
            // construction, and it might be possible that the current top limb is zero. If that is the case,
            // the promotion below triggers an assertion failure when destroying
            // the static int in debug mode.
            m_int.g_st().m_limbs[SSize - 1u] = 1u;
            // There's no space for the extra limb. Promote the integer and move on.
            m_int.promote(SSize + 1u);
            // Recover the upper limb.
            m_int.g_dy()._mp_d[SSize - 1u] = limb_copy;
        }
        if (m_int.g_dy()._mp_alloc == asize) {
            // There is not enough space for the extra limb, we need to reallocate.
            // NOTE: same as above, make sure the top limb contains something. mpz_realloc2() seems not to care,
            // but better safe than sorry.
            // NOTE: in practice this is never hit on current architectures: the only case we end up here is initing
            // with a 64bit integer when the limb is 32bit, but in that case we already promoted to size 2 above.
            const auto limb_copy = m_int.g_dy()._mp_d[asize - 1];
            m_int.g_dy()._mp_d[asize - 1] = 1u;
            ::mpz_realloc2(&m_int.g_dy(), static_cast<::mp_bitcnt_t>(asize) * 2u * GMP_NUMB_BITS);
            if (mppp_unlikely(m_int.g_dy()._mp_alloc / 2 != asize)) {
                // This means that there was some overflow in the determination of the bit size above.
                std::abort();
            }
            m_int.g_dy()._mp_d[asize - 1] = limb_copy;
        }
        // Write the extra limb, update the size.
        ++m_int.g_dy()._mp_size;
        m_int.g_dy()._mp_d[asize] = l;
    }
    // This is a small utility function to shift down the unsigned integer n by GMP_NUMB_BITS.
    // If GMP_NUMB_BITS is not smaller than the bit size of T, then an assertion will fire. We need this
    // little helper in order to avoid compiler warnings.
    template <typename T, enable_if_t<(GMP_NUMB_BITS < unsigned(std::numeric_limits<T>::digits)), int> = 0>
    static void checked_rshift(T &n)
    {
        n >>= GMP_NUMB_BITS;
    }
    template <typename T, enable_if_t<(GMP_NUMB_BITS >= unsigned(std::numeric_limits<T>::digits)), int> = 0>
    static void checked_rshift(T &)
    {
        assert(false);
    }
    // Dispatch for generic constructor.
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_unsigned<T>>::value, int> = 0>
    void dispatch_generic_ctor(T n)
    {
        auto nu = static_cast<unsigned long long>(n);
        while (nu) {
            push_limb(static_cast<::mp_limb_t>(nu & GMP_NUMB_MASK));
            if (GMP_NUMB_BITS >= unsigned(std::numeric_limits<unsigned long long>::digits)) {
                break;
            }
            checked_rshift(nu);
        }
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    void dispatch_generic_ctor(T n)
    {
        // NOTE: here we are using cast + unary minus to extract the abs value of n as an unsigned long long. See:
        // http://stackoverflow.com/questions/4536095/unary-minus-and-signed-to-unsigned-conversion
        // When operating on a negative value, this technique is not 100% portable: it requires an implementation
        // of signed integers such that the absolute value of the minimum (negative) value is not greater than
        // the maximum value of the unsigned counterpart. This is guaranteed on all computer architectures in use today,
        // but in theory there could be architectures where the assumption is not respected. See for instance the
        // discussion here:
        // http://stackoverflow.com/questions/11372044/unsigned-vs-signed-range-guarantees
        // Note that in any case we never run into UB, the only consequence is that for very large negative values
        // we could init the integer with the wrong value. We should be able to test for this in the unit tests.
        // Let's keep this in mind in the remote case this ever becomes a problem. In such case, we would probably need
        // to specialise the implementation for negative values to use bit shifting and modulo operations.
        const auto nu = static_cast<unsigned long long>(n);
        if (n >= T(0)) {
            dispatch_generic_ctor(nu);
        } else {
            dispatch_generic_ctor(-nu);
            neg();
        }
    }
    template <typename T, enable_if_t<disjunction<std::is_same<T, float>, std::is_same<T, double>>::value, int> = 0>
    void dispatch_generic_ctor(T x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot init integer from the non-finite floating-point value "
                                    + std::to_string(x));
        }
        MPPP_MAYBE_TLS mpz_raii tmp;
        ::mpz_set_d(&tmp.m_mpz, static_cast<double>(x));
        dispatch_mpz_ctor(&tmp.m_mpz);
    }
#if defined(MPPP_WITH_LONG_DOUBLE)
    template <typename T, enable_if_t<std::is_same<T, long double>::value, int> = 0>
    void dispatch_generic_ctor(T x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot init integer from the non-finite floating-point value "
                                    + std::to_string(x));
        }
        MPPP_MAYBE_TLS mpfr_raii mpfr;
        MPPP_MAYBE_TLS mpz_raii tmp;
        // NOTE: static checks for overflows are done above.
        constexpr int d2 = std::numeric_limits<long double>::digits10 * 4;
        ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(d2));
        ::mpfr_set_ld(&mpfr.m_mpfr, x, MPFR_RNDN);
        ::mpfr_get_z(&tmp.m_mpz, &mpfr.m_mpfr, MPFR_RNDZ);
        dispatch_mpz_ctor(&tmp.m_mpz);
    }
#endif
    // Ctor from mpz_t.
    void dispatch_mpz_ctor(const ::mpz_t n)
    {
        const auto asize = ::mpz_size(n);
        if (asize > SSize) {
            // Too big, need to promote.
            // Destroy static.
            m_int.g_st().~s_storage();
            // Init dynamic.
            ::new (static_cast<void *>(&m_int.m_dy)) d_storage;
            mppp_impl::mpz_init_set_nlimbs(m_int.m_dy, *n);
        } else {
            // All this is noexcept.
            // NOTE: m_st() inits to zero the whole array of static limbs, and we copy
            // in only the used limbs.
            m_int.g_st()._mp_size = n->_mp_size;
            mppp_impl::copy_limbs_no(n->_mp_d, n->_mp_d + asize, m_int.g_st().m_limbs.data());
        }
    }

public:
    /// Generic constructor.
    /**
     * \verbatim embed:rst:leading-asterisk
     * .. note::
     *
     *    This constructor is enabled only if ``T`` is one of the interoperable types.
     * \endverbatim
     *
     * This constructor will initialize an integer with the value of \p x. The initialization is always
     * successful if \p T is an integral type (construction from \p bool yields 1 for \p true, 0 for \p false).
     * If \p T is a floating-point type, the construction will fail if \p x is not finite. Construction from a
     * floating-point type yields the truncated counterpart of the input value.
     *
     * @param x value that will be used to initialize \p this.
     *
     * @throws std::domain_error if \p x is a non-finite floating-point value.
     */
    template <typename T, generic_ctor_enabler<T> = 0>
    explicit mp_integer(T x) : m_int()
    {
        dispatch_generic_ctor(x);
    }
    /// Constructor from C string.
    /**
     * This constructor will initialize \p this from the null-terminated string \p s, which must represent
     * an integer value in base \p base. The expected format is the same as specified by the \p mpz_set_str()
     * GMP function. \p base may vary from 2 to 62, or be zero. In the latter case, the base is inferred
     * from the leading characters of the string.
     *
     * @param s the input string.
     * @param base the base used in the string representation.
     *
     * @throws std::invalid_argument if the \p base parameter is invalid or if \p s is not a valid string representation
     * of an integer in the specified base.
     *
     * \verbatim embed:rst:leading-asterisk
     * .. seealso::
     *
     *    https://gmplib.org/manual/Assigning-Integers.html
     * \endverbatim
     */
    explicit mp_integer(const char *s, int base = 10) : m_int()
    {
        if (mppp_unlikely(base != 0 && (base < 2 || base > 62))) {
            throw std::invalid_argument(
                "In the constructor from string, a base of " + std::to_string(base)
                + " was specified, but the only valid values are 0 and any value in the [2,62] range");
        }
        MPPP_MAYBE_TLS mpz_raii mpz;
        if (mppp_unlikely(::mpz_set_str(&mpz.m_mpz, s, base))) {
            if (base) {
                throw std::invalid_argument(std::string("The string '") + s + "' is not a valid integer in base "
                                            + std::to_string(base));
            } else {
                throw std::invalid_argument(std::string("The string '") + s
                                            + "' is not a valid integer any supported base");
            }
        }
        dispatch_mpz_ctor(&mpz.m_mpz);
    }
    /// Constructor from C++ string (equivalent to the constructor from C string).
    /**
     * @param s the input string.
     * @param base the base used in the string representation.
     *
     * @throws unspecified any exception thrown by the constructor from C string.
     */
    explicit mp_integer(const std::string &s, int base = 10) : mp_integer(s.c_str(), base)
    {
    }
    /// Constructor from \p mpz_t.
    /**
     * This constructor will initialize \p this with the value of the GMP integer \p n. The storage type of \p this
     * will be static if \p n fits in the static storage, otherwise it will be dynamic.
     *
     * \verbatim embed:rst:leading-asterisk
     * .. warning::
     *
     *    It is up to the user to ensure that ``n`` has been correctly initialized. Calling this constructor
     *    with an uninitialized ``n`` is undefined behaviour.
     * \endverbatim
     *
     * @param n the input GMP integer.
     */
    explicit mp_integer(const ::mpz_t n) : m_int()
    {
        dispatch_mpz_ctor(n);
    }
    /// Copy assignment operator.
    /**
     * This operator will perform a deep copy of \p other, preserving its storage type as well.
     *
     * @param other the assignment argument.
     *
     * @return a reference to \p this.
     */
    mp_integer &operator=(const mp_integer &other) = default;
    /// Move assignment operator.
    /**
     * After the move, \p other will be in an unspecified but valid state, and the storage type of \p this will be
     * <tt>other</tt>'s original storage type.
     *
     * @param other the assignment argument.
     *
     * @return a reference to \p this.
     */
    mp_integer &operator=(mp_integer &&other) = default;
    /// Generic assignment operator.
    /**
     * \verbatim embed:rst:leading-asterisk
     * .. note::
     *
     *    This assignment operator is enabled only if ``T`` is an interoperable type.
     * \endverbatim
     *
     * The body of this operator is equivalent to:
     * \verbatim embed:rst:leading-asterisk
     * .. code-block:: c++
     *
     *    return *this = mp_integer{x};
     * \endverbatim
     * That is, a temporary integer is constructed from \p x and it is then move-assigned to \p this.
     *
     * @param x the assignment argument.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the generic constructor.
     */
    template <typename T, generic_assignment_enabler<T> = 0>
    mp_integer &operator=(const T &x)
    {
        return *this = mp_integer{x};
    }
    /// Test for static storage.
    /**
     * @return \p true if the storage type is static, \p false otherwise.
     */
    bool is_static() const
    {
        return m_int.is_static();
    }
    /// Check for dynamic storage.
    /**
     * @return \p true if the storage type is dynamic, \p false otherwise.
     */
    bool is_dynamic() const
    {
        return m_int.is_dynamic();
    }
    /// Output stream operator for mp_integer.
    /**
     * This operator will print to the stream \p os the mp_integer \p n in base 10. Internally it uses the
     * mp_integer::to_string() method.
     *
     * @param os the target stream.
     * @param n the input integer.
     *
     * @return a reference to \p os.
     *
     * @throws unspecified any exception thrown by mp_integer::to_string().
     */
    friend std::ostream &operator<<(std::ostream &os, const mp_integer &n)
    {
        return os << n.to_string();
    }
    /// Input stream operator for mp_integer.
    /**
     * Equivalent to extracting a line from the stream, using it to construct a temporary
     * mp_integer and then assigning the temporary to \p n.
     *
     * @param is input stream.
     * @param n integer to which the contents of the stream will be assigned.
     *
     * @return reference to \p is.
     *
     * @throws unspecified any exception thrown by the constructor from string of mp_integer.
     */
    friend std::istream &operator>>(std::istream &is, mp_integer &n)
    {
        MPPP_MAYBE_TLS std::string tmp_str;
        std::getline(is, tmp_str);
        n = mp_integer{tmp_str};
        return is;
    }
    /// Conversion to string.
    /**
     * This method will convert \p this into a string in base \p base using the GMP function \p mpz_get_str().
     *
     * @param base the desired base.
     *
     * @return a string representation of \p this.
     *
     * @throws std::invalid_argument if \p base is smaller than 2 or greater than 62.
     *
     * \verbatim embed:rst:leading-asterisk
     * .. seealso::
     *
     *    https://gmplib.org/manual/Converting-Integers.html
     * \endverbatim
     */
    std::string to_string(int base = 10) const
    {
        if (mppp_unlikely(base < 2 || base > 62)) {
            throw std::invalid_argument("Invalid base for string conversion: the base must be between "
                                        "2 and 62, but a value of "
                                        + std::to_string(base) + " was provided instead");
        }
        return mppp_impl::mpz_to_str(get_mpz_view(), base);
    }
    // NOTE: maybe provide a method to access the lower-level str conversion that writes to
    // std::vector<char>?

private:
    // Implementation of the conversion operator.
    template <typename T>
    using generic_conversion_enabler = generic_ctor_enabler<T>;
    // Conversion to bool.
    template <typename T, enable_if_t<std::is_same<bool, T>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        return {true, m_int.m_st._mp_size != 0};
    }
    // Try to convert this to an unsigned long long. The abs value of this will be considered for the conversion.
    // Requires nonzero this.
    std::pair<bool, unsigned long long> convert_to_ull() const
    {
        const auto asize = size();
        assert(asize);
        // Get the pointer to the limbs.
        const ::mp_limb_t *ptr = is_static() ? m_int.g_st().m_limbs.data() : m_int.g_dy()._mp_d;
        // Init retval with the first limb.
        auto retval = static_cast<unsigned long long>(ptr[0] & GMP_NUMB_MASK);
        // Add the other limbs, if any.
        unsigned long long shift = GMP_NUMB_BITS;
        constexpr unsigned ull_bits = std::numeric_limits<unsigned long long>::digits;
        for (std::size_t i = 1u; i < asize; ++i, shift += GMP_NUMB_BITS) {
            if (shift >= ull_bits) {
                // We need to shift left the current limb. If the shift is too large, we run into UB
                // and it also means that the value does not fit in unsigned long long (and, by extension,
                // in any other unsigned integral type).
                return {false, 0ull};
            }
            const auto l = static_cast<unsigned long long>(ptr[i] & GMP_NUMB_MASK);
            if (l >> (ull_bits - shift)) {
                // Shifting the current limb is well-defined from the point of view of the language, but the result
                // wraps around: the value does not fit in unsigned long long.
                // NOTE: I suspect this branch can be triggered on common architectures only with nail builds.
                return {false, 0ull};
            }
            // This will not overflow, as there is no carry from retval and l << shift is fine.
            retval += l << shift;
        }
        return {true, retval};
    }
    // Conversion to unsigned ints, excluding bool.
    template <typename T,
              enable_if_t<conjunction<std::is_integral<T>, std::is_unsigned<T>, negation<std::is_same<bool, T>>>::value,
                          int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        // Handle zero.
        if (!m_int.m_st._mp_size) {
            return {true, T(0)};
        }
        // Handle negative value.
        if (m_int.m_st._mp_size < 0) {
            return {false, T(0)};
        }
        // Attempt conversion to ull.
        const auto candidate = convert_to_ull();
        if (!candidate.first) {
            // The conversion to ull failed.
            return {false, T(0)};
        }
        if (candidate.second > std::numeric_limits<T>::max()) {
            // The conversion to ull succeeded, but the value exceeds the limit of T.
            return {false, T(0)};
        }
        // The conversion to the target unsigned integral type is fine.
        return {true, static_cast<T>(candidate.second)};
    }
    // Conversion to signed ints.
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        // NOTE: here we will assume that unsigned long long can represent the absolute value of any
        // machine integer. This is not necessarily the case on any possible implementation, but it holds
        // true on any current architecture. See the comments below and in the generic constructor regarding
        // the method of computing the absolute value of a negative int.
        using uT = typename std::make_unsigned<T>::type;
        // Handle zero.
        if (!m_int.m_st._mp_size) {
            return {true, T(0)};
        }
        // Attempt conversion to ull.
        const auto candidate = convert_to_ull();
        if (!candidate.first) {
            // The conversion to ull failed: the value is too large to be represented by any integer type.
            return {false, T(0)};
        }
        if (m_int.m_st._mp_size > 0) {
            // If the value is positive, we check that the candidate does not exceed the
            // max value of the type.
            if (candidate.second > uT(std::numeric_limits<T>::max())) {
                return {false, T(0)};
            }
            return {true, static_cast<T>(candidate.second)};
        } else {
            // For negative values, we need to establish if the value fits the negative range of the
            // target type, and we must make sure to take the negative of the candidate correctly.
            // NOTE: see the comments in the generic ctor about the portability of this technique
            // for computing the absolute value.
            constexpr auto min_abs = -static_cast<unsigned long long>(std::numeric_limits<T>::min());
            constexpr auto max = static_cast<unsigned long long>(std::numeric_limits<T>::max());
            if (candidate.second > min_abs) {
                // The value is too small.
                return {false, T(0)};
            }
            // The abs of min might be greater than max. The idea then is that we decrease the magnitude
            // of the candidate to the safe negation range via division, we negate it and then we recover the
            // original candidate value. If the abs of min is not greater than max, ceil_ratio will be 1, r will be zero
            // and everything will still work.
            constexpr auto ceil_ratio = min_abs / max + unsigned((min_abs % max) != 0u);
            const unsigned long long q = candidate.second / ceil_ratio, r = candidate.second % ceil_ratio;
            auto retval = static_cast<T>(-static_cast<T>(q));
            static_assert(ceil_ratio <= uT(std::numeric_limits<T>::max()), "Overflow error.");
            retval = static_cast<T>(retval * static_cast<T>(ceil_ratio));
            retval = static_cast<T>(retval - static_cast<T>(r));
            return {true, retval};
        }
    }
    // Implementation of the conversion to floating-point through GMP/MPFR routines.
    template <typename T, enable_if_t<disjunction<std::is_same<T, float>, std::is_same<T, double>>::value, int> = 0>
    static std::pair<bool, T> mpz_float_conversion(const mpz_struct_t &m)
    {
        return {true, static_cast<T>(::mpz_get_d(&m))};
    }
#if defined(MPPP_WITH_LONG_DOUBLE)
    template <typename T, enable_if_t<std::is_same<T, long double>::value, int> = 0>
    static std::pair<bool, T> mpz_float_conversion(const mpz_struct_t &m)
    {
        MPPP_MAYBE_TLS mpfr_raii mpfr;
        constexpr int d2 = std::numeric_limits<long double>::digits10 * 4;
        ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(d2));
        ::mpfr_set_z(&mpfr.m_mpfr, &m, MPFR_RNDN);
        return {true, ::mpfr_get_ld(&mpfr.m_mpfr, MPFR_RNDN)};
    }
#endif
    // Conversion to floating-point.
    template <typename T, enable_if_t<std::is_floating_point<T>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        // Handle zero.
        if (!m_int.m_st._mp_size) {
            return {true, T(0)};
        }
        if (std::numeric_limits<T>::is_iec559) {
            // Optimization for single-limb integers.
            //
            // NOTE: the reasoning here is as follows. If the floating-point type has infinity,
            // then its "range" is the whole real line. The C++ standard guarantees that:
            // """
            // A prvalue of an integer type or of an unscoped enumeration type can be converted to a prvalue of a
            // floating point type. The result is exact if possible. If the value being converted is in the range
            // of values that can be represented but the value cannot be represented exactly,
            // it is an implementation-defined choice of either the next lower or higher representable value. If the
            // value being converted is outside the range of values that can be represented, the behavior is undefined.
            // """
            // This seems to indicate that if the limb value "overflows" the finite range of the floating-point type, we
            // will get either the max/min finite value or +-inf. Additionally, the IEEE standard seems to indicate
            // that an overflowing conversion will produce infinity:
            // http://stackoverflow.com/questions/40694384/integer-to-float-conversions-with-ieee-fp
            //
            // Get the pointer to the limbs.
            const ::mp_limb_t *ptr = is_static() ? m_int.g_st().m_limbs.data() : m_int.g_dy()._mp_d;
            if (m_int.m_st._mp_size == 1) {
                return {true, static_cast<T>(ptr[0] & GMP_NUMB_MASK)};
            }
            if (m_int.m_st._mp_size == -1) {
                return {true, -static_cast<T>(ptr[0] & GMP_NUMB_MASK)};
            }
        }
        // For all the other cases, just delegate to the GMP/MPFR routines.
        return mpz_float_conversion<T>(*static_cast<const mpz_struct_t *>(get_mpz_view()));
    }

public:
    /// Generic conversion operator.
    /**
     * \verbatim embed:rst:leading-asterisk
     * .. note::
     *
     *    This operator is enabled only if ``T`` is an interoperable type.
     * \endverbatim
     *
     * This operator will convert \p this to the type \p T. Conversion to \p bool yields \p false if \p this is zero,
     * \p true otherwise. Conversion to other integral types yields the exact result, if representable by the target
     * type \p T. Conversion to floating-point types might yield inexact values and infinities.
     *
     * @return \p this converted to \p T.
     *
     * @throws std::overflow_error if \p T is an integral type and the value of \p this cannot be represented by \p T.
     */
    template <typename T, generic_conversion_enabler<T> = 0>
    explicit operator T() const
    {
        const auto retval = dispatch_conversion<T>();
        if (mppp_unlikely(!retval.first)) {
            throw std::overflow_error("Conversion of the integer " + to_string() + " to the type " + typeid(T).name()
                                      + " results in overflow");
        }
        return retval.second;
    }
    /// Promote to dynamic storage.
    /**
     * This method will promote the storage type of \p this from static to dynamic.
     *
     * @return \p false if the storage type of \p this is already dynamic and no promotion takes place, \p true
     * otherwise.
     */
    bool promote()
    {
        if (is_static()) {
            m_int.promote();
            return true;
        }
        return false;
    }
    /// Demote to static storage.
    /**
     * This method will demote the storage type of \p this from dynamic to static.
     *
     * @return \p false if the storage type of \p this is already static and no demotion takes place, or if the current
     * value of \p this does not fit in static storage, \p true otherwise.
     */
    bool demote()
    {
        if (is_dynamic()) {
            return m_int.demote();
        }
        return false;
    }
    /// Size in bits.
    /**
     * @return the number of bits needed to represent \p this. If \p this is zero, zero will be returned.
     */
    std::size_t nbits() const
    {
        return m_int.m_st._mp_size ? ::mpz_sizeinbase(get_mpz_view(), 2) : 0u;
    }
    /// Size in limbs.
    /**
     * @return the number of limbs needed to represent \p this. If \p this is zero, zero will be returned.
     */
    std::size_t size() const
    {
        if (is_static()) {
            return std::size_t(m_int.g_st().abs_size());
        }
        return ::mpz_size(&m_int.g_dy());
    }
    /// Sign.
    /**
     * @return 0 if \p this is zero, 1 if \p this is positive, -1 if \p this is negative.
     */
    int sgn() const
    {
        // NOTE: size is part of the common initial sequence.
        if (m_int.m_st._mp_size != 0) {
            return m_int.m_st._mp_size > 0 ? 1 : -1;
        } else {
            return 0;
        }
    }
    /// Sign function.
    /**
     * @param n the mp_integer whose sign will be computed.
     *
     * @return 0 if \p this is zero, 1 if \p this is positive, -1 if \p this is negative.
     */
    friend int sgn(const mp_integer &n)
    {
        return n.sgn();
    }
    /// Get an \p mpz_t view.
    /**
     * This method will return an object of an unspecified type \p mpz_view which is implicitly convertible
     * to a const pointer to an \p mpz_t struct (and which can thus be used as a <tt>const mpz_t</tt>
     * parameter in GMP functions). In addition to the implicit conversion operator, the <tt>const mpz_t</tt>
     * object can also be retrieved via the <tt>get()</tt> method of the \p mpz_view class.
     * The view provides a read-only GMP-compatible representation of the integer stored in \p this.
     *
     * \verbatim embed:rst:leading-asterisk
     * .. note::
     *
     *   It is important to keep in mind the following facts about the returned ``mpz_view`` object:
     *
     *   * ``mpz_view`` objects are strictly read-only: it is impossible to alter ``this`` through an ``mpz_view``, and
     *     ``mpz_view`` objects can be used in the GMP API only where a ``const mpz_t`` parameter is expected;
     *   * ``mpz_view`` objects can only be move-constructed (the other constructors and the assignment operators
     *     are disabled);
     *   * the returned object and the pointer returned by its ``get()`` method might reference internal data
     *     belonging to ``this``, and they can thus be used safely only during the lifetime of ``this``;
     *   * the lifetime of the pointer returned by the ``get()`` method is tied to the lifetime of the ``mpz_view``
     *     object (that is, if the ``mpz_view`` object is destroyed, any pointer previously returned by ``get()``
     *     becomes invalid);
     *   * any modification to ``this`` will also invalidate the view and the pointer.
     * \endverbatim
     *
     * @return an \p mpz view of \p this.
     */
    mpz_view get_mpz_view() const
    {
        return mpz_view(*this);
    }
    /// Negate in-place.
    /**
     * This method will set \p this to <tt>-this</tt>.
     *
     * @return a reference to \p this.
     */
    mp_integer &neg()
    {
        if (is_static()) {
            m_int.g_st()._mp_size = -m_int.g_st()._mp_size;
        } else {
            ::mpz_neg(&m_int.g_dy(), &m_int.g_dy());
        }
        return *this;
    }
    /// Binary negation.
    /**
     * This method will set \p rop to <tt>-n</tt>.
     *
     * @param rop the return value.
     * @param n the integer that will be negated.
     */
    friend void neg(mp_integer &rop, const mp_integer &n)
    {
        rop = n;
        rop.neg();
    }
    /// Unary negation.
    /**
     * @param n the integer that will be negated.
     *
     * @return <tt>-n</tt>.
     */
    friend mp_integer neg(const mp_integer &n)
    {
        mp_integer ret(n);
        ret.neg();
        return ret;
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
    // Small helper to compute the size after subtraction via mpn. s is a strictly positive size.
    static mpz_size_t sub_compute_size(const ::mp_limb_t *rdata, mpz_size_t s)
    {
        assert(s > 0);
        mpz_size_t cur_idx = s - 1;
        for (; cur_idx >= 0; --cur_idx) {
            if (rdata[cur_idx] & GMP_NUMB_MASK) {
                break;
            }
        }
        return cur_idx + 1;
    }
    // NOTE: this function (and its other overloads) will return true in case of success, false in case of failure
    // (i.e., the addition overflows and the static size is not enough).
    static bool static_add_impl(s_int &rop, const s_int &op1, const s_int &op2, mpz_size_t asize1, mpz_size_t asize2,
                                int sign1, int sign2, const std::integral_constant<int, 0> &)
    {
        using mppp_impl::copy_limbs;
        auto rdata = &rop.m_limbs[0];
        auto data1 = &op1.m_limbs[0], data2 = &op2.m_limbs[0];
        const auto size1 = op1._mp_size;
        // NOTE: cannot trust the size member from op2, as op2 could've been negated if
        // we are actually subtracting.
        const auto size2 = (sign2 >= 0) ? asize2 : -asize2;
        // mpn functions require nonzero arguments.
        if (mppp_unlikely(!sign2)) {
            rop._mp_size = size1;
            copy_limbs(data1, data1 + asize1, rdata);
            return true;
        }
        if (mppp_unlikely(!sign1)) {
            rop._mp_size = size2;
            copy_limbs(data2, data2 + asize2, rdata);
            return true;
        }
        // Check, for op1 and op2, whether:
        // - the asize is the max static size, and, if yes,
        // - the highest bit in the highest limb is set.
        // If this condition holds for op1 and op2, we return failure, as the computation might require
        // an extra limb.
        // NOTE: the reason we check this is that we do not want to run into the situation in which we have written
        // something into rop (via the mpn functions below), only to realize later that the computation overflows.
        // This would be bad because, in case of overlapping arguments, it would destroy one or two operands without
        // possibility of recovering. The alternative would be to do the computation in some local buffer and then
        // copy
        // it out, but that is rather costly. Note that this means that in principle a computation that could fit in
        // static storage ends up triggering a promotion.
        const bool c1 = std::size_t(asize1) == SSize && ((data1[asize1 - 1] & GMP_NUMB_MASK) >> (GMP_NUMB_BITS - 1));
        const bool c2 = std::size_t(asize2) == SSize && ((data2[asize2 - 1] & GMP_NUMB_MASK) >> (GMP_NUMB_BITS - 1));
        if (mppp_unlikely(c1 || c2)) {
            return false;
        }
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
                    assert(asize1 < s_int::s_size);
                    rop._mp_size = size1 + sign1;
                    // NOTE: there should be no need to use GMP_NUMB_MASK here.
                    rdata[asize1] = 1u;
                } else {
                    // Without carry, the size is unchanged.
                    rop._mp_size = size1;
                }
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
                    assert(asize2 < s_int::s_size);
                    rop._mp_size = size2 + sign2;
                    rdata[asize2] = 1u;
                } else {
                    rop._mp_size = size2;
                }
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
                rop._mp_size = sub_compute_size(rdata, asize1);
                if (sign1 != 1) {
                    rop._mp_size = -rop._mp_size;
                }
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
                rop._mp_size = sub_compute_size(rdata, asize2);
                if (sign2 != 1) {
                    rop._mp_size = -rop._mp_size;
                }
            }
        }
        return true;
    }
    // Optimization for single-limb statics with no nails.
    static bool static_add_impl(s_int &rop, const s_int &op1, const s_int &op2, mpz_size_t asize1, mpz_size_t asize2,
                                int sign1, int sign2, const std::integral_constant<int, 1> &)
    {
        using mppp_impl::limb_add_overflow;
        auto rdata = &rop.m_limbs[0];
        auto data1 = &op1.m_limbs[0], data2 = &op2.m_limbs[0];
        // NOTE: both asizes have to be 0 or 1 here.
        assert((asize1 == 1 && data1[0] != 0u) || (asize1 == 0 && data1[0] == 0u));
        assert((asize2 == 1 && data2[0] != 0u) || (asize2 == 0 && data2[0] == 0u));
        ::mp_limb_t tmp;
        if (sign1 == sign2) {
            // When the signs are identical, we can implement addition as a true addition.
            if (mppp_unlikely(limb_add_overflow(data1[0], data2[0], &tmp))) {
                return false;
            }
            // Assign the output. asize can be zero (sign1 == sign2 == 0) or 1.
            rop._mp_size = sign1;
            rdata[0] = tmp;
        } else {
            // When the signs differ, we need to implement addition as a subtraction.
            // NOTE: this also includes the case in which only one of the operands is zero.
            if (data1[0] >= data2[0]) {
                // op1 is not smaller than op2.
                tmp = data1[0] - data2[0];
                // asize is either 1 or 0 (0 iff abs(op1) == abs(op2)).
                rop._mp_size = sign1;
                if (mppp_unlikely(!tmp)) {
                    rop._mp_size = 0;
                }
                rdata[0] = tmp;
            } else {
                // NOTE: this has to be one, as data2[0] and data1[0] cannot be equal.
                rop._mp_size = sign2;
                rdata[0] = data2[0] - data1[0];
            }
        }
        return true;
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
    static bool static_add_impl(s_int &rop, const s_int &op1, const s_int &op2, mpz_size_t asize1, mpz_size_t asize2,
                                int sign1, int sign2, const std::integral_constant<int, 2> &)
    {
        using mppp_impl::limb_add_overflow;
        auto rdata = &rop.m_limbs[0];
        auto data1 = &op1.m_limbs[0], data2 = &op2.m_limbs[0];
        if (sign1 == sign2) {
            // NOTE: this handles the case in which the numbers have the same sign, including 0 + 0.
            //
            // NOTE: this is the implementation for 2 limbs, even if potentially the operands have 1 limb.
            // The idea here is that it's better to do a few computations more rather than paying the branching
            // cost. 1-limb operands will have the upper limb set to zero from the zero-initialization of
            // the limbs of static ints.
            //
            // NOTE: the rop hi limb might spill over either from the addition of the hi limbs
            // of op1 and op2, or by the addition of carry coming over from the addition of
            // the lo limbs of op1 and op2.
            //
            // Add the hi and lo limbs.
            const auto a = data1[0], b = data2[0], c = data1[1], d = data2[1];
            ::mp_limb_t lo, hi1, hi2;
            const ::mp_limb_t cy_lo = limb_add_overflow(a, b, &lo), cy_hi1 = limb_add_overflow(c, d, &hi1),
                              cy_hi2 = limb_add_overflow(hi1, cy_lo, &hi2);
            // The result will overflow if we have any carry relating to the high limb.
            if (mppp_unlikely(cy_hi1 || cy_hi2)) {
                return false;
            }
            // For the size compuation:
            // - if sign1 == 0, size is zero,
            // - if sign1 == +-1, then the size is either +-1 or +-2: asize is 2 if the result
            //   has a nonzero 2nd limb, otherwise asize is 1.
            rop._mp_size = sign1;
            if (hi2) {
                rop._mp_size = sign1 + sign1;
            }
            rdata[0] = lo;
            rdata[1] = hi2;
        } else {
            // When the signs differ, we need to implement addition as a subtraction.
            // NOTE: this also includes the case in which only one of the operands is zero.
            if (asize1 > asize2 || (asize1 == asize2 && compare_limbs_2(data1, data2, asize1) >= 0)) {
                // op1 is >= op2 in absolute value.
                const auto lo = data1[0] - data2[0];
                // If there's a borrow, then hi1 > hi2, otherwise we would have a negative result.
                assert(data1[0] >= data2[0] || data1[1] > data2[1]);
                // This can never wrap around, at most it goes to zero.
                const auto hi = data1[1] - data2[1] - static_cast<::mp_limb_t>(data1[0] < data2[0]);
                // asize can be 0, 1 or 2.
                rop._mp_size = 0;
                if (hi) {
                    rop._mp_size = sign1 + sign1;
                } else if (lo) {
                    rop._mp_size = sign1;
                }
                rdata[0] = lo;
                rdata[1] = hi;
            } else {
                // op2 is > op1 in absolute value.
                const auto lo = data2[0] - data1[0];
                assert(data2[0] >= data1[0] || data2[1] > data1[1]);
                const auto hi = data2[1] - data1[1] - static_cast<::mp_limb_t>(data2[0] < data1[0]);
                // asize can be 1 or 2, but not zero as we know abs(op1) != abs(op2).
                rop._mp_size = sign2;
                if (hi) {
                    rop._mp_size = sign2 + sign2;
                }
                rdata[0] = lo;
                rdata[1] = hi;
            }
        }
        return true;
    }
    template <bool AddOrSub>
    static bool static_addsub(s_int &rop, const s_int &op1, const s_int &op2)
    {
        // NOTE: effectively negate op2 if we are subtracting.
        mpz_size_t asize1 = op1._mp_size, asize2 = AddOrSub ? op2._mp_size : -op2._mp_size;
        int sign1 = asize1 != 0, sign2 = asize2 != 0;
        if (asize1 < 0) {
            asize1 = -asize1;
            sign1 = -1;
        }
        if (asize2 < 0) {
            asize2 = -asize2;
            sign2 = -1;
        }
        const bool retval = static_add_impl(rop, op1, op2, asize1, asize2, sign1, sign2, static_add_algo<s_int>{});
        if (static_add_algo<s_int>::value == 0 && retval) {
            // If we used the mpn functions and we actually wrote into rop, then
            // make sure we zero out the unused limbs.
            rop.zero_unused_limbs();
        }
        return retval;
    }
    // Dispatching for the binary addition operator.
    static mp_integer dispatch_binary_add(const mp_integer &op1, const mp_integer &op2)
    {
        mp_integer retval;
        add(retval, op1, op2);
        return retval;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static mp_integer dispatch_binary_add(const mp_integer &op1, T n)
    {
        mp_integer retval{n};
        add(retval, retval, op1);
        return retval;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static mp_integer dispatch_binary_add(T n, const mp_integer &op2)
    {
        return dispatch_binary_add(op2, n);
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static T dispatch_binary_add(const mp_integer &op1, T x)
    {
        return static_cast<T>(op1) + x;
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static T dispatch_binary_add(T x, const mp_integer &op2)
    {
        return dispatch_binary_add(op2, x);
    }
    // Dispatching for in-place add.
    static void dispatch_in_place_add(mp_integer &retval, const mp_integer &n)
    {
        add(retval, retval, n);
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static void dispatch_in_place_add(mp_integer &retval, const T &n)
    {
        add(retval, retval, mp_integer{n});
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static void dispatch_in_place_add(mp_integer &retval, const T &x)
    {
        retval = static_cast<T>(retval) + x;
    }
    // Dispatching for the binary subtraction operator.
    static mp_integer dispatch_binary_sub(const mp_integer &op1, const mp_integer &op2)
    {
        mp_integer retval;
        sub(retval, op1, op2);
        return retval;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static mp_integer dispatch_binary_sub(const mp_integer &op1, T n)
    {
        mp_integer retval{n};
        sub(retval, retval, op1);
        retval.neg();
        return retval;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static mp_integer dispatch_binary_sub(T n, const mp_integer &op2)
    {
        auto retval = dispatch_binary_sub(op2, n);
        retval.neg();
        return retval;
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static T dispatch_binary_sub(const mp_integer &op1, T x)
    {
        return static_cast<T>(op1) - x;
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static T dispatch_binary_sub(T x, const mp_integer &op2)
    {
        return -dispatch_binary_sub(op2, x);
    }
    // Dispatching for in-place sub.
    static void dispatch_in_place_sub(mp_integer &retval, const mp_integer &n)
    {
        sub(retval, retval, n);
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static void dispatch_in_place_sub(mp_integer &retval, const T &n)
    {
        sub(retval, retval, mp_integer{n});
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static void dispatch_in_place_sub(mp_integer &retval, const T &x)
    {
        retval = static_cast<T>(retval) - x;
    }

public:
    /// Ternary add.
    /**
     * This function will set \p rop to <tt>op1 + op2</tt>.
     *
     * @param rop the return value.
     * @param op1 the first argument.
     * @param op2 the second argument.
     */
    friend void add(mp_integer &rop, const mp_integer &op1, const mp_integer &op2)
    {
        const bool sr = rop.is_static(), s1 = op1.is_static(), s2 = op2.is_static();
        if (mppp_likely(sr && s1 && s2)) {
            // Optimise the case of all statics.
            if (mppp_likely(static_addsub<true>(rop.m_int.g_st(), op1.m_int.g_st(), op2.m_int.g_st()))) {
                return;
            }
        }
        if (sr) {
            rop.m_int.promote(SSize + 1u);
        }
        ::mpz_add(&rop.m_int.g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    }
    /// Identity operator.
    /**
     * @return a copy of \p this.
     */
    mp_integer operator+() const
    {
        return *this;
    }
    /// Binary addition operator.
    /**
     * @param op1 the first argument.
     * @param op2 the second argument.
     *
     * @return <tt>op1 + op2</tt>.
     */
    template <typename T, typename U>
    friend common_t<T, U> operator+(const T &op1, const U &op2)
    {
        return dispatch_binary_add(op1, op2);
    }
    /// In-place addition operator.
    /**
     * @param op the addend.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the assignment of a floating-point value to mp_integer, iff \p T
     * is a floating-point type.
     */
    template <typename T, in_place_enabler<T> = 0>
    mp_integer &operator+=(const T &op)
    {
        dispatch_in_place_add(*this, op);
        return *this;
    }

private:
#if defined(_MSC_VER)
    template <typename T>
    using in_place_lenabler = enable_if_t<is_supported_interop<T>::value, T &>;
#else
    template <typename T>
    using in_place_lenabler = enable_if_t<is_supported_interop<T>::value, int>;
#endif

public:
#if defined(_MSC_VER)
    template <typename T>
    friend in_place_lenabler<T> operator+=(T &x, const mp_integer &n)
#else
    /// In-place addition for interoperable types.
    /**
     * \verbatim embed:rst:leading-asterisk
     * .. note::
     *
     *    This operator is enabled only if ``T`` is an interoperable type for :cpp:class:`mppp::mp_integer`.
     * \endverbatim
     *
     * The body of this operator is equivalent to:
     * \verbatim embed:rst:leading-asterisk
     * .. code-block:: c++
     *
     *    return x = static_cast<T>(x + n);
     * \endverbatim
     *
     * That is, the result of the corresponding binary operation is cast back to \p T and assigned to \p x.
     *
     * @param x the first argument.
     * @param n the second argument.
     *
     * @return a reference to \p x.
     *
     * @throws unspecified any exception thrown by the conversion operator of mppp::mp_integer.
     */
    template <typename T, in_place_lenabler<T> = 0>
    friend T &operator+=(T &x, const mp_integer &n)
#endif
    {
        // NOTE: if x is an integral, then the static cast is a generic conversion from
        // mp_integer to the integral, which can fail because of overflow. Otherwise, the
        // static cast is a redundant cast to float of x + n, which is already a float.
        return x = static_cast<T>(x + n);
    }
    /// Prefix increment.
    /**
     * Increment \p this by one.
     *
     * @return reference to \p this after the increment.
     */
    mp_integer &operator++()
    {
        add_ui(*this, *this, 1u);
        return *this;
    }
    /// Suffix increment.
    /**
     * Increment \p this by one and return a copy of \p this as it was before the increment.
     *
     * @return a copy of \p this before the increment.
     */
    mp_integer operator++(int)
    {
        mp_integer retval(*this);
        ++(*this);
        return retval;
    }
    /// Ternary subtraction.
    /**
     * This function will set \p rop to <tt>op1 - op2</tt>.
     *
     * @param rop the return value.
     * @param op1 the first argument.
     * @param op2 the second argument.
     */
    friend void sub(mp_integer &rop, const mp_integer &op1, const mp_integer &op2)
    {
        const bool sr = rop.is_static(), s1 = op1.is_static(), s2 = op2.is_static();
        if (mppp_likely(sr && s1 && s2)) {
            // Optimise the case of all statics.
            if (mppp_likely(static_addsub<false>(rop.m_int.g_st(), op1.m_int.g_st(), op2.m_int.g_st()))) {
                return;
            }
        }
        if (sr) {
            rop.m_int.promote(SSize + 1u);
        }
        ::mpz_sub(&rop.m_int.g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    }
    /// Negated copy.
    /**
     * @return a negated copy of \p this.
     */
    mp_integer operator-() const
    {
        mp_integer retval{*this};
        retval.neg();
        return retval;
    }
    /// Binary subtraction operator.
    /**
     * @param op1 the first argument.
     * @param op2 the second argument.
     *
     * @return <tt>op1 - op2</tt>.
     */
    template <typename T, typename U>
    friend common_t<T, U> operator-(const T &op1, const U &op2)
    {
        return dispatch_binary_sub(op1, op2);
    }
    /// In-place subtraction operator.
    /**
     * @param op the subtrahend.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the assignment of a floating-point value to mp_integer, iff \p T
     * is a floating-point type.
     */
    template <typename T, in_place_enabler<T> = 0>
    mp_integer &operator-=(const T &op)
    {
        dispatch_in_place_sub(*this, op);
        return *this;
    }
#if defined(_MSC_VER)
    template <typename T>
    friend in_place_lenabler<T> operator-=(T &x, const mp_integer &n)
#else
    /// In-place subtraction for interoperable types.
    /**
     * \verbatim embed:rst:leading-asterisk
     * .. note::
     *
     *    This operator is enabled only if ``T`` is an interoperable type for :cpp:class:`mppp::mp_integer`.
     * \endverbatim
     *
     * The body of this operator is equivalent to:
     * \verbatim embed:rst:leading-asterisk
     * .. code-block:: c++
     *
     *    return x = static_cast<T>(x - n);
     * \endverbatim
     *
     * That is, the result of the corresponding binary operation is cast back to \p T and assigned to \p x.
     *
     * @param x the first argument.
     * @param n the second argument.
     *
     * @return a reference to \p x.
     *
     * @throws unspecified any exception thrown by the conversion operator of mppp::mp_integer.
     */
    template <typename T, in_place_lenabler<T> = 0>
    friend T &operator-=(T &x, const mp_integer &n)
#endif
    {
        return x = static_cast<T>(x - n);
    }
    /// Prefix decrement.
    /**
     * Decrement \p this by one.
     *
     * @return reference to \p this after the decrement.
     */
    mp_integer &operator--()
    {
        // NOTE: this should be written in terms of sub_ui(), once implemented.
        neg();
        add_ui(*this, *this, 1u);
        neg();
        return *this;
    }
    /// Suffix decrement.
    /**
     * Decrement \p this by one and return a copy of \p this as it was before the decrement.
     *
     * @return a copy of \p this before the decrement.
     */
    mp_integer operator--(int)
    {
        mp_integer retval(*this);
        --(*this);
        return retval;
    }

private:
    // Metaprogramming for selecting the algorithm for static addition with ui. The selection happens via
    // an std::integral_constant with 3 possible values:
    // - 0 (default case): use the GMP mpn functions,
    // - 1: selected when there are no nail bits and the static size is 1,
    // - 2: selected when there are no nail bits and the static size is 2.
    template <typename SInt>
    using static_add_ui_algo = std::integral_constant<int, (!GMP_NAIL_BITS && SInt::s_size == 1)
                                                               ? 1
                                                               : ((!GMP_NAIL_BITS && SInt::s_size == 2) ? 2 : 0)>;
    // mpn implementation.
    static bool static_add_ui_impl(s_int &rop, const s_int &op1, mpz_size_t asize1, int sign1, unsigned long op2,
                                   const std::integral_constant<int, 0> &)
    {
        using mppp_impl::copy_limbs;
        auto rdata = &rop.m_limbs[0];
        auto data1 = &op1.m_limbs[0];
        const auto size1 = op1._mp_size;
        const auto l2 = static_cast<::mp_limb_t>(op2);
        // mpn functions require nonzero arguments.
        if (mppp_unlikely(!l2)) {
            rop._mp_size = size1;
            copy_limbs(data1, data1 + asize1, rdata);
            return true;
        }
        if (mppp_unlikely(!sign1)) {
            // NOTE: this has to be 1 because l2 == 0 is handled above.
            rop._mp_size = 1;
            rdata[0] = l2;
            return true;
        }
        // This is the same overflow check as in static_add().
        const bool c1 = std::size_t(asize1) == SSize && ((data1[asize1 - 1] & GMP_NUMB_MASK) >> (GMP_NUMB_BITS - 1));
        const bool c2 = SSize == 1u && (l2 >> (GMP_NUMB_BITS - 1));
        if (mppp_unlikely(c1 || c2)) {
            return false;
        }
        if (sign1 == 1) {
            // op1 and op2 are both strictly positive.
            if (::mpn_add_1(rdata, data1, static_cast<::mp_size_t>(asize1), l2)) {
                assert(asize1 < s_int::s_size);
                rop._mp_size = size1 + 1;
                // NOTE: there should be no need to use GMP_NUMB_MASK here.
                rdata[asize1] = 1u;
            } else {
                // Without carry, the size is unchanged.
                rop._mp_size = size1;
            }
        } else {
            // op1 is strictly negative, op2 is strictly positive.
            if (asize1 > 1 || (asize1 == 1 && (data1[0] & GMP_NUMB_MASK) >= l2)) {
                // abs(op1) >= abs(op2).
                const auto br = ::mpn_sub_1(rdata, data1, static_cast<::mp_size_t>(asize1), l2);
                (void)br;
                assert(!br);
                // The asize can be the original one or original - 1 (we subtracted a limb).
                rop._mp_size = size1 + static_cast<mpz_size_t>(!(rdata[asize1 - 1] & GMP_NUMB_MASK));
            } else {
                // abs(op2) > abs(op1).
                const auto br = ::mpn_sub_1(rdata, &l2, 1, data1[0]);
                (void)br;
                assert(!br);
                // The size must be 1, as abs(op2) == abs(op1) is handled above.
                assert((rdata[0] & GMP_NUMB_MASK));
                rop._mp_size = 1;
            }
        }
        return true;
    }
    // 1-limb optimisation (no nails).
    static bool static_add_ui_impl(s_int &rop, const s_int &op1, mpz_size_t, int sign1, unsigned long op2,
                                   const std::integral_constant<int, 1> &)
    {
        using mppp_impl::limb_add_overflow;
        const auto l1 = op1.m_limbs[0];
        const auto l2 = static_cast<::mp_limb_t>(op2);
        ::mp_limb_t tmp;
        if (sign1 >= 0) {
            // True unsigned addition.
            if (mppp_unlikely(limb_add_overflow(l1, l2, &tmp))) {
                return false;
            }
            rop._mp_size = (tmp != 0u);
            rop.m_limbs[0] = tmp;
        } else {
            // op1 is negative, op2 is non-negative. Implement as a sub.
            if (l1 >= l2) {
                // op1 has larger or equal abs. The result will be non-positive.
                tmp = l1 - l2;
                // size is -1 or 0, 0 iff l1 == l2.
                rop._mp_size = -static_cast<mpz_size_t>(tmp != 0u);
                rop.m_limbs[0] = tmp;
            } else {
                // op1 has smaller abs. The result will be positive.
                rop._mp_size = 1;
                rop.m_limbs[0] = l2 - l1;
            }
        }
        return true;
    }
    // 2-limb optimisation (no nails).
    static bool static_add_ui_impl(s_int &rop, const s_int &op1, mpz_size_t asize1, int sign1, unsigned long op2,
                                   const std::integral_constant<int, 2> &)
    {
        using mppp_impl::limb_add_overflow;
        auto rdata = &rop.m_limbs[0];
        auto data1 = &op1.m_limbs[0];
        const auto l2 = static_cast<::mp_limb_t>(op2);
        if (sign1 >= 0) {
            // True unsigned addition.
            // These two limbs will contain the result.
            ::mp_limb_t lo, hi;
            // Add l2 to the low limb, placing the result in lo.
            const ::mp_limb_t cy_lo = limb_add_overflow(data1[0], l2, &lo);
            // Add the carry from the low addition to the high limb, placing the result in hi.
            const ::mp_limb_t cy_hi = limb_add_overflow(data1[1], cy_lo, &hi);
            if (mppp_unlikely(cy_hi)) {
                return false;
            }
            // Compute the new size. It can be 0, 1 or 2.
            rop._mp_size = hi ? 2 : (lo ? 1 : 0);
            // Write out.
            rdata[0] = lo;
            rdata[1] = hi;
        } else {
            // op1 is negative, l2 is non-negative. Compare their absolute values.
            if (asize1 == 2 || data1[0] >= l2) {
                // op1 is >= op2 in absolute value.
                const auto lo = data1[0] - l2;
                // Sub from hi the borrow.
                const auto hi = data1[1] - static_cast<::mp_limb_t>(data1[0] < l2);
                // The final size can be -2, -1 or 0.
                rop._mp_size = hi ? -2 : (lo ? -1 : 0);
                rdata[0] = lo;
                rdata[1] = hi;
            } else {
                // op2 > op1 in absolute value.
                // Size has to be 1.
                rop._mp_size = 1;
                rdata[0] = l2 - data1[0];
                rdata[1] = 0u;
            }
        }
        return true;
    }
    static bool static_add_ui(s_int &rop, const s_int &op1, unsigned long op2)
    {
        mpz_size_t asize1 = op1._mp_size;
        int sign1 = asize1 != 0;
        if (asize1 < 0) {
            asize1 = -asize1;
            sign1 = -1;
        }
        const bool retval = static_add_ui_impl(rop, op1, asize1, sign1, op2, static_add_ui_algo<s_int>{});
        if (static_add_ui_algo<s_int>::value == 0 && retval) {
            // If we used the mpn functions and we actually wrote into rop, then
            // make sure we zero out the unused limbs.
            rop.zero_unused_limbs();
        }
        return retval;
    }

public:
    /// Ternary add with <tt>unsigned long</tt>.
    /**
     * This function will set \p rop to <tt>op1 + op2</tt>.
     *
     * @param rop the return value.
     * @param op1 the first argument.
     * @param op2 the second argument.
     */
    friend void add_ui(mp_integer &rop, const mp_integer &op1, unsigned long op2)
    {
        if (std::numeric_limits<unsigned long>::max() > GMP_NUMB_MASK) {
            // For the optimised version below to kick in we need to be sure we can safely convert
            // unsigned long to an ::mp_limb_t, modulo nail bits. This because in the optimised version
            // we cast forcibly op2 to ::mp_limb_t. Otherwise, we just call add() after converting op2 to an
            // mp_integer.
            add(rop, op1, mp_integer{op2});
            return;
        }
        const bool sr = rop.is_static(), s1 = op1.is_static();
        if (mppp_likely(sr && s1)) {
            // Optimise the case of all statics.
            if (mppp_likely(static_add_ui(rop.m_int.g_st(), op1.m_int.g_st(), op2))) {
                return;
            }
        }
        if (sr) {
            rop.m_int.promote(SSize + 1u);
        }
        ::mpz_add_ui(&rop.m_int.g_dy(), op1.get_mpz_view(), op2);
    }

private:
    // The double limb multiplication optimization is available in the following cases:
    // - no nails, we are on a 64bit MSVC build, the limb type has exactly 64 bits and GMP_NUMB_BITS is 64,
    // - no nails, we have a 128bit unsigned available, the limb type has exactly 64 bits and GMP_NUMB_BITS is 64,
    // - no nails, the smallest 64 bit unsigned type has exactly 64 bits, the limb type has exactly 32 bits and
    //   GMP_NUMB_BITS is 32.
    // NOTE: here we are checking that GMP_NUMB_BITS is the same as the limits ::digits property, which is probably
    // rather redundant on any conceivable architecture.
    using have_dlimb_mul = std::integral_constant<bool,
#if (defined(_MSC_VER) && defined(_WIN64) && (GMP_NUMB_BITS == 64)) || (defined(MPPP_UINT128) && (GMP_NUMB_BITS == 64))
                                                  !GMP_NAIL_BITS && std::numeric_limits<::mp_limb_t>::digits == 64
#elif GMP_NUMB_BITS == 32
                                                  !GMP_NAIL_BITS
                                                      && std::numeric_limits<std::uint_least64_t>::digits == 64
                                                      && std::numeric_limits<::mp_limb_t>::digits == 32
#else
                                                  false
#endif
                                                  >;
    template <typename SInt>
    using static_mul_algo = std::integral_constant<int, (SInt::s_size == 1 && have_dlimb_mul::value)
                                                            ? 1
                                                            : ((SInt::s_size == 2 && have_dlimb_mul::value) ? 2 : 0)>;
    // mpn implementation.
    // NOTE: this function (and the other overloads) returns 0 in case of success, otherwise it returns a hint
    // about the size in limbs of the result.
    static std::size_t static_mul_impl(s_int &rop, const s_int &op1, const s_int &op2, mpz_size_t asize1,
                                       mpz_size_t asize2, int sign1, int sign2, const std::integral_constant<int, 0> &)
    {
        using mppp_impl::copy_limbs_no;
        // Handle zeroes.
        if (mppp_unlikely(!sign1 || !sign2)) {
            rop._mp_size = 0;
            return 0u;
        }
        auto rdata = &rop.m_limbs[0];
        auto data1 = &op1.m_limbs[0], data2 = &op2.m_limbs[0];
        const auto max_asize = std::size_t(asize1 + asize2);
        // Temporary storage, to be used if we cannot write into rop.
        std::array<::mp_limb_t, SSize * 2u> res;
        // We can write directly into rop if these conditions hold:
        // - rop does not overlap with op1 and op2,
        // - SSize is large enough to hold the max size of the result.
        ::mp_limb_t *MPPP_RESTRICT res_data
            = (rdata != data1 && rdata != data2 && max_asize <= SSize) ? rdata : res.data();
        // Proceed to the multiplication.
        ::mp_limb_t hi;
        if (asize2 == 1) {
            // NOTE: the 1-limb versions do not write the hi limb, we have to write it ourselves.
            hi = ::mpn_mul_1(res_data, data1, static_cast<::mp_size_t>(asize1), data2[0]);
            res_data[asize1] = hi;
        } else if (asize1 == 1) {
            hi = ::mpn_mul_1(res_data, data2, static_cast<::mp_size_t>(asize2), data1[0]);
            res_data[asize2] = hi;
        } else if (asize1 == asize2) {
            ::mpn_mul_n(res_data, data1, data2, static_cast<::mp_size_t>(asize1));
            hi = res_data[2 * asize1 - 1];
        } else if (asize1 >= asize2) {
            hi = ::mpn_mul(res_data, data1, static_cast<::mp_size_t>(asize1), data2, static_cast<::mp_size_t>(asize2));
        } else {
            hi = ::mpn_mul(res_data, data2, static_cast<::mp_size_t>(asize2), data1, static_cast<::mp_size_t>(asize1));
        }
        // The actual size.
        const std::size_t asize = max_asize - unsigned(hi == 0u);
        if (res_data == rdata) {
            // If we wrote directly into rop, it means that we had enough storage in it to begin with.
            rop._mp_size = mpz_size_t(asize);
            if (sign1 != sign2) {
                rop._mp_size = -rop._mp_size;
            }
            return 0u;
        }
        // If we used the temporary storage, we need to check if we can write into rop.
        if (asize > SSize) {
            // Not enough space, return the size.
            return asize;
        }
        // Enough space, set size and copy limbs.
        rop._mp_size = mpz_size_t(asize);
        if (sign1 != sign2) {
            rop._mp_size = -rop._mp_size;
        }
        copy_limbs_no(res_data, res_data + asize, rdata);
        return 0u;
    }
#if defined(_MSC_VER) && defined(_WIN64) && (GMP_NUMB_BITS == 64) && !GMP_NAIL_BITS
    static ::mp_limb_t dlimb_mul(::mp_limb_t op1, ::mp_limb_t op2, ::mp_limb_t *hi)
    {
        return ::UnsignedMultiply128(op1, op2, hi);
    }
#elif defined(MPPP_UINT128) && (GMP_NUMB_BITS == 64) && !GMP_NAIL_BITS
    static ::mp_limb_t dlimb_mul(::mp_limb_t op1, ::mp_limb_t op2, ::mp_limb_t *hi)
    {
        using dlimb_t = MPPP_UINT128;
        const dlimb_t res = dlimb_t(op1) * op2;
        *hi = static_cast<::mp_limb_t>(res >> 64);
        return static_cast<::mp_limb_t>(res);
    }
#elif GMP_NUMB_BITS == 32 && !GMP_NAIL_BITS
    static ::mp_limb_t dlimb_mul(::mp_limb_t op1, ::mp_limb_t op2, ::mp_limb_t *hi)
    {
        using dlimb_t = std::uint_least64_t;
        const dlimb_t res = dlimb_t(op1) * op2;
        *hi = static_cast<::mp_limb_t>(res >> 32);
        return static_cast<::mp_limb_t>(res);
    }
#endif
    // 1-limb optimization via dlimb.
    static std::size_t static_mul_impl(s_int &rop, const s_int &op1, const s_int &op2, mpz_size_t, mpz_size_t,
                                       int sign1, int sign2, const std::integral_constant<int, 1> &)
    {
        ::mp_limb_t hi;
        const ::mp_limb_t lo = dlimb_mul(op1.m_limbs[0], op2.m_limbs[0], &hi);
        if (mppp_unlikely(hi)) {
            return 2u;
        }
        const mpz_size_t asize = (lo != 0u);
        rop._mp_size = asize;
        if (sign1 != sign2) {
            rop._mp_size = -rop._mp_size;
        }
        rop.m_limbs[0] = lo;
        return 0u;
    }
    // 2-limb optimization via dlimb.
    static std::size_t static_mul_impl(s_int &rop, const s_int &op1, const s_int &op2, mpz_size_t asize1,
                                       mpz_size_t asize2, int sign1, int sign2, const std::integral_constant<int, 2> &)
    {
        using mppp_impl::limb_add_overflow;
        if (mppp_unlikely(!asize1 || !asize2)) {
            // Handle zeroes.
            rop._mp_size = 0;
            rop.m_limbs[0] = 0u;
            rop.m_limbs[1] = 0u;
            return 0u;
        }
        if (asize1 == 1 && asize2 == 1) {
            rop.m_limbs[0] = dlimb_mul(op1.m_limbs[0], op2.m_limbs[0], &rop.m_limbs[1]);
            rop._mp_size = static_cast<mpz_size_t>((asize1 + asize2) - mpz_size_t(rop.m_limbs[1] == 0u));
            if (sign1 != sign2) {
                rop._mp_size = -rop._mp_size;
            }
            return 0u;
        }
        if (asize1 != asize2) {
            // The only possibility of success is 2limbs x 1limb.
            //
            //             b      a X
            //                    c
            // --------------------
            // tmp[2] tmp[1] tmp[0]
            //
            ::mp_limb_t a = op1.m_limbs[0], b = op1.m_limbs[1], c = op2.m_limbs[0];
            if (asize1 < asize2) {
                // Switch them around if needed.
                a = op2.m_limbs[0], b = op2.m_limbs[1], c = op1.m_limbs[0];
            }
            ::mp_limb_t ca_lo, ca_hi, cb_lo, cb_hi, tmp0, tmp1, tmp2;
            ca_lo = dlimb_mul(c, a, &ca_hi);
            cb_lo = dlimb_mul(c, b, &cb_hi);
            tmp0 = ca_lo;
            const auto cy = limb_add_overflow(cb_lo, ca_hi, &tmp1);
            tmp2 = cb_hi + cy;
            // Now determine the size. asize must be at least 2.
            const mpz_size_t asize = 2 + mpz_size_t(tmp2 != 0u);
            if (asize == 2) {
                // Size is good, write out the result.
                rop._mp_size = asize;
                if (sign1 != sign2) {
                    rop._mp_size = -rop._mp_size;
                }
                rop.m_limbs[0] = tmp0;
                rop.m_limbs[1] = tmp1;
                return 0u;
            }
        }
        // Return 4 as a size hint. Real size could be 3, but GMP will require 4 limbs
        // of storage to perform the operation anyway.
        return 4u;
    }
    static std::size_t static_mul(s_int &rop, const s_int &op1, const s_int &op2)
    {
        // Cache a few quantities, detect signs.
        mpz_size_t asize1 = op1._mp_size, asize2 = op2._mp_size;
        int sign1 = asize1 != 0, sign2 = asize2 != 0;
        if (asize1 < 0) {
            asize1 = -asize1;
            sign1 = -1;
        }
        if (asize2 < 0) {
            asize2 = -asize2;
            sign2 = -1;
        }
        const std::size_t retval
            = static_mul_impl(rop, op1, op2, asize1, asize2, sign1, sign2, static_mul_algo<s_int>{});
        if (static_mul_algo<s_int>::value == 0 && retval == 0u) {
            rop.zero_unused_limbs();
        }
        return retval;
    }
    // Dispatching for the binary multiplication operator.
    static mp_integer dispatch_binary_mul(const mp_integer &op1, const mp_integer &op2)
    {
        mp_integer retval;
        mul(retval, op1, op2);
        return retval;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static mp_integer dispatch_binary_mul(const mp_integer &op1, T n)
    {
        // NOTE: with respect to addition, here we separate the retval
        // from the operands. Having a separate destination is generally better
        // for multiplication.
        mp_integer retval;
        mul(retval, op1, mp_integer{n});
        return retval;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static mp_integer dispatch_binary_mul(T n, const mp_integer &op2)
    {
        return dispatch_binary_mul(op2, n);
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static T dispatch_binary_mul(const mp_integer &op1, T x)
    {
        return static_cast<T>(op1) * x;
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static T dispatch_binary_mul(T x, const mp_integer &op2)
    {
        return dispatch_binary_mul(op2, x);
    }
    // Dispatching for in-place multiplication.
    static void dispatch_in_place_mul(mp_integer &retval, const mp_integer &n)
    {
        mul(retval, retval, n);
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static void dispatch_in_place_mul(mp_integer &retval, const T &n)
    {
        mul(retval, retval, mp_integer{n});
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static void dispatch_in_place_mul(mp_integer &retval, const T &x)
    {
        retval = static_cast<T>(retval) * x;
    }

public:
    /// Ternary multiplication.
    /**
     * This function will set \p rop to <tt>op1 * op2</tt>.
     *
     * @param rop the return value.
     * @param op1 the first argument.
     * @param op2 the second argument.
     */
    friend void mul(mp_integer &rop, const mp_integer &op1, const mp_integer &op2)
    {
        const bool sr = rop.is_static(), s1 = op1.is_static(), s2 = op2.is_static();
        std::size_t size_hint = 0u;
        if (mppp_likely(sr && s1 && s2)) {
            size_hint = static_mul(rop.m_int.g_st(), op1.m_int.g_st(), op2.m_int.g_st());
            if (mppp_likely(size_hint == 0u)) {
                return;
            }
        }
        if (sr) {
            // We use the size hint from the static_mul if available, otherwise a normal promotion will take place.
            // NOTE: here the best way of proceeding would be to calculate the max size of the result based on
            // the op1/op2 sizes, but for whatever reason this computation has disastrous performance consequences
            // on micro-benchmarks. We need to understand if that's the case in real-world scenarios as well, and
            // revisit this.
            rop.m_int.promote(size_hint);
        }
        ::mpz_mul(&rop.m_int.g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    }
    /// Binary multiplication operator.
    /**
     * @param op1 the first argument.
     * @param op2 the second argument.
     *
     * @return <tt>op1 * op2</tt>.
     */
    template <typename T, typename U>
    friend common_t<T, U> operator*(const T &op1, const U &op2)
    {
        return dispatch_binary_mul(op1, op2);
    }
    /// In-place multiplication operator.
    /**
     * @param op the multiplicand.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the assignment of a floating-point value to mp_integer, iff \p T
     * is a floating-point type.
     */
    template <typename T, in_place_enabler<T> = 0>
    mp_integer &operator*=(const T &op)
    {
        dispatch_in_place_mul(*this, op);
        return *this;
    }
#if defined(_MSC_VER)
    template <typename T>
    friend in_place_lenabler<T> operator*=(T &x, const mp_integer &n)
#else
    /// In-place multiplication for interoperable types.
    /**
     * \verbatim embed:rst:leading-asterisk
     * .. note::
     *
     *    This operator is enabled only if ``T`` is an interoperable type for :cpp:class:`mppp::mp_integer`.
     * \endverbatim
     *
     * The body of this operator is equivalent to:
     * \verbatim embed:rst:leading-asterisk
     * .. code-block:: c++
     *
     *    return x = static_cast<T>(x * n);
     * \endverbatim
     *
     * That is, the result of the corresponding binary operation is cast back to \p T and assigned to \p x.
     *
     * @param x the first argument.
     * @param n the second argument.
     *
     * @return a reference to \p x.
     *
     * @throws unspecified any exception thrown by the conversion operator of mppp::mp_integer.
     */
    template <typename T, in_place_lenabler<T> = 0>
    friend T &operator*=(T &x, const mp_integer &n)
#endif
    {
        return x = static_cast<T>(x * n);
    }

private:
    // Selection of the algorithm for addmul: if optimised algorithms exist for both add and mul, then use the
    // optimised addmul algos. Otherwise, use the mpn one.
    template <typename SInt>
    using static_addmul_algo
        = std::integral_constant<int,
                                 (static_add_algo<SInt>::value == 2 && static_mul_algo<SInt>::value == 2)
                                     ? 2
                                     : ((static_add_algo<SInt>::value == 1 && static_mul_algo<SInt>::value == 1) ? 1
                                                                                                                 : 0)>;
    // NOTE: same return value as mul: 0 for success, otherwise a hint for the size of the result.
    static std::size_t static_addmul_impl(s_int &rop, const s_int &op1, const s_int &op2, mpz_size_t asizer,
                                          mpz_size_t asize1, mpz_size_t asize2, int signr, int sign1, int sign2,
                                          const std::integral_constant<int, 0> &)
    {
        // First try to do the static prod, if it does not work it's a failure.
        s_int prod;
        if (mppp_unlikely(
                static_mul_impl(prod, op1, op2, asize1, asize2, sign1, sign2, std::integral_constant<int, 0>{}))) {
            // This is the maximum size a static addmul can have.
            return SSize * 2u + 1u;
        }
        // Determine sign and asize of the product.
        mpz_size_t asize_prod = prod._mp_size;
        int sign_prod = (asize_prod != 0);
        if (asize_prod < 0) {
            asize_prod = -asize_prod;
            sign_prod = -1;
        }
        // Try to do the add.
        if (mppp_unlikely(!static_add_impl(rop, rop, prod, asizer, asize_prod, signr, sign_prod,
                                           std::integral_constant<int, 0>{}))) {
            return SSize + 1u;
        }
        return 0u;
    }
    static std::size_t static_addmul_impl(s_int &rop, const s_int &op1, const s_int &op2, mpz_size_t, mpz_size_t,
                                          mpz_size_t, int signr, int sign1, int sign2,
                                          const std::integral_constant<int, 1> &)
    {
        using mppp_impl::limb_add_overflow;
        // First we do op1 * op2.
        ::mp_limb_t tmp;
        const ::mp_limb_t prod = dlimb_mul(op1.m_limbs[0], op2.m_limbs[0], &tmp);
        if (mppp_unlikely(tmp)) {
            return 3u;
        }
        // Determine the sign of the product: 0, 1 or -1.
        int sign_prod = prod != 0u;
        if (sign1 != sign2) {
            sign_prod = -sign_prod;
        }
        // Now add/sub.
        if (signr == sign_prod) {
            // Same sign, do addition with overflow check.
            if (mppp_unlikely(limb_add_overflow(rop.m_limbs[0], prod, &tmp))) {
                return 2u;
            }
            // Assign the output.
            rop._mp_size = signr;
            rop.m_limbs[0] = tmp;
        } else {
            // When the signs differ, we need to implement addition as a subtraction.
            if (rop.m_limbs[0] >= prod) {
                // abs(rop) >= abs(prod).
                tmp = rop.m_limbs[0] - prod;
                // asize is either 1 or 0 (0 iff rop == prod).
                rop._mp_size = signr;
                if (mppp_unlikely(!tmp)) {
                    rop._mp_size = 0;
                }
                rop.m_limbs[0] = tmp;
            } else {
                // NOTE: this cannot be zero, as rop and prod cannot be equal.
                rop._mp_size = sign_prod;
                rop.m_limbs[0] = prod - rop.m_limbs[0];
            }
        }
        return 0u;
    }
    static std::size_t static_addmul_impl(s_int &rop, const s_int &op1, const s_int &op2, mpz_size_t asizer,
                                          mpz_size_t asize1, mpz_size_t asize2, int signr, int sign1, int sign2,
                                          const std::integral_constant<int, 2> &)
    {
        using mppp_impl::limb_add_overflow;
        if (mppp_unlikely(!asize1 || !asize2)) {
            // If op1 or op2 are zero, rop will be unchanged.
            return 0u;
        }
        // Handle op1 * op2.
        std::array<::mp_limb_t, 2> prod;
        int sign_prod = 1;
        if (sign1 != sign2) {
            sign_prod = -1;
        }
        mpz_size_t asize_prod;
        if (asize1 == 1 && asize2 == 1) {
            prod[0] = dlimb_mul(op1.m_limbs[0], op2.m_limbs[0], &prod[1]);
            asize_prod = (asize1 + asize2) - mpz_size_t(prod[1] == 0u);
        } else {
            // The only possibility of success is 2limbs x 1limb.
            //
            //       b       a X
            //               c
            // ---------------
            // prod[1] prod[0]
            //
            if (mppp_unlikely(asize1 == asize2)) {
                // This means that both sizes are 2.
                return 5u;
            }
            ::mp_limb_t a = op1.m_limbs[0], b = op1.m_limbs[1], c = op2.m_limbs[0];
            if (asize1 < asize2) {
                // Switch them around if needed.
                a = op2.m_limbs[0], b = op2.m_limbs[1], c = op1.m_limbs[0];
            }
            ::mp_limb_t ca_hi, cb_hi;
            // These are the three operations that build the result, two mults, one add.
            prod[0] = dlimb_mul(c, a, &ca_hi);
            prod[1] = dlimb_mul(c, b, &cb_hi);
            // NOTE: we can use this with overlapping arguments because the first two
            // are passed as copies.
            const auto cy = limb_add_overflow(prod[1], ca_hi, &prod[1]);
            // Check if the third limb exists, if so we return failure.
            if (mppp_unlikely(cb_hi || cy)) {
                return 4u;
            }
            asize_prod = 2;
        }
        // Proceed to the addition.
        if (signr == sign_prod) {
            // Add the hi and lo limbs.
            ::mp_limb_t lo, hi1, hi2;
            const ::mp_limb_t cy_lo = limb_add_overflow(rop.m_limbs[0], prod[0], &lo),
                              cy_hi1 = limb_add_overflow(rop.m_limbs[1], prod[1], &hi1),
                              cy_hi2 = limb_add_overflow(hi1, cy_lo, &hi2);
            // The result will overflow if we have any carry relating to the high limb.
            if (mppp_unlikely(cy_hi1 || cy_hi2)) {
                return 3u;
            }
            // For the size compuation:
            // - cannot be zero, as prod is not zero,
            // - if signr == +-1, then the size is either +-1 or +-2: asize is 2 if the result
            //   has a nonzero 2nd limb, otherwise asize is 1.
            rop._mp_size = signr;
            if (hi2) {
                rop._mp_size = signr + signr;
            }
            rop.m_limbs[0] = lo;
            rop.m_limbs[1] = hi2;
        } else {
            // When the signs differ, we need to implement addition as a subtraction.
            if (asizer > asize_prod
                || (asizer == asize_prod && compare_limbs_2(&rop.m_limbs[0], &prod[0], asizer) >= 0)) {
                // rop >= prod in absolute value.
                const auto lo = rop.m_limbs[0] - prod[0];
                // If there's a borrow, then hi1 > hi2, otherwise we would have a negative result.
                assert(rop.m_limbs[0] >= prod[0] || rop.m_limbs[1] > prod[1]);
                // This can never wrap around, at most it goes to zero.
                const auto hi = rop.m_limbs[1] - prod[1] - static_cast<::mp_limb_t>(rop.m_limbs[0] < prod[0]);
                // asize can be 0, 1 or 2.
                rop._mp_size = 0;
                if (hi) {
                    rop._mp_size = signr + signr;
                } else if (lo) {
                    rop._mp_size = signr;
                }
                rop.m_limbs[0] = lo;
                rop.m_limbs[1] = hi;
            } else {
                // prod > rop in absolute value.
                const auto lo = prod[0] - rop.m_limbs[0];
                assert(prod[0] >= rop.m_limbs[0] || prod[1] > rop.m_limbs[1]);
                const auto hi = prod[1] - rop.m_limbs[1] - static_cast<::mp_limb_t>(prod[0] < rop.m_limbs[0]);
                // asize can be 1 or 2, but not zero as we know abs(prod) != abs(rop).
                rop._mp_size = sign_prod;
                if (hi) {
                    rop._mp_size = sign_prod + sign_prod;
                }
                rop.m_limbs[0] = lo;
                rop.m_limbs[1] = hi;
            }
        }
        return 0u;
    }
    static std::size_t static_addmul(s_int &rop, const s_int &op1, const s_int &op2)
    {
        mpz_size_t asizer = rop._mp_size, asize1 = op1._mp_size, asize2 = op2._mp_size;
        int signr = asizer != 0, sign1 = asize1 != 0, sign2 = asize2 != 0;
        if (asizer < 0) {
            asizer = -asizer;
            signr = -1;
        }
        if (asize1 < 0) {
            asize1 = -asize1;
            sign1 = -1;
        }
        if (asize2 < 0) {
            asize2 = -asize2;
            sign2 = -1;
        }
        const std::size_t retval = static_addmul_impl(rop, op1, op2, asizer, asize1, asize2, signr, sign1, sign2,
                                                      static_addmul_algo<s_int>{});
        if (static_addmul_algo<s_int>::value == 0 && retval == 0u) {
            rop.zero_unused_limbs();
        }
        return retval;
    }

public:
    /// Ternary multiply–accumulate.
    /**
     * This function will set \p rop to <tt>rop + op1 * op2</tt>.
     *
     * @param rop the return value.
     * @param op1 the first argument.
     * @param op2 the second argument.
     */
    friend void addmul(mp_integer &rop, const mp_integer &op1, const mp_integer &op2)
    {
        const bool sr = rop.is_static(), s1 = op1.is_static(), s2 = op2.is_static();
        std::size_t size_hint = 0u;
        if (mppp_likely(sr && s1 && s2)) {
            size_hint = static_addmul(rop.m_int.g_st(), op1.m_int.g_st(), op2.m_int.g_st());
            if (mppp_likely(size_hint == 0u)) {
                return;
            }
        }
        if (sr) {
            rop.m_int.promote(size_hint);
        }
        ::mpz_addmul(&rop.m_int.g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    }

private:
    // Detect the presence of dual-limb division. This is currently possible only if:
    // - we are on a 32bit build (with the usual constraints that the types have exactly 32/64 bits and no nails),
    // - we are on a 64bit build and we have the 128bit int type available (plus usual constraints).
    // MSVC currently does not provide any primitive for 128bit division.
    using have_dlimb_div = std::integral_constant<bool,
#if defined(MPPP_UINT128) && (GMP_NUMB_BITS == 64)
                                                  !GMP_NAIL_BITS && std::numeric_limits<::mp_limb_t>::digits == 64
#elif GMP_NUMB_BITS == 32
                                                  !GMP_NAIL_BITS
                                                      && std::numeric_limits<std::uint_least64_t>::digits == 64
                                                      && std::numeric_limits<::mp_limb_t>::digits == 32
#else
                                                  false
#endif
                                                  >;
    // Selection of algorithm for static division:
    // - for 1 limb, we can always do static division,
    // - for 2 limbs, we need the dual limb division if avaiable,
    // - otherwise we just use the mpn functions.
    template <typename SInt>
    using static_div_algo
        = std::integral_constant<int, SInt::s_size == 1 ? 1 : ((SInt::s_size == 2 && have_dlimb_div::value) ? 2 : 0)>;
    // mpn implementation.
    static void static_div_impl(s_int &q, s_int &r, const s_int &op1, const s_int &op2, mpz_size_t asize1,
                                mpz_size_t asize2, int sign1, int sign2, const std::integral_constant<int, 0> &)
    {
        using mppp_impl::copy_limbs_no;
        // First we check if the divisor is larger than the dividend (in abs limb size), as the mpn function
        // requires asize1 >= asize2.
        if (asize2 > asize1) {
            // Copy op1 into the remainder.
            r = op1;
            // Zero out q.
            q._mp_size = 0;
            return;
        }
        // We need to take care of potentially overlapping arguments. We know that q and r are distinct, but op1
        // could overlap with q or r, and op2 could overlap with op1, q or r.
        std::array<::mp_limb_t, SSize> op1_alt, op2_alt;
        const ::mp_limb_t *data1 = op1.m_limbs.data();
        const ::mp_limb_t *data2 = op2.m_limbs.data();
        if (&op1 == &q || &op1 == &r) {
            copy_limbs_no(data1, data1 + asize1, op1_alt.data());
            data1 = op1_alt.data();
        }
        if (&op2 == &q || &op2 == &r || &op1 == &op2) {
            copy_limbs_no(data2, data2 + asize2, op2_alt.data());
            data2 = op2_alt.data();
        }
        // Small helper function to verify that all pointers are distinct. Used exclusively for debugging purposes.
        auto distinct_op = [&q, &r, data1, data2]() -> bool {
            const ::mp_limb_t *ptrs[] = {q.m_limbs.data(), r.m_limbs.data(), data1, data2};
            std::sort(ptrs, ptrs + 4, std::less<const ::mp_limb_t *>());
            return std::unique(ptrs, ptrs + 4) == (ptrs + 4);
        };
        (void)distinct_op;
        assert(distinct_op());
        // Proceed to the division.
        if (asize2 == 1) {
            // Optimization when the divisor has 1 limb.
            r.m_limbs[0]
                = ::mpn_divrem_1(q.m_limbs.data(), ::mp_size_t(0), data1, static_cast<::mp_size_t>(asize1), data2[0]);
        } else {
            // General implementation.
            ::mpn_tdiv_qr(q.m_limbs.data(), r.m_limbs.data(), ::mp_size_t(0), data1, static_cast<::mp_size_t>(asize1),
                          data2, static_cast<::mp_size_t>(asize2));
        }
        // Complete the quotient: compute size and sign.
        q._mp_size = asize1 - asize2 + 1;
        while (q._mp_size && !(q.m_limbs[static_cast<std::size_t>(q._mp_size - 1)] & GMP_NUMB_MASK)) {
            --q._mp_size;
        }
        if (sign1 != sign2) {
            q._mp_size = -q._mp_size;
        }
        // Complete the remainder.
        r._mp_size = asize2;
        while (r._mp_size && !(r.m_limbs[static_cast<std::size_t>(r._mp_size - 1)] & GMP_NUMB_MASK)) {
            --r._mp_size;
        }
        if (sign1 == -1) {
            r._mp_size = -r._mp_size;
        }
    }
    // 1-limb optimisation.
    static void static_div_impl(s_int &q, s_int &r, const s_int &op1, const s_int &op2, mpz_size_t, mpz_size_t,
                                int sign1, int sign2, const std::integral_constant<int, 1> &)
    {
        // NOTE: here we have to use GMP_NUMB_MASK because if s_size is 1 this implementation is *always*
        // called, even if we have nail bits (whereas the optimisation for other operations currently kicks in
        // only without nail bits). Thus, we need to discard from m_limbs[0] the nail bits before doing the
        // division.
        const ::mp_limb_t q_ = (op1.m_limbs[0] & GMP_NUMB_MASK) / (op2.m_limbs[0] & GMP_NUMB_MASK),
                          r_ = (op1.m_limbs[0] & GMP_NUMB_MASK) % (op2.m_limbs[0] & GMP_NUMB_MASK);
        // Write q.
        q._mp_size = (q_ != 0u);
        if (sign1 != sign2) {
            q._mp_size = -q._mp_size;
        }
        // NOTE: there should be no need here to mask.
        q.m_limbs[0] = q_;
        // Write r.
        r._mp_size = (r_ != 0u);
        // Following C++11, the sign of r is the sign of op1:
        // http://stackoverflow.com/questions/13100711/operator-modulo-change-in-c-11
        if (sign1 == -1) {
            r._mp_size = -r._mp_size;
        }
        r.m_limbs[0] = r_;
    }
#if defined(MPPP_UINT128) && (GMP_NUMB_BITS == 64) && !GMP_NAIL_BITS
    static void dlimb_div(::mp_limb_t op11, ::mp_limb_t op12, ::mp_limb_t op21, ::mp_limb_t op22,
                          ::mp_limb_t *MPPP_RESTRICT q1, ::mp_limb_t *MPPP_RESTRICT q2, ::mp_limb_t *MPPP_RESTRICT r1,
                          ::mp_limb_t *MPPP_RESTRICT r2)
    {
        using dlimb_t = MPPP_UINT128;
        const auto op1 = op11 + (dlimb_t(op12) << 64);
        const auto op2 = op21 + (dlimb_t(op22) << 64);
        const auto q = op1 / op2, r = op1 % op2;
        *q1 = static_cast<::mp_limb_t>(q & ::mp_limb_t(-1));
        *q2 = static_cast<::mp_limb_t>(q >> 64);
        *r1 = static_cast<::mp_limb_t>(r & ::mp_limb_t(-1));
        *r2 = static_cast<::mp_limb_t>(r >> 64);
    }
    // Without remainder.
    static void dlimb_div(::mp_limb_t op11, ::mp_limb_t op12, ::mp_limb_t op21, ::mp_limb_t op22,
                          ::mp_limb_t *MPPP_RESTRICT q1, ::mp_limb_t *MPPP_RESTRICT q2)
    {
        using dlimb_t = MPPP_UINT128;
        const auto op1 = op11 + (dlimb_t(op12) << 64);
        const auto op2 = op21 + (dlimb_t(op22) << 64);
        const auto q = op1 / op2;
        *q1 = static_cast<::mp_limb_t>(q & ::mp_limb_t(-1));
        *q2 = static_cast<::mp_limb_t>(q >> 64);
    }
#elif GMP_NUMB_BITS == 32 && !GMP_NAIL_BITS
    static void dlimb_div(::mp_limb_t op11, ::mp_limb_t op12, ::mp_limb_t op21, ::mp_limb_t op22,
                          ::mp_limb_t *MPPP_RESTRICT q1, ::mp_limb_t *MPPP_RESTRICT q2, ::mp_limb_t *MPPP_RESTRICT r1,
                          ::mp_limb_t *MPPP_RESTRICT r2)
    {
        using dlimb_t = std::uint_least64_t;
        const auto op1 = op11 + (dlimb_t(op12) << 32);
        const auto op2 = op21 + (dlimb_t(op22) << 32);
        const auto q = op1 / op2, r = op1 % op2;
        *q1 = static_cast<::mp_limb_t>(q & ::mp_limb_t(-1));
        *q2 = static_cast<::mp_limb_t>(q >> 32);
        *r1 = static_cast<::mp_limb_t>(r & ::mp_limb_t(-1));
        *r2 = static_cast<::mp_limb_t>(r >> 32);
    }
    static void dlimb_div(::mp_limb_t op11, ::mp_limb_t op12, ::mp_limb_t op21, ::mp_limb_t op22,
                          ::mp_limb_t *MPPP_RESTRICT q1, ::mp_limb_t *MPPP_RESTRICT q2)
    {
        using dlimb_t = std::uint_least64_t;
        const auto op1 = op11 + (dlimb_t(op12) << 32);
        const auto op2 = op21 + (dlimb_t(op22) << 32);
        const auto q = op1 / op2;
        *q1 = static_cast<::mp_limb_t>(q & ::mp_limb_t(-1));
        *q2 = static_cast<::mp_limb_t>(q >> 32);
    }
#endif
    // 2-limbs optimisation.
    static void static_div_impl(s_int &q, s_int &r, const s_int &op1, const s_int &op2, mpz_size_t asize1,
                                mpz_size_t asize2, int sign1, int sign2, const std::integral_constant<int, 2> &)
    {
        if (asize1 < 2 && asize2 < 2) {
            // NOTE: testing indicates that it pays off to optimize the case in which the operands have
            // fewer than 2 limbs. This a slightly modified version of the 1-limb division from above,
            // without the need to mask as this function is called only if there are no nail bits.
            const ::mp_limb_t q_ = op1.m_limbs[0] / op2.m_limbs[0], r_ = op1.m_limbs[0] % op2.m_limbs[0];
            q._mp_size = (q_ != 0u);
            if (sign1 != sign2) {
                q._mp_size = -q._mp_size;
            }
            q.m_limbs[0] = q_;
            q.m_limbs[1] = 0u;
            r._mp_size = (r_ != 0u);
            if (sign1 == -1) {
                r._mp_size = -r._mp_size;
            }
            r.m_limbs[0] = r_;
            r.m_limbs[1] = 0u;
            return;
        }
        // Perform the division.
        ::mp_limb_t q1, q2, r1, r2;
        dlimb_div(op1.m_limbs[0], op1.m_limbs[1], op2.m_limbs[0], op2.m_limbs[1], &q1, &q2, &r1, &r2);
        // Write out.
        q._mp_size = q2 ? 2 : (q1 ? 1 : 0);
        if (sign1 != sign2) {
            q._mp_size = -q._mp_size;
        }
        q.m_limbs[0] = q1;
        q.m_limbs[1] = q2;
        r._mp_size = r2 ? 2 : (r1 ? 1 : 0);
        if (sign1 == -1) {
            r._mp_size = -r._mp_size;
        }
        r.m_limbs[0] = r1;
        r.m_limbs[1] = r2;
    }
    static void static_div(s_int &q, s_int &r, const s_int &op1, const s_int &op2)
    {
        mpz_size_t asize1 = op1._mp_size, asize2 = op2._mp_size;
        int sign1 = asize1 != 0, sign2 = asize2 != 0;
        if (asize1 < 0) {
            asize1 = -asize1;
            sign1 = -1;
        }
        if (asize2 < 0) {
            asize2 = -asize2;
            sign2 = -1;
        }
        static_div_impl(q, r, op1, op2, asize1, asize2, sign1, sign2, static_div_algo<s_int>{});
        if (static_div_algo<s_int>::value == 0) {
            q.zero_unused_limbs();
            r.zero_unused_limbs();
        }
    }
    // Dispatching for the binary division operator.
    static mp_integer dispatch_binary_div(const mp_integer &op1, const mp_integer &op2)
    {
        mp_integer retval, r;
        tdiv_qr(retval, r, op1, op2);
        return retval;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static mp_integer dispatch_binary_div(const mp_integer &op1, T n)
    {
        mp_integer retval, r;
        tdiv_qr(retval, r, op1, mp_integer{n});
        return retval;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static mp_integer dispatch_binary_div(T n, const mp_integer &op2)
    {
        mp_integer retval, r;
        tdiv_qr(retval, r, mp_integer{n}, op2);
        return retval;
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static T dispatch_binary_div(const mp_integer &op1, T x)
    {
        return static_cast<T>(op1) / x;
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static T dispatch_binary_div(T x, const mp_integer &op2)
    {
        return x / static_cast<T>(op2);
    }
    // Dispatching for in-place div.
    static void dispatch_in_place_div(mp_integer &retval, const mp_integer &n)
    {
        mp_integer r;
        tdiv_qr(retval, r, retval, n);
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static void dispatch_in_place_div(mp_integer &retval, const T &n)
    {
        mp_integer r;
        tdiv_qr(retval, r, retval, mp_integer{n});
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static void dispatch_in_place_div(mp_integer &retval, const T &x)
    {
        retval = static_cast<T>(retval) / x;
    }
    // Enablers for modulo operators. They are special because they don't accept floating-point.
    template <typename T, typename U>
    using common_mod_t
        = enable_if_t<conjunction<negation<std::is_floating_point<T>>, negation<std::is_floating_point<U>>>::value,
                      common_t<T, U>>;
    template <typename T>
    using in_place_mod_enabler
        = enable_if_t<disjunction<is_supported_integral<T>, std::is_same<T, mp_integer>>::value, int>;
    // Dispatching for the binary modulo operator.
    static mp_integer dispatch_binary_mod(const mp_integer &op1, const mp_integer &op2)
    {
        mp_integer q, retval;
        tdiv_qr(q, retval, op1, op2);
        return retval;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static mp_integer dispatch_binary_mod(const mp_integer &op1, T n)
    {
        mp_integer q, retval;
        tdiv_qr(q, retval, op1, mp_integer{n});
        return retval;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static mp_integer dispatch_binary_mod(T n, const mp_integer &op2)
    {
        mp_integer q, retval;
        tdiv_qr(q, retval, mp_integer{n}, op2);
        return retval;
    }
    // Dispatching for in-place mod.
    static void dispatch_in_place_mod(mp_integer &retval, const mp_integer &n)
    {
        mp_integer q;
        tdiv_qr(q, retval, retval, n);
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static void dispatch_in_place_mod(mp_integer &retval, const T &n)
    {
        mp_integer q;
        tdiv_qr(q, retval, retval, mp_integer{n});
    }

public:
    /// Ternary truncated division.
    /**
     * This function will set \p q to the truncated quotient <tt>n / d</tt> and \p r to
     * <tt>n % d</tt>. The remainder \p r has the same sign as \p n. \p q and \p r must be two distinct objects.
     *
     * @param q the quotient.
     * @param r the remainder.
     * @param n the dividend.
     * @param d the divisor.
     *
     * @throws std::invalid_argument if \p q and \p r are the same object.
     * @throws zero_division_error if \p d is zero.
     */
    friend void tdiv_qr(mp_integer &q, mp_integer &r, const mp_integer &n, const mp_integer &d)
    {
        if (mppp_unlikely(&q == &r)) {
            throw std::invalid_argument("When performing a division with remainder, the quotient 'q' and the "
                                        "remainder 'r' must be distinct objects");
        }
        if (mppp_unlikely(d.sgn() == 0)) {
            throw zero_division_error("Integer division by zero");
        }
        const bool sq = q.is_static(), sr = r.is_static(), s1 = n.is_static(), s2 = d.is_static();
        if (mppp_likely(sq && sr && s1 && s2)) {
            static_div(q.m_int.g_st(), r.m_int.g_st(), n.m_int.g_st(), d.m_int.g_st());
            // Division can never fail.
            return;
        }
        if (sq) {
            q.m_int.promote();
        }
        if (sr) {
            r.m_int.promote();
        }
        ::mpz_tdiv_qr(&q.m_int.g_dy(), &r.m_int.g_dy(), n.get_mpz_view(), d.get_mpz_view());
    }
    /// Binary division operator.
    /**
     * @param n the dividend.
     * @param d the divisor.
     *
     * @return <tt>n / d</tt>. The result is truncated if only integral types are involved in the division.
     *
     * @throws zero_division_error if \p d is zero and only integral types are involved in the division.
     */
    template <typename T, typename U>
    friend common_t<T, U> operator/(const T &n, const U &d)
    {
        return dispatch_binary_div(n, d);
    }
    /// In-place division operator.
    /**
     * @param d the divisor.
     *
     * @return a reference to \p this. The result is truncated.
     *
     * @throws zero_division_error if \p d is zero and only integral types are involved in the division.
     * @throws unspecified any exception thrown by the assignment of a floating-point value to mp_integer, iff \p T
     * is a floating-point type.
     */
    template <typename T, in_place_enabler<T> = 0>
    mp_integer &operator/=(const T &d)
    {
        dispatch_in_place_div(*this, d);
        return *this;
    }
#if defined(_MSC_VER)
    template <typename T>
    friend in_place_lenabler<T> operator/=(T &x, const mp_integer &n)
#else
    /// In-place division for interoperable types.
    /**
     * \verbatim embed:rst:leading-asterisk
     * .. note::
     *
     *    This operator is enabled only if ``T`` is an interoperable type for :cpp:class:`mppp::mp_integer`.
     * \endverbatim
     *
     * The body of this operator is equivalent to:
     * \verbatim embed:rst:leading-asterisk
     * .. code-block:: c++
     *
     *    return x = static_cast<T>(x / n);
     * \endverbatim
     *
     * That is, the result of the corresponding binary operation is cast back to \p T and assigned to \p x.
     *
     * @param x the first argument.
     * @param n the second argument.
     *
     * @return a reference to \p x.
     *
     * @throws unspecified any exception thrown by the conversion operator of mppp::mp_integer or by
     * mppp::mp_integer::operator/().
     */
    template <typename T, in_place_lenabler<T> = 0>
    friend T &operator/=(T &x, const mp_integer &n)
#endif
    {
        return x = static_cast<T>(x / n);
    }
    /// Binary modulo operator.
    /**
     * @param n the dividend.
     * @param d the divisor.
     *
     * @return <tt>n % d</tt>.
     *
     * @throws zero_division_error if \p d is zero.
     */
    template <typename T, typename U>
    friend common_mod_t<T, U> operator%(const T &n, const U &d)
    {
        return dispatch_binary_mod(n, d);
    }
    /// In-place modulo operator.
    /**
     * @param d the divisor.
     *
     * @return a reference to \p this..
     *
     * @throws zero_division_error if \p d is zero.
     */
    template <typename T, in_place_mod_enabler<T> = 0>
    mp_integer &operator%=(const T &d)
    {
        dispatch_in_place_mod(*this, d);
        return *this;
    }

private:
    // Selection of the algorithm for static mul_2exp.
    template <typename SInt>
    using static_mul_2exp_algo = std::integral_constant<int, SInt::s_size == 1 ? 1 : (SInt::s_size == 2 ? 2 : 0)>;
    // mpn implementation.
    static std::size_t static_mul_2exp_impl(s_int &rop, const s_int &n, ::mp_bitcnt_t s,
                                            const std::integral_constant<int, 0> &)
    {
        using mppp_impl::copy_limbs_no;
        mpz_size_t asize = n._mp_size;
        if (s == 0u || asize == 0) {
            // If shift is zero, or the operand is zero, write n into rop and return success.
            rop = n;
            return 0u;
        }
        // Finish setting up asize and sign.
        int sign = asize != 0;
        if (asize < 0) {
            asize = -asize;
            sign = -1;
        }
        // ls: number of entire limbs shifted.
        // rs: effective shift that will be passed to the mpn function.
        const auto ls = s / GMP_NUMB_BITS, rs = s % GMP_NUMB_BITS;
        // At the very minimum, the new asize will be the old asize
        // plus ls.
        const mpz_size_t new_asize = asize + static_cast<mpz_size_t>(ls);
        if (std::size_t(new_asize) < SSize) {
            // In this case the operation will always succeed, and we can write directly into rop.
            ::mp_limb_t ret = 0u;
            if (rs) {
                // Perform the shift via the mpn function, if we are effectively shifting at least 1 bit.
                // Overlapping is fine, as it is guaranteed that rop.m_limbs.data() + ls >= n.m_limbs.data().
                ret = ::mpn_lshift(rop.m_limbs.data() + ls, n.m_limbs.data(), static_cast<::mp_size_t>(asize),
                                   unsigned(rs));
                // Write bits spilling out.
                rop.m_limbs[std::size_t(new_asize)] = ret;
            } else {
                // Otherwise, just copy over (the mpn function requires the shift to be at least 1).
                // NOTE: we have to use move_backward here because the ranges are overlapping and they do not
                // start at the same pointer (in which case we could've used copy_limbs()). Here we know ls is not
                // zero: we don't have a remainder, and s == 0 was already handled above. Hence, new_asize > asize.
                assert(new_asize > asize);
                std::move_backward(n.m_limbs.begin(), n.m_limbs.begin() + asize, rop.m_limbs.begin() + new_asize);
            }
            // Zero the lower limbs vacated by the shift.
            std::fill(rop.m_limbs.data(), rop.m_limbs.data() + ls, ::mp_limb_t(0));
            // Set the size.
            rop._mp_size = new_asize + (ret != 0u);
            if (sign == -1) {
                rop._mp_size = -rop._mp_size;
            }
            return 0u;
        }
        if (std::size_t(new_asize) == SSize) {
            if (rs) {
                // In this case the operation may fail, so we need to write to temporary storage.
                std::array<::mp_limb_t, SSize> tmp;
                if (::mpn_lshift(tmp.data(), n.m_limbs.data(), static_cast<::mp_size_t>(asize), unsigned(rs))) {
                    return SSize + 1u;
                }
                // The shift was successful without spill over, copy the content from the tmp
                // storage into rop.
                copy_limbs_no(tmp.data(), tmp.data() + asize, rop.m_limbs.data() + ls);
            } else {
                // If we shifted by a multiple of the limb size, then we can write directly to rop.
                assert(new_asize > asize);
                std::move_backward(n.m_limbs.begin(), n.m_limbs.begin() + asize, rop.m_limbs.begin() + new_asize);
            }
            // Zero the lower limbs vacated by the shift.
            std::fill(rop.m_limbs.data(), rop.m_limbs.data() + ls, ::mp_limb_t(0));
            // Set the size.
            rop._mp_size = new_asize;
            if (sign == -1) {
                rop._mp_size = -rop._mp_size;
            }
            return 0u;
        }
        // Shift is too much, the size will overflow the static limit.
        // Return a hint for the size of the result.
        return std::size_t(new_asize) + 1u;
    }
    // 1-limb optimisation.
    static std::size_t static_mul_2exp_impl(s_int &rop, const s_int &n, ::mp_bitcnt_t s,
                                            const std::integral_constant<int, 1> &)
    {
        const ::mp_limb_t l = n.m_limbs[0] & GMP_NUMB_MASK;
        if (s == 0u || l == 0u) {
            // If shift is zero, or the operand is zero, write n into rop and return success.
            rop = n;
            return 0u;
        }
        if (mppp_unlikely(s >= GMP_NUMB_BITS || (l >> (GMP_NUMB_BITS - s)))) {
            // The two conditions:
            // - if the shift is >= number of data bits, the operation will certainly
            //   fail (as the operand is nonzero);
            // - shifting by s would overflow  the single limb.
            // NOTE: s is at least 1, so in the right shift above we never risk UB due to too much shift.
            // NOTE: for the size hint: s / nbits is the number of entire limbs shifted, +1 because the shifted
            // limbs add to the current size (1), +1 because another limb might be needed.
            return std::size_t(s) / GMP_NUMB_BITS + 2u;
        }
        // Write out.
        rop.m_limbs[0] = l << s;
        // Size is the same as the input value.
        rop._mp_size = n._mp_size;
        return 0u;
    }
    // 2-limb optimisation.
    static std::size_t static_mul_2exp_impl(s_int &rop, const s_int &n, ::mp_bitcnt_t s,
                                            const std::integral_constant<int, 2> &)
    {
        mpz_size_t asize = n._mp_size;
        if (s == 0u || asize == 0) {
            rop = n;
            return 0u;
        }
        int sign = asize != 0;
        if (asize < 0) {
            asize = -asize;
            sign = -1;
        }
        // Too much shift, this can never work on a nonzero value.
        static_assert(GMP_NUMB_BITS
                          < std::numeric_limits<typename std::decay<decltype(GMP_NUMB_BITS)>::type>::max() / 2u,
                      "Overflow error.");
        if (mppp_unlikely(s >= 2u * GMP_NUMB_BITS)) {
            // NOTE: this is the generic formula to estimate the final size.
            return std::size_t(s) / GMP_NUMB_BITS + 1u + std::size_t(asize);
        }
        if (s == GMP_NUMB_BITS) {
            // This case can be dealt with moving lo into hi, but only if asize is 1.
            if (mppp_unlikely(asize == 2)) {
                // asize is 2, shift is too much.
                return 3u;
            }
            rop.m_limbs[1u] = n.m_limbs[0u];
            rop.m_limbs[0u] = 0u;
            // The size has to be 2.
            rop._mp_size = 2;
            if (sign == -1) {
                rop._mp_size = -2;
            }
            return 0u;
        }
        // Temp hi lo limbs to store the result that will eventually go into rop.
        ::mp_limb_t lo = n.m_limbs[0u], hi = n.m_limbs[1u];
        if (s > GMP_NUMB_BITS) {
            if (mppp_unlikely(asize == 2)) {
                return std::size_t(s) / GMP_NUMB_BITS + 1u + std::size_t(asize);
            }
            // Move lo to hi and set lo to zero.
            hi = n.m_limbs[0u];
            lo = 0u;
            // Update the shift.
            s = static_cast<::mp_bitcnt_t>(s - GMP_NUMB_BITS);
        }
        // Check that hi will not be shifted too much. Note that
        // here and below s can never be zero, so we never shift too much.
        assert(s > 0u && s < GMP_NUMB_BITS);
        if (mppp_unlikely((hi & GMP_NUMB_MASK) >> (GMP_NUMB_BITS - s))) {
            return 3u;
        }
        // Shift hi and lo. hi gets the carry over from lo.
        hi = ((hi & GMP_NUMB_MASK) << s) + ((lo & GMP_NUMB_MASK) >> (GMP_NUMB_BITS - s));
        // NOTE: here the result needs to be masked as well as the shift could
        // end up writing in nail bits.
        lo = ((lo & GMP_NUMB_MASK) << s) & GMP_NUMB_MASK;
        // Write rop.
        rop.m_limbs[0u] = lo;
        rop.m_limbs[1u] = hi;
        // asize is at least 1.
        rop._mp_size = 1 + (hi != 0u);
        if (sign == -1) {
            rop._mp_size = -rop._mp_size;
        }
        return 0u;
    }
    static std::size_t static_mul_2exp(s_int &rop, const s_int &n, ::mp_bitcnt_t s)
    {
        const std::size_t retval = static_mul_2exp_impl(rop, n, s, static_mul_2exp_algo<s_int>{});
        if (static_mul_2exp_algo<s_int>::value == 0 && retval == 0u) {
            rop.zero_unused_limbs();
        }
        return retval;
    }

public:
    /// Ternary left shift.
    /**
     * This function will set \p rop to \p n multiplied by <tt>2**s</tt>.
     *
     * @param rop the return value.
     * @param n the multiplicand.
     * @param s the bit shift value.
     */
    friend void mul_2exp(mp_integer &rop, const mp_integer &n, ::mp_bitcnt_t s)
    {
        const bool sr = rop.is_static(), sn = n.is_static();
        std::size_t size_hint = 0u;
        if (mppp_likely(sr && sn)) {
            size_hint = static_mul_2exp(rop.m_int.g_st(), n.m_int.g_st(), s);
            if (mppp_likely(size_hint == 0u)) {
                return;
            }
        }
        if (sr) {
            rop.m_int.promote(size_hint);
        }
        ::mpz_mul_2exp(&rop.m_int.g_dy(), n.get_mpz_view(), s);
    }
#if defined(_MSC_VER)
    template <typename T>
    friend shift_op_enabler<T> operator<<(const mp_integer &n, T s)
#else
    /// Left shift operator.
    /**
     * @param n the multiplicand.
     * @param s the bit shift value.
     *
     * @return \p n times <tt>2**s</tt>.
     *
     * @throws std::domain_error if \p s is negative or larger than an implementation-defined value.
     */
    template <typename T, shift_op_enabler<T> = 0>
    friend mp_integer operator<<(const mp_integer &n, T s)
#endif
    {
        mp_integer retval;
        mul_2exp(retval, n, cast_to_bitcnt(s));
        return retval;
    }
#if defined(_MSC_VER)
    template <typename T>
    shift_op_enabler<T> &operator<<=(T s)
#else
    /// In-place left shift operator.
    /**
     * @param s the bit shift value.
     *
     * @return a reference to \p this.
     *
     * @throws std::domain_error if \p s is negative or larger than an implementation-defined value.
     */
    template <typename T, shift_op_enabler<T> = 0>
    mp_integer &operator<<=(T s)
#endif
    {
        mul_2exp(*this, *this, cast_to_bitcnt(s));
        return *this;
    }

private:
    // Selection of the algorithm for static tdiv_q_2exp.
    template <typename SInt>
    using static_tdiv_q_2exp_algo = std::integral_constant<int, SInt::s_size == 1 ? 1 : (SInt::s_size == 2 ? 2 : 0)>;
    // mpn implementation.
    static void static_tdiv_q_2exp_impl(s_int &rop, const s_int &n, ::mp_bitcnt_t s,
                                        const std::integral_constant<int, 0> &)
    {
        mpz_size_t asize = n._mp_size;
        if (s == 0u || asize == 0) {
            // If shift is zero, or the operand is zero, write n into rop and return.
            rop = n;
            return;
        }
        // Finish setting up asize and sign.
        int sign = asize != 0;
        if (asize < 0) {
            asize = -asize;
            sign = -1;
        }
        // ls: number of entire limbs shifted.
        // rs: effective shift that will be passed to the mpn function.
        const auto ls = s / GMP_NUMB_BITS, rs = s % GMP_NUMB_BITS;
        if (ls >= std::size_t(asize)) {
            // If we shift by a number of entire limbs equal to or larger than the asize,
            // the result will be zero.
            rop._mp_size = 0;
            return;
        }
        // The maximum new asize will be the old asize minus ls.
        const mpz_size_t new_asize = asize - static_cast<mpz_size_t>(ls);
        if (rs) {
            // Perform the shift via the mpn function, if we are effectively shifting at least 1 bit.
            // Overlapping is fine, as it is guaranteed that rop.m_limbs.data() <= n.m_limbs.data() + ls.
            ::mpn_rshift(rop.m_limbs.data(), n.m_limbs.data() + ls, static_cast<::mp_size_t>(new_asize), unsigned(rs));
        } else {
            // Otherwise, just copy over (the mpn function requires the shift to be at least 1).
            // NOTE: std::move requires that the destination iterator is not within the input range.
            // Here we are sure that ls > 0 because we don't have a remainder, and the zero shift case
            // was already handled above.
            assert(ls);
            std::move(n.m_limbs.begin() + ls, n.m_limbs.begin() + asize, rop.m_limbs.begin());
        }
        // Set the size. We need to check if the top limb is zero.
        rop._mp_size = new_asize - ((rop.m_limbs[std::size_t(new_asize - 1)] & GMP_NUMB_MASK) == 0u);
        if (sign == -1) {
            rop._mp_size = -rop._mp_size;
        }
    }
    // 1-limb optimisation.
    static void static_tdiv_q_2exp_impl(s_int &rop, const s_int &n, ::mp_bitcnt_t s,
                                        const std::integral_constant<int, 1> &)
    {
        const ::mp_limb_t l = n.m_limbs[0] & GMP_NUMB_MASK;
        if (s == 0u || l == 0u) {
            rop = n;
            return;
        }
        if (s >= GMP_NUMB_BITS) {
            // We are shifting by the limb's bit size or greater, the result will be zero.
            rop._mp_size = 0;
            rop.m_limbs[0] = 0u;
            return;
        }
        // Compute the result.
        const auto res = l >> s;
        // Size is the original one or zero.
        rop._mp_size = res ? n._mp_size : 0;
        rop.m_limbs[0] = res;
    }
    // 2-limb optimisation.
    static void static_tdiv_q_2exp_impl(s_int &rop, const s_int &n, ::mp_bitcnt_t s,
                                        const std::integral_constant<int, 2> &)
    {
        mpz_size_t asize = n._mp_size;
        if (s == 0u || asize == 0) {
            rop = n;
            return;
        }
        int sign = asize != 0;
        if (asize < 0) {
            asize = -asize;
            sign = -1;
        }
        // If shift is too large, zero the result and return.
        static_assert(GMP_NUMB_BITS
                          < std::numeric_limits<typename std::decay<decltype(GMP_NUMB_BITS)>::type>::max() / 2u,
                      "Overflow error.");
        if (s >= 2u * GMP_NUMB_BITS) {
            rop._mp_size = 0;
            rop.m_limbs[0u] = 0u;
            rop.m_limbs[1u] = 0u;
            return;
        }
        if (s >= GMP_NUMB_BITS) {
            // NOTE: here the effective shift < GMP_NUMB_BITS, otherwise it would have been caught
            // in the check above.
            const auto lo = (n.m_limbs[1u] & GMP_NUMB_MASK) >> (s - GMP_NUMB_BITS);
            // The size could be zero or +-1, depending
            // on the new content of m_limbs[0] and the previous
            // sign of _mp_size.
            rop._mp_size = lo ? sign : 0;
            rop.m_limbs[0u] = lo;
            rop.m_limbs[1u] = 0u;
            return;
        }
        assert(s > 0u && s < GMP_NUMB_BITS);
        // This represents the bits in hi that will be shifted down into lo.
        // We move them up so we can add tmp to the new lo to account for them.
        // NOTE: mask the final result to avoid spillover into potential nail bits.
        const auto tmp = ((n.m_limbs[1u] & GMP_NUMB_MASK) << (GMP_NUMB_BITS - s)) & GMP_NUMB_MASK;
        rop.m_limbs[0u] = ((n.m_limbs[0u] & GMP_NUMB_MASK) >> s) + tmp;
        rop.m_limbs[1u] = (n.m_limbs[1u] & GMP_NUMB_MASK) >> s;
        // The effective shift was less than 1 entire limb. The new asize must be the old one,
        // or one less than that.
        rop._mp_size = asize - ((rop.m_limbs[std::size_t(asize - 1)] & GMP_NUMB_MASK) == 0u);
        if (sign == -1) {
            rop._mp_size = -rop._mp_size;
        }
    }
    static void static_tdiv_q_2exp(s_int &rop, const s_int &n, ::mp_bitcnt_t s)
    {
        static_tdiv_q_2exp_impl(rop, n, s, static_tdiv_q_2exp_algo<s_int>{});
        if (static_tdiv_q_2exp_algo<s_int>::value == 0) {
            rop.zero_unused_limbs();
        }
    }

public:
    /// Ternary right shift.
    /**
     * This function will set \p rop to \p n divided by <tt>2**s</tt>. \p rop will be the truncated result of the
     * division.
     *
     * @param rop the return value.
     * @param n the dividend.
     * @param s the bit shift value.
     */
    friend void tdiv_q_2exp(mp_integer &rop, const mp_integer &n, ::mp_bitcnt_t s)
    {
        const bool sr = rop.is_static(), sn = n.is_static();
        if (mppp_likely(sr && sn)) {
            static_tdiv_q_2exp(rop.m_int.g_st(), n.m_int.g_st(), s);
            return;
        }
        if (sr) {
            rop.m_int.promote();
        }
        ::mpz_tdiv_q_2exp(&rop.m_int.g_dy(), n.get_mpz_view(), s);
    }
#if defined(_MSC_VER)
    template <typename T>
    friend shift_op_enabler<T> operator>>(const mp_integer &n, T s)
#else
    /// Right shift operator.
    /**
     * @param n the dividend.
     * @param s the bit shift value.
     *
     * @return \p n divided by <tt>2**s</tt>. The result will be truncated.
     *
     * @throws std::domain_error if \p s is negative or larger than an implementation-defined value.
     */
    template <typename T, shift_op_enabler<T> = 0>
    friend mp_integer operator>>(const mp_integer &n, T s)
#endif
    {
        mp_integer retval;
        tdiv_q_2exp(retval, n, cast_to_bitcnt(s));
        return retval;
    }
#if defined(_MSC_VER)
    template <typename T>
    shift_op_enabler<T> &operator>>=(T s)
#else
    /// In-place right shift operator.
    /**
     * @param s the bit shift value.
     *
     * @return a reference to \p this.
     *
     * @throws std::domain_error if \p s is negative or larger than an implementation-defined value.
     */
    template <typename T, shift_op_enabler<T> = 0>
    mp_integer &operator>>=(T s)
#endif
    {
        tdiv_q_2exp(*this, *this, cast_to_bitcnt(s));
        return *this;
    }

private:
    // Selection of the algorithm for static cmp.
    template <typename SInt>
    using static_cmp_algo = std::integral_constant<int, SInt::s_size == 1 ? 1 : (SInt::s_size == 2 ? 2 : 0)>;
    // mpn implementation.
    static int static_cmp(const s_int &n1, const s_int &n2, const std::integral_constant<int, 0> &)
    {
        if (n1._mp_size < n2._mp_size) {
            return -1;
        }
        if (n2._mp_size < n1._mp_size) {
            return 1;
        }
        // The two sizes are equal, compare the absolute values.
        const auto asize = n1._mp_size >= 0 ? n1._mp_size : -n1._mp_size;
        if (asize == 0) {
            // Both operands are zero.
            // NOTE: we do this special casing in order to avoid calling mpn_cmp() on zero operands. It seems to
            // work, but the official GMP docs say one is not supposed to call mpn functions on zero operands.
            return 0;
        }
        const int cmp_abs = ::mpn_cmp(n1.m_limbs.data(), n2.m_limbs.data(), static_cast<::mp_size_t>(asize));
        // If the values are non-negative, return the comparison of the absolute values, otherwise invert it.
        return (n1._mp_size >= 0) ? cmp_abs : -cmp_abs;
    }
    // 1-limb optimisation.
    static int static_cmp(const s_int &n1, const s_int &n2, const std::integral_constant<int, 1> &)
    {
        if (n1._mp_size < n2._mp_size) {
            return -1;
        }
        if (n2._mp_size < n1._mp_size) {
            return 1;
        }
        int cmp_abs = (n1.m_limbs[0u] & GMP_NUMB_MASK) > (n2.m_limbs[0u] & GMP_NUMB_MASK);
        if (!cmp_abs) {
            cmp_abs = -static_cast<int>((n1.m_limbs[0u] & GMP_NUMB_MASK) < (n2.m_limbs[0u] & GMP_NUMB_MASK));
        }
        return (n1._mp_size >= 0) ? cmp_abs : -cmp_abs;
    }
    // 2-limb optimisation.
    static int static_cmp(const s_int &n1, const s_int &n2, const std::integral_constant<int, 2> &)
    {
        if (n1._mp_size < n2._mp_size) {
            return -1;
        }
        if (n2._mp_size < n1._mp_size) {
            return 1;
        }
        auto asize = n1._mp_size >= 0 ? n1._mp_size : -n1._mp_size;
        while (asize != 0) {
            --asize;
            if ((n1.m_limbs[std::size_t(asize)] & GMP_NUMB_MASK) > (n2.m_limbs[std::size_t(asize)] & GMP_NUMB_MASK)) {
                return (n1._mp_size >= 0) ? 1 : -1;
            }
            if ((n1.m_limbs[std::size_t(asize)] & GMP_NUMB_MASK) < (n2.m_limbs[std::size_t(asize)] & GMP_NUMB_MASK)) {
                return (n1._mp_size >= 0) ? -1 : 1;
            }
        }
        return 0;
    }
    // Equality operator.
    // NOTE: special implementation instead of using cmp, this should be faster.
    static bool dispatch_equality(const mp_integer &a, const mp_integer &b)
    {
        const mp_size_t size_a = a.m_int.m_st._mp_size, size_b = b.m_int.m_st._mp_size;
        if (size_a != size_b) {
            return false;
        }
        const ::mp_limb_t *ptr_a, *ptr_b;
        std::size_t asize;
        if (a.is_static()) {
            ptr_a = a.m_int.g_st().m_limbs.data();
            asize = static_cast<std::size_t>((size_a >= 0) ? size_a : -size_a);
        } else {
            ptr_a = a.m_int.g_dy()._mp_d;
            asize = ::mpz_size(&a.m_int.g_dy());
        }
        if (b.is_static()) {
            ptr_b = b.m_int.g_st().m_limbs.data();
        } else {
            ptr_b = b.m_int.g_dy()._mp_d;
        }
        auto limb_cmp
            = [](const ::mp_limb_t &l1, const ::mp_limb_t &l2) { return (l1 & GMP_NUMB_MASK) == (l2 & GMP_NUMB_MASK); };
#if defined(_MSC_VER)
        return std::equal(stdext::make_checked_array_iterator(ptr_a, asize),
                          stdext::make_checked_array_iterator(ptr_a, asize, asize),
                          stdext::make_checked_array_iterator(ptr_b, asize), limb_cmp);
#else
        return std::equal(ptr_a, ptr_a + asize, ptr_b, limb_cmp);
#endif
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static bool dispatch_equality(const mp_integer &a, T n)
    {
        return dispatch_equality(a, mp_integer{n});
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static bool dispatch_equality(T n, const mp_integer &a)
    {
        return dispatch_equality(a, n);
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static bool dispatch_equality(const mp_integer &a, T x)
    {
        return static_cast<T>(a) == x;
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static bool dispatch_equality(T x, const mp_integer &a)
    {
        return dispatch_equality(a, x);
    }
    // Less-than operator.
    static bool dispatch_less_than(const mp_integer &a, const mp_integer &b)
    {
        return cmp(a, b) < 0;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static bool dispatch_less_than(const mp_integer &a, T n)
    {
        return dispatch_less_than(a, mp_integer{n});
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static bool dispatch_less_than(T n, const mp_integer &a)
    {
        return dispatch_greater_than(a, mp_integer{n});
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static bool dispatch_less_than(const mp_integer &a, T x)
    {
        return static_cast<T>(a) < x;
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static bool dispatch_less_than(T x, const mp_integer &a)
    {
        return dispatch_greater_than(a, x);
    }
    // Greater-than operator.
    static bool dispatch_greater_than(const mp_integer &a, const mp_integer &b)
    {
        return cmp(a, b) > 0;
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static bool dispatch_greater_than(const mp_integer &a, T n)
    {
        return dispatch_greater_than(a, mp_integer{n});
    }
    template <typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
    static bool dispatch_greater_than(T n, const mp_integer &a)
    {
        return dispatch_less_than(a, mp_integer{n});
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static bool dispatch_greater_than(const mp_integer &a, T x)
    {
        return static_cast<T>(a) > x;
    }
    template <typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
    static bool dispatch_greater_than(T x, const mp_integer &a)
    {
        return dispatch_less_than(a, x);
    }
// The enabler for relational operators.
#if defined(_MSC_VER)
    template <typename T, typename U>
    using rel_enabler = enable_if_t<!std::is_same<void, common_t<T, U>>::value, bool>;
#else
    template <typename T, typename U>
    using rel_enabler = enable_if_t<!std::is_same<void, common_t<T, U>>::value, int>;
#endif

public:
    /// Comparison function for mp_integer.
    /**
     * @param op1 first argument.
     * @param op2 second argument.
     *
     * @return \p 0 if <tt>op1 == op2</tt>, a negative value if <tt>op1 < op2</tt>, a positive value if
     * <tt>op1 > op2</tt>.
     */
    friend int cmp(const mp_integer &op1, const mp_integer &op2)
    {
        const bool s1 = op1.is_static(), s2 = op2.is_static();
        if (mppp_likely(s1 && s2)) {
            return static_cmp(op1.m_int.g_st(), op2.m_int.g_st(), static_cmp_algo<s_int>{});
        }
        return ::mpz_cmp(op1.get_mpz_view(), op2.get_mpz_view());
    }
#if defined(_MSC_VER)
    template <typename T, typename U>
    friend rel_enabler<T, U> operator==(const T &op1, const U &op2)
#else
    /// Equality operator.
    /**
     * @param op1 first argument.
     * @param op2 second argument.
     *
     * @return \p true if <tt>op1 == op2</tt>, \p false otherwise.
     */
    template <typename T, typename U, rel_enabler<T, U> = 0>
    friend bool operator==(const T &op1, const U &op2)
#endif
    {
        return dispatch_equality(op1, op2);
    }
#if defined(_MSC_VER)
    template <typename T, typename U>
    friend rel_enabler<T, U> operator!=(const T &op1, const U &op2)
#else
    /// Inequality operator.
    /**
     * @param op1 first argument.
     * @param op2 second argument.
     *
     * @return \p true if <tt>op1 != op2</tt>, \p false otherwise.
     */
    template <typename T, typename U, rel_enabler<T, U> = 0>
    friend bool operator!=(const T &op1, const U &op2)
#endif
    {
        return !(op1 == op2);
    }
#if defined(_MSC_VER)
    template <typename T, typename U>
    friend rel_enabler<T, U> operator<(const T &op1, const U &op2)
#else
    /// Less-than operator.
    /**
     * @param op1 first argument.
     * @param op2 second argument.
     *
     * @return \p true if <tt>op1 < op2</tt>, \p false otherwise.
     */
    template <typename T, typename U, rel_enabler<T, U> = 0>
    friend bool operator<(const T &op1, const U &op2)
#endif
    {
        return dispatch_less_than(op1, op2);
    }
#if defined(_MSC_VER)
    template <typename T, typename U>
    friend rel_enabler<T, U> operator>=(const T &op1, const U &op2)
#else
    /// Greater-than or equal operator.
    /**
     * @param op1 first argument.
     * @param op2 second argument.
     *
     * @return \p true if <tt>op1 >= op2</tt>, \p false otherwise.
     */
    template <typename T, typename U, rel_enabler<T, U> = 0>
    friend bool operator>=(const T &op1, const U &op2)
#endif
    {
        return !(op1 < op2);
    }
#if defined(_MSC_VER)
    template <typename T, typename U>
    friend rel_enabler<T, U> operator>(const T &op1, const U &op2)
#else
    /// Greater-than operator.
    /**
     * @param op1 first argument.
     * @param op2 second argument.
     *
     * @return \p true if <tt>op1 > op2</tt>, \p false otherwise.
     */
    template <typename T, typename U, rel_enabler<T, U> = 0>
    friend bool operator>(const T &op1, const U &op2)
#endif
    {
        return dispatch_greater_than(op1, op2);
    }
#if defined(_MSC_VER)
    template <typename T, typename U>
    friend rel_enabler<T, U> operator<=(const T &op1, const U &op2)
#else
    /// Less-than or equal operator.
    /**
     * @param op1 first argument.
     * @param op2 second argument.
     *
     * @return \p true if <tt>op1 <= op2</tt>, \p false otherwise.
     */
    template <typename T, typename U, rel_enabler<T, U> = 0>
    friend bool operator<=(const T &op1, const U &op2)
#endif
    {
        return !(op1 > op2);
    }
    /// Ternary exponentiation.
    /**
     * This function will set \p rop to <tt>base**exp</tt>.
     *
     * @param rop the return value.
     * @param base the base.
     * @param exp the exponent.
     */
    friend void pow_ui(mp_integer &rop, const mp_integer &base, unsigned long exp)
    {
        if (rop.is_static()) {
            MPPP_MAYBE_TLS mpz_raii tmp;
            ::mpz_pow_ui(&tmp.m_mpz, base.get_mpz_view(), exp);
            rop = mp_integer(&tmp.m_mpz);
        } else {
            ::mpz_pow_ui(&rop.m_int.g_dy(), base.get_mpz_view(), exp);
        }
    }
    /// Binary exponentiation.
    /**
     * @param base the base.
     * @param exp the exponent.
     *
     * @return <tt>base**exp</tt>.
     */
    friend mp_integer pow_ui(const mp_integer &base, unsigned long exp)
    {
        mp_integer retval;
        pow_ui(retval, base, exp);
        return retval;
    }

private:
    template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
    static bool exp_nonnegative(const T &exp)
    {
        return exp >= T(0);
    }
    static bool exp_nonnegative(const mp_integer &exp)
    {
        return exp.sgn() >= 0;
    }
    template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
    static unsigned long exp_to_ulong(const T &exp)
    {
        assert(exp >= T(0));
        // NOTE: make_unsigned<T>::type is T if T is already unsigned.
        if (mppp_unlikely(static_cast<typename std::make_unsigned<T>::type>(exp)
                          > std::numeric_limits<unsigned long>::max())) {
            throw std::overflow_error("Cannot convert the integral value " + std::to_string(exp)
                                      + " to unsigned long: the value is too large.");
        }
        return static_cast<unsigned long>(exp);
    }
    static unsigned long exp_to_ulong(const mp_integer &exp)
    {
        try {
            return static_cast<unsigned long>(exp);
        } catch (const std::overflow_error &) {
            // Rewrite the error message.
            throw std::overflow_error("Cannot convert the integral value " + exp.to_string()
                                      + " to unsigned long: the value is too large.");
        }
    }
    template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
    static bool base_is_zero(const T &base)
    {
        return base == T(0);
    }
    static bool base_is_zero(const mp_integer &base)
    {
        return base.is_zero();
    }
    template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
    static bool exp_is_odd(const T &exp)
    {
        return (exp % T(2)) != T(0);
    }
    static bool exp_is_odd(const mp_integer &exp)
    {
        return exp.odd_p();
    }
    template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
    static std::string exp_to_string(const T &exp)
    {
        return std::to_string(exp);
    }
    static std::string exp_to_string(const mp_integer &exp)
    {
        return exp.to_string();
    }
    // Implementation of pow().
    // mp_integer -- integral overload.
    template <typename T, enable_if_t<disjunction<std::is_same<T, mp_integer>, std::is_integral<T>>::value, int> = 0>
    static mp_integer pow_impl(const mp_integer &base, const T &exp)
    {
        mp_integer rop;
        if (exp_nonnegative(exp)) {
            pow_ui(rop, base, exp_to_ulong(exp));
        } else if (mppp_unlikely(base_is_zero(base))) {
            // 0**-n is a division by zero.
            throw zero_division_error("cannot raise zero to the negative power " + exp_to_string(exp));
        } else if (base.is_one()) {
            // 1**n == 1.
            rop = 1;
        } else if (base.is_negative_one()) {
            if (exp_is_odd(exp)) {
                // 1**(-2n-1) == -1.
                rop = -1;
            } else {
                // 1**(-2n) == 1.
                rop = 1;
            }
        } else {
            // m**-n == 1 / m**n == 0.
            rop = 0;
        }
        return rop;
    }
    // C++ integral -- mp_integer overload.
    template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
    static mp_integer pow_impl(const T &base, const mp_integer &exp)
    {
        return pow_impl(mp_integer{base}, exp);
    }
    // mp_integer -- FP overload.
    template <typename T, enable_if_t<std::is_floating_point<T>::value, int> = 0>
    static T pow_impl(const mp_integer &base, const T &exp)
    {
        return std::pow(static_cast<T>(base), exp);
    }
    // FP -- mp_integer overload.
    template <typename T, enable_if_t<std::is_floating_point<T>::value, int> = 0>
    static T pow_impl(const T &base, const mp_integer &exp)
    {
        return std::pow(base, static_cast<T>(exp));
    }

public:
    /// Generic binary exponentiation.
    /**
     * \verbatim embed:rst:leading-asterisk
     * .. note::
     *
     *    This function is enabled only if at least one argument is an :cpp:class:`mppp::mp_integer`
     *    and the other argument is either an :cpp:class:`mppp::mp_integer` or an interoperable type for
     *    :cpp:class:`mppp::mp_integer`.
     * \endverbatim
     *
     * This function will raise \p base to the power \p exp, and return the result. If one of the arguments
     * is a floating-point value, then the result will be computed via <tt>std::pow()</tt> and it will also be a
     * floating-point value. Otherwise, the result is computed via mppp::mp_integer::pow_ui() and its type is
     * mppp::mp_integer. In case of a negative integral exponent and integral base, the result will be zero unless
     * the absolute value of \p base is 1.
     *
     * @param base the base.
     * @param exp the exponent.
     *
     * @return <tt>base**exp</tt>.
     *
     * @throws std::overflow_error if \p base and \p exp are integrals and \p exp is non-negative and outside the range
     * of <tt>unsigned long</tt>.
     * @throws zero_division_error if \p base and \p exp are integrals and \p base is zero and \p exp is negative.
     */
    template <typename T, typename U>
    friend common_t<T, U> pow(const T &base, const U &exp)
    {
        return pow_impl(base, exp);
    }
    /// In-place absolute value.
    /**
     * This method will set \p this to its absolute value.
     *
     * @return reference to \p this.
     */
    mp_integer &abs()
    {
        if (is_static()) {
            if (m_int.g_st()._mp_size < 0) {
                m_int.g_st()._mp_size = -m_int.g_st()._mp_size;
            }
        } else {
            ::mpz_abs(&m_int.g_dy(), &m_int.g_dy());
        }
        return *this;
    }
    /// Binary absolute value.
    /**
     * This function will set \p rop to the absolute value of \p n.
     *
     * @param rop the return value.
     * @param n the argument.
     */
    friend void abs(mp_integer &rop, const mp_integer &n)
    {
        rop = n;
        rop.abs();
    }
    /// Unary absolute value.
    /**
     * @param n the argument.
     *
     * @return the absolute value of \p n.
     */
    friend mp_integer abs(const mp_integer &n)
    {
        mp_integer ret(n);
        ret.abs();
        return ret;
    }
    /// Hash value.
    /**
     * This function will return a hash value for \p n. The hash value depends only on the value of \p n.
     *
     * @param n mp_integer whose hash value will be computed.
     *
     * @return a hash value for \p n.
     */
    friend std::size_t hash(const mp_integer &n)
    {
        std::size_t asize;
        // NOTE: size is part of the common initial sequence.
        const mpz_size_t size = n.m_int.m_st._mp_size;
        const ::mp_limb_t *ptr;
        if (n.m_int.is_static()) {
            asize = static_cast<std::size_t>((size >= 0) ? size : -size);
            ptr = n.m_int.g_st().m_limbs.data();
        } else {
            asize = ::mpz_size(&n.m_int.g_dy());
            ptr = n.m_int.g_dy()._mp_d;
        }
        // Init the retval as the signed size.
        auto retval = static_cast<std::size_t>(size);
        // Combine the limbs.
        for (std::size_t i = 0; i < asize; ++i) {
            // The hash combiner. This is lifted directly from Boost. See also:
            // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3876.pdf
            retval ^= (ptr[i] & GMP_NUMB_MASK) + std::size_t(0x9e3779b9) + (retval << 6) + (retval >> 2);
        }
        return retval;
    }

private:
    static void nextprime_impl(mp_integer &rop, const mp_integer &n)
    {
        if (rop.is_static()) {
            MPPP_MAYBE_TLS mpz_raii tmp;
            ::mpz_nextprime(&tmp.m_mpz, n.get_mpz_view());
            rop = mp_integer(&tmp.m_mpz);
        } else {
            ::mpz_nextprime(&rop.m_int.g_dy(), n.get_mpz_view());
        }
    }

public:
    /// Compute next prime number (binary version).
    /**
     * This function will set \p rop to the first prime number greater than \p n.
     * Note that for negative values of \p n this function always returns 2.
     *
     * @param rop the return value.
     * @param n the mp_integer argument.
     */
    friend void nextprime(mp_integer &rop, const mp_integer &n)
    {
        // NOTE: nextprime on negative numbers always returns 2.
        nextprime_impl(rop, n);
    }
    /// Compute next prime number (unary version).
    /**
     * @param n the mp_integer argument.
     *
     * @return the first prime number greater than \p n.
     */
    friend mp_integer nextprime(const mp_integer &n)
    {
        mp_integer retval;
        nextprime_impl(retval, n);
        return retval;
    }
    /// Compute next prime number (in-place version).
    /**
     * This method will set \p this to the first prime number greater than the current value.
     *
     * @return a reference to \p this.
     */
    mp_integer &nextprime()
    {
        nextprime_impl(*this, *this);
        return *this;
    }
    /// Test primality.
    /**
     * This method will run a series of probabilistic tests to determine if \p this is a prime number.
     * It will return \p 2 if \p this is definitely a prime, \p 1 if \p this is probably a prime and \p 0 if \p this
     * is definitely not-prime.
     *
     * @param reps the number of tests to run.
     *
     * @return an integer indicating if \p this is a prime.
     *
     * @throws std::invalid_argument if \p reps is less than 1 or if \p this is negative.
     */
    int probab_prime_p(int reps = 25) const
    {
        if (mppp_unlikely(reps < 1)) {
            throw std::invalid_argument("The number of primality tests must be at least 1, but a value of "
                                        + std::to_string(reps) + " was provided instead");
        }
        if (mppp_unlikely(sgn() < 0)) {
            throw std::invalid_argument("Cannot run primality tests on the negative number " + to_string());
        }
        return ::mpz_probab_prime_p(get_mpz_view(), reps);
    }
    /// Test primality.
    /**
     * This is the free-function version of mp_integer::probab_prime_p().
     *
     * @param n the mp_integer whose primality will be tested.
     * @param reps the number of tests to run.
     *
     * @return an integer indicating if \p this is a prime.
     *
     * @throws unspecified any exception thrown by mp_integer::probab_prime_p().
     */
    friend int probab_prime_p(const mp_integer &n, int reps = 25)
    {
        return n.probab_prime_p(reps);
    }

private:
    static void sqrt_impl(mp_integer &rop, const mp_integer &n)
    {
        using mppp_impl::copy_limbs_no;
        if (mppp_unlikely(n.m_int.m_st._mp_size < 0)) {
            throw std::domain_error("Cannot compute the square root of the negative number " + n.to_string());
        }
        const bool sr = rop.is_static(), sn = n.is_static();
        if (mppp_likely(sr && sn)) {
            s_int &rs = rop.m_int.g_st();
            const s_int &ns = n.m_int.g_st();
            // NOTE: we know this is not negative, from the check above.
            const mpz_size_t size = ns._mp_size;
            if (!size) {
                // Special casing for zero.
                rs._mp_size = 0;
                rs.zero_unused_limbs();
                return;
            }
            // In case of overlap we need to go through a tmp variable.
            std::array<::mp_limb_t, SSize> tmp;
            const bool overlap = (&rs == &ns);
            auto out_ptr = overlap ? tmp.data() : rs.m_limbs.data();
            ::mpn_sqrtrem(out_ptr, nullptr, ns.m_limbs.data(), static_cast<::mp_size_t>(size));
            // Compute the size of the output (which is ceil(size / 2)).
            const mpz_size_t new_size = size / 2 + size % 2;
            assert(!new_size || (out_ptr[new_size - 1] & GMP_NUMB_MASK));
            // Write out the result.
            rs._mp_size = new_size;
            if (overlap) {
                copy_limbs_no(out_ptr, out_ptr + new_size, rs.m_limbs.data());
            }
            // Clear out unused limbs.
            rs.zero_unused_limbs();
            return;
        }
        if (sr) {
            rop.promote();
        }
        ::mpz_sqrt(&rop.m_int.g_dy(), n.get_mpz_view());
    }

public:
    /// Integer square root (in-place version).
    /**
     * This method will set \p this to its integer square root.
     *
     * @return a reference to \p this.
     *
     * @throws std::domain_error if \p this is negative.
     */
    mp_integer &sqrt()
    {
        sqrt_impl(*this, *this);
        return *this;
    }
    /// Integer square root (binary version).
    /**
     * This method will set \p rop to the integer square root of \p n.
     *
     * @param rop the return value.
     * @param n the mp_integer whose integer square root will be computed.
     *
     * @throws std::domain_error if \p n is negative.
     */
    friend void sqrt(mp_integer &rop, const mp_integer &n)
    {
        sqrt_impl(rop, n);
    }
    /// Integer square root (unary version).
    /**
     * @param n the mp_integer whose integer square root will be computed.
     *
     * @return the integer square root of \p n.
     *
     * @throws std::domain_error if \p n is negative.
     */
    friend mp_integer sqrt(const mp_integer &n)
    {
        mp_integer retval;
        sqrt_impl(retval, n);
        return retval;
    }
    /// Test if value is odd.
    /**
     * @return \p true if \p this is odd, \p false otherwise.
     */
    bool odd_p() const
    {
        if (is_static()) {
            // NOTE: as usual we assume that a zero static integer has all limbs set to zero.
            return (m_int.g_st().m_limbs[0] & GMP_NUMB_MASK) & ::mp_limb_t(1);
        }
        return mpz_odd_p(&m_int.g_dy());
    }
    /// Test if integer is odd.
    /**
     * @param n the argument.
     *
     * @return \p true if \p n is odd, \p false otherwise.
     */
    friend bool odd_p(const mp_integer &n)
    {
        return n.odd_p();
    }
    /// Test if value is even.
    /**
     * @return \p true if \p this is even, \p false otherwise.
     */
    bool even_p() const
    {
        return !odd_p();
    }
    /// Test if integer is even.
    /**
     * @param n the argument.
     *
     * @return \p true if \p n is even, \p false otherwise.
     */
    friend bool even_p(const mp_integer &n)
    {
        return n.even_p();
    }
    /// Factorial.
    /**
     * This function will set \p rop to the factorial of \p n.
     *
     * @param rop the return value.
     * @param n the argument for the factorial.
     *
     * @throws std::invalid_argument if \p n is larger than an implementation-defined limit.
     */
    friend void fac_ui(mp_integer &rop, unsigned long n)
    {
        // NOTE: we put a limit here because the GMP function just crashes and burns
        // if n is too large, and n does not even need to be that large.
        constexpr auto max_fac = 1000000ull;
        if (mppp_unlikely(n > max_fac)) {
            throw std::invalid_argument(
                "The value " + std::to_string(n)
                + " is too large to be used as input for the factorial function (the maximum allowed value is "
                + std::to_string(max_fac) + ")");
        }
        if (rop.is_static()) {
            MPPP_MAYBE_TLS mpz_raii tmp;
            ::mpz_fac_ui(&tmp.m_mpz, n);
            rop = mp_integer(&tmp.m_mpz);
        } else {
            ::mpz_fac_ui(&rop.m_int.g_dy(), n);
        }
    }
    /// Binomial coefficient (ternary version).
    /**
     * This function will set \p rop to the binomial coefficient of \p n and \p k. Negative values of \p n are
     * supported.
     *
     * @param rop the return value.
     * @param n the top argument.
     * @param k the bottom argument.
     */
    friend void bin_ui(mp_integer &rop, const mp_integer &n, unsigned long k)
    {
        if (rop.is_static()) {
            MPPP_MAYBE_TLS mpz_raii tmp;
            ::mpz_bin_ui(&tmp.m_mpz, n.get_mpz_view(), k);
            rop = mp_integer(&tmp.m_mpz);
        } else {
            ::mpz_bin_ui(&rop.m_int.g_dy(), n.get_mpz_view(), k);
        }
    }
    /// Binomial coefficient (binary version).
    /**
     * @param n the top argument.
     * @param k the bottom argument.
     *
     * @return the binomial coefficient of \p n and \p k.
     */
    friend mp_integer bin_ui(const mp_integer &n, unsigned long k)
    {
        mp_integer retval;
        bin_ui(retval, n, k);
        return retval;
    }

private:
    template <typename T, typename U>
    using binomial_enabler_impl = std::
        integral_constant<bool, disjunction<conjunction<std::is_same<mp_integer, T>, std::is_same<mp_integer, U>>,
                                            conjunction<std::is_same<mp_integer, T>, is_supported_integral<U>>,
                                            conjunction<std::is_same<mp_integer, U>, is_supported_integral<T>>>::value>;
#if defined(_MSC_VER)
    template <typename T, typename U>
    using binomial_enabler = enable_if_t<binomial_enabler_impl<T, U>::value, mp_integer>;
#else
    template <typename T, typename U>
    using binomial_enabler = enable_if_t<binomial_enabler_impl<T, U>::value, int>;
#endif
    template <typename T>
    static mp_integer binomial_impl(const mp_integer &n, const T &k)
    {
        // NOTE: here we re-use some helper methods used in the implementation of pow().
        if (exp_nonnegative(k)) {
            return bin_ui(n, exp_to_ulong(k));
        }
        // This is the case k < 0, handled according to:
        // http://arxiv.org/abs/1105.3689/
        if (n.sgn() >= 0) {
            // n >= 0, k < 0.
            return mp_integer{};
        }
        // n < 0, k < 0.
        if (k <= n) {
            // The formula is: (-1)**(n-k) * binomial(-k-1,n-k).
            // Cache n-k.
            const mp_integer nmk{n - k};
            mp_integer tmp{k};
            ++tmp;
            tmp.neg();
            auto retval = bin_ui(tmp, exp_to_ulong(nmk));
            if (nmk.odd_p()) {
                retval.neg();
            }
            return retval;
        }
        return mp_integer{};
    }
    template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
    static mp_integer binomial_impl(const T &n, const mp_integer &k)
    {
        return binomial_impl(mp_integer{n}, k);
    }

public:
#if defined(_MSC_VER)
    template <typename T, typename U>
    friend binomial_enabler<T, U> binomial(const T &n, const U &k)
#else
    /// Generic binomial coefficient.
    /**
     * \verbatim embed:rst:leading-asterisk
     * .. note::
     *
     *    This function is enabled only in the following cases:
     *
     *    * ``T`` and ``U`` are both :cpp:class:`mppp::mp_integer`,
     *    * ``T`` is an :cpp:class:`mppp::mp_integer` and ``U`` is an integral interoperable type for
     *      :cpp:class:`mppp::mp_integer`,
     *    * ``U`` is an :cpp:class:`mppp::mp_integer` and ``T`` is an integral interoperable type for
     *      :cpp:class:`mppp::mp_integer`.
     * \endverbatim
     *
     * This function will compute the binomial coefficient \f$ {{n}\choose{k}} \f$, supporting integral input values.
     * The implementation can handle positive and negative values for both the top and the bottom argument. Internally,
     * the mp_integer::bin_ui() function will be employed.
     *
     * \verbatim embed:rst:leading-asterisk
     * .. seealso::
     *
     *    http://arxiv.org/abs/1105.3689/
     * \endverbatim
     *
     * @param n the top argument.
     * @param k the bottom argument.
     *
     * @return \f$ {{n}\choose{k}} \f$.
     *
     * @throws std::overflow_error if \p k is greater than an implementation-defined value.
     */
    template <typename T, typename U, binomial_enabler<T, U> = 0>
    friend mp_integer binomial(const T &n, const U &k)
#endif
    {
        return binomial_impl(n, k);
    }

private:
    // Exact division.
    // mpn implementation.
    static void static_divexact_impl(s_int &q, const s_int &op1, const s_int &op2, mpz_size_t asize1, mpz_size_t asize2,
                                     int sign1, int sign2, const std::integral_constant<int, 0> &)
    {
        using mppp_impl::copy_limbs_no;
        if (asize1 == 0) {
            // Special casing if the numerator is zero (the mpn functions do not work with zero operands).
            q._mp_size = 0;
            return;
        }
#if __GNU_MP_VERSION > 6 || (__GNU_MP_VERSION == 6 && __GNU_MP_VERSION_MINOR >= 1)
        // NOTE: mpn_divexact_1() is available since GMP 6.1.0.
        if (asize2 == 1) {
            // Optimisation in case the dividend has only 1 limb.
            // NOTE: overlapping arguments are fine here.
            ::mpn_divexact_1(q.m_limbs.data(), op1.m_limbs.data(), static_cast<::mp_size_t>(asize1), op2.m_limbs[0]);
            // Complete the quotient: compute size and sign.
            q._mp_size = asize1 - asize2 + 1;
            while (q._mp_size && !(q.m_limbs[static_cast<std::size_t>(q._mp_size - 1)] & GMP_NUMB_MASK)) {
                --q._mp_size;
            }
            if (sign1 != sign2) {
                q._mp_size = -q._mp_size;
            }
            return;
        }
#else
        // Avoid compiler warnings for unused parameters.
        (void)sign1;
        (void)sign2;
        (void)asize2;
#endif
        // General implementation (via the mpz function).
        MPPP_MAYBE_TLS mpz_raii tmp;
        ::mpz_divexact(&tmp.m_mpz, op1.get_mpz_view(), op2.get_mpz_view());
        // Copy over from the tmp struct into q.
        q._mp_size = tmp.m_mpz._mp_size;
        copy_limbs_no(tmp.m_mpz._mp_d, tmp.m_mpz._mp_d + (q._mp_size >= 0 ? q._mp_size : -q._mp_size),
                      q.m_limbs.data());
    }
    // 1-limb optimisation.
    static void static_divexact_impl(s_int &q, const s_int &op1, const s_int &op2, mpz_size_t, mpz_size_t, int sign1,
                                     int sign2, const std::integral_constant<int, 1> &)
    {
        const ::mp_limb_t q_ = (op1.m_limbs[0] & GMP_NUMB_MASK) / (op2.m_limbs[0] & GMP_NUMB_MASK);
        // Write q.
        q._mp_size = (q_ != 0u);
        if (sign1 != sign2) {
            q._mp_size = -q._mp_size;
        }
        q.m_limbs[0] = q_;
    }
    // 2-limbs optimisation.
    static void static_divexact_impl(s_int &q, const s_int &op1, const s_int &op2, mpz_size_t asize1, mpz_size_t asize2,
                                     int sign1, int sign2, const std::integral_constant<int, 2> &)
    {
        if (asize1 < 2 && asize2 < 2) {
            // 1-limb optimisation.
            const ::mp_limb_t q_ = op1.m_limbs[0] / op2.m_limbs[0];
            q._mp_size = (q_ != 0u);
            if (sign1 != sign2) {
                q._mp_size = -q._mp_size;
            }
            q.m_limbs[0] = q_;
            q.m_limbs[1] = 0u;
            return;
        }
        // General case.
        ::mp_limb_t q1, q2;
        dlimb_div(op1.m_limbs[0], op1.m_limbs[1], op2.m_limbs[0], op2.m_limbs[1], &q1, &q2);
        q._mp_size = q2 ? 2 : (q1 ? 1 : 0);
        if (sign1 != sign2) {
            q._mp_size = -q._mp_size;
        }
        q.m_limbs[0] = q1;
        q.m_limbs[1] = q2;
    }
    static void static_divexact(s_int &q, const s_int &op1, const s_int &op2)
    {
        mpz_size_t asize1 = op1._mp_size, asize2 = op2._mp_size;
        int sign1 = asize1 != 0, sign2 = asize2 != 0;
        if (asize1 < 0) {
            asize1 = -asize1;
            sign1 = -1;
        }
        if (asize2 < 0) {
            asize2 = -asize2;
            sign2 = -1;
        }
        assert(asize1 == 0 || asize2 <= asize1);
        // NOTE: use static_div_algo for the algorithm selection.
        static_divexact_impl(q, op1, op2, asize1, asize2, sign1, sign2, static_div_algo<s_int>{});
        if (static_div_algo<s_int>::value == 0) {
            q.zero_unused_limbs();
        }
    }

public:
    /// Exact division (ternary version).
    /**
     * This function will set \p rop to the quotient of \p n and \p d.
     *
     * \verbatim embed:rst:leading-asterisk
     * .. warning::
     *
     *    If ``d`` does not divide ``n`` exactly, the behaviour will be undefined.
     * \endverbatim
     *
     * @param rop the return value.
     * @param n the dividend.
     * @param d the divisor.
     */
    friend void divexact(mp_integer &rop, const mp_integer &n, const mp_integer &d)
    {
        const bool sr = rop.is_static(), s1 = n.is_static(), s2 = d.is_static();
        if (mppp_likely(sr && s1 && s2)) {
            static_divexact(rop.m_int.g_st(), n.m_int.g_st(), d.m_int.g_st());
            // Division can never fail.
            return;
        }
        if (sr) {
            rop.m_int.promote();
        }
        ::mpz_divexact(&rop.m_int.g_dy(), n.get_mpz_view(), d.get_mpz_view());
    }
    /// Exact division (binary version).
    /**
     * \verbatim embed:rst:leading-asterisk
     * .. warning::
     *
     *    If ``d`` does not divide ``n`` exactly, the behaviour will be undefined.
     * \endverbatim
     *
     * @param n the dividend.
     * @param d the divisor.
     *
     * @return the quotient of \p n and \p d.
     */
    friend mp_integer divexact(const mp_integer &n, const mp_integer &d)
    {
        mp_integer retval;
        divexact(retval, n, d);
        return retval;
    }

private:
    static void static_gcd(s_int &rop, const s_int &op1, const s_int &op2)
    {
        using mppp_impl::copy_limbs_no;
        mpz_size_t asize1 = op1._mp_size, asize2 = op2._mp_size;
        if (asize1 < 0) {
            asize1 = -asize1;
        }
        if (asize2 < 0) {
            asize2 = -asize2;
        }
        // Handle zeroes.
        if (!asize1) {
            rop._mp_size = asize2;
            rop.m_limbs = op2.m_limbs;
            return;
        }
        if (!asize2) {
            rop._mp_size = asize1;
            rop.m_limbs = op1.m_limbs;
            return;
        }
        // Special casing if an operand has asize 1.
        if (asize1 == 1) {
            rop._mp_size = 1;
            rop.m_limbs[0] = ::mpn_gcd_1(op2.m_limbs.data(), static_cast<::mp_size_t>(asize2), op1.m_limbs[0]);
            return;
        }
        if (asize2 == 1) {
            rop._mp_size = 1;
            rop.m_limbs[0] = ::mpn_gcd_1(op1.m_limbs.data(), static_cast<::mp_size_t>(asize1), op2.m_limbs[0]);
            return;
        }
        // General case, via mpz.
        // NOTE: there is an mpn_gcd() function, but it seems difficult to use. Apparently, and contrary to
        // what stated in the latest documentation, the mpn function requires odd operands and bit size
        // (not only limb size!) of the second operand not greater than the first. See for instance the old
        // documentation:
        // ftp://ftp.gnu.org/old-gnu/Manuals/gmp-3.1.1/html_chapter/gmp_9.html
        // Indeed, compiling GMP in debug mode and then trying to use the mpn function without respecting the above
        // results in assertion failures. For now let's keep it like this, the small operand cases are handled above
        // (partially) via mpn_gcd_1(), and in the future we can also think about binary GCD for 1/2 limbs optimisation.
        MPPP_MAYBE_TLS mpz_raii tmp;
        ::mpz_gcd(&tmp.m_mpz, op1.get_mpz_view(), op2.get_mpz_view());
        // Copy over.
        rop._mp_size = tmp.m_mpz._mp_size;
        assert(rop._mp_size > 0);
        copy_limbs_no(tmp.m_mpz._mp_d, tmp.m_mpz._mp_d + rop._mp_size, rop.m_limbs.data());
    }

public:
    /// GCD (ternary version).
    /**
     * This function will set \p rop to the GCD of \p op1 and \p op2. The result is always positive.
     * If both operands are zero, zero is returned.
     *
     * @param rop the return value.
     * @param op1 the first operand.
     * @param op2 the second operand.
     */
    friend void gcd(mp_integer &rop, const mp_integer &op1, const mp_integer &op2)
    {
        const bool sr = rop.is_static(), s1 = op1.is_static(), s2 = op2.is_static();
        if (mppp_likely(sr && s1 && s2)) {
            static_gcd(rop.m_int.g_st(), op1.m_int.g_st(), op2.m_int.g_st());
            rop.m_int.g_st().zero_unused_limbs();
            return;
        }
        if (sr) {
            rop.m_int.promote();
        }
        ::mpz_gcd(&rop.m_int.g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    }
    /// GCD (binary version).
    /**
     * @param op1 the first operand.
     * @param op2 the second operand.
     *
     * @return the GCD of \p op1 and \p op2.
     */
    friend mp_integer gcd(const mp_integer &op1, const mp_integer &op2)
    {
        mp_integer retval;
        gcd(retval, op1, op2);
        return retval;
    }
    /// Return a reference to the internal union.
    /**
     * This method returns a reference to the union used internally to implement the mp_integer class.
     *
     * @return a reference to the internal union member.
     */
    mppp_impl::integer_union<SSize> &_get_union()
    {
        return m_int;
    }
    /// Return a const reference to the internal union.
    /**
     * This method returns a const reference to the union used internally to implement the mp_integer class.
     *
     * @return a const reference to the internal union member.
     */
    const mppp_impl::integer_union<SSize> &_get_union() const
    {
        return m_int;
    }
    /// Get a pointer to the dynamic storage.
    /**
     * This method will first promote \p this to dynamic storage (if \p this is not already employing dynamic storage),
     * and it will then return a pointer to the internal \p mpz_t structure. The returned pointer can be used as an
     * argument for the functions of the GMP API.
     *
     * \verbatim embed:rst:leading-asterisk
     * .. note::
     *
     *    The returned pointer is tied to the lifetime of ``this``. Calling :cpp:func:`mppp::mp_integer::demote()` or
     *    assigning an :cpp:class:`mppp::mp_integer` with static storage to ``this`` will invalidate the returned
     *    pointer.
     * \endverbatim
     *
     * @return a pointer to the internal \p mpz_t structure.
     */
    std::remove_extent<::mpz_t>::type *get_mpz_t()
    {
        promote();
        return &m_int.g_dy();
    }
    /// Test if the value is zero.
    /**
     * @return \p true if the value represented by \p this is zero, \p false otherwise.
     */
    bool is_zero() const
    {
        return m_int.m_st._mp_size == 0;
    }
    /// Test if an mppp::mp_integer is zero.
    /**
     * @param n the mppp::mp_integer to be tested.
     *
     * @return \p true if \p n is zero, \p false otherwise.
     */
    friend bool is_zero(const mp_integer &n)
    {
        return n.is_zero();
    }

private:
    // Implementation of is_one()/is_negative_one().
    template <int One>
    bool is_one_impl() const
    {
        if (m_int.m_st._mp_size != One) {
            return false;
        }
        // Get the pointer to the limbs.
        const ::mp_limb_t *ptr = is_static() ? m_int.g_st().m_limbs.data() : m_int.g_dy()._mp_d;
        return (ptr[0] & GMP_NUMB_MASK) == 1u;
    }

public:
    /// Test if the value is equal to one.
    /**
     * @return \p true if the value represented by \p this is 1, \p false otherwise.
     */
    bool is_one() const
    {
        return is_one_impl<1>();
    }
    /// Test if an mppp::mp_integer is equal to one.
    /**
     * @param n the mppp::mp_integer to be tested.
     *
     * @return \p true if \p n is equal to 1, \p false otherwise.
     */
    friend bool is_one(const mp_integer &n)
    {
        return n.is_one();
    }
    /// Test if the value is equal to minus one.
    /**
     * @return \p true if the value represented by \p this is -1, \p false otherwise.
     */
    bool is_negative_one() const
    {
        return is_one_impl<-1>();
    }
    /// Test if an mppp::mp_integer is equal to minus one.
    /**
     * @param n the mppp::mp_integer to be tested.
     *
     * @return \p true if \p n is equal to -1, \p false otherwise.
     */
    friend bool is_negative_one(const mp_integer &n)
    {
        return n.is_negative_one();
    }

private:
    mppp_impl::integer_union<SSize> m_int;
};

template <std::size_t SSize>
constexpr std::size_t mp_integer<SSize>::ssize;

namespace mppp_impl
{

// A small wrapper to avoid name clashing below, in the specialisation of std::hash.
template <size_t SSize>
inline std::size_t hash_wrapper(const mp_integer<SSize> &n)
{
    return hash(n);
}
}
}

namespace std
{

/// Specialisation of \p std::hash for mppp::mp_integer.
template <size_t SSize>
struct hash<mppp::mp_integer<SSize>> {
    /// The argument type.
    typedef mppp::mp_integer<SSize> argument_type;
    /// The result type.
    typedef size_t result_type;
    /// Call operator.
    /**
     * @param n the mppp::mp_integer whose hash will be returned.
     *
     * @return the hash value of \p n, as calculated by mppp::mp_integer::hash().
     */
    result_type operator()(const argument_type &n) const
    {
        return mppp::mppp_impl::hash_wrapper(n);
    }
};
}

#if defined(_MSC_VER)

#pragma warning(pop)

#endif

#endif
