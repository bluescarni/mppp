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

The combination of these two techniques results in a performance increase, on small operands,
with respect to GMP. The price to pay is a small overhead when operating on large operands.

mp++ was created to cater to the requirements of computer algebra systems, which typically need to be able
to manipulate arbitrarily-large integers but which, in practice, often end up storing many small integers
(e.g., as coefficients in a polynomial).

Contents:
---------

.. toctree::
   :maxdepth: 2

   installation.rst
   configuration.rst
   reference.rst
   benchmarks.rst

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
