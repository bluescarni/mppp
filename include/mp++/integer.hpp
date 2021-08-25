// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_INTEGER_HPP
#define MPPP_INTEGER_HPP

#include <mp++/config.hpp>

// NOTE: cinttypes comes from the std::abs() include mess:
// http://en.cppreference.com/w/cpp/numeric/math/abs

#include <algorithm>
#include <array>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <tuple>
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
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/tracking.hpp>

#endif

#include <mp++/concepts.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/exceptions.hpp>
#include <mp++/fwd.hpp>
#include <mp++/type_name.hpp>

#if defined(MPPP_WITH_MPFR)
#include <mp++/detail/mpfr.hpp>
#endif

// Compiler configuration.
// NOTE: check for MSVC first, as clang-cl does define both __clang__ and _MSC_VER,
// and we want to configure it as MSVC.
#if defined(_MSC_VER)

// Disable clang-format here, as the include order seems to matter.
// clang-format off
#include <windows.h>
#include <Winnt.h>
// clang-format on

// We use the BitScanReverse(64) intrinsic in the implementation of limb_size_nbits(), but
// only if we are *not* on clang-cl: there, we can use GCC-style intrinsics.
// https://msdn.microsoft.com/en-us/library/fbxyd7zd.aspx
#if !defined(__clang__)
#if _WIN64
#pragma intrinsic(_BitScanReverse64)
#else
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#endif
#endif

// Disable some warnings for MSVC.
#pragma warning(push)
#pragma warning(disable : 4127)
#pragma warning(disable : 4146)
#pragma warning(disable : 4804)

// Checked iterators functionality.
#include <iterator>

#endif

namespace mppp
{

// Strongly typed enum to represent a bit count in the constructor
// of integer from number of bits.
enum class integer_bitcnt_t : ::mp_bitcnt_t {};

namespace detail
{

// Small helper to get the size in limbs from an mpz_t. Will return zero if n is zero.
inline std::size_t get_mpz_size(const ::mpz_t n)
{
    return (n->_mp_size >= 0) ? static_cast<std::size_t>(n->_mp_size) : static_cast<std::size_t>(nint_abs(n->_mp_size));
}

#if defined(_MSC_VER) && defined(__clang__)

// NOTE: clang-cl gives a deprecation warning
// here due to the fact that the dllexported
// cache class is missing a copy assignment
// operator. Not sure what to make of it.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"

#endif

// Structure for caching allocated arrays of limbs.
// NOTE: needs to be public for testing purposes.
struct MPPP_DLL_PUBLIC mpz_alloc_cache {
    // Arrays up to this size will be cached.
    static constexpr std::size_t max_size = 10;
    // Max number of arrays to cache for each size.
    static constexpr std::size_t max_entries = 100;
    // The actual cache.
    std::array<std::array<::mp_limb_t *, max_entries>, max_size> caches;
    // The number of arrays actually stored in each cache entry.
    std::array<std::size_t, max_size> sizes;
    // NOTE: use round brackets init for the usual GCC 4.8 workaround.
    // NOTE: this will zero initialise recursively both members: we will
    // have all nullptrs in the caches, and all cache sizes will be zeroes.
    constexpr mpz_alloc_cache() noexcept : caches(), sizes() {}
    mpz_alloc_cache(const mpz_alloc_cache &) = delete;
    mpz_alloc_cache(mpz_alloc_cache &&) = delete;
    mpz_alloc_cache &operator=(const mpz_alloc_cache &) = delete;
    mpz_alloc_cache &operator=(mpz_alloc_cache &&) = delete;
    // Clear the cache, deallocating all the data in the arrays.
    void clear() noexcept;
    ~mpz_alloc_cache()
    {
        clear();
    }
};

#if defined(_MSC_VER) && defined(__clang__)

#pragma clang diagnostic pop

#endif

#if defined(MPPP_HAVE_THREAD_LOCAL)

// Get a reference to the thread-local mpz allocation
// cache. Used only for debugging.
MPPP_DLL_PUBLIC mpz_alloc_cache &get_thread_local_mpz_cache();

#endif

// Helper function to init an mpz to zero with nlimbs preallocated limbs.
MPPP_DLL_PUBLIC void mpz_init_nlimbs(mpz_struct_t &, std::size_t);

// Small helper to determine how many GMP limbs we need to fit nbits bits.
constexpr ::mp_bitcnt_t nbits_to_nlimbs(::mp_bitcnt_t nbits)
{
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    return static_cast<::mp_bitcnt_t>(nbits / unsigned(GMP_NUMB_BITS) + ((nbits % unsigned(GMP_NUMB_BITS)) != 0u));
}

// Helper function to init an mpz to zero with enough space for nbits bits. The
// nlimbs parameter must be consistent with the nbits parameter (it will be computed
// outside this function).
MPPP_DLL_PUBLIC void mpz_init_nbits(mpz_struct_t &, ::mp_bitcnt_t, std::size_t);

// Thin wrapper around mpz_clear(): will add entry to cache if possible instead of clearing.
MPPP_DLL_PUBLIC void mpz_clear_wrap(mpz_struct_t &);

// Combined init+set.
inline void mpz_init_set_nlimbs(mpz_struct_t &m0, const mpz_struct_t &m1)
{
    mpz_init_nlimbs(m0, get_mpz_size(&m1));
    mpz_set(&m0, &m1);
}

// Convert an mpz to a string in a specific base, to be written into out.
MPPP_DLL_PUBLIC void mpz_to_str(std::vector<char> &, const mpz_struct_t *, int = 10);

// Convenience overload for the above.
inline std::string mpz_to_str(const mpz_struct_t *mpz, int base = 10)
{
    MPPP_MAYBE_TLS std::vector<char> tmp;
    mpz_to_str(tmp, mpz, base);
    return tmp.data();
}

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

// Add a and b, store the result in res, and return 1 if there's unsigned overflow, 0 otherwise.
// NOTE: recent GCC versions have builtins for this, but they don't seem to make much of a difference:
// https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html
inline ::mp_limb_t limb_add_overflow(::mp_limb_t a, ::mp_limb_t b, ::mp_limb_t *res)
{
    *res = a + b;
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    return *res < a;
}

// Implementation of the function to count the number of leading zeroes
// in an unsigned integral value for GCC/clang. The clz builtin is available
// in all supported GCC/clang versions.
#if defined(__clang__) || defined(__GNUC__)

// Dispatch based on the integer type.
inline int builtin_clz_impl(unsigned n)
{
    return __builtin_clz(n);
}

inline int builtin_clz_impl(unsigned long n)
{
    return __builtin_clzl(n);
}

inline int builtin_clz_impl(unsigned long long n)
{
    return __builtin_clzll(n);
}

// NOTE: we checked earlier that mp_limb_t is one of the 3 types supported by the
// clz builtin. No need to constrain the template.
template <typename T>
inline unsigned builtin_clz(T n)
{
    assert(n != 0u);
    return static_cast<unsigned>(builtin_clz_impl(n));
}

#endif

// Determine the size in (numeric) bits of limb l.
// l does not need to be masked (we will mask inside this function),
// and, after masking, it cannot be zero.
inline unsigned limb_size_nbits(::mp_limb_t l)
{
    assert((l & GMP_NUMB_MASK) != 0u);
#if defined(__clang__) || defined(__GNUC__)
    // Implementation using GCC/clang builtins.
    // NOTE: here we need nl_digits<::mp_limb_t>() (instead of GMP_NUMB_BITS) because
    // builtin_clz() counts zeroes also in the nail bits.
    return (unsigned(nl_digits<::mp_limb_t>()) - builtin_clz(l & GMP_NUMB_MASK));
#elif defined(_MSC_VER)
    // Implementation using MSVC's intrinsics.
    unsigned long index;
#if _WIN64
    _BitScanReverse64
#else
    _BitScanReverse
#endif
        (&index, l & GMP_NUMB_MASK);
    return static_cast<unsigned>(index) + 1u;
#else
    // Implementation via GMP.
    // NOTE: assuming GMP knows how to deal with nails, so not masking.
    return static_cast<unsigned>(mpn_sizeinbase(&l, 1, 2));
#endif
}

// Machinery for the conversion of a large uint to a limb array.

// Definition of the limb array type.
template <typename T>
struct limb_array_t_ {
    // We want only unsigned ints.
    static_assert(is_integral<T>::value && is_unsigned<T>::value, "Type error.");
    // Overflow check.
    static_assert(unsigned(nl_digits<T>()) <= nl_max<::mp_bitcnt_t>(), "Overflow error.");
    // The max number of GMP limbs needed to represent an instance of T.
    constexpr static std::size_t size = nbits_to_nlimbs(static_cast<::mp_bitcnt_t>(nl_digits<T>()));
    // Overflow check.
    static_assert(size <= nl_max<std::size_t>(), "Overflow error.");
    // The type definition.
    using type = std::array<::mp_limb_t, static_cast<std::size_t>(size)>;
};

// Handy alias.
template <typename T>
using limb_array_t = typename limb_array_t_<T>::type;

// This is a small utility function to shift down the unsigned integer n by GMP_NUMB_BITS.
// If GMP_NUMB_BITS is not smaller than the bit size of T, then an assertion will fire. We need this
// little helper in order to avoid compiler warnings.
template <typename T>
inline void u_checked_rshift(T &n, const std::true_type &)
{
    static_assert(is_integral<T>::value && is_unsigned<T>::value, "Invalid type.");
    n >>= GMP_NUMB_BITS;
}

template <typename T>
inline void u_checked_rshift(T &, const std::false_type &)
{
    static_assert(is_integral<T>::value && is_unsigned<T>::value, "Invalid type.");
    assert(false);
}

// Convert a large unsigned integer into a limb array, and return the effective size.
// n must be > GMP_NUMB_MAX.
template <typename T>
inline std::size_t uint_to_limb_array(limb_array_t<T> &rop, T n)
{
    static_assert(is_integral<T>::value && is_unsigned<T>::value, "Invalid type.");
    assert(n > GMP_NUMB_MAX);
    // We can assign the first two limbs directly, as we know n > GMP_NUMB_MAX.
    rop[0] = static_cast<::mp_limb_t>(n & GMP_NUMB_MASK);
    constexpr auto dispatcher = std::integral_constant<bool, (GMP_NUMB_BITS < nl_constants<T>::digits)>{};
    u_checked_rshift(n, dispatcher);
    assert(n);
    rop[1] = static_cast<::mp_limb_t>(n & GMP_NUMB_MASK);
    u_checked_rshift(n, dispatcher);
    std::size_t size = 2;
    // NOTE: currently this code is hit only on 32-bit archs with 64-bit integers,
    // and we have no nail builds: we cannot go past 2 limbs size.
    // LCOV_EXCL_START
    for (; n; ++size, u_checked_rshift(n, dispatcher)) {
        rop[size] = static_cast<::mp_limb_t>(n & GMP_NUMB_MASK);
    }
    // LCOV_EXCL_STOP
    assert(size <= rop.size());
    return size;
}

// Small utility to check that no nail bits are set.
inline bool check_no_nails(const ::mp_limb_t &l)
{
    return l <= GMP_NUMB_MAX;
}

// Small utility to compute the absolute value of the size of a 2-limbs number from its lo and hi limbs.
// Requires no nail bits in hi or lo.
inline mpz_size_t size_from_lohi(const ::mp_limb_t &lo, const ::mp_limb_t &hi)
{
    assert(check_no_nails(lo) && check_no_nails(hi));
    const auto lonz = static_cast<unsigned>(lo != 0u), hinz = static_cast<unsigned>(hi != 0u);
    // NOTE: this contraption ensures the correct result:
    // hi | lo | asize
    // -----------------------------
    //  1 |  1 | 1 * 2 + (0 & 1) = 2
    //  1 |  0 | 1 * 2 + (0 & 0) = 2
    //  0 |  1 | 0 * 2 + (1 & 1) = 1
    //  0 |  0 | 0 * 2 + (1 & 0) = 0
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    return static_cast<mpz_size_t>(hinz * 2u + (static_cast<unsigned>(!hinz) & lonz));
}

// Branchless sign function for C++ integrals:
// https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
template <typename T>
constexpr int integral_sign(T n)
{
    static_assert(is_integral<T>::value && is_signed<T>::value,
                  "Invalid type: this function requires signed integrals in input.");
    return (T(0) < n) - (n < T(0));
}

// Small utility to selectively disable checked iterators warnings
// in MSVC. See:
// https://msdn.microsoft.com/en-us/library/dn217887.aspx
// On compilers other than MSVC, this just returns the input value.
template <typename T>
inline auto make_uai(T *ptr) ->
#if defined(_MSC_VER)
    decltype(stdext::make_unchecked_array_iterator(ptr))
#else
    decltype(ptr)
#endif
{
    return
#if defined(_MSC_VER)
        stdext::make_unchecked_array_iterator(ptr);
#else
        ptr;
#endif
}

// The static integer class.
template <std::size_t SSize>
struct static_int {
    // Let's put a hard cap and sanity check on the static size.
    static_assert(SSize > 0u && SSize <= 64u, "Invalid static size for integer.");
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
    // Zero the limbs from index idx up to the end of the limbs array, but only
    // if the static size is a target for special optimisations.
    void zero_upper_limbs(std::size_t idx)
    {
        if (SSize <= opt_size) {
            std::fill(m_limbs.begin() + idx, m_limbs.end(), ::mp_limb_t(0));
        }
    }
    // Zero the limbs that are not used for representing the value, but only
    // if the static size is a target for special optimisations.
    // This is normally not needed, but it is useful when using the GMP mpn api on a static int:
    // the GMP api does not clear unused limbs, but we rely on unused limbs being zero when optimizing operations
    // for few static limbs.
    void zero_unused_limbs()
    {
        zero_upper_limbs(static_cast<std::size_t>(abs_size()));
    }
    // Default constructor, inits to zero.
    static_int() : _mp_size(0)
    {
        // Zero the limbs, if needed.
        zero_upper_limbs(0);
    }
    // Let's avoid copying the _mp_alloc member, as it is never written to and it must always
    // have the same value.
    static_int(const static_int &other) : _mp_size(other._mp_size)
    {
        if (SSize <= opt_size) {
            // In this case, we know that other's upper limbs are already
            // properly zeroed.
            m_limbs = other.m_limbs;
        } else {
            // Otherwise, we cannot read potentially uninited limbs.
            const auto asize = other.abs_size();
            // Copy over the limbs. This is safe, as other is a distinct object from this.
            copy_limbs_no(other.m_limbs.data(), other.m_limbs.data() + asize, m_limbs.data());
            // No need to zero, we are in non-optimised static size.
        }
    }
    // Same as copy constructor.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    static_int(static_int &&other) noexcept : static_int(other) {}
    // These 2 constructors are used in the generic constructor of integer_union.
    //
    // Constructor from a size and a single limb (will be the least significant limb).
    explicit static_int(mpz_size_t size, ::mp_limb_t l) : _mp_size(size)
    {
        // Input sanity checks.
        assert(size <= s_size && size >= -s_size);
        // l and size must both be zero or both nonzero.
        assert((l && size) || (!l && !size));
        // The limb is coming in from the decomposition of an uintegral value,
        // it should not have any nail bit set.
        assert(l <= GMP_NUMB_MAX);
        // NOTE: this is ok if l is zero: after init, the size will be zero
        // and either all limbs have been set to zero (for SSize <= opt_size),
        // or only the first limb has been set to zero (for SSize > opt_size).
        m_limbs[0] = l;
        // Zero fill the remaining limbs, if needed.
        zero_upper_limbs(1);
    }
    // Constructor from a (signed) size and a limb range. The limbs in the range will be
    // copied as the least significant limbs.
    explicit static_int(mpz_size_t size, const ::mp_limb_t *begin, std::size_t asize) : _mp_size(size)
    {
        // Input sanity checks.
        assert(asize <= SSize);
        assert(size <= s_size && size >= -s_size);
        assert(size == static_cast<mpz_size_t>(asize) || size == -static_cast<mpz_size_t>(asize));
        // Copy the input limbs.
        // NOTE: here we are constructing a *new* object, thus I don't think it's possible that begin
        // overlaps with m_limbs.data() (unless some UBish stuff is being attempted).
        copy_limbs_no(begin, begin + asize, m_limbs.data());
        // Zero fill the remaining limbs, if needed.
        zero_upper_limbs(asize);
    }
    // NOLINTNEXTLINE(cert-oop54-cpp)
    static_int &operator=(const static_int &other)
    {
        _mp_size = other._mp_size;
        if (SSize <= opt_size) {
            // In this case, we know other's upper limbs are properly zeroed out.
            // NOTE: self assignment of std::array should be fine.
            m_limbs = other.m_limbs;
        } else {
            // Otherwise, we must avoid reading from uninited limbs.
            // Copy over the limbs. There's potential overlap here.
            const auto asize = other.abs_size();
            copy_limbs(other.m_limbs.data(), other.m_limbs.data() + asize, m_limbs.data());
            // No need to zero, we are in non-optimised static size.
        }
        return *this;
    }
    static_int &operator=(static_int &&other) noexcept
    {
        // Just forward to the copy assignment.
        // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
        return operator=(other);
    }
    // Swap primitive.
    void swap(static_int &other) noexcept
    {
        if (SSize <= opt_size) {
            // In this case, we know other's upper limbs are properly zeroed out.
            // NOTE: self swap of std::array should be fine.
            std::swap(_mp_size, other._mp_size);
            std::swap(m_limbs, other.m_limbs);
        } else {
            // Otherwise, we must avoid reading from uninited limbs.
            // Copy over the limbs of this to temp storage.
            limbs_type tmp;
            const auto asize1 = abs_size();
            copy_limbs_no(m_limbs.data(), m_limbs.data() + asize1, tmp.data());

            // Copy over the limbs of other to this.
            // NOTE: potential overlap here.
            copy_limbs(other.m_limbs.data(), other.m_limbs.data() + other.abs_size(), m_limbs.data());

            // Copy over the limbs in temp storage to other.
            copy_limbs_no(tmp.data(), tmp.data() + asize1, other.m_limbs.data());

            // Swap the sizes.
            std::swap(_mp_size, other._mp_size);
        }
    }
#ifdef __MINGW32__

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"

#endif
    MPPP_NODISCARD bool dtor_checks() const
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
#ifdef __MINGW32__

#pragma GCC diagnostic pop

#endif
    ~static_int()
    {
        assert(dtor_checks());
    }
    // Size in limbs (absolute value of the _mp_size member).
    MPPP_NODISCARD mpz_size_t abs_size() const
    {
        return std::abs(_mp_size);
    }
    // NOTE: the retval here can be used only in read-only mode, otherwise
    // we will have UB due to the const_cast use.
    MPPP_NODISCARD mpz_struct_t get_mpz_view() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        return mpz_struct_t{_mp_alloc, _mp_size, const_cast<::mp_limb_t *>(m_limbs.data())};
    }
    mpz_alloc_t _mp_alloc = s_alloc;
    // NOLINTNEXTLINE(modernize-use-default-member-init)
    mpz_size_t _mp_size;
    limbs_type m_limbs;
};

// {static_int,mpz} union.
template <std::size_t SSize>
union integer_union {
    using s_storage = static_int<SSize>;
    using d_storage = mpz_struct_t;
    // Def ctor, will init to static.
    integer_union() : m_st() {}
    // Copy constructor, does a deep copy maintaining the storage class of other.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
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
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    integer_union(integer_union &&other) noexcept
    {
        if (other.is_static()) {
            // Activate the static member with a copy.
            ::new (static_cast<void *>(&m_st)) s_storage(other.g_st());
        } else {
            // Activate dynamic member and shallow copy it from other.
            ::new (static_cast<void *>(&m_dy)) d_storage(other.g_dy());
            // Downgrade the other to an empty static.
            other.g_dy().~d_storage();
            ::new (static_cast<void *>(&other.m_st)) s_storage();
        }
    }
    // Special casing for bool.
    void dispatch_generic_ctor(bool b)
    {
        // Construct the static. No need for masking in the only limb.
        ::new (static_cast<void *>(&m_st)) s_storage{static_cast<mpz_size_t>(b), static_cast<::mp_limb_t>(b)};
    }
    // Construction from unsigned ints. The Neg flag will negate the integer after construction, it is for use in
    // the constructor from signed ints.
    template <typename T, bool Neg = false, enable_if_t<conjunction<is_integral<T>, is_unsigned<T>>::value, int> = 0>
    void dispatch_generic_ctor(T n)
    {
        if (n <= GMP_NUMB_MAX) {
            // Special codepath if n fits directly in a single limb.
            // No need for the mask as we are sure that n <= GMP_NUMB_MAX.
            ::new (static_cast<void *>(&m_st)) s_storage{Neg ? -(n != 0u) : (n != 0u), static_cast<::mp_limb_t>(n)};
            return;
        }
        // Convert n into an array of limbs.
        limb_array_t<T> tmp;
        const auto size = uint_to_limb_array(tmp, n);
        construct_from_limb_array<false>(tmp.data(), size);
        // Negate if requested.
        if (Neg) {
            neg();
        }
    }
    // Construction from signed ints.
    template <typename T, enable_if_t<conjunction<is_integral<T>, is_signed<T>>::value, int> = 0>
    void dispatch_generic_ctor(T n)
    {
        if (n >= T(0)) {
            // Positive value, just cast to unsigned.
            dispatch_generic_ctor(make_unsigned(n));
        } else {
            // Negative value, use its abs.
            dispatch_generic_ctor<make_unsigned_t<T>, true>(nint_abs(n));
        }
    }
    // Construction from float/double.
    template <typename T, enable_if_t<disjunction<std::is_same<T, float>, std::is_same<T, double>>::value, int> = 0>
    void dispatch_generic_ctor(T x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot construct an integer from the non-finite floating-point value "
                                    + to_string(x));
        }
        MPPP_MAYBE_TLS mpz_raii tmp;
        mpz_set_d(&tmp.m_mpz, static_cast<double>(x));
        dispatch_mpz_ctor(&tmp.m_mpz);
    }
#if defined(MPPP_WITH_MPFR)
    // Construction from long double, requires MPFR.
    void dispatch_generic_ctor(long double x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot construct an integer from the non-finite floating-point value "
                                    + to_string(x));
        }
        // NOTE: static checks for overflows and for the precision value are done in mpfr.hpp.
        constexpr int d2 = std::numeric_limits<long double>::max_digits10 * 4;
        MPPP_MAYBE_TLS mpfr_raii mpfr(static_cast<::mpfr_prec_t>(d2));
        MPPP_MAYBE_TLS mpz_raii tmp;
        ::mpfr_set_ld(&mpfr.m_mpfr, x, MPFR_RNDN);
        ::mpfr_get_z(&tmp.m_mpz, &mpfr.m_mpfr, MPFR_RNDZ);
        dispatch_mpz_ctor(&tmp.m_mpz);
    }
#endif
    // The generic constructor.
    template <typename T>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit integer_union(const T &x)
    {
        dispatch_generic_ctor(x);
    }
    // Implementation of the constructor from string. Abstracted into separate function because it is re-used.
    void dispatch_c_string_ctor(const char *s, int base)
    {
        if (mppp_unlikely(base != 0 && (base < 2 || base > 62))) {
            throw std::invalid_argument(
                "In the constructor of integer from string, a base of " + to_string(base)
                + " was specified, but the only valid values are 0 and any value in the [2,62] range");
        }
        MPPP_MAYBE_TLS mpz_raii mpz;
        if (mppp_unlikely(mpz_set_str(&mpz.m_mpz, s, base))) {
            if (base != 0) {
                throw std::invalid_argument(std::string("The string '") + s + "' is not a valid integer in base "
                                            + to_string(base));
            } else {
                throw std::invalid_argument(std::string("The string '") + s
                                            + "' is not a valid integer in any supported base");
            }
        }
        dispatch_mpz_ctor(&mpz.m_mpz);
    }
    // Constructor from C string and base.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit integer_union(const char *s, int base)
    {
        dispatch_c_string_ctor(s, base);
    }
    // Constructor from string range and base.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit integer_union(const char *begin, const char *end, int base)
    {
        // Copy the range into a local buffer.
        MPPP_MAYBE_TLS std::vector<char> buffer;
        buffer.assign(begin, end);
        buffer.emplace_back('\0');
        dispatch_c_string_ctor(buffer.data(), base);
    }
    // Constructor from mpz_t. Abstracted into separate function because it is re-used.
    void dispatch_mpz_ctor(const ::mpz_t n)
    {
        const auto asize = get_mpz_size(n);
        if (asize > SSize) {
            // Too big, need to use dynamic.
            ::new (static_cast<void *>(&m_dy)) d_storage;
            mpz_init_set_nlimbs(m_dy, *n);
        } else {
            ::new (static_cast<void *>(&m_st)) s_storage{n->_mp_size, n->_mp_d, asize};
        }
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit integer_union(const ::mpz_t n)
    {
        dispatch_mpz_ctor(n);
    }
#if !defined(_MSC_VER)
    // Move ctor from mpz_t.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit integer_union(::mpz_t &&n)
    {
        const auto asize = get_mpz_size(n);
        if (asize > SSize) {
            // Too big, make shallow copy into dynamic. this will now
            // own the resources of n.
            ::new (static_cast<void *>(&m_dy)) d_storage(*n);
        } else {
            // Fits into small.
            ::new (static_cast<void *>(&m_st)) s_storage{n->_mp_size, n->_mp_d, asize};
            // Clear n: its resources have been copied into this, and we must
            // ensure uniform behaviour with the case in which we shallow-copied it
            // into dynamic storage.
            mpz_clear_wrap(*n);
        }
    }
#endif
    // Implementation of the ctor from an array of limbs. CheckArray establishes
    // if p is checked for sanity.
    template <bool CheckArray>
    void construct_from_limb_array(const ::mp_limb_t *p, std::size_t size)
    {
        if (CheckArray) {
            // If size is not zero, then the most significant limb must contain something.
            if (mppp_unlikely(size && !p[size - 1u])) {
                throw std::invalid_argument(
                    "When initialising an integer from an array of limbs, the last element of the "
                    "limbs array must be nonzero");
            }
            // NOTE: no builds in the CI have nail bits, cannot test this failure.
            // LCOV_EXCL_START
            // All elements of p must be <= GMP_NUMB_MAX.
            if (mppp_unlikely(std::any_of(p, p + size, [](::mp_limb_t l) { return l > GMP_NUMB_MAX; }))) {
                throw std::invalid_argument("When initialising an integer from an array of limbs, every element of the "
                                            "limbs array must not be greater than GMP_NUMB_MAX");
            }
            // LCOV_EXCL_STOP
        } else {
            assert(!size || p[size - 1u]);
            assert(std::all_of(p, p + size, [](::mp_limb_t l) { return l <= GMP_NUMB_MAX; }));
        }
        if (size <= SSize) {
            // Fits into small. This constructor will take care
            // of zeroing out the top limbs as well.
            ::new (static_cast<void *>(&m_st)) s_storage{static_cast<mpz_size_t>(size), p, size};
        } else {
            // Convert size to mpz_size_t before anything else, for exception safety.
            const auto s = safe_cast<mpz_size_t>(size);
            // Init the dynamic storage struct.
            ::new (static_cast<void *>(&m_dy)) d_storage;
            // Init to zero, with size precallocated limbs.
            // NOTE: currently this does not throw, it will just abort() in case of overflow,
            // allocation errors, etc. If this ever becomes throwing, we probably need to move
            // the init of d_storage below this line, so that if an exception is thrown we have
            // not constructed the union member yet.
            mpz_init_nlimbs(m_dy, size);
            // Copy the input limbs.
            // NOTE: here we are constructing a *new* object, thus I don't think it's possible that p
            // overlaps with m_dy._mp_d (unless some UBish stuff is being attempted).
            copy_limbs_no(p, p + size, m_dy._mp_d);
            // Assign the size.
            m_dy._mp_size = s;
        }
    }
    // Constructor from array of limbs.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit integer_union(const ::mp_limb_t *p, std::size_t size)
    {
        construct_from_limb_array<true>(p, size);
    }
    // Constructor from number of bits.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit integer_union(integer_bitcnt_t nbits_)
    {
        const auto nbits = static_cast<::mp_bitcnt_t>(nbits_);
        const auto nlimbs = safe_cast<std::size_t>(nbits_to_nlimbs(nbits));
        if (nlimbs <= SSize) {
            // Static storage is enough for the requested number of bits. Def init.
            ::new (static_cast<void *>(&m_st)) s_storage();
        } else {
            // Init the dynamic storage struct.
            ::new (static_cast<void *>(&m_dy)) d_storage;
            // Init to zero with the desired number of bits.
            mpz_init_nbits(m_dy, nbits, nlimbs);
        }
    }
    // Copy assignment operator, performs a deep copy maintaining the storage class.
    // NOLINTNEXTLINE(cert-oop54-cpp)
    integer_union &operator=(const integer_union &other)
    {
        const bool s1 = is_static(), s2 = other.is_static();
        if (s1 && s2) {
            // Self assignment is fine, handled in the static.
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
            // Self assignment is fine, mpz_set() can have aliasing arguments.
            mpz_set(&g_dy(), &other.g_dy());
        }
        return *this;
    }
    // Move assignment, same as above plus possibly steals resources. If this is static
    // and other is dynamic, other is downgraded to a zero static.
    integer_union &operator=(integer_union &&other) noexcept
    {
        const bool s1 = is_static(), s2 = other.is_static();
        if (s1 && s2) {
            // Self assignment is fine, handled in the static.
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
            // Swap with other. Self-assignment is fine, mpz_swap() can have
            // aliasing arguments.
            mpz_swap(&g_dy(), &other.g_dy());
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
        mpz_clear_wrap(g_dy());
        g_dy().~d_storage();
    }
    // Check storage type.
    MPPP_NODISCARD bool is_static() const
    {
        return m_st._mp_alloc == s_storage::s_alloc;
    }
    MPPP_NODISCARD bool is_dynamic() const
    {
        return m_st._mp_alloc != s_storage::s_alloc;
    }
    // Getters for st and dy.
    MPPP_NODISCARD const s_storage &g_st() const
    {
        assert(is_static());
        return m_st;
    }
    s_storage &g_st()
    {
        assert(is_static());
        return m_st;
    }
    MPPP_NODISCARD const d_storage &g_dy() const
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
        // Get a static_view.
        const auto v = g_st().get_mpz_view();
        if (nlimbs == 0u) {
            // If nlimbs is zero, we will allocate exactly the needed
            // number of limbs to represent this.
            mpz_init_set_nlimbs(tmp_mpz, v);
        } else {
            // Otherwise, we preallocate nlimbs and then set tmp_mpz
            // to the value of this.
            mpz_init_nlimbs(tmp_mpz, nlimbs);
            mpz_set(&tmp_mpz, &v);
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
        const auto dyn_size = get_mpz_size(&g_dy());
        // If the dynamic size is greater than the static size, we cannot demote.
        if (dyn_size > SSize) {
            return false;
        }
        // Copy over the limbs to temporary storage.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        std::array<::mp_limb_t, SSize> tmp;
        copy_limbs_no(g_dy()._mp_d, g_dy()._mp_d + dyn_size, tmp.data());
        const auto signed_size = g_dy()._mp_size;
        // Destroy the dynamic storage.
        destroy_dynamic();
        // Init the static storage with the saved data. The unused limbs will be zeroed
        // by the invoked static_int ctor.
        ::new (static_cast<void *>(&m_st)) s_storage{signed_size, tmp.data(), dyn_size};
        return true;
    }
    // Negation.
    void neg()
    {
        if (is_static()) {
            g_st()._mp_size = -g_st()._mp_size;
        } else {
            mpz_neg(&g_dy(), &g_dy());
        }
    }
    // NOTE: keep these public as we need them below.
    s_storage m_st;
    d_storage m_dy;
};
} // namespace detail

// Fwd declarations.
template <std::size_t SSize>
integer<SSize> &sqrt(integer<SSize> &, const integer<SSize> &);

template <std::size_t SSize>
integer<SSize> &sqr(integer<SSize> &, const integer<SSize> &);

namespace detail
{

template <std::size_t SSize>
void nextprime_impl(integer<SSize> &, const integer<SSize> &);

} // namespace detail

// Detect C++ arithmetic types compatible with integer.
template <typename T>
using is_integer_cpp_arithmetic = detail::conjunction<is_cpp_arithmetic<T>
#if !defined(MPPP_WITH_MPFR)
                                                      ,
                                                      detail::negation<std::is_same<T, long double>>
#endif
                                                      >;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL integer_cpp_arithmetic = is_integer_cpp_arithmetic<T>::value;

#endif

// Detect C++ complex types compatible with integer.
template <typename T>
using is_integer_cpp_complex = detail::conjunction<is_cpp_complex<T>
#if !defined(MPPP_WITH_MPFR)
                                                   ,
                                                   detail::negation<std::is_same<T, std::complex<long double>>>
#endif
                                                   >;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL integer_cpp_complex = is_integer_cpp_complex<T>::value;

#endif

namespace detail
{

// For private use only.
template <typename T>
using is_integer_cpp_floating_point = detail::conjunction<is_cpp_floating_point<T>
#if !defined(MPPP_WITH_MPFR)
                                                          ,
                                                          detail::negation<std::is_same<T, long double>>
#endif
                                                          >;

} // namespace detail

// NOTE: a few misc future directions:
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
// - functions still to be de-branched: all the mpn implementations, if worth it.
//   Probably better to wait for benchmarks before moving.
// - for s11n, we could consider implementing binary_save/load overloads based on iterators. These could return
//   the iterator at the end of the serialised representation, and maybe the original iterator if errors
//   occurred? Probably some code could be refactored/shared with the char * interface, taking advantage
//   of the fact that std::copy() actually returns something. Longer term, we probably should add optional
//   support for boost.serialization, cereal, etc.
// - for the comparison operators, we should consider using lower level primitives for comparisons with
//   both C++ integrals and FP, instead of going through type conversions (same as done for add_ui() etc.).
// - perhaps we should consider adding new overloads to functions which return more than one value
//   (e.g., tdiv_qr(), sqrtrem(), etc.). At this time we have only the GMP-style overload, perhaps
//   we could have an overload that returns the two values as tuple/pair/array.
// - the lt/gt static implementations could be specialised for 2-limbs integers. But we need to have
//   benchmarks before doing it.
// - Regarding complex interoperability (for integer but rational as well): it seems like for
//   mixed-mode binary operations there might be benefits in converting the integer argument not
//   to complex<T> (as we are doing now), but rather T, because like this we end up using
//   real vs complex rather than complex vs complex primitives. It's not however 100% clear
//   to me that proceeding like this is always equivalent to doing the complex promotion
//   (which is what the usual type coercion rules would dictate),
//   and in any case the performance of integer vs complex arithmetics is not a high
//   priority at this time. Perhaps revisit this topic in the future.
// - We can probably remove the public dependency on the GMP library by writing
//   thin wrappers for all the invoked GMP functions, implemented in the mp++ library.
//   Same for MPFR.
// - Implement a binary version of fac_ui(). Note that it will not be able to be called
//   via ADL because the argument is a primitive type.

// NOTE: about the nails:
// - whenever we need to read the *numerical value* of a limb (e.g., in our optimised primitives),
//   we don't make any assumption about nail bits. That is, we assume that anything could be in that bit,
//   so we always mask the limb read with GMP_NUMB_MASK (which, today, is a no-op on virtually all setups).
//   This ensures that we are actually working with a subsection of a multiprecision value, uncontaminated
//   by extraneous bits. If, on the other hand, we are just doing a copy of the limb we don't care about
//   the nail bit as we are not using the numerical value of the limb, and any extraneous bit has no
//   consequence on any computation.
// - If we are using the mpz/mpn API, we don't care about nail bits because we assume that is handled by
//   the GMP library itself. Not our problem.
// - When we produce manually (i.e., without using the mpz/mpn API) some limb to be written out to
//   an integer, we take care of ensuring that no nail bits are set. This is actually currently required
//   by the GMP API:
//
//   "All the mpn functions accepting limb data will expect the nail bits to be zero on entry, and will
//    return data with the nails similarly all zero."
//
//    Again, this masking before writing will be a no-op in most cases. And in many cases we don't have
//    to do anything explicitly as many of our primitives require no nails or they just don't end up
//    writing into nail bits (see the shift operators for an exception to this).
// - This whole topic of nails is at this time largely academic and unlikely to have any real impact,
//   as it seems like nobody builds GMP with nails support nowadays. If we ever want to get rid of
//   all the masking, see commit 30c23c8984d2955d19c35af84e7845dba88d94c0 for a starting point.

