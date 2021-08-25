// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <mp++/config.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>

namespace mppp
{

namespace detail
{

namespace
{

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
                  // mp_bitcnt_t is used in shift operators, so we double-check it is an unsigned integral. If it is not
                  // we might end up shifting by negative values, which is UB.
                  std::is_integral<::mp_bitcnt_t>::value && std::is_unsigned<::mp_bitcnt_t>::value &&
                  // The mpn functions accept sizes as ::mp_size_t, but we generally represent sizes as mpz_size_t.
                  // We need then to make sure we can always cast safely mpz_size_t to ::mp_size_t. Inspection
                  // of the gmp.h header seems to indicate that mpz_size_t is never larger than ::mp_size_t.
                  // NOTE: since mp_size_t is the size type used by mpn functions, it is expected that it cannot
                  // have smaller range than mpz_size_t, otherwise mpz_t would contain numbers too large to be
                  // used as arguments in the mpn functions.
                  // NOLINTNEXTLINE(misc-redundant-expression)
                  nl_min<mpz_size_t>() >= nl_min<::mp_size_t>() && nl_max<mpz_size_t>() <= nl_max<::mp_size_t>(),
              "Invalid mpz_t struct layout and/or GMP types.");

#if MPPP_CPLUSPLUS >= 201703L

// If we have C++17, we can use structured bindings to test the layout of mpz_struct_t
// and its members' types.
constexpr bool test_mpz_struct_t()
{
    // NOTE: if mpz_struct_t has more or fewer members, this will result
    // in a compile-time error.
    auto [alloc, size, ptr] = mpz_struct_t{};
    ignore(alloc, size, ptr);
    // NOLINTNEXTLINE(misc-redundant-expression)
    return std::is_same<decltype(alloc), mpz_alloc_t>::value && std::is_same<decltype(size), mpz_size_t>::value
           && std::is_same<decltype(ptr), ::mp_limb_t *>::value;
}

static_assert(test_mpz_struct_t(), "The mpz_struct_t does not have the expected layout.");

#endif

// The reason we are asserting this is the following: in a few places we are using the wrap-around property
// of unsigned arithmetics, but if mp_limb_t is a narrow unsigned type (e.g., unsigned short or unsigned char)
// then there could be a promotion to other types triggered by the standard integral promotions,
// and the wrap around behaviour would not be there any more. This is just a theoretical concern at the moment.
static_assert(disjunction<std::is_same<::mp_limb_t, unsigned long>, std::is_same<::mp_limb_t, unsigned long long>,
                          std::is_same<::mp_limb_t, unsigned>>::value,
              "Invalid type for mp_limb_t.");

} // namespace

void mpz_alloc_cache::clear() noexcept
{
#if !defined(NDEBUG)
    std::cout << "Cleaning up the mpz alloc cache." << std::endl;
#endif
    // Get the GMP free() function.
    void (*ffp)(void *, std::size_t) = nullptr;
    ::mp_get_memory_functions(nullptr, nullptr, &ffp);
    assert(ffp != nullptr);
    for (std::size_t i = 0; i < max_size; ++i) {
        // Free all the limbs arrays allocated for this size.
        for (std::size_t j = 0; j < sizes[i]; ++j) {
            ffp(static_cast<void *>(caches[i][j]), i + 1u);
        }
        // Reset the number of limbs array present in this
        // cache entry.
        sizes[i] = 0u;
    }
}

#if defined(MPPP_HAVE_THREAD_LOCAL)

namespace
{

// Thread local allocation cache.
// NOTE: some link/notes regarding thread_local:
// - thread_local alone also implies "static":
//   http://eel.is/c++draft/dcl.stc
//   https://stackoverflow.com/questions/22794382/are-c11-thread-local-variables-automatically-static
// - all variables declared thread_local have "thread storage duration", that is,
//   they live for the duration of the thread: http://eel.is/c++draft/basic.stc.thread
// - there are 2 ways variables with thread storage duration may be initialised:
//   - static initialisation, which is possible for constexpr-like entities:
//     http://eel.is/c++draft/basic.start.static
//     Note that it says: "Constant initialization is performed if a variable or temporary object with static or thread
//     storage duration is initialized by a constant initializer for the entity". So it seems like block-scope
//     variables with thread storage duration will be initialised as part of constant initialisation at thread startup,
//     if possible. See also the cppreference page:
//     https://en.cppreference.com/w/cpp/language/storage_duration#Static_local_variables
//     (here it is talking about static local variables, but it should apply also to thread_local variables
//     as indicated here: https://en.cppreference.com/w/cpp/language/initialization).
//   - dynamic initialisation otherwise, meaning that the variable is initialised the first time control
//     passes through its declaration:
//     http://eel.is/c++draft/stmt.dcl#4
// - destruction of objects with thread storage duration happens before the destruction of objects with
//   static storage duration:
//   http://eel.is/c++draft/basic.start.term#2
// - "All static initialization strongly happens before any dynamic initialization.":
//   http://eel.is/c++draft/basic.start#static-2
// - "If the completion of the constructor or dynamic initialization of an object with thread storage duration is
//   sequenced before that of another, the completion of the destructor of the second is sequenced before the initiation
//   of the destructor of the first." (that is, objects are destroyed in reverse with respect to construction order):
//   http://eel.is/c++draft/basic.start.term#3
// NOTE: because the ctor of mpz_alloc_cache is constexpr,
// then the init of this thread_local variable will happen
// before any dynamic initialisation:
// https://en.cppreference.com/w/cpp/language/constant_initialization
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
MPPP_CONSTINIT thread_local mpz_alloc_cache mpz_alloc_cache_inst;

// Implementation of the init of an mpz from cache.
bool mpz_init_from_cache_impl(mpz_struct_t &rop, std::size_t nlimbs)
{
    auto &mpzc = mpz_alloc_cache_inst;
    if (nlimbs != 0u && nlimbs <= mpzc.max_size && mpzc.sizes[nlimbs - 1u] != 0u) {
        // LCOV_EXCL_START
        if (mppp_unlikely(nlimbs > make_unsigned(nl_max<mpz_alloc_t>()))) {
            std::abort();
        }
        // LCOV_EXCL_STOP
        const auto idx = nlimbs - 1u;
        rop._mp_alloc = static_cast<mpz_alloc_t>(nlimbs);
        rop._mp_size = 0;
        rop._mp_d = mpzc.caches[idx][mpzc.sizes[idx] - 1u];
        --mpzc.sizes[idx];
        return true;
    }
    return false;
}

} // namespace

mpz_alloc_cache &get_thread_local_mpz_cache()
{
    return mpz_alloc_cache_inst;
}

#endif

void mpz_init_nlimbs(mpz_struct_t &rop, std::size_t nlimbs)
{
#if defined(MPPP_HAVE_THREAD_LOCAL)
    if (!mpz_init_from_cache_impl(rop, nlimbs)) {
#endif
        // LCOV_EXCL_START
        // A bit of horrid overflow checking.
        if (mppp_unlikely(nlimbs > nl_max<std::size_t>() / unsigned(GMP_NUMB_BITS))) {
            // NOTE: here we are doing what GMP does in case of memory allocation errors. It does not make much sense
            // to do anything else, as long as GMP does not provide error recovery.
            std::abort();
        }
        // LCOV_EXCL_STOP
        const auto nbits = static_cast<std::size_t>(unsigned(GMP_NUMB_BITS) * nlimbs);
        // LCOV_EXCL_START
        if (mppp_unlikely(nbits > nl_max<::mp_bitcnt_t>())) {
            std::abort();
        }
        // LCOV_EXCL_STOP
        // NOTE: nbits == 0 is allowed.
        mpz_init2(&rop, static_cast<::mp_bitcnt_t>(nbits));
        assert(make_unsigned_t<mpz_alloc_t>(rop._mp_alloc) >= nlimbs);
#if defined(MPPP_HAVE_THREAD_LOCAL)
    }
#endif
}

void mpz_init_nbits(mpz_struct_t &rop, ::mp_bitcnt_t nbits, std::size_t nlimbs)
{
    // Check nlimbs.
    assert(nlimbs == nbits_to_nlimbs(nbits));
#if defined(MPPP_HAVE_THREAD_LOCAL)
    if (!mpz_init_from_cache_impl(rop, nlimbs)) {
#endif
        ignore(nlimbs);
        // NOTE: nbits == 0 is allowed.
        mpz_init2(&rop, nbits);
#if defined(MPPP_HAVE_THREAD_LOCAL)
    }
#endif
}

void mpz_clear_wrap(mpz_struct_t &m)
{
#if defined(MPPP_HAVE_THREAD_LOCAL)
    auto &mpzc = mpz_alloc_cache_inst;
    const auto ualloc = make_unsigned(m._mp_alloc);
    if (ualloc != 0u && ualloc <= mpzc.max_size && mpzc.sizes[ualloc - 1u] < mpzc.max_entries) {
        const auto idx = ualloc - 1u;
        mpzc.caches[idx][mpzc.sizes[idx]] = m._mp_d;
        ++mpzc.sizes[idx];
    } else {
#endif
        mpz_clear(&m);
#if defined(MPPP_HAVE_THREAD_LOCAL)
    }
#endif
}

void mpz_to_str(std::vector<char> &out, const mpz_struct_t *mpz, int base)
{
    assert(base >= 2 && base <= 62);
    const auto size_base = mpz_sizeinbase(mpz, base);
    // LCOV_EXCL_START
    if (mppp_unlikely(size_base > nl_max<std::size_t>() - 2u)) {
        throw std::overflow_error("Too many digits in the conversion of mpz_t to string");
    }
    // LCOV_EXCL_STOP
    // Total max size is the size in base plus an optional sign and the null terminator.
    const auto total_size = size_base + 2u;
    // NOTE: possible improvement: use a null allocator to avoid initing the chars each time
    // we resize up.
    // Overflow check.
    // LCOV_EXCL_START
    if (mppp_unlikely(total_size > nl_max<std::vector<char>::size_type>())) {
        throw std::overflow_error("Too many digits in the conversion of mpz_t to string");
    }
    // LCOV_EXCL_STOP
    out.resize(static_cast<std::vector<char>::size_type>(total_size));
    mpz_get_str(out.data(), base, mpz);
}

std::ostream &integer_stream_operator_impl(std::ostream &os, const mpz_struct_t *n, int n_sgn)
{
    // Get the stream width.
    const auto width = os.width();

    // Fetch the stream's flags.
    const auto flags = os.flags();

    // Start by figuring out the base.
    const auto base = stream_flags_to_base(flags);

    // Should we prefix the base? Do it only if:
    // - the number is nonzero,
    // - the showbase flag is set,
    // - the base is not 10.
    const bool with_base_prefix = n_sgn != 0 && (flags & std::ios_base::showbase) != 0 && base != 10;

    // Uppercase?
    const bool uppercase = (flags & std::ios_base::uppercase) != 0;

    // Write out to a temporary vector in the required base. This will produce
    // a representation in the required base, with no base prefix and no
    // extra '+' for nonnegative integers.
    MPPP_MAYBE_TLS std::vector<char> tmp;
    mpz_to_str(tmp, n, base);
    // NOTE: tmp contains the terminator, and it might be
    // larger than needed. Make sure to shrink it so that
    // the last element is the terminator.
    tmp.resize(static_cast<decltype(tmp.size())>(std::strlen(tmp.data())) + 1u);

    if (n_sgn == -1) {
        // Negative number.
        if (with_base_prefix) {
            // If we need the base prefix, we will have to add the base after the minus sign.
            assert(tmp[0] == '-');
            if (base == 16) {
                constexpr std::array<char, 2> hex_prefix = {{'0', 'x'}};
                tmp.insert(tmp.begin() + 1, hex_prefix.begin(), hex_prefix.end());
            } else {
                tmp.insert(tmp.begin() + 1, '0');
            }
        }
    } else {
        // Nonnegative number. We will be prepending up to 3 characters to the number
        // representation:
        // - 1 or 2 for the base prefix ('0' for octal, '0x'/'0X' for hex),
        // - the '+' sign, if requested.
        const bool with_plus = (flags & std::ios_base::showpos) != 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        std::array<char, 3> prep_buffer;
        const auto prep_n = [&prep_buffer, with_plus, with_base_prefix, base]() -> std::size_t {
            std::size_t ret = 0;
            if (with_plus) {
                prep_buffer[ret++] = '+';
            }
            if (with_base_prefix) {
                prep_buffer[ret++] = '0';
                if (base == 16) {
                    prep_buffer[ret++] = 'x';
                }
            }
            return ret;
        }();
        // Prepend the additional characters.
        tmp.insert(tmp.begin(), prep_buffer.data(), prep_buffer.data() + prep_n);
    }

    // Apply a final toupper() transformation in base 16, if needed,
    // but do it before doing the filling in order to avoid
    // uppercasing the fill character.
    if (base == 16 && uppercase) {
        const auto &cloc = std::locale::classic();
        for (decltype(tmp.size()) i = 0; i < tmp.size() - 1u; ++i) {
            if (std::isalpha(tmp[i], cloc)) {
                tmp[i] = std::toupper(tmp[i], cloc);
            }
        }
    }

    // Compute the total size of the number
    // representation (i.e., without fill characters).
    // NOTE: -1 because of the terminator.
    assert(!tmp.empty());
    const auto final_size = tmp.size() - 1u;

    // We are going to do the filling
    // only if the stream width is larger
    // than the total size of the number.
    if (width >= 0 && make_unsigned(width) > final_size) {
        // Determine the fill type.
        const auto fill = stream_flags_to_fill(flags);
        // Compute how much fill we need.
        const auto fill_size = safe_cast<decltype(tmp.size())>(make_unsigned(width) - final_size);
        // Get the fill character.
        const auto fill_char = os.fill();

        switch (fill) {
            case 1:
                // Left fill: fill characters at the end.
                // NOTE: minus 1 because of the terminator.
                tmp.insert(tmp.end() - 1, fill_size, fill_char);
                break;
            case 2:
                // Right fill: fill characters at the beginning.
                tmp.insert(tmp.begin(), fill_size, fill_char);
                break;
            default: {
                assert(fill == 3);

                // Internal fill: the fill characters are always after the sign (if present)
                // and the base prefix (if present).
                auto delta = static_cast<int>(tmp[0] == '+' || tmp[0] == '-');
                if (with_base_prefix) {
                    delta += 1 + static_cast<int>(base == 16);
                }
                tmp.insert(tmp.begin() + delta, fill_size, fill_char);
                break;
            }
        }
    }

    // Write out the unformatted data.
    // NOTE: minus 1 because of the terminator.
    os.write(tmp.data(), safe_cast<std::streamsize>(tmp.size() - 1u));

    // Reset the stream width to zero, like the operator<<() does for builtin types.
    // https://en.cppreference.com/w/cpp/io/manip/setw
    // Do it here so we ensure we don't alter the state of the stream until the very end.
    os.width(0);

    return os;
}

} // namespace detail

void free_integer_caches()
{
#if defined(MPPP_HAVE_THREAD_LOCAL)
    detail::mpz_alloc_cache_inst.clear();
#endif
}

} // namespace mppp
