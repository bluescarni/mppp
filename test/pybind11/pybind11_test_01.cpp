#include <mp++/extra/pybind11.hpp>
#include <mp++/mp++.hpp>
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_PLUGIN(pybind11_test_01)
{
    py::module m("pybind11_test_01", "");

    mppp_pybind11::init(m);

    m.def("test_int1_conversion", [](const mppp::integer<1> &n) { return n; });
    m.def("test_int2_conversion", [](const mppp::integer<2> &n) { return n; });

    m.def("test_rat1_conversion", [](const mppp::rational<1> &q) { return q; });
    m.def("test_rat2_conversion", [](const mppp::rational<2> &q) { return q; });

#if defined(MPPP_WITH_MPFR)
    m.def("test_real_conversion", [](const mppp::real &r) { return r; });
    m.def("flup", []() { return mppp::real{"1.1", 1024}; });
#endif

    return m.ptr();
}
