.. _benchmarks:

Benchmarks
==========

This section contains various benchmarks comparing mp++ to other multiprecision libraries. All benchmarks
were run on an AMD Ryzen 1700 in a 64-bit GNU/Linux environment, using the GCC compiler.
The benchmarking code is available `here <https://github.com/bluescarni/mppp/tree/master/benchmark>`__.

In addition to mp++, the following libraries are used in the benchmarks:

* the `Boost.Multiprecision <http://www.boost.org/doc/libs/1_63_0/libs/multiprecision/doc/html/index.html>`__ library.
  Specifically, the ``cpp_int`` and the ``mpz_int`` integer classes are employed in the benchmarks. The former
  is a multiprecision integer class adopting a small-value optimisation, the latter is a thin wrapper around the GMP
  ``mpz_t`` type. In the benchmarks, the ``mpz_int`` class is used only for ease of initialisation
  and destruction of GMP objects: all arithmetic operations are implemented by calling directly the GMP API;
* the `FLINT <http://flintlib.org/>`__ library. This library provides a data type called ``fmpz_t`` which, similarly to
  mp++, provides a small-value optimisation on top of GMP.

The benchmark results were last updated on **20170909**, using the following package versions:

* GCC 7.2,
* mp++ 0.5,
* GMP 6.1.2,
* MPFR 3.1.5,
* Boost 1.63.0,
* FLINT 2.5.2.

Integer benchmarks
------------------

.. _integer_dot_product:

Dot product
^^^^^^^^^^^

This benchmark measures the time needed to compute the dot product of two integer vectors of size
:math:`3\times 10^7` containing randomly-generated values. For each benchmarked library, the timings
are split into three bars:

* the ``init`` bar, which accounts for the time needed to initialise the vectors of integers,
* the ``arithmetic`` bar, which represents the time needed to perform the dot product,
* the ``total`` bar, which represents the total runtime of the benchmark.

The multiply-add primitives of each library are used for the accumulation of the dot product. The benchmark
is run in a variety of different setups.

.. _integer1_dot_product_unsigned:

1-limb unsigned integers
........................

In this setup, the integer vectors are initialised with small *non-negative* values, and the final dot product
is less than :math:`2^{64}`. mp++ integers with 1 limb of static size are employed.

.. figure:: _static/integer1_dot_product_unsigned.png
   :width: 100%
   :align: center

It can be immediately seen how the initialisation cost for the ``mpz_int`` class is much higher than for the other
integer types. This is due to the fact that the GMP API always uses dynamically-allocated memory, even for small values.
The other integer types all employ a small-value optimisation, and thus avoid the performance cost of heap allocation.

In this particular benchmark, mp++ is about 3 times faster than GMP (as measured via the ``mpz_int`` wrapper)
in the arithmetic portion of the benchmark. mp++ is also faster than ``cpp_int`` and FLINT, albeit by a smaller margin.

1-limb signed integers
......................

This setup is almost identical to the :ref:`previous one <integer1_dot_product_unsigned>`, but this time the vectors
are initialised with both positive *and* negative values.

.. figure:: _static/integer1_dot_product_signed.png
   :width: 100%
   :align: center

The presence of both positive and negative values has a noticeable performance impact with respect to the previous test
for all libraries, both during the initialisation of the vectors and in the arithmetic part of the benchmark.
This is due to the fact that sign handling in multiprecision computations is typically implemented
with branches, and when positive and negative values are equally likely the effectiveness of the CPU's branch predictor
is much reduced.

The performance advantage of mp++ with respect to the other libraries is retained, albeit by a smaller margin.

.. _integer2_dot_product_unsigned:

2-limb unsigned integers
........................

This setup is the 2-limb version of the :ref:`1-limb unsigned benchmark <integer1_dot_product_unsigned>`:
the vectors are initialised with larger non-negative values, the final result is less than :math:`2^{128}`, and
mp++ integers with 2 limbs of static size are now employed.

.. figure:: _static/integer2_dot_product_unsigned.png
   :width: 100%
   :align: center

The benchmark shows that mp++'s specialised arithmetic functions pay off in terms of raw performance in this scenario.

2-limb signed integers
......................

This setup is the signed version of the :ref:`previous benchmark <integer2_dot_product_unsigned>`.

.. figure:: _static/integer2_dot_product_signed.png
   :width: 100%
   :align: center

As explained earlier, arithmetic with mixed positive and negative values is more expensive than arithmetic with only
non-negative values.

Vector multiplication
^^^^^^^^^^^^^^^^^^^^^

This benchmark is very similar to the :ref:`dot product benchmark <integer_dot_product>`, with one crucial difference:
instead of accumulating the dot product of two randomly-generated vectors of size :math:`3\times 10^7` into a scalar
value, the element-wise product of the two vectors is stored in a third vector, and the final
dot product is computed as the sum of the values in this third vector.

This allows to measure the efficiency
of the multiplication and addition operations (whereas in the dot product benchmark the multiply-add primitives were
employed), and it also increases the pressure on the memory subsystem (due to the need to write the elements' products
into a vector rather than accumulating them directly into a scalar).

.. _integer1_vec_mul_unsigned:

1-limb unsigned integers
........................

In this setup, the integer vectors are initialised with small *non-negative* values, and the final result
is less than :math:`2^{64}`. mp++ integers with 1 limb of static size are employed.

.. figure:: _static/integer1_vec_mul_unsigned.png
   :width: 100%
   :align: center

This time mp++ is more than 5 times faster than GMP in the arithmetic portion of the benchmark, while still maintaining
a performance advantage over ``cpp_int`` and FLINT. The performance of ``cpp_int`` might be hurt by its relatively large
memory footprint (32 bytes vs mp++'s 16 and FLINT's 8).

1-limb signed integers
........................

In this setup, the vectors are initialised with both positive *and* negative values.

.. figure:: _static/integer1_vec_mul_signed.png
   :width: 100%
   :align: center

We can see again how the introduction of mixed positive and negative values impacts performance negatively with respect
to the :ref:`unsigned setup <integer1_vec_mul_unsigned>`.

.. _integer2_vec_mul_unsigned:

2-limb unsigned integers
........................

This setup is the 2-limb version of the :ref:`1-limb unsigned benchmark <integer1_vec_mul_unsigned>`:
the vectors are initialised with larger non-negative values, the final result is less than :math:`2^{128}`, and
mp++ integers with 2 limbs of static size are now employed.

.. figure:: _static/integer2_vec_mul_unsigned.png
   :width: 100%
   :align: center

The benchmark shows again that mp++'s specialised arithmetic functions deliver strong performance.

2-limb signed integers
........................

This setup is the signed version of the :ref:`previous benchmark <integer2_vec_mul_unsigned>`.

.. figure:: _static/integer2_vec_mul_signed.png
   :width: 100%
   :align: center

Sorting
^^^^^^^

This benchmark consists of the sorting (via ``std::sort()``) of a randomly-generated vector of :math:`3\times 10^7` integers.

1-limb unsigned integers
........................

In this setup, the integer vector is initialised with small *non-negative* values. mp++ integers with 1 limb of static size are employed.

.. figure:: _static/integer1_sort_unsigned.png
   :width: 100%
   :align: center

Here again it can be seen how the small-value optimisation implemented in mp++, ``cpp_int`` and FLINT pays off on large
datasets with respect to plain GMP integers. mp++ shows a modest performance increase with respect to ``cpp_int``
and FLINT.

1-limb signed integers
......................

In this setup, the vector is initialised with both positive *and* negative values.

.. figure:: _static/integer1_sort_signed.png
   :width: 100%
   :align: center

2-limb unsigned integers
........................

In this setup, the integer vector is initialised with *non-negative* values in the :math:`\left[2^{64},2^{128}\right)` range.
mp++ integers with 2 limbs of static size are employed.

.. figure:: _static/integer2_sort_unsigned.png
   :width: 100%
   :align: center

2-limb signed integers
......................

In this setup, the vector is initialised with both positive *and* negative values in the :math:`\left(-2^{128},2^{128}\right)` range.
mp++ integers with 2 limbs of static size are employed.

.. figure:: _static/integer2_sort_signed.png
   :width: 100%
   :align: center
