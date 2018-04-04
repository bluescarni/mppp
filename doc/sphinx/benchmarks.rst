.. _benchmarks:

Benchmarks
==========

This section contains various benchmarks comparing mp++ to other multiprecision libraries. All benchmarks
were run on an AMD Ryzen 1700 in a 64-bit GNU/Linux environment, using the GCC compiler.
The benchmarking code is available `here <https://github.com/bluescarni/mppp/tree/master/benchmark>`__.

In addition to mp++, the following libraries are used in the benchmarks:

* the `Boost.Multiprecision <https://www.boost.org/doc/libs/1_66_0/libs/multiprecision/doc/html/index.html>`__ library.
  Specifically, the ``cpp_int`` and the ``mpz_int`` integer classes are employed in the benchmarks. The former
  is a multiprecision integer class adopting a small-value optimisation, the latter is a thin wrapper around the GMP
  ``mpz_t`` type. In the benchmarks, the ``mpz_int`` class is used only for ease of initialisation
  and destruction of GMP objects: all arithmetic operations are implemented by calling directly the GMP API;
* the `FLINT <http://flintlib.org/>`__ library. This library provides a data type called ``fmpz_t`` which, similarly to
  mp++, provides a small-value optimisation on top of GMP.

The benchmark results were last updated on **20180214**, using the following package versions:

* GCC 7.3,
* mp++ 0.9,
* GMP 6.1.2,
* MPFR 3.1.5,
* Boost 1.65.0,
* FLINT 2.5.2.

.. toctree::
   :maxdepth: 2

   integer_benchmarks.rst
