// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>
#include <type_traits>
#include <utility>

#include <mp++/complex.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("sin")
{
    complex r0{0};
    r0.sin();
    REQUIRE(std::is_same<complex &, decltype(r0.sin())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 0;
    complex rop;
    REQUIRE(sin(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(sin(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 0;
    REQUIRE(sin(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(sin(r0))>::value);
    REQUIRE(sin(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = -3477145.505821145952150948552814901255983599021178846624357468003184819234122082_r128
                     - 11566109.75061239466450916613575380278841378386526844455469668582818457691027414_icr128;
    REQUIRE(abs(sin(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(sin(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    sin(rop, r0);
    REQUIRE(abs(sin(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.sin();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("cos")
{
    complex r0{0};
    r0.cos();
    REQUIRE(std::is_same<complex &, decltype(r0.cos())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    r0 = 0;
    complex rop;
    REQUIRE(cos(rop, r0) == 1);
    REQUIRE(std::is_same<complex &, decltype(cos(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 0;
    REQUIRE(cos(r0) == 1);
    REQUIRE(std::is_same<complex, decltype(cos(r0))>::value);
    REQUIRE(cos(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = -11566109.75061243431101520956556591949723754219053371249017516792370714209598772_r128
                     + 3477145.50582113403313294830227617866277878730213937550348996784857152867760457_icr128;
    REQUIRE(abs(cos(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(cos(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    cos(rop, r0);
    REQUIRE(abs(cos(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.cos();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("tan")
{
    complex r0{0};
    r0.tan();
    REQUIRE(std::is_same<complex &, decltype(r0.tan())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 0;
    complex rop;
    REQUIRE(tan(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(tan(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 0;
    REQUIRE(tan(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(tan(r0))>::value);
    REQUIRE(tan(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp
        = 0.000000000000001890189676714721384220241981190955655135507359104069236873613135618527690521095_r128
          + 0.9999999999999971404350972771617738963031747668395411637027059575795413115886363_icr128;
    REQUIRE(abs(tan(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(tan(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    tan(rop, r0);
    REQUIRE(abs(tan(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.tan();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("asin")
{
    complex r0{0};
    r0.asin();
    REQUIRE(std::is_same<complex &, decltype(r0.asin())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 0;
    complex rop;
    REQUIRE(asin(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(asin(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 0;
    REQUIRE(asin(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(asin(r0))>::value);
    REQUIRE(asin(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = 0.7546465680316338218183628152858352854952339799986454116643882618292191135655677_r128
                     + 3.843568166588382136361882162833819833941936267903054343013316986770713537294589_icr128;
    REQUIRE(abs(asin(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(asin(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    asin(rop, r0);
    REQUIRE(abs(asin(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.asin();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("acos")
{
    complex r0{1};
    r0.acos();
    REQUIRE(std::is_same<complex &, decltype(r0.acos())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 1;
    complex rop;
    REQUIRE(acos(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(acos(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 1;
    REQUIRE(acos(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(acos(r0))>::value);
    REQUIRE(acos(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = 0.8161497587632627974129588763539161566033507196889074988230840343246890895775313_r128
                     - 3.843568166588382136361882162833819833941936267903054343013316986770713537294589_icr128;
    REQUIRE(abs(acos(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(acos(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    acos(rop, r0);
    REQUIRE(abs(acos(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.acos();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("atan")
{
    complex r0{0};
    r0.atan();
    REQUIRE(std::is_same<complex &, decltype(r0.atan())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 0;
    complex rop;
    REQUIRE(atan(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(atan(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 0;
    REQUIRE(atan(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(atan(r0))>::value);
    REQUIRE(atan(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = 1.541418415437035272780230040413865549360259490637330587415721014670186092931507_r128
                     + 0.03117586962523930896585162884521581712290333191736875549738954416580110376441166_icr128;
    REQUIRE(abs(atan(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(atan(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    atan(rop, r0);
    REQUIRE(abs(atan(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.atan();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("sin_cos")
{
    using Catch::Matchers::Message;

    // sin_cos.
    complex sop{1, complex_prec_t(detail::real_deduce_precision(0) * 2)},
        cop{2, complex_prec_t(detail::real_deduce_precision(0) * 3)};
    sin_cos(sop, cop, complex{32});
    REQUIRE(sop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(cop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sop == sin(real{32}));
    REQUIRE(cop == cos(real{32}));

    REQUIRE_THROWS_MATCHES(
        sin_cos(sop, sop, complex{32}), std::invalid_argument,
        Message("In the complex sin_cos() function, the return values 'sop' and 'cop' must be distinct objects"));

    // Try with overlapping op/sop and op/cop.
    sop = real{1, detail::real_deduce_precision(0) * 2};
    cop = real{2, detail::real_deduce_precision(0) * 3};
    sin_cos(sop, cop, sop);
    REQUIRE(sop.get_prec() == detail::real_deduce_precision(0) * 2);
    REQUIRE(cop.get_prec() == detail::real_deduce_precision(0) * 2);
    REQUIRE(sop == sin(real{1, detail::real_deduce_precision(0) * 2}));
    REQUIRE(cop == cos(real{1, detail::real_deduce_precision(0) * 2}));

    sop = real{1, detail::real_deduce_precision(0) * 2};
    cop = real{2, detail::real_deduce_precision(0) * 3};
    sin_cos(sop, cop, cop);
    REQUIRE(sop.get_prec() == detail::real_deduce_precision(0) * 3);
    REQUIRE(cop.get_prec() == detail::real_deduce_precision(0) * 3);
    REQUIRE(sop == sin(real{2, detail::real_deduce_precision(0) * 3}));
    REQUIRE(cop == cos(real{2, detail::real_deduce_precision(0) * 3}));
}
