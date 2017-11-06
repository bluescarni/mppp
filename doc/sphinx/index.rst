.. mp++ documentation master file, created by
   sphinx-quickstart on Fri Dec 23 14:58:38 2016.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to mp++'s documentation!
================================

mp++ is a C++11 header-only library for multiprecision arithmetic, currently supporting arbitrary-precision integers,
rationals and floats, and quadruple-precision floats.

Based on well-known libraries such as `GMP <http://www.gmplib.org>`__, `MPFR <http://www.mpfr.org>`__, and others,
mp++ was initially conceived as a `GMP <http://www.gmplib.org>`__ wrapper with a special focus on performance with
small operands. In particular, a small buffer optimisation and custom implementations of basic mathematical primitives are
instrumental in achieving a performance increase, with respect to GMP and other integer multiprecision libraries, which can be
substantial (see the :ref:`benchmarks <benchmarks>` section). The price to pay is a small overhead when operating on large integers.

Eventually, a multiprecision rational class and two multiprecision floating-point classes were added, and today a secondary objective
of mp++ is to provide a modern, consistent and unified C++ interface to several lower-level multiprecision libraries.

mp++ is a spinoff of the `Piranha <https://github.com/bluescarni/piranha>`__ library, released under the
`MPL2 <https://www.mozilla.org/en-US/MPL/2.0/FAQ/>`__ license.

.. note::

   This documentation refers to the latest development version of mp++, and it may describe features
   not yet available in mp++'s stable releases.

.. toctree::
   :maxdepth: 2

   installation.rst
   reference.rst
   benchmarks.rst
   changelog.rst
