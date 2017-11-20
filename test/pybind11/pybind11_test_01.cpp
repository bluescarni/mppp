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

namespace py = pybind11;

template <typename T>
static inline std::vector<T> test_vector(const std::vector<T> &v)
{
    return v;
}

template <typename T>
static inline std::unordered_map<std::string, T> test_unordered_map(const std::unordered_map<std::string, T> &us)
{
    return us;
}

PYBIND11_MODULE(pybind11_test_01, m)
{
    mppp_pybind11::init(m);

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

    m.def("test_int1_conversion", [](const mppp::integer<1> &n) { return n; });
    m.def("test_int2_conversion", [](const mppp::integer<2> &n) { return n; });

    m.def("test_rat1_conversion", [](const mppp::rational<1> &q) { return q; });
    m.def("test_rat2_conversion", [](const mppp::rational<2> &q) { return q; });

#if defined(MPPP_WITH_MPFR)
    m.def("test_real_conversion", [](const mppp::real &r) { return r; });
    m.def("test_real_conversion", [](const mppp::real &r, ::mpfr_prec_t prec) { return mppp::real{r, prec}; });
#endif

#if defined(MPPP_WITH_QUADMATH)
    m.def("test_real128_conversion", [](const mppp::real128 &r) { return r; });
#endif

    m.def("test_overload", [](const mppp::integer<1> &n) { return n; });
    m.def("test_overload", [](const mppp::rational<1> &q) { return q; });
#if defined(MPPP_WITH_QUADMATH)
    m.def("test_overload", [](const mppp::real128 &r) { return r; });
#endif
#if defined(MPPP_WITH_MPFR)
    m.def("test_overload", [](const mppp::real &r) { return r; });
#endif

    m.def("test_vector_conversion", test_vector<mppp::integer<1>>);
    m.def("test_vector_conversion", test_vector<mppp::integer<2>>);
    m.def("test_vector_conversion", test_vector<mppp::rational<1>>);
    m.def("test_vector_conversion", test_vector<mppp::rational<2>>);
#if defined(MPPP_WITH_QUADMATH)
    m.def("test_vector_conversion", test_vector<mppp::real128>);
#endif
#if defined(MPPP_WITH_MPFR)
    m.def("test_vector_conversion", test_vector<mppp::real>);
#endif

    m.def("test_unordered_map_conversion", test_unordered_map<mppp::integer<1>>);
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::integer<2>>);
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::rational<1>>);
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::rational<2>>);
#if defined(MPPP_WITH_QUADMATH)
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::real128>);
#endif
#if defined(MPPP_WITH_MPFR)
    m.def("test_unordered_map_conversion", test_unordered_map<mppp::real>);
#endif
}
