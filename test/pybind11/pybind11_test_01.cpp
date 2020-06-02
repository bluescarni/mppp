#include <mp++/extra/pybind11.hpp>
#include <mp++/mp++.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(__clang__) || defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#if defined(__clang__) || defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

template <typename T>
static inline std::vector<T> test_vector(const std::vector<T> &v)
{
    return v;
}

template <typename T>
static inline std::unordered_map<std::string, T> test_unordered_map(const std::unordered_map<std::string, T> &um)
{
    return um;
}

PYBIND11_MODULE(pybind11_test_01, m)
{
    mppp_pybind11::init();

    m.def("has_quadmath", []() {
#if defined(MPPP_WITH_QUADMATH)
        return true;
#else
        return false;
#endif
    });

    m.def("has_mpfr", []() {
#if defined(MPPP_WITH_MPFR)
        return true;
#else
        return false;
#endif
    });

    m.def("has_mpc", []() {
#if defined(MPPP_WITH_MPC)
        return true;
#else
        return false;
#endif
    });

    m.def("test_int1_conversion", [](const mppp::integer<1> &n) { return n; });
    m.def("test_int2_conversion", [](const mppp::integer<2> &n) { return n; });

    m.def("test_rat1_conversion", [](const mppp::rational<1> &q) { return q; });
    m.def("test_rat2_conversion", [](const mppp::rational<2> &q) { return q; });

#if defined(MPPP_WITH_MPFR)
    m.def("test_real_conversion", [](const mppp::real &r) { return r; });
    m.def("test_real_conversion", [](const mppp::real &r, ::mpfr_prec_t prec) { return mppp::real{r, prec}; });
#endif

#if defined(MPPP_WITH_MPC)
    m.def("test_complex_conversion", [](const mppp::complex &c) { return c; });
    // NOTE: apparent coverage detection issues with multiline lambdas.
    // LCOV_EXCL_START
    m.def("test_complex_conversion", [](const mppp::complex &c, ::mpfr_prec_t prec) {
        return mppp::complex{c, mppp::complex_prec_t(prec)};
    });
    // LCOV_EXCL_STOP
#endif

#if defined(MPPP_WITH_QUADMATH)
    m.def("test_real128_conversion", [](const mppp::real128 &r) { return r; });
    m.def("test_complex128_conversion", [](const mppp::complex128 &c) { return c; });
#endif

    m.def("test_overload", [](const mppp::integer<1> &n) { return n; });
    m.def("test_overload", [](const mppp::rational<1> &q) { return q; });
#if defined(MPPP_WITH_QUADMATH)
    m.def("test_overload", [](const mppp::real128 &r) { return r; });
    m.def("test_overload", [](const mppp::complex128 &c) { return c; });
#endif
#if defined(MPPP_WITH_MPFR)
    m.def("test_overload", [](const mppp::real &r) { return r; });
#endif
#if defined(MPPP_WITH_MPC)
    m.def("test_overload", [](const mppp::complex &c) { return c; });
#endif

    m.def("test_vector_conversion", test_vector<mppp::integer<1>>);
    m.def("test_vector_conversion", test_vector<mppp::integer<2>>);
    m.def("test_vector_conversion", test_vector<mppp::rational<1>>);
    m.def("test_vector_conversion", test_vector<mppp::rational<2>>);
#if defined(MPPP_WITH_QUADMATH)
    m.def("test_vector_conversion", test_vector<mppp::real128>);
    m.def("test_vector_conversion", test_vector<mppp::complex128>);
#endif
#if defined(MPPP_WITH_MPFR)
    m.def("test_vector_conversion", test_vector<mppp::real>);
#endif
#if defined(MPPP_WITH_MPC)
    m.def("test_vector_conversion", test_vector<mppp::complex>);
#endif

    m.def("test_unordered_map_conversion", test_unordered_map<mppp::integer<1>>);
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::integer<2>>);
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::rational<1>>);
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::rational<2>>);
#if defined(MPPP_WITH_QUADMATH)
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::real128>);
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::complex128>);
#endif
#if defined(MPPP_WITH_MPFR)
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::real>);
#endif
#if defined(MPPP_WITH_MPC)
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::complex>);
#endif

    m.def("test_zero_division_error", []() { return mppp::integer<1>{1} / 0; });
}
