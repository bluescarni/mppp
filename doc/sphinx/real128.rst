Quadruple-precision floats
==========================

.. note::

   The functionality described in this page is available only if mp++ was configured
   with the ``MPPP_WITH_QUADMATH`` option enabled (see the :ref:`installation instructions <installation>`).

.. versionadded:: 0.5

*#include <mp++/real128.hpp>*

The ``real128`` class
---------------------

.. doxygenclass:: mppp::real128
   :members:

Types
-----

.. cpp:type:: __float128

   A quadruple-precision floating-point type available in recent versions of the GCC and clang compilers.
   This is the type wrapped by the :cpp:class:`~mppp::real128` class.

   .. seealso::

      https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html

Concepts
--------

.. cpp:concept:: template <typename T> mppp::Real128CppInteroperable

   This concept is satisfied by fundamental C++ types that can interoperate with :cpp:class:`~mppp::real128`.
   Specifically:

   * on GCC, this concept is satisfied by the types satisfying :cpp:concept:`mppp::CppInteroperable`;
   * on Clang, this concept is satisfied by the types satisfying :cpp:concept:`mppp::CppInteroperable`,
     minus ``long double``.

.. cpp:concept:: template <typename T> mppp::Real128MpppInteroperable

   This concept is satisfied by mp++ types that can interoperate with :cpp:class:`~mppp::real128`.
   Specifically, the concept is satisfied if ``T`` is either :cpp:class:`~mppp::integer` or
   :cpp:class:`~mppp::rational`.

.. cpp:concept:: template <typename T, typename U> mppp::Real128CppOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <real128_operators>`
   involving :cpp:class:`~mppp::real128` and C++ types. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::real128`, or
   * one type is :cpp:class:`~mppp::real128` and the other is a :cpp:concept:`~mppp::Real128CppInteroperable` type.

.. cpp:concept:: template <typename T, typename U> mppp::Real128MpppOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <real128_operators>`
   involving :cpp:class:`~mppp::real128` and mp++ types. Specifically, the concept will be ``true`` if
   one type is :cpp:class:`~mppp::real128` and the other type satisfies :cpp:concept:`~mppp::Real128MpppInteroperable`.

.. cpp:concept:: template <typename T, typename U> mppp::Real128OpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <real128_operators>` and :ref:`functions <real128_functions>`
   involving :cpp:class:`~mppp::real128`. Specifically, the concept will be ``true`` if
   ``T`` and ``U`` satisfy :cpp:concept:`~mppp::Real128CppOpTypes` or :cpp:concept:`~mppp::Real128MpppOpTypes`.

.. _real128_functions:

Functions
---------

.. _real128_conversion:

Conversion
~~~~~~~~~~

.. doxygengroup:: real128_conversion
   :content-only:

.. _real128_arithmetic:

Arithmetic
~~~~~~~~~~

.. doxygengroup:: real128_arithmetic
   :content-only:

.. _real128_comparison:

Comparison
~~~~~~~~~~

.. doxygengroup:: real128_comparison
   :content-only:

.. _real128_roots:

Roots
~~~~~

.. doxygengroup:: real128_roots
   :content-only:

.. _real128_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. doxygengroup:: real128_exponentiation
   :content-only:

.. _real128_trig:

Trigonometry
~~~~~~~~~~~~

.. doxygengroup:: real128_trig
   :content-only:

.. _real128_logexp:

Logarithms and exponentials
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygengroup:: real128_logexp
   :content-only:

.. _real128_io:

Input/Output
~~~~~~~~~~~~

.. doxygengroup:: real128_io
   :content-only:

.. _real128_operators:

Operators
---------

.. doxygengroup:: real128_operators
   :content-only:

.. _real128_constants:

Constants
---------

A few mathematical constants are provided. The constants are available as inline variables
(e.g., :cpp:var:`mppp::pi_128`, requires C++17 or later) and as constexpr functions (e.g., :cpp:func:`mppp::real128_pi()`,
always available). Inline variables and constexpr functions provide exactly the same functionality,
but inline variables are more convenient if C++17 is an option.

.. note::
   Some of these constants are also available as
   `macros <https://gcc.gnu.org/onlinedocs/libquadmath/Typedef-and-constants.html#Typedef-and-constants>`__
   from the quadmath library.

.. doxygengroup:: real128_constants
   :content-only:
