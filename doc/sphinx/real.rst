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

.. cpp:concept:: template <typename T, typename U> mppp::RealInPlaceOpTypes

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

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::sqrt1pm1(mppp::real &rop, T &&op)

   .. versionadded:: 0.19

   .. note::
      This function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled
      (see the :ref:`installation instructions <installation>`).

   Binary :cpp:class:`~mppp::real` sqrt1pm1.

   This function will compute :math:`\sqrt{1+x}-1`, where :math:`x` is the value of *op*,
   and store the result into *rop*. The precision of the result will be equal to the precision
   of *op*.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::sqrt1pm1(T &&r)

   .. versionadded:: 0.19

   .. note::
      This function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled
      (see the :ref:`installation instructions <installation>`).

   Unary :cpp:class:`~mppp::real` sqrt1pm1.

   This function will compute and return :math:`\sqrt{1+x}-1`, where :math:`x`
   is the value of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the operand.

   :return: the sqrt1pm1 of *r*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

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

   .. note::
      This function is available from MPFR 4 onwards.

   Binary :cpp:class:`~mppp::real` k-th root.

   This function will compute the k-th root of *op* and store it
   into *rop*. The precision of the result will be equal to the precision
   of *op*.

   If *k* is zero, the result will be NaN. If *k* is odd (resp. even) and *op*
   negative (including negative infinity), the result will be a negative number (resp. NaN).
   If *op* is zero, the result will be zero with the sign obtained by the usual limit rules, i.e.,
   the same sign as *op* if *k* is odd, and positive if *k* is even.

   :param rop: the return value.
   :param op: the operand.
   :param k: the degree of the root.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::rootn_ui(T &&r, unsigned long k)

   .. versionadded:: 0.12

   .. note::
      This function is available from MPFR 4 onwards.

   Unary :cpp:class:`~mppp::real` k-th root.

   This function will compute and return the k-th root of *r*.
   The precision of the result will be equal to the precision
   of *r*.

   If *k* is zero, the result will be NaN. If *k* is odd (resp. even) and *r*
   negative (including negative infinity), the result will be a negative number (resp. NaN).
   If *r* is zero, the result will be zero with the sign obtained by the usual limit rules, i.e.,
   the same sign as *r* if *k* is odd, and positive if *k* is even.

   :param r: the operand.
   :param k: the degree of the root.

   :return: the k-th root of *r*.

.. _real_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U> mppp::real &mppp::pow(mppp::real &rop, T &&op1, U &&op2)

   Ternary exponentiation.

   This function will set *rop* to *op1* raised to the power of *op2*.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param op1: the base.
   :param op2: the exponent.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, typename U> mppp::real pow(T &&op1, U &&op2)

   .. note::

      This function participates in overload resolution only if ``T`` and ``U`` satisfy
      the :cpp:concept:`~mppp::RealOpTypes` concept.

   Binary exponentiation.

   This function will compute and return *op1* raised to the power of *op2*.
   The precision of the result will be set to the largest precision among the operands.

   Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
   before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
   to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
   :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
   precision, if set, or it is automatically deduced depending on the type and value of the
   operand to be converted.

   :param op1: the base.
   :param op2: the exponent.

   :return: *op1* raised to the power of *op2*.

   :exception unspecified: any exception thrown by the generic assignment operator of :cpp:class:`~mppp::real`.

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::sqr(mppp::real &rop, T &&op)

   .. versionadded:: 0.19

   Binary :cpp:class:`~mppp::real` squaring.

   This function will compute the square of *op* and store it
   into *rop*. The precision of the result will be equal to the precision
   of *op*.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::sqr(T &&r)

   .. versionadded:: 0.19

   Unary :cpp:class:`~mppp::real` squaring.

   This function will compute and return the square of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the operand.

   :return: the square of *r*.

.. _real_trig:

Trigonometry
~~~~~~~~~~~~

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::sin(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::cos(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::tan(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::sec(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::csc(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::cot(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::sin_pi(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::cos_pi(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::tan_pi(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::cot_pi(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::sinc(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::sinc_pi(mppp::real &rop, T &&x)

   .. note::
      The functions ``sin_pi()``, ``cos_pi()``, ``tan_pi()``,
      ``cot_pi()``, ``sinc()`` and ``sinc_pi()`` are available only
      if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled
      (see the :ref:`installation instructions <installation>`).

   Binary basic trigonometric functions.

   These functions will set *rop* to, respectively:

   * :math:`\sin\left( x \right)`,
   * :math:`\cos\left( x \right)`,
   * :math:`\tan\left( x \right)`,
   * :math:`\sec\left( x \right)`,
   * :math:`\csc\left( x \right)`,
   * :math:`\cot\left( x \right)`,
   * :math:`\sin\left( \pi x \right)`,
   * :math:`\cos\left( \pi x \right)`,
   * :math:`\tan\left( \pi x \right)`,
   * :math:`\cot\left( \pi x \right)`,
   * :math:`\frac{\sin\left( x \right)}{x}`,
   * :math:`\frac{\sin\left( \pi x \right)}{\pi x}`.

   The precision of the result will be equal to the precision of *x*.

   :param rop: the return value.
   :param x: the argument.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

   .. versionadded:: 0.19

      The functions ``sin_pi()``, ``cos_pi()``, ``tan_pi()``,
      ``cot_pi()``, ``sinc()`` and ``sinc_pi()``.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::sin(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::cos(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::tan(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::sec(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::csc(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::cot(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::sin_pi(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::cos_pi(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::tan_pi(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::cot_pi(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::sinc(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::sinc_pi(T &&x)

   .. note::
      The functions ``sin_pi()``, ``cos_pi()``, ``tan_pi()``,
      ``cot_pi()``, ``sinc()`` and ``sinc_pi()`` are available only
      if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled
      (see the :ref:`installation instructions <installation>`).

   Unary basic trigonometric functions.

   These functions will return, respectively:

   * :math:`\sin\left( x \right)`,
   * :math:`\cos\left( x \right)`,
   * :math:`\tan\left( x \right)`,
   * :math:`\sec\left( x \right)`,
   * :math:`\csc\left( x \right)`,
   * :math:`\cot\left( x \right)`,
   * :math:`\sin\left( \pi x \right)`,
   * :math:`\cos\left( \pi x \right)`,
   * :math:`\tan\left( \pi x \right)`,
   * :math:`\cot\left( \pi x \right)`,
   * :math:`\frac{\sin\left( x \right)}{x}`,
   * :math:`\frac{\sin\left( \pi x \right)}{\pi x}`.

   The precision of the result will be equal to the precision of *x*.

   :param x: the argument.

   :return: the trigonometric function of *x*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

   .. versionadded:: 0.19

      The functions ``sin_pi()``, ``cos_pi()``, ``tan_pi()``,
      ``cot_pi()``, ``sinc()`` and ``sinc_pi()``.

.. cpp:function:: template <mppp::CvrReal T> void mppp::sin_cos(mppp::real &sop, mppp::real &cop, T &&op)

   Simultaneous sine and cosine.

   This function will set *sop* and *cop* respectively to the sine and cosine of *op*.
   *sop* and *cop* must be distinct objects. The precision of *sop* and *rop* will be set to the
   precision of *op*.

   :param sop: the sine return value.
   :param cop: the cosine return value.
   :param op: the operand.

   :exception std\:\:invalid_argument: if *sop* and *cop* are the same object.

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::asin(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::acos(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::atan(mppp::real &rop, T &&op)

   Binary basic inverse trigonometric functions.

   These functions will set *rop* to, respectively, the arcsine, arccosine and
   arctangent of *op*.
   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::asin(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::acos(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::atan(T &&r)

   Unary basic inverse trigonometric functions.

   These functions will return, respectively, the arcsine, arccosine and
   arctangent of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the arcsine, arccosine or arctangent of *r*.

.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U> mppp::real &mppp::atan2(mppp::real &rop, T &&y, U &&x)

   Ternary arctangent-2.

   This function will set *rop* to the arctangent-2 of *y* and *x*.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param y: the sine argument.
   :param x: the cosine argument.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::RealOpTypes<T> U> mppp::real mppp::atan2(T &&y, U &&x)

   Binary arctangent-2.

   This function will compute and return the arctangent-2 of *y* and *x*.

   Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
   before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
   to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment
   operator of :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is
   either the default precision, if set, or it is automatically deduced depending on the type
   and value of the operand to be converted.

   :param y: the sine argument.
   :param x: the cosine argument.

   :return: the arctangent-2 of *y* and *x*.

   :exception unspecified: any exception thrown by the generic assignment operator of :cpp:class:`~mppp::real`.

.. _real_hyper:

Hyperbolic functions
~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::sinh(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::cosh(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::tanh(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::sech(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::csch(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::coth(mppp::real &rop, T &&op)

   Binary basic hyperbolic functions.

   These functions will set *rop* to, respectively, the hyperbolic sine, cosine, tangent, secant,
   cosecant and cotangent of *op*.
   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::sinh(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::cosh(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::tanh(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::sech(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::csch(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::coth(T &&r)

   Unary basic hyperbolic functions.

   These functions will return, respectively, the hyperbolic sine, cosine, tangent,
   secant, cosecant and cotangent of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the hyperbolic sine, cosine, tangent, secant, cosecant or cotangent of *r*.

.. cpp:function:: template <mppp::CvrReal T> void mppp::sinh_cosh(mppp::real &sop, mppp::real &cop, T &&op)

   Simultaneous hyperbolic sine and cosine.

   This function will set *sop* and *cop* respectively to the hyperbolic sine and cosine of *op*.
   *sop* and *cop* must be distinct objects. The precision of *sop* and *rop* will be set to the
   precision of *op*.

   :param sop: the hyperbolic sine return value.
   :param cop: the hyperbolic cosine return value.
   :param op: the operand.

   :exception std\:\:invalid_argument: if *sop* and *cop* are the same object.

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::asinh(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::acosh(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::atanh(mppp::real &rop, T &&op)

   Binary basic inverse hyperbolic functions.

   These functions will set *rop* to, respectively, the inverse hyperbolic sine, cosine and
   tangent of *op*.
   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::asinh(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::acosh(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::atanh(T &&r)

   Unary basic inverse hyperbolic functions.

   These functions will return, respectively, the inverse hyperbolic sine, cosine and
   tangent of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the inverse hyperbolic sine, cosine or tangent of *r*.

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

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::exp(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::exp2(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::exp10(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::expm1(T &&r)

   Unary exponentials.

   These functions will return, respectively,

   * ``e**r``,
   * ``2**r``,
   * ``10**r``,
   * ``e**r-1``.

   The precision of the result will be equal to the precision of *r*.

   :param r: the exponent.

   :return: the exponential of *r*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::log(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::log2(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::log10(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::log1p(mppp::real &rop, T &&op)

   Binary logarithms.

   These functions will set *rop* to, respectively,

   * ``log(op)``,
   * ``log2(op)``,
   * ``log10(op)``,
   * ``log(1+op)``.

   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::log(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::log2(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::log10(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::log1p(T &&r)

   Unary logarithms.

   These functions will return, respectively,

   * ``log(r)``,
   * ``log2(r)``,
   * ``log10(r)``,
   * ``log(1+op)``.

   The precision of the result will be equal to the precision of *r*.

   :param r: the operand.

   :return: the logarithm of *r*.

.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U> mppp::real &mppp::log_hypot(mppp::real &rop, T &&x, U &&y)

   .. versionadded:: 0.19

   .. note::
      This function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled
      (see the :ref:`installation instructions <installation>`).

   Ternary log hypot function.

   This function will set *rop* to :math:`\log\left(\sqrt{x^2+y^2}\right)`.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param x: the first argument.
   :param y: the second argument.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <typename T, typename U> mppp::real mppp::log_hypot(T &&x, U &&y)

   .. versionadded:: 0.19

   .. note::
      This function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled
      (see the :ref:`installation instructions <installation>`).

   .. note::

      This function participates in overload resolution only if ``T`` and ``U`` satisfy
      the :cpp:concept:`~mppp::RealOpTypes` concept.

   Binary log hypot function.

   This function will compute and return :math:`\log\left(\sqrt{x^2+y^2}\right)`.

   Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
   before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
   to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment
   operator of :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is
   either the default precision, if set, or it is automatically deduced depending on the type
   and value of the operand to be converted.

   :param x: the first argument.
   :param y: the second argument.

   :return: the log hypot function of *x* and *y*.

   :exception unspecified: any exception thrown by the generic assignment operator of :cpp:class:`~mppp::real`.
   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. _real_gamma:

Gamma functions
~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::gamma(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::lngamma(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::lgamma(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::digamma(mppp::real &rop, T &&op)

   Binary gamma functions.

   These functions will set *rop* to, respectively,

   * :math:`\Gamma\left(op\right)`,
   * :math:`\ln\Gamma\left(op\right)`,
   * :math:`\ln\left|\Gamma\left(op\right)\right|`,
   * :math:`\psi\left(op\right)`.

   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::gamma(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::lngamma(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::lgamma(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::digamma(T &&r)

   Unary gamma functions.

   These functions will return, respectively,

   * :math:`\Gamma\left(r\right)`,
   * :math:`\ln\Gamma\left(r\right)`,
   * :math:`\ln\left|\Gamma\left(r\right)\right|`,
   * :math:`\psi\left(r\right)`.

   The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the Gamma function, logarithm of the Gamma function,
     logarithm of the absolute value of the Gamma function, or the
     Digamma function of *r*.

.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U> mppp::real &mppp::gamma_inc(mppp::real &rop, T &&x, U &&y)

   .. versionadded:: 0.17

   .. note::
      This function is available from MPFR 4 onwards.

   Ternary incomplete Gamma function.

   This function will set *rop* to the upper incomplete Gamma function of *x* and *y*.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param x: the first argument.
   :param y: the second argument.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::RealOpTypes<T> U> mppp::real mppp::gamma_inc(T &&x, U &&y)

   .. versionadded:: 0.17

   .. note::
      This function is available from MPFR 4 onwards.

   Binary incomplete Gamma function.

   This function will compute and return the upper incomplete Gamma function of *x* and *y*.

   Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
   before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
   to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment
   operator of :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is
   either the default precision, if set, or it is automatically deduced depending on the type
   and value of the operand to be converted.

   :param x: the first argument.
   :param y: the second argument.

   :return: the upper incomplete Gamma function of *x* and *y*

   :exception unspecified: any exception thrown by the generic assignment operator of :cpp:class:`~mppp::real`.

.. _real_bessel:

Bessel functions
~~~~~~~~~~~~~~~~

.. versionadded:: 0.17

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::j0(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::j1(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::jn(mppp::real &rop, long n, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::y0(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::y1(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::yn(mppp::real &rop, long n, T &&op)

   Bessel functions.

   These functions will set *rop* to, respectively,

   * the Bessel function of the first kind of order 0 :math:`\left(J_0\right)` of *op*,
   * the Bessel function of the first kind of order 1 :math:`\left(J_1\right)` of *op*,
   * the Bessel function of the first kind of order *n* :math:`\left(J_n\right)` of *op*,
   * the Bessel function of the second kind of order 0 :math:`\left(Y_0\right)` of *op*,
   * the Bessel function of the second kind of order 1 :math:`\left(Y_1\right)` of *op*,
   * the Bessel function of the second kind of order *n* :math:`\left(Y_n\right)` of *op*.

   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::j0(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::j1(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::jn(long n, T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::y0(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::y1(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::yn(long n, T &&r)

   Bessel functions.

   These functions will return, respectively,

   * the Bessel function of the first kind of order 0 :math:`\left(J_0\right)` of *r*,
   * the Bessel function of the first kind of order 1 :math:`\left(J_1\right)` of *r*,
   * the Bessel function of the first kind of order *n* :math:`\left(J_n\right)` of *r*,
   * the Bessel function of the second kind of order 0 :math:`\left(Y_0\right)` of *r*,
   * the Bessel function of the second kind of order 1 :math:`\left(Y_1\right)` of *r*,
   * the Bessel function of the second kind of order *n* :math:`\left(Y_n\right)` of *r*.

   The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the Bessel function of *r*.

.. _real_err_func:

Error functions
~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::erf(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::erfc(mppp::real &rop, T &&op)

   Binary error functions.

   These functions will set *rop* to, respectively, the error function and the complementary
   error function of *op*.
   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::erf(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::erfc(T &&r)

   Unary error functions.

   These functions will return, respectively, the error function and the complementary
   error function of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the error function or the complementary error function of *r*.

.. _real_other_specfunc:

Other special functions
~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::eint(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::li2(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::zeta(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::ai(mppp::real &rop, T &&op)

   Other binary special functions.

   These functions will set *rop* to, respectively,

   * the exponential integral,
   * the dilogarithm,
   * the Riemann Zeta function,
   * the Airy function,

   of *op*. The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::eint(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::li2(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::zeta(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::ai(T &&r)

   Other unary special functions.

   These functions will return, respectively,

   * the exponential integral,
   * the dilogarithm,
   * the Riemann Zeta function,
   * the Airy function,

   of *r*. The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the exponential integral, dilogarithm, Riemann Zeta function or Airy function
      of *r*.

.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U> mppp::real &mppp::beta(mppp::real &rop, T &&x, U &&y)

   .. versionadded:: 0.17

   .. note::
      This function is available from MPFR 4 onwards.

   Ternary beta function.

   This function will set *rop* to the beta function of *x* and *y*.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param x: the first argument.
   :param y: the second argument.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::RealOpTypes<T> U> mppp::real mppp::beta(T &&x, U &&y)

   .. versionadded:: 0.17

   .. note::
      This function is available from MPFR 4 onwards.

   Binary beta function.

   This function will compute and return the beta function of *x* and *y*.

   Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
   before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
   to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment
   operator of :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is
   either the default precision, if set, or it is automatically deduced depending on the type
   and value of the operand to be converted.

   :param x: the first argument.
   :param y: the second argument.

   :return: the beta function of *x* and *y*.

   :exception unspecified: any exception thrown by the generic assignment operator of :cpp:class:`~mppp::real`.

.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U> mppp::real &mppp::hypot(mppp::real &rop, T &&x, U &&y)

   Ternary hypot function.

   This function will set *rop* to :math:`\sqrt{x^2+y^2}`.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param x: the first argument.
   :param y: the second argument.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::RealOpTypes<T> U> mppp::real mppp::hypot(T &&x, U &&y)

   Binary hypot function.

   This function will compute and return :math:`\sqrt{x^2+y^2}`.

   Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
   before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
   to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment
   operator of :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is
   either the default precision, if set, or it is automatically deduced depending on the type
   and value of the operand to be converted.

   :param x: the first argument.
   :param y: the second argument.

   :return: the hypot function of *x* and *y*.

   :exception unspecified: any exception thrown by the generic assignment operator of :cpp:class:`~mppp::real`.

.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U> mppp::real &mppp::agm(mppp::real &rop, T &&x, U &&y)

   Ternary AGM.

   This function will set *rop* to the arithmetic-geometric mean of *x* and *y*.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param x: the first argument.
   :param y: the second argument.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::RealOpTypes<T> U> mppp::real mppp::agm(T &&x, U &&y)

   Binary AGM.

   This function will compute and return the arithmetic-geometric mean of *x* and *y*.

   Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
   before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
   to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment
   operator of :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is
   either the default precision, if set, or it is automatically deduced depending on the type
   and value of the operand to be converted.

   :param x: the first argument.
   :param y: the second argument.

   :return: the AGM of *x* and *y*.

   :exception unspecified: any exception thrown by the generic assignment operator of :cpp:class:`~mppp::real`.

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

.. _real_literals:

User-defined literals
---------------------

.. versionadded:: 0.19

.. cpp:function:: template <char... Chars> mppp::real mppp::literals::operator"" _r128()
.. cpp:function:: template <char... Chars> mppp::real mppp::literals::operator"" _r256()
.. cpp:function:: template <char... Chars> mppp::real mppp::literals::operator"" _r512()
.. cpp:function:: template <char... Chars> mppp::real mppp::literals::operator"" _r1024()

   User-defined real literals.

   These numeric literal operator templates can be used to construct
   :cpp:class:`mppp::real` instances with, respectively, 128, 256, 512
   and 1024 bits of precision. Floating-point literals in decimal and
   hexadecimal format are supported.

   .. seealso::

      https://en.cppreference.com/w/cpp/language/floating_literal

   :exception std\:\:invalid_argument: if the input sequence of characters is not
     a valid floating-point literal (as defined by the C++ standard).
