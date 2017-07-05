Multiprecision reals
====================

.. versionadded:: 0.4

*#include <mp++/real.hpp>*

The ``real`` class
----------------------

.. doxygenclass:: mppp::real
   :members:

Types
-----

.. cpp:type:: slong = implementation_defined

.. cpp:type:: ulong = implementation_defined

Concepts
--------

.. cpp:concept:: template <typename T> mppp::RealInteroperable

   This concept is satisfied if the type ``T`` can interoperate with a :cpp:class:`~mppp::rational`
   with static size ``SSize``. Specifically, this concept will be ``true`` if either:

   * ``T`` is :cpp:concept:`CppInteroperable`, or
   * ``T`` is an :cpp:class:`~mppp::integer` with static size ``SSize``.

.. _real_functions:

Functions
---------

.. _real_io:

Input/Output
~~~~~~~~~~~~

.. doxygengroup:: real_io
   :content-only:
