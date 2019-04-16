Multiprecision floats
=====================

.. note::

   The functionality described in this section is available only if mp++ was configured
   with the ``MPPP_WITH_MPFR`` option enabled (see the :ref:`installation instructions <installation>`).

.. versionadded:: 0.5

*#include <mp++/real.hpp>*

The ``real`` class
------------------

.. doxygenclass:: mppp::real
   :members:

Types
-----

.. cpp:type:: mppp::mpfr_struct_t = std::remove_extent<mpfr_t>::type

   The C structure used by MPFR to represent arbitrary-precision floats.
   The MPFR type ``mpfr_t`` is defined as an array of size 1 of this structure.

.. cpp:type:: mpfr_prec_t

   An integral type defined by the MPFR library, used to represent the precision of ``mpfr_t``
   and (by extension) :cpp:class:`~mppp::real` objects.

.. cpp:type:: mpfr_exp_t

   An integral type defined by the MPFR library, used to represent the exponent of ``mpfr_t``
   and (by extension) :cpp:class:`~mppp::real` objects.

.. cpp:enum-class:: mppp::real_kind

   This scoped enum is used to initialise a :cpp:class:`~mppp::real` with
   one of the three special values NaN, infinity or zero.

   .. cpp:enumerator:: nan = MPFR_NAN_KIND
   .. cpp:enumerator:: inf = MPFR_INF_KIND
   .. cpp:enumerator:: zero = MPFR_ZERO_KIND

.. seealso::

   https://www.mpfr.org/mpfr-current/mpfr.html#Nomenclature-and-Types

Concepts
--------

.. cpp:concept:: template <typename T> mppp::RealInteroperable

   This concept is satisfied if the type ``T`` can interoperate with :cpp:class:`~mppp::real`.
   Specifically, this concept will be ``true`` if ``T`` is either:

   * a :cpp:concept:`CppInteroperable` type, or
   * an :cpp:class:`~mppp::integer`, or
   * a :cpp:class:`~mppp::rational`, or
   * :cpp:class:`~mppp::real128`.

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

   A corresponding boolean type trait called ``are_real_op_types`` is also available (even if the compiler does
   not support concepts).

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

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::sqrt(mppp::real &rop, T &&op)

   Binary :cpp:class:`~mppp::real` square root.

   This function will compute the square root of *op* and store it
   into *rop*. The precision of the result will be equal to the precision
   of *op*.

   If *op* is -0, *rop* will be set to -0. If *op* is negative, *rop* will be set to NaN.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::sqrt(T &&r)

   Unary :cpp:class:`~mppp::real` square root.

   This function will compute and return the square root of *r*.
   The precision of the result will be equal to the precision of *r*.

   If *r* is -0, the result will be -0. If *r* is negative, the result will be NaN.

   :param r: the operand.

   :return: the square root of *r*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::rec_sqrt(mppp::real &rop, T &&op)

   .. versionadded:: 0.12

   Binary :cpp:class:`~mppp::real` reciprocal square root.

   This function will compute the reciprocal square root of *op* and store it into *rop*. The precision
   of the result will be equal to the precision of *op*.

   If *op* is zero, *rop* will be set to a positive infinity (regardless of the sign of *op*).
   If *op* is a positive infinity, *rop* will be set to +0. If *op* is negative, *rop* will be set to NaN.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::rec_sqrt(T &&r)

   .. versionadded:: 0.12

   Unary :cpp:class:`~mppp::real` reciprocal square root.

   This function will compute and return the reciprocal square root of *r*.
   The precision of the result will be equal to the precision of *r*.

   If *r* is zero, a positive infinity will be returned (regardless of the sign of *r*).
   If *r* is a positive infinity, +0 will be returned. If *r* is negative,
   NaN will be returned.

   :param r: the operand.

   :return: the reciprocal square root of *r*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::cbrt(mppp::real &rop, T &&op)

   .. versionadded:: 0.12

   Binary :cpp:class:`~mppp::real` cubic root.

   This function will compute the cubic root of *op* and store it
   into *rop*. The precision of the result will be equal to the precision
   of *op*.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::cbrt(T &&r)

   .. versionadded:: 0.12

   Unary :cpp:class:`~mppp::real` cubic root.

   This function will compute and return the cubic root of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the operand.

   :return: the cubic root of *r*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::rootn_ui(mppp::real &rop, T &&op, unsigned long k)

   .. versionadded:: 0.12

   Binary :cpp:class:`~mppp::real` k-th root.

   This function will compute the k-th root of *op* and store it
   into *rop*. The precision of the result will be equal to the precision
   of *op*.

   If *k* is zero, the result will be NaN. If *k* is odd (resp. even) and *op*
   negative (including negative infinity), the result will be a negative number (resp. NaN).
   If *op* is zero, the result will be zero with the sign obtained by the usual limit rules, i.e.,
   the same sign as *op* if *k* is odd, and positive if *k* is even.

   .. note::
      This function is available from MPFR 4 onwards.

   :param rop: the return value.
   :param op: the operand.
   :param k: the degree of the root.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::rootn_ui(T &&r, unsigned long k)

   .. versionadded:: 0.12

   Unary :cpp:class:`~mppp::real` k-th root.

   This function will compute and return the k-th root of *r*.
   The precision of the result will be equal to the precision
   of *r*.

   If *k* is zero, the result will be NaN. If *k* is odd (resp. even) and *r*
   negative (including negative infinity), the result will be a negative number (resp. NaN).
   If *r* is zero, the result will be zero with the sign obtained by the usual limit rules, i.e.,
   the same sign as *r* if *k* is odd, and positive if *k* is even.

   .. note::
      This function is available from MPFR 4 onwards.

   :param r: the operand.
   :param k: the degree of the root.

   :return: the k-th root of *r*.

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

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::exp(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::exp2(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::exp10(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::expm1(mppp::real &rop, T &&op)

   Binary exponentials.

   These functions will set *rop* to, respectively,

   * ``e**op``,
   * ``2**op``,
   * ``10**op``,
   * ``e**op-1``.

   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the exponent.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real exp(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real exp2(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real exp10(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real expm1(T &&r)

   Unary exponentials.

   These functions will return, respectively,

   * ``e**r``,
   * ``2**r``,
   * ``10**r``,
   * ``e**r-1``.

   The precision of the result will be equal to the precision of *r*.

   :param r: the exponent.

   :return: the exponential of *r*.

.. _real_gamma:

Gamma functions
~~~~~~~~~~~~~~~

.. doxygengroup:: real_gamma
   :content-only:

.. _real_intrem:

Integer and remainder related functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygengroup:: real_intrem
   :content-only:

.. _real_io:

Input/Output
~~~~~~~~~~~~

.. doxygengroup:: real_io
   :content-only:

.. _real_operators:

Mathematical operators
----------------------

.. doxygengroup:: real_operators
   :content-only:

.. _real_constants:

Constants
---------

.. doxygengroup:: real_constants
   :content-only:
