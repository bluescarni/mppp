Multiprecision floats
=====================

.. note::

   The functionality described in this page is available only if mp++ was configured
   with the ``MPPP_WITH_MPFR`` option enabled (see the :ref:`installation instructions <installation>`).

.. versionadded:: 0.5

*#include <mp++/real.hpp>*

The ``real`` class
------------------

.. doxygenclass:: mppp::real
   :members:

Types
-----

.. cpp:type:: mpfr_prec_t

   An integral type defined by the MPFR library, used to represent the precision of ``mpfr_t``
   and (by extension) :cpp:class:`~mppp::real` objects.

   .. seealso::

      http://www.mpfr.org/mpfr-current/mpfr.html#Nomenclature-and-Types

.. doxygentypedef:: mppp::mpfr_struct_t

.. doxygenstruct:: mppp::real_prec
   :members:

Concepts
--------

.. cpp:concept:: template <typename T> mppp::RealInteroperable

   This concept is satisfied if the type ``T`` can interoperate with :cpp:class:`~mppp::real`.
   Specifically, this concept will be ``true`` if:

   * ``T`` is :cpp:concept:`CppInteroperable`, or
   * ``T`` is an :cpp:class:`~mppp::integer`, or
   * ``T`` is a :cpp:class:`~mppp::rational`, or
   * ``T`` is a :cpp:class:`~mppp::real128`.

.. cpp:concept:: template <typename T> mppp::CvrReal

   This concept is satisfied if the type ``T``, after the removal of reference and cv qualifiers,
   is the same as :cpp:class:`mppp::real`.

.. cpp:concept:: template <typename... Args> mppp::RealSetArgs

   This concept is satisfied if the types in the parameter pack ``Args``
   can be used as argument types in one of the :cpp:func:`mppp::real::set()` method overloads.
   In other words, this concept is satisfied if the expression

   .. code-block:: c++

      r.set(x, y, z, ...);

   is valid (where ``r`` is a non-const :cpp:class:`~mppp::real` and ``x``, ``y``, ``z``, etc. are const
   references to the types in ``Args``).

.. cpp:concept:: template <typename T, typename U> mppp::RealOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <real_operators>` and :ref:`functions <real_functions>`
   involving :cpp:class:`~mppp::real`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` both satisfy :cpp:concept:`~mppp::CvrReal`,
   * one type satisfies :cpp:concept:`~mppp::CvrReal` and the other type, after the removal of reference
     and cv qualifiers, satisfies :cpp:concept:`~mppp::RealInteroperable`.

.. cpp:concept:: template <typename T, typename U> mppp::RealCompoundOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic in-place :ref:`operators <real_operators>`
   involving :cpp:class:`~mppp::real`. Specifically, the concept will be ``true`` if
   ``T`` and ``U`` satisfy :cpp:concept:`~mppp::RealOpTypes` and ``T``, after the removal
   of reference, is not const.

.. _real_functions:

Functions
---------

.. _real_prec:

Precision handling
~~~~~~~~~~~~~~~~~~

.. doxygengroup:: real_prec
   :content-only:

.. _real_assignment:

Assignment
~~~~~~~~~~

.. doxygengroup:: real_assignment
   :content-only:

.. _real_conversion:

Conversion
~~~~~~~~~~

.. doxygengroup:: real_conversion
   :content-only:

.. _real_arithmetic:

Arithmetic
~~~~~~~~~~

.. doxygengroup:: real_arithmetic
   :content-only:

.. _real_comparison:

Comparison
~~~~~~~~~~

.. doxygengroup:: real_comparison
   :content-only:

.. _real_roots:

Roots
~~~~~

.. doxygengroup:: real_roots
   :content-only:

.. _real_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. doxygengroup:: real_exponentiation
   :content-only:

.. _real_trig:

Trigonometry
~~~~~~~~~~~~

.. doxygengroup:: real_trig
   :content-only:

.. _real_logexp:

Logarithms and exponentials
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygengroup:: real_logexp
   :content-only:

.. _real_io:

Input/Output
~~~~~~~~~~~~

.. doxygengroup:: real_io
   :content-only:

.. _real_operators:

Operators
---------

.. doxygengroup:: real_operators
   :content-only:

.. _real_constants:

Constants
---------

.. doxygengroup:: real_constants
   :content-only:
