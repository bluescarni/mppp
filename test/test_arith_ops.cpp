/* Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)

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

#include <cstddef>
#include <gmp.h>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <mp++.hpp>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

// Type traits to detect the availability of operators.
template <typename T, typename U>
using add_t = decltype(std::declval<const T &>() + std::declval<const U &>());

template <typename T, typename U>
using inplace_add_t = decltype(std::declval<T &>() += std::declval<const U &>());

template <typename T, typename U>
using is_addable = is_detected<add_t, T, U>;

template <typename T, typename U>
using is_addable_inplace = is_detected<inplace_add_t, T, U>;

struct add_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Binary add.
        integer n1{1}, n2{-2};
        REQUIRE((lex_cast(+n2) == "-2"));
        REQUIRE((lex_cast(n1 + n2) == "-1"));
        REQUIRE((std::is_same<decltype(n1 + n2), integer>::value));
        REQUIRE((lex_cast(n1 + char(4)) == "5"));
        REQUIRE((lex_cast(char(4) + n2) == "2"));
        REQUIRE((std::is_same<decltype(n1 + char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) + n2), integer>::value));
        REQUIRE((lex_cast(n1 + (unsigned char)(4)) == "5"));
        REQUIRE((lex_cast((unsigned char)(4) + n2) == "2"));
        REQUIRE((lex_cast(n1 + short(4)) == "5"));
        REQUIRE((lex_cast(short(4) + n2) == "2"));
        REQUIRE((lex_cast(n1 + 4) == "5"));
        REQUIRE((lex_cast(4 + n2) == "2"));
        REQUIRE((std::is_same<decltype(n1 + 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 + n2), integer>::value));
        REQUIRE((lex_cast(n1 + 4u) == "5"));
        REQUIRE((lex_cast(4u + n2) == "2"));
        REQUIRE((n1 + 4.f == 5.f));
        REQUIRE((4.f + n2 == 2.f));
        REQUIRE((std::is_same<decltype(n1 + 4.f), float>::value));
        REQUIRE((std::is_same<decltype(4.f + n2), float>::value));
        REQUIRE((n1 + 4. == 5.));
        REQUIRE((4. + n2 == 2.));
        REQUIRE((std::is_same<decltype(n1 + 4.), double>::value));
        REQUIRE((std::is_same<decltype(4. + n2), double>::value));
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE((n1 + 4.l == 5.l));
        REQUIRE((4.l + n2 == 2.l));
        REQUIRE((std::is_same<decltype(n1 + 4.l), long double>::value));
        REQUIRE((std::is_same<decltype(4.l + n2), long double>::value));
#endif
        // In-place add.
        integer retval{1};
        retval += n1;
        REQUIRE((lex_cast(retval) == "2"));
        retval += 1;
        REQUIRE((lex_cast(retval) == "3"));
        retval += short(-1);
        REQUIRE((lex_cast(retval) == "2"));
        retval += (signed char)(-1);
        REQUIRE((lex_cast(retval) == "1"));
        retval += (long long)(-5);
        REQUIRE((lex_cast(retval) == "-4"));
        retval += (unsigned long long)(20);
        REQUIRE((lex_cast(retval) == "16"));
        retval += 2.5f;
        REQUIRE((lex_cast(retval) == "18"));
        retval += -3.5;
        REQUIRE((lex_cast(retval) == "14"));
#if defined(MPPP_WITH_LONG_DOUBLE)
        retval += -1.5l;
        REQUIRE((lex_cast(retval) == "12"));
#endif
        if (std::numeric_limits<double>::is_iec559) {
            retval = 1;
            REQUIRE_THROWS_PREDICATE(retval += std::numeric_limits<double>::infinity(), std::domain_error,
                                     [](const std::domain_error &ex) {
                                         return std::string(ex.what())
                                                == "Cannot init integer from the non-finite floating-point value "
                                                       + std::to_string(std::numeric_limits<double>::infinity());
                                     });
        }
        // Type traits.
        REQUIRE((!is_addable<integer, std::string>::value));
        REQUIRE((!is_addable<std::string, integer>::value));
        REQUIRE((!is_addable_inplace<integer, std::string>::value));
        REQUIRE((!is_addable_inplace<const integer, int>::value));
        REQUIRE((!is_addable_inplace<std::string, integer>::value));
        REQUIRE((!is_addable_inplace<const int, integer>::value));
        // In-place with interop on the lhs.
        short nl = 1;
        nl += integer{1};
        REQUIRE((std::is_same<short &, decltype(nl += integer{1})>::value));
        REQUIRE(nl == 2);
        nl += integer{-3};
        REQUIRE(nl == -1);
        unsigned long long unl = 1;
        unl += integer{1};
        REQUIRE(unl == 2);
        REQUIRE_THROWS_AS(unl += integer{-3}, std::overflow_error);
        REQUIRE_THROWS_AS(unl += integer{std::numeric_limits<unsigned long long>::max()}, std::overflow_error);
        double dl = 1.2;
        dl += integer{1};
        REQUIRE(dl == 1.2 + 1.);
        REQUIRE((std::is_same<double &, decltype(dl += integer{1})>::value));
        if (std::numeric_limits<double>::is_iec559) {
            dl = std::numeric_limits<double>::infinity();
            dl += integer{1};
            REQUIRE(dl == std::numeric_limits<double>::infinity());
        }
        // Increment ops.
        retval = integer{0};
        REQUIRE((lex_cast(++retval) == "1"));
        REQUIRE((lex_cast(++retval) == "2"));
        REQUIRE((std::is_same<decltype(++retval), integer &>::value));
        retval = integer{-2};
        ++retval;
        REQUIRE((lex_cast(retval) == "-1"));
        ++retval;
        REQUIRE((lex_cast(retval) == "0"));
        ++retval;
        REQUIRE((lex_cast(retval) == "1"));
        REQUIRE((lex_cast(retval++) == "1"));
        REQUIRE((lex_cast(retval++) == "2"));
        REQUIRE((lex_cast(retval++) == "3"));
        // Couple of tests at the boundaries
        mpz_raii tmp;
        retval = integer{GMP_NUMB_MAX};
        ::mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        ++retval;
        ::mpz_add_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
        retval = integer{GMP_NUMB_MAX};
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        ::mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        retval++;
        ::mpz_add_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
        retval = integer{GMP_NUMB_MAX};
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        ::mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        retval++;
        ::mpz_add_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
    }
};

TEST_CASE("add")
{
    tuple_for_each(sizes{}, add_tester{});
}

struct sub_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n1{1}, n2{-2};
        REQUIRE((lex_cast(-n2) == "2"));
        REQUIRE((lex_cast(n1 - n2) == "3"));
        REQUIRE((std::is_same<decltype(n1 - n2), integer>::value));
        REQUIRE((lex_cast(n1 - char(4)) == "-3"));
        REQUIRE((lex_cast(char(4) - n2) == "6"));
        REQUIRE((std::is_same<decltype(n1 - char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) - n2), integer>::value));
        REQUIRE((lex_cast(n1 - (unsigned char)(4)) == "-3"));
        REQUIRE((lex_cast((unsigned char)(4) - n2) == "6"));
        REQUIRE((lex_cast(n1 - short(4)) == "-3"));
        REQUIRE((lex_cast(short(4) - n2) == "6"));
        REQUIRE((lex_cast(n1 - 4) == "-3"));
        REQUIRE((lex_cast(4 - n2) == "6"));
        REQUIRE((std::is_same<decltype(n1 - 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 - n2), integer>::value));
        REQUIRE((lex_cast(n1 - 4u) == "-3"));
        REQUIRE((lex_cast(4u - n2) == "6"));
        REQUIRE((n1 - 4.f == -3.f));
        REQUIRE((4.f - n2 == 6.f));
        REQUIRE((std::is_same<decltype(n1 - 4.f), float>::value));
        REQUIRE((std::is_same<decltype(4.f - n2), float>::value));
        REQUIRE((n1 - 4. == -3.));
        REQUIRE((4. - n2 == 6.));
        REQUIRE((std::is_same<decltype(n1 - 4.), double>::value));
        REQUIRE((std::is_same<decltype(4. - n2), double>::value));
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE((n1 - 4.l == -3.l));
        REQUIRE((4.l - n2 == 6.l));
        REQUIRE((std::is_same<decltype(n1 - 4.l), long double>::value));
        REQUIRE((std::is_same<decltype(4.l - n2), long double>::value));
#endif
        // In-place sub.
        integer retval{1};
        retval -= n1;
        REQUIRE((lex_cast(retval) == "0"));
        retval -= 1;
        REQUIRE((lex_cast(retval) == "-1"));
        retval -= short(-1);
        REQUIRE((lex_cast(retval) == "0"));
        retval -= (signed char)(-1);
        REQUIRE((lex_cast(retval) == "1"));
        retval -= (long long)(-5);
        REQUIRE((lex_cast(retval) == "6"));
        retval -= (unsigned long long)(20);
        REQUIRE((lex_cast(retval) == "-14"));
        retval -= 2.5f;
        REQUIRE((lex_cast(retval) == "-16"));
        retval -= -3.5;
        REQUIRE((lex_cast(retval) == "-12"));
#if defined(MPPP_WITH_LONG_DOUBLE)
        retval -= -1.5l;
        REQUIRE((lex_cast(retval) == "-10"));
#endif
        if (std::numeric_limits<double>::is_iec559) {
            retval = 1;
            REQUIRE_THROWS_PREDICATE(retval -= std::numeric_limits<double>::infinity(), std::domain_error,
                                     [](const std::domain_error &ex) {
                                         return std::string(ex.what())
                                                == "Cannot init integer from the non-finite floating-point value "
                                                       + std::to_string(-std::numeric_limits<double>::infinity());
                                     });
        }
        // In-place with interop on the lhs.
        short nl = 1;
        nl -= integer{1};
        REQUIRE((std::is_same<short &, decltype(nl -= integer{1})>::value));
        REQUIRE(nl == 0);
        nl -= integer{-3};
        REQUIRE(nl == 3);
        unsigned long long unl = 1;
        unl -= integer{1};
        REQUIRE(unl == 0);
        REQUIRE_THROWS_AS(unl -= integer{1}, std::overflow_error);
        double dl = 1.2;
        dl -= integer{1};
        REQUIRE(dl == 1.2 - 1.);
        REQUIRE((std::is_same<double &, decltype(dl -= integer{1})>::value));
        if (std::numeric_limits<double>::is_iec559) {
            dl = std::numeric_limits<double>::infinity();
            dl -= integer{1};
            REQUIRE(dl == std::numeric_limits<double>::infinity());
        }
        // Decrement ops.
        retval = integer{0};
        REQUIRE((lex_cast(--retval) == "-1"));
        REQUIRE((lex_cast(--retval) == "-2"));
        REQUIRE((std::is_same<decltype(--retval), integer &>::value));
        retval = integer{2};
        --retval;
        REQUIRE((lex_cast(retval) == "1"));
        --retval;
        REQUIRE((lex_cast(retval) == "0"));
        --retval;
        REQUIRE((lex_cast(retval) == "-1"));
        REQUIRE((lex_cast(retval--) == "-1"));
        REQUIRE((lex_cast(retval--) == "-2"));
        REQUIRE((lex_cast(retval--) == "-3"));
        // Couple of tests at the boundaries
        mpz_raii tmp;
        retval = integer{GMP_NUMB_MAX};
        retval.neg();
        ::mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        --retval;
        ::mpz_sub_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
        retval = integer{GMP_NUMB_MAX};
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        retval.neg();
        ::mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        retval--;
        ::mpz_sub_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
        retval = integer{GMP_NUMB_MAX};
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        mul_2exp(retval, retval, GMP_NUMB_BITS);
        add(retval, retval, integer{GMP_NUMB_MAX});
        retval.neg();
        ::mpz_set(&tmp.m_mpz, retval.get_mpz_view());
        retval--;
        ::mpz_sub_ui(&tmp.m_mpz, &tmp.m_mpz, 1);
        REQUIRE((lex_cast(retval) == lex_cast(tmp)));
    }
};

TEST_CASE("sub")
{
    tuple_for_each(sizes{}, sub_tester{});
}

struct mul_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n1{1}, n2{-2};
        REQUIRE((lex_cast(n1 * n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 * n2), integer>::value));
        REQUIRE((lex_cast(n1 * char(4)) == "4"));
        REQUIRE((lex_cast(char(4) * n2) == "-8"));
        REQUIRE((std::is_same<decltype(n1 * char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) * n2), integer>::value));
        REQUIRE((lex_cast(n1 * (unsigned char)(4)) == "4"));
        REQUIRE((lex_cast((unsigned char)(4) * n2) == "-8"));
        REQUIRE((lex_cast(n1 * short(4)) == "4"));
        REQUIRE((lex_cast(short(4) * n2) == "-8"));
        REQUIRE((lex_cast(n1 * 4) == "4"));
        REQUIRE((lex_cast(4 * n2) == "-8"));
        REQUIRE((std::is_same<decltype(n1 * 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 * n2), integer>::value));
        REQUIRE((lex_cast(n1 * 4u) == "4"));
        REQUIRE((lex_cast(4u * n2) == "-8"));
        REQUIRE((n1 * 4.f == 4.f));
        REQUIRE((4.f * n2 == -8.f));
        REQUIRE((std::is_same<decltype(n1 * 4.f), float>::value));
        REQUIRE((std::is_same<decltype(4.f * n2), float>::value));
        REQUIRE((n1 * 4. == 4.));
        REQUIRE((4. * n2 == -8.));
        REQUIRE((std::is_same<decltype(n1 * 4.), double>::value));
        REQUIRE((std::is_same<decltype(4. * n2), double>::value));
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE((n1 * 4.l == 4.l));
        REQUIRE((4.l * n2 == -8.l));
        REQUIRE((std::is_same<decltype(n1 * 4.l), long double>::value));
        REQUIRE((std::is_same<decltype(4.l * n2), long double>::value));
#endif
        // In-place mul.
        integer retval{1};
        retval *= n1;
        REQUIRE((lex_cast(retval) == "1"));
        retval *= 1;
        REQUIRE((lex_cast(retval) == "1"));
        retval *= short(-1);
        REQUIRE((lex_cast(retval) == "-1"));
        retval *= (signed char)(-1);
        REQUIRE((lex_cast(retval) == "1"));
        retval *= (long long)(-5);
        REQUIRE((lex_cast(retval) == "-5"));
        retval *= (unsigned long long)(20);
        REQUIRE((lex_cast(retval) == "-100"));
        retval *= 2.5f;
        REQUIRE((lex_cast(retval) == "-250"));
        retval *= -3.5;
        REQUIRE((lex_cast(retval) == "875"));
#if defined(MPPP_WITH_LONG_DOUBLE)
        retval *= -1.5l;
        REQUIRE((lex_cast(retval) == "-1312"));
#endif
        if (std::numeric_limits<double>::is_iec559) {
            retval = 1;
            REQUIRE_THROWS_PREDICATE(retval *= std::numeric_limits<double>::infinity(), std::domain_error,
                                     [](const std::domain_error &ex) {
                                         return std::string(ex.what())
                                                == "Cannot init integer from the non-finite floating-point value "
                                                       + std::to_string(std::numeric_limits<double>::infinity());
                                     });
        }
        // In-place with interop on the lhs.
        short nl = 1;
        nl *= integer{3};
        REQUIRE((std::is_same<short &, decltype(nl *= integer{1})>::value));
        REQUIRE(nl == 3);
        nl *= integer{-3};
        REQUIRE(nl == -9);
        unsigned long long unl = 1;
        unl *= integer{2};
        REQUIRE(unl == 2);
        REQUIRE_THROWS_AS(unl *= integer{-1}, std::overflow_error);
        double dl = 1.2;
        dl *= integer{2};
        REQUIRE(dl == 1.2 * 2.);
        REQUIRE((std::is_same<double &, decltype(dl *= integer{1})>::value));
        if (std::numeric_limits<double>::is_iec559) {
            dl = std::numeric_limits<double>::infinity();
            dl *= integer{2};
            REQUIRE(dl == std::numeric_limits<double>::infinity());
        }
    }
};

TEST_CASE("mul")
{
    tuple_for_each(sizes{}, mul_tester{});
}

struct div_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n1{4}, n2{-2};
        REQUIRE((lex_cast(n1 / n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 / n2), integer>::value));
        REQUIRE((lex_cast(n1 / char(4)) == "1"));
        REQUIRE((lex_cast(char(4) / n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 / char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) / n2), integer>::value));
        REQUIRE((lex_cast(n1 / (unsigned char)(4)) == "1"));
        REQUIRE((lex_cast((unsigned char)(4) / n2) == "-2"));
        REQUIRE((lex_cast(n1 / short(4)) == "1"));
        REQUIRE((lex_cast(short(4) / n2) == "-2"));
        REQUIRE((lex_cast(n1 / 4) == "1"));
        REQUIRE((lex_cast(4 / n2) == "-2"));
        REQUIRE((std::is_same<decltype(n1 / 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 / n2), integer>::value));
        REQUIRE((lex_cast(n1 / 4u) == "1"));
        REQUIRE((lex_cast(4u / n2) == "-2"));
        REQUIRE((n1 / 4.f == 1.f));
        REQUIRE((4.f / n2 == -2.f));
        REQUIRE((std::is_same<decltype(n1 / 4.f), float>::value));
        REQUIRE((std::is_same<decltype(4.f / n2), float>::value));
        REQUIRE((n1 / 4. == 1.));
        REQUIRE((4. / n2 == -2.));
        REQUIRE((std::is_same<decltype(n1 / 4.), double>::value));
        REQUIRE((std::is_same<decltype(4. / n2), double>::value));
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE((n1 / 4.l == 1.l));
        REQUIRE((4.l / n2 == -2.l));
        REQUIRE((std::is_same<decltype(n1 / 4.l), long double>::value));
        REQUIRE((std::is_same<decltype(4.l / n2), long double>::value));
#endif
        // In-place div.
        integer retval{2};
        retval /= n1;
        REQUIRE((lex_cast(retval) == "0"));
        retval = 2;
        retval /= 1;
        REQUIRE((lex_cast(retval) == "2"));
        retval /= short(-1);
        REQUIRE((lex_cast(retval) == "-2"));
        retval /= (signed char)(-1);
        REQUIRE((lex_cast(retval) == "2"));
        retval /= (long long)(-5);
        REQUIRE((lex_cast(retval) == "0"));
        retval = -20;
        retval /= (unsigned long long)(20);
        REQUIRE((lex_cast(retval) == "-1"));
        retval /= 2.5f;
        REQUIRE((lex_cast(retval) == "0"));
        retval = 10;
        retval /= -3.5;
        REQUIRE((lex_cast(retval) == lex_cast(integer{10. / -3.5})));
#if defined(MPPP_WITH_LONG_DOUBLE)
        retval /= -1.5l;
        REQUIRE((lex_cast(retval) == lex_cast(integer{10. / -3.5 / -1.5l})));
#endif
        // In-place with interop on the lhs.
        short nl = 12;
        nl /= integer{3};
        REQUIRE((std::is_same<short &, decltype(nl /= integer{1})>::value));
        REQUIRE(nl == 4);
        nl /= integer{-2};
        REQUIRE(nl == -2);
        REQUIRE_THROWS_AS(nl /= integer{}, zero_division_error);
        unsigned long long unl = 24;
        unl /= integer{2};
        REQUIRE(unl == 12);
        REQUIRE_THROWS_AS(unl /= integer{-1}, std::overflow_error);
        double dl = 1.2;
        dl /= integer{2};
        REQUIRE(dl == 1.2 / 2.);
        REQUIRE((std::is_same<double &, decltype(dl /= integer{1})>::value));
        if (std::numeric_limits<double>::is_iec559) {
            dl = std::numeric_limits<double>::infinity();
            dl /= integer{2};
            REQUIRE(dl == std::numeric_limits<double>::infinity());
        }
        // Error checking.
        REQUIRE_THROWS_PREDICATE(integer{1} / integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(integer{1} / 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(1 / integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval /= integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval /= 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        if (std::numeric_limits<double>::is_iec559) {
            REQUIRE((integer{4} / 0. == std::numeric_limits<double>::infinity()));
            REQUIRE((integer{-4} / 0. == -std::numeric_limits<double>::infinity()));
            REQUIRE_THROWS_PREDICATE(retval /= 0., std::domain_error, [&retval](const std::domain_error &ex) {
                return std::string(ex.what())
                       == "Cannot init integer from the non-finite floating-point value "
                              + (retval.sgn() > 0 ? std::to_string(std::numeric_limits<double>::infinity())
                                                  : std::to_string(-std::numeric_limits<double>::infinity()));
            });
        }
    }
};

TEST_CASE("div")
{
    tuple_for_each(sizes{}, div_tester{});
}

struct shift_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer ret;
        REQUIRE((lex_cast(ret << 0) == "0"));
        REQUIRE((lex_cast(ret << 1u) == "0"));
        REQUIRE((lex_cast(ret << short(2)) == "0"));
        ret = 1;
        REQUIRE((lex_cast(ret << 1) == "2"));
        REQUIRE((lex_cast(ret << 2ll) == "4"));
        ret.neg();
        REQUIRE((lex_cast(ret << 3ull) == "-8"));
        REQUIRE((lex_cast(ret <<= 3ull) == "-8"));
        REQUIRE((lex_cast(ret <<= char(1)) == "-16"));
        REQUIRE((lex_cast(ret <<= (signed char)(0)) == "-16"));
        REQUIRE((lex_cast(ret >> 0) == "-16"));
        REQUIRE((lex_cast(ret >> 1) == "-8"));
        REQUIRE((lex_cast(ret >>= 1ul) == "-8"));
        REQUIRE((lex_cast(ret >>= short(1)) == "-4"));
        REQUIRE((lex_cast(ret >> 128) == "0"));
        // Error handling.
        REQUIRE_THROWS_PREDICATE(ret << -1, std::domain_error, [](const std::domain_error &ex) {
            return std::string(ex.what()) == "Cannot bit shift by -1: negative values are not supported";
        });
        REQUIRE_THROWS_PREDICATE(ret <<= -2, std::domain_error, [](const std::domain_error &ex) {
            return std::string(ex.what()) == "Cannot bit shift by -2: negative values are not supported";
        });
        REQUIRE_THROWS_PREDICATE(ret >> -1, std::domain_error, [](const std::domain_error &ex) {
            return std::string(ex.what()) == "Cannot bit shift by -1: negative values are not supported";
        });
        REQUIRE_THROWS_PREDICATE(ret >>= -2, std::domain_error, [](const std::domain_error &ex) {
            return std::string(ex.what()) == "Cannot bit shift by -2: negative values are not supported";
        });
        if (std::numeric_limits<unsigned long long>::max() > std::numeric_limits<::mp_bitcnt_t>::max()) {
            REQUIRE_THROWS_PREDICATE(ret << std::numeric_limits<unsigned long long>::max(), std::domain_error,
                                     [](const std::domain_error &ex) {
                                         return std::string(ex.what())
                                                == "Cannot bit shift by "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + ": the value is too large";
                                     });
            REQUIRE_THROWS_PREDICATE(ret <<= std::numeric_limits<unsigned long long>::max(), std::domain_error,
                                     [](const std::domain_error &ex) {
                                         return std::string(ex.what())
                                                == "Cannot bit shift by "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + ": the value is too large";
                                     });
            REQUIRE_THROWS_PREDICATE(ret >> std::numeric_limits<unsigned long long>::max(), std::domain_error,
                                     [](const std::domain_error &ex) {
                                         return std::string(ex.what())
                                                == "Cannot bit shift by "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + ": the value is too large";
                                     });
            REQUIRE_THROWS_PREDICATE(ret >>= std::numeric_limits<unsigned long long>::max(), std::domain_error,
                                     [](const std::domain_error &ex) {
                                         return std::string(ex.what())
                                                == "Cannot bit shift by "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + ": the value is too large";
                                     });
        }
        if ((unsigned long long)std::numeric_limits<long long>::max() > std::numeric_limits<::mp_bitcnt_t>::max()) {
            REQUIRE_THROWS_PREDICATE(
                ret << std::numeric_limits<long long>::max(), std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot bit shift by " + std::to_string(std::numeric_limits<long long>::max())
                                  + ": the value is too large";
                });
            REQUIRE_THROWS_PREDICATE(
                ret <<= std::numeric_limits<long long>::max(), std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot bit shift by " + std::to_string(std::numeric_limits<long long>::max())
                                  + ": the value is too large";
                });
            REQUIRE_THROWS_PREDICATE(
                ret >> std::numeric_limits<long long>::max(), std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot bit shift by " + std::to_string(std::numeric_limits<long long>::max())
                                  + ": the value is too large";
                });
            REQUIRE_THROWS_PREDICATE(
                ret >>= std::numeric_limits<long long>::max(), std::domain_error, [](const std::domain_error &ex) {
                    return std::string(ex.what())
                           == "Cannot bit shift by " + std::to_string(std::numeric_limits<long long>::max())
                                  + ": the value is too large";
                });
        }
    }
};

TEST_CASE("shift")
{
    tuple_for_each(sizes{}, shift_tester{});
}

struct mod_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n1{4}, n2{-2};
        REQUIRE((lex_cast(n1 % n2) == "0"));
        REQUIRE((std::is_same<decltype(n1 % n2), integer>::value));
        REQUIRE((lex_cast(n1 % char(3)) == "1"));
        REQUIRE((lex_cast(char(3) % n2) == "1"));
        REQUIRE((std::is_same<decltype(n1 % char(4)), integer>::value));
        REQUIRE((std::is_same<decltype(char(4) % n2), integer>::value));
        REQUIRE((lex_cast(-n1 % (unsigned char)(3)) == "-1"));
        REQUIRE((lex_cast((unsigned char)(3) % n2) == "1"));
        REQUIRE((lex_cast(n1 % short(3)) == "1"));
        REQUIRE((lex_cast(-short(3) % n2) == "-1"));
        REQUIRE((lex_cast(n1 % -3) == "1"));
        REQUIRE((lex_cast(3 % -n2) == "1"));
        REQUIRE((std::is_same<decltype(n1 % 4), integer>::value));
        REQUIRE((std::is_same<decltype(4 % n2), integer>::value));
        REQUIRE((lex_cast(n1 % 3u) == "1"));
        REQUIRE((lex_cast(3u % n2) == "1"));
        REQUIRE((lex_cast(0u % n2) == "0"));
        // In-place mod.
        integer retval{-2};
        retval %= n1;
        REQUIRE((lex_cast(retval) == "-2"));
        retval = 3;
        retval %= 2;
        REQUIRE((lex_cast(retval) == "1"));
        retval = -3;
        retval %= short(2);
        REQUIRE((lex_cast(retval) == "-1"));
        retval %= (signed char)(-1);
        REQUIRE((lex_cast(retval) == "0"));
        retval = 26;
        retval %= (long long)(-5);
        REQUIRE((lex_cast(retval) == "1"));
        retval = -19;
        retval %= (unsigned long long)(7);
        REQUIRE((lex_cast(retval) == "-5"));
        // Error checking.
        REQUIRE_THROWS_PREDICATE(integer{1} % integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(integer{1} % 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(1 % integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval %= integer{0}, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
        REQUIRE_THROWS_PREDICATE(retval %= 0, zero_division_error, [](const zero_division_error &ex) {
            return std::string(ex.what()) == "Integer division by zero";
        });
    }
};

TEST_CASE("mod")
{
    tuple_for_each(sizes{}, mod_tester{});
}

struct rel_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n1{4}, n2{-2};

        REQUIRE(n1 != n2);
        REQUIRE(n1 == n1);
        REQUIRE(integer{} == integer{});
        REQUIRE(integer{} == 0);
        REQUIRE(0 == integer{});
        REQUIRE(n1 == 4);
        REQUIRE(4u == n1);
        REQUIRE(n1 != 3);
        REQUIRE((signed char)-3 != n1);
        REQUIRE(4ull == n1);
        REQUIRE(-2 == n2);
        REQUIRE(n2 == short(-2));
        REQUIRE(-2.f == n2);
        REQUIRE(n2 == -2.f);
        REQUIRE(-3.f != n2);
        REQUIRE(n2 != -3.f);
        REQUIRE(-2. == n2);
        REQUIRE(n2 == -2.);
        REQUIRE(-3. != n2);
        REQUIRE(n2 != -3.);
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE(-2.l == n2);
        REQUIRE(n2 == -2.l);
        REQUIRE(-3.l != n2);
        REQUIRE(n2 != -3.l);
#endif

        REQUIRE(n2 < n1);
        REQUIRE(n2 < 0);
        REQUIRE(-3 < n2);
        REQUIRE(n2 < 0u);
        REQUIRE(-3ll < n2);
        REQUIRE(n2 < 0.f);
        REQUIRE(-3.f < n2);
        REQUIRE(n2 < 0.);
        REQUIRE(-3. < n2);
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE(n2 < 0.l);
        REQUIRE(-3.l < n2);
#endif

        REQUIRE(n1 > n2);
        REQUIRE(0 > n2);
        REQUIRE(n2 > -3);
        REQUIRE(0u > n2);
        REQUIRE(n2 > -3ll);
        REQUIRE(0.f > n2);
        REQUIRE(n2 > -3.f);
        REQUIRE(0. > n2);
        REQUIRE(n2 > -3.);
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE(0.l > n2);
        REQUIRE(n2 > -3.l);
#endif

        REQUIRE(n2 <= n1);
        REQUIRE(n1 <= n1);
        REQUIRE(integer{} <= integer{});
        REQUIRE(integer{} <= 0);
        REQUIRE(0 <= integer{});
        REQUIRE(-2 <= n2);
        REQUIRE(n2 <= -2);
        REQUIRE(n2 <= 0);
        REQUIRE(-3 <= n2);
        REQUIRE(n2 <= 0u);
        REQUIRE(-3ll <= n2);
        REQUIRE(n2 <= 0.f);
        REQUIRE(-3.f <= n2);
        REQUIRE(-2.f <= n2);
        REQUIRE(n2 <= -2.f);
        REQUIRE(n2 <= 0.);
        REQUIRE(-3. <= n2);
        REQUIRE(-2. <= n2);
        REQUIRE(n2 <= -2.);
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE(n2 <= 0.l);
        REQUIRE(-3.l <= n2);
        REQUIRE(-2.l <= n2);
        REQUIRE(n2 <= -2.l);
#endif

        REQUIRE(n1 >= n2);
        REQUIRE(n1 >= n1);
        REQUIRE(integer{} >= integer{});
        REQUIRE(integer{} >= 0);
        REQUIRE(0 >= integer{});
        REQUIRE(-2 >= n2);
        REQUIRE(n2 >= -2);
        REQUIRE(0 >= n2);
        REQUIRE(n2 >= -3);
        REQUIRE(0u >= n2);
        REQUIRE(n2 >= -3ll);
        REQUIRE(0.f >= n2);
        REQUIRE(n2 >= -3.f);
        REQUIRE(-2.f >= n2);
        REQUIRE(n2 >= -2.f);
        REQUIRE(0. >= n2);
        REQUIRE(n2 >= -3.);
        REQUIRE(-2. >= n2);
        REQUIRE(n2 >= -2.);
#if defined(MPPP_WITH_LONG_DOUBLE)
        REQUIRE(0.l >= n2);
        REQUIRE(n2 >= -3.l);
        REQUIRE(-2.l >= n2);
        REQUIRE(n2 >= -2.l);
#endif
    }
};

TEST_CASE("rel")
{
    tuple_for_each(sizes{}, rel_tester{});
}
