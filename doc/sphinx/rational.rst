Multiprecision rationals
========================


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
