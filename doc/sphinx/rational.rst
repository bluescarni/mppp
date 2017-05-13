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

.. cpp:concept:: template <typename T, typename U, std::size_t SSize> mppp::RationalNumDenCtorTypes

   This concept is satisfied if the types ``T`` and ``U`` can be used to construct, respectively,
   the numerator and denominator in the two-argument constructor of a :cpp:class:`~mppp::rational`
   with static size ``SSize``. Specifically, this concept will be ``true`` if ``T`` is either an
   :cpp:class:`~mppp::integer` with static size ``SSize`` or an integral
   :cpp:concept:`CppInteroperable` type, and if the same holds for ``U``.