// NOTE: about the zeroing of the upper limbs in static_int:
// - the idea here is that, to implement optimised basic primitives for small static sizes, it is convenient
//   to rely on the fact that unused limbs are zeroed out. This can reduce branching and it generally
//   simplifies the code.
// - Because of this, whenever we write into a "small" static int, we must take care of zeroing out the upper limbs
//   that are unused in the representation of the current value. This holds both when using the mpn/mpz functions
//   and when implementing our own specialised primitives (note however that if we can prove that we are calling mpn
//   or mpz on a non-small integer, then we can omit the call to zero_upper_limbs() - we do this occasionally).
// - If the static size is small, we can copy/assign limb arrays around without caring for the effective size, as
//   all limbs are always initialised to some value. With large static size, we cannot do that as the upper
//   limbs are not necessarily inited.
// - Contrary to the nail business, ensuring that the upper limbs are zeroed is essential, and it is something
//   we check in the unit tests.

// Multiprecision integer class.
template <std::size_t SSize>
class integer
{
#if defined(MPPP_WITH_BOOST_S11N)
    // Boost serialization support.
    friend class boost::serialization::access;

    template <typename Archive>
    void save(Archive &ar, unsigned) const
    {
        ar << to_string();
    }

    template <typename Archive>
    void load(Archive &ar, unsigned)
    {
        std::string tmp;
        ar >> tmp;

        *this = integer{tmp};
    }

    // Overloads for binary archives.
    void save(boost::archive::binary_oarchive &ar, unsigned) const
    {
        MPPP_MAYBE_TLS std::vector<char> buffer;
        binary_save(buffer);

        // Record the size and the raw data.
        ar << buffer.size();
        ar << boost::serialization::make_binary_object(buffer.data(), detail::safe_cast<std::size_t>(buffer.size()));
    }

    void load(boost::archive::binary_iarchive &ar, unsigned)
    {
        MPPP_MAYBE_TLS std::vector<char> buffer;

        // Recover the size.
        decltype(buffer.size()) s;
        ar >> s;
        buffer.resize(s);

        ar >> boost::serialization::make_binary_object(buffer.data(), detail::safe_cast<std::size_t>(buffer.size()));

        binary_load(buffer);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif

    // Typedefs for ease of use.
    using s_storage = detail::static_int<SSize>;
    using d_storage = detail::mpz_struct_t;
    // The underlying static int.
    using s_int = s_storage;
    // mpz view class.
    // NOTE: this class looks more complicated than it seemingly should be - it looks like
    // it should be enough to shallow-copy around an mpz_struct_t with the limbs pointing
    // either to a static integer or a dynamic one. This option, however, has a problem:
    // in case of a dynamic integer, one can get 2 different mpz_struct_t instances, one
    // via m_int.g_dy(), the other via the view class, which internally point to the same
    // limbs vector. Having 2 *distinct* mpz_t point to the same limbs vector is not
    // something which is supported by the GMP API, at least when one mpz_t is being written into.
    // This happens for instance when using arithmetic functions
    // in which the return object is the same as one of the operands (GMP supports aliasing
    // if multiple mpz_t are the same object, but in this case they are distinct objects.)
    //
    // In the current implementation, mixing dynamic views with mpz_t objects in the GMP
    // API is fine, as dynamic views point to unique mpz_t objects within the integers.
    // We still however have potential for aliasing when using the static views: these
    // do not point to "real" mpz_t objects, they are mpz_t objects internally pointing
    // to the limbs of static integers. So, for instance, in an add() operation with
    // rop dynamic, and op1 and op2 statics and the same object, we create two separate
    // static views pointing to the same limbs internally, and feed them to the GMP API.
    //
    // This seems to work however: the data in the static views is strictly read-only and
    // it cannot be modified by a modification to a rop, as this needs to be a real mpz_t
    // in order to be used in the GMP API (views are convertible to const mpz_t only,
    // not mutable mpz_t). That is, static views and rops can only point to distinct
    // limbs vectors.
    //
    // The bottom line is that, in practice, the GMP API does not seem to be bothered by the fact
    // that const arguments might overlap behind its back. If this ever becomes a problem,
    // we can consider a "true" static view which does not simply point to an existing
    // static integer but actually copies it as a data member.
    struct mpz_view {
        explicit mpz_view(const integer &n)
            // NOTE: explicitly initialize mpz_struct_t() in order to avoid reading from
            // uninited memory in the move constructor, in case of a dynamic view.
            : m_static_view(n.is_static() ? n.m_int.g_st().get_mpz_view() : detail::mpz_struct_t()),
              m_ptr(n.is_static() ? &m_static_view : &(n.m_int.g_dy()))
        {
        }
        mpz_view(const mpz_view &) = delete;
        mpz_view(mpz_view &&other) noexcept
            : m_static_view(other.m_static_view),
              // NOTE: we need to re-init ptr here if other is a static view, because in that case
              // other.m_ptr will be pointing to an mpz_struct_t in other and we want it to point to
              // the struct in this now.
              m_ptr((other.m_ptr == &other.m_static_view) ? &m_static_view : other.m_ptr)
        {
        }
        ~mpz_view() = default;
        mpz_view &operator=(const mpz_view &) = delete;
        mpz_view &operator=(mpz_view &&) = delete;
        // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
        operator const detail::mpz_struct_t *() const
        {
            return get();
        }
        MPPP_NODISCARD const detail::mpz_struct_t *get() const
        {
            return m_ptr;
        }
        detail::mpz_struct_t m_static_view;
        const detail::mpz_struct_t *m_ptr;
    };
    // Make friends with rational.
    template <std::size_t>
    friend class rational;

public:
    // Alias for the template parameter SSize.
    static constexpr std::size_t ssize = SSize;
    // Default constructor.
    integer() = default;
    // Copy constructor.
    integer(const integer &) = default;
    // Move constructor.
    // NOLINTNEXTLINE(hicpp-noexcept-move, performance-noexcept-move-constructor)
    integer(integer &&other) = default;
    // Constructor from an array of limbs.
    explicit integer(const ::mp_limb_t *p, std::size_t size) : m_int(p, size) {}
    // Constructor from number of bits.
    explicit integer(integer_bitcnt_t nbits) : m_int(nbits) {}
    // Generic constructor.
#if defined(MPPP_HAVE_CONCEPTS)
    template <typename T>
    requires integer_cpp_arithmetic<T> &&(!cpp_integral<T>)
#else
    template <
        typename T,
        detail::enable_if_t<
            detail::conjunction<is_integer_cpp_arithmetic<T>, detail::negation<is_cpp_integral<T>>>::value, int> = 0>
#endif
        explicit integer(const T &x)
        : m_int(x)
    {
    }
#if defined(MPPP_HAVE_CONCEPTS)
    template <typename T>
    requires integer_cpp_arithmetic<T> && cpp_integral<T>
#else
    template <typename T, detail::enable_if_t<
                              detail::conjunction<is_integer_cpp_arithmetic<T>, is_cpp_integral<T>>::value, int> = 0>
#endif
    integer(const T &x) : m_int(x)
    {
    }
    // Generic constructor from a C++ complex type.
#if defined(MPPP_HAVE_CONCEPTS)
    template <integer_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_integer_cpp_complex<T>::value, int> = 0>
#endif
    explicit integer(const T &c)
        : integer(c.imag() == 0
                      ? c.real()
                      : throw std::domain_error(
                          "Cannot construct an integer from a complex C++ value with a non-zero imaginary part of "
                          + detail::to_string(c.imag())))
    {
    }

private:
    // A tag to call private ctors.
    struct ptag {
    };
    explicit integer(const ptag &, const char *s, int base) : m_int(s, base) {}
    explicit integer(const ptag &, const std::string &s, int base) : integer(s.c_str(), base) {}
#if defined(MPPP_HAVE_STRING_VIEW)
    explicit integer(const ptag &, const std::string_view &s, int base) : integer(s.data(), s.data() + s.size(), base)
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
    explicit integer(const T &s, int base = 10) : integer(ptag{}, s, base)
    {
    }
    // Constructor from range of characters.
    explicit integer(const char *begin, const char *end, int base = 10) : m_int(begin, end, base) {}
    // Copy constructor from mpz_t.
    explicit integer(const ::mpz_t n) : m_int(n) {}
#if !defined(_MSC_VER)
    // Move constructor from mpz_t.
    // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
    explicit integer(::mpz_t &&n) : m_int(std::move(n)) {}
#endif
    ~integer() = default;
    // Copy assignment operator.
    integer &operator=(const integer &) = default;
    // Move assignment operator.
    // NOLINTNEXTLINE(hicpp-noexcept-move, performance-noexcept-move-constructor)
    integer &operator=(integer &&) = default;

private:
    // Implementation of the assignment from unsigned C++ integral.
    template <typename T, bool Neg = false,
              detail::enable_if_t<detail::conjunction<detail::is_integral<T>, detail::is_unsigned<T>>::value, int> = 0>
    void dispatch_assignment(T n)
    {
        const auto s = is_static();
        if (n <= GMP_NUMB_MAX) {
            // Optimise the case in which n fits in a single limb.
            const auto size = static_cast<detail::mpz_size_t>(n != 0);
            if (s) {
                // Just write the limb into static storage.
                m_int.g_st()._mp_size = Neg ? -size : size;
                m_int.g_st().m_limbs[0] = static_cast<::mp_limb_t>(n);
                // Zero fill the remaining limbs.
                m_int.g_st().zero_upper_limbs(1);
            } else {
                // Destroy the dynamic structure, re-init an appropriate static.
                m_int.destroy_dynamic();
                // The constructor will take care of zeroing the upper limbs.
                ::new (static_cast<void *>(&m_int.m_st)) s_storage(Neg ? -size : size, static_cast<::mp_limb_t>(n));
            }
            return;
        }
        // Convert n into an array of limbs.
        detail::limb_array_t<T> tmp;
        const auto size = detail::uint_to_limb_array(tmp, n);
        if (s && size <= SSize) {
            // this is static, and n also fits in static. Overwrite the existing value.
            // NOTE: we know size is small, casting is fine.
            m_int.g_st()._mp_size = static_cast<detail::mpz_size_t>(size);
            detail::copy_limbs_no(tmp.data(), tmp.data() + size, m_int.g_st().m_limbs.data());
            // Zero fill the remaining limbs.
            m_int.g_st().zero_upper_limbs(size);
        } else if (!s && size > SSize) {
            // this is dynamic and n requires dynamic storage.
            // Convert the size to detail::mpz_size_t, do it before anything else for exception safety.
            const auto new_mpz_size = detail::safe_cast<detail::mpz_size_t>(size);
            if (m_int.g_dy()._mp_alloc < new_mpz_size) {
                // There's not enough space for the new integer. We'll clear the existing
                // mpz_t (but not destory it), and re-init with the necessary number of limbs.
                detail::mpz_clear_wrap(m_int.g_dy());
                // NOTE: do not use g_dy() here, as in principle mpz_clear() could touch
                // the _mp_alloc member in unpredictable ways, and then g_dy() would assert
                // out in debug builds.
                detail::mpz_init_nlimbs(m_int.m_dy, size);
            }
            // Assign the new size.
            m_int.g_dy()._mp_size = new_mpz_size;
            // Copy over.
            detail::copy_limbs_no(tmp.data(), tmp.data() + size, m_int.g_dy()._mp_d);
        } else if (s && size > SSize) {
            // this is static and n requires dynamic storage.
            const auto new_mpz_size = detail::safe_cast<detail::mpz_size_t>(size);
            // Destroy static.
            m_int.g_st().~s_storage();
            // Init the dynamic struct.
            ::new (static_cast<void *>(&m_int.m_dy)) d_storage;
            // Init to zero, with the necessary amount of allocated limbs.
            // NOTE: need to use m_dy instead of g_dy() here as usual: the alloc
            // tag has not been set yet.
            detail::mpz_init_nlimbs(m_int.m_dy, size);
            // Assign the new size.
            m_int.g_dy()._mp_size = new_mpz_size;
            // Copy over.
            detail::copy_limbs_no(tmp.data(), tmp.data() + size, m_int.g_dy()._mp_d);
        } else {
            // This is dynamic and n fits into static.
            assert(!s && size <= SSize);
            // Destroy the dynamic storage.
            m_int.destroy_dynamic();
            // Init a static with the content from tmp. The constructor
            // will zero the upper limbs.
            ::new (static_cast<void *>(&m_int.m_st)) s_storage{static_cast<detail::mpz_size_t>(size), tmp.data(), size};
        }
        // Negate if requested.
        if (Neg) {
            neg();
        }
    }
    // Assignment from signed integral: take its abs() and negate if necessary, as usual.
    template <typename T,
              detail::enable_if_t<detail::conjunction<detail::is_integral<T>, detail::is_signed<T>>::value, int> = 0>
    void dispatch_assignment(T n)
    {
        if (n >= T(0)) {
            // Positive value, just cast to unsigned.
            dispatch_assignment(detail::make_unsigned(n));
        } else {
            // Negative value, use its abs.
            dispatch_assignment<detail::make_unsigned_t<T>, true>(detail::nint_abs(n));
        }
    }
    // Special casing for bool.
    void dispatch_assignment(bool n)
    {
        if (is_static()) {
            m_int.g_st()._mp_size = static_cast<detail::mpz_size_t>(n);
            m_int.g_st().m_limbs[0] = static_cast<::mp_limb_t>(n);
            // Zero out the upper limbs.
            m_int.g_st().zero_upper_limbs(1);
        } else {
            m_int.destroy_dynamic();
            // Construct from size and single limb. This will zero the upper limbs.
            ::new (static_cast<void *>(&m_int.m_st))
                s_storage{static_cast<detail::mpz_size_t>(n), static_cast<::mp_limb_t>(n)};
        }
    }
    // Assignment from float/double. Uses the mpz_set_d() function.
    template <typename T,
              detail::enable_if_t<detail::disjunction<std::is_same<T, float>, std::is_same<T, double>>::value, int> = 0>
    void dispatch_assignment(T x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot assign the non-finite floating-point value " + detail::to_string(x)
                                    + " to an integer");
        }
        MPPP_MAYBE_TLS detail::mpz_raii tmp;
        mpz_set_d(&tmp.m_mpz, static_cast<double>(x));
        *this = &tmp.m_mpz;
    }
#if defined(MPPP_WITH_MPFR)
    // Assignment from long double, requires MPFR.
    void dispatch_assignment(long double x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot assign the non-finite floating-point value " + detail::to_string(x)
                                    + " to an integer");
        }
        // NOTE: static checks for overflows and for the precision value are done in mpfr.hpp.
        constexpr int d2 = std::numeric_limits<long double>::max_digits10 * 4;
        MPPP_MAYBE_TLS detail::mpfr_raii mpfr(static_cast<::mpfr_prec_t>(d2));
        MPPP_MAYBE_TLS detail::mpz_raii tmp;
        ::mpfr_set_ld(&mpfr.m_mpfr, x, MPFR_RNDN);
        ::mpfr_get_z(&tmp.m_mpz, &mpfr.m_mpfr, MPFR_RNDZ);
        *this = &tmp.m_mpz;
    }
#endif

public:
    // Generic assignment operator from a fundamental C++ type.
#if defined(MPPP_HAVE_CONCEPTS)
    template <integer_cpp_arithmetic T>
#else
    template <typename T, detail::enable_if_t<is_integer_cpp_arithmetic<T>::value, int> = 0>
#endif
    integer &operator=(const T &x)
    {
        dispatch_assignment(x);
        return *this;
    }
    // Generic assignment operator from a complex C++ type.
#if defined(MPPP_HAVE_CONCEPTS)
    template <integer_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_integer_cpp_complex<T>::value, int> = 0>
#endif
    integer &operator=(const T &c)
    {
        if (mppp_unlikely(c.imag() != 0)) {
            throw std::domain_error("Cannot assign a complex C++ value with a non-zero imaginary part of "
                                    + detail::to_string(c.imag()) + " to an integer");
        }
        // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
        return *this = c.real();
    }

    // Declaration of the assignments from
    // other mp++ classes.
    integer &operator=(const rational<SSize> &);
#if defined(MPPP_WITH_QUADMATH)
    integer &operator=(const real128 &);
    integer &operator=(const complex128 &);
#endif
#if defined(MPPP_WITH_MPFR)
    integer &operator=(const real &);
#endif
#if defined(MPPP_WITH_MPC)
    integer &operator=(const complex &);
#endif

    // Assignment from string.
#if defined(MPPP_HAVE_CONCEPTS)
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    integer &operator=(const T &s)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
        return *this = integer{s};
    }
    // Copy assignment from mpz_t.
    integer &operator=(const ::mpz_t n)
    {
        const auto asize = detail::get_mpz_size(n);
        const auto s = is_static();
        if (s && asize <= SSize) {
            // this is static, n fits into static. Copy over.
            m_int.g_st()._mp_size = n->_mp_size;
            detail::copy_limbs_no(n->_mp_d, n->_mp_d + asize, m_int.g_st().m_limbs.data());
            // Zero the non-copied limbs, if necessary.
            m_int.g_st().zero_upper_limbs(asize);
        } else if (!s && asize > SSize) {
            // Dynamic to dynamic.
            mpz_set(&m_int.m_dy, n);
        } else if (s && asize > SSize) {
            // this is static, n is too big. Promote and assign.
            // Destroy static.
            m_int.g_st().~s_storage();
            // Init dynamic.
            ::new (static_cast<void *>(&m_int.m_dy)) d_storage;
            detail::mpz_init_set_nlimbs(m_int.m_dy, *n);
        } else {
            // This is dynamic and n fits into static.
            assert(!s && asize <= SSize);
            // Destroy the dynamic storage.
            m_int.destroy_dynamic();
            // Init a static with the content from n. This will zero the upper limbs.
            ::new (static_cast<void *>(&m_int.m_st)) s_storage{n->_mp_size, n->_mp_d, asize};
        }
        return *this;
    }
#if !defined(_MSC_VER)
    // Move assignment from mpz_t.
    integer &operator=(::mpz_t &&n)
    {
        const auto asize = detail::get_mpz_size(n);
        const auto s = is_static();
        if (s && asize <= SSize) {
            // this is static, n fits into static. Copy over.
            m_int.g_st()._mp_size = n->_mp_size;
            detail::copy_limbs_no(n->_mp_d, n->_mp_d + asize, m_int.g_st().m_limbs.data());
            // Zero the non-copied limbs, if necessary.
            m_int.g_st().zero_upper_limbs(asize);
            // Clear out n.
            detail::mpz_clear_wrap(*n);
        } else if (!s && asize > SSize) {
            // Dynamic to dynamic: clear this, shallow copy n.
            detail::mpz_clear_wrap(m_int.m_dy);
            m_int.m_dy = *n;
        } else if (s && asize > SSize) {
            // this is static, n is too big. Promote and assign.
            // Destroy static.
            m_int.g_st().~s_storage();
            // Init dynamic with a shallow copy.
            ::new (static_cast<void *>(&m_int.m_dy)) d_storage(*n);
        } else {
            // This is dynamic and n fits into static.
            assert(!s && asize <= SSize);
            // Destroy the dynamic storage.
            m_int.destroy_dynamic();
            // Init a static with the content from n. This will zero the upper limbs.
            ::new (static_cast<void *>(&m_int.m_st)) s_storage{n->_mp_size, n->_mp_d, asize};
            // Clear out n.
            detail::mpz_clear_wrap(*n);
        }
        return *this;
    }
#endif
    // Set to zero.
    integer &set_zero()
    {
        if (is_static()) {
            m_int.g_st()._mp_size = 0;
            // Zero out the whole limbs array, if needed.
            m_int.g_st().zero_upper_limbs(0);
        } else {
            m_int.destroy_dynamic();
            // Def construction of static results in zero.
            ::new (static_cast<void *>(&m_int.m_st)) s_storage();
        }
        return *this;
    }

private:
    template <bool PlusOrMinus>
    integer &set_one_impl()
    {
        if (is_static()) {
            m_int.g_st()._mp_size = PlusOrMinus ? 1 : -1;
            m_int.g_st().m_limbs[0] = 1;
            // Zero the unused limbs, if needed.
            m_int.g_st().zero_upper_limbs(1);
        } else {
            m_int.destroy_dynamic();
            // Construct from a single limb the static. This will zero any unused limb.
            ::new (static_cast<void *>(&m_int.m_st)) s_storage{PlusOrMinus ? 1 : -1, 1u};
        }
        return *this;
    }

public:
    // Set to one.
    integer &set_one()
    {
        return set_one_impl<true>();
    }
    // Set to minus one.
    integer &set_negative_one()
    {
        return set_one_impl<false>();
    }
    // Test for static storage.
    MPPP_NODISCARD bool is_static() const
    {
        return m_int.is_static();
    }
    // Test for dynamic storage.
    MPPP_NODISCARD bool is_dynamic() const
    {
        return m_int.is_dynamic();
    }
    // Conversion to string.
    MPPP_NODISCARD std::string to_string(int base = 10) const
    {
        if (mppp_unlikely(base < 2 || base > 62)) {
            throw std::invalid_argument("Invalid base for string conversion: the base must be between "
                                        "2 and 62, but a value of "
                                        + detail::to_string(base) + " was provided instead");
        }
        return detail::mpz_to_str(get_mpz_view(), base);
    }
    // NOTE: maybe provide a member function to access the lower-level str conversion that writes to
    // std::vector<char>?

private:
    // Conversion to bool.
    template <typename T, detail::enable_if_t<std::is_same<bool, T>::value, int> = 0>
    MPPP_NODISCARD std::pair<bool, T> dispatch_conversion() const
    {
        return std::make_pair(true, m_int.m_st._mp_size != 0);
    }
    // Implementation of the conversion to unsigned types which fit in a limb.
    template <typename T, bool Sign, detail::enable_if_t<(detail::nl_constants<T>::digits <= GMP_NUMB_BITS), int> = 0>
    MPPP_NODISCARD std::pair<bool, T> convert_to_unsigned() const
    {
        static_assert(detail::is_integral<T>::value && detail::is_unsigned<T>::value, "Invalid type.");
        assert((Sign && m_int.m_st._mp_size > 0) || (!Sign && m_int.m_st._mp_size < 0));
        if ((Sign && m_int.m_st._mp_size != 1) || (!Sign && m_int.m_st._mp_size != -1)) {
            // If the asize is not 1, the conversion will fail.
            return std::make_pair(false, T(0));
        }
        // Get the pointer to the limbs.
        const ::mp_limb_t *ptr = is_static() ? m_int.g_st().m_limbs.data() : m_int.g_dy()._mp_d;
        if ((ptr[0] & GMP_NUMB_MASK) > detail::nl_max<T>()) {
            // The only limb has a value which exceeds the limit of T.
            return std::make_pair(false, T(0));
        }
        // There's a single limb and the result fits.
        return std::make_pair(true, static_cast<T>(ptr[0] & GMP_NUMB_MASK));
    }
    // Implementation of the conversion to unsigned types which do not fit in a limb.
    template <typename T, bool Sign, detail::enable_if_t<(detail::nl_constants<T>::digits > GMP_NUMB_BITS), int> = 0>
    MPPP_NODISCARD std::pair<bool, T> convert_to_unsigned() const
    {
        static_assert(detail::is_integral<T>::value && detail::is_unsigned<T>::value, "Invalid type.");
        assert((Sign && m_int.m_st._mp_size > 0) || (!Sign && m_int.m_st._mp_size < 0));
        const auto asize = Sign ? static_cast<std::size_t>(m_int.m_st._mp_size)
                                : static_cast<std::size_t>(detail::nint_abs(m_int.m_st._mp_size));
        // Get the pointer to the limbs.
        const ::mp_limb_t *ptr = is_static() ? m_int.g_st().m_limbs.data() : m_int.g_dy()._mp_d;
        // Init the retval with the first limb. This is safe as T has more bits than the limb type.
        auto retval = static_cast<T>(ptr[0] & GMP_NUMB_MASK);
        // Add the other limbs, if any.
        constexpr unsigned u_bits = detail::nl_digits<T>();
        unsigned shift(GMP_NUMB_BITS);
        for (std::size_t i = 1u; i < asize; ++i, shift += unsigned(GMP_NUMB_BITS)) {
            if (shift >= u_bits) {
                // We need to shift left the current limb. If the shift is too large, we run into UB
                // and it also means that the value does not fit in T.
                return std::make_pair(false, T(0));
            }
            // Get the current limb. Safe as T has more bits than the limb type.
            const auto l = static_cast<T>(ptr[i] & GMP_NUMB_MASK);
            // LCOV_EXCL_START
            if (l >> (u_bits - shift)) {
                // Left-shifting the current limb is well-defined from the point of view of the language, but the result
                // overflows: the value does not fit in T.
                // NOTE: I suspect this branch can be triggered on common architectures only with nail builds.
                return std::make_pair(false, T(0));
            }
            // LCOV_EXCL_STOP
            // This will not overflow, as there is no carry from retval and l << shift is fine.
            retval = static_cast<T>(retval + (l << shift));
        }
        return std::make_pair(true, retval);
    }
    // Conversion to unsigned ints, excluding bool.
    template <typename T, detail::enable_if_t<detail::conjunction<detail::is_integral<T>, detail::is_unsigned<T>,
                                                                  detail::negation<std::is_same<bool, T>>>::value,
                                              int> = 0>
    MPPP_NODISCARD std::pair<bool, T> dispatch_conversion() const
    {
        // Handle zero.
        if (!m_int.m_st._mp_size) {
            return std::make_pair(true, T(0));
        }
        // Handle negative value.
        if (m_int.m_st._mp_size < 0) {
            return std::make_pair(false, T(0));
        }
        return convert_to_unsigned<T, true>();
    }
    // Conversion to signed ints.
    //
    // NOTE: the implementation of conversion to signed at the moment is split into 2 branches:
    // a specialised implementation for T not larger than the limb type, and a slower implementation
    // for T larger than the limb type. The slower implementation is based on the conversion
    // to the unsigned counterpart of T, and it can probably be improved performance-wise.
    //
    // Helper type trait to detect conversion to small signed integers (i.e., the absolute values of T fit
    // into a limb). We need this instead of just typedeffing an std::integral_constant because MSVC
    // chokes on constexpr functions in a SFINAE context.
    template <typename T>
    struct sconv_is_small {
        static const bool value
            = detail::c_max(detail::make_unsigned(detail::nl_max<T>()), detail::nint_abs(detail::nl_min<T>()))
              <= GMP_NUMB_MAX;
    };
    // Overload if the all the absolute values of T fit into a limb.
    template <typename T, detail::enable_if_t<sconv_is_small<T>::value, int> = 0>
    MPPP_NODISCARD std::pair<bool, T> convert_to_signed() const
    {
        static_assert(detail::is_integral<T>::value && detail::is_signed<T>::value, "Invalid type.");
        assert(size());
        // Cache for convenience.
        constexpr auto Tmax = detail::make_unsigned(detail::nl_max<T>());
        if (m_int.m_st._mp_size != 1 && m_int.m_st._mp_size != -1) {
            // this consists of more than 1 limb, the conversion is not possible.
            return std::make_pair(false, T(0));
        }
        // Get the pointer to the limbs.
        const ::mp_limb_t *ptr = is_static() ? m_int.g_st().m_limbs.data() : m_int.g_dy()._mp_d;
        // The candidate output value.
        const ::mp_limb_t candidate = ptr[0] & GMP_NUMB_MASK;
        // Branch out on the sign.
        if (m_int.m_st._mp_size > 0) {
            // This is positive, it needs to fit within max().
            if (candidate <= Tmax) {
                return std::make_pair(true, static_cast<T>(candidate));
            }
            return std::make_pair(false, T(0));
        } else {
            return detail::unsigned_to_nsigned<T>(candidate);
        }
    }
    // Overload if not all the absolute values of T fit into a limb.
    template <typename T, detail::enable_if_t<!sconv_is_small<T>::value, int> = 0>
    MPPP_NODISCARD std::pair<bool, T> convert_to_signed() const
    {
        // Cache for convenience.
        constexpr auto Tmax = detail::make_unsigned(detail::nl_max<T>());
        // Branch out depending on the sign of this.
        if (m_int.m_st._mp_size > 0) {
            // Attempt conversion to the unsigned counterpart.
            const auto candidate = convert_to_unsigned<detail::make_unsigned_t<T>, true>();
            if (candidate.first && candidate.second <= Tmax) {
                // The conversion to unsigned was successful, and the result fits in
                // the positive range of T. Return the result.
                return std::make_pair(true, static_cast<T>(candidate.second));
            }
            // The conversion to unsigned failed, or the result does not fit.
            return std::make_pair(false, T(0));
        } else {
            // Attempt conversion to the unsigned counterpart.
            const auto candidate = convert_to_unsigned<detail::make_unsigned_t<T>, false>();
            if (candidate.first) {
                // The converstion to unsigned was successful, try to negate now.
                return detail::unsigned_to_nsigned<T>(candidate.second);
            }
            // The conversion to unsigned failed.
            return std::make_pair(false, T(0));
        }
    }
    template <typename T,
              detail::enable_if_t<detail::conjunction<detail::is_integral<T>, detail::is_signed<T>>::value, int> = 0>
    MPPP_NODISCARD std::pair<bool, T> dispatch_conversion() const
    {
        // Handle zero.
        if (!m_int.m_st._mp_size) {
            return std::make_pair(true, T(0));
        }
        return convert_to_signed<T>();
    }
    // Implementation of the conversion to floating-point through GMP/MPFR routines.
    template <typename T,
              detail::enable_if_t<detail::disjunction<std::is_same<T, float>, std::is_same<T, double>>::value, int> = 0>
    static std::pair<bool, T> mpz_float_conversion(const detail::mpz_struct_t &m)
    {
        return std::make_pair(true, static_cast<T>(mpz_get_d(&m)));
    }
#if defined(MPPP_WITH_MPFR)
    template <typename T, detail::enable_if_t<std::is_same<T, long double>::value, int> = 0>
    static std::pair<bool, T> mpz_float_conversion(const detail::mpz_struct_t &m)
    {
        constexpr int d2 = std::numeric_limits<long double>::max_digits10 * 4;
        MPPP_MAYBE_TLS detail::mpfr_raii mpfr(static_cast<::mpfr_prec_t>(d2));
        ::mpfr_set_z(&mpfr.m_mpfr, &m, MPFR_RNDN);
        return std::make_pair(true, ::mpfr_get_ld(&mpfr.m_mpfr, MPFR_RNDN));
    }
#endif
    // Conversion to floating-point.
    template <typename T, detail::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    MPPP_NODISCARD std::pair<bool, T> dispatch_conversion() const
    {
        // Handle zero.
        if (!m_int.m_st._mp_size) {
            return std::make_pair(true, T(0));
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
                return std::make_pair(true, static_cast<T>(ptr[0] & GMP_NUMB_MASK));
            }
            if (m_int.m_st._mp_size == -1) {
                return std::make_pair(true, -static_cast<T>(ptr[0] & GMP_NUMB_MASK));
            }
        }
        // For all the other cases, just delegate to the GMP/MPFR routines.
        return mpz_float_conversion<T>(*static_cast<const detail::mpz_struct_t *>(get_mpz_view()));
    }

public:
    // NOTE: in C++17 and later it is possible to implement implicit conversion
    // towards C++ types higher in the hierarchy (e.g., double). This works in C++17
    // because of mandatory copy elision, which allows to skip a step in the conversion
    // sequence which, in C++14, makes things ambiguous. See the discussion here:
    // https://cpplang.slack.com/archives/C21PKDHSL/p1624893578046400
    // And the code snippet here:
    // https://godbolt.org/z/snK19f5Wf
    // Unfortunately, for some reason clang goes ballistic if we make the conversion
    // operator conditionally explicit, taking huge amounts of time/memory
    // to compile the tests. We can probably revisit this behaviour in the future,
    // perhaps even using C++20's conditionally-explicit syntax rather than
    // SFINAE dispatching.

    // Generic conversion operator to a C++ fundamental type.
#if defined(MPPP_HAVE_CONCEPTS)
    template <integer_cpp_arithmetic T>
#else
    template <typename T, detail::enable_if_t<is_integer_cpp_arithmetic<T>::value, int> = 0>
#endif
    explicit operator T() const
    {
        auto retval = dispatch_conversion<T>();
        if (mppp_unlikely(!retval.first)) {
            throw std::overflow_error("The conversion of the integer " + to_string() + " to the type '" + type_name<T>()
                                      + "' results in overflow");
        }
        return std::move(retval.second);
    }
    // Generic conversion operator to a C++ complex type.
#if defined(MPPP_HAVE_CONCEPTS)
    template <integer_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_integer_cpp_complex<T>::value, int> = 0>
#endif
    explicit operator T() const
    {
        return T(static_cast<typename T::value_type>(*this));
    }
    // Generic conversion member function to a C++ fundamental type.
#if defined(MPPP_HAVE_CONCEPTS)
    template <integer_cpp_arithmetic T>
#else
    template <typename T, detail::enable_if_t<is_integer_cpp_arithmetic<T>::value, int> = 0>
#endif
    bool get(T &rop) const
    {
        auto retval = dispatch_conversion<T>();
        if (retval.first) {
            rop = std::move(retval.second);
            return true;
        }
        return false;
    }
    // Generic conversion member function to a C++ complex type.
#if defined(MPPP_HAVE_CONCEPTS)
    template <integer_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_integer_cpp_complex<T>::value, int> = 0>
#endif
    bool get(T &rop) const
    {
        rop = static_cast<T>(*this);
        return true;
    }
    // Promote to dynamic storage.
    bool promote()
    {
        if (is_static()) {
            m_int.promote();
            return true;
        }
        return false;
    }
    // Demote to static storage.
    bool demote()
    {
        if (is_dynamic()) {
            return m_int.demote();
        }
        return false;
    }
    // Size in bits.
    MPPP_NODISCARD std::size_t nbits() const
    {
        const std::size_t ls = size();
        if (ls == 0u) {
            return 0;
        }
        const ::mp_limb_t *lptr = is_static() ? m_int.g_st().m_limbs.data() : m_int.g_dy()._mp_d;
        // LCOV_EXCL_START
        if (mppp_unlikely(ls > detail::nl_max<std::size_t>() / unsigned(GMP_NUMB_BITS))) {
            throw std::overflow_error("Overflow in the computation of the number of bits required to represent an "
                                      "integer - the limb size is "
                                      + detail::to_string(ls));
        }
        // LCOV_EXCL_STOP
        // Index of the most significant limb.
        const std::size_t idx = ls - 1u;
        return static_cast<std::size_t>(idx * unsigned(GMP_NUMB_BITS) + detail::limb_size_nbits(lptr[idx]));
    }
    // Size in limbs.
    MPPP_NODISCARD std::size_t size() const
    {
        // NOTE: the idea here is that, regardless of what mpz_size_t is exactly, the
        // asize of an integer represents ultimately the size of a limb array, and as such
        // it has to be representable by std::size_t.
        return (m_int.m_st._mp_size) >= 0 ? static_cast<std::size_t>(m_int.m_st._mp_size)
                                          : static_cast<std::size_t>(detail::nint_abs(m_int.m_st._mp_size));
    }
    // Sign.
    MPPP_NODISCARD int sgn() const
    {
        // NOTE: size is part of the common initial sequence.
        return detail::integral_sign(m_int.m_st._mp_size);
    }
    // Get an mpz_t view.
    MPPP_NODISCARD mpz_view get_mpz_view() const
    {
        return mpz_view(*this);
    }
    // Negate in-place.
    integer &neg()
    {
        m_int.neg();
        return *this;
    }
    // In-place absolute value.
    integer &abs()
    {
        if (is_static()) {
            if (m_int.g_st()._mp_size < 0) {
                m_int.g_st()._mp_size = -m_int.g_st()._mp_size;
            }
        } else {
            mpz_abs(&m_int.g_dy(), &m_int.g_dy());
        }
        return *this;
    }
    // Compute next prime number (in-place version).
    integer &nextprime()
    {
        detail::nextprime_impl(*this, *this);
        return *this;
    }
    // Test primality.
    MPPP_NODISCARD int probab_prime_p(int reps = 25) const
    {
        if (mppp_unlikely(reps < 1)) {
            throw std::invalid_argument("The number of primality tests must be at least 1, but a value of "
                                        + detail::to_string(reps) + " was provided instead");
        }
        if (mppp_unlikely(sgn() < 0)) {
            throw std::invalid_argument("Cannot run primality tests on the negative number " + to_string());
        }
        return mpz_probab_prime_p(get_mpz_view(), reps);
    }
    // Integer square root (in-place version).
    integer &sqrt()
    {
        return mppp::sqrt(*this, *this);
    }
    // Integer squaring (in-place version).
    integer &sqr()
    {
        return mppp::sqr(*this, *this);
    }
    // Test if value is odd.
    MPPP_NODISCARD bool odd_p() const
    {
        if (is_static()) {
            if (SSize <= s_storage::opt_size) {
                // NOTE: when SSize is an optimised size, we can be sure the first limb
                // has been zeroed even if the value of the integer is zero.
                return (m_int.g_st().m_limbs[0] & GMP_NUMB_MASK) & ::mp_limb_t(1);
            } else {
                // Otherwise, we add an extra check for zero.
                return m_int.g_st()._mp_size && ((m_int.g_st().m_limbs[0] & GMP_NUMB_MASK) & ::mp_limb_t(1));
            }
        }
        return mpz_odd_p(&m_int.g_dy());
    }
    // Test if value is even.
    MPPP_NODISCARD bool even_p() const
    {
        return !odd_p();
    }
    // Return a reference to the internal union.
    detail::integer_union<SSize> &_get_union()
    {
        return m_int;
    }
    // Return a const reference to the internal union.
    MPPP_NODISCARD const detail::integer_union<SSize> &_get_union() const
    {
        return m_int;
    }
    // Get a pointer to the dynamic storage.
    std::remove_extent<::mpz_t>::type *get_mpz_t()
    {
        promote();
        return &m_int.g_dy();
    }
    // Test if the value is zero.
    MPPP_NODISCARD bool is_zero() const
    {
        return m_int.m_st._mp_size == 0;
    }

private:
    // Implementation of is_one()/is_negative_one().
    template <int One>
    MPPP_NODISCARD bool is_one_impl() const
    {
        if (m_int.m_st._mp_size != One) {
            return false;
        }
        // Get the pointer to the limbs.
        const ::mp_limb_t *ptr = is_static() ? m_int.g_st().m_limbs.data() : m_int.g_dy()._mp_d;
        return (ptr[0] & GMP_NUMB_MASK) == 1u;
    }

public:
    // Test if the value is equal to one.
    MPPP_NODISCARD bool is_one() const
    {
        return is_one_impl<1>();
    }
    // Test if the value is equal to minus one.
    MPPP_NODISCARD bool is_negative_one() const
    {
        return is_one_impl<-1>();
    }

private:
    // NOTE: this needs to be const instead of constexpr due to an MSVC bug.
    static const char binary_size_errmsg[];

public:
    // Size of the serialised binary representation.
    MPPP_NODISCARD std::size_t binary_size() const
    {
        const auto asize = size();
        // LCOV_EXCL_START
        // Here we have the very theoretical situation in which we run into possible overflow,
        // as the limb array and the size member are distinct objects and thus although individually
        // their size must fit in size_t, together they could exceed it.
        if (mppp_unlikely(asize > (std::numeric_limits<std::size_t>::max() - sizeof(detail::mpz_size_t))
                                      / sizeof(::mp_limb_t))) {
            throw std::overflow_error(binary_size_errmsg);
        }
        // LCOV_EXCL_STOP
        return sizeof(detail::mpz_size_t) + asize * sizeof(::mp_limb_t);
    }

private:
    void binary_save_impl(char *dest, std::size_t bs) const
    {
        assert(bs == binary_size());
        // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
        auto ptr = reinterpret_cast<const char *>(&m_int.m_st._mp_size);
        // NOTE: std::copy() has the usual aliasing restrictions to take into account.
        // Here it should not matter, unless one is somehow trying to save an integer
        // into itself (I guess?). It's probably not necessary to put these aliasing
        // restrictions in the user docs.
        std::copy(ptr, ptr + sizeof(detail::mpz_size_t), detail::make_uai(dest));
        ptr = reinterpret_cast<const char *>(is_static() ? m_int.g_st().m_limbs.data() : m_int.g_dy()._mp_d);
        std::copy(ptr, ptr + (bs - sizeof(detail::mpz_size_t)), detail::make_uai(dest + sizeof(detail::mpz_size_t)));
    }

public:
    // Serialise into a memory buffer.
    std::size_t binary_save(char *dest) const
    {
        const auto bs = binary_size();
        binary_save_impl(dest, bs);
        return bs;
    }
    // Serialise into a ``std::vector<char>``.
    std::size_t binary_save(std::vector<char> &dest) const
    {
        const auto bs = binary_size();
        if (dest.size() < bs) {
            dest.resize(detail::safe_cast<decltype(dest.size())>(bs));
        }
        binary_save_impl(dest.data(), bs);
        return bs;
    }
    // Serialise into a ``std::array<char>``.
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
    // Serialise into a ``std::ostream``.
    std::size_t binary_save(std::ostream &dest) const
    {
        const auto bs = binary_size();
        // NOTE: there does not seem to be a reliable way of detecting how many bytes
        // are actually written via write(). See the question here and especially the comments:
        // https://stackoverflow.com/questions/14238572/how-many-bytes-actually-written-by-ostreamwrite
        // Seems almost like tellp() would work, but if an error occurs in the stream, then
        // it returns unconditionally -1, so it is not very useful for our purposes.
        // Thus, we will just return 0 on failure, and the full binary size otherwise.
        //
        // Write the raw data to stream.
        dest.write(reinterpret_cast<const char *>(&m_int.m_st._mp_size),
                   detail::safe_cast<std::streamsize>(sizeof(detail::mpz_size_t)));
        if (!dest.good()) {
            // !dest.good() means that the last write operation failed. Bail out now.
            return 0;
        }
        dest.write(reinterpret_cast<const char *>(is_static() ? m_int.g_st().m_limbs.data() : m_int.g_dy()._mp_d),
                   detail::safe_cast<std::streamsize>(bs - sizeof(detail::mpz_size_t)));
        return dest.good() ? bs : 0u;
    }

private:
    // Error message in case of invalid data during binary deserialisation.
    static const char bl_data_errmsg[];
    // A couple of helpers to check a deserialised integer.
    void bl_dynamic_check(const detail::make_unsigned_t<detail::mpz_size_t> &asize)
    {
        // NOTE: here we are sure that asize is nonzero, as we end up in dynamic storage iff asize > SSize,
        // and SSize is at least 1.
        assert(asize > 0u);
        if (mppp_unlikely(!(m_int.g_dy()._mp_d[static_cast<std::size_t>(asize - 1u)] & GMP_NUMB_MASK))) {
            // Reset to zero before throwing.
            m_int.g_dy()._mp_size = 0;
            throw std::invalid_argument(bl_data_errmsg);
        }
    }
    void bl_static_check(const detail::make_unsigned_t<detail::mpz_size_t> &asize)
    {
        // NOTE: here we need to check that asize is nonzero.
        if (mppp_unlikely(asize && !(m_int.g_st().m_limbs[static_cast<std::size_t>(asize - 1u)] & GMP_NUMB_MASK))) {
            // Reset this to zero before throwing.
            m_int.g_st()._mp_size = 0;
            m_int.g_st().zero_upper_limbs(0);
            throw std::invalid_argument(bl_data_errmsg);
        }
    }
    // Read the size and asize of a serialised integer stored in a buffer
    // starting at src.
    static std::pair<detail::mpz_size_t, detail::make_unsigned_t<detail::mpz_size_t>>
    bl_read_size_asize(const char *src)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
        detail::mpz_size_t size;
        std::copy(src, src + sizeof(detail::mpz_size_t), detail::make_uai(reinterpret_cast<char *>(&size)));
        // NOTE: we don't use std::size_t here for the asize as we don't have any assurance
        // that a value in the std::size_t range was written into the buffer.
        return std::make_pair(size, size >= 0 ? detail::make_unsigned(size) : detail::nint_abs(size));
    }
    // Low level implementation of binary load. src must point to the start of the serialised
    // limb array.
    void binary_load_impl(const char *src, const detail::mpz_size_t &size,
                          const detail::make_unsigned_t<detail::mpz_size_t> &asize)
    {
        // Check for overflow in asize.
        // LCOV_EXCL_START
        if (mppp_unlikely(asize > std::numeric_limits<std::size_t>::max() / sizeof(::mp_limb_t))) {
            throw std::overflow_error(binary_size_errmsg);
        }
        // LCOV_EXCL_STOP
        // Detect current storage.
        const bool s = is_static();
        if (s && asize <= SSize) {
            // this is static, the content of src fit into static storage.
            // Set the size.
            m_int.g_st()._mp_size = size;
            // Copy over the data from the source.
            std::copy(src, src + static_cast<std::size_t>(sizeof(::mp_limb_t) * asize),
                      detail::make_uai(reinterpret_cast<char *>(m_int.g_st().m_limbs.data())));
            // Clear the upper limbs, if needed.
            m_int.g_st().zero_upper_limbs(static_cast<std::size_t>(asize));
            // Check the deserialised value.
            bl_static_check(asize);
        } else if (s && asize > SSize) {
            // this is static, the content of src do not fit into static storage.
            // Destroy static storage.
            m_int.g_st().~s_storage();
            // Construct the dynamic struct.
            ::new (static_cast<void *>(&m_int.m_dy)) d_storage;
            // Init the mpz. This will set the value to zero.
            detail::mpz_init_nlimbs(m_int.m_dy, static_cast<std::size_t>(asize));
            // Set the size.
            m_int.g_dy()._mp_size = size;
            // Copy over the data from the source.
            std::copy(src, src + static_cast<std::size_t>(sizeof(::mp_limb_t) * asize),
                      detail::make_uai(reinterpret_cast<char *>(m_int.g_dy()._mp_d)));
            // Check the deserialised value.
            bl_dynamic_check(asize);
        } else if (!s && asize <= SSize) {
            // this is dynamic, src contains a static integer.
            // Destroy the dynamic this.
            m_int.destroy_dynamic();
            // Def construct the static integer. This will set everything to zero.
            ::new (static_cast<void *>(&m_int.m_st)) s_storage();
            // Set the size.
            m_int.g_st()._mp_size = size;
            // Copy over the data from the source.
            std::copy(src, src + static_cast<std::size_t>(sizeof(::mp_limb_t) * asize),
                      detail::make_uai(reinterpret_cast<char *>(m_int.g_st().m_limbs.data())));
            // NOTE: no need to clear the upper limbs: they were already zeroed out
            // by the default constructor of static_int.
            // Check the deserialised value.
            bl_static_check(asize);
        } else {
            // this is dynamic, src contains a dynamic integer.
            // If this does not have enough storage, we need to allocate.
            if (detail::get_mpz_size(&m_int.g_dy()) < asize) {
                // Clear, but do not destroy, the dynamic storage.
                detail::mpz_clear_wrap(m_int.g_dy());
                // Re-init to zero with the necessary size.
                // NOTE: do not use g_dy() here, as in principle mpz_clear() could touch
                // the _mp_alloc member in unpredictable ways, and then g_dy() would assert
                // out in debug builds.
                detail::mpz_init_nlimbs(m_int.m_dy, static_cast<std::size_t>(asize));
            }
            // Set the size.
            m_int.g_dy()._mp_size = size;
            // Copy over the data from the source.
            std::copy(src, src + static_cast<std::size_t>(sizeof(::mp_limb_t) * asize),
                      detail::make_uai(reinterpret_cast<char *>(m_int.g_dy()._mp_d)));
            // Check the deserialised value.
            bl_dynamic_check(asize);
        }
    }
    // Small helper to determine how many bytes have been read after
    // the successful deserialisation of an integer with abs size asize.
    // NOTE: since we deserialised the integer from a contiguous buffer, the
    // total number of bytes read has to be representable by std::size_t (unlike
    // in binary_size(), where we do have to check for overflow).
    static std::size_t read_bytes(const detail::make_unsigned_t<detail::mpz_size_t> &asize)
    {
        return static_cast<std::size_t>(sizeof(detail::mpz_size_t) + asize * sizeof(::mp_limb_t));
    }

public:
    // Load a value from a memory buffer.
    std::size_t binary_load(const char *src)
    {
        // NOTE: disable the use of structured bindings
        // on MSVC altogether, due to this clang-cl issue:
        // https://bugs.llvm.org/show_bug.cgi?id=41745
#if MPPP_CPLUSPLUS >= 201703L && !defined(_MSC_VER)
        const auto [size, asize] = bl_read_size_asize(src);
#else
        detail::mpz_size_t size;
        detail::make_unsigned_t<detail::mpz_size_t> asize;
        std::tie(size, asize) = bl_read_size_asize(src);
#endif
        binary_load_impl(src + sizeof(detail::mpz_size_t), size, asize);
        return read_bytes(asize);
    }

private:
    // Deserialisation from vector-like type.
    template <typename Vector>
    std::size_t binary_load_vector(const Vector &src, const char *name)
    {
        // Verify we can at least read the size out of src.
        if (mppp_unlikely(src.size() < sizeof(detail::mpz_size_t))) {
            throw std::invalid_argument(std::string("Invalid vector size in the deserialisation of an integer via a ")
                                        + name + ": the " + name + " size must be at least "
                                        + std::to_string(sizeof(detail::mpz_size_t)) + " bytes, but it is only "
                                        + std::to_string(src.size()) + " bytes");
        }
        // Size in bytes of the limbs portion of the data.
        const auto lsize = src.size() - sizeof(detail::mpz_size_t);
#if MPPP_CPLUSPLUS >= 201703L && !defined(_MSC_VER)
        // NOTE: not sure why the coverage is not detected here.
        // LCOV_EXCL_START
        const auto [size, asize] = bl_read_size_asize(src.data());
        // LCOV_EXCL_STOP
#else
        detail::mpz_size_t size;
        detail::make_unsigned_t<detail::mpz_size_t> asize;
        std::tie(size, asize) = bl_read_size_asize(src.data());
#endif
        // The number of entire limbs stored in the vector must be at least the integer
        // limb size stored at the beginning of the vector.
        if (mppp_unlikely(lsize / sizeof(::mp_limb_t) < asize)) {
            throw std::invalid_argument(
                std::string("Invalid vector size in the deserialisation of an integer via a ") + name
                + ": the number of limbs stored in the " + name + " (" + std::to_string(lsize / sizeof(::mp_limb_t))
                + ") is less than the integer size in limbs stored in the header of the vector ("
                + std::to_string(asize) + ")");
        }
        binary_load_impl(src.data() + sizeof(detail::mpz_size_t), size, asize);
        return read_bytes(asize);
    }

public:
    // Load a value from a vector.
    std::size_t binary_load(const std::vector<char> &src)
    {
        return binary_load_vector(src, "std::vector");
    }
    // Load a value from an array.
    template <std::size_t S>
    std::size_t binary_load(const std::array<char, S> &src)
    {
        return binary_load_vector(src, "std::array");
    }
    // Load a value from a stream.
    std::size_t binary_load(std::istream &src)
    {
        // Let's start by reading size/asize.
        // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
        detail::mpz_size_t size;
        src.read(reinterpret_cast<char *>(&size), detail::safe_cast<std::streamsize>(sizeof(detail::mpz_size_t)));
        if (!src.good()) {
            // Something went wrong with reading, return 0.
            return 0;
        }
        // Determine asize.
        const auto asize = size >= 0 ? detail::make_unsigned(size) : detail::nint_abs(size);
        // Check for overflow.
        // LCOV_EXCL_START
        if (mppp_unlikely(asize > std::numeric_limits<std::size_t>::max() / sizeof(::mp_limb_t))) {
            throw std::overflow_error("Overflow in the computation of the size in bytes of an integer being "
                                      "deserialised via the stream interface");
        }
        // LCOV_EXCL_STOP
        // Size in bytes of the limbs array.
        const auto lsize = static_cast<std::size_t>(sizeof(::mp_limb_t) * asize);
        // Now let's read from the stream into a local buffer.
        // NOTE: of course here we could avoid the local buffer with a specific implementation
        // of binary_load_impl() adapted for reading from a stream. For the first version, let's
        // keep things simple - we can always improve the performance at a later stage
        // if needed at all.
        MPPP_MAYBE_TLS std::vector<char> buffer;
        if (lsize > buffer.size()) {
            // Enlarge the local buffer if needed.
            buffer.resize(detail::safe_cast<decltype(buffer.size())>(lsize));
        }
        src.read(buffer.data(), detail::safe_cast<std::streamsize>(lsize));
        if (!src.good()) {
            // Something went wrong with reading, return 0.
            return 0;
        }
        // Everything ok with the deserialisation of the integer in the local buffer.
        // Invoke the low-level deser routine.
        binary_load_impl(buffer.data(), size, asize);
        // NOTE: just recompute the binary size from scratch: the function we use elsewhere,
        // read_bytes(), assumes that all data is coming from a contiguous buffer in order to
        // avoid an overflow check, but here we don't have this guarantee.
        return binary_size();
    }

private:
    detail::integer_union<SSize> m_int;
};

#if MPPP_CPLUSPLUS < 201703L

// NOTE: from C++17 static constexpr members are implicitly inline, and it's not necessary
// any more (actually, it's deprecated) to re-declare them outside the class.
// https://stackoverflow.com/questions/39646958/constexpr-static-member-before-after-c17

template <std::size_t SSize>
constexpr std::size_t integer<SSize>::ssize;

#endif

template <std::size_t SSize>
const char integer<SSize>::binary_size_errmsg[] = "Overflow in the computation of the binary size of an integer";

template <std::size_t SSize>
const char integer<SSize>::bl_data_errmsg[]
    = "Invalid data detected in the binary deserialisation of an integer: the most "
      "significant limb of the value cannot be zero";

namespace detail
{

// Swap u1 and u2. u1 must be static, u2 must be dynamic.
template <std::size_t SSize>
inline void integer_swap_static_dynamic(integer_union<SSize> &u1, integer_union<SSize> &u2) noexcept
{
    using s_storage = typename integer_union<SSize>::s_storage;
    using d_storage = typename integer_union<SSize>::d_storage;

    assert(u1.is_static());
    assert(!u2.is_static());

    // Copy the static in temp storage.
    const auto n1_copy(u1.g_st());
    // Destroy the static.
    u1.g_st().~s_storage();
    // Construct the dynamic struct, shallow-copying from n2.
    ::new (static_cast<void *>(&u1.m_dy)) d_storage(u2.g_dy());
    // Re-create n2 as a static copying from n1_copy.
    u2.g_dy().~d_storage();
    ::new (static_cast<void *>(&u2.m_st)) s_storage(n1_copy);
}

} // namespace detail

// Swap.
template <std::size_t SSize>
inline void swap(integer<SSize> &n1, integer<SSize> &n2) noexcept
{
    auto &u1 = n1._get_union();
    auto &u2 = n2._get_union();

    const bool s1 = u1.is_static(), s2 = u2.is_static();
    if (s1 && s2) {
        // Self swap is fine, handled in the static.
        u1.g_st().swap(u2.g_st());
    } else if (s1 && !s2) {
        detail::integer_swap_static_dynamic(u1, u2);
    } else if (!s1 && s2) {
        // Mirror of the above.
        detail::integer_swap_static_dynamic(u2, u1);
    } else {
        // Swap with other. Self swap is fine, mpz_swap() can have
        // aliasing arguments.
        mpz_swap(&u1.g_dy(), &u2.g_dy());
    }
}

// Set to zero.
template <std::size_t SSize>
inline integer<SSize> &set_zero(integer<SSize> &n)
{
    return n.set_zero();
}

// Set to one.
template <std::size_t SSize>
inline integer<SSize> &set_one(integer<SSize> &n)
{
    return n.set_one();
}

// Set to minus one.
template <std::size_t SSize>
inline integer<SSize> &set_negative_one(integer<SSize> &n)
{
    return n.set_negative_one();
}

// Generic conversion function to C++ fundamental types.
#if defined(MPPP_HAVE_CONCEPTS)
template <integer_cpp_arithmetic T, std::size_t SSize>
#else
template <typename T, std::size_t SSize, detail::enable_if_t<is_integer_cpp_arithmetic<T>::value, int> = 0>
#endif
inline bool get(T &rop, const integer<SSize> &n)
{
    return n.get(rop);
}

// Generic conversion function to C++ complex types.
#if defined(MPPP_HAVE_CONCEPTS)
template <integer_cpp_complex T, std::size_t SSize>
#else
template <typename T, std::size_t SSize, detail::enable_if_t<is_integer_cpp_complex<T>::value, int> = 0>
#endif
inline bool get(T &rop, const integer<SSize> &n)
{
    return n.get(rop);
}

namespace detail
{

// Machinery for the determination of the result of a binary operation involving integer.
// Default is empty for SFINAE.
template <typename, typename, typename = void>
struct integer_common_type {
};

template <std::size_t SSize>
struct integer_common_type<integer<SSize>, integer<SSize>> {
    using type = integer<SSize>;
};

template <std::size_t SSize, typename U>
struct integer_common_type<integer<SSize>, U, enable_if_t<is_cpp_integral<U>::value>> {
    using type = integer<SSize>;
};

template <std::size_t SSize, typename T>
struct integer_common_type<T, integer<SSize>, enable_if_t<is_cpp_integral<T>::value>> {
    using type = integer<SSize>;
};

template <std::size_t SSize, typename U>
struct integer_common_type<
    integer<SSize>, U, enable_if_t<disjunction<is_integer_cpp_floating_point<U>, is_integer_cpp_complex<U>>::value>> {
    using type = U;
};

template <std::size_t SSize, typename T>
struct integer_common_type<
    T, integer<SSize>, enable_if_t<disjunction<is_integer_cpp_floating_point<T>, is_integer_cpp_complex<T>>::value>> {
    using type = T;
};

template <typename T, typename U>
using integer_common_t = typename integer_common_type<T, U>::type;

// Various utilities used in both the operators and the functions.
template <typename T, typename U>
struct is_same_ssize_integer : std::false_type {
};

template <std::size_t SSize>
struct is_same_ssize_integer<integer<SSize>, integer<SSize>> : std::true_type {
};

template <typename T>
struct is_integer : std::false_type {
};

template <std::size_t SSize>
struct is_integer<integer<SSize>> : std::true_type {
};

} // namespace detail

template <typename T, typename U>
using are_integer_op_types = detail::is_detected<detail::integer_common_t, T, U>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL integer_op_types = are_integer_op_types<T, U>::value;

#endif

template <typename T, typename U>
using are_integer_real_op_types
    = detail::conjunction<are_integer_op_types<T, U>, detail::negation<is_integer_cpp_complex<T>>,
                          detail::negation<is_integer_cpp_complex<U>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL integer_real_op_types = are_integer_real_op_types<T, U>::value;

#endif

template <typename T, typename U>
using are_integer_integral_op_types
    = detail::disjunction<detail::is_same_ssize_integer<T, U>,
                          detail::conjunction<detail::is_integer<T>, is_cpp_integral<U>>,
                          detail::conjunction<detail::is_integer<U>, is_cpp_integral<T>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL integer_integral_op_types = are_integer_integral_op_types<T, U>::value;

#endif

namespace detail
{

// Metaprogramming for selecting the algorithm for static addition. The selection happens via
// an std::integral_constant with 3 possible values:
// - 0 (default case): use the GMP mpn functions,
// - 1: selected when there are no nail bits and the static size is 1,
// - 2: selected when there are no nail bits and the static size is 2.
template <typename SInt>
using integer_static_add_algo = std::integral_constant<
    int, (!GMP_NAIL_BITS && SInt::s_size == 1) ? 1 : ((!GMP_NAIL_BITS && SInt::s_size == 2) ? 2 : 0)>;

// General implementation via mpn.
// Small helper to compute the size after subtraction via mpn. s is a strictly positive size.
inline mpz_size_t integer_sub_compute_size(const ::mp_limb_t *rdata, mpz_size_t s)
{
    assert(s > 0);
    mpz_size_t cur_idx = s - 1;
    for (; cur_idx >= 0; --cur_idx) {
        if ((rdata[cur_idx] & GMP_NUMB_MASK) != 0u) {
            break;
        }
    }
    return cur_idx + 1;
}

// NOTE: this function (and its other overloads) will return true in case of success, false in case of failure
// (i.e., the addition overflows and the static size is not enough).
template <std::size_t SSize>
inline bool static_add_impl(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2,
                            mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2,
                            const std::integral_constant<int, 0> &)
{
    auto rdata = rop.m_limbs.data();
    auto data1 = op1.m_limbs.data(), data2 = op2.m_limbs.data();
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
    // copy it out, but that is rather costly. Note that this means that in principle a computation that could fit in
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
            // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
            ::mp_limb_t cy;
            if (asize2 == 1) {
                // NOTE: we are not masking data2[0] with GMP_NUMB_MASK, I am assuming the mpn function
                // is able to deal with a limb with a nail.
                cy = mpn_add_1(rdata, data1, static_cast<::mp_size_t>(asize1), data2[0]);
            } else if (asize1 == asize2) {
                cy = mpn_add_n(rdata, data1, data2, static_cast<::mp_size_t>(asize1));
            } else {
                cy = mpn_add(rdata, data1, static_cast<::mp_size_t>(asize1), data2, static_cast<::mp_size_t>(asize2));
            }
            if (cy) {
                assert(asize1 < static_int<SSize>::s_size);
                rop._mp_size = size1 + sign1;
                // NOTE: there should be no need to use GMP_NUMB_MASK here.
                rdata[asize1] = 1u;
            } else {
                // Without carry, the size is unchanged.
                rop._mp_size = size1;
            }
        } else {
            // The number of limbs of op2 > op1.
            // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
            ::mp_limb_t cy;
            if (asize1 == 1) {
                cy = mpn_add_1(rdata, data2, static_cast<::mp_size_t>(asize2), data1[0]);
            } else {
                cy = mpn_add(rdata, data2, static_cast<::mp_size_t>(asize2), data1, static_cast<::mp_size_t>(asize1));
            }
            if (cy) {
                assert(asize2 < static_int<SSize>::s_size);
                rop._mp_size = size2 + sign2;
                rdata[asize2] = 1u;
            } else {
                rop._mp_size = size2;
            }
        }
    } else {
        if (asize1 > asize2 || (asize1 == asize2 && mpn_cmp(data1, data2, static_cast<::mp_size_t>(asize1)) >= 0)) {
            // abs(op1) >= abs(op2).
            // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
            ::mp_limb_t br;
            if (asize2 == 1) {
                br = mpn_sub_1(rdata, data1, static_cast<::mp_size_t>(asize1), data2[0]);
            } else if (asize1 == asize2) {
                br = mpn_sub_n(rdata, data1, data2, static_cast<::mp_size_t>(asize1));
            } else {
                br = mpn_sub(rdata, data1, static_cast<::mp_size_t>(asize1), data2, static_cast<::mp_size_t>(asize2));
            }
            assert(!br);
            rop._mp_size = integer_sub_compute_size(rdata, asize1);
            if (sign1 != 1) {
                rop._mp_size = -rop._mp_size;
            }
        } else {
            // abs(op2) > abs(op1).
            // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
            ::mp_limb_t br;
            if (asize1 == 1) {
                br = mpn_sub_1(rdata, data2, static_cast<::mp_size_t>(asize2), data1[0]);
            } else {
                br = mpn_sub(rdata, data2, static_cast<::mp_size_t>(asize2), data1, static_cast<::mp_size_t>(asize1));
            }
            assert(!br);
            rop._mp_size = integer_sub_compute_size(rdata, asize2);
            if (sign2 != 1) {
                rop._mp_size = -rop._mp_size;
            }
        }
    }
    return true;
}

// Optimization for single-limb statics with no nails.
template <std::size_t SSize>
inline bool static_add_impl(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2,
                            mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2,
                            const std::integral_constant<int, 1> &)
{
    ignore(asize1, asize2);
    auto rdata = rop.m_limbs.data();
    auto data1 = op1.m_limbs.data(), data2 = op2.m_limbs.data();
    // NOTE: both asizes have to be 0 or 1 here.
    assert((asize1 == 1 && data1[0] != 0u) || (asize1 == 0 && data1[0] == 0u));
    assert((asize2 == 1 && data2[0] != 0u) || (asize2 == 0 && data2[0] == 0u));
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
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
            rop._mp_size = sign1 * (data1[0] != data2[0]);
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
inline int integer_compare_limbs_2(const ::mp_limb_t *data1, const ::mp_limb_t *data2, mpz_size_t asize)
{
    // NOTE: this requires no nail bits.
    // NOLINTNEXTLINE(cert-dcl03-c, hicpp-static-assert, misc-static-assert)
    assert(!GMP_NAIL_BITS);
    assert(asize == 1 || asize == 2);
    // Start comparing from the top.
    auto cmp_idx = asize - 1;
    if (data1[cmp_idx] != data2[cmp_idx]) {
        return data1[cmp_idx] > data2[cmp_idx] ? 1 : -1;
    }
    // The top limbs are equal, move down one limb.
    // If we are already at the bottom limb, it means the two numbers are equal.
    if (cmp_idx == 0) {
        return 0;
    }
    --cmp_idx;
    if (data1[cmp_idx] != data2[cmp_idx]) {
        return data1[cmp_idx] > data2[cmp_idx] ? 1 : -1;
    }
    return 0;
}

template <std::size_t SSize>
inline bool static_add_impl(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2,
                            mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2,
                            const std::integral_constant<int, 2> &)
{
    auto rdata = rop.m_limbs.data();
    auto data1 = op1.m_limbs.data(), data2 = op2.m_limbs.data();
    if (sign1 == sign2) {
        // NOTE: this handles the case in which the numbers have the same sign, including 0 + 0.
        //
        // NOTE: this is the implementation for 2 limbs, even if potentially the operands have 1 limb.
        // The idea here is that it's better to do a few computations more rather than paying the branching
        // cost. 1-limb operands will have the upper limb set to zero from the zero-initialisation of
        // the limbs of static ints.
        //
        // NOTE: the rop hi limb might spill over either from the addition of the hi limbs
        // of op1 and op2, or by the addition of carry coming over from the addition of
        // the lo limbs of op1 and op2.
        //
        // Add the hi and lo limbs.
        const auto a = data1[0], b = data2[0], c = data1[1], d = data2[1];
        // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
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
        rop._mp_size = sign1 * ((hi2 != 0u) + 1);
        rdata[0] = lo;
        rdata[1] = hi2;
    } else {
        // When the signs differ, we need to implement addition as a subtraction.
        // NOTE: this also includes the case in which only one of the operands is zero.
        if (asize1 > asize2 || (asize1 == asize2 && integer_compare_limbs_2(data1, data2, asize1) >= 0)) {
            // op1 is >= op2 in absolute value.
            const auto lo = data1[0] - data2[0];
            // If there's a borrow, then hi1 > hi2, otherwise we would have a negative result.
            assert(data1[0] >= data2[0] || data1[1] > data2[1]);
            // This can never wrap around, at most it goes to zero.
            const auto hi = data1[1] - data2[1] - static_cast<::mp_limb_t>(data1[0] < data2[0]);
            // asize can be 0, 1 or 2. The sign1 (which cannot be zero due to the branch we are in)
            // takes care of the sign of the size.
            rop._mp_size = sign1 * size_from_lohi(lo, hi);
            rdata[0] = lo;
            rdata[1] = hi;
        } else {
            // op2 is > op1 in absolute value.
            const auto lo = data2[0] - data1[0];
            assert(data2[0] >= data1[0] || data2[1] > data1[1]);
            const auto hi = data2[1] - data1[1] - static_cast<::mp_limb_t>(data2[0] < data1[0]);
            // asize can be 1 or 2, but not zero as we know abs(op1) != abs(op2). Same idea as above.
            rop._mp_size = sign2 * (static_cast<int>(hi != 0u) + 1);
            rdata[0] = lo;
            rdata[1] = hi;
        }
    }
    return true;
}

template <bool AddOrSub, std::size_t SSize>
inline bool static_addsub(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2)
{
    const mpz_size_t asize1 = std::abs(op1._mp_size), asize2 = std::abs(op2._mp_size);
    const int sign1 = integral_sign(op1._mp_size),
              // NOTE: effectively negate op2 if we are subtracting.
        sign2 = AddOrSub ? integral_sign(op2._mp_size) : -integral_sign(op2._mp_size);
    const bool retval
        = static_add_impl(rop, op1, op2, asize1, asize2, sign1, sign2, integer_static_add_algo<static_int<SSize>>{});
    if (integer_static_add_algo<static_int<SSize>>::value == 0 && retval) {
        // If we used the mpn functions and we actually wrote into rop, then
        // make sure we zero out the unused limbs.
        // NOTE: this is necessary because, in theory, we may end up using mpn functions also for SSize <= opt_size.
        // This is not the case on any tested build so far.
        rop.zero_unused_limbs();
    }
    return retval;
}
} // namespace detail

