.. _benchmarks:

Benchmarks
==========

This section contains various benchmarks against GMP and other multiprecision libraries. All benchmarks
were run on an Intel Core i7-6700K Skylake in a 64bit GNU/Linux environment, using the GCC compiler.
The `Nonius <https://nonius.io/>`__ micro-benchmarking framework was used to take the measurements.
The benchmarking code is available `here <https://github.com/bluescarni/mppp/tree/master/benchmark>`__.

Half-limb multiplication
------------------------

This benchmark measures the time needed to compute the element-wise multiplication of two vectors
of 100 integers.

.. figure:: _static/plot.svg
   :width: 100%
