.. _tutorial_pybind11:

Extra: integration with pybind11
================================

.. warning::

   The functionality described in this section is experimental, and it may be
   subject to API changes in future versions.

.. versionadded:: 0.6

*#include <mp++/extra/pybind11.hpp>*

The ``mp++/extra/pybind11.hpp`` header provides facilities to seamlessly translate
mp++ multiprecision objects to/from Python in `pybind11 <https://github.com/pybind/pybind11>`__
modules. pybind11 is a C++11 library that, similarly to the older
`Boost.Python <http://www.boost.org/doc/libs/1_66_0/libs/python/doc/html/index.html>`__ library,
allows to use C++ functions and classes from Python.

The API for the pybind11 integration currently includes a single function in the ``mppp_pybind11`` namespace:

.. doxygenfunction:: mppp_pybind11::init()

Including the ``mp++/extra/pybind11.hpp`` header and invoking the :cpp:func:`mppp_pybind11::init()` function will register
`custom type casters <http://pybind11.readthedocs.io/en/master/advanced/cast/custom.html>`__
that will automatically translate to/from Python mp++ objects used as arguments and return values in
functions exposed from C++. The translation rules are the following:

* :cpp:class:`~mppp::integer` objects are converted to/from Python integers,
* :cpp:class:`~mppp::rational` objects are converted to/from Python `fractions <https://docs.python.org/3.6/library/fractions.html>`__,
* :cpp:class:`~mppp::real` and :cpp:class:`~mppp::real128` objects are translated to/from `mpmath's mpf objects <http://mpmath.org/>`__.

If the mpmath library is not installed, the translations for :cpp:class:`~mppp::real128` and :cpp:class:`~mppp::real` will be disabled
at runtime.

Let's take a look at an example of a pybind11 module enabling automatic translation of mp++ objects:

.. code-block:: c++

    #include <mp++/mp++.hpp>
    #include <mp++/extra/pybind11.hpp>

    #include <string>
    #include <unordered_map>
    #include <vector>

    #include <pybind11/pybind11.h>
    #include <pybind11/stl.h>

    // A couple of functions accepting and returning C++ containers.
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
        // Init the pybind11 integration for this module.
        mppp_pybind11::init();

        // Expose a few functions testing the automatic translation of mp++ objects.
        // -------------------------------------------------------------------------

        m.def("test_int1_conversion", [](const mppp::integer<1> &n) { return n; });

        m.def("test_rat1_conversion", [](const mppp::rational<1> &q) { return q; });

        m.def("test_real_conversion", [](const mppp::real &r) { return r; });
        m.def("test_real_conversion", [](const mppp::real &r, ::mpfr_prec_t prec) { return mppp::real{r, prec}; });

        m.def("test_real128_conversion", [](const mppp::real128 &r) { return r; });

        m.def("test_overload", [](const mppp::integer<1> &n) { return n; });
        m.def("test_overload", [](const mppp::rational<1> &q) { return q; });
        m.def("test_overload", [](const mppp::real128 &r) { return r; });
        m.def("test_overload", [](const mppp::real &r) { return r; });

        m.def("test_vector_conversion", test_vector<mppp::integer<1>>);
        m.def("test_vector_conversion", test_vector<mppp::rational<1>>);
        m.def("test_vector_conversion", test_vector<mppp::real128>);
        m.def("test_vector_conversion", test_vector<mppp::real>);

        m.def("test_unordered_map_conversion", test_unordered_map<mppp::integer<1>>);
        m.def("test_unordered_map_conversion", test_unordered_map<mppp::rational<1>>);
        m.def("test_unordered_map_conversion", test_unordered_map<mppp::real128>);
        m.def("test_unordered_map_conversion", test_unordered_map<mppp::real>);
    }

Note that we have exposed functions which just return a copy of their input parameter.
This will allow us to verify that the automatic translation between mp++ and Python objects
works as intended. Now let's try to call the exposed functions from Python:

>>> import pybind11_test_01 as p
>>> from fractions import Fraction as F
>>> p.test_int1_conversion(42)
42
>>> p.test_int1_conversion(-1)
-1
>>> p.test_rat1_conversion(F(3, 4))
Fraction(3, 4)
>>> p.test_rat1_conversion(F(-1, 2))
Fraction(-1, 2)

Indeed, the Python objects passed as arguments to the exposed functions are correctly translated to mp++ objects
before being passed to the C++ functions, and the mp++ return values are correctly translated back to the original Python objects.

Let's try with some floating-point objects:

>>> from mpmath import mpf, mp
>>> p.test_real_conversion(mpf("1.1"))
mpf('1.1000000000000001')

The default precision in mpmath is 53 (double-precision), and indeed the conversion between ``mpf`` on the Python side
and :cpp:class:`~mppp::real` on the C++ side is done with 53 bits of precision. We can increase the precision to 200 bits and
verify that the value is correctly preserved and translated:

>>> mp.prec = 200
>>> p.test_real_conversion(mpf("1.1"))
mpf('1.1000000000000000000000000000000000000000000000000000000000002')

If the precision is set **exactly** to 113, ``mpf`` objects can be converted to :cpp:class:`~mppp::real128`:

>>> mp.prec = 113
>>> p.test_real128_conversion(mpf("1.1"))
mpf('1.10000000000000000000000000000000008')
>>> mp.prec = 114
>>> p.test_real128_conversion(mpf("1.1"))
Traceback (most recent call last):
     ...
TypeError: test_real128_conversion(): incompatible function arguments. The following argument types are supported:
    1. (arg0: real128) -> real128
<BLANKLINE>
Invoked with: mpf('1.09999999999999999999999999999999998')

A :cpp:class:`~mppp::real128` will be successfully converted to an ``mpf`` iff the current mpmath working precision is exactly 113.
A :cpp:class:`~mppp::real` will be successfully converted to an ``mpf`` iff its precision is not greater than the current mpmath working precision:

>>> mp.prec = 53;
>>> p.test_real_conversion(mpf("1.1"), 100)
Traceback (most recent call last):
     ...
ValueError: Cannot convert the real 1.1000000000000000888178419700125 to an mpf: the precision of the real (100) is smaller than the current mpf precision (53). Please increase the current mpf precision to at least 100 in order to avoid this error
>>> mp.prec = 100;
>>> p.test_real_conversion(mpf("1.1"), 100)
mpf('1.1000000000000000000000000000003')

Overloaded functions are supported as well:

>>> p.test_overload(-2)
-2
>>> p.test_overload(F(6, 7))
Fraction(6, 7)
>>> p.test_overload(mpf("1.3"))
mpf('1.2999999999999999999999999999994')

Note that, due to the fact that ``mpf`` arguments can be converted both to :cpp:class:`~mppp::real128` and :cpp:class:`~mppp::real`,
overloads with :cpp:class:`~mppp::real128` arguments should be exposed **before** overloads with :cpp:class:`~mppp::real` arguments
(otherwise, the :cpp:class:`~mppp::real` overload will always be preferred).

.. note::

    There's an important caveat to keep in mind when translating to/from :cpp:class:`~mppp::real128`. The IEEE 754 quadruple precision
    format, implemented by :cpp:class:`~mppp::real128`, has a limited exponent range. For instance, the value :math:`2^{-30000}` becomes
    simply zero in quadruple precision, but, in mpmath, it doesn't:

    >>> mp.prec = 113
    >>> mpf(2)**-30000
    mpf('1.25930254358409145729153078521520406e-9031')

    This happens because mpmath features a much larger (practically unlimited) range for the value of the exponent.
    As a consequence, a conversion from ``mpf`` to :cpp:class:`~mppp::real128` will **not** preserve the exact value if the absolute value of the
    exponent is too large:

    >>> p.test_real128_conversion(mpf(2)**-30000)
    mpf('0.0')
    >>> p.test_real128_conversion(mpf(2)**30000)
    mpf('+inf')

Finally, we can verify that the conversion between mp++ and Python works also when containers are involved:

>>> p.test_vector_conversion([1, 2, 3])
[1, 2, 3]
>>> p.test_vector_conversion([F(1), F(1, 2), F(1, 3)])
[Fraction(1, 1), Fraction(1, 2), Fraction(1, 3)]
>>> p.test_vector_conversion([mpf(1), mpf(2), mpf(3)])
[mpf('1.0'), mpf('2.0'), mpf('3.0')]
>>> p.test_unordered_map_conversion({'a': 1, 'b': 3})
{'a': 1, 'b': 3}
>>> p.test_unordered_map_conversion({'a': F(1, 2), 'b': F(1, 3)})
{'a': Fraction(1, 2), 'b': Fraction(1, 3)}
>>> p.test_unordered_map_conversion({'a': mpf(1), 'b': mpf(3)})
{'a': mpf('1.0'), 'b': mpf('3.0')}