// Ternary addition.
template <std::size_t SSize>
inline integer<SSize> &add(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool s1 = op1.is_static(), s2 = op2.is_static();
    bool sr = rop.is_static();
    if (mppp_likely(s1 && s2)) {
        // If both op1 and op2 are static, we will try to do the static add.
        // We might need to downgrade rop to static.
        if (!sr) {
            // NOTE: here we are sure rop is distinct from op1/op2, as
            // rop is dynamic and op1/op2 are both static.
            rop.set_zero();
            sr = true;
        }
        if (mppp_likely(detail::static_addsub<true>(rop._get_union().g_st(), op1._get_union().g_st(),
                                                    op2._get_union().g_st()))) {
            return rop;
        }
    }
    if (sr) {
        rop._get_union().promote(SSize + 1u);
    }
    mpz_add(&rop._get_union().g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    return rop;
}

namespace detail
{

// Metaprogramming for selecting the algorithm for static add/sub with a single limb. The selection happens via
// an std::integral_constant with 3 possible values:
// - 0 (default case): use the GMP mpn functions,
// - 1: selected when there are no nail bits and the static size is 1,
// - 2: selected when there are no nail bits and the static size is 2.
template <typename SInt>
using integer_static_addsub_1_algo = std::integral_constant<
    int, (!GMP_NAIL_BITS && SInt::s_size == 1) ? 1 : ((!GMP_NAIL_BITS && SInt::s_size == 2) ? 2 : 0)>;

// mpn implementation.
template <bool AddOrSub, std::size_t SSize>
inline bool static_addsub_1_impl(static_int<SSize> &rop, const static_int<SSize> &op1, mpz_size_t asize1, int sign1,
                                 ::mp_limb_t l2, const std::integral_constant<int, 0> &)
{
    auto rdata = rop.m_limbs.data();
    auto data1 = op1.m_limbs.data();
    const auto size1 = op1._mp_size;
    // mpn functions require nonzero arguments.
    if (mppp_unlikely(!l2)) {
        rop._mp_size = size1;
        copy_limbs(data1, data1 + asize1, rdata);
        return true;
    }
    // We now know l2 is not zero, so its "sign" must be either 1 or -1.
    constexpr int sign2 = AddOrSub ? 1 : -1;
    if (mppp_unlikely(!sign1)) {
        // NOTE: this has to be +-1 because l2 == 0 is handled above.
        rop._mp_size = sign2;
        rdata[0] = l2;
        return true;
    }
    // This is the same overflow check as in static_addsub(). As in static_addsub(), this is not as tight/precise
    // as it could be.
    const bool c1 = std::size_t(asize1) == SSize && ((data1[asize1 - 1] & GMP_NUMB_MASK) >> (GMP_NUMB_BITS - 1));
    const bool c2 = SSize == 1u && (l2 >> (GMP_NUMB_BITS - 1));
    if (mppp_unlikely(c1 || c2)) {
        return false;
    }
    if (sign1 == sign2) {
        // op1 and op2 have the same sign. Implement as a true addition.
        if (mpn_add_1(rdata, data1, static_cast<::mp_size_t>(asize1), l2)) {
            assert(asize1 < static_int<SSize>::s_size);
            // New size is old size + 1 if old size was positive, old size - 1 otherwise.
            rop._mp_size = size1 + sign2;
            // NOTE: there should be no need to use GMP_NUMB_MASK here.
            rdata[asize1] = 1u;
        } else {
            // Without carry, the size is unchanged.
            rop._mp_size = size1;
        }
    } else {
        // op1 and op2 have different signs. Implement as a subtraction.
        if (asize1 > 1 || (asize1 == 1 && (data1[0] & GMP_NUMB_MASK) >= l2)) {
            // abs(op1) >= abs(op2).
            const auto br = mpn_sub_1(rdata, data1, static_cast<::mp_size_t>(asize1), l2);
            ignore(br);
            assert(!br);
            // The asize can be the original one or original - 1 (we subtracted a limb). If size1 was positive,
            // sign2 has to be negative and we potentially subtract 1, if size1 was negative then sign2 has to be
            // positive and we potentially add 1.
            rop._mp_size = size1 + sign2 * !(rdata[asize1 - 1] & GMP_NUMB_MASK);
        } else {
            // abs(op2) > abs(op1).
            const auto br = mpn_sub_1(rdata, &l2, 1, data1[0]);
            ignore(br);
            assert(!br);
            // The size must be +-1, as abs(op2) == abs(op1) is handled above.
            assert((rdata[0] & GMP_NUMB_MASK));
            rop._mp_size = sign2;
        }
    }
    return true;
}

// 1-limb optimisation (no nails).
template <bool AddOrSub, std::size_t SSize>
inline bool static_addsub_1_impl(static_int<SSize> &rop, const static_int<SSize> &op1, mpz_size_t, int sign1,
                                 ::mp_limb_t l2, const std::integral_constant<int, 1> &)
{
    const auto l1 = op1.m_limbs[0];
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mp_limb_t tmp;
    if ((sign1 >= 0 && AddOrSub) || (sign1 <= 0 && !AddOrSub)) {
        // op1 non-negative and addition, or op1 non-positive and subtraction. Implement
        // as a true addition.
        if (mppp_unlikely(limb_add_overflow(l1, l2, &tmp))) {
            return false;
        }
        // The size is 1 for addition, -1 for subtraction, unless
        // the result is zero.
        rop._mp_size = (AddOrSub ? 1 : -1) * (tmp != 0u);
        rop.m_limbs[0] = tmp;
    } else {
        // op1 negative and addition, or op1 positive and subtraction. Implement
        // as a subtraction.
        if (l1 >= l2) {
            // op1 has larger or equal abs.
            tmp = l1 - l2;
            // asize is 1 or 0, 0 iff l1 == l2. Sign is negative for add, positive for sub.
            rop._mp_size = (AddOrSub ? -1 : 1) * (tmp != 0u);
            rop.m_limbs[0] = tmp;
        } else {
            // op1 has smaller abs. The result will be positive for add, negative
            // for sub (cannot be zero as it is handled above).
            rop._mp_size = AddOrSub ? 1 : -1;
            rop.m_limbs[0] = l2 - l1;
        }
    }
    return true;
}

// 2-limb optimisation (no nails).
template <bool AddOrSub, std::size_t SSize>
inline bool static_addsub_1_impl(static_int<SSize> &rop, const static_int<SSize> &op1, mpz_size_t asize1, int sign1,
                                 ::mp_limb_t l2, const std::integral_constant<int, 2> &)
{
    auto rdata = rop.m_limbs.data();
    auto data1 = op1.m_limbs.data();
    if ((sign1 >= 0 && AddOrSub) || (sign1 <= 0 && !AddOrSub)) {
        // op1 non-negative and addition, or op1 non-positive and subtraction. Implement
        // as a true addition.
        // These two limbs will contain the result.
        // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
        ::mp_limb_t lo, hi;
        // Add l2 to the low limb, placing the result in lo.
        const ::mp_limb_t cy_lo = limb_add_overflow(data1[0], l2, &lo);
        // Add the carry from the low addition to the high limb, placing the result in hi.
        const ::mp_limb_t cy_hi = limb_add_overflow(data1[1], cy_lo, &hi);
        if (mppp_unlikely(cy_hi)) {
            return false;
        }
        // Compute the new asize. It can be 0, 1 or 2.
        rop._mp_size = (AddOrSub ? 1 : -1) * size_from_lohi(lo, hi);
        // Write out.
        rdata[0] = lo;
        rdata[1] = hi;
    } else {
        // op1 negative and addition, or op1 positive and subtraction. Implement
        // as a subtraction.
        // Compare their absolute values.
        if (asize1 == 2 || data1[0] >= l2) {
            // op1 is >= op2 in absolute value.
            const auto lo = data1[0] - l2;
            // Sub from hi the borrow.
            const auto hi = data1[1] - static_cast<::mp_limb_t>(data1[0] < l2);
            // The final asize can be 2, 1 or 0. Sign is negative for add, positive for sub.
            rop._mp_size = (AddOrSub ? -1 : 1) * size_from_lohi(lo, hi);
            rdata[0] = lo;
            rdata[1] = hi;
        } else {
            // op2 > op1 in absolute value.
            // Size has to be +-1.
            rop._mp_size = AddOrSub ? 1 : -1;
            rdata[0] = l2 - data1[0];
            rdata[1] = 0u;
        }
    }
    return true;
}

template <bool AddOrSub, std::size_t SSize>
inline bool static_addsub_1(static_int<SSize> &rop, const static_int<SSize> &op1, ::mp_limb_t op2)
{
    const mpz_size_t asize1 = std::abs(op1._mp_size);
    const int sign1 = integral_sign(op1._mp_size);
    const bool retval = static_addsub_1_impl<AddOrSub>(rop, op1, asize1, sign1, op2,
                                                       integer_static_addsub_1_algo<static_int<SSize>>{});
    if (integer_static_addsub_1_algo<static_int<SSize>>::value == 0 && retval) {
        // If we used the mpn functions and we actually wrote into rop, then
        // make sure we zero out the unused limbs.
        // NOTE: same as above, we may have used mpn function when SSize <= opt_size.
        rop.zero_unused_limbs();
    }
    return retval;
}

// Implementation of add_ui().
template <std::size_t SSize, typename T>
inline integer<SSize> &add_ui_impl(integer<SSize> &rop, const integer<SSize> &op1, const T &op2)
{
    if (op2 > GMP_NUMB_MAX) {
        // For the optimised version below to kick in we need to be sure we can safely convert
        // op2 to an ::mp_limb_t, modulo nail bits. Otherwise, we just call add() after converting
        // op2 to an integer.
        MPPP_MAYBE_TLS integer<SSize> tmp;
        tmp = op2;
        return add(rop, op1, tmp);
    }
    const bool s1 = op1.is_static();
    bool sr = rop.is_static();
    if (mppp_likely(s1)) {
        if (!sr) {
            rop.set_zero();
            sr = true;
        }
        if (mppp_likely(static_addsub_1<true>(rop._get_union().g_st(), op1._get_union().g_st(),
                                              static_cast<::mp_limb_t>(op2)))) {
            return rop;
        }
    }
    if (sr) {
        rop._get_union().promote(SSize + 1u);
    }
    // NOTE: at this point we know that:
    // - op2 fits in a limb (accounting for nail bits as well),
    // - op2 may overflow unsigned long, which is the unsigned integral data type
    //   the GMP API expects.
    if (op2 <= std::numeric_limits<unsigned long>::max()) {
        // op2 actually fits in unsigned long, let's just invoke the mpz_add_ui() function directly.
        mpz_add_ui(&rop._get_union().g_dy(), op1.get_mpz_view(), static_cast<unsigned long>(op2));
    } else {
        // LCOV_EXCL_START
        // op2 overflows unsigned long, but still fits in a limb. We will create a fake mpz struct
        // with read-only access for use in the mpz_add() function.
        // NOTE: this branch is possible at the moment only on Windows 64 bit, where unsigned long
        // is 32bit and the mp_limb_t is 64bit. op2 could then be an unsigned long long (64bit) which
        // still fits in an ::mp_limb_t.
        ::mp_limb_t op2_copy[1] = {static_cast<::mp_limb_t>(op2)};
        // NOTE: we have 1 allocated limb, with address &op2_copy. The size has to be 1,
        // as op2 is unsigned, fits in an mp_limbt_t and it is not zero (otherwise we would've taken
        // the other branch).
        const detail::mpz_struct_t tmp_mpz{1, 1, op2_copy};
        mpz_add(&rop._get_union().g_dy(), op1.get_mpz_view(), &tmp_mpz);
        // LCOV_EXCL_STOP
    }
    return rop;
}

// NOTE: special-case bool in order to avoid spurious compiler warnings when
// mixing up bool and other integral types.
template <std::size_t SSize>
inline integer<SSize> &add_ui_impl(integer<SSize> &rop, const integer<SSize> &op1, bool op2)
{
    return add_ui_impl(rop, op1, static_cast<unsigned>(op2));
}

} // namespace detail

// Ternary addition with C++ unsigned integral types.
#if defined(MPPP_HAVE_CONCEPTS)
template <std::size_t SSize, cpp_unsigned_integral T>
#else
template <std::size_t SSize, typename T, detail::enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
#endif
inline integer<SSize> &add_ui(integer<SSize> &rop, const integer<SSize> &op1, const T &op2)
{
    return detail::add_ui_impl(rop, op1, op2);
}

// Ternary addition with C++ signed integral types.
#if defined(MPPP_HAVE_CONCEPTS)
template <std::size_t SSize, cpp_signed_integral T>
#else
template <std::size_t SSize, typename T, detail::enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
#endif
inline integer<SSize> &add_si(integer<SSize> &rop, const integer<SSize> &op1, const T &op2)
{
    if (op2 >= detail::uncvref_t<decltype(op2)>(0)) {
        return add_ui(rop, op1, detail::make_unsigned(op2));
    }
    return sub_ui(rop, op1, detail::nint_abs(op2));
}

// Ternary subtraction.
template <std::size_t SSize>
inline integer<SSize> &sub(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool s1 = op1.is_static(), s2 = op2.is_static();
    bool sr = rop.is_static();
    if (mppp_likely(s1 && s2)) {
        if (!sr) {
            rop.set_zero();
            sr = true;
        }
        if (mppp_likely(detail::static_addsub<false>(rop._get_union().g_st(), op1._get_union().g_st(),
                                                     op2._get_union().g_st()))) {
            return rop;
        }
    }
    if (sr) {
        rop._get_union().promote(SSize + 1u);
    }
    mpz_sub(&rop._get_union().g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    return rop;
}

namespace detail
{

// Implementation of sub_ui().
template <std::size_t SSize, typename T>
inline integer<SSize> &sub_ui_impl(integer<SSize> &rop, const integer<SSize> &op1, const T &op2)
{
    if (op2 > GMP_NUMB_MASK) {
        MPPP_MAYBE_TLS integer<SSize> tmp;
        tmp = op2;
        return sub(rop, op1, tmp);
    }
    const bool s1 = op1.is_static();
    bool sr = rop.is_static();
    if (mppp_likely(s1)) {
        if (!sr) {
            rop.set_zero();
            sr = true;
        }
        if (mppp_likely(static_addsub_1<false>(rop._get_union().g_st(), op1._get_union().g_st(),
                                               static_cast<::mp_limb_t>(op2)))) {
            return rop;
        }
    }
    if (sr) {
        rop._get_union().promote(SSize + 1u);
    }
    if (op2 <= std::numeric_limits<unsigned long>::max()) {
        mpz_sub_ui(&rop._get_union().g_dy(), op1.get_mpz_view(), static_cast<unsigned long>(op2));
    } else {
        // LCOV_EXCL_START
        ::mp_limb_t op2_copy[1] = {static_cast<::mp_limb_t>(op2)};
        const detail::mpz_struct_t tmp_mpz{1, 1, op2_copy};
        mpz_sub(&rop._get_union().g_dy(), op1.get_mpz_view(), &tmp_mpz);
        // LCOV_EXCL_STOP
    }
    return rop;
}

// NOTE: special-case bool in order to avoid spurious compiler warnings when
// mixing up bool and other integral types.
template <std::size_t SSize>
inline integer<SSize> &sub_ui_impl(integer<SSize> &rop, const integer<SSize> &op1, bool op2)
{
    return sub_ui_impl(rop, op1, static_cast<unsigned>(op2));
}

} // namespace detail

// Ternary subtraction with C++ unsigned integral types.
#if defined(MPPP_HAVE_CONCEPTS)
template <std::size_t SSize, cpp_unsigned_integral T>
#else
template <std::size_t SSize, typename T, detail::enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
#endif
inline integer<SSize> &sub_ui(integer<SSize> &rop, const integer<SSize> &op1, const T &op2)
{
    return detail::sub_ui_impl(rop, op1, op2);
}

// Ternary subtraction with C++ signed integral types.
#if defined(MPPP_HAVE_CONCEPTS)
template <std::size_t SSize, cpp_signed_integral T>
#else
template <std::size_t SSize, typename T, detail::enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
#endif
inline integer<SSize> &sub_si(integer<SSize> &rop, const integer<SSize> &op1, const T &op2)
{
    if (op2 >= detail::uncvref_t<decltype(op2)>(0)) {
        return sub_ui(rop, op1, detail::make_unsigned(op2));
    }
    return add_ui(rop, op1, detail::nint_abs(op2));
}

namespace detail
{

// The double limb multiplication optimization is available in the following cases:
// - no nails, we are on a 64bit MSVC build, the limb type has exactly 64 bits and GMP_NUMB_BITS is 64,
// - no nails, we have a 128bit unsigned available, the limb type has exactly 64 bits and GMP_NUMB_BITS is 64,
// - no nails, the smallest 64 bit unsigned type has exactly 64 bits, the limb type has exactly 32 bits and
//   GMP_NUMB_BITS is 32.
// NOTE: here we are checking that GMP_NUMB_BITS is the same as the limits ::digits property, which is probably
// rather redundant on any conceivable architecture.
using integer_have_dlimb_mul = std::integral_constant<bool,
#if (defined(_MSC_VER) && defined(_WIN64) && (GMP_NUMB_BITS == 64))                                                    \
    || (defined(MPPP_HAVE_GCC_INT128) && (GMP_NUMB_BITS == 64))
                                                      !GMP_NAIL_BITS && nl_digits<::mp_limb_t>() == 64
#elif GMP_NUMB_BITS == 32
                                                      !GMP_NAIL_BITS && nl_digits<std::uint_least64_t>() == 64
                                                          && nl_digits<::mp_limb_t>() == 32
#else
                                                      false
#endif
                                                      >;

template <typename SInt>
using integer_static_mul_algo
    = std::integral_constant<int, (SInt::s_size == 1 && integer_have_dlimb_mul::value)
                                      ? 1
                                      : ((SInt::s_size == 2 && integer_have_dlimb_mul::value) ? 2 : 0)>;

// mpn implementation.
// NOTE: this function (and the other overloads) returns 0 in case of success, otherwise it returns a hint
// about the size in limbs of the result.
template <std::size_t SSize>
inline std::size_t static_mul_impl(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2,
                                   mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2,
                                   const std::integral_constant<int, 0> &)
{
    // Handle zeroes.
    if (mppp_unlikely(!sign1 || !sign2)) {
        rop._mp_size = 0;
        return 0u;
    }
    auto rdata = rop.m_limbs.data();
    auto data1 = op1.m_limbs.data(), data2 = op2.m_limbs.data();
    // NOLINTNEXTLINE(bugprone-misplaced-widening-cast)
    const auto max_asize = static_cast<std::size_t>(asize1 + asize2);
    // Temporary storage, to be used if we cannot write into rop.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, SSize * 2u> res;
    // We can write directly into rop if these conditions hold:
    // - rop does not overlap with op1 and op2,
    // - SSize is large enough to hold the max size of the result.
    ::mp_limb_t *MPPP_RESTRICT res_data = (rdata != data1 && rdata != data2 && max_asize <= SSize) ? rdata : res.data();
    // Proceed to the multiplication.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mp_limb_t hi;
    if (asize2 == 1) {
        // NOTE: the 1-limb versions do not write the hi limb, we have to write it ourselves.
        hi = mpn_mul_1(res_data, data1, static_cast<::mp_size_t>(asize1), data2[0]);
        res_data[asize1] = hi;
    } else if (asize1 == 1) {
        hi = mpn_mul_1(res_data, data2, static_cast<::mp_size_t>(asize2), data1[0]);
        res_data[asize2] = hi;
    } else if (asize1 == asize2) {
        mpn_mul_n(res_data, data1, data2, static_cast<::mp_size_t>(asize1));
        hi = res_data[2 * asize1 - 1];
    } else if (asize1 >= asize2) {
        hi = mpn_mul(res_data, data1, static_cast<::mp_size_t>(asize1), data2, static_cast<::mp_size_t>(asize2));
    } else {
        hi = mpn_mul(res_data, data2, static_cast<::mp_size_t>(asize2), data1, static_cast<::mp_size_t>(asize1));
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
    // NOTE: here we are sure that res_data != rdata, as we checked it earlier.
    copy_limbs_no(res_data, res_data + asize, rdata);
    return 0u;
}

#if defined(_MSC_VER) && defined(_WIN64) && (GMP_NUMB_BITS == 64) && !GMP_NAIL_BITS

inline ::mp_limb_t dlimb_mul(::mp_limb_t op1, ::mp_limb_t op2, ::mp_limb_t *hi)
{
    return ::UnsignedMultiply128(op1, op2, hi);
}

#elif defined(MPPP_HAVE_GCC_INT128) && (GMP_NUMB_BITS == 64) && !GMP_NAIL_BITS

inline ::mp_limb_t dlimb_mul(::mp_limb_t op1, ::mp_limb_t op2, ::mp_limb_t *hi)
{
    using dlimb_t = __uint128_t;
    const dlimb_t res = dlimb_t(op1) * op2;
    *hi = static_cast<::mp_limb_t>(res >> 64);
    return static_cast<::mp_limb_t>(res);
}

#elif GMP_NUMB_BITS == 32 && !GMP_NAIL_BITS

inline ::mp_limb_t dlimb_mul(::mp_limb_t op1, ::mp_limb_t op2, ::mp_limb_t *hi)
{
    using dlimb_t = std::uint_least64_t;
    const dlimb_t res = dlimb_t(op1) * op2;
    *hi = static_cast<::mp_limb_t>(res >> 32);
    return static_cast<::mp_limb_t>(res);
}

#endif

// 1-limb optimization via dlimb.
template <std::size_t SSize>
inline std::size_t static_mul_impl(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2,
                                   mpz_size_t, mpz_size_t, int, int, const std::integral_constant<int, 1> &)
{
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mp_limb_t hi;
    const ::mp_limb_t lo = dlimb_mul(op1.m_limbs[0], op2.m_limbs[0], &hi);
    if (mppp_unlikely(hi)) {
        return 2u;
    }
    // The size will be zero if at least one operand is zero, otherwise +-1
    // depending on the signs of the operands.
    rop._mp_size = op1._mp_size * op2._mp_size;
    rop.m_limbs[0] = lo;
    return 0u;
}

// 2-limb optimization via dlimb.
template <std::size_t SSize>
inline std::size_t static_mul_impl(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2,
                                   mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2,
                                   const std::integral_constant<int, 2> &)
{
    if (asize1 <= 1 && asize2 <= 1) {
        // NOTE: this handles zeroes as well: we know that the limbs are all zeroes in such
        // a case (as we always guarantee unused limbs are zeroed), and the sign1 * sign2 multiplication takes care of
        // the size.
        rop.m_limbs[0] = dlimb_mul(op1.m_limbs[0], op2.m_limbs[0], &rop.m_limbs[1]);
        rop._mp_size = sign1 * sign2 * static_cast<mpz_size_t>(2 - (rop.m_limbs[1] == 0u));
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
        // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
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
            rop._mp_size = sign1 * sign2 * asize;
            rop.m_limbs[0] = tmp0;
            rop.m_limbs[1] = tmp1;
            return 0u;
        }
    }
    // Return 4 as a size hint. Real size could be 3, but GMP will require 4 limbs
    // of storage to perform the operation anyway.
    return 4u;
}

template <std::size_t SSize>
inline std::size_t static_mul(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2)
{
    // Cache a few quantities, detect signs.
    const mpz_size_t asize1 = std::abs(op1._mp_size), asize2 = std::abs(op2._mp_size);
    const int sign1 = integral_sign(op1._mp_size), sign2 = integral_sign(op2._mp_size);
    const std::size_t retval
        = static_mul_impl(rop, op1, op2, asize1, asize2, sign1, sign2, integer_static_mul_algo<static_int<SSize>>{});
    if (integer_static_mul_algo<static_int<SSize>>::value == 0 && retval == 0u) {
        // If we used the mpn functions, and actually wrote into the result,
        // zero the unused limbs on top (if necessary).
        // NOTE: same as above, we may end up using mpn functions for SSize <= opt_size.
        rop.zero_unused_limbs();
    }
    return retval;
}
} // namespace detail

// Ternary multiplication.
template <std::size_t SSize>
inline integer<SSize> &mul(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool s1 = op1.is_static(), s2 = op2.is_static();
    bool sr = rop.is_static();
    std::size_t size_hint = 0u;
    if (mppp_likely(s1 && s2)) {
        if (!sr) {
            rop.set_zero();
            sr = true;
        }
        size_hint = static_mul(rop._get_union().g_st(), op1._get_union().g_st(), op2._get_union().g_st());
        if (mppp_likely(size_hint == 0u)) {
            return rop;
        }
    }
    if (sr) {
        // We use the size hint from the static_mul if available, otherwise a normal promotion will take place.
        // NOTE: here the best way of proceeding would be to calculate the max size of the result based on
        // the op1/op2 sizes, but for whatever reason this computation has disastrous performance consequences
        // on micro-benchmarks. We need to understand if that's the case in real-world scenarios as well, and
        // revisit this.
        rop._get_union().promote(size_hint);
    }
    mpz_mul(&rop._get_union().g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    return rop;
}

namespace detail
{

// Selection of the algorithm for addmul: if optimised algorithms exist for both add and mul, then use the
// optimised addmul algos. Otherwise, use the mpn one.
template <typename SInt>
using integer_static_addmul_algo = std::integral_constant<
    int, (integer_static_add_algo<SInt>::value == 2 && integer_static_mul_algo<SInt>::value == 2)
             ? 2
             : ((integer_static_add_algo<SInt>::value == 1 && integer_static_mul_algo<SInt>::value == 1) ? 1 : 0)>;

// NOTE: same return value as mul: 0 for success, otherwise a hint for the size of the result.
template <std::size_t SSize>
inline std::size_t static_addmul_impl(static_int<SSize> &rop, const static_int<SSize> &op1,
                                      const static_int<SSize> &op2, mpz_size_t asizer, mpz_size_t asize1,
                                      mpz_size_t asize2, int signr, int sign1, int sign2,
                                      const std::integral_constant<int, 0> &)
{
    // First try to do the static prod, if it does not work it's a failure.
    static_int<SSize> prod;
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
    if (mppp_unlikely(
            !static_add_impl(rop, rop, prod, asizer, asize_prod, signr, sign_prod, std::integral_constant<int, 0>{}))) {
        return SSize + 1u;
    }
    return 0u;
}

template <std::size_t SSize>
inline std::size_t static_addmul_impl(static_int<SSize> &rop, const static_int<SSize> &op1,
                                      const static_int<SSize> &op2, mpz_size_t, mpz_size_t, mpz_size_t, int signr,
                                      int sign1, int sign2, const std::integral_constant<int, 1> &)
{
    // First we do op1 * op2.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mp_limb_t tmp;
    const ::mp_limb_t prod = dlimb_mul(op1.m_limbs[0], op2.m_limbs[0], &tmp);
    if (mppp_unlikely(tmp)) {
        return 3u;
    }
    // Determine the sign of the product: 0, 1 or -1.
    const int sign_prod = sign1 * sign2;
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
            rop._mp_size = signr * static_cast<int>(tmp != 0u);
            rop.m_limbs[0] = tmp;
        } else {
            // NOTE: this cannot be zero, as rop and prod cannot be equal.
            rop._mp_size = sign_prod;
            rop.m_limbs[0] = prod - rop.m_limbs[0];
        }
    }
    return 0u;
}

template <std::size_t SSize>
inline std::size_t static_addmul_impl(static_int<SSize> &rop, const static_int<SSize> &op1,
                                      const static_int<SSize> &op2, mpz_size_t asizer, mpz_size_t asize1,
                                      mpz_size_t asize2, int signr, int sign1, int sign2,
                                      const std::integral_constant<int, 2> &)
{
    if (mppp_unlikely(!asize1 || !asize2)) {
        // If op1 or op2 are zero, rop will be unchanged.
        return 0u;
    }
    // Handle op1 * op2.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, 2> prod;
    const int sign_prod = sign1 * sign2;
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    mpz_size_t asize_prod;
    if (asize1 == 1 && asize2 == 1) {
        prod[0] = dlimb_mul(op1.m_limbs[0], op2.m_limbs[0], &prod[1]);
        asize_prod = (asize1 + asize2) - (prod[1] == 0u);
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
        // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
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
        // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
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
        rop._mp_size = signr * (static_cast<int>(hi2 != 0u) + 1);
        rop.m_limbs[0] = lo;
        rop.m_limbs[1] = hi2;
    } else {
        // When the signs differ, we need to implement addition as a subtraction.
        if (asizer > asize_prod
            || (asizer == asize_prod && integer_compare_limbs_2(&rop.m_limbs[0], &prod[0], asizer) >= 0)) {
            // rop >= prod in absolute value.
            const auto lo = rop.m_limbs[0] - prod[0];
            // If there's a borrow, then hi1 > hi2, otherwise we would have a negative result.
            assert(rop.m_limbs[0] >= prod[0] || rop.m_limbs[1] > prod[1]);
            // This can never wrap around, at most it goes to zero.
            const auto hi = rop.m_limbs[1] - prod[1] - static_cast<::mp_limb_t>(rop.m_limbs[0] < prod[0]);
            // asize can be 0, 1 or 2.
            rop._mp_size = signr * size_from_lohi(lo, hi);
            rop.m_limbs[0] = lo;
            rop.m_limbs[1] = hi;
        } else {
            // prod > rop in absolute value.
            const auto lo = prod[0] - rop.m_limbs[0];
            assert(prod[0] >= rop.m_limbs[0] || prod[1] > rop.m_limbs[1]);
            const auto hi = prod[1] - rop.m_limbs[1] - static_cast<::mp_limb_t>(prod[0] < rop.m_limbs[0]);
            // asize can be 1 or 2, but not zero as we know abs(prod) != abs(rop).
            rop._mp_size = sign_prod * (static_cast<int>(hi != 0u) + 1);
            rop.m_limbs[0] = lo;
            rop.m_limbs[1] = hi;
        }
    }
    return 0u;
}

template <bool AddOrSub, std::size_t SSize>
inline std::size_t static_addsubmul(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2)
{
    // NOTE: according to the benchmarks, it seems a clear win here to use abs/integral_sign instead
    // of branching: there's about a ~10% penalty in the unsigned case, but ~20% faster for the signed case.
    const mpz_size_t asizer = std::abs(rop._mp_size), asize1 = std::abs(op1._mp_size), asize2 = std::abs(op2._mp_size);
    const int signr = integral_sign(rop._mp_size), sign1 = integral_sign(op1._mp_size),
              // NOTE: negate op2 in case we are doing a submul.
        sign2 = AddOrSub ? integral_sign(op2._mp_size) : -integral_sign(op2._mp_size);
    const std::size_t retval = static_addmul_impl(rop, op1, op2, asizer, asize1, asize2, signr, sign1, sign2,
                                                  integer_static_addmul_algo<static_int<SSize>>{});
    if (integer_static_addmul_algo<static_int<SSize>>::value == 0 && retval == 0u) {
        // If we used the mpn functions, and actually wrote into the result,
        // zero the unused limbs on top (if necessary).
        // NOTE: as usual, potential of mpn use on optimised size.
        rop.zero_unused_limbs();
    }
    return retval;
}
} // namespace detail

// Ternary multiply–add.
template <std::size_t SSize>
inline integer<SSize> &addmul(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool sr = rop.is_static(), s1 = op1.is_static(), s2 = op2.is_static();
    std::size_t size_hint = 0u;
    if (mppp_likely(sr && s1 && s2)) {
        size_hint
            = detail::static_addsubmul<true>(rop._get_union().g_st(), op1._get_union().g_st(), op2._get_union().g_st());
        if (mppp_likely(size_hint == 0u)) {
            return rop;
        }
    }
    if (sr) {
        rop._get_union().promote(size_hint);
    }
    mpz_addmul(&rop._get_union().g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    return rop;
}

// Ternary multiply–sub.
template <std::size_t SSize>
inline integer<SSize> &submul(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool sr = rop.is_static(), s1 = op1.is_static(), s2 = op2.is_static();
    std::size_t size_hint = 0u;
    if (mppp_likely(sr && s1 && s2)) {
        size_hint = detail::static_addsubmul<false>(rop._get_union().g_st(), op1._get_union().g_st(),
                                                    op2._get_union().g_st());
        if (mppp_likely(size_hint == 0u)) {
            return rop;
        }
    }
    if (sr) {
        rop._get_union().promote(size_hint);
    }
    mpz_submul(&rop._get_union().g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    return rop;
}

namespace detail
{

// mpn implementation.
template <std::size_t SSize>
inline std::size_t static_mul_2exp(static_int<SSize> &rop, const static_int<SSize> &n, std::size_t s)
{
    static_assert(SSize > static_int<SSize>::opt_size, "Cannot use mpn functions for optimised static size.");
    mpz_size_t asize = n._mp_size;
    if (s == 0u || asize == 0) {
        // If shift is zero, or the operand is zero, write n into rop and return success.
        rop = n;
        return 0u;
    }
    // Finish setting up asize and sign.
    int sign = 1;
    if (asize < 0) {
        asize = -asize;
        sign = -1;
    }
    // ls: number of entire limbs shifted.
    // rs: effective shift that will be passed to the mpn function.
    const std::size_t ls = s / unsigned(GMP_NUMB_BITS), rs = s % unsigned(GMP_NUMB_BITS);
    // At the very minimum, the new asize will be the old asize plus ls.
    // Check if we can represent it first.
    // NOTE: use >= because we may need to increase by 1 new_asize at the end, as a size hint.
    // LCOV_EXCL_START
    if (mppp_unlikely(ls >= nl_max<std::size_t>() - static_cast<std::size_t>(asize))) {
        // NOTE: don't think this can be hit on any setup currently.
        throw std::overflow_error("A left bitshift value of " + detail::to_string(s) + " is too large");
    }
    // LCOV_EXCL_STOP
    const std::size_t new_asize = static_cast<std::size_t>(asize) + ls;
    if (new_asize < SSize) {
        // In this case the operation will always succeed, and we can write directly into rop.
        ::mp_limb_t ret = 0u;
        if (rs) {
            // Perform the shift via the mpn function, if we are effectively shifting at least 1 bit.
            // Overlapping is fine, as it is guaranteed that rop.m_limbs.data() + ls >= n.m_limbs.data().
            ret = mpn_lshift(rop.m_limbs.data() + ls, n.m_limbs.data(), static_cast<::mp_size_t>(asize), unsigned(rs));
            // Write bits spilling out.
            rop.m_limbs[new_asize] = ret;
        } else {
            // Otherwise, just copy over (the mpn function requires the shift to be at least 1).
            // NOTE: we have to use move_backward here because the ranges may be overlapping but not
            // starting at the same pointer (in which case we could've used copy_limbs()). Here we know ls is not
            // zero: we don't have a remainder, and s == 0 was already handled above. Hence, new_asize > asize.
            assert(new_asize > static_cast<std::size_t>(asize));
            std::move_backward(n.m_limbs.begin(), n.m_limbs.begin() + asize, rop.m_limbs.begin() + new_asize);
        }
        // Zero the lower limbs vacated by the shift. We need to do this as we don't know
        // the state of rop.
        std::fill(rop.m_limbs.data(), rop.m_limbs.data() + ls, ::mp_limb_t(0));
        // Set the size.
        rop._mp_size = sign * (static_cast<mpz_size_t>(new_asize) + (ret != 0u));
        return 0u;
    }
    if (new_asize == SSize) {
        if (rs) {
            // In this case the operation may fail, so we need to write to temporary storage.
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
            std::array<::mp_limb_t, SSize> tmp;
            if (mpn_lshift(tmp.data(), n.m_limbs.data(), static_cast<::mp_size_t>(asize), unsigned(rs))) {
                return SSize + 1u;
            }
            // The shift was successful without spill over, copy the content from the tmp
            // storage into rop.
            copy_limbs_no(tmp.data(), tmp.data() + asize, rop.m_limbs.data() + ls);
        } else {
            // If we shifted by a multiple of the limb size, then we can write directly to rop.
            assert(new_asize > static_cast<std::size_t>(asize));
            std::move_backward(n.m_limbs.begin(), n.m_limbs.begin() + asize, rop.m_limbs.begin() + new_asize);
        }
        // Zero the lower limbs vacated by the shift.
        std::fill(rop.m_limbs.data(), rop.m_limbs.data() + ls, ::mp_limb_t(0));
        // Set the size.
        rop._mp_size = sign * static_cast<mpz_size_t>(new_asize);
        return 0u;
    }
    // Shift is too much, the size will overflow the static limit. Return a hint for the size of the result.
    // We checked above we can compute this safely.
    return new_asize + 1u;
}

// 1-limb optimisation.
inline std::size_t static_mul_2exp(static_int<1> &rop, const static_int<1> &n, std::size_t s)
{
    const ::mp_limb_t l = n.m_limbs[0] & GMP_NUMB_MASK;
    if (s == 0u || l == 0u) {
        // If shift is zero, or the operand is zero, write n into rop and return success.
        rop = n;
        return 0u;
    }
    if (mppp_unlikely(s >= unsigned(GMP_NUMB_BITS) || (l >> (unsigned(GMP_NUMB_BITS) - s)))) {
        // The two conditions:
        // - if the shift is >= number of data bits, the operation will certainly
        //   fail (as the operand is nonzero);
        // - shifting by s would overflow the single limb.
        // NOTE: s is at least 1, so in the right shift above we never risk UB due to too much shift.
        // NOTE: for the size hint: s / nbits is the number of entire limbs shifted, +1 because the shifted
        // limbs add to the current size (1), +1 because another limb might be needed.
        // NOTE: paranoia static assert to make sure there's no chance of overflowing.
        static_assert(GMP_NUMB_BITS > 1, "Invalid number of bits.");
        return s / unsigned(GMP_NUMB_BITS) + 2u;
    }
    // Write out.
    rop.m_limbs[0] = l << s;
    // Size is the same as the input value.
    rop._mp_size = n._mp_size;
    return 0u;
}

// 2-limb optimisation.
inline std::size_t static_mul_2exp(static_int<2> &rop, const static_int<2> &n, std::size_t s)
{
    // NOTE: it looks like in this function abs + integral sign are a definite
    // win in all cases, possibly because there's enough work to do in the function
    // to make any branching advantage (in case of only unsigned operands)
    // to be negligible.
    const mpz_size_t asize = std::abs(n._mp_size);
    if (s == 0u || asize == 0) {
        rop = n;
        return 0u;
    }
    const int sign = integral_sign(n._mp_size);
    // Too much shift, this can never work on a nonzero value.
    static_assert(unsigned(GMP_NUMB_BITS) <= nl_max<unsigned>() / 2u, "Overflow error.");
    if (mppp_unlikely(s >= 2u * unsigned(GMP_NUMB_BITS))) {
        // NOTE: this is the generic formula to estimate the final size.
        // NOTE: paranoia static assert to make sure there's no chance of overflowing.
        static_assert(GMP_NUMB_BITS > 1, "Invalid number of bits.");
        // NOTE: asize is 2 at most, no need to check for overflow.
        return s / unsigned(GMP_NUMB_BITS) + 1u + static_cast<std::size_t>(asize);
    }
    if (s == unsigned(GMP_NUMB_BITS)) {
        // This case can be dealt with moving lo into hi, but only if asize is 1.
        if (mppp_unlikely(asize == 2)) {
            // asize is 2, shift is too much.
            return 3u;
        }
        rop.m_limbs[1u] = n.m_limbs[0u];
        rop.m_limbs[0u] = 0u;
        // The asize has to be 2.
        rop._mp_size = 2 * sign;
        return 0u;
    }
    // Temp hi lo limbs to store the result that will eventually go into rop.
    ::mp_limb_t lo = n.m_limbs[0u], hi = n.m_limbs[1u];
    if (s > unsigned(GMP_NUMB_BITS)) {
        if (mppp_unlikely(asize == 2)) {
            return s / unsigned(GMP_NUMB_BITS) + 1u + static_cast<std::size_t>(asize);
        }
        // Move lo to hi and set lo to zero.
        hi = n.m_limbs[0u];
        lo = 0u;
        // Update the shift.
        s = static_cast<std::size_t>(s - unsigned(GMP_NUMB_BITS));
    }
    // Check that hi will not be shifted too much. Note that
    // here and below s can never be zero, so we never shift too much.
    assert(s > 0u && s < unsigned(GMP_NUMB_BITS));
    if (mppp_unlikely((hi & GMP_NUMB_MASK) >> (unsigned(GMP_NUMB_BITS) - s))) {
        return 3u;
    }
    // Shift hi and lo. hi gets the carry over from lo.
    hi = ((hi & GMP_NUMB_MASK) << s) + ((lo & GMP_NUMB_MASK) >> (unsigned(GMP_NUMB_BITS) - s));
    // NOTE: here the result needs to be masked as well as the shift could
    // end up writing in nail bits.
    lo = ((lo & GMP_NUMB_MASK) << s) & GMP_NUMB_MASK;
    // Write rop.
    rop.m_limbs[0u] = lo;
    rop.m_limbs[1u] = hi;
    // asize is at least 1.
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    rop._mp_size = sign * (1 + (hi != 0u));
    return 0u;
}
} // namespace detail

// Ternary left shift.
template <std::size_t SSize>
inline integer<SSize> &mul_2exp(integer<SSize> &rop, const integer<SSize> &n, ::mp_bitcnt_t s)
{
    const bool sn = n.is_static();
    bool sr = rop.is_static();
    std::size_t size_hint = 0u;
    if (mppp_likely(sn)) {
        // NOTE: we cast to size_t because it's more convenient for reasoning about number of limbs
        // in the implementation functions.
        // NOTE: do it before touching rop, for exception safety.
        const auto s_size = detail::safe_cast<std::size_t>(s);
        if (!sr) {
            rop.set_zero();
            sr = true;
        }
        size_hint = static_mul_2exp(rop._get_union().g_st(), n._get_union().g_st(), s_size);
        if (mppp_likely(size_hint == 0u)) {
            return rop;
        }
    }
    if (sr) {
        rop._get_union().promote(size_hint);
    }
    mpz_mul_2exp(&rop._get_union().g_dy(), n.get_mpz_view(), s);
    return rop;
}

namespace detail
{

// Selection of the algorithm for the
// static squaring. We'll be using the
// double-limb mul primitives if available.
template <typename SInt>
using integer_static_sqr_algo
    = std::integral_constant<int, (SInt::s_size == 1 && integer_have_dlimb_mul::value)
                                      ? 1
                                      : ((SInt::s_size == 2 && integer_have_dlimb_mul::value) ? 2 : 0)>;

// mpn implementation.
// NOTE: this function (and the other overloads) returns 0 in case of success, otherwise it returns a hint
// about the size in limbs of the result.
template <std::size_t SSize>
inline std::size_t static_sqr_impl(static_int<SSize> &rop, const static_int<SSize> &op,
                                   const std::integral_constant<int, 0> &)
{
    const auto asize = static_cast<std::size_t>(std::abs(op._mp_size));

    // Handle zero.
    if (mppp_unlikely(asize == 0u)) {
        rop._mp_size = 0;
        return 0u;
    }

    // Temporary storage for mpn_sqr(). The largest possible
    // size needed is twice the static size.
    // NOTE: here we could add some logic, like in static_mul_impl(),
    // to check whether we can write directly into rop instead
    // of using this temporary storage. Need to profile first,
    // however, because I am not sure whether this is worth
    // it or not performance-wise.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, SSize * 2u> res;
    const auto res_data = res.data();

    // Run the mpn function.
    mpn_sqr(res_data, op.m_limbs.data(), static_cast<::mp_size_t>(asize));

    // Compute the actual size of the result: it could be asize * 2
    // or 1 less, depending on whether the most significant limb of the
    // result is zero.
    const auto res_size = asize * 2u - static_cast<std::size_t>((res_data[asize * 2u - 1u] & GMP_NUMB_MASK) == 0u);
    if (res_size > SSize) {
        // Not enough space: return asize * 2, which is
        // what mpz_mul() would like to allocate for the result.
        return asize * 2u;
    }

    // Enough space, write out the result.
    rop._mp_size = static_cast<mpz_size_t>(res_size);
    copy_limbs_no(res_data, res_data + res_size, rop.m_limbs.data());

    return 0u;
}

// 1-limb optimization via dlimb.
template <std::size_t SSize>
inline std::size_t static_sqr_impl(static_int<SSize> &rop, const static_int<SSize> &op,
                                   const std::integral_constant<int, 1> &)
{
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mp_limb_t hi;
    const ::mp_limb_t lo = dlimb_mul(op.m_limbs[0], op.m_limbs[0], &hi);
    if (mppp_unlikely(hi)) {
        return 2u;
    }

    // NOTE: if op is zero, the output size is zero,
    // otherwise the output size is 1 (cannot be 2, handled
    // above).
    rop._mp_size = static_cast<mpz_size_t>(op._mp_size != 0);
    rop.m_limbs[0] = lo;

    return 0;
}

// 2-limb optimization via dlimb.
template <std::size_t SSize>
inline std::size_t static_sqr_impl(static_int<SSize> &rop, const static_int<SSize> &op,
                                   const std::integral_constant<int, 2> &)
{
    const auto asize = std::abs(op._mp_size);

    if (asize == 2) {
        // If asize is 2, this function cannot succeed.
        // Return 4 as a size hint. Real size could be 3, but GMP will require 4 limbs
        // of storage to perform the operation anyway.
        return 4u;
    }

    // NOTE: this handles zeroes as well: we know that the limbs are all zeroes in such
    // a case (as we always guarantee unused limbs are zeroed).
    rop.m_limbs[0] = dlimb_mul(op.m_limbs[0], op.m_limbs[0], &rop.m_limbs[1]);
    // NOTE: if asize == 0, then rop's limbs will have been set
    // to zero via dlimb_mul(), and rop's size will also thus be zero.
    // Otherwise, rop's size is 1 or 2, depending on whether the top limb
    // is set or not.
    rop._mp_size = static_cast<mpz_size_t>(2 - (asize == 0) - (rop.m_limbs[1] == 0u));

    return 0;
}

template <std::size_t SSize>
inline std::size_t static_sqr(static_int<SSize> &rop, const static_int<SSize> &op)
{
    const std::size_t retval = static_sqr_impl(rop, op, integer_static_sqr_algo<static_int<SSize>>{});

    if (integer_static_sqr_algo<static_int<SSize>>::value == 0 && retval == 0u) {
        // If we used the mpn functions, and actually wrote into the result,
        // zero the unused limbs on top (if necessary).
        // NOTE: as elsewhere, if we don't have double-limb primitives
        // available, we may end up using mpn functions also for
        // small sizes.
        rop.zero_unused_limbs();
    }

    return retval;
}

} // namespace detail

// Binary squaring.
template <std::size_t SSize>
inline integer<SSize> &sqr(integer<SSize> &rop, const integer<SSize> &n)
{
    const bool sn = n.is_static();
    bool sr = rop.is_static();
    std::size_t size_hint = 0u;
    if (mppp_likely(sn)) {
        if (!sr) {
            rop.set_zero();
            sr = true;
        }
        size_hint = static_sqr(rop._get_union().g_st(), n._get_union().g_st());
        if (mppp_likely(size_hint == 0u)) {
            return rop;
        }
    }
    if (sr) {
        rop._get_union().promote(size_hint);
    }
    mpz_mul(&rop._get_union().g_dy(), n.get_mpz_view(), n.get_mpz_view());
    return rop;
}

// Unary squaring.
template <std::size_t SSize>
inline integer<SSize> sqr(const integer<SSize> &n)
{
    integer<SSize> retval;
    sqr(retval, n);
    return retval;
}

namespace detail
{

// Detect the presence of dual-limb division/remainder. This is currently possible only if:
// - we are on a 32bit build (with the usual constraints that the types have exactly 32/64 bits and no nails),
// - we are on a 64bit build and we have the 128bit int type available (plus usual constraints).
// NOTE: starting from MSVC 2019, there are some 128bit division intrinsics
// available:
// https://docs.microsoft.com/en-us/cpp/intrinsics/udiv128
using integer_have_dlimb_div = std::integral_constant<bool,
#if defined(MPPP_HAVE_GCC_INT128) && (GMP_NUMB_BITS == 64)
                                                      !GMP_NAIL_BITS && nl_digits<::mp_limb_t>() == 64
#elif GMP_NUMB_BITS == 32
                                                      !GMP_NAIL_BITS && nl_digits<std::uint_least64_t>() == 64
                                                          && nl_digits<::mp_limb_t>() == 32
#else
                                                      false
#endif
                                                      >;

// Selection of the algorithm for static modular squaring:
// - for 1 limb, we need both the double-limb multiplication AND
//   division/remainder primitives,
// - otherwise we just use the mpn functions.
// NOTE: the integer_have_dlimb_mul and integer_have_dlimb_div machinery
// already checks for lack of nail bits, bit sizes, etc.
template <typename SInt>
using integer_static_sqrm_algo = std::integral_constant<
    int, (SInt::s_size == 1 && integer_have_dlimb_mul::value && integer_have_dlimb_div::value) ? 1 : 0>;

// mpn implementation.
// NOTE: this assumes that mod is not zero.
template <std::size_t SSize>
inline void static_sqrm_impl(static_int<SSize> &rop, const static_int<SSize> &op, const static_int<SSize> &mod,
                             const std::integral_constant<int, 0> &)
{
    // Fetch the asize of the operand.
    const auto asize = static_cast<std::size_t>(std::abs(op._mp_size));

    // Handle zero operand.
    if (mppp_unlikely(asize == 0u)) {
        rop._mp_size = 0;
        return;
    }

    // Fetch the asize of the mod argument.
    const auto mod_asize = static_cast<std::size_t>(std::abs(mod._mp_size));
    assert(mod_asize != 0u);

    // Temporary storage for mpn_sqr(). The largest possible
    // size needed is twice the static size.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, SSize * 2u> sqr_res;
    const auto sqr_res_data = sqr_res.data();

    // Run the squaring function.
    mpn_sqr(sqr_res_data, op.m_limbs.data(), static_cast<::mp_size_t>(asize));
    // Compute the actual size of the result of the squaring: it could be asize * 2
    // or 1 less, depending on whether the most significant limb of the
    // result is zero.
    const auto sqr_res_asize
        = asize * 2u - static_cast<std::size_t>((sqr_res_data[asize * 2u - 1u] & GMP_NUMB_MASK) == 0u);

    if (mod_asize > sqr_res_asize) {
        // The divisor is larger than the square of op.
        // Copy the square of op to rop and exit.
        rop._mp_size = static_cast<mpz_size_t>(sqr_res_asize);
        copy_limbs_no(sqr_res_data, sqr_res_data + sqr_res_asize, rop.m_limbs.data());

        return;
    }

    // Temporary storage for the modulo operation.
    // Need space for both quotient and remainder.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, SSize * 2u> q_res, r_res;
    // Run the modulo operation.
    // NOTE: here we could add some logic, like in tdiv_qr(),
    // to check whether we can write directly into rop instead
    // of using this temporary storage. Need to profile first,
    // however, because I am not sure whether this is worth
    // it or not performance-wise.
    // NOTE: ret_size will never be negative, as the
    // dividend is a square and thus also never negative.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    mpz_size_t ret_size;
    if (mod_asize == 1u) {
        // Optimization when the divisor has 1 limb.
        r_res[0] = mpn_divrem_1(q_res.data(), 0, sqr_res_data, static_cast<::mp_size_t>(sqr_res_asize), mod.m_limbs[0]);
        // The size can be 1 or zero, depending on the value
        // of the only limb in r_res.
        ret_size = static_cast<mpz_size_t>((r_res[0] & GMP_NUMB_MASK) != 0u);
    } else {
        // The general case.
        mpn_tdiv_qr(q_res.data(), r_res.data(), 0, sqr_res_data, static_cast<::mp_size_t>(sqr_res_asize),
                    mod.m_limbs.data(), static_cast<::mp_size_t>(mod_asize));
        // Determine the size of the output, which will
        // be in the [0, mod_asize] range.
        ret_size = static_cast<mpz_size_t>(mod_asize);
        while (ret_size && !(r_res[static_cast<std::size_t>(ret_size - 1)] & GMP_NUMB_MASK)) {
            --ret_size;
        }
    }

    // Write out the result.
    rop._mp_size = ret_size;
    copy_limbs_no(r_res.data(), r_res.data() + ret_size, rop.m_limbs.data());
}

#if defined(MPPP_HAVE_GCC_INT128) && (GMP_NUMB_BITS == 64) && !GMP_NAIL_BITS

inline ::mp_limb_t static_sqrm_impl_1(::mp_limb_t op, ::mp_limb_t mod)
{
    using dlimb_t = __uint128_t;
    return static_cast<::mp_limb_t>((dlimb_t(op) * op) % mod);
}

#elif GMP_NUMB_BITS == 32 && !GMP_NAIL_BITS

inline ::mp_limb_t static_sqrm_impl_1(::mp_limb_t op, ::mp_limb_t mod)
{
    using dlimb_t = std::uint_least64_t;
    return static_cast<::mp_limb_t>((dlimb_t(op) * op) % mod);
}

#endif

// 1-limb optimization via dlimb.
// NOTE: this assumes that mod is not zero.
template <std::size_t SSize>
inline void static_sqrm_impl(static_int<SSize> &rop, const static_int<SSize> &op, const static_int<SSize> &mod,
                             const std::integral_constant<int, 1> &)
{
    assert(mod._mp_size != 0);

    // NOTE: no need for masking, as
    // we are sure that there are no nails if
    // this overload is selected.
    const auto ret = static_sqrm_impl_1(op.m_limbs[0], mod.m_limbs[0]);

    rop._mp_size = static_cast<mpz_size_t>(ret != 0u);
    rop.m_limbs[0] = ret;
}

template <std::size_t SSize>
inline void static_sqrm(static_int<SSize> &rop, const static_int<SSize> &op, const static_int<SSize> &mod)
{
    static_sqrm_impl(rop, op, mod, integer_static_sqrm_algo<static_int<SSize>>{});

    if (integer_static_sqrm_algo<static_int<SSize>>::value == 0) {
        // If we used the mpn functions, zero the unused limbs on top (if necessary).
        // NOTE: as elsewhere, if we don't have double-limb primitives
        // available, we may end up using mpn functions also for
        // small sizes.
        rop.zero_unused_limbs();
    }
}

} // namespace detail

// Ternary modular squaring.
template <std::size_t SSize>
inline integer<SSize> &sqrm(integer<SSize> &rop, const integer<SSize> &op, const integer<SSize> &mod)
{
    if (mppp_unlikely(mod.sgn() == 0)) {
        throw zero_division_error("Integer division by zero");
    }

    const bool sr = rop.is_static(), so = op.is_static(), sm = mod.is_static();

    if (mppp_likely(so && sm)) {
        if (!sr) {
            rop.set_zero();
        }

        detail::static_sqrm(rop._get_union().g_st(), op._get_union().g_st(), mod._get_union().g_st());

        // Modular squaring can never fail.
        return rop;
    }

    if (sr) {
        rop._get_union().promote();
    }

    // NOTE: use temp storage to avoid issues with overlapping
    // arguments.
    MPPP_MAYBE_TLS detail::mpz_raii tmp;
    mpz_mul(&tmp.m_mpz, op.get_mpz_view(), op.get_mpz_view());
    mpz_tdiv_r(&rop._get_union().g_dy(), &tmp.m_mpz, mod.get_mpz_view());

    return rop;
}

// Binary modular squaring.
template <std::size_t SSize>
inline integer<SSize> sqrm(const integer<SSize> &op, const integer<SSize> &mod)
{
    integer<SSize> retval;
    sqrm(retval, op, mod);
    return retval;
}

// Binary negation.
template <std::size_t SSize>
inline integer<SSize> &neg(integer<SSize> &rop, const integer<SSize> &n)
{
    rop = n;
    return rop.neg();
}

// Unary negation.
template <std::size_t SSize>
inline integer<SSize> neg(const integer<SSize> &n)
{
    integer<SSize> ret(n);
    ret.neg();
    return ret;
}

// Binary absolute value.
template <std::size_t SSize>
inline integer<SSize> &abs(integer<SSize> &rop, const integer<SSize> &n)
{
    rop = n;
    return rop.abs();
}

// Unary absolute value.
template <std::size_t SSize>
inline integer<SSize> abs(const integer<SSize> &n)
{
    integer<SSize> ret(n);
    ret.abs();
    return ret;
}

namespace detail
{

// Selection of the algorithm for static division:
// - for 1 limb, we can always do static division,
// - for 2 limbs, we need the dual limb division if avaiable,
// - otherwise we just use the mpn functions.
template <typename SInt>
using integer_static_div_algo
    = std::integral_constant<int,
                             SInt::s_size == 1 ? 1 : ((SInt::s_size == 2 && integer_have_dlimb_div::value) ? 2 : 0)>;

// mpn implementation.
template <std::size_t SSize>
inline void static_tdiv_qr_impl(static_int<SSize> &q, static_int<SSize> &r, const static_int<SSize> &op1,
                                const static_int<SSize> &op2, mpz_size_t asize1, mpz_size_t asize2, int sign1,
                                int sign2, const std::integral_constant<int, 0> &)
{
    // First we check if the divisor is larger than the dividend (in abs limb size), as the mpn function
    // requires asize1 >= asize2.
    if (asize2 > asize1) {
        // Copy op1 into the remainder.
        r = op1;
        // Zero out q.
        q._mp_size = 0;
        return;
    }
    // NOTE: we checked outside that the divisor is not zero, and now asize1 >= asize2. Hence,
    // both operands are nonzero.
    // We need to take care of potentially overlapping arguments. We know that q and r are distinct, but op1
    // could overlap with q or r, and op2 could overlap with op1, q or r.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
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
    // Verify that all pointers are distinct. Used exclusively for debugging purposes.
    assert(([&q, &r, data1, data2]() -> bool {
        const ::mp_limb_t *ptrs[] = {q.m_limbs.data(), r.m_limbs.data(), data1, data2};
        std::sort(ptrs, ptrs + 4, std::less<const ::mp_limb_t *>());
        return std::unique(ptrs, ptrs + 4) == (ptrs + 4);
    }()));
    // Proceed to the division.
    if (asize2 == 1) {
        // Optimization when the divisor has 1 limb.
        r.m_limbs[0]
            = mpn_divrem_1(q.m_limbs.data(), ::mp_size_t(0), data1, static_cast<::mp_size_t>(asize1), data2[0]);
    } else {
        // General implementation.
        mpn_tdiv_qr(q.m_limbs.data(), r.m_limbs.data(), ::mp_size_t(0), data1, static_cast<::mp_size_t>(asize1), data2,
                    static_cast<::mp_size_t>(asize2));
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
template <std::size_t SSize>
inline void static_tdiv_qr_impl(static_int<SSize> &q, static_int<SSize> &r, const static_int<SSize> &op1,
                                const static_int<SSize> &op2, mpz_size_t, mpz_size_t, int sign1, int sign2,
                                const std::integral_constant<int, 1> &)
{
    // NOTE: here we have to use GMP_NUMB_MASK because if s_size is 1 this implementation is *always*
    // called, even if we have nail bits (whereas the optimisation for other operations currently kicks in
    // only without nail bits). Thus, we need to discard from m_limbs[0] the nail bits before doing the
    // division.
    const ::mp_limb_t n = op1.m_limbs[0] & GMP_NUMB_MASK, d = op2.m_limbs[0] & GMP_NUMB_MASK, q_ = n / d, r_ = n % d;
    // Write q.
    q._mp_size = sign1 * sign2 * (n >= d);
    // NOTE: there should be no need here to mask.
    q.m_limbs[0] = q_;
    // Write r.
    // Following C++11, the sign of r is the sign of op1:
    // http://stackoverflow.com/questions/13100711/operator-modulo-change-in-c-11
    r._mp_size = sign1 * (r_ != 0u);
    r.m_limbs[0] = r_;
}

// Implementation of the 2-limb division/remainder primitives,
// parametrised on the double limb type and limb bit width.
// These assume that there are no nail bits and that
// the bit width of DLimb is exactly twice the bit
// width of the limb type.

// Quotient+remainder.
template <typename DLimb, int NBits>
inline void dlimb_tdiv_qr_impl(::mp_limb_t op11, ::mp_limb_t op12, ::mp_limb_t op21, ::mp_limb_t op22,
                               ::mp_limb_t *MPPP_RESTRICT q1, ::mp_limb_t *MPPP_RESTRICT q2,
                               ::mp_limb_t *MPPP_RESTRICT r1, ::mp_limb_t *MPPP_RESTRICT r2)
{
    const auto op1 = op11 + (DLimb(op12) << NBits);
    const auto op2 = op21 + (DLimb(op22) << NBits);
    const auto q = op1 / op2, r = op1 % op2;
    *q1 = static_cast<::mp_limb_t>(q & ::mp_limb_t(-1));
    *q2 = static_cast<::mp_limb_t>(q >> NBits);
    *r1 = static_cast<::mp_limb_t>(r & ::mp_limb_t(-1));
    *r2 = static_cast<::mp_limb_t>(r >> NBits);
}

// Quotient only.
template <typename DLimb, int NBits>
inline void dlimb_tdiv_q_impl(::mp_limb_t op11, ::mp_limb_t op12, ::mp_limb_t op21, ::mp_limb_t op22,
                              ::mp_limb_t *MPPP_RESTRICT q1, ::mp_limb_t *MPPP_RESTRICT q2)
{
    const auto op1 = op11 + (DLimb(op12) << NBits);
    const auto op2 = op21 + (DLimb(op22) << NBits);
    const auto q = op1 / op2;
    *q1 = static_cast<::mp_limb_t>(q & ::mp_limb_t(-1));
    *q2 = static_cast<::mp_limb_t>(q >> NBits);
}

#if defined(MPPP_HAVE_GCC_INT128) && (GMP_NUMB_BITS == 64) && !GMP_NAIL_BITS

inline void dlimb_tdiv_qr(::mp_limb_t op11, ::mp_limb_t op12, ::mp_limb_t op21, ::mp_limb_t op22,
                          ::mp_limb_t *MPPP_RESTRICT q1, ::mp_limb_t *MPPP_RESTRICT q2, ::mp_limb_t *MPPP_RESTRICT r1,
                          ::mp_limb_t *MPPP_RESTRICT r2)
{
    dlimb_tdiv_qr_impl<__uint128_t, 64>(op11, op12, op21, op22, q1, q2, r1, r2);
}

inline void dlimb_tdiv_q(::mp_limb_t op11, ::mp_limb_t op12, ::mp_limb_t op21, ::mp_limb_t op22,
                         ::mp_limb_t *MPPP_RESTRICT q1, ::mp_limb_t *MPPP_RESTRICT q2)
{
    dlimb_tdiv_q_impl<__uint128_t, 64>(op11, op12, op21, op22, q1, q2);
}

#elif GMP_NUMB_BITS == 32 && !GMP_NAIL_BITS

inline void dlimb_tdiv_qr(::mp_limb_t op11, ::mp_limb_t op12, ::mp_limb_t op21, ::mp_limb_t op22,
                          ::mp_limb_t *MPPP_RESTRICT q1, ::mp_limb_t *MPPP_RESTRICT q2, ::mp_limb_t *MPPP_RESTRICT r1,
                          ::mp_limb_t *MPPP_RESTRICT r2)
{
    dlimb_tdiv_qr_impl<std::uint_least64_t, 32>(op11, op12, op21, op22, q1, q2, r1, r2);
}

inline void dlimb_tdiv_q(::mp_limb_t op11, ::mp_limb_t op12, ::mp_limb_t op21, ::mp_limb_t op22,
                         ::mp_limb_t *MPPP_RESTRICT q1, ::mp_limb_t *MPPP_RESTRICT q2)
{
    dlimb_tdiv_q_impl<std::uint_least64_t, 32>(op11, op12, op21, op22, q1, q2);
}

#endif

// 2-limbs optimisation.
template <std::size_t SSize>
inline void static_tdiv_qr_impl(static_int<SSize> &q, static_int<SSize> &r, const static_int<SSize> &op1,
                                const static_int<SSize> &op2, mpz_size_t asize1, mpz_size_t asize2, int sign1,
                                int sign2, const std::integral_constant<int, 2> &)
{
    if (asize1 < 2 && asize2 < 2) {
        // NOTE: testing indicates that it pays off to optimize the case in which the operands have
        // fewer than 2 limbs. This a slightly modified version of the 1-limb division from above,
        // without the need to mask as this function is called only if there are no nail bits.
        const ::mp_limb_t n = op1.m_limbs[0], d = op2.m_limbs[0], q_ = n / d, r_ = n % d;
        q._mp_size = sign1 * sign2 * (n >= d);
        q.m_limbs[0] = q_;
        q.m_limbs[1] = 0u;
        r._mp_size = sign1 * (r_ != 0u);
        r.m_limbs[0] = r_;
        r.m_limbs[1] = 0u;
        return;
    }
    // Perform the division.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mp_limb_t q1, q2, r1, r2;
    dlimb_tdiv_qr(op1.m_limbs[0], op1.m_limbs[1], op2.m_limbs[0], op2.m_limbs[1], &q1, &q2, &r1, &r2);
    // Write out.
    q._mp_size = sign1 * sign2 * size_from_lohi(q1, q2);
    q.m_limbs[0] = q1;
    q.m_limbs[1] = q2;
    r._mp_size = sign1 * size_from_lohi(r1, r2);
    r.m_limbs[0] = r1;
    r.m_limbs[1] = r2;
}

template <std::size_t SSize>
inline void static_tdiv_qr(static_int<SSize> &q, static_int<SSize> &r, const static_int<SSize> &op1,
                           const static_int<SSize> &op2)
{
    const auto s1(op1._mp_size), s2(op2._mp_size);
    const mpz_size_t asize1 = std::abs(s1), asize2 = std::abs(s2);
    const int sign1 = integral_sign(s1), sign2 = integral_sign(s2);
    static_tdiv_qr_impl(q, r, op1, op2, asize1, asize2, sign1, sign2, integer_static_div_algo<static_int<SSize>>{});
    if (integer_static_div_algo<static_int<SSize>>::value == 0) {
        // If we used the mpn functions, zero the unused limbs on top (if necessary).
        // NOTE: as usual, potential of mpn use on optimised size.
        q.zero_unused_limbs();
        r.zero_unused_limbs();
    }
}
} // namespace detail

// Truncated division with remainder.
template <std::size_t SSize>
inline void tdiv_qr(integer<SSize> &q, integer<SSize> &r, const integer<SSize> &n, const integer<SSize> &d)
{
    if (mppp_unlikely(&q == &r)) {
        throw std::invalid_argument("When performing a division with remainder, the quotient 'q' and the "
                                    "remainder 'r' must be distinct objects");
    }
    if (mppp_unlikely(d.sgn() == 0)) {
        throw zero_division_error("Integer division by zero");
    }
    const bool sq = q.is_static(), sr = r.is_static(), s1 = n.is_static(), s2 = d.is_static();
    if (mppp_likely(s1 && s2)) {
        if (!sq) {
            q.set_zero();
        }
        if (!sr) {
            r.set_zero();
        }
        static_tdiv_qr(q._get_union().g_st(), r._get_union().g_st(), n._get_union().g_st(), d._get_union().g_st());
        // Division can never fail.
        return;
    }
    if (sq) {
        q._get_union().promote();
    }
    if (sr) {
        r._get_union().promote();
    }
    mpz_tdiv_qr(&q._get_union().g_dy(), &r._get_union().g_dy(), n.get_mpz_view(), d.get_mpz_view());
}

namespace detail
{

// mpn implementation.
template <std::size_t SSize>
inline void static_tdiv_q_impl(static_int<SSize> &q, const static_int<SSize> &op1, const static_int<SSize> &op2,
                               mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2,
                               const std::integral_constant<int, 0> &)
{
    // First we check if the divisor is larger than the dividend (in abs limb size), as the mpn function
    // requires asize1 >= asize2.
    if (asize2 > asize1) {
        // Zero out q.
        q._mp_size = 0;
        return;
    }
    // NOTE: we checked outside that the divisor is not zero, and now asize1 >= asize2. Hence,
    // both operands are nonzero.
    // We need to take care of potentially overlapping arguments. op1 could overlap with q, and op2
    // could overlap with op1 or q.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, SSize> op1_alt, op2_alt;
    const ::mp_limb_t *data1 = op1.m_limbs.data();
    const ::mp_limb_t *data2 = op2.m_limbs.data();
    if (&op1 == &q) {
        copy_limbs_no(data1, data1 + asize1, op1_alt.data());
        data1 = op1_alt.data();
    }
    if (&op2 == &q || &op1 == &op2) {
        copy_limbs_no(data2, data2 + asize2, op2_alt.data());
        data2 = op2_alt.data();
    }
    // Verify that all pointers are distinct.
    assert(([&q, data1, data2]() -> bool {
        const ::mp_limb_t *ptrs[] = {q.m_limbs.data(), data1, data2};
        std::sort(ptrs, ptrs + 3, std::less<const ::mp_limb_t *>());
        return std::unique(ptrs, ptrs + 3) == (ptrs + 3);
    }()));
    // Proceed to the division.
    if (asize2 == 1) {
        // Optimization when the divisor has 1 limb.
        mpn_divrem_1(q.m_limbs.data(), ::mp_size_t(0), data1, static_cast<::mp_size_t>(asize1), data2[0]);
    } else {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        std::array<::mp_limb_t, SSize> r_unused;
        // General implementation.
        mpn_tdiv_qr(q.m_limbs.data(), r_unused.data(), ::mp_size_t(0), data1, static_cast<::mp_size_t>(asize1), data2,
                    static_cast<::mp_size_t>(asize2));
    }
    // Complete the quotient: compute size and sign.
    q._mp_size = asize1 - asize2 + 1;
    while (q._mp_size && !(q.m_limbs[static_cast<std::size_t>(q._mp_size - 1)] & GMP_NUMB_MASK)) {
        --q._mp_size;
    }
    if (sign1 != sign2) {
        q._mp_size = -q._mp_size;
    }
}

// 1-limb optimisation.
template <std::size_t SSize>
inline void static_tdiv_q_impl(static_int<SSize> &q, const static_int<SSize> &op1, const static_int<SSize> &op2,
                               mpz_size_t, mpz_size_t, int sign1, int sign2, const std::integral_constant<int, 1> &)
{
    // NOTE: here we have to use GMP_NUMB_MASK because if s_size is 1 this implementation is *always*
    // called, even if we have nail bits (whereas the optimisation for other operations currently kicks in
    // only without nail bits). Thus, we need to discard from m_limbs[0] the nail bits before doing the
    // division.
    const ::mp_limb_t n = op1.m_limbs[0] & GMP_NUMB_MASK, d = op2.m_limbs[0] & GMP_NUMB_MASK, q_ = n / d;
    // Write q.
    q._mp_size = sign1 * sign2 * (n >= d);
    // NOTE: there should be no need here to mask.
    q.m_limbs[0] = q_;
}

// 2-limbs optimisation.
template <std::size_t SSize>
inline void static_tdiv_q_impl(static_int<SSize> &q, const static_int<SSize> &op1, const static_int<SSize> &op2,
                               mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2,
                               const std::integral_constant<int, 2> &)
{
    if (asize1 < 2 && asize2 < 2) {
        // NOTE: testing indicates that it pays off to optimize the case in which the operands have
        // fewer than 2 limbs. This a slightly modified version of the 1-limb division from above,
        // without the need to mask as this function is called only if there are no nail bits.
        const ::mp_limb_t n = op1.m_limbs[0], d = op2.m_limbs[0], q_ = n / d;
        q._mp_size = sign1 * sign2 * (n >= d);
        q.m_limbs[0] = q_;
        q.m_limbs[1] = 0u;
        return;
    }
    // Perform the division.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mp_limb_t q1, q2;
    dlimb_tdiv_q(op1.m_limbs[0], op1.m_limbs[1], op2.m_limbs[0], op2.m_limbs[1], &q1, &q2);
    // Write out.
    q._mp_size = sign1 * sign2 * size_from_lohi(q1, q2);
    q.m_limbs[0] = q1;
    q.m_limbs[1] = q2;
}

template <std::size_t SSize>
inline void static_tdiv_q(static_int<SSize> &q, const static_int<SSize> &op1, const static_int<SSize> &op2)
{
    const auto s1(op1._mp_size), s2(op2._mp_size);
    const mpz_size_t asize1 = std::abs(s1), asize2 = std::abs(s2);
    const int sign1 = integral_sign(s1), sign2 = integral_sign(s2);
    // NOTE: use the same algorithm dispatching as in the division with remainder.
    static_tdiv_q_impl(q, op1, op2, asize1, asize2, sign1, sign2, integer_static_div_algo<static_int<SSize>>{});
    if (integer_static_div_algo<static_int<SSize>>::value == 0) {
        // If we used the mpn functions, zero the unused limbs on top (if necessary).
        // NOTE: as usual, potential of mpn use on optimised size.
        q.zero_unused_limbs();
    }
}
} // namespace detail

// Truncated division without remainder.
template <std::size_t SSize>
inline integer<SSize> &tdiv_q(integer<SSize> &q, const integer<SSize> &n, const integer<SSize> &d)
{
    if (mppp_unlikely(d.sgn() == 0)) {
        throw zero_division_error("Integer division by zero");
    }
    const bool sq = q.is_static(), s1 = n.is_static(), s2 = d.is_static();
    if (mppp_likely(s1 && s2)) {
        if (!sq) {
            q.set_zero();
        }
        static_tdiv_q(q._get_union().g_st(), n._get_union().g_st(), d._get_union().g_st());
        // Division can never fail.
        return q;
    }
    if (sq) {
        q._get_union().promote();
    }
    mpz_tdiv_q(&q._get_union().g_dy(), n.get_mpz_view(), d.get_mpz_view());
    return q;
}

namespace detail
{

// mpn implementation.
// NOTE: the Gcd flag specifies whether the divisor op2 is a strictly positive quantity.
template <bool Gcd, std::size_t SSize>
inline void static_divexact_impl(static_int<SSize> &q, const static_int<SSize> &op1, const static_int<SSize> &op2,
                                 mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2,
                                 const std::integral_constant<int, 0> &)
{
    assert(!Gcd || sign2 == 1);
    if (asize1 == 0) {
        // Special casing if the numerator is zero (the mpn functions do not work with zero operands).
        q._mp_size = 0;
        return;
    }
#if defined(MPPP_GMP_HAVE_MPN_DIVEXACT_1)
    if (asize2 == 1) {
        // Optimisation in case the dividend has only 1 limb.
        // NOTE: overlapping arguments are fine here.
        mpn_divexact_1(q.m_limbs.data(), op1.m_limbs.data(), static_cast<::mp_size_t>(asize1), op2.m_limbs[0]);
        // Complete the quotient: compute size and sign.
        q._mp_size = asize1 - asize2 + 1;
        while (q._mp_size && !(q.m_limbs[static_cast<std::size_t>(q._mp_size - 1)] & GMP_NUMB_MASK)) {
            --q._mp_size;
        }
        if (sign1 != (Gcd ? 1 : sign2)) {
            q._mp_size = -q._mp_size;
        }
        return;
    }
#else
    // Avoid compiler warnings for unused parameters.
    ignore(sign1, sign2, asize2);
#endif
    // General implementation (via the mpz function).
    MPPP_MAYBE_TLS mpz_raii tmp;
    const auto v1 = op1.get_mpz_view();
    const auto v2 = op2.get_mpz_view();
    mpz_divexact(&tmp.m_mpz, &v1, &v2);
    // Copy over from the tmp struct into q.
    q._mp_size = tmp.m_mpz._mp_size;
    copy_limbs_no(tmp.m_mpz._mp_d, tmp.m_mpz._mp_d + (q._mp_size >= 0 ? q._mp_size : -q._mp_size), q.m_limbs.data());
}

// 1-limb optimisation.
template <bool Gcd, std::size_t SSize>
inline void static_divexact_impl(static_int<SSize> &q, const static_int<SSize> &op1, const static_int<SSize> &op2,
                                 mpz_size_t, mpz_size_t, int sign1, int sign2, const std::integral_constant<int, 1> &)
{
    assert(!Gcd || sign2 == 1);
    const ::mp_limb_t n = op1.m_limbs[0] & GMP_NUMB_MASK, d = op2.m_limbs[0] & GMP_NUMB_MASK, q_ = n / d;
    // Write q.
    q._mp_size = sign1 * (Gcd ? 1 : sign2) * (n >= d);
    q.m_limbs[0] = q_;
}

// 2-limbs optimisation.
template <bool Gcd, std::size_t SSize>
inline void static_divexact_impl(static_int<SSize> &q, const static_int<SSize> &op1, const static_int<SSize> &op2,
                                 mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2,
                                 const std::integral_constant<int, 2> &)
{
    assert(!Gcd || sign2 == 1);
    if (asize1 < 2 && asize2 < 2) {
        // 1-limb optimisation.
        const ::mp_limb_t n = op1.m_limbs[0], d = op2.m_limbs[0], q_ = n / d;
        q._mp_size = sign1 * (Gcd ? 1 : sign2) * (n >= d);
        q.m_limbs[0] = q_;
        q.m_limbs[1] = 0u;
        return;
    }
    // General case.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mp_limb_t q1, q2;
    dlimb_tdiv_q(op1.m_limbs[0], op1.m_limbs[1], op2.m_limbs[0], op2.m_limbs[1], &q1, &q2);
    q._mp_size = sign1 * (Gcd ? 1 : sign2) * size_from_lohi(q1, q2);
    q.m_limbs[0] = q1;
    q.m_limbs[1] = q2;
}

template <std::size_t SSize>
inline void static_divexact(static_int<SSize> &q, const static_int<SSize> &op1, const static_int<SSize> &op2)
{
    const auto s1(op1._mp_size), s2(op2._mp_size);
    const mpz_size_t asize1 = std::abs(s1), asize2 = std::abs(s2);
    const int sign1 = integral_sign(s1), sign2 = integral_sign(s2);
    // NOTE: op2 divides exactly op1, so either op1 is zero or the absolute size
    // of op1 must not be smaller than op2's.
    assert(asize1 == 0 || asize2 <= asize1);
    // NOTE: use integer_static_div_algo for the algorithm selection.
    static_divexact_impl<false>(q, op1, op2, asize1, asize2, sign1, sign2,
                                integer_static_div_algo<static_int<SSize>>{});
    if (integer_static_div_algo<static_int<SSize>>::value == 0) {
        // If we used the mpn functions, zero the unused limbs on top (if necessary).
        // NOTE: as usual, potential of mpn use on optimised size.
        q.zero_unused_limbs();
    }
}
} // namespace detail

// Exact division (ternary version).
template <std::size_t SSize>
inline integer<SSize> &divexact(integer<SSize> &rop, const integer<SSize> &n, const integer<SSize> &d)
{
    const bool sr = rop.is_static(), s1 = n.is_static(), s2 = d.is_static();
    if (mppp_likely(s1 && s2)) {
        if (!sr) {
            rop.set_zero();
        }
        static_divexact(rop._get_union().g_st(), n._get_union().g_st(), d._get_union().g_st());
        // Division can never fail.
        return rop;
    }
    if (sr) {
        rop._get_union().promote();
    }
    mpz_divexact(&rop._get_union().g_dy(), n.get_mpz_view(), d.get_mpz_view());
    return rop;
}

// Exact division (binary version).
template <std::size_t SSize>
inline integer<SSize> divexact(const integer<SSize> &n, const integer<SSize> &d)
{
    integer<SSize> retval;
    divexact(retval, n, d);
    return retval;
}

namespace detail
{

template <std::size_t SSize>
inline void static_divexact_gcd(static_int<SSize> &q, const static_int<SSize> &op1, const static_int<SSize> &op2)
{
    const auto s1(op1._mp_size);
    const mpz_size_t asize1 = std::abs(s1), asize2 = op2._mp_size;
    const int sign1 = integral_sign(s1);
    // NOTE: op2 divides exactly op1, so either op1 is zero or the absolute size
    // of op1 must not be smaller than op2's.
    assert(asize1 == 0 || asize2 <= asize1);
    assert(asize2 > 0);
    // NOTE: use integer_static_div_algo for the algorithm selection.
    static_divexact_impl<true>(q, op1, op2, asize1, asize2, sign1, 1, integer_static_div_algo<static_int<SSize>>{});
    if (integer_static_div_algo<static_int<SSize>>::value == 0) {
        // If we used the mpn functions, zero the unused limbs on top (if necessary).
        // NOTE: as usual, potential of mpn use on optimised size.
        q.zero_unused_limbs();
    }
}
} // namespace detail

// Exact division with positive divisor (ternary version).
template <std::size_t SSize>
inline integer<SSize> &divexact_gcd(integer<SSize> &rop, const integer<SSize> &n, const integer<SSize> &d)
{
    assert(d.sgn() > 0);
    const bool sr = rop.is_static(), s1 = n.is_static(), s2 = d.is_static();
    if (mppp_likely(s1 && s2)) {
        if (!sr) {
            rop.set_zero();
        }
        static_divexact_gcd(rop._get_union().g_st(), n._get_union().g_st(), d._get_union().g_st());
        // Division can never fail.
        return rop;
    }
    if (sr) {
        rop._get_union().promote();
    }
    // NOTE: there's no public mpz_divexact_gcd() function in GMP, just use
    // mpz_divexact() directly.
    mpz_divexact(&rop._get_union().g_dy(), n.get_mpz_view(), d.get_mpz_view());
    return rop;
}

// Exact division with positive divisor (binary version).
template <std::size_t SSize>
inline integer<SSize> divexact_gcd(const integer<SSize> &n, const integer<SSize> &d)
{
    integer<SSize> retval;
    divexact_gcd(retval, n, d);
    return retval;
}

namespace detail
{

// mpn implementation.
template <std::size_t SSize>
inline void static_tdiv_q_2exp(static_int<SSize> &rop, const static_int<SSize> &n, ::mp_bitcnt_t s)
{
    static_assert(SSize > static_int<SSize>::opt_size, "Cannot use mpn functions for optimised static size.");
    mpz_size_t asize = n._mp_size;
    if (s == 0u || asize == 0) {
        // If shift is zero, or the operand is zero, write n into rop and return.
        rop = n;
        return;
    }
    // Finish setting up asize and sign.
    int sign = 1;
    if (asize < 0) {
        asize = -asize;
        sign = -1;
    }
    // ls: number of entire limbs shifted.
    // rs: effective shift that will be passed to the mpn function.
    const auto ls = s / unsigned(GMP_NUMB_BITS), rs = s % unsigned(GMP_NUMB_BITS);
    if (ls >= static_cast<std::size_t>(asize)) {
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
        mpn_rshift(rop.m_limbs.data(), n.m_limbs.data() + ls, static_cast<::mp_size_t>(new_asize), unsigned(rs));
    } else {
        // Otherwise, just copy over (the mpn function requires the shift to be at least 1).
        // NOTE: std::move requires that the destination iterator is not within the input range.
        // Here we are sure that ls > 0 because we don't have a remainder, and the zero shift case
        // was already handled above.
        assert(ls);
        std::move(n.m_limbs.begin() + ls, n.m_limbs.begin() + asize, rop.m_limbs.begin());
    }
    // Set the size. We need to check if the top limb is zero.
    rop._mp_size = sign * (new_asize - ((rop.m_limbs[static_cast<std::size_t>(new_asize - 1)] & GMP_NUMB_MASK) == 0u));
}

// 1-limb optimisation.
inline void static_tdiv_q_2exp(static_int<1> &rop, const static_int<1> &n, ::mp_bitcnt_t s)
{
    const ::mp_limb_t l = n.m_limbs[0] & GMP_NUMB_MASK;
    if (s == 0u || l == 0u) {
        rop = n;
        return;
    }
    if (s >= unsigned(GMP_NUMB_BITS)) {
        // We are shifting by the limb's bit size or greater, the result will be zero.
        rop._mp_size = 0;
        rop.m_limbs[0] = 0u;
        return;
    }
    // Compute the result.
    const auto res = l >> s;
    // Size is the original one or zero.
    rop._mp_size = static_cast<int>(res != 0u) * n._mp_size;
    rop.m_limbs[0] = res;
}

// 2-limb optimisation.
inline void static_tdiv_q_2exp(static_int<2> &rop, const static_int<2> &n, ::mp_bitcnt_t s)
{
    mpz_size_t asize = n._mp_size;
    if (s == 0u || asize == 0) {
        rop = n;
        return;
    }
    int sign = 1;
    if (asize < 0) {
        asize = -asize;
        sign = -1;
    }
    // If shift is too large, zero the result and return.
    static_assert(unsigned(GMP_NUMB_BITS) <= nl_max<unsigned>() / 2u, "Overflow error.");
    if (s >= 2u * unsigned(GMP_NUMB_BITS)) {
        rop._mp_size = 0;
        rop.m_limbs[0u] = 0u;
        rop.m_limbs[1u] = 0u;
        return;
    }
    if (s >= unsigned(GMP_NUMB_BITS)) {
        // NOTE: here the effective shift < GMP_NUMB_BITS, otherwise it would have been caught
        // in the check above.
        const auto lo = (n.m_limbs[1u] & GMP_NUMB_MASK) >> (s - unsigned(GMP_NUMB_BITS));
        // The size could be zero or +-1, depending
        // on the new content of m_limbs[0] and the previous
        // sign of _mp_size.
        rop._mp_size = static_cast<int>(lo != 0u) * sign;
        rop.m_limbs[0u] = lo;
        rop.m_limbs[1u] = 0u;
        return;
    }
    assert(s > 0u && s < unsigned(GMP_NUMB_BITS));
    // This represents the bits in hi that will be shifted down into lo.
    // We move them up so we can add tmp to the new lo to account for them.
    // NOTE: mask the final result to avoid spillover into potential nail bits.
    const auto tmp = ((n.m_limbs[1u] & GMP_NUMB_MASK) << (unsigned(GMP_NUMB_BITS) - s)) & GMP_NUMB_MASK;
    rop.m_limbs[0u] = ((n.m_limbs[0u] & GMP_NUMB_MASK) >> s) + tmp;
    rop.m_limbs[1u] = (n.m_limbs[1u] & GMP_NUMB_MASK) >> s;
    // The effective shift was less than 1 entire limb. The new asize must be the old one,
    // or one less than that.
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    rop._mp_size = sign * (asize - ((rop.m_limbs[std::size_t(asize - 1)] & GMP_NUMB_MASK) == 0u));
}
} // namespace detail

// Ternary right shift.
template <std::size_t SSize>
inline integer<SSize> &tdiv_q_2exp(integer<SSize> &rop, const integer<SSize> &n, ::mp_bitcnt_t s)
{
    const bool sn = n.is_static(), sr = rop.is_static();
    if (mppp_likely(sn)) {
        if (!sr) {
            rop.set_zero();
        }
        static_tdiv_q_2exp(rop._get_union().g_st(), n._get_union().g_st(), s);
        return rop;
    }
    if (sr) {
        rop._get_union().promote();
    }
    mpz_tdiv_q_2exp(&rop._get_union().g_dy(), n.get_mpz_view(), s);
    return rop;
}

namespace detail
{

// mpn implementation.
template <std::size_t SSize>
inline int static_cmp(const static_int<SSize> &n1, const static_int<SSize> &n2)
{
    if (n1._mp_size < n2._mp_size) {
        return -1;
    }
    if (n2._mp_size < n1._mp_size) {
        return 1;
    }
    // The two sizes are equal, compare the absolute values.
    const auto asize = n1._mp_size >= 0 ? n1._mp_size : -n1._mp_size;
    if (asize) {
        // NOTE: reduce the value of the comparison to the [-1, 1] range, so that
        // if we need to negate it below we ensure not to run into overflows.
        const int cmp_abs
            = integral_sign(mpn_cmp(n1.m_limbs.data(), n2.m_limbs.data(), static_cast<::mp_size_t>(asize)));
        // If the values are non-negative, return the comparison of the absolute values, otherwise invert it.
        return (n1._mp_size >= 0) ? cmp_abs : -cmp_abs;
    }
    // Both operands are zero.
    // NOTE: we do this special casing in order to avoid calling mpn_cmp() on zero operands. It seems to
    // work, but the official GMP docs say one is not supposed to call mpn functions on zero operands.
    return 0;
}

// 1-limb optimisation.
inline int static_cmp(const static_int<1> &n1, const static_int<1> &n2)
{
    if (n1._mp_size < n2._mp_size) {
        return -1;
    }
    if (n2._mp_size < n1._mp_size) {
        return 1;
    }
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    int cmp_abs = (n1.m_limbs[0u] & GMP_NUMB_MASK) > (n2.m_limbs[0u] & GMP_NUMB_MASK);
    if (cmp_abs == 0) {
        cmp_abs = -static_cast<int>((n1.m_limbs[0u] & GMP_NUMB_MASK) < (n2.m_limbs[0u] & GMP_NUMB_MASK));
    }
    return (n1._mp_size >= 0) ? cmp_abs : -cmp_abs;
}

// 2-limb optimisation.
inline int static_cmp(const static_int<2> &n1, const static_int<2> &n2)
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
} // namespace detail

// Comparison function.
template <std::size_t SSize>
inline int cmp(const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool s1 = op1.is_static(), s2 = op2.is_static();
    if (mppp_likely(s1 && s2)) {
        return static_cmp(op1._get_union().g_st(), op2._get_union().g_st());
    }
    return mpz_cmp(op1.get_mpz_view(), op2.get_mpz_view());
}

// Sign function.
template <std::size_t SSize>
inline int sgn(const integer<SSize> &n)
{
    return n.sgn();
}

// Test if integer is odd.
template <std::size_t SSize>
inline bool odd_p(const integer<SSize> &n)
{
    return n.odd_p();
}

// Test if integer is even.
template <std::size_t SSize>
inline bool even_p(const integer<SSize> &n)
{
    return n.even_p();
}

// Test if an integer is zero.
template <std::size_t SSize>
inline bool is_zero(const integer<SSize> &n)
{
    return n.is_zero();
}

// Test if an integer is equal to one.
template <std::size_t SSize>
inline bool is_one(const integer<SSize> &n)
{
    return n.is_one();
}

// Test if an integer is equal to minus one.
template <std::size_t SSize>
inline bool is_negative_one(const integer<SSize> &n)
{
    return n.is_negative_one();
}

namespace detail
{

// 1-limb implementation.
inline bool static_not_impl(static_int<1> &rop, const static_int<1> &op, mpz_size_t, int sign)
{
    const ::mp_limb_t l = op.m_limbs[0] & GMP_NUMB_MASK;
    if (sign >= 0) {
        if (mppp_unlikely(l == GMP_NUMB_MAX)) {
            // NOTting would overflow the limb.
            return false;
        }
        rop._mp_size = -1;
        rop.m_limbs[0] = l + 1u;
        return true;
    }
    // Size can be zero or 1.
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    rop._mp_size = l != 1u;
    rop.m_limbs[0] = l - 1u;
    return true;
}

// 2-limbs implementation.
inline bool static_not_impl(static_int<2> &rop, const static_int<2> &op, mpz_size_t, int sign)
{
    const ::mp_limb_t lo = op.m_limbs[0] & GMP_NUMB_MASK, hi = op.m_limbs[1] & GMP_NUMB_MASK;
    if (sign >= 0) {
        if (mppp_unlikely(lo == GMP_NUMB_MAX && hi == GMP_NUMB_MAX)) {
            // The static has the maximum possible value, NOT will overflow it.
            return false;
        }
        const ::mp_limb_t new_lo = (lo + 1u) & GMP_NUMB_MASK, new_hi = hi + unsigned(lo == GMP_NUMB_MAX);
        // asize can be -1 or -2, never zero.
        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        rop._mp_size = -1 - (new_hi != 0u);
        rop.m_limbs[0] = new_lo;
        rop.m_limbs[1] = new_hi;
        return true;
    }
    const ::mp_limb_t new_lo = (lo - 1u) & GMP_NUMB_MASK, new_hi = hi - unsigned(lo == 0u);
    // Size could be 0, 1 or 2.
    rop._mp_size = size_from_lohi(new_lo, new_hi);
    rop.m_limbs[0] = new_lo;
    rop.m_limbs[1] = new_hi;
    return true;
}

// n-limbs implementation.
template <std::size_t SSize>
inline bool static_not_impl(static_int<SSize> &rop, const static_int<SSize> &op, mpz_size_t asize, int sign)
{
    auto data = op.m_limbs.data();
    if (sign >= 0) {
        if (mppp_unlikely(static_cast<mpz_size_t>(SSize) == asize && std::all_of(data, data + asize, [](::mp_limb_t l) {
                              return (l & GMP_NUMB_MASK) == GMP_NUMB_MAX;
                          }))) {
            // If the current size is the max possible static size, and the value
            // is the max possible value, then we will overflow.
            return false;
        }
        if (sign) {
            const auto cy
                = static_cast<mpz_size_t>(mpn_add_1(rop.m_limbs.data(), data, static_cast<::mp_size_t>(asize), 1));
            if (cy) {
                // If there's a carry, we'll need to write into the upper limb.
                assert(asize < static_cast<mpz_size_t>(SSize));
                rop.m_limbs[static_cast<std::size_t>(asize)] = 1;
            }
            rop._mp_size = -asize - cy;
        } else {
            // Special case zero, as mpn functions don't want zero operands.
            rop.m_limbs[0] = 1;
            rop._mp_size = -1;
        }
        return true;
    }
    mpn_sub_1(rop.m_limbs.data(), data, static_cast<::mp_size_t>(asize), 1);
    // Size will be the original asize minus possibly 1, if the original topmost limb
    // has become zero.
    rop._mp_size
        = asize - static_cast<mpz_size_t>((rop.m_limbs[static_cast<std::size_t>(asize - 1)] & GMP_NUMB_MASK) == 0u);
    return true;
}

template <std::size_t SSize>
inline bool static_not(static_int<SSize> &rop, const static_int<SSize> &op)
{
    mpz_size_t asize = op._mp_size;
    int sign = asize != 0;
    if (asize < 0) {
        asize = -asize;
        sign = -1;
    }
    // NOTE: currently the implementation dispatches only on the static size: there's no need to
    // zero upper limbs as mpn functions are never used in case of optimised size.
    return static_not_impl(rop, op, asize, sign);
}
} // namespace detail

// Bitwise NOT.
template <std::size_t SSize>
inline integer<SSize> &bitwise_not(integer<SSize> &rop, const integer<SSize> &op)
{
    bool sr = rop.is_static();
    const bool s = op.is_static();
    if (mppp_likely(s)) {
        if (!sr) {
            rop.set_zero();
            sr = true;
        }
        if (mppp_likely(static_not(rop._get_union().g_st(), op._get_union().g_st()))) {
            return rop;
        }
    }
    if (sr) {
        rop._get_union().promote();
    }
    mpz_com(&rop._get_union().g_dy(), op.get_mpz_view());
    return rop;
}

namespace detail
{

// 1-limb implementation.
inline void static_ior_impl(static_int<1> &rop, const static_int<1> &op1, const static_int<1> &op2, mpz_size_t,
                            mpz_size_t, int sign1, int sign2)
{
    const ::mp_limb_t l1 = op1.m_limbs[0] & GMP_NUMB_MASK, l2 = op2.m_limbs[0] & GMP_NUMB_MASK;
    if (sign1 >= 0 && sign2 >= 0) {
        // The easy case: both are nonnegative.
        // NOTE: no need to mask, as we masked l1 and l2 already, and OR
        // won't turn on any upper bit.
        ::mp_limb_t ret = l1 | l2;
        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        rop._mp_size = ret != 0u;
        rop.m_limbs[0] = ret;
        return;
    }
    // NOTE: my understanding is the following:
    // - the size of the operands is considered to be their real bit size, rounded up to GMP_NUMB_BITS,
    //   plus a phantom sign bit on top due to the fake two's complement representation for negative numbers.
    //   We never need to represent explicitly this fake bit;
    // - the size of the result will be the max operand size.
    // For instance, for ORing 25 and -5 we start like this:
    // (0) ... 0 1 1 0 0 1 | <-- 25 (5 "real bits", plus a fake bit on top at index GMP_NUMB_BITS)
    // (0) ... 0 0 0 1 0 1   <-- 5
    // We do the two's complement of 5 to produce -5:
    // (0) ... 0 1 1 0 0 1 | <-- 25
    // (1) ... 1 1 1 0 1 1   <-- -5 (in two's complement)
    // ---------------
    // (1) ... 1 1 1 0 1 1
    // The result has the sign bit set, thus it's a negative number. We take again
    // the two's complement to get its absolute value:
    // (0) ... 0 0 0 1 0 1
    // So the final result is -5.
    const unsigned sign_mask = unsigned(sign1 < 0) + (unsigned(sign2 < 0) << 1);
    // NOTE: at least 1 of the operands is strictly negative, so the result
    // has to be negative as well (because of ORing the sign bits).
    rop._mp_size = -1;
    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
    switch (sign_mask) {
        case 1u:
            // op1 negative, op2 nonnegative.
            // NOTE: here we know that the values we are 2sc-ing are not zero
            // (they are strictly negative). The complement will turn on
            // the nail bits, which will remain turned on after the ORing.
            // The final 2sc will set them back to zero, so no need to mask.
            rop.m_limbs[0] = ~((~l1 + 1u) | l2) + 1u;
            break;
        case 2u:
            // op1 nonnegative, op2 negative.
            rop.m_limbs[0] = ~((~l2 + 1u) | l1) + 1u;
            break;
        case 3u:
            // Both negative.
            rop.m_limbs[0] = ~((~l1 + 1u) | (~l2 + 1u)) + 1u;
            break;
    }
}

// Small helper to compute the two's complement of a nonzero 2-limbs static integer.
inline void twosc(std::array<::mp_limb_t, 2> &arr, ::mp_limb_t lo, ::mp_limb_t hi)
{
    assert(hi != 0u || lo != 0u);
    arr[0] = (~lo + 1u) & GMP_NUMB_MASK;
    arr[1] = (~hi + unsigned(lo == 0u)) & GMP_NUMB_MASK;
}

// 2-limbs implementation.
inline void static_ior_impl(static_int<2> &rop, const static_int<2> &op1, const static_int<2> &op2, mpz_size_t,
                            mpz_size_t, int sign1, int sign2)
{
    const ::mp_limb_t lo1 = (op1.m_limbs[0] & GMP_NUMB_MASK), hi1 = (op1.m_limbs[1] & GMP_NUMB_MASK),
                      lo2 = (op2.m_limbs[0] & GMP_NUMB_MASK), hi2 = (op2.m_limbs[1] & GMP_NUMB_MASK);
    if (sign1 >= 0 && sign2 >= 0) {
        // The easy case: both are nonnegative.
        const ::mp_limb_t lo = lo1 | lo2, hi = hi1 | hi2;
        rop._mp_size = size_from_lohi(lo, hi);
        rop.m_limbs[0] = lo;
        rop.m_limbs[1] = hi;
        return;
    }
    const unsigned sign_mask = unsigned(sign1 < 0) + (unsigned(sign2 < 0) << 1);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, 2> tmp1, tmp2;
    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
    switch (sign_mask) {
        case 1u:
            // op1 negative, op2 nonnegative.
            twosc(tmp1, lo1, hi1);
            // NOTE: here lo2, hi2 and the 2 limbs in tmp1 have already
            // been masked for nail bits.
            twosc(rop.m_limbs, tmp1[0] | lo2, tmp1[1] | hi2);
            break;
        case 2u:
            // op1 nonnegative, op2 negative.
            twosc(tmp2, lo2, hi2);
            twosc(rop.m_limbs, tmp2[0] | lo1, tmp2[1] | hi1);
            break;
        case 3u:
            // Both negative.
            twosc(tmp1, lo1, hi1);
            twosc(tmp2, lo2, hi2);
            twosc(rop.m_limbs, tmp1[0] | tmp2[0], tmp1[1] | tmp2[1]);
            break;
    }
    // Size is -1 or -2: we could have 1-limb operands, and even with
    // 2-limbs operands, the final 2sc - in case of negative values - could
    // zero out the upper limb.
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    rop._mp_size = -2 + (rop.m_limbs[1] == 0u);
}

// Compute the two's complement of an n-limbs nonzero integer. The new size of
// the integer will be returned.
inline mpz_size_t twosc(::mp_limb_t *rop, const ::mp_limb_t *sp, mpz_size_t n)
{
    // Make sure the size is nonzero, and the value is nonzero as well.
    assert(n > 0);
    assert(std::any_of(sp, sp + n, [](::mp_limb_t l) { return (l & GMP_NUMB_MASK) != 0u; }));
    // Create a copy so we can compare to the original value later.
    auto size = n;
    // Flip the bits.
    mpn_com(rop, sp, static_cast<::mp_size_t>(size));
    // Compute the new size.
    if ((rop[size - 1] & GMP_NUMB_MASK) == 0u) {
        --size;
        for (; size != 0 && (rop[size - 1] & GMP_NUMB_MASK) == 0u; --size) {
        }
    }
    // Add 1.
    if (size != 0) {
        // If rop is nonzero, use the mpn_add_1() primitive, storing the carry
        // and updating the size if necessary.
        if (mpn_add_1(rop, rop, size, 1) != 0u) {
            // This needs to hold as sp is nonzero: 2sc can never
            // overflow the highest limb.
            assert(size < n);
            rop[size++] = 1;
        }
    } else {
        // If rop is zero, we cannot use mpn functions, just set the value directly.
        rop[0] = 1;
        size = 1;
    }
    // NOTE: the new size cannot be greater than n, the only way for this
    // to be possible would be if the input was zero (but this is prevented)
    // by the top assert).
    assert(size <= n);
    return size;
}

// mpn implementation.
template <std::size_t SSize>
inline void static_ior_impl(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2,
                            mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2)
{
    auto data1 = op1.m_limbs.data(), data2 = op2.m_limbs.data();
    // Handle zeroes.
    if (!sign1) {
        // NOTE: manual copy rather than assignment, to avoid
        // a few branches.
        rop._mp_size = op2._mp_size;
        copy_limbs(data2, data2 + asize2, rop.m_limbs.data());
        return;
    }
    if (!sign2) {
        rop._mp_size = op1._mp_size;
        copy_limbs(data1, data1 + asize1, rop.m_limbs.data());
        return;
    }
    // Make sure data1/asize1 refer to the largest operand.
    if (asize1 < asize2) {
        std::swap(data1, data2);
        std::swap(asize1, asize2);
        std::swap(sign1, sign2);
    }
    if (sign1 > 0 && sign2 > 0) {
        // The easy case: both are nonnegative.
        // Set the size.
        rop._mp_size = asize1;
        // Compute the ior.
        mpn_ior_n(rop.m_limbs.data(), data1, data2, static_cast<::mp_size_t>(asize2));
        // Copy extra limbs from data1.
        copy_limbs(data1 + asize2, data1 + asize1, rop.m_limbs.data() + asize2);
        return;
    }
    const unsigned sign_mask = unsigned(sign1 < 0) + (unsigned(sign2 < 0) << 1);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, SSize> tmp1, tmp2;
    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
    switch (sign_mask) {
        case 1u:
            // op1 negative, op2 nonnegative.
            twosc(tmp1.data(), data1, asize1);
            // NOTE: in all 3 cases, the mpn_ior_n() is done with the minimum size among the operands
            // (asize2). In this case, due to the twosc, the first most significant limbs in tmp1 might
            // be zero, but according to the mpn docs this is not a problem.
            mpn_ior_n(rop.m_limbs.data(), tmp1.data(), data2, static_cast<::mp_size_t>(asize2));
            // Copy over the remaining limbs from the largest operand.
            copy_limbs(tmp1.data() + asize2, tmp1.data() + asize1, rop.m_limbs.data() + asize2);
            // The final twosc. This will return the effective abs size, which we need to negate.
            rop._mp_size = -twosc(rop.m_limbs.data(), rop.m_limbs.data(), asize1);
            break;
        case 2u:
            // op1 nonnegative, op2 negative.
            twosc(tmp2.data(), data2, asize2);
            // NOTE: after the twosc, the limbs in tmp2 from asize2 to asize1 should be set
            // to all 1s. We don't need to actually do that: ORing op1 with these high all-1 limbs
            // produces all-1 limbs in the result, and the final twosc will flip them back to zero.
            mpn_ior_n(rop.m_limbs.data(), data1, tmp2.data(), static_cast<::mp_size_t>(asize2));
            rop._mp_size = -twosc(rop.m_limbs.data(), rop.m_limbs.data(), asize2);
            break;
        case 3u:
            twosc(tmp1.data(), data1, asize1);
            twosc(tmp2.data(), data2, asize2);
            mpn_ior_n(rop.m_limbs.data(), tmp1.data(), tmp2.data(), static_cast<::mp_size_t>(asize2));
            rop._mp_size = -twosc(rop.m_limbs.data(), rop.m_limbs.data(), asize2);
            break;
    }
}

// The dispatching function for the static implementation.
template <std::size_t SSize>
inline void static_ior(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2)
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
    // NOTE: currently the implementation dispatches only on the static size: there's no need to
    // zero upper limbs as mpn functions are never used in case of optimised size.
    static_ior_impl(rop, op1, op2, asize1, asize2, sign1, sign2);
}
} // namespace detail

// Bitwise OR.
template <std::size_t SSize>
inline integer<SSize> &bitwise_ior(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool sr = rop.is_static(), s1 = op1.is_static(), s2 = op2.is_static();
    if (mppp_likely(s1 && s2)) {
        if (!sr) {
            rop.set_zero();
        }
        static_ior(rop._get_union().g_st(), op1._get_union().g_st(), op2._get_union().g_st());
        return rop;
    }
    if (sr) {
        rop._get_union().promote();
    }
    mpz_ior(&rop._get_union().g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    return rop;
}

namespace detail
{

// 1-limb implementation.
inline bool static_and_impl(static_int<1> &rop, const static_int<1> &op1, const static_int<1> &op2, mpz_size_t,
                            mpz_size_t, int sign1, int sign2)
{
    const ::mp_limb_t l1 = op1.m_limbs[0] & GMP_NUMB_MASK, l2 = op2.m_limbs[0] & GMP_NUMB_MASK;
    if (sign1 >= 0 && sign2 >= 0) {
        // The easy case: both are nonnegative.
        // NOTE: no need to mask, as we masked l1 and l2 already, and AND
        // won't turn on any upper bit.
        ::mp_limb_t ret = l1 & l2;
        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        rop._mp_size = ret != 0u;
        rop.m_limbs[0] = ret;
        return true;
    }
    const unsigned sign_mask = unsigned(sign1 < 0) + (unsigned(sign2 < 0) << 1);
    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
    switch (sign_mask) {
        case 1u: {
            // op1 negative, op2 nonnegative.
            // The result will be nonnegative, and it does not need to be masked:
            // nail bits will be switched off by ANDing with l2.
            const ::mp_limb_t ret = (~l1 + 1u) & l2;
            // NOLINTNEXTLINE(readability-implicit-bool-conversion)
            rop._mp_size = ret != 0u;
            rop.m_limbs[0] = ret;
            return true;
        }
        case 2u: {
            // op1 nonnegative, op2 negative.
            // This is the symmetric of above.
            const ::mp_limb_t ret = l1 & (~l2 + 1u);
            // NOLINTNEXTLINE(readability-implicit-bool-conversion)
            rop._mp_size = ret != 0u;
            rop.m_limbs[0] = ret;
            return true;
        }
        case 3u: {
            // Both negative. In this case, the result will be negative,
            // unless the ANDing of the 2scs results in zero: in that
            // case, we return failure as we need extra storage for the result.
            ::mp_limb_t ret = (~l1 + 1u) & (~l2 + 1u);
            // NOTE: need to mask here, as nail bits will have been flipped
            // by the NOTing above.
            if (mppp_unlikely(!(ret & GMP_NUMB_MASK))) {
                return false;
            }
            // The NOT here will flip back the nail bits, no need to mask.
            ret = ~ret + 1u;
            // NOLINTNEXTLINE(readability-implicit-bool-conversion)
            rop._mp_size = -(ret != 0u);
            rop.m_limbs[0] = ret;
            return true;
        }
    }
    // Keep the compiler happy.
    // LCOV_EXCL_START
    assert(false);
    return true;
    // LCOV_EXCL_STOP
}

// 2-limbs implementation.
inline bool static_and_impl(static_int<2> &rop, const static_int<2> &op1, const static_int<2> &op2, mpz_size_t,
                            mpz_size_t, int sign1, int sign2)
{
    const ::mp_limb_t lo1 = (op1.m_limbs[0] & GMP_NUMB_MASK), hi1 = (op1.m_limbs[1] & GMP_NUMB_MASK),
                      lo2 = (op2.m_limbs[0] & GMP_NUMB_MASK), hi2 = (op2.m_limbs[1] & GMP_NUMB_MASK);
    if (sign1 >= 0 && sign2 >= 0) {
        // The easy case: both are nonnegative.
        const ::mp_limb_t lo = lo1 & lo2, hi = hi1 & hi2;
        rop._mp_size = size_from_lohi(lo, hi);
        rop.m_limbs[0] = lo;
        rop.m_limbs[1] = hi;
        return true;
    }
    const unsigned sign_mask = unsigned(sign1 < 0) + (unsigned(sign2 < 0) << 1);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, 2> tmp1, tmp2;
    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
    switch (sign_mask) {
        case 1u:
            // op1 negative, op2 nonnegative.
            twosc(tmp1, lo1, hi1);
            // NOTE: here lo2, hi2 and the 2 limbs in tmp1 have already
            // been masked for nail bits.
            rop.m_limbs[0] = tmp1[0] & lo2;
            rop.m_limbs[1] = tmp1[1] & hi2;
            rop._mp_size = size_from_lohi(rop.m_limbs[0], rop.m_limbs[1]);
            return true;
        case 2u:
            // op1 nonnegative, op2 negative.
            twosc(tmp2, lo2, hi2);
            rop.m_limbs[0] = tmp2[0] & lo1;
            rop.m_limbs[1] = tmp2[1] & hi1;
            rop._mp_size = size_from_lohi(rop.m_limbs[0], rop.m_limbs[1]);
            return true;
        case 3u: {
            // Both negative.
            twosc(tmp1, lo1, hi1);
            twosc(tmp2, lo2, hi2);
            const ::mp_limb_t new_lo = tmp1[0] & tmp2[0], new_hi = tmp1[1] & tmp2[1];
            if (mppp_unlikely(!new_lo && !new_hi)) {
                // When both operands are negative, we could end up in a situation
                // where, after the 2scs, the ANDing returns zero. In this case,
                // we need extra storage.
                return false;
            }
            twosc(rop.m_limbs, new_lo, new_hi);
            rop._mp_size = -size_from_lohi(rop.m_limbs[0], rop.m_limbs[1]);
            return true;
        }
    }
    // LCOV_EXCL_START
    assert(false);
    return true;
    // LCOV_EXCL_STOP
}

// Small helper to compute the abs size of a static int, knowing it must
// be at most asize > 0.
template <std::size_t SSize>
inline mpz_size_t compute_static_int_asize(const static_int<SSize> &r, mpz_size_t asize)
{
    assert(asize > 0);
    if (!(r.m_limbs[static_cast<std::size_t>(asize - 1)] & GMP_NUMB_MASK)) {
        --asize;
        for (; asize && !(r.m_limbs[static_cast<std::size_t>(asize - 1)] & GMP_NUMB_MASK); --asize) {
        }
    }
    return asize;
}

// mpn implementation.
template <std::size_t SSize>
inline bool static_and_impl(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2,
                            mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2)
{
    auto data1 = op1.m_limbs.data(), data2 = op2.m_limbs.data();
    // Handle zeroes.
    if (!sign1 || !sign2) {
        rop._mp_size = 0;
        return true;
    }
    // Make sure data1/asize1 refer to the largest operand.
    if (asize1 < asize2) {
        std::swap(data1, data2);
        std::swap(asize1, asize2);
        std::swap(sign1, sign2);
    }
    if (sign1 > 0 && sign2 > 0) {
        // The easy case: both are nonnegative.
        // Compute the and.
        mpn_and_n(rop.m_limbs.data(), data1, data2, static_cast<::mp_size_t>(asize2));
        // The asize will be at most asize2. Upper limbs could be zero due to the ANDing.
        rop._mp_size = compute_static_int_asize(rop, asize2);
        return true;
    }
    const unsigned sign_mask = unsigned(sign1 < 0) + (unsigned(sign2 < 0) << 1);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, SSize> tmp1, tmp2, tmpr;
    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
    switch (sign_mask) {
        case 1u:
            // op1 negative, op2 nonnegative.
            twosc(tmp1.data(), data1, asize1);
            // NOTE: in all 3 cases, the mpn_and_n() is done with the minimum size among the operands
            // (asize2). In this case, due to the twosc, the first most significant limbs in tmp1 might
            // be zero, but according to the mpn docs this is not a problem.
            mpn_and_n(rop.m_limbs.data(), tmp1.data(), data2, static_cast<::mp_size_t>(asize2));
            // NOTE: size cannot be larger than asize2, as all the limbs above that limit from op1
            // will be set to zero by the ANDing.
            rop._mp_size = compute_static_int_asize(rop, asize2);
            return true;
        case 2u:
            // op1 nonnegative, op2 negative.
            twosc(tmp2.data(), data2, asize2);
            // Do the AND.
            mpn_and_n(rop.m_limbs.data(), data1, tmp2.data(), static_cast<::mp_size_t>(asize2));
            // Copy over the upper limbs of op1 to rop: the limbs in tmp2 from asize2 to asize1
            // are (virtually) set to all 1s by the twosc, so ANDing with the corresponding limbs
            // in op1 means simply copying op1 over.
            copy_limbs(data1 + asize2, data1 + asize1, rop.m_limbs.data() + asize2);
            // Compute the final size. It can be at most asize1.
            rop._mp_size = compute_static_int_asize(rop, asize1);
            return true;
        case 3u:
            twosc(tmp1.data(), data1, asize1);
            twosc(tmp2.data(), data2, asize2);
            // Write in temp storage, as we might overflow and we don't want to spoil
            // rop in that case.
            mpn_and_n(tmpr.data(), tmp1.data(), tmp2.data(), static_cast<::mp_size_t>(asize2));
            // Copy over the upper limbs of op1 to rop (same as above). Non overlapping,
            // as we are only using local storage.
            copy_limbs_no(tmp1.data() + asize2, tmp1.data() + asize1, tmpr.data() + asize2);
            // Check for zero.
            if (mppp_unlikely(std::all_of(tmpr.data(), tmpr.data() + asize1,
                                          [](::mp_limb_t l) { return (l & GMP_NUMB_MASK) == 0u; }))) {
                return false;
            }
            rop._mp_size = -twosc(rop.m_limbs.data(), tmpr.data(), asize1);
            return true;
    }
    // LCOV_EXCL_START
    assert(false);
    return true;
    // LCOV_EXCL_STOP
}

template <std::size_t SSize>
inline bool static_and(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2)
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
    // NOTE: currently the implementation dispatches only on the static size: there's no need to
    // zero upper limbs as mpn functions are never used in case of optimised size.
    return static_and_impl(rop, op1, op2, asize1, asize2, sign1, sign2);
}
} // namespace detail

// Bitwise AND.
template <std::size_t SSize>
inline integer<SSize> &bitwise_and(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool s1 = op1.is_static(), s2 = op2.is_static();
    bool sr = rop.is_static();
    if (mppp_likely(s1 && s2)) {
        if (!sr) {
            rop.set_zero();
            sr = true;
        }
        if (mppp_likely(static_and(rop._get_union().g_st(), op1._get_union().g_st(), op2._get_union().g_st()))) {
            return rop;
        }
    }
    if (sr) {
        rop._get_union().promote();
    }
    mpz_and(&rop._get_union().g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    return rop;
}

namespace detail
{

// 1-limb implementation.
inline bool static_xor_impl(static_int<1> &rop, const static_int<1> &op1, const static_int<1> &op2, mpz_size_t,
                            mpz_size_t, int sign1, int sign2)
{
    const ::mp_limb_t l1 = op1.m_limbs[0] & GMP_NUMB_MASK, l2 = op2.m_limbs[0] & GMP_NUMB_MASK;
    if (sign1 >= 0 && sign2 >= 0) {
        // The easy case: both are nonnegative.
        // NOTE: no need to mask, as we masked l1 and l2 already, and XOR
        // won't turn on any upper bit.
        ::mp_limb_t ret = l1 ^ l2;
        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        rop._mp_size = ret != 0u;
        rop.m_limbs[0] = ret;
        return false;
    }
    const unsigned sign_mask = unsigned(sign1 < 0) + (unsigned(sign2 < 0) << 1);
    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
    switch (sign_mask) {
        case 1u: {
            // op1 negative, op2 nonnegative. In this case, the result will be negative,
            // unless the XORing results in zero: in that
            // case, we return failure as we need extra storage for the result.
            ::mp_limb_t ret = (~l1 + 1u) ^ l2;
            // NOTE: need to mask here, as nail bits will have been flipped
            // by the NOTing above.
            if (mppp_unlikely(!(ret & GMP_NUMB_MASK))) {
                return false;
            }
            // The NOT here will flip back the nail bits, no need to mask.
            ret = ~ret + 1u;
            // NOLINTNEXTLINE(readability-implicit-bool-conversion)
            rop._mp_size = -(ret != 0u);
            rop.m_limbs[0] = ret;
            return true;
        }
        case 2u: {
            // Specular of the above.
            ::mp_limb_t ret = l1 ^ (~l2 + 1u);
            if (mppp_unlikely(!(ret & GMP_NUMB_MASK))) {
                return false;
            }
            ret = ~ret + 1u;
            // NOLINTNEXTLINE(readability-implicit-bool-conversion)
            rop._mp_size = -(ret != 0u);
            rop.m_limbs[0] = ret;
            return true;
        }
        case 3u: {
            // Both negative: the result will be nonnegative.
            // NOTE: the XOR will zero the nail bits, no need to mask.
            const ::mp_limb_t ret = (~l1 + 1u) ^ (~l2 + 1u);
            // NOLINTNEXTLINE(readability-implicit-bool-conversion)
            rop._mp_size = (ret != 0u);
            rop.m_limbs[0] = ret;
            return true;
        }
    }
    // LCOV_EXCL_START
    assert(false);
    return true;
    // LCOV_EXCL_STOP
}

// 2-limbs implementation.
inline bool static_xor_impl(static_int<2> &rop, const static_int<2> &op1, const static_int<2> &op2, mpz_size_t,
                            mpz_size_t, int sign1, int sign2)
{
    const ::mp_limb_t lo1 = (op1.m_limbs[0] & GMP_NUMB_MASK), hi1 = (op1.m_limbs[1] & GMP_NUMB_MASK),
                      lo2 = (op2.m_limbs[0] & GMP_NUMB_MASK), hi2 = (op2.m_limbs[1] & GMP_NUMB_MASK);
    if (sign1 >= 0 && sign2 >= 0) {
        // The easy case: both are nonnegative.
        const ::mp_limb_t lo = lo1 ^ lo2, hi = hi1 ^ hi2;
        rop._mp_size = size_from_lohi(lo, hi);
        rop.m_limbs[0] = lo;
        rop.m_limbs[1] = hi;
        return true;
    }
    const unsigned sign_mask = unsigned(sign1 < 0) + (unsigned(sign2 < 0) << 1);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, 2> tmp1, tmp2;
    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
    switch (sign_mask) {
        case 1u: {
            // op1 negative, op2 nonnegative. Result will be negative, unless
            // it overflows.
            twosc(tmp1, lo1, hi1);
            // NOTE: here lo2, hi2 and the 2 limbs in tmp1 have already
            // been masked for nail bits. The XOR will not change nail bits.
            const ::mp_limb_t new_lo = tmp1[0] ^ lo2, new_hi = tmp1[1] ^ hi2;
            if (mppp_unlikely(!new_lo && !new_hi)) {
                return false;
            }
            twosc(rop.m_limbs, new_lo, new_hi);
            rop._mp_size = -size_from_lohi(rop.m_limbs[0], rop.m_limbs[1]);
            return true;
        }
        case 2u: {
            // Mirror of the above.
            twosc(tmp2, lo2, hi2);
            const ::mp_limb_t new_lo = tmp2[0] ^ lo1, new_hi = tmp2[1] ^ hi1;
            if (mppp_unlikely(!new_lo && !new_hi)) {
                return false;
            }
            twosc(rop.m_limbs, new_lo, new_hi);
            rop._mp_size = -size_from_lohi(rop.m_limbs[0], rop.m_limbs[1]);
            return true;
        }
        case 3u: {
            // Both negative.
            twosc(tmp1, lo1, hi1);
            twosc(tmp2, lo2, hi2);
            rop.m_limbs[0] = tmp1[0] ^ tmp2[0];
            rop.m_limbs[1] = tmp1[1] ^ tmp2[1];
            rop._mp_size = size_from_lohi(rop.m_limbs[0], rop.m_limbs[1]);
            return true;
        }
    }
    // LCOV_EXCL_START
    assert(false);
    return true;
    // LCOV_EXCL_STOP
}

// mpn implementation.
template <std::size_t SSize>
inline bool static_xor_impl(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2,
                            mpz_size_t asize1, mpz_size_t asize2, int sign1, int sign2)
{
    auto data1 = op1.m_limbs.data(), data2 = op2.m_limbs.data();
    // Handle zeroes.
    if (!sign1) {
        // NOTE: manual copy rather than assignment, to avoid
        // a few branches.
        rop._mp_size = op2._mp_size;
        copy_limbs(data2, data2 + asize2, rop.m_limbs.data());
        return true;
    }
    if (!sign2) {
        rop._mp_size = op1._mp_size;
        copy_limbs(data1, data1 + asize1, rop.m_limbs.data());
        return true;
    }
    // Make sure data1/asize1 refer to the largest operand.
    if (asize1 < asize2) {
        std::swap(data1, data2);
        std::swap(asize1, asize2);
        std::swap(sign1, sign2);
    }
    if (sign1 > 0 && sign2 > 0) {
        // The easy case: both are nonnegative.
        // Compute the xor.
        mpn_xor_n(rop.m_limbs.data(), data1, data2, static_cast<::mp_size_t>(asize2));
        // Limbs from asize2 to asize1 in op1 get copied as-is, as they are XORed with
        // zeroes from op2.
        copy_limbs(data1 + asize2, data1 + asize1, rop.m_limbs.data() + asize2);
        // The asize will be at most asize1. Upper limbs could be zero due to the XORing
        // (e.g., the values are identical).
        rop._mp_size = compute_static_int_asize(rop, asize1);
        return true;
    }
    const unsigned sign_mask = unsigned(sign1 < 0) + (unsigned(sign2 < 0) << 1);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<::mp_limb_t, SSize> tmp1, tmp2, tmpr;
    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
    switch (sign_mask) {
        case 1u:
            // op1 negative, op2 nonnegative.
            twosc(tmp1.data(), data1, asize1);
            // NOTE: in all 3 cases, the mpn_xor_n() is done with the minimum size among the operands
            // (asize2). In this case, due to the twosc, the first most significant limbs in tmp1 might
            // be zero, but according to the mpn docs this is not a problem.
            mpn_xor_n(tmpr.data(), tmp1.data(), data2, static_cast<::mp_size_t>(asize2));
            // Copy over the limbs in tmp1 from asize2 to asize1.
            copy_limbs_no(tmp1.data() + asize2, tmp1.data() + asize1, tmpr.data() + asize2);
            // Check for zero.
            if (mppp_unlikely(std::all_of(tmpr.data(), tmpr.data() + asize1,
                                          [](::mp_limb_t l) { return (l & GMP_NUMB_MASK) == 0u; }))) {
                return false;
            }
            rop._mp_size = -twosc(rop.m_limbs.data(), tmpr.data(), asize1);
            return true;
        case 2u:
            // op1 nonnegative, op2 negative.
            twosc(tmp2.data(), data2, asize2);
            // Do the XOR.
            mpn_xor_n(tmpr.data(), data1, tmp2.data(), static_cast<::mp_size_t>(asize2));
            // The limbs in tmp2 from asize2 to asize1 have all been set to 1: XORing them
            // with the corresponding limbs in op1 means bit-flipping the limbs in op1.
            if (asize2 != asize1) {
                // NOTE: mpn functions require nonzero operands, so we need to branch here.
                mpn_com(tmpr.data() + asize2, data1 + asize2, asize1 - asize2);
            }
            // Check for zero.
            if (mppp_unlikely(std::all_of(tmpr.data(), tmpr.data() + asize1,
                                          [](::mp_limb_t l) { return (l & GMP_NUMB_MASK) == 0u; }))) {
                return false;
            }
            rop._mp_size = -twosc(rop.m_limbs.data(), tmpr.data(), asize1);
            return true;
        case 3u:
            twosc(tmp1.data(), data1, asize1);
            twosc(tmp2.data(), data2, asize2);
            mpn_xor_n(rop.m_limbs.data(), tmp1.data(), tmp2.data(), static_cast<::mp_size_t>(asize2));
            // Same as above, regarding the all-1 limbs in tmp2.
            if (asize2 != asize1) {
                // NOTE: mpn functions require nonzero operands, so we need to branch here.
                mpn_com(rop.m_limbs.data() + asize2, tmp1.data() + asize2, asize1 - asize2);
            }
            rop._mp_size = compute_static_int_asize(rop, asize1);
            return true;
    }
    // LCOV_EXCL_START
    assert(false);
    return true;
    // LCOV_EXCL_STOP
}

template <std::size_t SSize>
inline bool static_xor(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2)
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
    // NOTE: currently the implementation dispatches only on the static size: there's no need to
    // zero upper limbs as mpn functions are never used in case of optimised size.
    return static_xor_impl(rop, op1, op2, asize1, asize2, sign1, sign2);
}
} // namespace detail

// Bitwise XOR.
template <std::size_t SSize>
inline integer<SSize> &bitwise_xor(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool s1 = op1.is_static(), s2 = op2.is_static();
    bool sr = rop.is_static();
    if (mppp_likely(s1 && s2)) {
        if (!sr) {
            rop.set_zero();
            sr = true;
        }
        if (mppp_likely(static_xor(rop._get_union().g_st(), op1._get_union().g_st(), op2._get_union().g_st()))) {
            return rop;
        }
    }
    if (sr) {
        rop._get_union().promote();
    }
    mpz_xor(&rop._get_union().g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    return rop;
}

namespace detail
{

// mpn/mpz implementation.
template <std::size_t SSize>
inline void static_gcd_impl(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2,
                            mpz_size_t asize1, mpz_size_t asize2)
{
    // NOTE: performance testing indicates that, even if mpz_gcd() does special casing
    // for zeroes and 1-limb values, it is still worth it to repeat the special casing
    // here. This is probably because if we manage to actually call mpn_gcd_1() here,
    // we avoid interacting with dynamically allocated memory below in the thread-local
    // object. If we ever start using mpn_gcd(), we will probably have to re-do a
    // performance analysis.
    //
    // Handle zeroes.
    if (!asize1) {
        // NOTE: we want the result to be positive, and to copy only the set limbs.
        rop._mp_size = asize2;
        copy_limbs(op2.m_limbs.data(), op2.m_limbs.data() + asize2, rop.m_limbs.data());
        return;
    }
    if (!asize2) {
        rop._mp_size = asize1;
        copy_limbs(op1.m_limbs.data(), op1.m_limbs.data() + asize1, rop.m_limbs.data());
        return;
    }
    // Special casing if an operand has asize 1.
    if (asize1 == 1) {
        rop._mp_size = 1;
        rop.m_limbs[0] = mpn_gcd_1(op2.m_limbs.data(), static_cast<::mp_size_t>(asize2), op1.m_limbs[0]);
        return;
    }
    if (asize2 == 1) {
        rop._mp_size = 1;
        rop.m_limbs[0] = mpn_gcd_1(op1.m_limbs.data(), static_cast<::mp_size_t>(asize1), op2.m_limbs[0]);
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
    const auto v1 = op1.get_mpz_view();
    const auto v2 = op2.get_mpz_view();
    mpz_gcd(&tmp.m_mpz, &v1, &v2);
    // Copy over.
    rop._mp_size = tmp.m_mpz._mp_size;
    assert(rop._mp_size > 0);
    copy_limbs_no(tmp.m_mpz._mp_d, tmp.m_mpz._mp_d + rop._mp_size, rop.m_limbs.data());
}

// 1-limb optimization.
inline void static_gcd_impl(static_int<1> &rop, const static_int<1> &op1, const static_int<1> &op2, mpz_size_t asize1,
                            mpz_size_t asize2)
{
    // Handle the special cases first.
    if (asize1 == 0) {
        // gcd(0, n) = abs(n). This also covers the convention
        // that gcd(0, 0) = 0.
        // NOTE: don't use the copy assignment operator of static_int,
        // as the size needs to be set to positive.
        rop._mp_size = asize2;
        rop.m_limbs = op2.m_limbs;
        return;
    }
    if (asize2 == 0) {
        // gcd(n, 0) = abs(n).
        // NOTE: op1 is not zero, its size can only be 1.
        rop._mp_size = 1;
        rop.m_limbs = op1.m_limbs;
        return;
    }
    // At this point, asize1 == asize2 == 1, and the result
    // will also have size 1.
    rop._mp_size = 1;
    // Compute the limb value.
    // NOTE: as an alternative, here we could have a custom binary GCD. See for
    // instance this implementation by Howard Hinnant using compiler intrinsics:
    // https://groups.google.com/a/isocpp.org/forum/#!topic/std-proposals/8WB2Z9d7A0w
    // Testing however shows that mpn_gcd_1() is quite well optimised, so let's
    // use it and keep in mind the option about the binary GCD for the future.
    // NOTE: at the commit c146af86e416cf1348d0b3fc454600b47b523f4f we have an implementation
    // of the binary GCD, just in case.
    // NOTE: currently, the binary GCD is faster on Zen processors, but I believe
    // that is because the current GMP version does not have optimised GCD assembly for Zen.
    // We need to benchmark again when the next GMP version comes out. If the binary GCD
    // is still faster, we should consider using it instead of mpn_gcd_1(), as even on Intel
    // processors the binary GCD is only marginally slower than mpn_gcd_1().
    rop.m_limbs[0] = mpn_gcd_1(op1.m_limbs.data(), static_cast<::mp_size_t>(1), op2.m_limbs[0]);
}

template <std::size_t SSize>
inline void static_gcd(static_int<SSize> &rop, const static_int<SSize> &op1, const static_int<SSize> &op2)
{
    static_gcd_impl(rop, op1, op2, std::abs(op1._mp_size), std::abs(op2._mp_size));
    if (SSize > 1u) {
        // If we used the generic function, zero the unused limbs on top (if necessary).
        // NOTE: as usual, potential of mpn/mpz use on optimised size (e.g., with 2-limb
        // static ints we are currently invoking mpz_gcd() - this could produce a result
        // with only the lower limb, and the higher limb is not zeroed out).
        rop.zero_unused_limbs();
    }
}
} // namespace detail

// GCD (ternary version).
template <std::size_t SSize>
inline integer<SSize> &gcd(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool sr = rop.is_static(), s1 = op1.is_static(), s2 = op2.is_static();
    if (mppp_likely(s1 && s2)) {
        if (!sr) {
            rop.set_zero();
        }
        static_gcd(rop._get_union().g_st(), op1._get_union().g_st(), op2._get_union().g_st());
        return rop;
    }
    if (sr) {
        rop._get_union().promote();
    }
    mpz_gcd(&rop._get_union().g_dy(), op1.get_mpz_view(), op2.get_mpz_view());
    return rop;
}

// GCD (binary version).
template <std::size_t SSize>
inline integer<SSize> gcd(const integer<SSize> &op1, const integer<SSize> &op2)
{
    integer<SSize> retval;
    gcd(retval, op1, op2);
    return retval;
}

namespace detail
{

// The general-purpose implementation of ternary LCM.
template <std::size_t SSize>
inline void integer_ternary_lcm_generic(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    // Temporary working variable.
    MPPP_MAYBE_TLS integer<SSize> g;

    // rop = (abs(op1) / gcd(op1, op2)) * abs(op2).
    gcd(g, op1, op2);
    divexact_gcd(g, op1, g);
    mul(g, g, op2);
    abs(rop, g);
}

template <std::size_t SSize>
inline void integer_ternary_lcm_impl(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    integer_ternary_lcm_generic(rop, op1, op2);
}

inline void integer_ternary_lcm_impl(integer<1> &rop, const integer<1> &op1, const integer<1> &op2)
{
    if (mppp_likely(op1.is_static() && op2.is_static())) {
        // NOTE: the idea here is that if both op1 and op2
        // are static, we can use static integer primitives
        // directly in parts of the algorithm, thus avoiding
        // branching wrt the general-purpose primitives.
        integer<1> g;

        auto &g_st = g._get_union().g_st();
        const auto &op1_st = op1._get_union().g_st();
        const auto &op2_st = op2._get_union().g_st();

        // Use the static primitives for the first two
        // steps, as we are sure that no overflow
        // is possible.
        static_gcd(g_st, op1_st, op2_st);
        static_divexact_gcd(g_st, op1_st, g_st);

        // Switch to the general-purpose multiplication,
        // as the result could overflow.
        // NOTE: perhaps using the static mul primitive
        // here could help us squeeze a bit extra performance,
        // at the price of duplicating the overflow handling
        // logic. Keep in mind for the future.
        mul(rop, g, op2);
        // Turn rop into abs(rop).
        // NOTE: this is always safe to do because
        // rop will consist of at most 2 limbs.
        rop._get_union().m_st._mp_size = std::abs(rop._get_union().m_st._mp_size);
    } else {
        // op1/op2 are not both statics. Run
        // the general-purpose implementation.
        integer_ternary_lcm_generic(rop, op1, op2);
    }
}

} // namespace detail

// LCM (ternary version).
template <std::size_t SSize>
inline integer<SSize> &lcm(integer<SSize> &rop, const integer<SSize> &op1, const integer<SSize> &op2)
{
    // Handle the special case lcm(0, 0) == 0.
    if (mppp_unlikely(op1.is_zero() && op2.is_zero())) {
        rop.set_zero();
    } else {
        detail::integer_ternary_lcm_impl(rop, op1, op2);
    }

    return rop;
}

namespace detail
{

// The general-purpose implementation of binary LCM.
template <std::size_t SSize>
inline integer<SSize> integer_binary_lcm_generic(const integer<SSize> &op1, const integer<SSize> &op2)
{
    // retval = (abs(op1) / gcd(op1, op2)) * abs(op2).
    auto retval = gcd(op1, op2);
    divexact_gcd(retval, op1, retval);
    mul(retval, retval, op2);
    abs(retval, retval);

    return retval;
}

template <std::size_t SSize>
inline integer<SSize> integer_binary_lcm_impl(const integer<SSize> &op1, const integer<SSize> &op2)
{
    return integer_binary_lcm_generic(op1, op2);
}

inline integer<1> integer_binary_lcm_impl(const integer<1> &op1, const integer<1> &op2)
{
    if (mppp_likely(op1.is_static() && op2.is_static())) {
        integer<1> retval;

        auto &r_st = retval._get_union().g_st();
        const auto &op1_st = op1._get_union().g_st();
        const auto &op2_st = op2._get_union().g_st();

        static_gcd(r_st, op1_st, op2_st);
        static_divexact_gcd(r_st, op1_st, r_st);

        mul(retval, retval, op2);
        // Turn retval into abs(retval).
        // NOTE: this is always safe to do because
        // rop will consist of at most 2 limbs.
        retval._get_union().m_st._mp_size = std::abs(retval._get_union().m_st._mp_size);

        return retval;
    } else {
        return integer_binary_lcm_generic(op1, op2);
    }
}

} // namespace detail

// LCM (binary version).
// NOTE: don't implement on top of the ternary primitives, a custom
// implementation avoids the creation of an unnecessary temporary.
template <std::size_t SSize>
inline integer<SSize> lcm(const integer<SSize> &op1, const integer<SSize> &op2)
{
    // Handle the special case lcm(0, 0) == 0.
    if (mppp_unlikely(op1.is_zero() && op2.is_zero())) {
        return integer<SSize>{};
    } else {
        return detail::integer_binary_lcm_impl(op1, op2);
    }
}

// Factorial.
template <std::size_t SSize>
inline integer<SSize> &fac_ui(integer<SSize> &rop, unsigned long n)
{
    // NOTE: we put a limit here because the GMP function just crashes and burns
    // if n is too large, and n does not even need to be that large.
    constexpr auto max_fac = 1000000ull;
    if (mppp_unlikely(n > max_fac)) {
        throw std::invalid_argument(
            "The value " + detail::to_string(n)
            + " is too large to be used as input for the factorial function (the maximum allowed value is "
            + detail::to_string(max_fac) + ")");
    }
    // NOTE: let's get through a static temporary and then assign it to the rop,
    // so that rop will be static/dynamic according to the size of tmp.
    MPPP_MAYBE_TLS detail::mpz_raii tmp;
    mpz_fac_ui(&tmp.m_mpz, n);
    return rop = &tmp.m_mpz;
}

// Binomial coefficient (ternary version).
template <std::size_t SSize>
inline integer<SSize> &bin_ui(integer<SSize> &rop, const integer<SSize> &n, unsigned long k)
{
    MPPP_MAYBE_TLS detail::mpz_raii tmp;
    mpz_bin_ui(&tmp.m_mpz, n.get_mpz_view(), k);
    return rop = &tmp.m_mpz;
}

// Binomial coefficient (binary version).
template <std::size_t SSize>
inline integer<SSize> bin_ui(const integer<SSize> &n, unsigned long k)
{
    integer<SSize> retval;
    bin_ui(retval, n, k);
    return retval;
}

namespace detail
{

// These helpers are used here and in pow() as well.
template <typename T, enable_if_t<is_integral<T>::value, int> = 0>
inline unsigned long integer_exp_to_ulong(const T &exp)
{
#if !defined(__INTEL_COMPILER)
    assert(exp >= T(0));
#endif
    // NOTE: make_unsigned_t<T> is T if T is already unsigned.
    // Don't use the make_unsigned() helper, as exp might be
    // unsigned already.
    static_assert(!std::is_same<T, bool>::value, "Cannot use the bool type in make_unsigned_t.");
    if (mppp_unlikely(static_cast<make_unsigned_t<T>>(exp) > nl_max<unsigned long>())) {
        throw std::overflow_error("Cannot convert the integral value " + detail::to_string(exp)
                                  + " to unsigned long: the value is too large");
    }
    return static_cast<unsigned long>(exp);
}

// NOTE: special case bool, otherwise we end up invoking make_unsigned_t<bool> in the
// previous overload, which is not well-defined:
// https://en.cppreference.com/w/cpp/types/make_unsigned
inline unsigned long integer_exp_to_ulong(bool exp)
{

    return static_cast<unsigned long>(exp);
}

template <std::size_t SSize>
inline unsigned long integer_exp_to_ulong(const integer<SSize> &exp)
{
    try {
        return static_cast<unsigned long>(exp);
    } catch (const std::overflow_error &) {
        // Rewrite the error message.
        throw std::overflow_error("Cannot convert the integral value " + exp.to_string()
                                  + " to unsigned long: the value is too large");
    }
}

template <typename T, std::size_t SSize>
inline integer<SSize> binomial_impl(const integer<SSize> &n, const T &k)
{
    // NOTE: here we re-use some helper member functions used in the implementation of pow().
    if (sgn(k) >= 0) {
        return bin_ui(n, integer_exp_to_ulong(k));
    }
    // This is the case k < 0, handled according to:
    // https://arxiv.org/abs/1105.3689/
    if (n.sgn() >= 0) {
        // n >= 0, k < 0.
        return integer<SSize>{};
    }
    // n < 0, k < 0.
    if (k <= n) {
        // The formula is: (-1)**(n-k) * binomial(-k-1,n-k).
        // Cache n-k.
        const integer<SSize> nmk{n - k};
        integer<SSize> tmp{k};
        ++tmp;
        tmp.neg();
        auto retval = bin_ui(tmp, integer_exp_to_ulong(nmk));
        if (nmk.odd_p()) {
            retval.neg();
        }
        return retval;
    }
    return integer<SSize>{};
}

template <typename T, std::size_t SSize, enable_if_t<is_integral<T>::value, int> = 0>
inline integer<SSize> binomial_impl(const T &n, const integer<SSize> &k)
{
    return binomial_impl(integer<SSize>{n}, k);
}
} // namespace detail

// Generic binomial coefficient.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_integral_op_types<T, U>
inline auto
#else
template <typename T, typename U, detail::enable_if_t<are_integer_integral_op_types<T, U>::value, int> = 0>
inline detail::integer_common_t<T, U>
#endif
binomial(const T &n, const U &k)
{
    return detail::binomial_impl(n, k);
}

namespace detail
{

template <std::size_t SSize>
inline void nextprime_impl(integer<SSize> &rop, const integer<SSize> &n)
{
    MPPP_MAYBE_TLS mpz_raii tmp;
    mpz_nextprime(&tmp.m_mpz, n.get_mpz_view());
    rop = &tmp.m_mpz;
}
} // namespace detail

// Compute next prime number (binary version).
template <std::size_t SSize>
inline integer<SSize> &nextprime(integer<SSize> &rop, const integer<SSize> &n)
{
    // NOTE: nextprime on negative numbers always returns 2.
    detail::nextprime_impl(rop, n);
    return rop;
}

// Compute next prime number (unary version).
template <std::size_t SSize>
inline integer<SSize> nextprime(const integer<SSize> &n)
{
    integer<SSize> retval;
    detail::nextprime_impl(retval, n);
    return retval;
}

// Test primality.
template <std::size_t SSize>
inline int probab_prime_p(const integer<SSize> &n, int reps = 25)
{
    return n.probab_prime_p(reps);
}

// Ternary exponentiation.
template <std::size_t SSize>
inline integer<SSize> &pow_ui(integer<SSize> &rop, const integer<SSize> &base, unsigned long exp)
{
    MPPP_MAYBE_TLS detail::mpz_raii tmp;
    mpz_pow_ui(&tmp.m_mpz, base.get_mpz_view(), exp);
    return rop = &tmp.m_mpz;
}

// Binary exponentiation.
template <std::size_t SSize>
inline integer<SSize> pow_ui(const integer<SSize> &base, unsigned long exp)
{
    integer<SSize> retval;
    pow_ui(retval, base, exp);
    return retval;
}

namespace detail
{

// Various helpers for the implementation of pow().
template <typename T, enable_if_t<is_integral<T>::value, int> = 0>
inline bool integer_exp_is_odd(const T &exp)
{
    return (exp % T(2)) != T(0);
}

template <std::size_t SSize>
inline bool integer_exp_is_odd(const integer<SSize> &exp)
{
    return exp.odd_p();
}

// Implementation of pow().
// integer -- integral overload.
template <typename T, std::size_t SSize,
          enable_if_t<disjunction<std::is_same<T, integer<SSize>>, is_integral<T>>::value, int> = 0>
inline integer<SSize> pow_impl(const integer<SSize> &base, const T &exp)
{
    integer<SSize> rop;
    if (sgn(exp) >= 0) {
        pow_ui(rop, base, integer_exp_to_ulong(exp));
    } else if (mppp_unlikely(is_zero(base))) {
        // 0**-n is a division by zero.
        throw zero_division_error("Cannot raise zero to the negative power " + detail::to_string(exp));
    } else if (base.is_one()) {
        // 1**-n == 1.
        rop.set_one();
    } else if (base.is_negative_one()) {
        if (integer_exp_is_odd(exp)) {
            // (-1)**(-2n-1) == -1.
            rop.set_negative_one();
        } else {
            // (-1)**(-2n) == 1.
            rop.set_one();
        }
    } else {
        // m**-n == 1 / m**n == 0, truncated division.
        rop.set_zero();
    }
    return rop;
}

// C++ integral -- integer overload.
template <typename T, std::size_t SSize, enable_if_t<is_integral<T>::value, int> = 0>
inline integer<SSize> pow_impl(const T &base, const integer<SSize> &exp)
{
    return pow_impl(integer<SSize>{base}, exp);
}

// integer -- FP/complex overload.
template <typename T, std::size_t SSize,
          enable_if_t<disjunction<std::is_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T pow_impl(const integer<SSize> &base, const T &exp)
{
    return std::pow(static_cast<T>(base), exp);
}

// FP/complex -- integer overload.
template <typename T, std::size_t SSize,
          enable_if_t<disjunction<std::is_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T pow_impl(const T &base, const integer<SSize> &exp)
{
    return std::pow(base, static_cast<T>(exp));
}

} // namespace detail

// Generic binary exponentiation.
template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
requires integer_op_types<T, U>
inline auto
#else
inline detail::integer_common_t<T, U>
#endif
pow(const T &base, const U &exp)
{
    return detail::pow_impl(base, exp);
}

namespace detail
{

// Implementation of sqrt.
template <std::size_t SSize>
inline void sqrt_impl(integer<SSize> &rop, const integer<SSize> &n)
{
    if (mppp_unlikely(n._get_union().m_st._mp_size < 0)) {
        throw std::domain_error("Cannot compute the integer square root of the negative number " + n.to_string());
    }
    const bool sr = rop.is_static(), sn = n.is_static();
    if (mppp_likely(sn)) {
        if (!sr) {
            rop.set_zero();
        }
        auto &rs = rop._get_union().g_st();
        const auto &ns = n._get_union().g_st();
        // NOTE: we know n is not negative, from the check above.
        assert(ns._mp_size >= 0);
        // NOTE: cast this to the unsigned counterpart, this will make
        // the computation of new_size below more efficient.
        const auto size = static_cast<make_unsigned_t<mpz_size_t>>(ns._mp_size);
        if (mppp_likely(size)) {
            // In case of overlap we need to go through a tmp variable.
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
            std::array<::mp_limb_t, SSize> tmp;
            const bool overlap = (&rs == &ns);
            const auto rs_data = rs.m_limbs.data(), out_ptr = overlap ? tmp.data() : rs_data;
            mpn_sqrtrem(out_ptr, nullptr, ns.m_limbs.data(), static_cast<::mp_size_t>(size));
            // Compute the size of the output (which is ceil(size / 2)).
            const auto new_size = size / 2u + size % 2u;
            assert(!new_size || (out_ptr[new_size - 1u] & GMP_NUMB_MASK));
            // Write out the result.
            rs._mp_size = static_cast<mpz_size_t>(new_size);
            if (overlap) {
                copy_limbs_no(out_ptr, out_ptr + new_size, rs_data);
            }
            // Clear out unused limbs, if needed.
            rs.zero_upper_limbs(static_cast<std::size_t>(new_size));
        } else {
            // Special casing for zero.
            rs._mp_size = 0;
            rs.zero_upper_limbs(0);
        }
    } else {
        if (sr) {
            rop.promote();
        }
        mpz_sqrt(&rop._get_union().g_dy(), n.get_mpz_view());
    }
}
} // namespace detail

// Binary sqrt.
template <std::size_t SSize>
inline integer<SSize> &sqrt(integer<SSize> &rop, const integer<SSize> &n)
{
    detail::sqrt_impl(rop, n);
    return rop;
}

// Unary sqrt.
template <std::size_t SSize>
inline integer<SSize> sqrt(const integer<SSize> &n)
{
    integer<SSize> retval;
    detail::sqrt_impl(retval, n);
    return retval;
}

namespace detail
{

// Static sqrtrem implementation.
template <std::size_t SSize>
inline void static_sqrtrem(static_int<SSize> &rops, static_int<SSize> &rems, const static_int<SSize> &ns)
{
    // NOTE: we require non-negative n (this is checked in sqrtrem()).
    assert(ns._mp_size >= 0);
    // NOTE: cast this to the unsigned counterpart, this will make
    // the computation of rop_size below more efficient.
    const auto size = static_cast<make_unsigned_t<mpz_size_t>>(ns._mp_size);
    if (mppp_likely(size)) {
        // NOTE: rop and n must be separate. rem and n can coincide. See:
        // https://gmplib.org/manual/Low_002dlevel-Functions.html
        // In case of overlap of rop and n, we need to go through a tmp variable.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        std::array<::mp_limb_t, SSize> tmp;
        const bool overlap = (&rops == &ns);
        const auto rops_data = rops.m_limbs.data(), out_ptr = overlap ? tmp.data() : rops_data,
                   rems_data = rems.m_limbs.data();
        // Do the computation. The function will return the size of the remainder.
        const auto rem_size = mpn_sqrtrem(out_ptr, rems_data, ns.m_limbs.data(), static_cast<::mp_size_t>(size));
        // Compute the size of the output (which is ceil(size / 2)).
        const auto rop_size = size / 2u + size % 2u;
        assert(!rop_size || (out_ptr[rop_size - 1u] & GMP_NUMB_MASK));
        // Write out the result, clearing the unused limbs.
        rops._mp_size = static_cast<mpz_size_t>(rop_size);
        if (overlap) {
            copy_limbs_no(out_ptr, out_ptr + rop_size, rops_data);
        }
        rops.zero_upper_limbs(static_cast<std::size_t>(rop_size));
        rems._mp_size = static_cast<mpz_size_t>(rem_size);
        rems.zero_upper_limbs(static_cast<std::size_t>(rem_size));
    } else {
        // Special casing for zero.
        rops._mp_size = 0;
        rops.zero_upper_limbs(0);
        rems._mp_size = 0;
        rems.zero_upper_limbs(0);
    }
}

} // namespace detail

// sqrt with remainder.
template <std::size_t SSize>
inline void sqrtrem(integer<SSize> &rop, integer<SSize> &rem, const integer<SSize> &n)
{
    if (mppp_unlikely(&rop == &rem)) {
        throw std::invalid_argument("When performing an integer square root with remainder, the result 'rop' and the "
                                    "remainder 'rem' must be distinct objects");
    }
    if (mppp_unlikely(n.sgn() == -1)) {
        throw zero_division_error("Cannot compute the integer square root with remainder of the negative number "
                                  + n.to_string());
    }
    const bool srop = rop.is_static(), srem = rem.is_static(), ns = n.is_static();
    if (mppp_likely(ns)) {
        if (!srop) {
            rop.set_zero();
        }
        if (!srem) {
            rem.set_zero();
        }
        // NOTE: sqrtrem() can never fail.
        static_sqrtrem(rop._get_union().g_st(), rem._get_union().g_st(), n._get_union().g_st());
    } else {
        if (srop) {
            rop._get_union().promote();
        }
        if (srem) {
            rem._get_union().promote();
        }
        mpz_sqrtrem(&rop._get_union().g_dy(), &rem._get_union().g_dy(), n.get_mpz_view());
    }
}

// Detect perfect square.
template <std::size_t SSize>
inline bool perfect_square_p(const integer<SSize> &n)
{
    const auto &u = n._get_union();
    // NOTE: the size is part of the common initial sequence.
    const auto size = u.m_st._mp_size;
    if (mppp_likely(size > 0)) {
        // n is positive. Extract a pointer to the limbs
        // and call the mpn function.
        // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
        const mp_limb_t *ptr;
        if (mppp_likely(n.is_static())) {
            ptr = u.g_st().m_limbs.data();
        } else {
            ptr = u.g_dy()._mp_d;
        }
        // NOTE: as usual, we assume that we can freely cast any valid mpz_size_t to
        // mp_size_t when calling mpn functions.
        return mpn_perfect_square_p(ptr, static_cast<::mp_size_t>(size)) != 0;
    } else {
        // n is zero or negative. It is a perfect square
        // only if zero.
        return size == 0;
    }
}

// m-th root, ternary version.
template <std::size_t SSize>
inline bool root(integer<SSize> &rop, const integer<SSize> &n, unsigned long m)
{
    if (mppp_unlikely(!m)) {
        throw std::domain_error("Cannot compute the integer m-th root of an integer if m is zero");
    }
    if (mppp_unlikely(!(m % 2u) && n.sgn() == -1)) {
        throw std::domain_error("Cannot compute the integer root of degree " + std::to_string(m)
                                + " of the negative number " + n.to_string());
    }
    MPPP_MAYBE_TLS detail::mpz_raii tmp;
    const auto ret = mpz_root(&tmp.m_mpz, n.get_mpz_view(), m);
    rop = &tmp.m_mpz;
    return ret != 0;
}

// m-th root, binary version.
template <std::size_t SSize>
inline integer<SSize> root(const integer<SSize> &n, unsigned long m)
{
    integer<SSize> retval;
    root(retval, n, m);
    return retval;
}

// m-th root with remainder.
template <std::size_t SSize>
inline void rootrem(integer<SSize> &rop, integer<SSize> &rem, const integer<SSize> &n, unsigned long m)
{
    if (mppp_unlikely(!m)) {
        throw std::domain_error("Cannot compute the integer m-th root with remainder of an integer if m is zero");
    }
    if (mppp_unlikely(!(m % 2u) && n.sgn() == -1)) {
        throw std::domain_error("Cannot compute the integer root with remainder of degree " + std::to_string(m)
                                + " of the negative number " + n.to_string());
    }
    MPPP_MAYBE_TLS detail::mpz_raii tmp_rop;
    MPPP_MAYBE_TLS detail::mpz_raii tmp_rem;
    mpz_rootrem(&tmp_rop.m_mpz, &tmp_rem.m_mpz, n.get_mpz_view(), m);
    rop = &tmp_rop.m_mpz;
    rem = &tmp_rem.m_mpz;
}

// Detect perfect power.
template <std::size_t SSize>
inline bool perfect_power_p(const integer<SSize> &n)
{
    return mpz_perfect_power_p(n.get_mpz_view()) != 0;
}

namespace detail
{

// Helper to get the base from a stream's flags.
inline int stream_flags_to_base(std::ios_base::fmtflags flags)
{
    switch (flags & std::ios_base::basefield) {
        case std::ios_base::dec:
            return 10;
        case std::ios_base::hex:
            return 16;
        case std::ios_base::oct:
            return 8;
        default:
            // NOTE: in case more than one base
            // flag is set, or no base flags are set,
            // just use 10.
            return 10;
    }
}

// Helper to get the fill type from a stream's flags.
inline int stream_flags_to_fill(std::ios_base::fmtflags flags)
{
    switch (flags & std::ios_base::adjustfield) {
        case std::ios_base::left:
            return 1;
        case std::ios_base::right:
            return 2;
        case std::ios_base::internal:
            return 3;
        default:
            // NOTE: assume right fill if no fill is set,
            // or if multiple values are set.
            return 2;
    }
}

MPPP_DLL_PUBLIC std::ostream &integer_stream_operator_impl(std::ostream &, const mpz_struct_t *, int);

} // namespace detail

// Output stream operator.
template <std::size_t SSize>
inline std::ostream &operator<<(std::ostream &os, const integer<SSize> &n)
{
    return detail::integer_stream_operator_impl(os, n.get_mpz_view(), n.sgn());
}

// Binary size.
template <std::size_t SSize>
inline std::size_t binary_size(const integer<SSize> &n)
{
    return n.binary_size();
}

// Save in binary format.
template <std::size_t SSize, typename T>
inline auto binary_save(const integer<SSize> &n, T &&dest) -> decltype(n.binary_save(std::forward<T>(dest)))
{
    return n.binary_save(std::forward<T>(dest));
}

// Load in binary format.
template <std::size_t SSize, typename T>
inline auto binary_load(integer<SSize> &n, T &&src) -> decltype(n.binary_load(std::forward<T>(src)))
{
    return n.binary_load(std::forward<T>(src));
}

// Hash value.
template <std::size_t SSize>
inline std::size_t hash(const integer<SSize> &n)
{
    const detail::mpz_size_t size = n._get_union().m_st._mp_size;
    const std::size_t asize
        = size >= 0 ? static_cast<std::size_t>(size) : static_cast<std::size_t>(detail::nint_abs(size));
    const ::mp_limb_t *ptr
        = n._get_union().is_static() ? n._get_union().g_st().m_limbs.data() : n._get_union().g_dy()._mp_d;
    // Init the retval as the signed size.
    auto retval = static_cast<std::size_t>(size);
    // Combine the limbs.
    for (std::size_t i = 0; i < asize; ++i) {
        // The hash combiner. This is lifted directly from Boost. See also:
        // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3876.pdf
        retval ^= (ptr[i] & GMP_NUMB_MASK) + std::size_t(0x9e3779b9ul) + (retval << 6) + (retval >> 2);
    }
    return retval;
}

// Free the caches.
MPPP_DLL_PUBLIC void free_integer_caches();

namespace detail
{

// Dispatching for the binary addition operator.
template <std::size_t SSize>
inline integer<SSize> dispatch_binary_add(const integer<SSize> &op1, const integer<SSize> &op2)
{
    integer<SSize> retval;
    add(retval, op1, op2);
    return retval;
}

// NOTE: use the add_si/add_ui functions when adding to C++ integrals.
template <std::size_t SSize, typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_add(const integer<SSize> &op1, T n)
{
    integer<SSize> retval;
    add_ui(retval, op1, n);
    return retval;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_add(const integer<SSize> &op1, T n)
{
    integer<SSize> retval;
    add_si(retval, op1, n);
    return retval;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_add(T n, const integer<SSize> &op2)
{
    return dispatch_binary_add(op2, n);
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_add(const integer<SSize> &op1, const T &x)
{
    return static_cast<T>(op1) + x;
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_add(const T &x, const integer<SSize> &op2)
{
    return dispatch_binary_add(op2, x);
}

// Dispatching for in-place add.
template <std::size_t SSize>
inline void dispatch_in_place_add(integer<SSize> &retval, const integer<SSize> &n)
{
    add(retval, retval, n);
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline void dispatch_in_place_add(integer<SSize> &retval, const T &n)
{
    add_ui(retval, retval, n);
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline void dispatch_in_place_add(integer<SSize> &retval, const T &n)
{
    add_si(retval, retval, n);
}

template <std::size_t SSize, typename T,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_add(integer<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) + x;
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_arithmetic<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_add(T &rop, const integer<SSize> &op)
{
    rop = static_cast<T>(rop + op);
}

} // namespace detail

// Identity operator.
template <std::size_t SSize>
inline integer<SSize> operator+(const integer<SSize> &n)
{
    // NOTE: here potentially we could avoid a copy via either
    // a universal reference or maybe passing by copy n and then
    // moving in. Not sure how critical this is. Same in the negated
    // copy operator.
    return n;
}

// Binary addition.
template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
requires integer_op_types<T, U>
inline auto
#else
inline detail::integer_common_t<T, U>
#endif
operator+(const T &op1, const U &op2)
{
    return detail::dispatch_binary_add(op1, op2);
}

// In-place addition operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_op_types<T, U>::value, int> = 0>
#endif
inline T &operator+=(T &rop, const U &op)
{
    detail::dispatch_in_place_add(rop, op);
    return rop;
}

// Prefix increment.
template <std::size_t SSize>
inline integer<SSize> &operator++(integer<SSize> &n)
{
    add_ui(n, n, 1u);
    return n;
}

// Suffix increment.
template <std::size_t SSize>
inline integer<SSize> operator++(integer<SSize> &n, int)
{
    auto retval(n);
    ++n;
    return retval;
}

namespace detail
{

// Dispatching for the binary subtraction operator.
template <std::size_t SSize>
inline integer<SSize> dispatch_binary_sub(const integer<SSize> &op1, const integer<SSize> &op2)
{
    integer<SSize> retval;
    sub(retval, op1, op2);
    return retval;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_sub(const integer<SSize> &op1, T n)
{
    integer<SSize> retval;
    sub_ui(retval, op1, n);
    return retval;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_sub(const integer<SSize> &op1, T n)
{
    integer<SSize> retval;
    sub_si(retval, op1, n);
    return retval;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_sub(T n, const integer<SSize> &op2)
{
    auto retval = dispatch_binary_sub(op2, n);
    retval.neg();
    return retval;
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_sub(const integer<SSize> &op1, const T &x)
{
    return static_cast<T>(op1) - x;
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_sub(const T &x, const integer<SSize> &op2)
{
    return -dispatch_binary_sub(op2, x);
}

// Dispatching for in-place sub.
template <std::size_t SSize>
inline void dispatch_in_place_sub(integer<SSize> &retval, const integer<SSize> &n)
{
    sub(retval, retval, n);
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_unsigned_integral<T>::value, int> = 0>
inline void dispatch_in_place_sub(integer<SSize> &retval, const T &n)
{
    sub_ui(retval, retval, n);
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_signed_integral<T>::value, int> = 0>
inline void dispatch_in_place_sub(integer<SSize> &retval, const T &n)
{
    sub_si(retval, retval, n);
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_sub(integer<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) - x;
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_arithmetic<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_sub(T &rop, const integer<SSize> &op)
{
    rop = static_cast<T>(rop - op);
}

} // namespace detail

// Negated copy.
template <std::size_t SSize>
integer<SSize> operator-(const integer<SSize> &n)
{
    auto retval(n);
    retval.neg();
    return retval;
}

// Binary subtraction.
template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
requires integer_op_types<T, U>
inline auto
#else
inline detail::integer_common_t<T, U>
#endif
operator-(const T &op1, const U &op2)
{
    return detail::dispatch_binary_sub(op1, op2);
}

// In-place subtraction operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_op_types<T, U>::value, int> = 0>
#endif
inline T &operator-=(T &rop, const U &op)
{
    detail::dispatch_in_place_sub(rop, op);
    return rop;
}

// Prefix decrement.
template <std::size_t SSize>
inline integer<SSize> &operator--(integer<SSize> &n)
{
    sub_ui(n, n, 1u);
    return n;
}

// Suffix decrement.
template <std::size_t SSize>
inline integer<SSize> operator--(integer<SSize> &n, int)
{
    auto retval(n);
    --n;
    return retval;
}

namespace detail
{

// Dispatching for the binary multiplication operator.
template <std::size_t SSize>
inline integer<SSize> dispatch_binary_mul(const integer<SSize> &op1, const integer<SSize> &op2)
{
    integer<SSize> retval;
    mul(retval, op1, op2);
    return retval;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_mul(const integer<SSize> &op1, T n)
{
    // NOTE: with respect to addition, here we separate the retval
    // from the operands. Having a separate destination is generally better
    // for multiplication.
    integer<SSize> retval;
    mul(retval, op1, integer<SSize>{n});
    return retval;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_mul(T n, const integer<SSize> &op2)
{
    return dispatch_binary_mul(op2, n);
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_mul(const integer<SSize> &op1, const T &x)
{
    return static_cast<T>(op1) * x;
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_mul(const T &x, const integer<SSize> &op2)
{
    return dispatch_binary_mul(op2, x);
}

// Dispatching for in-place multiplication.
template <std::size_t SSize>
inline void dispatch_in_place_mul(integer<SSize> &retval, const integer<SSize> &n)
{
    mul(retval, retval, n);
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_mul(integer<SSize> &retval, const T &n)
{
    mul(retval, retval, integer<SSize>{n});
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_mul(integer<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) * x;
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_arithmetic<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_mul(T &rop, const integer<SSize> &op)
{
    rop = static_cast<T>(rop * op);
}

} // namespace detail

// Binary multiplication.
template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
requires integer_op_types<T, U>
inline auto
#else
inline detail::integer_common_t<T, U>
#endif
operator*(const T &op1, const U &op2)
{
    return detail::dispatch_binary_mul(op1, op2);
}

// In-place multiplication operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_op_types<T, U>::value, int> = 0>
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
inline integer<SSize> dispatch_binary_div(const integer<SSize> &op1, const integer<SSize> &op2)
{
    integer<SSize> retval;
    tdiv_q(retval, op1, op2);
    return retval;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_div(const integer<SSize> &op1, T n)
{
    integer<SSize> retval;
    tdiv_q(retval, op1, integer<SSize>{n});
    return retval;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_div(T n, const integer<SSize> &op2)
{
    integer<SSize> retval;
    tdiv_q(retval, integer<SSize>{n}, op2);
    return retval;
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_div(const integer<SSize> &op1, const T &x)
{
    return static_cast<T>(op1) / x;
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline T dispatch_binary_div(const T &x, const integer<SSize> &op2)
{
    return x / static_cast<T>(op2);
}

// Dispatching for in-place div.
template <std::size_t SSize>
inline void dispatch_in_place_div(integer<SSize> &retval, const integer<SSize> &n)
{
    tdiv_q(retval, retval, n);
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_div(integer<SSize> &retval, const T &n)
{
    tdiv_q(retval, retval, integer<SSize>{n});
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_div(integer<SSize> &retval, const T &x)
{
    retval = static_cast<T>(retval) / x;
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_arithmetic<T>, is_cpp_complex<T>>::value, int> = 0>
inline void dispatch_in_place_div(T &rop, const integer<SSize> &op)
{
    rop = static_cast<T>(rop / op);
}

// Dispatching for the binary modulo operator.
template <std::size_t SSize>
inline integer<SSize> dispatch_binary_mod(const integer<SSize> &op1, const integer<SSize> &op2)
{
    integer<SSize> q, retval;
    tdiv_qr(q, retval, op1, op2);
    return retval;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_mod(const integer<SSize> &op1, T n)
{
    integer<SSize> q, retval;
    tdiv_qr(q, retval, op1, integer<SSize>{n});
    return retval;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_binary_mod(T n, const integer<SSize> &op2)
{
    integer<SSize> q, retval;
    tdiv_qr(q, retval, integer<SSize>{n}, op2);
    return retval;
}

// Dispatching for in-place mod.
template <std::size_t SSize>
inline void dispatch_in_place_mod(integer<SSize> &retval, const integer<SSize> &n)
{
    integer<SSize> q;
    tdiv_qr(q, retval, retval, n);
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_mod(integer<SSize> &retval, const T &n)
{
    integer<SSize> q;
    tdiv_qr(q, retval, retval, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_mod(T &rop, const integer<SSize> &op)
{
    rop = static_cast<T>(rop % op);
}
} // namespace detail

// Binary division.
template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
requires integer_op_types<T, U>
inline auto
#else
inline detail::integer_common_t<T, U>
#endif
operator/(const T &n, const U &d)
{
    return detail::dispatch_binary_div(n, d);
}

// In-place division operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_op_types<T, U>::value, int> = 0>
#endif
inline T &operator/=(T &rop, const U &op)
{
    detail::dispatch_in_place_div(rop, op);
    return rop;
}

// Binary modulo operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_integral_op_types<T, U>
inline auto
#else
template <typename T, typename U, detail::enable_if_t<are_integer_integral_op_types<T, U>::value, int> = 0>
inline detail::integer_common_t<T, U>
#endif
operator%(const T &n, const U &d)
{
    return detail::dispatch_binary_mod(n, d);
}

// In-place modulo operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_integral_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_integral_op_types<T, U>::value, int> = 0>
#endif
inline T &operator%=(T &rop, const U &op)
{
    detail::dispatch_in_place_mod(rop, op);
    return rop;
}

// Binary left shift operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <cpp_integral T, std::size_t SSize>
#else
template <typename T, std::size_t SSize, detail::enable_if_t<is_cpp_integral<T>::value, int> = 0>
#endif
inline integer<SSize> operator<<(const integer<SSize> &n, T s)
{
    integer<SSize> retval;
    mul_2exp(retval, n, detail::safe_cast<::mp_bitcnt_t>(s));
    return retval;
}

// In-place left shift operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <cpp_integral T, std::size_t SSize>
#else
template <typename T, std::size_t SSize, detail::enable_if_t<is_cpp_integral<T>::value, int> = 0>
#endif
inline integer<SSize> &operator<<=(integer<SSize> &rop, T s)
{
    mul_2exp(rop, rop, detail::safe_cast<::mp_bitcnt_t>(s));
    return rop;
}

// Binary right shift operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <cpp_integral T, std::size_t SSize>
#else
template <typename T, std::size_t SSize, detail::enable_if_t<is_cpp_integral<T>::value, int> = 0>
#endif
inline integer<SSize> operator>>(const integer<SSize> &n, T s)
{
    integer<SSize> retval;
    tdiv_q_2exp(retval, n, detail::safe_cast<::mp_bitcnt_t>(s));
    return retval;
}

// In-place right shift operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <cpp_integral T, std::size_t SSize>
#else
template <typename T, std::size_t SSize, detail::enable_if_t<is_cpp_integral<T>::value, int> = 0>
#endif
inline integer<SSize> &operator>>=(integer<SSize> &rop, T s)
{
    tdiv_q_2exp(rop, rop, detail::safe_cast<::mp_bitcnt_t>(s));
    return rop;
}

namespace detail
{

// Equality operator.
// NOTE: special implementation instead of using cmp, this should be faster.
template <std::size_t SSize>
inline bool dispatch_equality(const integer<SSize> &a, const integer<SSize> &b)
{
    const mp_size_t size_a = a._get_union().m_st._mp_size, size_b = b._get_union().m_st._mp_size;
    if (size_a != size_b) {
        return false;
    }
    const std::size_t asize
        = size_a >= 0 ? static_cast<std::size_t>(size_a) : static_cast<std::size_t>(nint_abs(size_a));
    const ::mp_limb_t *ptr_a = a.is_static() ? a._get_union().g_st().m_limbs.data() : a._get_union().g_dy()._mp_d;
    const ::mp_limb_t *ptr_b = b.is_static() ? b._get_union().g_st().m_limbs.data() : b._get_union().g_dy()._mp_d;
    auto limb_cmp
        = [](const ::mp_limb_t &l1, const ::mp_limb_t &l2) { return (l1 & GMP_NUMB_MASK) == (l2 & GMP_NUMB_MASK); };
    // NOTE: cannot understand why, but MSVC wants a checked iterator as 3rd
    // parameter here.
    return std::equal(ptr_a, ptr_a + asize, make_uai(ptr_b), limb_cmp);
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline bool dispatch_equality(const integer<SSize> &a, T n)
{
    return dispatch_equality(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline bool dispatch_equality(T n, const integer<SSize> &a)
{
    return dispatch_equality(a, n);
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline bool dispatch_equality(const integer<SSize> &a, const T &x)
{
    return static_cast<T>(a) == x;
}

template <typename T, std::size_t SSize,
          enable_if_t<disjunction<is_cpp_floating_point<T>, is_cpp_complex<T>>::value, int> = 0>
inline bool dispatch_equality(const T &x, const integer<SSize> &a)
{
    return dispatch_equality(a, x);
}

// Less-than operator.

// 1-limb specialisation.
inline bool static_less_than(const static_int<1> &op1, const static_int<1> &op2)
{
    // NOTE: an implementation with 1 branch less and similar performance
    // is available at fdb84f57027ebf579a380f07501435eb17fd6db1.

    const auto size1 = op1._mp_size, size2 = op2._mp_size;

    // Compare sizes.
    // NOTE: under the assumption that we have positive/negative
    // integers with equal probability, it makes sense to check
    // for the sizes first.
    if (size1 < size2) {
        return true;
    }
    if (size1 > size2) {
        return false;
    }

    // Sizes are equal, compare the only limb.
    const auto l1 = op1.m_limbs[0] & GMP_NUMB_MASK;
    const auto l2 = op2.m_limbs[0] & GMP_NUMB_MASK;

    const auto lt = l1 < l2;
    const auto gt = l1 > l2;

    // NOTE: as usual, this branchless formulation is slightly
    // better for signed ints, slightly worse for unsigned
    // (wrt an if statement).
    return (size1 >= 0 && lt) || (size1 < 0 && gt);
}

// mpn implementation.
template <std::size_t SSize>
inline bool static_less_than(const static_int<SSize> &n1, const static_int<SSize> &n2)
{
    const auto size1 = n1._mp_size, size2 = n2._mp_size;

    // Compare sizes.
    if (size1 < size2) {
        return true;
    }
    if (size1 > size2) {
        return false;
    }

    // The two sizes are equal, compare the absolute values.
    if (size1) {
        const int cmp_abs = mpn_cmp(n1.m_limbs.data(), n2.m_limbs.data(), static_cast<::mp_size_t>(std::abs(size1)));
        return (size1 >= 0 && cmp_abs < 0) || (size1 < 0 && cmp_abs > 0);
    }
    // Both operands are zero.
    // NOTE: we do this special casing in order to avoid calling mpn_cmp() on zero operands. It seems to
    // work, but the official GMP docs say one is not supposed to call mpn functions on zero operands.
    return false;
}

template <std::size_t SSize>
inline bool dispatch_less_than(const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool s1 = op1.is_static(), s2 = op2.is_static();
    if (mppp_likely(s1 && s2)) {
        return static_less_than(op1._get_union().g_st(), op2._get_union().g_st());
    }

    return mpz_cmp(op1.get_mpz_view(), op2.get_mpz_view()) < 0;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline bool dispatch_less_than(const integer<SSize> &a, T n)
{
    return dispatch_less_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
bool dispatch_less_than(T, const integer<SSize> &);

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point<T>::value, int> = 0>
inline bool dispatch_less_than(const integer<SSize> &a, T x)
{
    return static_cast<T>(a) < x;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point<T>::value, int> = 0>
bool dispatch_less_than(T, const integer<SSize> &);

// Greater-than operator.

// NOTE: this is essentially the same as static_less_than.
inline bool static_greater_than(const static_int<1> &op1, const static_int<1> &op2)
{
    const auto size1 = op1._mp_size, size2 = op2._mp_size;

    if (size1 > size2) {
        return true;
    }
    if (size1 < size2) {
        return false;
    }

    const auto l1 = op1.m_limbs[0] & GMP_NUMB_MASK;
    const auto l2 = op2.m_limbs[0] & GMP_NUMB_MASK;

    const auto lt = l1 < l2;
    const auto gt = l1 > l2;

    return (size1 >= 0 && gt) || (size1 < 0 && lt);
}

template <std::size_t SSize>
inline bool static_greater_than(const static_int<SSize> &n1, const static_int<SSize> &n2)
{
    const auto size1 = n1._mp_size, size2 = n2._mp_size;

    if (size1 > size2) {
        return true;
    }
    if (size1 < size2) {
        return false;
    }

    if (size1) {
        const int cmp_abs = mpn_cmp(n1.m_limbs.data(), n2.m_limbs.data(), static_cast<::mp_size_t>(std::abs(size1)));
        return (size1 >= 0 && cmp_abs > 0) || (size1 < 0 && cmp_abs < 0);
    }
    return false;
}

template <std::size_t SSize>
inline bool dispatch_greater_than(const integer<SSize> &op1, const integer<SSize> &op2)
{
    const bool s1 = op1.is_static(), s2 = op2.is_static();
    if (mppp_likely(s1 && s2)) {
        return static_greater_than(op1._get_union().g_st(), op2._get_union().g_st());
    }

    return mpz_cmp(op1.get_mpz_view(), op2.get_mpz_view()) > 0;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline bool dispatch_greater_than(const integer<SSize> &a, T n)
{
    return dispatch_greater_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline bool dispatch_greater_than(T n, const integer<SSize> &a)
{
    return dispatch_less_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point<T>::value, int> = 0>
inline bool dispatch_greater_than(const integer<SSize> &a, T x)
{
    return static_cast<T>(a) > x;
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point<T>::value, int> = 0>
inline bool dispatch_greater_than(T x, const integer<SSize> &a)
{
    return dispatch_less_than(a, x);
}

// NOTE: implement these here as we need visibility of dispatch_greater_than().
template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int>>
inline bool dispatch_less_than(T n, const integer<SSize> &a)
{
    return dispatch_greater_than(a, integer<SSize>{n});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_floating_point<T>::value, int>>
inline bool dispatch_less_than(T x, const integer<SSize> &a)
{
    return dispatch_greater_than(a, x);
}
} // namespace detail

// Equality operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_op_types<T, U>::value, int> = 0>
#endif
inline bool operator==(const T &op1, const U &op2)
{
    return detail::dispatch_equality(op1, op2);
}

// Inequality operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_op_types<T, U>::value, int> = 0>
#endif
inline bool operator!=(const T &op1, const U &op2)
{
    return !(op1 == op2);
}

// Less-than operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator<(const T &op1, const U &op2)
{
    return detail::dispatch_less_than(op1, op2);
}

// Less-than or equal operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator<=(const T &op1, const U &op2)
{
    return !(op1 > op2);
}

// Greater-than operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator>(const T &op1, const U &op2)
{
    return detail::dispatch_greater_than(op1, op2);
}

// Greater-than or equal operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_real_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_real_op_types<T, U>::value, int> = 0>
#endif
inline bool operator>=(const T &op1, const U &op2)
{
    return !(op1 < op2);
}

// Unary bitwise NOT.
template <std::size_t SSize>
integer<SSize> operator~(const integer<SSize> &op)
{
    integer<SSize> retval;
    bitwise_not(retval, op);
    return retval;
}

namespace detail
{

// Dispatch for binary OR.
template <std::size_t SSize>
inline integer<SSize> dispatch_operator_or(const integer<SSize> &op1, const integer<SSize> &op2)
{
    integer<SSize> retval;
    bitwise_ior(retval, op1, op2);
    return retval;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_operator_or(const integer<SSize> &op1, const T &op2)
{
    return dispatch_operator_or(op1, integer<SSize>{op2});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_operator_or(const T &op1, const integer<SSize> &op2)
{
    return dispatch_operator_or(op2, op1);
}

// Dispatching for in-place OR.
template <std::size_t SSize>
inline void dispatch_in_place_or(integer<SSize> &rop, const integer<SSize> &op)
{
    bitwise_ior(rop, rop, op);
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_or(integer<SSize> &rop, const T &op)
{
    dispatch_in_place_or(rop, integer<SSize>{op});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_or(T &rop, const integer<SSize> &op)
{
    rop = static_cast<T>(rop | op);
}
} // namespace detail

// Binary bitwise OR operator
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_integral_op_types<T, U>
inline auto
#else
template <typename T, typename U, detail::enable_if_t<are_integer_integral_op_types<T, U>::value, int> = 0>
inline detail::integer_common_t<T, U>
#endif
operator|(const T &op1, const U &op2)
{
    return detail::dispatch_operator_or(op1, op2);
}

// In-place bitwise OR operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_integral_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_integral_op_types<T, U>::value, int> = 0>
#endif
inline T &operator|=(T &rop, const U &op)
{
    detail::dispatch_in_place_or(rop, op);
    return rop;
}

namespace detail
{

// Dispatch for binary AND.
template <std::size_t SSize>
inline integer<SSize> dispatch_operator_and(const integer<SSize> &op1, const integer<SSize> &op2)
{
    integer<SSize> retval;
    bitwise_and(retval, op1, op2);
    return retval;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_operator_and(const integer<SSize> &op1, const T &op2)
{
    return dispatch_operator_and(op1, integer<SSize>{op2});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_operator_and(const T &op1, const integer<SSize> &op2)
{
    return dispatch_operator_and(op2, op1);
}

// Dispatching for in-place AND.
template <std::size_t SSize>
inline void dispatch_in_place_and(integer<SSize> &rop, const integer<SSize> &op)
{
    bitwise_and(rop, rop, op);
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_and(integer<SSize> &rop, const T &op)
{
    dispatch_in_place_and(rop, integer<SSize>{op});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_and(T &rop, const integer<SSize> &op)
{
    rop = static_cast<T>(rop & op);
}
} // namespace detail

// Binary bitwise AND operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_integral_op_types<T, U>
inline auto
#else
template <typename T, typename U, detail::enable_if_t<are_integer_integral_op_types<T, U>::value, int> = 0>
inline detail::integer_common_t<T, U>
#endif
operator&(const T &op1, const U &op2)
{
    return detail::dispatch_operator_and(op1, op2);
}

// In-place bitwise AND operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_integral_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_integral_op_types<T, U>::value, int> = 0>
#endif
inline T &operator&=(T &rop, const U &op)
{
    detail::dispatch_in_place_and(rop, op);
    return rop;
}

namespace detail
{

// Dispatch for binary XOR.
template <std::size_t SSize>
inline integer<SSize> dispatch_operator_xor(const integer<SSize> &op1, const integer<SSize> &op2)
{
    integer<SSize> retval;
    bitwise_xor(retval, op1, op2);
    return retval;
}

template <std::size_t SSize, typename T, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_operator_xor(const integer<SSize> &op1, const T &op2)
{
    return dispatch_operator_xor(op1, integer<SSize>{op2});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline integer<SSize> dispatch_operator_xor(const T &op1, const integer<SSize> &op2)
{
    return dispatch_operator_xor(op2, op1);
}

// Dispatching for in-place XOR.
template <std::size_t SSize>
inline void dispatch_in_place_xor(integer<SSize> &rop, const integer<SSize> &op)
{
    bitwise_xor(rop, rop, op);
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_xor(integer<SSize> &rop, const T &op)
{
    dispatch_in_place_xor(rop, integer<SSize>{op});
}

template <typename T, std::size_t SSize, enable_if_t<is_cpp_integral<T>::value, int> = 0>
inline void dispatch_in_place_xor(T &rop, const integer<SSize> &op)
{
    rop = static_cast<T>(rop ^ op);
}
} // namespace detail

// Binary bitwise XOR operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_integral_op_types<T, U>
inline auto
#else
template <typename T, typename U, detail::enable_if_t<are_integer_integral_op_types<T, U>::value, int> = 0>
inline detail::integer_common_t<T, U>
#endif
operator^(const T &op1, const U &op2)
{
    return detail::dispatch_operator_xor(op1, op2);
}

// In-place bitwise XOR operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires integer_integral_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_integer_integral_op_types<T, U>::value, int> = 0>
#endif
inline T &operator^=(T &rop, const U &op)
{
    detail::dispatch_in_place_xor(rop, op);
    return rop;
}

} // namespace mppp

#if defined(MPPP_WITH_BOOST_S11N)

// Disable tracking for mppp::integer.
// NOTE: this code has been lifted from the Boost
// macro, which does not support directly
// class templates.

namespace boost
{

namespace serialization
{

template <std::size_t SSize>
struct tracking_level<mppp::integer<SSize>> {
    typedef mpl::integral_c_tag tag;
    typedef mpl::int_<track_never> type;
    BOOST_STATIC_CONSTANT(int, value = tracking_level::type::value);
    BOOST_STATIC_ASSERT((mpl::greater<implementation_level<mppp::integer<SSize>>, mpl::int_<primitive_type>>::value));
};

} // namespace serialization

} // namespace boost

#endif

namespace std
{

// Specialisation of std::hash for mppp::integer.
template <size_t SSize>
struct hash<mppp::integer<SSize>> {
// NOTE: these typedefs have been deprecated in C++17.
#if MPPP_CPLUSPLUS < 201703L
    // The argument type.
    using argument_type = mppp::integer<SSize>;
    // The result type.
    using result_type = size_t;
#endif
    // Call operator.
    size_t operator()(const mppp::integer<SSize> &n) const
    {
        return mppp::hash(n);
    }
};

} // namespace std

#if defined(_MSC_VER)

#pragma warning(pop)

#endif

#include <mp++/detail/integer_literals.hpp>

// Support for pretty printing in xeus-cling.
#if defined(__CLING__)

#if __has_include(<nlohmann/json.hpp>)

#include <nlohmann/json.hpp>

namespace mppp
{

template <std::size_t SSize>
inline nlohmann::json mime_bundle_repr(const integer<SSize> &n)
{
    auto bundle = nlohmann::json::object();

    bundle["text/plain"] = n.to_string();

    return bundle;
}

} // namespace mppp

#endif

#endif

#endif
