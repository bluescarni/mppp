Multiprecision integers
=======================


The ``integer`` class
---------------------

.. doxygenclass:: mppp::integer
   :members:

Concepts
--------

.. cpp:concept:: template <typename T, typename U> mppp::IntegerOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <integer_operators>` and :ref:`functions <integer_functions>`
   involving :cpp:class:`~mppp::integer`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::integer` with the same static size ``SSize``, or
   * one type is :cpp:class:`~mppp::integer` and the other is a :cpp:concept:`~mppp::CppInteroperable` type.

   Note that the modulo and bit-shifting operators have additional restrictions.

.. cpp:concept:: template <typename T, typename U> mppp::IntegerIntegralOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <integer_operators>` and :ref:`functions <integer_functions>`
   involving :cpp:class:`~mppp::integer` and C++ integral types. Specifically, the concept will be ``true``
   if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::integer` with the same static size ``SSize``, or
   * one type is :cpp:class:`~mppp::integer` and the other is an integral :cpp:concept:`~mppp::CppInteroperable` type.

.. cpp:concept:: template <typename T> mppp::IntegerShiftType

   This concept is satisfied if ``T`` is a type that can be used as shift argument in the bit shifting operators for
   :cpp:class:`~mppp::integer`. Specifically, this concept is satisfied by all :cpp:concept:`~mppp::CppInteroperable` integral types.

.. _integer_functions:

Functions
---------

Much of the functionality of the :cpp:class:`~mppp::integer` class is exposed via plain functions. These functions
mimic the GMP API where appropriate, but a variety of convenience/generic overloads is provided as well.

.. _integer_arithmetic:

Arithmetic
~~~~~~~~~~

.. doxygengroup:: integer_arithmetic
   :content-only:

.. _integer_division:

Division
~~~~~~~~

.. doxygengroup:: integer_division
   :content-only:

.. _integer_comparison:

Comparison
~~~~~~~~~~

.. doxygengroup:: integer_comparison
   :content-only:

.. _integer_ntheory:

Number theoretic functions
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygengroup:: integer_ntheory
   :content-only:

.. _integer_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. doxygengroup:: integer_exponentiation
   :content-only:

.. _integer_roots:

Roots
~~~~~

.. doxygengroup:: integer_roots
   :content-only:

.. _integer_io:

Input/Output
~~~~~~~~~~~~

.. doxygengroup:: integer_io
   :content-only:

Other
~~~~~

.. doxygengroup:: integer_other
   :content-only:

.. _integer_operators:

Operators
---------

Overloaded operators are provided for convenience. Their interface is generic, and their implementation
is typically built on top of basic :ref:`functions <integer_functions>`.

.. doxygengroup:: integer_operators
   :content-only:
