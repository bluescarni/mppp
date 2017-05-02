Arbitrary-precision integers
============================


The ``integer`` class
------------------------

.. doxygenclass:: mppp::integer
   :members:

Concepts
--------

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::IntegerOpType

   This concept is satisfied by all types with which an :cpp:class:`~mppp::integer` with static size ``SSize``
   can interact via its :ref:`overloaded operators <integer_operators>`. Specifically, this concept is satisfied by:

   * all :cpp:concept:`~mppp::CppInteroperable` types,
   * :cpp:class:`~mppp::integer` with static size ``SSize``.

   Note that the modulo and bit-shifting operators have additional restrictions.

.. _integer_operators:

Operators
---------

.. doxygengroup:: integer_operators
   :content-only:

.. _integer_functions:

Functions
---------

.. _integer_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. doxygengroup:: integer_exponentiation
   :content-only:

Other
~~~~~

.. doxygengroup:: integer_other
   :content-only:
