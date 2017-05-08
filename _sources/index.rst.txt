.. mp++ documentation master file, created by
   sphinx-quickstart on Fri Dec 23 14:58:38 2016.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to mp++'s documentation!
================================

mp++ is a small C++11 header-only library for multiprecision integer arithmetic. Based
on the well-known `GMP <http://www.gmplib.org>`__ library, mp++ places a strong emphasis on
optimising operations on small values. When dealing with small operands, mp++ will:

* avoid heap memory allocations as much as possible, and
* use optimised implementations of basic operations (instead of calling GMP functions).

The combination of these two techniques results in a performance increase, on small operands,
with respect to GMP (see the :ref:`benchmark <benchmarks>` section). The price to pay is a
small overhead when operating on large operands.

mp++ was created to cater to the requirements of computer algebra systems, which typically need to be able
to manipulate arbitrarily-large integers but which, in practice, often end up storing many small integers
(e.g., as coefficients in a polynomial).

mp++ is released under the `MPL2 <https://www.mozilla.org/en-US/MPL/2.0/FAQ/>`__ license.

.. toctree::

   installation.rst
   reference.rst
   benchmarks.rst
   changelog.rst
