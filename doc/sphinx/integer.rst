Arbitrary-precision integers
============================


The ``integer`` class
------------------------

.. doxygenclass:: mppp::integer
   :members:

Concepts
--------

.. cpp:concept:: template <typename T, typename U> mppp::IntegerBinaryOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   :ref:`overloaded binary operators <integer_operators>` for :cpp:class:`~mppp::integer`. Specifically,
   the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::integer` with the same static size ``SSize``, or
   * one type is :cpp:class:`~mppp::integer` and the other is a :cpp:concept:`~mppp::CppInteroperable` type.

   Note that the :cpp:concept:`modulo <mppp::IntegerModType>` and :cpp:concept:`bit-shifting <mppp::IntegerShiftType>`
   operators have additional restrictions.

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::IntegerOpType

   This concept is satisfied if ``T`` is a type with which an :cpp:class:`~mppp::integer` with static size ``SSize``
   can interact via its :ref:`overloaded operators <integer_operators>`. Specifically, this concept is satisfied by:

   * all :cpp:concept:`~mppp::CppInteroperable` types,
   * :cpp:class:`~mppp::integer` with static size ``SSize``.

   Note that the :cpp:concept:`modulo <mppp::IntegerModType>` and :cpp:concept:`bit-shifting <mppp::IntegerShiftType>`
   operators have additional restrictions.

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::IntegerModType

   This concept is satisfied if ``T`` is a type with which an :cpp:class:`~mppp::integer` with static size ``SSize``
   can interact via its modulo operators. Specifically, this concept is satisfied by:

   * all :cpp:concept:`~mppp::CppInteroperable` integral types,
   * :cpp:class:`~mppp::integer` with static size ``SSize``.

.. cpp:concept:: template <typename T> mppp::IntegerShiftType

   This concept is satisfied if ``T`` is a type that can be used as shift argument in the bit shifting operators for
   :cpp:class:`~mppp::integer`. Specifically, this concept is satisfied by all :cpp:concept:`~mppp::CppInteroperable` integral types.

.. _integer_functions:

Functions
---------

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

.. _integer_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. doxygengroup:: integer_exponentiation
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

.. doxygengroup:: integer_operators
   :content-only:
