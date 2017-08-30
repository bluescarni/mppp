Quadruple-precision floating-point
==================================

.. note::

   The functionality described in this page is available only if mp++ was configured
   with the ``MPPP_WITH_QUADMATH`` option enabled (see the :ref:`installation instructions <installation>`).

.. versionadded:: 0.5

*#include <mp++/real128.hpp>*

The ``real128`` class
---------------------

.. doxygenclass:: mppp::real128
   :members:

Types
-----

.. cpp:type:: __float128

   A quadruple-precision floating-point type available in the GCC compiler on most contemporary platforms.
   This is the type wrapped by the :cpp:class:`~mppp::real128` class.

   .. seealso::

      https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html

Concepts
--------

.. cpp:concept:: template <typename T, typename U> mppp::Real128CppOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <real128_operators>`
   involving :cpp:class:`~mppp::real128` and C++ types. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::real128`, or
   * one type is :cpp:class:`~mppp::real128` and the other is a :cpp:concept:`~mppp::CppInteroperable` type.

.. cpp:concept:: template <typename T, typename U> mppp::Real128MpppOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <real128_operators>`
   involving :cpp:class:`~mppp::real128` and mp++ types. Specifically, the concept will be ``true`` if
   one type is :cpp:class:`~mppp::real128` and the other is either :cpp:class:`~mppp::integer` or
   :cpp:class:`~mppp::rational`.

.. _real128_functions:

Functions
---------

.. _real128_arithmetic:

Arithmetic
~~~~~~~~~~

.. doxygengroup:: real128_arithmetic
   :content-only:

.. _real128_comparison:

Comparison
~~~~~~~~~~

.. doxygengroup:: real128_comparison
   :content-only:

.. _real128_roots:

Roots
~~~~~

.. doxygengroup:: real128_roots
   :content-only:

.. _real128_io:

Input/Output
~~~~~~~~~~~~

.. doxygengroup:: real128_io
   :content-only:

.. _real128_operators:

Operators
~~~~~~~~~

.. doxygengroup:: real128_operators
   :content-only:

.. _real128_constants:

Constants
~~~~~~~~~

.. note::

   The implementation of these constants currently assumes that the radix of the C++ ``double`` type is 2,
   and that ``double`` can represent exactly negative powers of two up to at least :math:`2^{-112}`.
   This is the case on essentially all modern architectures, but as an alternative to these functions
   it is possible to use the macros defined by the quadmath library (which, however, might require some
   special compiler switches to be usable).

.. seealso::

   https://gcc.gnu.org/onlinedocs/libquadmath/Typedef-and-constants.html#Typedef-and-constants

.. doxygengroup:: real128_constants
   :content-only:
