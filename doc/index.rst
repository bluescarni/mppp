.. mp++ documentation master file, created by
   sphinx-quickstart on Fri Dec 23 14:58:38 2016.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to mp++'s documentation!
================================

mp++ is a C++11/14/17 library for multiprecision arithmetic, currently
featuring:

* arbitrary-precision integers,
* arbitrary-precision rationals,
* arbitrary-precision floats,
* quadruple-precision floats.

Design goals include:

* excellent performance for small integer and rational
  operands,
* easy embeddability in computer algebra systems and
  generic C++ libraries,
* a large collection of arbitrary-precision special functions.

Design non-goals include:

* support for fixed-size wide integrals (i.e., no ``uint512_t``,
  try `Boost Multiprecision <https://www.boost.org/doc/libs/1_72_0/libs/multiprecision/doc/html/index.html>`__
  instead).

Based on well-known libraries such as `GMP <https://gmplib.org/>`__, `MPFR <https://www.mpfr.org>`__ and others,
mp++ was initially conceived as a `GMP <https://gmplib.org/>`__ wrapper with a special focus on performance with
small operands. In particular, a small buffer optimisation and custom
implementations of basic mathematical primitives are instrumental in
achieving a performance increase, with respect to GMP and other integer
multiprecision libraries, which can be
substantial (see the :ref:`benchmarks <benchmarks>` section).

Eventually, a multiprecision rational class and two multiprecision
floating-point classes were added, and today a secondary objective
of mp++ is to provide a modern, consistent and unified C++ interface
to several lower-level multiprecision libraries.

mp++ is a spinoff of the `Piranha <https://github.com/bluescarni/piranha>`__ library, released under the
`MPL2 <https://www.mozilla.org/en-US/MPL/2.0/FAQ/>`__ license.

If you are using mp++ as part of your research, teaching, or other
activities, we would be grateful if you could star
the repository and/or cite our work. The DOI of the latest version
and other citation resources are available
at `this link <https://doi.org/10.5281/zenodo.1043579>`__.

.. warning::

   mp++ is under active development, and, although we try not to break the API
   gratuitously, backwards-incompatible changes do happen from time to time.
   The API will be stabilised with the upcoming release of version 1.0.

.. note::

   This documentation refers to the latest development version of mp++, and it may describe features
   not yet available in mp++'s stable releases.

.. toctree::
   :maxdepth: 2

   installation.rst
   tutorial.rst
   reference.rst
   benchmarks.rst
   changelog.rst
