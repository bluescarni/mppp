// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// NOTE: in order to use the MPFR print functions,
// we must make sure that stdio.h is included before mpfr.h:
// https://www.mpfr.org/mpfr-current/mpfr.html#Formatted-Output-Functions
#include <cstdio>

#include <mp++/config.hpp>

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <ios>
#include <locale>
#include <ostream>
#include <sstream>
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

#endif

#include <mp++/complex.hpp>
#include <mp++/detail/mpc.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/parse_complex.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>
#include <mp++/real.hpp>

#if defined(MPPP_WITH_QUADMATH)

#include <mp++/complex128.hpp>
#include <mp++/real128.hpp>

#endif

namespace mppp
{

namespace detail
{

namespace
{

// Some misc tests to check that the mpc struct conforms to our expectations.
struct expected_mpc_struct_t {
    ::mpfr_t re;
    ::mpfr_t im;
};

static_assert(sizeof(expected_mpc_struct_t) == sizeof(mpc_struct_t) && offsetof(mpc_struct_t, re) == 0u
                  && offsetof(mpc_struct_t, im) == offsetof(expected_mpc_struct_t, im),
              "Invalid mpc_t struct layout and/or MPC types.");

// Double check that complex is a standard layout class.
static_assert(std::is_standard_layout<complex>::value, "complex is not a standard layout class.");

#if MPPP_CPLUSPLUS >= 201703L

// If we have C++17, we can use structured bindings to test the layout of mpc_struct_t
// and its members' types.
constexpr bool test_mpc_struct_t()
{
    auto [re, im] = mpc_struct_t{};

    static_assert(std::is_same<decltype(re), ::mpfr_t>::value);
    static_assert(std::is_same<decltype(im), ::mpfr_t>::value);

    ignore(re, im);

    return true;
}

static_assert(test_mpc_struct_t());

#endif

} // namespace

} // namespace detail

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
complex::complex()
{
    ::mpc_init2(&m_mpc, real_prec_min());
    ::mpfr_set_zero(mpc_realref(&m_mpc), 1);
    ::mpfr_set_zero(mpc_imagref(&m_mpc), 1);
}

// Init a complex with precision p, setting its value to (nan,nan). No precision
// checking is performed.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
complex::complex(const ptag &, ::mpfr_prec_t p, bool ignore_prec)
{
    assert(ignore_prec);
    assert(detail::real_prec_check(p));
    detail::ignore(ignore_prec);
    ::mpc_init2(&m_mpc, p);
}

complex::complex(const complex &other) : complex(&other.m_mpc) {}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
complex::complex(const complex &other, complex_prec_t p)
{
    // Init with custom precision, and then set.
    ::mpc_init2(&m_mpc, check_init_prec(static_cast<::mpfr_prec_t>(p)));
    ::mpc_set(&m_mpc, &other.m_mpc, MPC_RNDNN);
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
complex::complex(complex &&other, complex_prec_t p_)
{
    const auto p = static_cast<::mpfr_prec_t>(p_);

    // Check the precision first of all.
    check_init_prec(p);

    // Shallow copy other.
    m_mpc = other.m_mpc;
    // Mark the other as moved-from.
    other.m_mpc.re->_mpfr_d = nullptr;

    // Apply the precision.
    prec_round_impl<false>(p);
}

// Various helpers and constructors from string-like entities.
void complex::construct_from_c_string(const char *s, int base, ::mpfr_prec_t p)
{
    if (mppp_unlikely(base && (base < 2 || base > 62))) {
        throw std::invalid_argument("Cannot construct a complex from a string in base " + detail::to_string(base)
                                    + ": the base must either be zero or in the [2,62] range");
    }

    // Try parsing the real and imaginary parts.
    const auto res = detail::parse_complex(s);

    // Construct the real part.
    real re{res[0], res[1], base, p};

    // The imaginary part might not be present.
    auto im = (res[2] == nullptr) ? real{real_kind::zero, 1, p} : real{res[2], res[3], base, p};

    // Shallow-copy into this.
    m_mpc.re[0] = *re.get_mpfr_t();
    m_mpc.im[0] = *im.get_mpfr_t();

    // Deactivate the temporaries.
    re._get_mpfr_t()->_mpfr_d = nullptr;
    im._get_mpfr_t()->_mpfr_d = nullptr;
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
complex::complex(const stag &, const char *s, int base, ::mpfr_prec_t p)
{
    construct_from_c_string(s, base, p);
}

complex::complex(const stag &, const std::string &s, int base, ::mpfr_prec_t p)
    : complex(s.c_str(), base, complex_prec_t(p))
{
}

#if defined(MPPP_HAVE_STRING_VIEW)
complex::complex(const stag &, const std::string_view &s, int base, ::mpfr_prec_t p)
    : complex(s.data(), s.data() + s.size(), base, complex_prec_t(p))
{
}
#endif

// Constructor from range of characters, base and precision.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
complex::complex(const char *begin, const char *end, int base, complex_prec_t p)
{
    MPPP_MAYBE_TLS std::vector<char> buffer;
    buffer.assign(begin, end);
    buffer.emplace_back('\0');
    construct_from_c_string(buffer.data(), base, static_cast<::mpfr_prec_t>(p));
}

// Constructor from range of characters and precision.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
complex::complex(const char *begin, const char *end, complex_prec_t p) : complex(begin, end, 10, p) {}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
complex::complex(const ::mpc_t c)
{
    // Init with the same precision as other, and then set.
    ::mpc_init2(&m_mpc, mpfr_get_prec(mpc_realref(c)));
    ::mpc_set(&m_mpc, c, MPC_RNDNN);
}

// Copy assignment operator.
complex &complex::operator=(const complex &other)
{
    if (mppp_likely(this != &other)) {
        if (is_valid()) {
            // this has not been moved-from.
            // Copy the precision. This will also reset the internal value.
            // No need for prec checking as we assume other has a valid prec.
            set_prec_impl<false>(other.get_prec());
        } else {
            // this has been moved-from: init before setting.
            ::mpc_init2(&m_mpc, other.get_prec());
        }
        // Perform the actual copy from other.
        ::mpc_set(&m_mpc, &other.m_mpc, MPC_RNDNN);
    }
    return *this;
}

// Copy assignment from mpc_t.
complex &complex::operator=(const ::mpc_t c)
{
    // Set the precision, assuming the prec of c is valid.
    assert(mpfr_get_prec(mpc_realref(c)) == mpfr_get_prec(mpc_imagref(c)));
    set_prec_impl<false>(mpfr_get_prec(mpc_realref(c)));
    // Set the value.
    ::mpc_set(&m_mpc, c, MPC_RNDNN);
    return *this;
}

#if !defined(_MSC_VER) || defined(__clang__)

// Move assignment from mpc_t.
complex &complex::operator=(::mpc_t &&c)
{
    // Clear this.
    ::mpc_clear(&m_mpc);
    // Shallow copy c.
    m_mpc = *c;
    return *this;
}

#endif

complex &complex::set(const complex &c)
{
    return set(&c.m_mpc);
}

complex &complex::set(const ::mpc_t c)
{
    ::mpc_set(&m_mpc, c, MPC_RNDNN);
    return *this;
}

complex &complex::set_nan()
{
    ::mpc_set_nan(&m_mpc);

    return *this;
}

// Implementation of the assignment from string.
void complex::string_assignment_impl(const char *s, int base)
{
    if (mppp_unlikely(base && (base < 2 || base > 62))) {
        throw std::invalid_argument("Cannot assign a complex from a string in base " + detail::to_string(base)
                                    + ": the base must either be zero or in the [2,62] range");
    }

    re_ref re{*this};
    im_ref im{*this};

    try {
        // Try parsing the real and imaginary parts.
        const auto res = detail::parse_complex(s);

        // Set the real part.
        re->set(res[0], res[1], base);

        // Set the imaginary part, if present.
        if (res[2] == nullptr) {
            im->set_zero(1);
        } else {
            im->set(res[2], res[3], base);
        }
    } catch (...) {
        // In case of errors, make sure we set this to (nan,nan)
        // before re-throwing.
        re->set_nan();
        im->set_nan();

        throw;
    }
}

// Implementation of string setters.
complex &complex::set_impl(const char *s, int base)
{
    string_assignment_impl(s, base);
    return *this;
}

complex &complex::set_impl(const std::string &s, int base)
{
    return set(s.c_str(), base);
}

#if defined(MPPP_HAVE_STRING_VIEW)
complex &complex::set_impl(const std::string_view &s, int base)
{
    return set(s.data(), s.data() + s.size(), base);
}
#endif

// Set to character range.
complex &complex::set(const char *begin, const char *end, int base)
{
    MPPP_MAYBE_TLS std::vector<char> buffer;
    buffer.assign(begin, end);
    buffer.emplace_back('\0');
    return set(buffer.data(), base);
}

// Detect one.
bool complex::is_one() const
{
    re_cref re{*this};
    im_cref im{*this};

    return im->zero_p() && re->is_one();
}

// Destructively set the precision.
complex &complex::set_prec(::mpfr_prec_t p)
{
    set_prec_impl<true>(p);
    return *this;
}

// Set the precision maintaining the current value.
complex &complex::prec_round(::mpfr_prec_t p)
{
    prec_round_impl<true>(p);
    return *this;
}

std::string complex::to_string(int base) const
{
    re_cref re{*this};
    im_cref im{*this};

    return '(' + re->to_string(base) + ',' + im->to_string(base) + ')';
}

int cmpabs(const complex &c1, const complex &c2)
{
    ::mpfr_clear_erangeflag();
    auto retval = ::mpc_cmp_abs(c1.get_mpc_t(), c2.get_mpc_t());
    if (mppp_unlikely(::mpfr_erangeflag_p())) {
        ::mpfr_clear_erangeflag();
        throw std::domain_error(
            "Cannot compare the absolute values of two complex numbers if there are NaNs in the real/imaginary parts");
    }
    return retval;
}

std::ostream &operator<<(std::ostream &os, const complex &c)
{
    // Get the stream width.
    const auto width = os.width();

    // Fetch the stream's flags.
    const auto flags = os.flags();

    // Check if we are using scientific, fixed or both.
    const auto scientific = (flags & std::ios_base::scientific) != 0;
    const auto fixed = (flags & std::ios_base::fixed) != 0;
    const auto hexfloat = scientific && fixed;

    // Force decimal point character?
    const auto showpoint = (flags & std::ios_base::showpoint) != 0;

    // Force the plus sign?
    const bool with_plus = (flags & std::ios_base::showpos) != 0;

    // Uppercase?
    const bool uppercase = (flags & std::ios_base::uppercase) != 0;

    // Fetch the precision too.
    auto precision = os.precision();
    // NOTE: if the precision is negative, reset it
    // to the default value of 6.
    if (precision < 0) {
        precision = 6;
    }

    // Put together the format string.
    std::ostringstream oss;
    oss.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    oss.imbue(std::locale::classic());

    oss << '%';

    if (showpoint) {
        oss << '#';
    }

    if (with_plus) {
        oss << '+';
    }

    if (hexfloat) {
        // NOTE: in hexfloat mode, we want to ignore the precision
        // setting in the stream object and print the exact representation:
        // https://en.cppreference.com/w/cpp/locale/num_put/put
        oss << 'R';
    } else {
        oss << '.' << precision << 'R';
    }

    if (hexfloat) {
        oss << (uppercase ? 'A' : 'a');
    } else if (scientific) {
        oss << (uppercase ? 'E' : 'e');
    } else if (fixed) {
        // NOTE: in fixed format, the uppercase
        // setting is ignored for floating-point types:
        // https://en.cppreference.com/w/cpp/locale/num_put/put
        oss << 'f';
    } else {
        oss << (uppercase ? 'G' : 'g');
    }

    const auto fmt_str = oss.str();

    // The RAII struct to clean out the string allocated by mpfr_asprintf().
    // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
    struct out_str_cleaner {
        ~out_str_cleaner()
        {
            ::mpfr_free_str(ptr);
        }
        char *ptr;
    };

    // Init the tmp string.
    std::string tmp = "(";

    // Print the real and imaginary parts via mpfr_asprintf().
    char *out_str = nullptr;
    auto ret = ::mpfr_asprintf(&out_str, fmt_str.c_str(), mpc_realref(c.get_mpc_t()));
    if (ret == -1) {
        // LCOV_EXCL_START
        // NOTE: the MPFR docs state that if printf() returns -1, the errno variable and erange
        // flag are set. Let's clear them out before throwing.
        errno = 0;
        ::mpfr_clear_erangeflag();

        throw std::invalid_argument("The mpfr_asprintf() function returned the error code -1");
        // LCOV_EXCL_STOP
    }
    out_str_cleaner re_osc{out_str};

    tmp += out_str;
    tmp += ',';

    ret = ::mpfr_asprintf(&out_str, fmt_str.c_str(), mpc_imagref(c.get_mpc_t()));
    if (ret == -1) {
        // LCOV_EXCL_START
        // NOTE: the MPFR docs state that if printf() returns -1, the errno variable and erange
        // flag are set. Let's clear them out before throwing.
        errno = 0;
        ::mpfr_clear_erangeflag();

        throw std::invalid_argument("The mpfr_asprintf() function returned the error code -1");
        // LCOV_EXCL_STOP
    }
    out_str_cleaner im_osc{out_str};

    tmp += out_str;
    tmp += ')';

    // We are going to do the filling
    // only if the stream width is larger
    // than the total size of the string repr.
    if (width >= 0 && detail::make_unsigned(width) > tmp.size()) {
        // Determine the fill type.
        const auto fill = detail::stream_flags_to_fill(flags);

        // Compute how much fill we need.
        const auto fill_size = detail::safe_cast<decltype(tmp.size())>(detail::make_unsigned(width) - tmp.size());

        // Get the fill character.
        const auto fill_char = os.fill();

        if (fill == 1) {
            // Left fill: fill characters at the end.
            tmp.insert(tmp.end(), fill_size, fill_char);
        } else {
            assert(fill == 2 || fill == 3);

            // Right or internal fill: fill characters at the beginning.
            tmp.insert(tmp.begin(), fill_size, fill_char);
        }
    }

    os.write(tmp.data(), detail::safe_cast<std::streamsize>(tmp.size()));

    // Reset the stream width to zero, like the operator<<() does for builtin types.
    // https://en.cppreference.com/w/cpp/io/manip/setw
    // Do it here so we ensure we don't alter the state of the stream until the very end.
    os.width(0);

    return os;
}

#if defined(MPPP_MPFR_HAVE_MPFR_GET_STR_NDIGITS)

// Get the number of significant digits required for a round-tripping representation.
std::size_t complex::get_str_ndigits(int base) const
{
    re_cref re{*this};

    return re->get_str_ndigits(base);
}

std::size_t get_str_ndigits(const complex &c, int base)
{
    return c.get_str_ndigits(base);
}

#endif

complex &operator++(complex &c)
{
    if (mppp_unlikely(c.get_prec() < detail::real_deduce_precision(1))) {
        c.prec_round(detail::real_deduce_precision(1));
    }
    ::mpc_add_ui(c._get_mpc_t(), c.get_mpc_t(), 1ul, MPC_RNDNN);
    return c;
}

complex operator++(complex &c, int)
{
    auto retval(c);
    ++c;
    return retval;
}

complex &operator--(complex &c)
{
    if (mppp_unlikely(c.get_prec() < detail::real_deduce_precision(1))) {
        c.prec_round(detail::real_deduce_precision(1));
    }
    ::mpc_sub_ui(c._get_mpc_t(), c.get_mpc_t(), 1ul, MPC_RNDNN);
    return c;
}

complex operator--(complex &c, int)
{
    auto retval(c);
    --c;
    return retval;
}

namespace detail
{

void dispatch_complex_in_place_add(complex &a, const real &x)
{
    auto wrapper = [&x](::mpc_t c, const ::mpc_t o) { ::mpc_add_fr(c, o, x.get_mpfr_t(), MPC_RNDNN); };

    mpc_nary_op_impl<false>(x.get_prec(), wrapper, a, a);
}

void dispatch_complex_in_place_add(complex &a, bool n)
{
    auto wrapper = [n](::mpc_t c, const ::mpc_t o) { ::mpc_add_ui(c, o, static_cast<unsigned long>(n), MPC_RNDNN); };

    mpc_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

void dispatch_complex_in_place_sub(complex &a, const real &x)
{
    auto wrapper = [&x](::mpc_t c, const ::mpc_t o) { ::mpc_sub_fr(c, o, x.get_mpfr_t(), MPC_RNDNN); };

    mpc_nary_op_impl<false>(x.get_prec(), wrapper, a, a);
}

void dispatch_complex_in_place_sub(complex &a, bool n)
{
    auto wrapper = [n](::mpc_t c, const ::mpc_t o) { ::mpc_sub_ui(c, o, static_cast<unsigned long>(n), MPC_RNDNN); };

    mpc_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

void dispatch_complex_in_place_mul(complex &a, const real &x)
{
    auto wrapper = [&x](::mpc_t c, const ::mpc_t o) { ::mpc_mul_fr(c, o, x.get_mpfr_t(), MPC_RNDNN); };

    mpc_nary_op_impl<false>(x.get_prec(), wrapper, a, a);
}

void dispatch_complex_in_place_mul(complex &a, bool n)
{
    auto wrapper = [n](::mpc_t c, const ::mpc_t o) { ::mpc_mul_ui(c, o, static_cast<unsigned long>(n), MPC_RNDNN); };

    mpc_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

void dispatch_complex_in_place_div(complex &a, const real &x)
{
    auto wrapper = [&x](::mpc_t c, const ::mpc_t o) { ::mpc_div_fr(c, o, x.get_mpfr_t(), MPC_RNDNN); };

    mpc_nary_op_impl<false>(x.get_prec(), wrapper, a, a);
}

void dispatch_complex_in_place_div(complex &a, bool n)
{
    auto wrapper = [n](::mpc_t c, const ::mpc_t o) { ::mpc_div_ui(c, o, static_cast<unsigned long>(n), MPC_RNDNN); };

    mpc_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

bool dispatch_complex_equality(const complex &c1, const complex &c2)
{
    // NOTE: don't use mpc_cmp() as it does not handle NaNs properly.
    complex::re_cref re1{c1}, re2{c2};
    complex::im_cref im1{c1}, im2{c2};

    return *re1 == *re2 && *im1 == *im2;
}

} // namespace detail

// Wrapper to apply the input unary MPC function to this with
// MPC_RNDNN rounding mode. Returns a reference to this.
template <typename T>
complex &complex::self_mpc_unary(T &&f)
{
    std::forward<T>(f)(&m_mpc, &m_mpc, MPC_RNDNN);
    return *this;
}

// In-place negation.
complex &complex::neg()
{
    return self_mpc_unary(::mpc_neg);
}

// In-place conjugate.
complex &complex::conj()
{
    return self_mpc_unary(::mpc_conj);
}

// In-place absolute value.
complex &complex::abs()
{
    MPPP_MAYBE_TLS real tmp;

    tmp.set_prec(get_prec());
    ::mpc_abs(tmp._get_mpfr_t(), &m_mpc, MPFR_RNDN);

    mpfr_set(mpc_realref(&m_mpc), tmp.get_mpfr_t(), MPFR_RNDN);
    ::mpfr_set_zero(mpc_imagref(&m_mpc), 1);

    return *this;
}

// In-place norm.
complex &complex::norm()
{
    MPPP_MAYBE_TLS real tmp;

    tmp.set_prec(get_prec());
    ::mpc_norm(tmp._get_mpfr_t(), &m_mpc, MPFR_RNDN);

    mpfr_set(mpc_realref(&m_mpc), tmp.get_mpfr_t(), MPFR_RNDN);
    ::mpfr_set_zero(mpc_imagref(&m_mpc), 1);

    return *this;
}

// In-place arg.
complex &complex::arg()
{
    MPPP_MAYBE_TLS real tmp;

    tmp.set_prec(get_prec());
    ::mpc_arg(tmp._get_mpfr_t(), &m_mpc, MPFR_RNDN);

    mpfr_set(mpc_realref(&m_mpc), tmp.get_mpfr_t(), MPFR_RNDN);
    ::mpfr_set_zero(mpc_imagref(&m_mpc), 1);

    return *this;
}

// In-place proj.
complex &complex::proj()
{
    return self_mpc_unary(::mpc_proj);
}

// In-place squaring.
complex &complex::sqr()
{
    return self_mpc_unary(::mpc_sqr);
}

// In-place mul_i.
complex &complex::mul_i(int sgn)
{
    ::mpc_mul_i(&m_mpc, &m_mpc, sgn, MPC_RNDNN);
    return *this;
}

// In-place sqrt.
complex &complex::sqrt()
{
    return self_mpc_unary(::mpc_sqrt);
}

// In-place exp.
complex &complex::exp()
{
    return self_mpc_unary(::mpc_exp);
}

// In-place log.
complex &complex::log()
{
    return self_mpc_unary(::mpc_log);
}

// In-place log10.
complex &complex::log10()
{
    return self_mpc_unary(::mpc_log10);
}

// In-place trig.
complex &complex::sin()
{
    return self_mpc_unary(::mpc_sin);
}

complex &complex::cos()
{
    return self_mpc_unary(::mpc_cos);
}

complex &complex::tan()
{
    return self_mpc_unary(::mpc_tan);
}

complex &complex::asin()
{
    return self_mpc_unary(::mpc_asin);
}

complex &complex::acos()
{
    return self_mpc_unary(::mpc_acos);
}

complex &complex::atan()
{
    return self_mpc_unary(::mpc_atan);
}

// In-place hyper.
complex &complex::sinh()
{
    return self_mpc_unary(::mpc_sinh);
}

complex &complex::cosh()
{
    return self_mpc_unary(::mpc_cosh);
}

complex &complex::tanh()
{
    return self_mpc_unary(::mpc_tanh);
}

complex &complex::asinh()
{
    return self_mpc_unary(::mpc_asinh);
}

complex &complex::acosh()
{
    return self_mpc_unary(::mpc_acosh);
}

complex &complex::atanh()
{
    return self_mpc_unary(::mpc_atanh);
}

complex &set_rootofunity(complex &c, unsigned long n, unsigned long k)
{
    ::mpc_rootofunity(c._get_mpc_t(), n, k, MPC_RNDNN);
    return c;
}

// Free-function abs.
real &abs(real &rop, const complex &c)
{
    rop.set_prec(c.get_prec());
    ::mpc_abs(rop._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return rop;
}

real abs(const complex &c)
{
    real ret{real_kind::nan, c.get_prec()};
    ::mpc_abs(ret._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return ret;
}

// Free-function norm.
real &norm(real &rop, const complex &c)
{
    rop.set_prec(c.get_prec());
    ::mpc_norm(rop._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return rop;
}

real norm(const complex &c)
{
    real ret{real_kind::nan, c.get_prec()};
    ::mpc_norm(ret._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return ret;
}

// Free-function arg.
real &arg(real &rop, const complex &c)
{
    rop.set_prec(c.get_prec());
    ::mpc_arg(rop._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return rop;
}

real arg(const complex &c)
{
    real ret{real_kind::nan, c.get_prec()};
    ::mpc_arg(ret._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return ret;
}

// Implementations of the assignments of complex to other mp++ classes.

real &real::operator=(const complex &c)
{
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
    return *this = static_cast<real>(c);
}

#if defined(MPPP_WITH_QUADMATH)

real128 &real128::operator=(const complex &c)
{
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
    return *this = static_cast<real128>(c);
}

complex128 &complex128::operator=(const complex &c)
{
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
    return *this = static_cast<complex128>(c);
}

#endif

// Extract copies of the real/imaginary parts.
std::pair<real, real> complex::get_real_imag() const &
{
    return std::make_pair(*re_cref{*this}, *im_cref{*this});
}

#if defined(MPPP_WITH_BOOST_S11N)

void complex::load(boost::archive::binary_iarchive &ar, unsigned)
{
    // NOTE: for the binary archive, don't pass through the constructor,
    // but assign directly the re/im members.
    MPPP_MAYBE_TLS real re, im;

    ar >> re;
    ar >> im;

    {
        re_ref rr{*this};
        im_ref ir{*this};

        *rr = re;
        *ir = im;
    }
}

#endif

} // namespace mppp
