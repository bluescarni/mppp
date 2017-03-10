.. _benchmarks:

Benchmarks
==========

This section contains various benchmarks comparing mp++ to other multiprecision libraries. All benchmarks
were run on an Intel Core i7-6700K Skylake in a 64bit GNU/Linux environment, using the GCC compiler.
The benchmarking code is available `here <https://github.com/bluescarni/mppp/tree/master/benchmark>`__.

In addition to mp++, the following libraries are used in the benchmarks:

* the `Boost.Multiprecision <http://www.boost.org/doc/libs/1_63_0/libs/multiprecision/doc/html/index.html>`__ library.
  Specifically, the ``cpp_int`` and the ``mpz_int`` integer classes are employed in the benchmarks. The former
  is a multiprecision integer class adopting a small-value optimisation, the latter is a thin wrapper around the GMP
  ``mpz_t`` type. In the benchmarks, the ``mpz_int`` class is used only for ease of initialisation
  and destruction of GMP objects: all arithmetic operations are implemented by calling directly the GMP API;
* the `FLINT <http://flintlib.org/>`__ library. This library provides a data type called ``fmpz_t`` which, similarly to
  mp++, provides a small-value optimisation on top of GMP.

Dot product
-----------

This benchmark measures the time needed to compute the dot product of two integer vectors of size
:math:`3\times 10^7`. The final result fits in a 64bit value. For each benchmarked library, the timings
are split into three bars:

* the ``init`` bar, which accounts for the time needed to initialise the vectors of integers,
* the ``arithmetic`` bar, which represents the time needed to perform the dot product,
* the ``total`` bar, which represents the total runtime of the benchmark.

The mp++ timings have been taken using a single 64bit limb for the small-value optimisation. The
multiply-add primitives of each library are used for the accumulation of the dot product.

.. figure:: _static/bench_dot_product_1.svg
   :width: 100%
   :align: center

It can be immediately seen how the initialisation cost for the ``mpz_int`` class is much higher than for the other
integer types. This is due to the fact that the GMP API always uses dynamically-allocated memory, even for small values.
The other integer types all employ a small-value optimisation, and thus avoid the performance cost of heap allocation.

In this particular benchmark, mp++ is about 3 times faster than GMP (as measured via the ``mpz_int`` wrapper)
in the arithmetic portion of the benchmark. mp++ is also faster than ``cpp_int`` and FLINT, albeit by a smaller margin.

Vector multiplication
---------------------

This benchmark is very similar to the dot product benchmark above, with one crucial difference: instead of accumulating
the dot product into a scalar value, the element-wise product of the two vectors is stored in a third vector, and the final
dot product is computed as the sum of the values in this third vector.

This allows to measure the efficiency
of the multiplication and addition operations (whereas in the previous benchmark the multiply-add primitives were
employed), and it also increases the pressure on the memory subsystem (due to the need to write the elements' products
into a vector rather than accumulating them directly into a scalar).

.. figure:: _static/bench_vec_mul_1.svg
   :width: 100%
   :align: center

This time mp++ is more than 5 times faster than GMP in the arithmetic portion of the benchmark, while still maintaining
a performance advantage over ``cpp_int`` and FLINT.
