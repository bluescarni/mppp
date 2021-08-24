// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <atomic>
#include <cmath>
#include <complex>
#include <initializer_list>
#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>
#include <mp++/type_name.hpp>

#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

// NOLINTNEXTLINE(cert-err58-cpp, cert-msc32-c, cert-msc51-cpp, cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937 rng;

static const int ntrials = 1000;

using int_types = std::tuple<char, signed char, unsigned char, short, unsigned short, int, unsigned, long,
                             unsigned long, long long, unsigned long long, wchar_t
#if defined(MPPP_HAVE_GCC_INT128)
                             ,
                             __int128_t, __uint128_t
#endif
                             >;

using fp_types = std::tuple<float, double, long double>;

using int_t = integer<1>;
using rat_t = rational<1>;

// Base-10 string representation at full precision for a float.
template <typename T>
static inline std::string f2str(const T &x)
{
    std::ostringstream oss;
    oss << std::setprecision(std::numeric_limits<T>::max_digits10) << x;
    return oss.str();
}

constexpr bool ppc_arch =
#if defined(_ARCH_PPC)
    true
#else
    false
#endif
    ;

struct int_ctor_tester {
    template <typename T>
    void operator()(const T &) const
    {
        REQUIRE(real{T(0)}.zero_p());
        REQUIRE(!real{T(0)}.signbit());
        REQUIRE(real{T(0)}.get_prec() == detail::nl_digits<T>() + detail::is_signed<T>::value);
        REQUIRE((real{T(0), ::mpfr_prec_t(100)}.zero_p()));
        REQUIRE((real{T(0), ::mpfr_prec_t(100)}.get_prec() == 100));
        REQUIRE_THROWS_PREDICATE((real{T(0), 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                          + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + detail::to_string(real_prec_min());
        });
        REQUIRE_THROWS_PREDICATE((real{T(0), -1}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of -1: the maximum allowed precision is "
                          + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + detail::to_string(real_prec_min());
        });

        auto int_dist = integral_minmax_dist<T>{};
        for (int i = 0; i < ntrials; ++i) {
            auto n = int_dist(rng);
            REQUIRE(::mpfr_equal_p(real{n}.get_mpfr_t(),
                                   real{detail::to_string(n), 10, detail::nl_digits<T>()}.get_mpfr_t()));
            REQUIRE(::mpfr_equal_p(real{n, detail::nl_digits<T>() + 100}.get_mpfr_t(),
                                   real{detail::to_string(n), 10, detail::nl_digits<T>()}.get_mpfr_t()));
        }

        // Test for implicit construction.
        real r = T(42);
        REQUIRE(r == 42);
    }
};

struct fp_ctor_tester {
    template <typename T>
    void operator()(const T &) const
    {
        REQUIRE(real{T(0)}.zero_p());
        if (std::numeric_limits<T>::radix == 2) {
            REQUIRE(real{T(0)}.get_prec() == detail::nl_digits<T>());
        }
        if (std::numeric_limits<T>::is_iec559) {
            REQUIRE(!real{T(0)}.signbit());
            REQUIRE(real{-T(0)}.zero_p());
            REQUIRE(real{-T(0)}.signbit());
            REQUIRE(real{std::numeric_limits<T>::infinity()}.inf_p());
            REQUIRE(real{std::numeric_limits<T>::infinity()}.sgn() > 0);
            REQUIRE(real{-std::numeric_limits<T>::infinity()}.inf_p());
            REQUIRE(real{-std::numeric_limits<T>::infinity()}.sgn() < 0);
            REQUIRE(real{std::numeric_limits<T>::quiet_NaN()}.nan_p());
        }
        REQUIRE((real{T(0), ::mpfr_prec_t(100)}.zero_p()));
        REQUIRE((real{T(0), ::mpfr_prec_t(100)}.get_prec() == 100));
        REQUIRE_THROWS_PREDICATE((real{T(0), 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                          + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + detail::to_string(real_prec_min());
        });
        REQUIRE_THROWS_PREDICATE((real{T(0), -1}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of -1: the maximum allowed precision is "
                          + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + detail::to_string(real_prec_min());
        });

        if (std::numeric_limits<T>::radix != 2 || (ppc_arch && std::is_same<T, long double>::value)) {
            return;
        }

        // Test for implicit construction.
        real r = T(-1.5);
        REQUIRE(r == -1.5);

        std::uniform_real_distribution<T> dist(-T(100), T(100));
        for (int i = 0; i < ntrials; ++i) {
            auto x = dist(rng);
            REQUIRE(::mpfr_equal_p(real{x}.get_mpfr_t(), real{f2str(x), 10, detail::nl_digits<T>()}.get_mpfr_t()));
            REQUIRE(::mpfr_equal_p(real{x, detail::nl_digits<T>() + 100}.get_mpfr_t(),
                                   real{f2str(x), 10, detail::nl_digits<T>()}.get_mpfr_t()));
        }
    }
};

struct foobar {
};

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real constructors")
{
    REQUIRE((!std::is_constructible<real, foobar>::value));
    // Default constructor.
    real r1;
    REQUIRE(r1.is_valid());
    REQUIRE(r1.get_prec() == real_prec_min());
    REQUIRE(r1.zero_p());
    REQUIRE(!r1.signbit());
    // Copy ctor.
    real r3{real{4}};
    REQUIRE(r3.is_valid());
    REQUIRE(::mpfr_equal_p(r3.get_mpfr_t(), real{4}.get_mpfr_t()));
    REQUIRE(r3.get_prec() == real{4}.get_prec());
    real r4{real{4, 123}};
    REQUIRE(::mpfr_equal_p(r4.get_mpfr_t(), real{4, 123}.get_mpfr_t()));
    REQUIRE(r4.get_prec() == 123);
    // Copy ctor with different precision.
    real r5{real{4}, 512};
    REQUIRE(::mpfr_equal_p(r5.get_mpfr_t(), real{4}.get_mpfr_t()));
    REQUIRE(r5.get_prec() == 512);
    if (std::numeric_limits<double>::radix == 2 && detail::nl_digits<double>() > 12) {
        real r6{real{1.3}, 12};
        REQUIRE(!::mpfr_equal_p(r6.get_mpfr_t(), real{1.3}.get_mpfr_t()));
        REQUIRE(r6.get_prec() == 12);
    }
    REQUIRE_THROWS_PREDICATE(
        (real{static_cast<const real &>(real{4}), -1}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of -1: the maximum allowed precision is "
                          + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + detail::to_string(real_prec_min());
        });
    REQUIRE_THROWS_PREDICATE(
        (real{static_cast<const real &>(real{4}), 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                          + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + detail::to_string(real_prec_min());
        });
    if (real_prec_min() > 1) {
        REQUIRE_THROWS_PREDICATE(
            (real{static_cast<const real &>(real{4}), 1}), std::invalid_argument, [](const std::invalid_argument &ex) {
                return ex.what()
                       == "Cannot init a real with a precision of 1: the maximum allowed precision is "
                              + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                              + detail::to_string(real_prec_min());
            });
    }
    if (real_prec_max() < detail::nl_max<::mpfr_prec_t>()) {
        REQUIRE_THROWS_PREDICATE(
            (real{static_cast<const real &>(real{4}), detail::nl_max<::mpfr_prec_t>()}), std::invalid_argument,
            [](const std::invalid_argument &ex) {
                return ex.what()
                       == "Cannot init a real with a precision of " + detail::to_string(detail::nl_max<::mpfr_prec_t>())
                              + ": the maximum allowed precision is " + detail::to_string(real_prec_max())
                              + ", the minimum allowed precision is " + detail::to_string(real_prec_min());
            });
    }
    // Move constructor.
    real r7{real{123}};
    REQUIRE(::mpfr_equal_p(r7.get_mpfr_t(), real{123}.get_mpfr_t()));
    REQUIRE(r7.get_prec() == real{123}.get_prec());
    real r8{42, 50}, r9{std::move(r8)};
    REQUIRE(::mpfr_equal_p(r9.get_mpfr_t(), real{42, 50}.get_mpfr_t()));
    REQUIRE(r9.get_prec() == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r8.is_valid());
    // Revive via assignments.
    r8 = r9;
    REQUIRE(r8.is_valid());
    real r8a(std::move(r8));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r8.is_valid());
    r8 = std::move(r8a);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r8a.is_valid());
    REQUIRE(r8.is_valid());
    // Move ctor with different precision.
    REQUIRE(real{real{42}, 512}.get_prec() == 512);
    REQUIRE(real{real{42}, 512} == 42);
    REQUIRE(real{real{3}, real_prec_min()}.get_prec() == real_prec_min());
    REQUIRE(real{real{1}, real_prec_min()} == 1);
    real r8b{42};
    real r8c{std::move(r8b), 123};
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r8b.is_valid());
    REQUIRE(r8c == 42);
    REQUIRE(r8c.get_prec() == 123);
    // Revive via move assignment.
    r8b = real{56, 87};
    REQUIRE(r8b.is_valid());
    REQUIRE(r8b == 56);
    REQUIRE(r8b.get_prec() == 87);
    real r8d{std::move(r8b), 95};
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r8b.is_valid());
    REQUIRE(r8d == 56);
    REQUIRE(r8d.get_prec() == 95);
    // Revive via copy assignment.
    r8b = r8d;
    REQUIRE(r8b.is_valid());
    REQUIRE(r8b == 56);
    REQUIRE(r8b.get_prec() == 95);
    REQUIRE_THROWS_PREDICATE((real{real{4}, -5}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of -5: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{real{4}, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    // String constructors.
    REQUIRE((::mpfr_equal_p(real{"123", 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{"123", 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((real{"123", 100}.get_prec() == 100));
    REQUIRE((real{std::string{"123"}, 111}.get_prec() == 111));
    REQUIRE((::mpfr_equal_p(real{std::string{"123"}, 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
#if defined(MPPP_HAVE_STRING_VIEW)
    REQUIRE((::mpfr_equal_p(real{std::string_view{"123"}, 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((real{std::string_view{"123"}, 10, 100}.get_prec() == 100));
    REQUIRE((::mpfr_equal_p(real{std::string_view{"123"}, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((real{std::string_view{"123"}, 100}.get_prec() == 100));
#endif
    // Leading whitespaces are ok.
    REQUIRE((::mpfr_equal_p(real{"   123", 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{std::string{"   123"}, 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
#if defined(MPPP_HAVE_STRING_VIEW)
    REQUIRE((::mpfr_equal_p(real{std::string_view{"   123"}, 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
#endif
    REQUIRE((real{"123", 10, 100}.get_prec() == 100));
    REQUIRE((real{std::string{"123"}, 10, 100}.get_prec() == 100));
#if defined(MPPP_HAVE_STRING_VIEW)
    REQUIRE((real{std::string_view{"123"}, 10, 100}.get_prec() == 100));
#endif
    REQUIRE((::mpfr_equal_p(real{"-1.23E2", 10, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{std::string{"-1.23E2"}, 10, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
#if defined(MPPP_HAVE_STRING_VIEW)
    REQUIRE((::mpfr_equal_p(real{std::string_view{"-1.23E2"}, 10, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
#endif
    if (std::numeric_limits<double>::radix == 2) {
        REQUIRE((::mpfr_equal_p(real{"5E-1", 10, 100}.get_mpfr_t(), real{.5}.get_mpfr_t())));
        REQUIRE((::mpfr_equal_p(real{"-25e-2", 10, 100}.get_mpfr_t(), real{-.25}.get_mpfr_t())));
        REQUIRE((::mpfr_equal_p(real{std::string{"-25e-2"}, 10, 100}.get_mpfr_t(), real{-.25}.get_mpfr_t())));
#if defined(MPPP_HAVE_STRING_VIEW)
        REQUIRE((::mpfr_equal_p(real{std::string_view{"-25e-2"}, 10, 100}.get_mpfr_t(), real{-.25}.get_mpfr_t())));
#endif
    }
    REQUIRE((::mpfr_equal_p(real{"-11120", 3, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{"-11120", 3, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{"1111011", 2, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((real{"nan", 10, 42}.nan_p()));
    REQUIRE((real{"inf", 10, 42}.inf_p()));
    REQUIRE((real{"-inf", 10, 42}.inf_p()));
    REQUIRE((real{"inf", 10, 42}.sgn() > 0));
    REQUIRE((real{"-inf", 10, 42}.sgn() < 0));
    REQUIRE_THROWS_PREDICATE((real{"12", -1, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string("Cannot construct a real from a string in base -1: the base must either be zero or in "
                              "the [2,62] range");
    });
    REQUIRE_THROWS_PREDICATE((real{"12", 80, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string("Cannot construct a real from a string in base 80: the base must either be zero or in "
                              "the [2,62] range");
    });
    REQUIRE_THROWS_PREDICATE(
        (real{std::string{"12"}, -1, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string(
                       "Cannot construct a real from a string in base -1: the base must either be zero or in "
                       "the [2,62] range");
        });
#if defined(MPPP_HAVE_STRING_VIEW)
    REQUIRE_THROWS_PREDICATE(
        (real{std::string_view{"12"}, -1, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string(
                       "Cannot construct a real from a string in base -1: the base must either be zero or in "
                       "the [2,62] range");
        });
#endif
    REQUIRE_THROWS_PREDICATE(
        (real{std::string{"12"}, 80, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string(
                       "Cannot construct a real from a string in base 80: the base must either be zero or in "
                       "the [2,62] range");
        });
    REQUIRE_THROWS_PREDICATE((real{"12", 10, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{"123", 10, -100}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of -100: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{"123", -100}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of -100: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{"123", 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{"hell-o", 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what() == std::string("The string 'hell-o' does not represent a valid real in base 10");
    });
    REQUIRE_THROWS_PREDICATE(
        (real{std::string{"123"}, 10, -100}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of -100: the maximum allowed precision is "
                          + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + detail::to_string(real_prec_min());
        });
    REQUIRE_THROWS_PREDICATE(
        (real{std::string{"hell-o"}, 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what() == std::string("The string 'hell-o' does not represent a valid real in base 10");
        });
    REQUIRE_THROWS_PREDICATE((real{"123 ", 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what() == std::string("The string '123 ' does not represent a valid real in base 10");
    });
    REQUIRE_THROWS_PREDICATE((real{" 123 ", 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what() == std::string("The string ' 123 ' does not represent a valid real in base 10");
    });
    const std::vector<char> vc = {',', '-', '1', '2', '3', '4'};
    REQUIRE((::mpfr_equal_p(real{vc.data() + 2, vc.data() + 6, 10, 100}.get_mpfr_t(), real{1234}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{vc.data() + 1, vc.data() + 5, 10, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{vc.data() + 2, vc.data() + 6, 100}.get_mpfr_t(), real{1234}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{vc.data() + 1, vc.data() + 5, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE((real{vc.data() + 1, vc.data() + 5, 100}.get_prec() == 100));
    REQUIRE((real{vc.data() + 2, vc.data() + 6, 100}.get_prec() == 100));
    REQUIRE_THROWS_PREDICATE(
        (real{vc.data(), vc.data() + 6, 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what() == std::string("The string ',-1234' does not represent a valid real in base 10");
        });

    const std::vector<char> vc2 = {'1', '2', '3', '4'};
    REQUIRE_THROWS_PREDICATE(
        (real{vc2.data(), vc2.data() + 4, 10, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                          + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + detail::to_string(real_prec_min());
        });
    REQUIRE_THROWS_PREDICATE(
        (real{vc2.data(), vc2.data() + 4, -10}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of -10: the maximum allowed precision is "
                          + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + detail::to_string(real_prec_min());
        });
#if defined(MPPP_HAVE_STRING_VIEW)
    REQUIRE((::mpfr_equal_p(real{std::string_view{vc.data() + 2, 4}, 10, 100}.get_mpfr_t(), real{1234}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{std::string_view{vc.data() + 1, 4}, 10, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE_THROWS_PREDICATE(
        (real{std::string_view{vc.data(), 6}, 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what() == std::string("The string ',-1234' does not represent a valid real in base 10");
        });
#endif
    tuple_for_each(int_types{}, int_ctor_tester{});
    tuple_for_each(fp_types{}, fp_ctor_tester{});
    // Special handling of bool.
    REQUIRE(real{false}.zero_p());
    REQUIRE(real{false}.get_prec() == detail::c_max(::mpfr_prec_t(detail::nl_digits<bool>()), real_prec_min()));
    REQUIRE(mpfr_cmp_ui(real{true}.get_mpfr_t(), 1ul) == 0);
    REQUIRE(real{true}.get_prec() == detail::c_max(::mpfr_prec_t(detail::nl_digits<bool>()), real_prec_min()));
    REQUIRE((real{false, ::mpfr_prec_t(128)}.zero_p()));
    REQUIRE((real{false, ::mpfr_prec_t(128)}.get_prec() == 128));
    REQUIRE(mpfr_cmp_ui((real{true, ::mpfr_prec_t(128)}).get_mpfr_t(), 1ul) == 0);
    REQUIRE((real{true, ::mpfr_prec_t(128)}.get_prec() == 128));
    REQUIRE_THROWS_PREDICATE((real{false, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });

    // Implicit ctor from bool.
    {
        real r00 = true;
        REQUIRE(r00 == 1);

        real r11 = false;
        REQUIRE(r11 == 0);
    }

    // Construction from integer.
    REQUIRE(real{int_t{}}.zero_p());
    REQUIRE(real{int_t{}}.get_prec() == real_prec_min());
    REQUIRE(mpfr_cmp_ui((real{int_t{1}}).get_mpfr_t(), 1ul) == 0);
    REQUIRE(real{int_t{1}}.get_prec() == GMP_NUMB_BITS);
    REQUIRE(mpfr_cmp_ui((real{int_t{42}}).get_mpfr_t(), 42ul) == 0);
    REQUIRE(real{int_t{42}}.get_prec() == GMP_NUMB_BITS);
    REQUIRE(mpfr_cmp_si((real{-int_t{1}}).get_mpfr_t(), -1l) == 0);
    REQUIRE(real{int_t{-1}}.get_prec() == GMP_NUMB_BITS);
    REQUIRE(mpfr_cmp_si((real{-int_t{42}}).get_mpfr_t(), -42l) == 0);
    REQUIRE(real{int_t{-42}}.get_prec() == GMP_NUMB_BITS);
    real r0{int_t{42} << GMP_NUMB_BITS};
    REQUIRE(r0.get_prec() == 2 * GMP_NUMB_BITS);
    real tmp{42};
    ::mpfr_mul_2ui(tmp._get_mpfr_t(), tmp.get_mpfr_t(), GMP_NUMB_BITS, MPFR_RNDN);
    REQUIRE((::mpfr_equal_p(tmp.get_mpfr_t(), r0.get_mpfr_t())));
    tmp = real{-42};
    r0 = real{int_t{-42} << GMP_NUMB_BITS};
    ::mpfr_mul_2ui(tmp._get_mpfr_t(), tmp.get_mpfr_t(), GMP_NUMB_BITS, MPFR_RNDN);
    REQUIRE((::mpfr_equal_p(tmp.get_mpfr_t(), r0.get_mpfr_t())));
    REQUIRE_THROWS_PREDICATE((real{int_t{}, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{int_t{}, -1}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of -1: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });

    // Implicit ctor.
    {
        real r00 = 123_z1;
        REQUIRE(r00 == 123);
    }

    // Construction from rational.
    REQUIRE(real{rat_t{}}.zero_p());
    REQUIRE(real{rat_t{}}.get_prec() == GMP_NUMB_BITS);
    REQUIRE(mpfr_cmp_ui((real{rat_t{1}}).get_mpfr_t(), 1ul) == 0);
    REQUIRE(real{rat_t{1}}.get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE(mpfr_cmp_ui((real{rat_t{42}}).get_mpfr_t(), 42ul) == 0);
    REQUIRE(real{rat_t{42}}.get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE(mpfr_cmp_si((real{-rat_t{1}}).get_mpfr_t(), -1l) == 0);
    REQUIRE(real{rat_t{-1}}.get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE(mpfr_cmp_si((real{-rat_t{42}}).get_mpfr_t(), -42l) == 0);
    REQUIRE(real{rat_t{-42}}.get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE((::mpfr_equal_p((real{rat_t{5, 2}}).get_mpfr_t(), (real{"2.5", 10, 64}).get_mpfr_t())));
    REQUIRE((real{rat_t{5, 2}}.get_prec()) == GMP_NUMB_BITS * 2);
    REQUIRE((::mpfr_equal_p((real{rat_t{5, -2}}).get_mpfr_t(), (real{"-25e-1", 10, 64}).get_mpfr_t())));
    REQUIRE((real{rat_t{-5, 2}}).get_prec() == GMP_NUMB_BITS * 2);
    tmp = real{42, GMP_NUMB_BITS * 3};
    r0 = real{rat_t{int_t{42} << GMP_NUMB_BITS, 5}};
    ::mpfr_mul_2ui(tmp._get_mpfr_t(), tmp.get_mpfr_t(), GMP_NUMB_BITS, MPFR_RNDN);
    mpfr_div_ui(tmp._get_mpfr_t(), tmp.get_mpfr_t(), 5ul, MPFR_RNDN);
    REQUIRE((::mpfr_equal_p(tmp.get_mpfr_t(), r0.get_mpfr_t())));
    REQUIRE(r0.get_prec() == GMP_NUMB_BITS * 3);
    REQUIRE_THROWS_PREDICATE((real{rat_t{}, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{rat_t{}, -1}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of -1: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });

    // Implicit ctor.
    {
        real r00 = 123_q1;
        REQUIRE(r00 == 123);
    }

#if defined(MPPP_WITH_QUADMATH)
    REQUIRE(real{real128{}}.zero_p());
    REQUIRE(real{-real128{}}.zero_p());
    REQUIRE(!real{real128{}}.signbit());
    REQUIRE(real{-real128{}}.signbit());
    REQUIRE(real{real128{}}.get_prec() == 113);
    REQUIRE(real{real128{-1}}.get_prec() == 113);
    REQUIRE(real{real128{1}}.get_prec() == 113);
    REQUIRE(mpfr_cmp_ui((real{real128{1}}).get_mpfr_t(), 1ul) == 0);
    REQUIRE(mpfr_cmp_si((real{real128{-1}}).get_mpfr_t(), -1l) == 0);
    REQUIRE(mpfr_cmp_ui((real{real128{1123}}).get_mpfr_t(), 1123ul) == 0);
    REQUIRE(mpfr_cmp_si((real{real128{-1123}}).get_mpfr_t(), -1123l) == 0);
    REQUIRE(real{real128_inf()}.inf_p());
    REQUIRE(real{real128_inf()}.sgn() > 0);
    REQUIRE(real{-real128_inf()}.inf_p());
    REQUIRE(real{-real128_inf()}.sgn() < 0);
    REQUIRE(real{real128_nan()}.nan_p());
    REQUIRE(::mpfr_equal_p(real{real128{"3.40917866435610111081769936359662259e-2"}}.get_mpfr_t(),
                           real{"3.40917866435610111081769936359662259e-2", 10, 113}.get_mpfr_t()));
    REQUIRE(::mpfr_equal_p(real{-real128{"3.40917866435610111081769936359662259e-2"}}.get_mpfr_t(),
                           real{"-3.40917866435610111081769936359662259e-2", 10, 113}.get_mpfr_t()));
    // Subnormal values.
    REQUIRE(::mpfr_equal_p(real{real128{"3.40917866435610111081769936359662259e-4957"}}.get_mpfr_t(),
                           real{"3.40917866435610111081769936359662259e-4957", 10, 113}.get_mpfr_t()));
    REQUIRE(::mpfr_equal_p(real{-real128{"3.40917866435610111081769936359662259e-4957"}}.get_mpfr_t(),
                           real{"-3.40917866435610111081769936359662259e-4957", 10, 113}.get_mpfr_t()));
    // Custom precision.
    REQUIRE((real{real128{"3.40917866435610111081769936359662259e-2"}, 64}).get_prec() == 64);
    REQUIRE((real{real128{"-3.40917866435610111081769936359662259e-2"}, 64}).get_prec() == 64);
    REQUIRE(::mpfr_equal_p(real{real128{"3.40917866435610111081769936359662259e-2"}, 64}.get_mpfr_t(),
                           real{"3.40917866435610111081769936359662259e-2", 10, 64}.get_mpfr_t()));
    REQUIRE(::mpfr_equal_p(real{-real128{"3.40917866435610111081769936359662259e-2"}, 64}.get_mpfr_t(),
                           real{"-3.40917866435610111081769936359662259e-2", 10, 64}.get_mpfr_t()));
    REQUIRE_THROWS_PREDICATE((real{real128{}, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{real128{}, -1}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of -1: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });

    // Implicit ctor.
    {
        real r00 = -123_rq;
        REQUIRE(r00 == -123);
    }
#endif
    // Constructor from mpfr_t.
    ::mpfr_t m;
    ::mpfr_init2(m, 123);
    mpfr_set_ui(m, 42ul, MPFR_RNDN);
    real rtmp{m};
    REQUIRE(rtmp.get_prec() == 123);
    REQUIRE(mpfr_cmp_ui(rtmp.get_mpfr_t(), 42ul) == 0);
    mpfr_set_si(m, -63l, MPFR_RNDN);
#if defined(_MSC_VER) && !defined(__clang__)
    ::mpfr_clear(m);
#else
    // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
    real rtmp2{std::move(m)};
    REQUIRE(rtmp2.get_prec() == 123);
    REQUIRE(mpfr_cmp_si(rtmp2.get_mpfr_t(), -63l) == 0);
#endif

    using Catch::Matchers::Message;

    // Constructors from std::complex.
    REQUIRE(real{std::complex<double>{-4, 0}} == -4);
    REQUIRE(real{std::complex<double>{-4, 0}}.get_prec() == detail::real_deduce_precision(4.));
    REQUIRE_THROWS_MATCHES((real{std::complex<double>{-4, 1}}), std::domain_error,
                           Message("Cannot construct a real from a complex C++ value with a non-zero imaginary part of "
                                   + detail::to_string(1.)));

    REQUIRE(real{std::complex<double>{-4, 0}, 34} == -4);
    REQUIRE(real{std::complex<double>{-4, 0}, 34}.get_prec() == 34);
    REQUIRE_THROWS_MATCHES((real{std::complex<double>{-4, 1}, 34}), std::domain_error,
                           Message("Cannot construct a real from a complex C++ value with a non-zero imaginary part of "
                                   + detail::to_string(1.)));
    REQUIRE_THROWS_MATCHES((real{std::complex<double>{4, 0}, -1}), std::invalid_argument,
                           Message("Cannot init a real with a precision of -1: the maximum allowed precision is "
                                   + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                   + detail::to_string(real_prec_min())));

    REQUIRE(!std::is_convertible<std::complex<double>, real>::value);
}

TEST_CASE("real kind constructors")
{
    // Ctor from nan.
    real r0{real_kind::nan, 12};
    REQUIRE(r0.nan_p());
    REQUIRE(r0.get_prec() == 12);
    r0 = real{real_kind::nan, 0, 11};
    REQUIRE(r0.nan_p());
    REQUIRE(r0.get_prec() == 11);
    r0 = real{real_kind::nan, -1, 10};
    REQUIRE(r0.nan_p());
    REQUIRE(r0.get_prec() == 10);
    // Ctor from inf.
    r0 = real{real_kind::inf, 12};
    REQUIRE(r0.inf_p());
    REQUIRE(r0.get_prec() == 12);
    REQUIRE(!r0.signbit());
    r0 = real{real_kind::inf, -1, 11};
    REQUIRE(r0.inf_p());
    REQUIRE(r0.get_prec() == 11);
    REQUIRE(r0.signbit());
    r0 = real{real_kind::inf, 0, 10};
    REQUIRE(r0.inf_p());
    REQUIRE(r0.get_prec() == 10);
    REQUIRE(!r0.signbit());
    // Ctor from zero.
    r0 = real{real_kind::zero, 12};
    REQUIRE(r0.zero_p());
    REQUIRE(r0.get_prec() == 12);
    REQUIRE(!r0.signbit());
    r0 = real{real_kind::zero, -1, 11};
    REQUIRE(r0.zero_p());
    REQUIRE(r0.get_prec() == 11);
    REQUIRE(r0.signbit());
    r0 = real{real_kind::zero, 0, 10};
    REQUIRE(r0.zero_p());
    REQUIRE(r0.get_prec() == 10);
    REQUIRE(!r0.signbit());
    // Error handling.
    REQUIRE_THROWS_PREDICATE((real{real_kind::nan, 0, -1}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of -1: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{real_kind::nan, -100}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of -100: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{real_kind::nan, 0, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{real_kind::nan, 0}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    // Wrong value for the real_kind enum.
    REQUIRE_THROWS_PREDICATE(
        (real{real_kind(MPFR_NAN_KIND + MPFR_INF_KIND + MPFR_ZERO_KIND), 12}), std::invalid_argument,
        [](const std::invalid_argument &ex) {
            return ex.what()
                   == "The 'real_kind' value passed to the constructor of a real ("
                          + detail::to_string(MPFR_NAN_KIND + MPFR_INF_KIND + MPFR_ZERO_KIND)
                          + ") is not one of the three allowed values ('nan'="
                          + detail::to_string(static_cast<std::underlying_type<::mpfr_kind_t>::type>(real_kind::nan))
                          + ", 'inf'="
                          + detail::to_string(static_cast<std::underlying_type<::mpfr_kind_t>::type>(real_kind::inf))
                          + " and 'zero'="
                          + detail::to_string(static_cast<std::underlying_type<::mpfr_kind_t>::type>(real_kind::zero))
                          + ")";
        });
}

TEST_CASE("real 2exp ctors")
{
    using Catch::Matchers::Message;

    auto r = real{0ul, 4, 56};
    REQUIRE(r.zero_p());
    REQUIRE(!r.signbit());
    REQUIRE(r.get_prec() == 56);
    REQUIRE_THROWS_MATCHES((r = real{0ul, 4, -1}), std::invalid_argument,
                           Message("Cannot init a real with a precision of -1: the maximum allowed precision is "
                                   + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                   + detail::to_string(real_prec_min())));
    r = real{2ul, 4, 57};
    REQUIRE(r == 32);
    REQUIRE(r.get_prec() == 57);

    r = real{0l, 4, 56};
    REQUIRE(r.zero_p());
    REQUIRE(!r.signbit());
    REQUIRE(r.get_prec() == 56);
    REQUIRE_THROWS_MATCHES((r = real{0l, 4, -1}), std::invalid_argument,
                           Message("Cannot init a real with a precision of -1: the maximum allowed precision is "
                                   + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                   + detail::to_string(real_prec_min())));
    r = real{2l, 4, 57};
    REQUIRE(r == 32);
    REQUIRE(r.get_prec() == 57);

    r = real{0_z1, 4, 56};
    REQUIRE(r.zero_p());
    REQUIRE(!r.signbit());
    REQUIRE(r.get_prec() == 56);
    REQUIRE_THROWS_MATCHES((r = real{0_z1, 4, -1}), std::invalid_argument,
                           Message("Cannot init a real with a precision of -1: the maximum allowed precision is "
                                   + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                   + detail::to_string(real_prec_min())));
    r = real{2_z1, 4, 57};
    REQUIRE(r == 32);
    REQUIRE(r.get_prec() == 57);
}

struct int_ass_tester {
    template <typename T>
    void operator()(const T &) const
    {
        real r{12};
        r.set_prec(123);
        r = T(0);
        REQUIRE(r.get_prec() == detail::nl_digits<T>() + detail::is_signed<T>::value);
        REQUIRE(r.zero_p());
        r.set_prec(123);
        r = T(42);
        REQUIRE(r.get_prec() == detail::nl_digits<T>() + detail::is_signed<T>::value);
        REQUIRE(::mpfr_equal_p(r.get_mpfr_t(), real{"42", 10, 100}.get_mpfr_t()));
        auto int_dist = integral_minmax_dist<T>{};
        for (int i = 0; i < ntrials; ++i) {
            auto n = int_dist(rng);
            r = n;
            REQUIRE(
                ::mpfr_equal_p(r.get_mpfr_t(), real{detail::to_string(n), 10, detail::nl_digits<T>()}.get_mpfr_t()));
        }
    }
};

struct fp_ass_tester {
    template <typename T>
    void operator()(const T &) const
    {
        real r{12};
        r.set_prec(123);
        r = T(0);
        REQUIRE(r.zero_p());
        if (std::numeric_limits<T>::radix == 2) {
            REQUIRE(r.get_prec() == detail::nl_digits<T>());
        }
        if (std::numeric_limits<T>::is_iec559) {
            REQUIRE(!r.signbit());
            r = -T(0);
            REQUIRE(r.zero_p());
            REQUIRE(r.signbit());
            r = std::numeric_limits<T>::infinity();
            REQUIRE(r.inf_p());
            REQUIRE(r.sgn() > 0);
            r = -std::numeric_limits<T>::infinity();
            REQUIRE(r.inf_p());
            REQUIRE(r.sgn() < 0);
            r = std::numeric_limits<T>::quiet_NaN();
            REQUIRE(r.nan_p());
        }
        if (std::numeric_limits<T>::radix != 2 || (ppc_arch && std::is_same<T, long double>::value)) {
            return;
        }
        std::uniform_real_distribution<T> dist(-T(100), T(100));
        for (int i = 0; i < ntrials; ++i) {
            auto x = dist(rng);
            r = x;
            REQUIRE(::mpfr_equal_p(r.get_mpfr_t(), real{f2str(x), 10, detail::nl_digits<T>()}.get_mpfr_t()));
        }
    }
};

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real assignment")
{
    REQUIRE((!std::is_assignable<real &, const foobar &>::value));
    real r0{123}, r1;
    r1 = r0;
    REQUIRE(r0.get_prec() == r1.get_prec());
    REQUIRE(::mpfr_equal_p(r0.get_mpfr_t(), r1.get_mpfr_t()));
    auto old_r1(r1);
    // Self assignment.
    r1 = *&r1;
    REQUIRE(old_r1.get_prec() == r1.get_prec());
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), old_r1.get_mpfr_t()));
    // Move assignment.
    real r2{456, 10};
    r2 = std::move(r1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 10);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{456, 10}.get_mpfr_t()));
    REQUIRE(r2.get_prec() == r0.get_prec());
    REQUIRE(::mpfr_equal_p(r2.get_mpfr_t(), old_r1.get_mpfr_t()));
    // Revive moved-from object with copy or move assignment.
    real r3{std::move(r1)};
    r1 = r2;
    REQUIRE(r1.get_prec() == r0.get_prec());
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), old_r1.get_mpfr_t()));
    real r4{std::move(r1)};
    r1 = real{321, 84};
    REQUIRE(r1.get_prec() == 84);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{321, 84}.get_mpfr_t()));
    // Swapping.
    real r5{123, 45}, r6{67, 89};
    swap(r5, r6);
    REQUIRE(r5.get_prec() == 89);
    REQUIRE(r6.get_prec() == 45);
    REQUIRE(::mpfr_equal_p(r5.get_mpfr_t(), real{67, 89}.get_mpfr_t()));
    REQUIRE(::mpfr_equal_p(r6.get_mpfr_t(), real{123, 45}.get_mpfr_t()));
    // Generic assignment.
    tuple_for_each(int_types{}, int_ass_tester{});
    tuple_for_each(fp_types{}, fp_ass_tester{});
    // Special handling of bool.
    real r7;
    r7 = false;
    REQUIRE(r7.zero_p());
    REQUIRE(r7.get_prec() == detail::c_max(::mpfr_prec_t(detail::nl_digits<bool>()), real_prec_min()));
    r7 = true;
    REQUIRE(mpfr_cmp_ui(r7.get_mpfr_t(), 1ul) == 0);
    REQUIRE(r7.get_prec() == detail::c_max(::mpfr_prec_t(detail::nl_digits<bool>()), real_prec_min()));
    // Assignment from integer.
    real r8;
    r8 = int_t{};
    REQUIRE(r8.zero_p());
    REQUIRE(r8.get_prec() == real_prec_min());
    r8 = int_t{1};
    REQUIRE(mpfr_cmp_ui((r8).get_mpfr_t(), 1ul) == 0);
    REQUIRE(r8.get_prec() == GMP_NUMB_BITS);
    r8 = int_t{42};
    REQUIRE(mpfr_cmp_ui((r8).get_mpfr_t(), 42ul) == 0);
    REQUIRE(r8.get_prec() == GMP_NUMB_BITS);
    r8 = -int_t{1};
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -1l) == 0);
    REQUIRE(r8.get_prec() == GMP_NUMB_BITS);
    r8 = -int_t{42};
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -42l) == 0);
    REQUIRE(r8.get_prec() == GMP_NUMB_BITS);
    r8 = int_t{42} << GMP_NUMB_BITS;
    REQUIRE(r8.get_prec() == 2 * GMP_NUMB_BITS);
    real tmp{42};
    ::mpfr_mul_2ui(tmp._get_mpfr_t(), tmp.get_mpfr_t(), GMP_NUMB_BITS, MPFR_RNDN);
    REQUIRE((::mpfr_equal_p(tmp.get_mpfr_t(), r8.get_mpfr_t())));
    tmp = real{-42};
    r8 = int_t{-42} << GMP_NUMB_BITS;
    ::mpfr_mul_2ui(tmp._get_mpfr_t(), tmp.get_mpfr_t(), GMP_NUMB_BITS, MPFR_RNDN);
    REQUIRE((::mpfr_equal_p(tmp.get_mpfr_t(), r8.get_mpfr_t())));
    // Assignment from rational.
    r8 = rat_t{};
    REQUIRE(r8.zero_p());
    REQUIRE(r8.get_prec() == GMP_NUMB_BITS);
    r8 = rat_t{1};
    REQUIRE(mpfr_cmp_ui((r8).get_mpfr_t(), 1ul) == 0);
    REQUIRE(r8.get_prec() == GMP_NUMB_BITS * 2);
    r8 = rat_t{42};
    REQUIRE(mpfr_cmp_ui((r8).get_mpfr_t(), 42ul) == 0);
    REQUIRE(r8.get_prec() == GMP_NUMB_BITS * 2);
    r8 = -rat_t{1};
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -1l) == 0);
    REQUIRE(r8.get_prec() == GMP_NUMB_BITS * 2);
    r8 = rat_t{-42};
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -42l) == 0);
    REQUIRE(r8.get_prec() == GMP_NUMB_BITS * 2);
    r8 = rat_t{5, 2};
    REQUIRE((::mpfr_equal_p((r8).get_mpfr_t(), (real{"2.5", 10, 64}).get_mpfr_t())));
    REQUIRE((r8.get_prec()) == GMP_NUMB_BITS * 2);
    r8 = rat_t{5, -2};
    REQUIRE((::mpfr_equal_p((r8).get_mpfr_t(), (real{"-25e-1", 10, 64}).get_mpfr_t())));
    REQUIRE((r8).get_prec() == GMP_NUMB_BITS * 2);
    tmp = real{42, GMP_NUMB_BITS * 3};
    r8 = rat_t{int_t{42} << GMP_NUMB_BITS, 5};
    ::mpfr_mul_2ui(tmp._get_mpfr_t(), tmp.get_mpfr_t(), GMP_NUMB_BITS, MPFR_RNDN);
    mpfr_div_ui(tmp._get_mpfr_t(), tmp.get_mpfr_t(), 5ul, MPFR_RNDN);
    REQUIRE((::mpfr_equal_p(tmp.get_mpfr_t(), r8.get_mpfr_t())));
    REQUIRE(r8.get_prec() == GMP_NUMB_BITS * 3);
#if defined(MPPP_WITH_QUADMATH)
    r8 = real128{};
    REQUIRE(r8.zero_p());
    REQUIRE(!r8.signbit());
    r8 = -real128{};
    REQUIRE(r8.zero_p());
    REQUIRE(r8.signbit());
    REQUIRE(r8.get_prec() == 113);
    r8 = real128{-1};
    REQUIRE(r8.get_prec() == 113);
    r8 = real128{1};
    REQUIRE(r8.get_prec() == 113);
    REQUIRE(mpfr_cmp_ui((r8).get_mpfr_t(), 1ul) == 0);
    r8 = real128{-1};
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -1l) == 0);
    r8 = real128{1123};
    REQUIRE(mpfr_cmp_ui((r8).get_mpfr_t(), 1123ul) == 0);
    r8 = real128{-1123};
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -1123l) == 0);
    r8 = real128_inf();
    REQUIRE(r8.inf_p());
    REQUIRE(r8.sgn() > 0);
    r8 = -real128_inf();
    REQUIRE(r8.inf_p());
    REQUIRE(r8.sgn() < 0);
    r8 = real128_nan();
    REQUIRE(r8.nan_p());
    r8 = real128{"3.40917866435610111081769936359662259e-2"};
    REQUIRE(::mpfr_equal_p(r8.get_mpfr_t(), real{"3.40917866435610111081769936359662259e-2", 10, 113}.get_mpfr_t()));
    r8 = -real128{"3.40917866435610111081769936359662259e-2"};
    REQUIRE(::mpfr_equal_p(r8.get_mpfr_t(), real{"-3.40917866435610111081769936359662259e-2", 10, 113}.get_mpfr_t()));
    // Subnormal values.
    r8 = real128{"3.40917866435610111081769936359662259e-4957"};
    REQUIRE(::mpfr_equal_p(r8.get_mpfr_t(), real{"3.40917866435610111081769936359662259e-4957", 10, 113}.get_mpfr_t()));
    r8 = -real128{"3.40917866435610111081769936359662259e-4957"};
    REQUIRE(
        ::mpfr_equal_p(r8.get_mpfr_t(), real{"-3.40917866435610111081769936359662259e-4957", 10, 113}.get_mpfr_t()));
#endif
    // The setter function.
    r8.set_prec(212);
    r8.set(real{1234, 55});
    REQUIRE(r8.get_prec() == 212);
    REQUIRE(mpfr_cmp_ui((r8).get_mpfr_t(), 1234ul) == 0);
    r8.set_prec(115);
    r8.set(12u);
    REQUIRE(r8.get_prec() == 115);
    REQUIRE(mpfr_cmp_ui((r8).get_mpfr_t(), 12ul) == 0);
    r8.set_prec(116);
    r8.set(short(-42));
    REQUIRE(r8.get_prec() == 116);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -42l) == 0);
    r8.set_prec(26);
    r8.set(false);
    REQUIRE(r8.get_prec() == 26);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 0l) == 0);
    r8.set_prec(126);
    r8.set(-123.);
    REQUIRE(r8.get_prec() == 126);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -123l) == 0);
    r8.set_prec(125);
    r8.set(123.f);
    REQUIRE(r8.get_prec() == 125);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 123l) == 0);
    r8.set_prec(136);
    r8.set(int_t{-12345l});
    REQUIRE(r8.get_prec() == 136);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -12345l) == 0);
    r8.set_prec(135);
    r8.set(rat_t{-9, 3});
    REQUIRE(r8.get_prec() == 135);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -3l) == 0);
#if defined(MPPP_WITH_QUADMATH)
    r8.set_prec(185);
    r8.set(real128{-456});
    REQUIRE(r8.get_prec() == 185);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -456l) == 0);
#endif
    // Setter from string.
    r8.set_prec(123);
    r8.set("-4.321e3");
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -4321) == 0);
    r8.set("4.321e3", 0);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 4321) == 0);
    r8.set("0b10011010010", 0);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1234) == 0);
    r8.set("0b10011010011", 2);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1235) == 0);
    r8.set("0b1.0011010011e10", 2);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1235) == 0);
    REQUIRE_THROWS_PREDICATE(r8.set("4.321e3", -1), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string{"Cannot assign a real from a string in base -1: the base must either be zero or in the "
                              "[2,62] range"};
    });
    REQUIRE_THROWS_PREDICATE(r8.set("4.321e3", 65), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string{"Cannot assign a real from a string in base 65: the base must either be zero or in the "
                              "[2,62] range"};
    });
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1235) == 0);
    REQUIRE_THROWS_PREDICATE(r8.set("hell-o"), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string{"The string 'hell-o' cannot be interpreted as a floating-point value in base 10"};
    });
    REQUIRE(r8.nan_p());
    r8 = 12;
    REQUIRE_THROWS_PREDICATE(r8.set("hell-o", 11), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string{"The string 'hell-o' cannot be interpreted as a floating-point value in base 11"};
    });
    REQUIRE_THROWS_PREDICATE(r8.set("baboo", 0), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what() == std::string{"The string 'baboo' cannot be interpreted as a floating-point value in base 0"};
    });
    REQUIRE(r8.nan_p());
    r8.set(std::string{"-4.321e3"});
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -4321) == 0);
    r8.set(std::string{"4.321e3"}, 0);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 4321) == 0);
    r8.set(std::string{"0b10011010010"}, 0);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1234) == 0);
    r8.set(std::string{"0b10011010011"}, 2);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1235) == 0);
    r8.set(std::string{"0b1.0011010011e10"}, 2);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1235) == 0);
    REQUIRE_THROWS_PREDICATE(
        r8.set(std::string{"4.321e3"}, -1), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{
                       "Cannot assign a real from a string in base -1: the base must either be zero or in the "
                       "[2,62] range"};
        });
    REQUIRE_THROWS_PREDICATE(
        r8.set(std::string{"4.321e3"}, 65), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{
                       "Cannot assign a real from a string in base 65: the base must either be zero or in the "
                       "[2,62] range"};
        });
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1235) == 0);
    REQUIRE_THROWS_PREDICATE(r8.set(std::string{"hell-o"}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string{"The string 'hell-o' cannot be interpreted as a floating-point value in base 10"};
    });
    REQUIRE_THROWS_PREDICATE(
        r8.set(std::string("baboo"), 0), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{"The string 'baboo' cannot be interpreted as a floating-point value in base 0"};
        });
    REQUIRE(r8.nan_p());
    r8 = 12;
    REQUIRE_THROWS_PREDICATE(
        r8.set(std::string{"hell-o"}, 11), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{"The string 'hell-o' cannot be interpreted as a floating-point value in base 11"};
        });
    REQUIRE(r8.nan_p());
    // Setter to char range.
    r8.set_prec(123);
    const std::vector<char> vc = {',', '-', '1', '2', '3', '4'};
    r8.set(vc.data() + 1, vc.data() + 5);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -123) == 0);
    REQUIRE(r8.get_prec() == 123);
    r8.set(vc.data() + 1, vc.data() + 6, 0);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -1234) == 0);
    REQUIRE(r8.get_prec() == 123);
    r8.set(vc.data() + 1, vc.data() + 4, 4);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -6) == 0);
    REQUIRE(r8.get_prec() == 123);
    REQUIRE_THROWS_PREDICATE(
        r8.set(vc.data() + 1, vc.data() + 5, -1), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{
                       "Cannot assign a real from a string in base -1: the base must either be zero or in the "
                       "[2,62] range"};
        });
    REQUIRE_THROWS_PREDICATE(
        r8.set(vc.data() + 1, vc.data() + 5, 65), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{
                       "Cannot assign a real from a string in base 65: the base must either be zero or in the "
                       "[2,62] range"};
        });
    REQUIRE_THROWS_PREDICATE(
        r8.set(vc.data(), vc.data() + 5), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{"The string ',-123' cannot be interpreted as a floating-point value in base 10"};
        });
    REQUIRE_THROWS_PREDICATE(
        r8.set(vc.data(), vc.data() + 5, 19), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{"The string ',-123' cannot be interpreted as a floating-point value in base 19"};
        });
    REQUIRE_THROWS_PREDICATE(
        r8.set(vc.data(), vc.data() + 4, 0), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{"The string ',-12' cannot be interpreted as a floating-point value in base 0"};
        });
#if defined(MPPP_HAVE_STRING_VIEW)
    r8.set(std::string_view{"-4.321e3"});
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), -4321) == 0);
    r8.set(std::string_view{"4.321e3"}, 0);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 4321) == 0);
    r8.set(std::string_view{"0b10011010010"}, 0);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1234) == 0);
    r8.set(std::string_view{"0b10011010011"}, 2);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1235) == 0);
    r8.set(std::string_view{"0b1.0011010011e10"}, 2);
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1235) == 0);
    REQUIRE_THROWS_PREDICATE(
        r8.set(std::string_view{"4.321e3"}, -1), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{
                       "Cannot assign a real from a string in base -1: the base must either be zero or in the "
                       "[2,62] range"};
        });
    REQUIRE_THROWS_PREDICATE(
        r8.set(std::string_view{"4.321e3"}, 65), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{
                       "Cannot assign a real from a string in base 65: the base must either be zero or in the "
                       "[2,62] range"};
        });
    REQUIRE(mpfr_cmp_si((r8).get_mpfr_t(), 1235) == 0);
    REQUIRE_THROWS_PREDICATE(
        r8.set(std::string_view{"hell-o"}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{"The string 'hell-o' cannot be interpreted as a floating-point value in base 10"};
        });
    REQUIRE(r8.nan_p());
    r8 = 12;
    REQUIRE_THROWS_PREDICATE(
        r8.set(std::string_view{"hell-o"}, 11), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{"The string 'hell-o' cannot be interpreted as a floating-point value in base 11"};
        });
    REQUIRE_THROWS_PREDICATE(
        r8.set(std::string_view{"baboo"}, 0), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{"The string 'baboo' cannot be interpreted as a floating-point value in base 0"};
        });
    REQUIRE(r8.nan_p());
#endif
    {
        // Assignment from mpfr_t.
        ::mpfr_t m;
        ::mpfr_init2(m, 123);
        mpfr_set_ui(m, 42ul, MPFR_RNDN);
        real rtmp;
        rtmp = m;
        REQUIRE(rtmp.get_prec() == 123);
        REQUIRE(mpfr_cmp_ui(rtmp.get_mpfr_t(), 42ul) == 0);
        mpfr_set_si(m, -63l, MPFR_RNDN);
#if defined(_MSC_VER) && !defined(__clang__)
        ::mpfr_clear(m);
#else
        real rtmp2{46, 46};
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        rtmp2 = std::move(m);
        REQUIRE(rtmp2.get_prec() == 123);
        REQUIRE(mpfr_cmp_si(rtmp2.get_mpfr_t(), -63l) == 0);
#endif
    }
    {
        // Setter from mpfr_t.
        ::mpfr_t m;
        ::mpfr_init2(m, 123);
        mpfr_set_ui(m, 42ul, MPFR_RNDN);
        real rtmp{12, 12};
        rtmp.set(m);
        REQUIRE(rtmp.get_prec() == 12);
        REQUIRE(mpfr_cmp_ui(rtmp.get_mpfr_t(), 42ul) == 0);
        ::mpfr_clear(m);
    }
    // The setter free function.
    r8.set_prec(154);
    set(r8, 12);
    REQUIRE(mpfr_cmp_ui(r8.get_mpfr_t(), 12ul) == 0);
    REQUIRE(r8.get_prec() == 154);
    r8.set_prec(254);
    set(r8, "123", 7);
    REQUIRE(mpfr_cmp_ui(r8.get_mpfr_t(), 66ul) == 0);
    REQUIRE(r8.get_prec() == 254);
    REQUIRE((std::is_same<real &, decltype(set(r8, "123", 7))>::value));
    r8.set_prec(253);
    set(r8, rat_t{45, 5});
    REQUIRE(mpfr_cmp_ui(r8.get_mpfr_t(), 9ul) == 0);
    REQUIRE(r8.get_prec() == 253);
    // Special values setters.
    r8.set_nan();
    REQUIRE(r8.nan_p());
    REQUIRE(r8.get_prec() == 253);
    r8.set_prec(251);
    r8.set(4);
    set_nan(r8);
    REQUIRE(r8.nan_p());
    REQUIRE(r8.get_prec() == 251);
    r8.set_inf();
    REQUIRE(r8.inf_p());
    REQUIRE(r8.get_prec() == 251);
    REQUIRE(r8.sgn() > 0);
    set_inf(r8, 1);
    REQUIRE(r8.inf_p());
    REQUIRE(r8.get_prec() == 251);
    REQUIRE(r8.sgn() > 0);
    r8.set_prec(45);
    r8.set_inf(-1);
    REQUIRE(r8.inf_p());
    REQUIRE(r8.get_prec() == 45);
    REQUIRE(r8.sgn() < 0);
    set_inf(r8, -10);
    REQUIRE(r8.inf_p());
    REQUIRE(r8.get_prec() == 45);
    REQUIRE(r8.sgn() < 0);
    r8.set_prec(86);
    r8.set_zero();
    REQUIRE(r8.zero_p());
    REQUIRE(r8.get_prec() == 86);
    REQUIRE(r8.sgn() == 0);
    REQUIRE(!r8.signbit());
    set_zero(r8, 1);
    REQUIRE(r8.zero_p());
    REQUIRE(r8.get_prec() == 86);
    REQUIRE(r8.sgn() == 0);
    r8.set_prec(45);
    r8.set_zero(-1);
    REQUIRE(r8.zero_p());
    REQUIRE(r8.get_prec() == 45);
    REQUIRE(r8.sgn() == 0);
    REQUIRE(r8.signbit());
    set_zero(r8, -10);
    REQUIRE(r8.zero_p());
    REQUIRE(r8.get_prec() == 45);
    REQUIRE(r8.sgn() == 0);
    REQUIRE(r8.signbit());

    using Catch::Matchers::Message;

    // Assignment from std::complex.
    r8 = real{};
    REQUIRE((r8 = std::complex<double>{-4, 0}) == -4);
    REQUIRE(r8.get_prec() == detail::real_deduce_precision(4.));
    REQUIRE_THROWS_MATCHES((r8 = std::complex<double>{-4, 1}), std::domain_error,
                           Message("Cannot construct a real from a complex C++ value with a non-zero imaginary part of "
                                   + detail::to_string(1.)));
    REQUIRE(r8 == -4);

    // Setter to std::complex.
    r8 = real{};
    REQUIRE(&r8.set(std::complex<double>{2, 0}) == &r8);
    REQUIRE(std::is_same<real &, decltype(r8.set(std::complex<double>{2, 0}))>::value);
    REQUIRE(r8 == 2);
    REQUIRE(r8.get_prec() == real_prec_min());
    r8 = real{1, 32};
    r8.set(std::complex<double>{-42, 0});
    REQUIRE(r8 == -42);
    REQUIRE(r8.get_prec() == 32);

    REQUIRE_THROWS_MATCHES(
        r8.set(std::complex<double>{-4, 1}), std::domain_error,
        Message("Cannot set a real to a complex C++ value with a non-zero imaginary part of " + detail::to_string(1.)));
    REQUIRE(r8 == -42);
    REQUIRE(r8.get_prec() == 32);

    // Try the free function too.
    REQUIRE(&set(r8, std::complex<double>{-5, 0}) == &r8);
    REQUIRE(std::is_same<real &, decltype(set(r8, std::complex<double>{-5, 0}))>::value);
    REQUIRE(r8 == -5);
    REQUIRE(r8.get_prec() == 32);
}

struct int_conv_tester {
    template <typename T>
    void operator()(const T &) const
    {
        T rop(0);
        real r0{T(0)};
        REQUIRE(static_cast<T>(r0) == T(0));
        REQUIRE(r0.get(rop));
        REQUIRE(get(rop, r0));
        REQUIRE(rop == T(0));
        r0 = real{T(42)};
        REQUIRE(static_cast<T>(r0) == T(42));
        REQUIRE(r0.get(rop));
        REQUIRE(get(rop, r0));
        REQUIRE(rop == T(42));
        auto int_dist = integral_minmax_dist<T>{};
        for (int i = 0; i < ntrials; ++i) {
            const auto tmp = int_dist(rng);
            REQUIRE(static_cast<T>(real{tmp}) == tmp);
            REQUIRE(real{tmp}.get(rop));
            REQUIRE(get(rop, real{tmp}));
            REQUIRE(rop == tmp);
        }
        rop = T(0);
        REQUIRE_THROWS_PREDICATE(static_cast<T>(real{int_t{detail::nl_max<T>()} + 1}), std::overflow_error,
                                 [](const std::overflow_error &ex) {
                                     return ex.what()
                                            == "Conversion of the real "
                                                   + real{int_t{detail::nl_max<T>()} + 1}.to_string() + " to the type '"
                                                   + type_name<T>() + "' results in overflow";
                                 });
        REQUIRE((!real{int_t{detail::nl_max<T>()} + 1}.get(rop)));
        REQUIRE(!get(rop, real{int_t{detail::nl_max<T>()} + 1}));
        REQUIRE(rop == T(0));
        REQUIRE_THROWS_PREDICATE(static_cast<T>(real{int_t{detail::nl_min<T>()} - 1}), std::overflow_error,
                                 [](const std::overflow_error &ex) {
                                     return ex.what()
                                            == "Conversion of the real "
                                                   + real{int_t{detail::nl_min<T>()} - 1}.to_string() + " to the type '"
                                                   + type_name<T>() + "' results in overflow";
                                 });
        REQUIRE((!real{int_t{detail::nl_min<T>()} - 1}.get(rop)));
        REQUIRE(!get(rop, real{int_t{detail::nl_min<T>()} - 1}));
        REQUIRE(rop == T(0));
        REQUIRE_THROWS_PREDICATE(
            static_cast<T>(real{"inf", 10, 100}), std::domain_error, [](const std::domain_error &ex) {
                return ex.what()
                       == (detail::is_unsigned<T>::value
                               ? std::string{"Cannot convert a non-finite real to a C++ unsigned integral type"}
                               : std::string{"Cannot convert a non-finite real to a C++ signed integral type"});
            });
        REQUIRE((!real{"inf", 10, 100}.get(rop)));
        REQUIRE(!get(rop, real{"inf", 10, 100}));
        REQUIRE(rop == T(0));
        REQUIRE_THROWS_PREDICATE(
            static_cast<T>(real{"-inf", 10, 100}), std::domain_error, [](const std::domain_error &ex) {
                return ex.what()
                       == (detail::is_unsigned<T>::value
                               ? std::string{"Cannot convert a non-finite real to a C++ unsigned integral type"}
                               : std::string{"Cannot convert a non-finite real to a C++ signed integral type"});
            });
        REQUIRE((!real{"-inf", 10, 100}.get(rop)));
        REQUIRE(!get(rop, real{"-inf", 10, 100}));
        REQUIRE(rop == T(0));
        REQUIRE_THROWS_PREDICATE(
            static_cast<T>(real{"nan", 10, 100}), std::domain_error, [](const std::domain_error &ex) {
                return ex.what()
                       == (detail::is_unsigned<T>::value
                               ? std::string{"Cannot convert a non-finite real to a C++ unsigned integral type"}
                               : std::string{"Cannot convert a non-finite real to a C++ signed integral type"});
            });
        REQUIRE((!real{"nan", 10, 100}.get(rop)));
        REQUIRE(!get(rop, real{"nan", 10, 100}));
        REQUIRE(rop == T(0));
    }
};

struct fp_conv_tester {
    template <typename T>
    void operator()(const T &) const
    {
        T rop(0);
        real r0{T(0)};
        REQUIRE(static_cast<T>(r0) == T(0));
        REQUIRE(r0.get(rop));
        REQUIRE(get(rop, r0));
        REQUIRE(rop == T(0));
        r0 = 42;
        REQUIRE(static_cast<T>(r0) == T(42));
        REQUIRE(r0.get(rop));
        REQUIRE(get(rop, r0));
        REQUIRE(rop == T(42));

        if (ppc_arch && std::is_same<T, long double>::value) {
            return;
        }

        std::uniform_real_distribution<T> dist(-T(1000), T(1000));
        for (int i = 0; i < ntrials; ++i) {
            const auto tmp = dist(rng);
            REQUIRE(static_cast<T>(real{tmp}) == tmp);
            REQUIRE(real{tmp}.get(rop));
            REQUIRE(get(rop, real{tmp}));
            REQUIRE(rop == tmp);
        }
        if (std::numeric_limits<T>::has_infinity && std::numeric_limits<T>::has_quiet_NaN) {
            REQUIRE(std::isinf(static_cast<T>(real{"inf", 10, 100})));
            REQUIRE((real{"inf", 10, 100}.get(rop)));
            REQUIRE(get(rop, real{"inf", 10, 100}));
            REQUIRE(rop == std::numeric_limits<T>::infinity());
            REQUIRE(std::isinf(static_cast<T>(real{"-inf", 10, 100})));
            REQUIRE((real{"-inf", 10, 100}.get(rop)));
            REQUIRE(get(rop, real{"-inf", 10, 100}));
            REQUIRE(rop == -std::numeric_limits<T>::infinity());
            REQUIRE(std::isnan(static_cast<T>(real{"nan", 10, 100})));
            REQUIRE((real{"nan", 10, 100}.get(rop)));
            REQUIRE(get(rop, real{"nan", 10, 100}));
            REQUIRE(std::isnan(rop));
        }
    }
};

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real conversion")
{
    tuple_for_each(int_types{}, int_conv_tester{});
    tuple_for_each(fp_types{}, fp_conv_tester{});
    // Bool conversion.
    bool brop = false;
    REQUIRE(static_cast<bool>(real{123}));
    REQUIRE(real{123}.get(brop));
    REQUIRE(get(brop, real{123}));
    REQUIRE(brop);
    REQUIRE(static_cast<bool>(real{-123}));
    REQUIRE(real{-123}.get(brop));
    REQUIRE(get(brop, real{-123}));
    REQUIRE(brop);
    REQUIRE(static_cast<bool>(real{"inf", 10, 100}));
    REQUIRE((real{"inf", 10, 100}.get(brop)));
    REQUIRE(get(brop, real{"inf", 10, 100}));
    REQUIRE(brop);
    REQUIRE(static_cast<bool>(real{"-inf", 10, 100}));
    REQUIRE((real{"-inf", 10, 100}.get(brop)));
    REQUIRE(get(brop, real{"-inf", 10, 100}));
    REQUIRE(brop);
    REQUIRE(static_cast<bool>(real{"nan", 10, 100}));
    REQUIRE((real{"nan", 10, 100}.get(brop)));
    REQUIRE(get(brop, real{"nan", 10, 100}));
    REQUIRE(brop);
    REQUIRE(!static_cast<bool>(real{0}));
    REQUIRE((real{0}.get(brop)));
    REQUIRE(get(brop, real{0}));
    REQUIRE(!brop);
    // Integer.
    int_t nrop{1};
    REQUIRE(static_cast<int_t>(real{0}) == 0);
    REQUIRE((real{0}.get(nrop)));
    REQUIRE(get(nrop, real{0}));
    REQUIRE(nrop == 0);
    REQUIRE(static_cast<int_t>(real{123}) == 123);
    REQUIRE((real{123}.get(nrop)));
    REQUIRE(get(nrop, real{123}));
    REQUIRE(nrop == 123);
    REQUIRE(static_cast<int_t>(real{-123}) == -123);
    REQUIRE((real{-123}.get(nrop)));
    REQUIRE(get(nrop, real{-123}));
    REQUIRE(nrop == -123);
    REQUIRE(static_cast<int_t>(real{"123.96", 10, 100}) == 123);
    REQUIRE((real{"123.96", 10, 100}.get(nrop)));
    REQUIRE(get(nrop, real{"123.96", 10, 100}));
    REQUIRE(nrop == 123);
    REQUIRE(static_cast<int_t>(real{"-123.96", 10, 100}) == -123);
    REQUIRE((real{"-123.96", 10, 100}.get(nrop)));
    REQUIRE(get(nrop, real{"-123.96", 10, 100}));
    REQUIRE(nrop == -123);
    nrop = 0;
    REQUIRE_THROWS_PREDICATE(static_cast<int_t>(real{"inf", 10, 100}), std::domain_error,
                             [](const std::domain_error &ex) {
                                 return ex.what() == std::string{"Cannot convert a non-finite real to an integer"};
                             });
    REQUIRE((!real{"inf", 10, 100}.get(nrop)));
    REQUIRE(!get(nrop, real{"inf", 10, 100}));
    REQUIRE(nrop == 0);
    REQUIRE_THROWS_PREDICATE(static_cast<int_t>(real{"-inf", 10, 100}), std::domain_error,
                             [](const std::domain_error &ex) {
                                 return ex.what() == std::string{"Cannot convert a non-finite real to an integer"};
                             });
    REQUIRE((!real{"-inf", 10, 100}.get(nrop)));
    REQUIRE(!get(nrop, real{"-inf", 10, 100}));
    REQUIRE(nrop == 0);
    REQUIRE_THROWS_PREDICATE(static_cast<int_t>(real{"nan", 10, 100}), std::domain_error,
                             [](const std::domain_error &ex) {
                                 return ex.what() == std::string{"Cannot convert a non-finite real to an integer"};
                             });
    REQUIRE((!real{"nan", 10, 100}.get(nrop)));
    REQUIRE(!get(nrop, real{"nan", 10, 100}));
    REQUIRE(nrop == 0);
    // Rational.
    rat_t qrop{1};
    REQUIRE(static_cast<rat_t>(real{0}) == 0);
    REQUIRE((real{0}.get(qrop)));
    REQUIRE(get(qrop, real{0}));
    REQUIRE(qrop == 0);
    REQUIRE(static_cast<rat_t>(real{123}) == 123);
    REQUIRE((real{123}.get(qrop)));
    REQUIRE(get(qrop, real{123}));
    REQUIRE(qrop == 123);
    REQUIRE(static_cast<rat_t>(real{-123}) == -123);
    REQUIRE((real{-123}.get(qrop)));
    REQUIRE(get(qrop, real{-123}));
    REQUIRE(qrop == -123);
    REQUIRE(static_cast<rat_t>(real{int_t{-123} << 110, 32}) == (int_t{-123} << 110));
    REQUIRE((real{int_t{-123} << 110, 32}.get(qrop)));
    REQUIRE(get(qrop, real{int_t{-123} << 110, 32}));
    REQUIRE(qrop == (int_t{-123} << 110));
    REQUIRE((static_cast<rat_t>(real{"4.1875", 10, 100}) == rat_t{67, 16}));
    REQUIRE((real{"4.1875", 10, 100}.get(qrop)));
    REQUIRE(get(qrop, real{"4.1875", 10, 100}));
    REQUIRE((qrop == rat_t{67, 16}));
    REQUIRE((static_cast<rat_t>(real{"-4.1875", 10, 100}) == -rat_t{67, 16}));
    REQUIRE((real{"-4.1875", 10, 100}.get(qrop)));
    REQUIRE(get(qrop, real{"-4.1875", 10, 100}));
    REQUIRE(qrop == (-rat_t{67, 16}));
    qrop = 0;
    REQUIRE_THROWS_PREDICATE(static_cast<rat_t>(real{"inf", 10, 100}), std::domain_error,
                             [](const std::domain_error &ex) {
                                 return ex.what() == std::string{"Cannot convert a non-finite real to a rational"};
                             });
    REQUIRE((!real{"inf", 10, 100}.get(qrop)));
    REQUIRE(!get(qrop, real{"inf", 10, 100}));
    REQUIRE(qrop == 0);
    REQUIRE_THROWS_PREDICATE(static_cast<rat_t>(real{"-inf", 10, 100}), std::domain_error,
                             [](const std::domain_error &ex) {
                                 return ex.what() == std::string{"Cannot convert a non-finite real to a rational"};
                             });
    REQUIRE((!real{"-inf", 10, 100}.get(qrop)));
    REQUIRE(!get(qrop, real{"-inf", 10, 100}));
    REQUIRE(qrop == 0);
    REQUIRE_THROWS_PREDICATE(static_cast<rat_t>(real{"nan", 10, 100}), std::domain_error,
                             [](const std::domain_error &ex) {
                                 return ex.what() == std::string{"Cannot convert a non-finite real to a rational"};
                             });
    REQUIRE((!real{"nan", 10, 100}.get(qrop)));
    REQUIRE(!get(qrop, real{"nan", 10, 100}));
    REQUIRE(qrop == 0);
#if defined(MPPP_WITH_QUADMATH)
    real128 rrop{1};
    // Zeroes and special values.
    REQUIRE(static_cast<real128>(real{}) == 0);
    REQUIRE(!static_cast<real128>(real{}).signbit());
    REQUIRE((real{0}.get(rrop)));
    REQUIRE(get(rrop, real{0}));
    REQUIRE(rrop == 0);
    REQUIRE(!rrop.signbit());
    REQUIRE(static_cast<real128>(real{"-0.", 10, 100}) == 0);
    REQUIRE(static_cast<real128>(real{"-0.", 10, 100}).signbit());
    REQUIRE((real{"-0.", 10, 100}.get(rrop)));
    REQUIRE(get(rrop, real{"-0.", 10, 100}));
    REQUIRE(rrop == 0);
    REQUIRE(rrop.signbit());
    REQUIRE(isnan(static_cast<real128>(real{"nan", 10, 100})));
    REQUIRE((real{"nan", 10, 100}.get(rrop)));
    REQUIRE(get(rrop, real{"nan", 10, 100}));
    REQUIRE(rrop.isnan());
    REQUIRE(static_cast<real128>(real{"inf", 10, 100}) == real128_inf());
    REQUIRE((real{"inf", 10, 100}.get(rrop)));
    REQUIRE(get(rrop, real{"inf", 10, 100}));
    REQUIRE(rrop.isinf());
    REQUIRE(static_cast<real128>(real{"-inf", 10, 100}) == -real128_inf());
    REQUIRE((real{"-inf", 10, 100}.get(rrop)));
    REQUIRE(get(rrop, real{"-inf", 10, 100}));
    REQUIRE(rrop.isinf());
    // Big and small.
    rrop = 1;
    real r0{1};
    ::mpfr_mul_2ui(r0._get_mpfr_t(), r0.get_mpfr_t(), 20000ul, MPFR_RNDN);
    REQUIRE(static_cast<real128>(r0) == real128_inf());
    REQUIRE((r0.get(rrop)));
    REQUIRE(get(rrop, r0));
    REQUIRE(rrop == real128_inf());
    r0 = 1;
    ::mpfr_mul_2ui(r0._get_mpfr_t(), r0.get_mpfr_t(), 262145ul, MPFR_RNDN);
    REQUIRE(static_cast<real128>(r0) == real128_inf());
    REQUIRE((r0.get(rrop)));
    REQUIRE(get(rrop, r0));
    REQUIRE(rrop == real128_inf());
    r0 = 1;
    ::mpfr_div_2ui(r0._get_mpfr_t(), r0.get_mpfr_t(), 20000ul, MPFR_RNDN);
    REQUIRE(static_cast<real128>(r0) == 0);
    REQUIRE((r0.get(rrop)));
    REQUIRE(get(rrop, r0));
    REQUIRE(rrop == 0);
    r0 = 1;
    ::mpfr_div_2ui(r0._get_mpfr_t(), r0.get_mpfr_t(), 262145ul, MPFR_RNDN);
    REQUIRE(static_cast<real128>(r0) == 0);
    REQUIRE((r0.get(rrop)));
    REQUIRE(get(rrop, r0));
    REQUIRE(rrop == 0);
    // Subnormals.
    REQUIRE(static_cast<real128>(real{real128{"3.40917866435610111081769936359662259e-4957"}})
            == real128{"3.40917866435610111081769936359662259e-4957"});
    REQUIRE((real{real128{"3.40917866435610111081769936359662259e-4957"}}.get(rrop)));
    REQUIRE(get(rrop, real{real128{"3.40917866435610111081769936359662259e-4957"}}));
    REQUIRE(rrop == real128{"3.40917866435610111081769936359662259e-4957"});
    REQUIRE(static_cast<real128>(real{real128{"-3.40917866435610111081769936359662259e-4957"}})
            == -real128{"3.40917866435610111081769936359662259e-4957"});
    REQUIRE((real{real128{"-3.40917866435610111081769936359662259e-4957"}}.get(rrop)));
    REQUIRE(get(rrop, real{real128{"-3.40917866435610111081769936359662259e-4957"}}));
    REQUIRE(rrop == -real128{"3.40917866435610111081769936359662259e-4957"});
    // A couple of normal values.
    REQUIRE(static_cast<real128>(real{real128{"3.40917866435610111081769936359662259e-4"}})
            == real128{"3.40917866435610111081769936359662259e-4"});
    REQUIRE((real{real128{"3.40917866435610111081769936359662259e-4"}}.get(rrop)));
    REQUIRE(get(rrop, real{real128{"3.40917866435610111081769936359662259e-4"}}));
    REQUIRE(rrop == real128{"3.40917866435610111081769936359662259e-4"});
    REQUIRE(static_cast<real128>(real{-real128{"3.40917866435610111081769936359662259e-4"}})
            == real128{"-3.40917866435610111081769936359662259e-4"});
    REQUIRE((real{real128{"-3.40917866435610111081769936359662259e-4"}}.get(rrop)));
    REQUIRE(get(rrop, real{real128{"-3.40917866435610111081769936359662259e-4"}}));
    REQUIRE(rrop == -real128{"3.40917866435610111081769936359662259e-4"});
    // A real with less precision than real128.
    REQUIRE(static_cast<real128>(real{123, 32}) == real128{123});
    REQUIRE((real{123, 32}.get(rrop)));
    REQUIRE(get(rrop, real{123, 32}));
    REQUIRE(rrop == 123);
    REQUIRE(static_cast<real128>(real{-123, 32}) == -real128{123});
    REQUIRE((real{-123, 32}.get(rrop)));
    REQUIRE(get(rrop, real{-123, 32}));
    REQUIRE(rrop == -123);
    // Larger precision.
    REQUIRE(abs(static_cast<real128>(real{"1.1", 10, 300}) - real128{"1.1"}) < 1E-33);
    REQUIRE((real{"1.1", 10, 300}.get(rrop)));
    REQUIRE(get(rrop, real{"1.1", 10, 300}));
    REQUIRE(abs(rrop - real128{"1.1"}) < 1E-33);
    REQUIRE(abs(static_cast<real128>(real{"-1.1", 10, 300}) - real128{"-1.1"}) < 1E-33);
    REQUIRE((real{"-1.1", 10, 300}.get(rrop)));
    REQUIRE(get(rrop, real{"-1.1", 10, 300}));
    REQUIRE(abs(rrop - real128{"-1.1"}) < 1E-33);
#endif

    // Conversions to std::complex.
    REQUIRE(static_cast<std::complex<double>>(real{45}) == std::complex<double>{45, 0});
    REQUIRE(static_cast<std::complex<float>>(real{-2}) == std::complex<float>{-2, 0});
    REQUIRE(static_cast<std::complex<long double>>(real{-12}) == std::complex<long double>{-12, 0});

    std::complex<double> crop{1, 2};
    REQUIRE(real{45}.get(crop));
    REQUIRE(crop == std::complex<double>{45, 0});
    REQUIRE(get(crop, real{88}));
    REQUIRE(crop == std::complex<double>{88, 0});
}

TEST_CASE("real set prec")
{
    real r;
    REQUIRE(r.get_prec() == real_prec_min());
    REQUIRE(r.zero_p());
    r.set_prec(156);
    REQUIRE(r.get_prec() == 156);
    REQUIRE(r.nan_p());
    r = real{};
    set_prec(r, 156);
    REQUIRE(get_prec(r) == 156);
    REQUIRE(r.nan_p());
    r = real{};
    REQUIRE_THROWS_PREDICATE(r.set_prec(-1), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot set the precision of a real to the value -1: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE(set_prec(r, 0), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot set the precision of a real to the value 0: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE(r.zero_p());
    r = 123;
    r.prec_round(64);
    REQUIRE(get_prec(r) == 64);
    REQUIRE(::mpfr_equal_p(r.get_mpfr_t(), real{123}.get_mpfr_t()));
    prec_round(r, 74);
    REQUIRE(get_prec(r) == 74);
    REQUIRE(::mpfr_equal_p(r.get_mpfr_t(), real{123}.get_mpfr_t()));
    REQUIRE_THROWS_PREDICATE(r.prec_round(-1), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot set the precision of a real to the value -1: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE(prec_round(r, 0), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot set the precision of a real to the value 0: the maximum allowed precision is "
                      + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + detail::to_string(real_prec_min());
    });
    REQUIRE(get_prec(r) == 74);
    REQUIRE(::mpfr_equal_p(r.get_mpfr_t(), real{123}.get_mpfr_t()));
}

TEST_CASE("real mt cleanup")
{
    // Test the cleanup machinery from multiple threads. This
    // should print, in debug mode, the cleanup message
    // once for each thread.
    std::atomic<unsigned> counter(0);
    auto func = [&counter]() {
        ++counter;
        while (counter.load() != 4u) {
        }
        real r;
    };
    std::thread t1(func), t2(func), t3(func), t4(func);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

#if MPPP_CPLUSPLUS >= 201703L

TEST_CASE("real nts")
{
    REQUIRE(std::is_nothrow_swappable_v<real>);
}

#endif

#if defined(MPPP_MPFR_HAVE_MPFR_GET_STR_NDIGITS)

TEST_CASE("real str_ndigits")
{
    using Catch::Matchers::Message;

    real r0{"1.1", 53};

    REQUIRE(r0.get_str_ndigits() == 17u);
    REQUIRE(r0.get_str_ndigits(10) == 17u);

    r0 = real{"1.1", 24};

    REQUIRE(get_str_ndigits(r0) == 9u);
    REQUIRE(get_str_ndigits(r0, 10) == 9u);

    REQUIRE_THROWS_MATCHES(
        r0.get_str_ndigits(1), std::invalid_argument,
        Message("Invalid base value for get_str_ndigits(): the base must be in the [2,62] range, but it is 1 instead"));
    REQUIRE_THROWS_MATCHES(
        get_str_ndigits(r0, -100), std::invalid_argument,
        Message(
            "Invalid base value for get_str_ndigits(): the base must be in the [2,62] range, but it is -100 instead"));
    REQUIRE_THROWS_MATCHES(
        get_str_ndigits(r0, 63), std::invalid_argument,
        Message(
            "Invalid base value for get_str_ndigits(): the base must be in the [2,62] range, but it is 63 instead"));
}

#endif
