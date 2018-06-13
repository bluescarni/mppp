.. _integer_reference:

Multiprecision integers
=======================

*#include <mp++/integer.hpp>*

The ``integer`` class
---------------------

.. doxygenclass:: mppp::integer
   :members:

Types
-----

.. cpp:type:: mp_limb_t

   This type is defined by the GMP library. It is used to represents a limb, that is,
   the part of a multiprecision integer that fits in a single machine word. This is an
   unsigned integral type, typically 64 or 32 bits wide.

   .. seealso::

      https://gmplib.org/manual/Nomenclature-and-Types.html#Nomenclature-and-Types

.. cpp:type:: mp_bitcnt_t

   This type is defined by the GMP library. It is an unsigned integral type used to count bits in a multiprecision
   number.

   .. seealso::

      https://gmplib.org/manual/Nomenclature-and-Types.html#Nomenclature-and-Types

.. cpp:enum-class:: mppp::integer_bitcnt_t : mp_bitcnt_t

   A strongly-typed counterpart to :cpp:type:`mp_bitcnt_t`, used in the constructor of :cpp:class:`~mppp::integer`
   from number of bits.

Concepts
--------

.. cpp:concept:: template <typename T, typename U> mppp::IntegerOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <integer_operators>` and :ref:`functions <integer_functions>`
   involving :cpp:class:`~mppp::integer`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::integer` with the same static size ``SSize``, or
   * one type is an :cpp:class:`~mppp::integer` and the other is a :cpp:concept:`~mppp::CppInteroperable` type.

   Note that the modulo, bit-shifting and bitwise logic operators have additional restrictions.

   A corresponding boolean type trait called ``are_integer_op_types`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, typename U> mppp::IntegerIntegralOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <integer_operators>` and :ref:`functions <integer_functions>`
   involving :cpp:class:`~mppp::integer` and C++ integral types. Specifically, the concept will be ``true``
   if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::integer` with the same static size, or
   * one type is an :cpp:class:`~mppp::integer` and the other is a :cpp:concept:`~mppp::CppIntegralInteroperable` type.

   A corresponding boolean type trait called ``are_integer_integral_op_types`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::IntegerBinarySaveDest

   This concept is satisfied if ``T`` is a type into which the serialised binary representation of an
   :cpp:class:`~mppp::integer` with static size ``SSize`` can be written. In other words, the concept is satisfied if
   an object of type ``T`` can be passed as an argument to one of the :cpp:func:`mppp::integer::binary_save()` overloads.

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::IntegerBinaryLoadSrc

   This concept is satisfied if ``T`` is a type from which the serialised binary representation of an
   :cpp:class:`~mppp::integer` with static size ``SSize`` can be loaded. In other words, the concept is satisfied if
   an object of type ``T`` can be passed as an argument to one of the :cpp:func:`mppp::integer::binary_load()` overloads.

.. _integer_functions:

Functions
---------

Much of the functionality of the :cpp:class:`~mppp::integer` class is exposed via plain functions. These functions
mimic the `GMP API <https://gmplib.org/manual/Integer-Functions.html>`__ where appropriate, but a variety of
convenience/generic overloads is provided as well.

.. _integer_assignment:

Assignment
~~~~~~~~~~

.. doxygengroup:: integer_assignment
   :content-only:

.. _integer_conversion:

Conversion
~~~~~~~~~~

.. doxygengroup:: integer_conversion
   :content-only:

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

.. _integer_logic:

Logic and bit fiddling
~~~~~~~~~~~~~~~~~~~~~~

.. versionadded:: 0.6

.. doxygengroup:: integer_logic
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

.. _integer_s11n:

Serialisation
~~~~~~~~~~~~~

.. versionadded:: 0.7

.. doxygengroup:: integer_s11n
   :content-only:

.. _integer_other:

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

.. _integer_std_specialisations:

Standard library specialisations
--------------------------------

.. cpp:class:: template <size_t SSize> std::hash<mppp::integer<SSize>>

   Specialisation of ``std::hash`` for :cpp:class:`mppp::integer`.

   .. cpp:type:: public argument_type = mppp::integer<SSize>
   .. cpp:type:: public result_type = size_t

   .. note::

      The :cpp:type:`argument_type` and :cpp:type:`result_type` type aliases are defined only until C++14.

   .. cpp:function:: public size_t operator()(const mppp::integer<SSize> &n) const

      :param n: the input :cpp:class:`mppp::integer`.

      :return: a hash value for *n*.
