.. _rational_reference:

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
   with static size ``SSize``. Specifically, this concept will be ``true`` if ``T`` satisfies
   :cpp:concept:`~mppp::CppInteroperable` or it is an :cpp:class:`~mppp::integer` with static size ``SSize``.

   A corresponding boolean type trait called ``is_rational_interoperable`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::RationalCvrInteroperable

   This concept is satisfied if the type ``T``, after the removal of reference and cv qualifiers,
   satisfies :cpp:concept:`~mppp::RationalInteroperable`.

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::RationalIntegralInteroperable

   This concept is satisfied if either:

   * ``T`` satisfies :cpp:concept:`~mppp::CppIntegralInteroperable`, or
   * ``T`` is an :cpp:class:`~mppp::integer` with static size ``SSize``.

   A corresponding boolean type trait called ``is_rational_integral_interoperable`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::RationalCvrIntegralInteroperable

   This concept is satisfied if the type ``T``, after the removal of reference and cv qualifiers,
   satisfies :cpp:concept:`~mppp::RationalIntegralInteroperable`.

.. cpp:concept:: template <typename T, typename U> mppp::RationalOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <rational_operators>` and :ref:`functions <rational_functions>`
   involving :cpp:class:`~mppp::rational`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::rational` with the same static size ``SSize``, or
   * one type is a :cpp:class:`~mppp::rational` and the other is a :cpp:concept:`~mppp::RationalInteroperable`
     type.

   A corresponding boolean type trait called ``are_rational_op_types`` is also available (even if the compiler does
   not support concepts).

.. _rational_functions:

Functions
---------

Much of the functionality of the :cpp:class:`~mppp::rational` class is exposed via plain functions. These functions
mimic the `GMP API <https://gmplib.org/manual/Rational-Number-Functions.html>`__ where appropriate, but a variety of
convenience/generic overloads is provided as well.

.. _rational_conversion:

Conversion
~~~~~~~~~~

.. doxygengroup:: rational_conversion
   :content-only:

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

.. _rational_ntheory:

Number theoretic functions
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. versionadded:: 0.8

.. doxygengroup:: rational_ntheory
   :content-only:

.. _rational_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. doxygengroup:: rational_exponentiation
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

.. _rational_std_specialisations:

Standard library specialisations
--------------------------------

.. cpp:class:: template <std::size_t SSize> std::hash<mppp::rational<SSize>>

   Specialisation of ``std::hash`` for :cpp:class:`mppp::rational`.

   .. cpp:type:: public argument_type = mppp::rational<SSize>
   .. cpp:type:: public result_type = std::size_t

   .. note::

      The :cpp:type:`argument_type` and :cpp:type:`result_type` type aliases are defined only until C++14.

   .. cpp:function:: public std::size_t operator()(const mppp::rational<SSize> &q) const

      :param q: the input :cpp:class:`mppp::rational`.

      :return: a hash value for *q*.
