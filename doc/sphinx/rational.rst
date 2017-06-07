Multiprecision rationals
========================

.. versionadded:: 0.3

*#include <mp++/rational.hpp>*

The ``rational`` class
----------------------

.. doxygenclass:: mppp::rational
   :members:

Concepts
--------

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::RationalInteroperable

   This concept is satisfied if the type ``T`` can interoperate with a :cpp:class:`~mppp::rational`
   with static size ``SSize``. Specifically, this concept will be ``true`` if either:

   * ``T`` is :cpp:concept:`CppInteroperable`, or
   * ``T`` is an :cpp:class:`~mppp::integer` with static size ``SSize``.

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::RationalIntegralInteroperable

   This concept is satisfied if ``T`` is a :cpp:concept:`~mppp::RationalInteroperable` type and it is not
   a floating-point type.

.. cpp:concept:: template <typename T, typename U> mppp::RationalOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <rational_operators>` and :ref:`functions <rational_functions>`
   involving :cpp:class:`~mppp::rational`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::rational` with the same static size ``SSize``, or
   * one type is a :cpp:class:`~mppp::rational` and the other is a :cpp:concept:`~mppp::RationalInteroperable`
     type.

.. _rational_functions:

Functions
---------

Much of the functionality of the :cpp:class:`~mppp::rational` class is exposed via plain functions. These functions
mimic the `GMP API <https://gmplib.org/manual/Rational-Number-Functions.html>`_ where appropriate, but a variety of
convenience/generic overloads is provided as well.

.. _rational_arithmetic:

Arithmetic
~~~~~~~~~~

.. doxygengroup:: rational_arithmetic
   :content-only:

.. _rational_comparison:

Comparison
~~~~~~~~~~

.. doxygengroup:: rational_comparison
   :content-only:

.. _rational_io:

Input/Output
~~~~~~~~~~~~

.. doxygengroup:: rational_io
   :content-only:

.. _rational_other:

Other
~~~~~

.. doxygengroup:: rational_other
   :content-only:

.. _rational_operators:

Operators
---------

Overloaded operators are provided for convenience. Their interface is generic, and their implementation
is typically built on top of basic :ref:`functions <rational_functions>`.

.. doxygengroup:: rational_operators
   :content-only:

.. cpp:function:: template <typename T, typename U> bool mppp::operator==(const T &op1, const U &op2)

   Equality operator.

   This operator is enabled only if ``T`` and ``U`` satisfy :cpp:concept:`~mppp::RationalOpTypes`.

   :arg op1: first argument.
   :arg op2: second argument.

   :return: ``true`` if ``op1 == op2``, ``false`` otherwise.

.. cpp:function:: template <typename T, typename U> bool mppp::operator!=(const T &op1, const U &op2)

   Inequality operator.

   This operator is enabled only if ``T`` and ``U`` satisfy :cpp:concept:`~mppp::RationalOpTypes`.

   :arg op1: first argument.
   :arg op2: second argument.

   :return: ``true`` if ``op1 != op2``, ``false`` otherwise.
