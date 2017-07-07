Multiprecision reals
====================

.. note::

   The :cpp:class:`~mppp::real` class and its ancillary functions are available only if mp++ was configured with the
   ``MPPP_WITH_ARB`` option enabled (see the :ref:`installation instructions <installation>`).

.. versionadded:: 0.4

*#include <mp++/real.hpp>*

The ``real`` class
----------------------

.. doxygenclass:: mppp::real
   :members:

Types
-----

.. cpp:type:: slong = implementation_defined

   This type is defined by the Arb library. It is a signed integral type with a bit size of 32 or 64 bits, depending
   on the platform.

.. cpp:type:: ulong = implementation_defined

   This type is defined by the Arb library. It is an unsigned integral type with a bit size of 32 or 64 bits, depending
   on the platform.

.. seealso :: http://arblib.org/issues.html#integer-types

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
