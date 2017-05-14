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

   This concept is satisfied if ``T`` is a :cpp:concept:`~mppp::RationalInteroperable` and it is not
   a floating-point type.
