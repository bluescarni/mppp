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

.. doxygengroup:: real128_constants
   :content-only:
