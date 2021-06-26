// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>
#include <string>
#include <utility>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real integer_p")
{
    real r0{0};
    REQUIRE(r0.integer_p());
    REQUIRE(integer_p(r0));
    r0 = .1;
    REQUIRE(!r0.integer_p());
    REQUIRE(!integer_p(r0));
    r0 = -.1;
    REQUIRE(!r0.integer_p());
    REQUIRE(!integer_p(r0));
    r0 = 1;
    REQUIRE(r0.integer_p());
    REQUIRE(integer_p(r0));
    r0 = -1;
    REQUIRE(r0.integer_p());
    REQUIRE(integer_p(r0));
    r0 = 12345;
    REQUIRE(r0.integer_p());
    REQUIRE(integer_p(r0));
    r0 = real{"inf", 128};
    REQUIRE(!r0.integer_p());
    REQUIRE(!integer_p(r0));
    r0 = -real{"inf", 128};
    REQUIRE(!r0.integer_p());
    REQUIRE(!integer_p(r0));
    r0 = real{"nan", 128};
    REQUIRE(!r0.integer_p());
    REQUIRE(!integer_p(r0));
}

TEST_CASE("real trunc")
{
    real r0{0};
    REQUIRE(r0.trunc() == 0);
    r0 = 0.1;
    REQUIRE(r0.trunc() == 0);
    r0 = -0.1;
    REQUIRE(r0.trunc() == 0);
    r0 = 1.001;
    REQUIRE(r0.trunc() == 1);
    r0 = -1.001;
    REQUIRE(r0.trunc() == -1);
    r0 = 1.999;
    REQUIRE(r0.trunc() == 1);
    r0 = -1.9999;
    REQUIRE(r0.trunc() == -1);
    // The binary function.
    real tmp{45.67, 50};
    r0.set_prec(4);
    // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
    auto tmp_ptr = r0.get_mpfr_t()->_mpfr_d;
    trunc(r0, std::move(tmp));
    REQUIRE(r0 == 45);
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == tmp_ptr);
    r0.set_prec(4);
    tmp = real{-49.99, 50};
    trunc(r0, real{-49.99, 50});
    REQUIRE(r0 == -49);
    REQUIRE(get_prec(r0) == 50);
    // The unary function.
    r0.set_prec(4);
    tmp = real{45.67, 50};
    r0 = trunc(std::move(tmp));
    REQUIRE(r0 == 45);
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == nullptr);
    tmp = real{45.67, 50};
    r0 = trunc(tmp);
    REQUIRE(r0 == 45);
    REQUIRE(get_prec(r0) == 50);
    r0.set_prec(4);
    r0 = trunc(real{-49.99, 50});
    REQUIRE(r0 == -49);
    REQUIRE(get_prec(r0) == 50);
    // Failure modes.
    r0.set_nan();
    REQUIRE_THROWS_PREDICATE(r0.trunc(), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot truncate a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(trunc(r0, real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot truncate a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(trunc(real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot truncate a NaN value"};
    });
}

TEST_CASE("real ceil")
{
    real r0{0};
    REQUIRE(r0.ceil() == 0);
    r0 = 0.1;
    REQUIRE(r0.ceil() == 1);
    r0 = -0.1;
    REQUIRE(r0.ceil() == 0);
    r0 = 1.001;
    REQUIRE(r0.ceil() == 2);
    r0 = -1.001;
    REQUIRE(r0.ceil() == -1);
    r0 = 1.999;
    REQUIRE(r0.ceil() == 2);
    r0 = -1.9999;
    REQUIRE(r0.ceil() == -1);
    // The binary function.
    real tmp{45.67, 50};
    r0.set_prec(4);
    // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
    auto tmp_ptr = r0.get_mpfr_t()->_mpfr_d;
    ceil(r0, std::move(tmp));
    REQUIRE(r0 == 46);
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == tmp_ptr);
    r0.set_prec(4);
    tmp = real{-49.99, 50};
    ceil(r0, real{-49.99, 50});
    REQUIRE(r0 == -49);
    REQUIRE(get_prec(r0) == 50);
    // The unary function.
    r0.set_prec(4);
    tmp = real{45.67, 50};
    r0 = ceil(std::move(tmp));
    REQUIRE(r0 == 46);
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == nullptr);
    tmp = real{45.67, 50};
    r0 = ceil(tmp);
    REQUIRE(r0 == 46);
    REQUIRE(get_prec(r0) == 50);
    r0.set_prec(4);
    r0 = ceil(real{-49.99, 50});
    REQUIRE(r0 == -49);
    REQUIRE(get_prec(r0) == 50);
    // Failure modes.
    r0.set_nan();
    REQUIRE_THROWS_PREDICATE(r0.ceil(), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot compute the ceiling of a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(ceil(r0, real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot compute the ceiling of a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(ceil(real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot compute the ceiling of a NaN value"};
    });
}

TEST_CASE("real floor")
{
    real r0{0};
    REQUIRE(r0.floor() == 0);
    r0 = 0.1;
    REQUIRE(r0.floor() == 0);
    r0 = -0.1;
    REQUIRE(r0.floor() == -1);
    r0 = 1.001;
    REQUIRE(r0.floor() == 1);
    r0 = -1.001;
    REQUIRE(r0.floor() == -2);
    r0 = 1.999;
    REQUIRE(r0.floor() == 1);
    r0 = -1.9999;
    REQUIRE(r0.floor() == -2);
    // The binary function.
    real tmp{45.67, 50};
    r0.set_prec(4);
    // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
    auto tmp_ptr = r0.get_mpfr_t()->_mpfr_d;
    floor(r0, std::move(tmp));
    REQUIRE(r0 == 45);
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == tmp_ptr);
    r0.set_prec(4);
    tmp = real{-49.99, 50};
    floor(r0, real{-49.99, 50});
    REQUIRE(r0 == -50);
    REQUIRE(get_prec(r0) == 50);
    // The unary function.
    r0.set_prec(4);
    tmp = real{45.67, 50};
    r0 = floor(std::move(tmp));
    REQUIRE(r0 == 45);
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == nullptr);
    tmp = real{45.67, 50};
    r0 = floor(tmp);
    REQUIRE(r0 == 45);
    REQUIRE(get_prec(r0) == 50);
    r0.set_prec(4);
    r0 = floor(real{-49.99, 50});
    REQUIRE(r0 == -50);
    REQUIRE(get_prec(r0) == 50);
    // Failure modes.
    r0.set_nan();
    REQUIRE_THROWS_PREDICATE(r0.floor(), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot compute the floor of a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(floor(r0, real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot compute the floor of a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(floor(real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot compute the floor of a NaN value"};
    });
}

TEST_CASE("real round")
{
    real r0{0};
    REQUIRE(r0.round() == 0);
    r0 = 0.1;
    REQUIRE(r0.round() == 0);
    r0 = -0.1;
    REQUIRE(r0.round() == 0);
    r0 = 1.001;
    REQUIRE(r0.round() == 1);
    r0 = -1.001;
    REQUIRE(r0.round() == -1);
    r0 = 1.999;
    REQUIRE(r0.round() == 2);
    r0 = -1.9999;
    REQUIRE(r0.round() == -2);
    r0 = real{"1.5", 20};
    REQUIRE(r0.round() == 2);
    r0 = real{"-1.5", 20};
    REQUIRE(r0.round() == -2);
    r0 = real{"2.5", 20};
    REQUIRE(r0.round() == 3);
    r0 = real{"-2.5", 20};
    REQUIRE(r0.round() == -3);
    // The binary function.
    real tmp{45.67, 50};
    r0.set_prec(4);
    // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
    auto tmp_ptr = r0.get_mpfr_t()->_mpfr_d;
    round(r0, std::move(tmp));
    REQUIRE(r0 == 46);
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == tmp_ptr);
    r0.set_prec(4);
    tmp = real{-49.99, 50};
    round(r0, real{-49.99, 50});
    REQUIRE(r0 == -50);
    REQUIRE(get_prec(r0) == 50);
    // The unary function.
    r0.set_prec(4);
    tmp = real{45.67, 50};
    r0 = round(std::move(tmp));
    REQUIRE(r0 == 46);
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == nullptr);
    tmp = real{45.67, 50};
    r0 = round(tmp);
    REQUIRE(r0 == 46);
    REQUIRE(get_prec(r0) == 50);
    r0.set_prec(4);
    r0 = round(real{-49.99, 50});
    REQUIRE(r0 == -50);
    REQUIRE(get_prec(r0) == 50);
    // Failure modes.
    r0.set_nan();
    REQUIRE_THROWS_PREDICATE(r0.round(), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot round a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(round(r0, real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot round a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(round(real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot round a NaN value"};
    });

    // Couple of extra tests for the free functions.
    REQUIRE(round(real{"1.5", 20}) == 2);
    REQUIRE(round(real{"-1.5", 20}) == -2);
    REQUIRE(round(real{"2.5", 20}) == 3);
    REQUIRE(round(real{"-2.5", 20}) == -3);
}

#if defined(MPPP_MPFR_HAVE_MPFR_ROUNDEVEN)

TEST_CASE("real roundeven")
{
    real r0{0};
    REQUIRE(r0.roundeven() == 0);
    r0 = 0.1;
    REQUIRE(r0.roundeven() == 0);
    r0 = -0.1;
    REQUIRE(r0.roundeven() == 0);
    r0 = 1.001;
    REQUIRE(r0.roundeven() == 1);
    r0 = -1.001;
    REQUIRE(r0.roundeven() == -1);
    r0 = 1.999;
    REQUIRE(r0.roundeven() == 2);
    r0 = -1.9999;
    REQUIRE(r0.roundeven() == -2);
    r0 = real{"1.5", 20};
    REQUIRE(r0.roundeven() == 2);
    r0 = real{"-1.5", 20};
    REQUIRE(r0.roundeven() == -2);
    r0 = real{"2.5", 20};
    REQUIRE(r0.roundeven() == 2);
    r0 = real{"-2.5", 20};
    REQUIRE(r0.roundeven() == -2);
    r0 = real{"3.5", 20};
    REQUIRE(r0.roundeven() == 4);
    r0 = real{"-3.5", 20};
    REQUIRE(r0.roundeven() == -4);
    // The binary function.
    real tmp{45.67, 50};
    r0.set_prec(4);
    // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
    auto tmp_ptr = r0.get_mpfr_t()->_mpfr_d;
    roundeven(r0, std::move(tmp));
    REQUIRE(r0 == 46);
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == tmp_ptr);
    r0.set_prec(4);
    tmp = real{-49.99, 50};
    roundeven(r0, real{-49.99, 50});
    REQUIRE(r0 == -50);
    REQUIRE(get_prec(r0) == 50);
    // The unary function.
    r0.set_prec(4);
    tmp = real{45.67, 50};
    r0 = roundeven(std::move(tmp));
    REQUIRE(r0 == 46);
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == nullptr);
    tmp = real{45.67, 50};
    r0 = roundeven(tmp);
    REQUIRE(r0 == 46);
    REQUIRE(get_prec(r0) == 50);
    r0.set_prec(4);
    r0 = roundeven(real{-49.99, 50});
    REQUIRE(r0 == -50);
    REQUIRE(get_prec(r0) == 50);
    // Failure modes.
    r0.set_nan();
    REQUIRE_THROWS_PREDICATE(r0.roundeven(), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot round a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(roundeven(r0, real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot round a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(roundeven(real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot round a NaN value"};
    });

    // Couple of extra tests for the free functions.
    REQUIRE(roundeven(real{"1.5", 20}) == 2);
    REQUIRE(roundeven(real{"-1.5", 20}) == -2);
    REQUIRE(roundeven(real{"2.5", 20}) == 2);
    REQUIRE(roundeven(real{"-2.5", 20}) == -2);
}

#endif

TEST_CASE("real frac")
{
    real r0{0};
    REQUIRE(r0.frac() == 0);
    r0 = 0.1;
    REQUIRE(r0.frac() == 0.1);
    r0 = -0.1;
    REQUIRE(r0.frac() == -0.1);
    r0 = 1.001;
    REQUIRE(r0.frac() == 1.001 - 1);
    r0 = -1.001;
    REQUIRE(r0.frac() == -1.001 + 1);
    r0 = 1.999;
    REQUIRE(r0.frac() == 1.999 - 1.);
    r0 = -1.999;
    REQUIRE(r0.frac() == -1.999 + 1);
    // The binary function.
    real tmp{45.67, 50};
    r0.set_prec(4);
    // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
    auto tmp_ptr = r0.get_mpfr_t()->_mpfr_d;
    frac(r0, std::move(tmp));
    REQUIRE(r0 == real{45.67, 50} - real{45, 50});
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == tmp_ptr);
    r0.set_prec(4);
    tmp = real{-49.99, 50};
    frac(r0, real{-49.99, 50});
    REQUIRE(r0 == real{-49.99, 50} + real{49, 50});
    REQUIRE(get_prec(r0) == 50);
    // The unary function.
    r0.set_prec(4);
    tmp = real{45.67, 50};
    r0 = frac(std::move(tmp));
    REQUIRE(r0 == real{45.67, 50} - real{45, 50});
    REQUIRE(get_prec(r0) == 50);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp.get_mpfr_t()->_mpfr_d == nullptr);
    tmp = real{45.67, 50};
    r0 = frac(tmp);
    REQUIRE(r0 == real{45.67, 50} - real{45, 50});
    REQUIRE(get_prec(r0) == 50);
    r0.set_prec(4);
    r0 = frac(real{-49.99, 50});
    REQUIRE(r0 == real{-49.99, 50} + real{49, 50});
    REQUIRE(get_prec(r0) == 50);
    // Failure modes.
    r0.set_nan();
    REQUIRE_THROWS_PREDICATE(r0.frac(), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot compute the fractional part of a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(frac(r0, real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot compute the fractional part of a NaN value"};
    });
    REQUIRE_THROWS_PREDICATE(frac(real{"nan", 12}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"Cannot compute the fractional part of a NaN value"};
    });
}

TEST_CASE("real modf")
{
    real iop, fop;
    modf(iop, fop, real{"1.25", 10});
    REQUIRE(iop == 1);
    REQUIRE(iop.get_prec() == 10);
    REQUIRE(fop == real{"0.25", 10});
    REQUIRE(fop.get_prec() == 10);

    REQUIRE_THROWS_PREDICATE(
        modf(iop, iop, real{"1.25", 10}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string{
                       "In the real modf() function, the return values 'iop' and 'fop' must be distinct objects"};
        });
    REQUIRE_THROWS_PREDICATE(modf(iop, fop, real{"nan", 10}), std::domain_error, [](const std::domain_error &ex) {
        return ex.what() == std::string{"In the real modf() function, the input argument cannot be NaN"};
    });

    // Try with overlapping op/sop and op/cop.
    iop = real{1, detail::real_deduce_precision(0) * 2};
    fop = real{2, detail::real_deduce_precision(0) * 3};
    modf(iop, fop, iop);
    REQUIRE(iop.get_prec() == detail::real_deduce_precision(0) * 2);
    REQUIRE(fop.get_prec() == detail::real_deduce_precision(0) * 2);
    REQUIRE(iop == 1);
    REQUIRE(fop == 0);

    iop = real{1, detail::real_deduce_precision(0) * 2};
    fop = real{2, detail::real_deduce_precision(0) * 3};
    modf(fop, iop, fop);
    REQUIRE(fop.get_prec() == detail::real_deduce_precision(0) * 3);
    REQUIRE(iop.get_prec() == detail::real_deduce_precision(0) * 3);
    REQUIRE(fop == 2);
    REQUIRE(iop == 0);
}

TEST_CASE("real fmod")
{
    real r0{12, 450};
    fmod(r0, real{1}, sqrt(real{2}));
    REQUIRE(abs(r0 - 1) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    real tmp1{1}, tmp2{sqrt(real{2})};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    fmod(r0, std::move(tmp1), tmp2);
    REQUIRE(abs(r0 - 1) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp1 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp1 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp1.get_prec() == detail::real_deduce_precision(0) / 2);
    tmp1 = real{1};
    tmp2 = real{sqrt(real{2})};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    fmod(r0, tmp1, std::move(tmp2));
    REQUIRE(abs(r0 - 1) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp2 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp2 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp2.get_prec() == detail::real_deduce_precision(0) / 2);

    // Some tests for the binary form too.
    REQUIRE(abs(fmod(real{1}, real{sqrt(real{2})}) - 1) < 1E-6);
    REQUIRE(fmod(real{4, 20}, real{5, 30}).get_prec() == 30);
    REQUIRE(fmod(real{4}, 5.) == fmod(real{4}, real{5.}));
    REQUIRE(fmod(5., real{4}) == fmod(real{5.}, real{4}));
    REQUIRE(fmod(real{4}, 5) == fmod(real{4}, real{5}));
    REQUIRE(fmod(5, real{4}) == fmod(real{5}, real{4}));
    REQUIRE(fmod(5., real{4}) == fmod(real{5.}, real{4}));
    REQUIRE(fmod(5, real{4}) == fmod(real{5}, real{4}));
    REQUIRE(fmod(real{4}, integer<1>{5}) == fmod(real{4}, real{integer<1>{5}}));
    REQUIRE(fmod(integer<1>{5}, real{4}) == fmod(real{integer<1>{5}}, real{4}));
    REQUIRE(fmod(real{4, detail::real_deduce_precision(0.) / 2}, 5.).get_prec() == detail::real_deduce_precision(0.));
    REQUIRE(fmod(4., real{5, detail::real_deduce_precision(0.) / 2}).get_prec() == detail::real_deduce_precision(0.));
    REQUIRE(fmod(real{4, detail::real_deduce_precision(0) / 2}, 5).get_prec() == detail::real_deduce_precision(0));
    REQUIRE(fmod(4, real{5, detail::real_deduce_precision(0) / 2}).get_prec() == detail::real_deduce_precision(0));
}

TEST_CASE("real remainder")
{
    real r0{12, 450};
    remainder(r0, real{1}, sqrt(real{2}));
    REQUIRE(abs(r0 - -0.414213562384) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    real tmp1{1}, tmp2{sqrt(real{2})};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    remainder(r0, std::move(tmp1), tmp2);
    REQUIRE(abs(r0 - -0.414213562384) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp1 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp1 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp1.get_prec() == detail::real_deduce_precision(0) / 2);
    tmp1 = real{1};
    tmp2 = real{sqrt(real{2})};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    remainder(r0, tmp1, std::move(tmp2));
    REQUIRE(abs(r0 - -0.414213562384) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp2 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp2 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp2.get_prec() == detail::real_deduce_precision(0) / 2);

    // Some tests for the binary form too.
    REQUIRE(abs(remainder(real{1}, real{sqrt(real{2})}) - -0.414213562384) < 1E-6);
    REQUIRE(remainder(real{4, 20}, real{5, 30}).get_prec() == 30);
    REQUIRE(remainder(real{4}, 5.) == remainder(real{4}, real{5.}));
    REQUIRE(remainder(5., real{4}) == remainder(real{5.}, real{4}));
    REQUIRE(remainder(real{4}, 5) == remainder(real{4}, real{5}));
    REQUIRE(remainder(5, real{4}) == remainder(real{5}, real{4}));
    REQUIRE(remainder(5., real{4}) == remainder(real{5.}, real{4}));
    REQUIRE(remainder(5, real{4}) == remainder(real{5}, real{4}));
    REQUIRE(remainder(real{4}, integer<1>{5}) == remainder(real{4}, real{integer<1>{5}}));
    REQUIRE(remainder(integer<1>{5}, real{4}) == remainder(real{integer<1>{5}}, real{4}));
    REQUIRE(remainder(real{4, detail::real_deduce_precision(0.) / 2}, 5.).get_prec()
            == detail::real_deduce_precision(0.));
    REQUIRE(remainder(4., real{5, detail::real_deduce_precision(0.) / 2}).get_prec()
            == detail::real_deduce_precision(0.));
    REQUIRE(remainder(real{4, detail::real_deduce_precision(0) / 2}, 5).get_prec() == detail::real_deduce_precision(0));
    REQUIRE(remainder(4, real{5, detail::real_deduce_precision(0) / 2}).get_prec() == detail::real_deduce_precision(0));
}

TEST_CASE("real remquo")
{
    long q = 0;

    real r0{12, 450};
    remquo(r0, &q, real{1}, sqrt(real{2}));
    REQUIRE(abs(r0 - -0.414213562384) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    real tmp1{1}, tmp2{sqrt(real{2})};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    remquo(r0, &q, std::move(tmp1), tmp2);
    REQUIRE(abs(r0 - -0.414213562384) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp1 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp1 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp1.get_prec() == detail::real_deduce_precision(0) / 2);
    tmp1 = real{1};
    tmp2 = real{sqrt(real{2})};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    remquo(r0, &q, tmp1, std::move(tmp2));
    REQUIRE(abs(r0 - -0.414213562384) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp2 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp2 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp2.get_prec() == detail::real_deduce_precision(0) / 2);
}

#if defined(MPPP_MPFR_HAVE_MPFR_FMODQUO)

TEST_CASE("real fmodquo")
{
    long q = 0;

    real r0{12, 450};
    fmodquo(r0, &q, real{1}, sqrt(real{2}));
    REQUIRE(abs(r0 - 1) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    real tmp1{1}, tmp2{sqrt(real{2})};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    fmodquo(r0, &q, std::move(tmp1), tmp2);
    REQUIRE(abs(r0 - 1) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp1 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp1 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp1.get_prec() == detail::real_deduce_precision(0) / 2);
    tmp1 = real{1};
    tmp2 = real{sqrt(real{2})};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    fmodquo(r0, &q, tmp1, std::move(tmp2));
    REQUIRE(abs(r0 - 1) < 1E-6);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp2 was swapped for r0.
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(tmp2 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp2.get_prec() == detail::real_deduce_precision(0) / 2);
}

#endif
