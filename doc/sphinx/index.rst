.. mp++ documentation master file, created by
   sphinx-quickstart on Fri Dec 23 14:58:38 2016.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to mp++'s documentation!
================================

mp++ is a small C++11 header-only library for multiprecision integer arithmetic. Based
on the well-known `GMP <http://www.gmplib.org>`__ library, mp++ places a strong emphasis on
optimising operations on small values. When operating on small operands, mp++ will:

* avoid heap memory allocations as much as possible, and
* use optimised implementations of basic operations (instead of calling GMP functions).

The combination of these two techniques results in a performance increase up to ~3x, on small operands,
with respect to GMP. The price to pay is a small overhead when operating on large operands.

mp++ was created to cater to the requirements of computer algebra systems, which typically need to be able
to manipulate arbitrarily-large integers but which, in practice, often end up storing many small integers
(e.g., as coefficients in a polynomial).

Installation
------------

mp++ is written in modern C++, and it requires a C++11-capable compiler. It is known to run
on the following setups:

* GCC 4.8 and later versions on GNU/Linux 32/64-bit,
* Clang 3.8 on GNU/Linux 64-bit,
* MSVC15 on Windows 32/64-bit,
* MinGW GCC 6 on Windows 64-bit,
* Intel compiler ICC 17 on GNU/Linux 64-bit (tested only occasionally).

The only mandatory dependency is the `GMP <http://www.gmplib.org>`__ library (GMP 5 and later versions are supported).
The `MPIR <http://mpir.org/>`__ fork of GMP can also be used as a drop-in replacement for GMP.
In order to interoperate with the ``long double`` C++ type, mp++ optionally depends
on the `GNU MPFR <http://www.mpfr.org>`__ multiprecision floating-point library (see the
:ref:`configuration <index_configuration>` section below). In order to compile the benchmark suite, the
`Boost <http://www.boost.org/>`__ C++ libraries are required.

In order to use mp++, you only have to include the ``mp++.hpp`` header in your project and to link to the GMP
library when compiling. E.g., on a Unix-like operating system with GCC you would compile your executable as:

.. code-block:: console

   $ g++ -std=c++11 my_executable.cpp -lgmp

.. _index_configuration:

Configuration
-------------

The library has a few configuration switches in the form of preprocessor definitions. These can be set
on the compiler command-line or in the source code **before** including the headers. The switches are:

* ``MPPP_WITH_LONG_DOUBLE``: if defined, mp++ will be able to interoperate with the ``long double`` C++ type (whereas
  normally floating-point interoperability is limited to the ``float`` and ``double`` types). This option requires
  the `GNU MPFR <http://www.mpfr.org>`__ multiprecision floating-point library (MPFR 3 and later versions are
  supported).
* ``MPPP_CUSTOM_NAMESPACE``: by default, all classes and functions are located in the ``mppp`` namespace. If
  ``MPPP_CUSTOM_NAMESPACE`` is defined, the classes and functions will be located in the namespace specified by the
  value of the definition.

.. .. doxygenclass:: mppp::mp_integer
..    :members:

Contents:

.. toctree::
   :maxdepth: 2



Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
