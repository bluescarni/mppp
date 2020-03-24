Multiprecision floats
=====================

.. note::

   The functionality described in this section is available only if mp++ was configured
   with the ``MPPP_WITH_MPFR`` option enabled (see the :ref:`installation instructions <installation>`).

.. versionadded:: 0.5

*#include <mp++/real.hpp>*

The real class
--------------

.. cpp:class:: mppp::real

   Multiprecision floating-point class.

   This class represents arbitrary-precision real values encoded in a binary floating-point format.
   It acts as a wrapper around the MPFR :cpp:type:`mpfr_t` type, pairing a multiprecision significand
   (whose size can be set at runtime) to a fixed-size exponent. In other words, :cpp:class:`~mppp::real`
   values can have an arbitrary number of binary digits of precision (limited only by the available memory),
   but the exponent range is limited.

   :cpp:class:`~mppp::real` aims to behave like a C++ floating-point type whose precision is a runtime property
   of the class instances rather than a compile-time property of the type. Because of this, the way precision
   is handled in :cpp:class:`~mppp::real` differs from the way it is managed in MPFR. The most important difference
   is that in operations involving :cpp:class:`~mppp::real` the precision of the result is usually determined
   by the precision of the operands, whereas in MPFR the precision of the operation is determined by the precision
   of the return value (which is always passed as the first function parameter in the MPFR API). For instance,
   in the following code,

   .. code-block:: c++

      auto x = real{5,200} + real{6,150};

   the first operand has a value of 5 and precision of 200 bits, while the second operand has a value of 6 and precision
   150 bits. The precision of the result ``x`` (and the precision at which the addition is computed) will be
   the maximum precision among the two operands, that is, 200 bits.

   The precision of a :cpp:class:`~mppp::real` can be set at construction, or it can be changed later via functions
   such as :cpp:func:`mppp::real::set_prec()`, :cpp:func:`mppp::real::prec_round()`, etc. By default,
   the precision of a :cpp:class:`~mppp::real` is automatically deduced upon construction following a set of heuristics
   aimed at ensuring that the constructed :cpp:class:`~mppp::real` represents exactly the value used for initialisation.
   For instance, by default, the construction of a :cpp:class:`~mppp::real` from a 32 bit integer will yield a
   :cpp:class:`~mppp::real` with a precision of 32 bits. This behaviour can be altered by specifying explicitly
   the desired precision value.

   Most of the functionality is exposed via plain :ref:`functions <real_functions>`, with the
   general convention that the functions are named after the corresponding MPFR functions minus the leading ``mpfr_``
   prefix. For instance, the MPFR call

   .. code-block:: c++

      mpfr_add(rop,a,b,MPFR_RNDN);

   that writes the result of ``a + b``, rounded to nearest, into ``rop``, becomes simply

   .. code-block:: c++

      add(rop,a,b);

   where the ``add()`` function is resolved via argument-dependent lookup. Function calls with overlapping arguments
   are allowed, unless noted otherwise. Unless otherwise specified, the :cpp:class:`~mppp::real` API always
   rounds to nearest (that is, the ``MPFR_RNDN`` rounding mode is used).

   Various :ref:`overloaded operators <real_operators>` are provided. The arithmetic operators always return
   a :cpp:class:`~mppp::real` result. The relational operators, ``==``, ``!=``, ``<``, ``>``, ``<=`` and ``>=`` will
   promote non-:cpp:class:`~mppp::real` arguments to :cpp:class:`~mppp::real` before performing the comparison.
   Alternative comparison functions
   treating NaNs specially are provided for use in the C++ standard library (and wherever strict weak ordering relations
   are needed).

   Member functions are provided to access directly the internal ``mpfr_t`` instance (see
   :cpp:func:`mppp::real::get_mpfr_t()` and :cpp:func:`mppp::real::_get_mpfr_t()`), so that
   it is possible to use transparently the MPFR API with :cpp:class:`~mppp::real` objects.

   .. cpp:function:: real()

      Default constructor.

      The value will be initialised to positive zero, the precision will be
      the value returned by :cpp:func:`mppp::real_prec_min()`.

   .. cpp:function:: real(const real &other)
   .. cpp:function:: real(real &&other) noexcept

      Copy and move constructors.

      The copy constructor performs an exact deep copy of the input object.

      After move construction, the only valid operations on *other* are
      destruction, copy/move assignment and the invocation of the :cpp:func:`~mppp::real::is_valid()`
      member function. After re-assignment, *other* can be used normally again.

      :param other: the construction argument.

   .. cpp:function:: explicit real(const real &other, mpfr_prec_t p)

      Copy constructor with custom precision.

      This constructor will set *this* to a copy of *other* with precision *p*. If *p*
      is smaller than the precision of *other*, a rounding operation will be performed,
      otherwise the value will be copied exactly.

      :param other: the construction argument.
      :param p: the desired precision.

      :exception std\:\:invalid_argument: if *p* is outside the range established by
        :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`.

   .. cpp:function:: explicit real(real_kind k, int sign, mpfr_prec_t p)
   .. cpp:function:: explicit real(real_kind k, mpfr_prec_t p)

      Constructors from a special value, sign and precision.

      This constructor will initialise ``this`` with one of the special values
      specified by the :cpp:type:`mppp::real_kind` enum. The precision of ``this``
      will be *p*.

      If *k* is not NaN, the sign bit will be set to positive if *sign*
      is nonnegative, negative otherwise.

      The second overload invokes the first one with a *sign* of zero.

      :param k: the desired special value.
      :param sign: the desired sign for ``this``.
      :param p: the desired precision for ``this``.

      :exception std\:\:invalid_argument: if *p* is outside the range established by
        :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`.

   .. cpp:function:: template <RealInteroperable T> explicit real(const T &x, mpfr_prec_t p)
   .. cpp:function:: template <RealInteroperable T> explicit real(const T &x)

      Generic constructors.

      The generic constructors will set ``this`` to the value of *x*.

      The variant with the *p* argument will set the precision of ``this``
      exactly to *p*.

      The variant without the *p* argument will set the
      precision of ``this`` according to the following
      heuristics:

      * if *x* is a C++ integral type ``I``, then the precision is set to the bit width of ``I``;
      * if *x* is a C++ floating-point type ``F``, then the precision is set to the number of binary digits
        in the significand of ``F``;
      * if *x* is :cpp:class:`~mppp::integer`, then the precision is set to the number of bits in use by
        *x* (rounded up to the next multiple of the limb type's bit width);
      * if *x* is :cpp:class:`~mppp::rational`, then the precision is set to the sum of the number of bits
        used by numerator and denominator (as established by the previous heuristic for :cpp:class:`~mppp::integer`);
      * if *x* is :cpp:class:`~mppp::real128`, then the precision is set to 113.

      These heuristics aim at preserving the value of *x* in the constructed :cpp:class:`~mppp::real`.

      Construction from ``bool`` will initialise ``this`` to 1 for ``true``, and 0 for ``false``.

      :param x: the construction argument.
      :param p: the desired precision.

      :exception std\:\:overflow_error: if an overflow occurs in the computation of the automatically-deduced precision.
      :exception std\:\:invalid_argument: if *p* is outside the range established by
        :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`.

   .. cpp:function:: template <StringType T> explicit real(const T &s, int base, mpfr_prec_t p)
   .. cpp:function:: template <StringType T> explicit real(const T &s, mpfr_prec_t p)

      Constructors from string, base and precision.

      The first constructor will set ``this`` to the value represented by the :cpp:concept:`~mppp::StringType` *s*, which
      is interpreted as a floating-point number in base *base*. *base* must be either zero (in which case the base
      will be automatically deduced) or a number in the :math:`\left[ 2,62 \right]` range.
      The valid string formats are detailed in the
      documentation of the MPFR function ``mpfr_set_str()``. Note that leading whitespaces are ignored, but trailing
      whitespaces will raise an error.

      The precision of ``this`` will be set to *p*.

      The second constructor calls the first one with a *base* value of 10.

      .. seealso::
         https://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions

      :param s: the input string.
      :param base: the base used in the string representation.
      :param p: the desired precision.

      :exception std\:\:invalid_argument: in the following cases:

         * *base* is not zero and not in the [2,62] range,
         * *p* is outside the valid bounds for a precision value,
         * *s* cannot be interpreted as a floating-point number.

      :exception unspecified: any exception thrown by memory errors in standard containers.

   .. cpp:function:: explicit real(const char *begin, const char *end, int base, mpfr_prec_t p)
   .. cpp:function:: explicit real(const char *begin, const char *end, mpfr_prec_t p)

      Constructors from range of characters, base and precision.

      The first constructor will initialise ``this`` from the content of the input half-open range,
      which is interpreted as the string representation of a floating-point value in base ``base``.

      Internally, the constructor will copy the content of the range to a local buffer, add a
      string terminator, and invoke the constructor from string, base and precision.

      The second constructor calls the first one with a *base* value of 10.

      :param begin: the start of the input range.
      :param end: the end of the input range.
      :param base: the base used in the string representation.
      :param p: the desired precision.

      :exception unspecified: any exception thrown by the constructor from string, or by memory
        allocation errors in standard containers.

   .. cpp:function:: explicit real(const mpfr_t x)

      Constructor from a :cpp:type:`mpfr_t`.

      This constructor will initialise ``this`` with an exact deep copy of *x*.

      .. warning::

         It is the user's responsibility to ensure that *x* has been correctly initialised
         with a precision within the bounds established by :cpp:func:`mppp::real_prec_min()`
         and :cpp:func:`mppp::real_prec_max()`.

      :param x: the :cpp:type:`mpfr_t` that will be deep-copied.

   .. cpp:function:: explicit real(mpfr_t &&x)

      Move constructor from a :cpp:type:`mpfr_t`.

      This constructor will initialise ``this`` with a shallow copy of *x*.

      .. warning::

         It is the user's responsibility to ensure that *x* has been correctly initialised
         with a precision within the bounds established by :cpp:func:`mppp::real_prec_min()`
         and :cpp:func:`mppp::real_prec_max()`.

         Additionally, the user must ensure that, after construction, ``mpfr_clear()`` is never
         called on *x*: the resources previously owned by *x* are now owned by ``this``, which
         will take care of releasing them when the destructor is called.

      :param x: the :cpp:type:`mpfr_t` that will be moved.

   .. cpp:function:: ~real()

      Destructor.

Types
-----

.. cpp:type:: mpfr_t

   This is the type used by the MPFR library to represent multiprecision floats.
   It is defined as an array of size 1 of an unspecified structure.

   .. seealso::

      https://www.mpfr.org/mpfr-current/mpfr.html#Nomenclature-and-Types

.. cpp:type:: mppp::mpfr_struct_t = std::remove_extent_t<mpfr_t>

   The C structure used by MPFR to represent arbitrary-precision floats.
   The MPFR type :cpp:type:`mpfr_t` is defined as an array of size 1 of this structure.

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
   can be used as argument types in one of the :cpp:func:`mppp::real::set()` member function overloads.
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

.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U> mppp::real &mppp::add(mppp::real &rop, T &&a, U &&b)
.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U> mppp::real &mppp::sub(mppp::real &rop, T &&a, U &&b)
.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U> mppp::real &mppp::mul(mppp::real &rop, T &&a, U &&b)
.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U> mppp::real &mppp::div(mppp::real &rop, T &&a, U &&b)

   Ternary basic :cpp:class:`~mppp::real` arithmetics.

   These functions will set *rop* to, respectively:

   * :math:`a+b`,
   * :math:`a-b`,
   * :math:`a \times b`,
   * :math:`a/b`.

   The precision of the result will be set to the largest precision among the operands.

   :param rop: the return value.
   :param a: the first operand.
   :param b: the second operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U, mppp::CvrReal V> mppp::real &mppp::fma(mppp::real &rop, T &&a, U &&b, V &&c)
.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U, mppp::CvrReal V> mppp::real &mppp::fms(mppp::real &rop, T &&a, U &&b, V &&c)

   Quaternary :cpp:class:`~mppp::real` multiply-add/sub.

   These functions will set *rop* to, respectively:

   * :math:`a \times b + c`,
   * :math:`a \times b - c`.

   The precision of the result will be set to the largest precision among the operands.

   :param rop: the return value.
   :param a: the first operand.
   :param b: the second operand.
   :param c: the third operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U, mppp::CvrReal V> mppp::real mppp::fma(T &&a, U &&b, V &&c)
.. cpp:function:: template <mppp::CvrReal T, mppp::CvrReal U, mppp::CvrReal V> mppp::real mppp::fms(T &&a, U &&b, V &&c)

   Ternary :cpp:class:`~mppp::real` multiply-add/sub.

   These functions will return, respectively:

   * :math:`a \times b + c`,
   * :math:`a \times b - c`.

   The precision of the result will be the largest precision among the operands.

   :param a: the first operand.
   :param b: the second operand.
   :param c: the third operand.

   :return: :math:`a \times b \pm c`.

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::neg(mppp::real &rop, T &&x)

   Binary :cpp:class:`~mppp::real` negation.

   This function will set *rop* to :math:`-x`. The precision of the result will be
   equal to the precision of *x*.

   :param rop: the return value.
   :param x: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::neg(T &&x)

   Unary :cpp:class:`~mppp::real` negation.

   This function will return :math:`-x`. The precision of the result will be
   equal to the precision of *x*.

   :param x: the operand.

   :return: :math:`-x`.

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::abs(mppp::real &rop, T &&x)

   Binary :cpp:class:`~mppp::real` absolute value.

   This function will set *rop* to :math:`\left| x \right|`. The precision of the result will be
   equal to the precision of *x*.

   :param rop: the return value.
   :param x: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::abs(T &&x)

   Unary :cpp:class:`~mppp::real` absolute value.

   This function will return :math:`\left| x \right|`. The precision of the result will be
   equal to the precision of *x*.

   :param x: the operand.

   :return: :math:`\left| x \right|`.

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::mul_2ui(mppp::real &rop, T &&x, unsigned long n)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::mul_2si(mppp::real &rop, T &&x, long n)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::div_2ui(mppp::real &rop, T &&x, unsigned long n)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::div_2si(mppp::real &rop, T &&x, long n)

   .. versionadded:: 0.19

   Ternary :cpp:class:`~mppp::real` primitives for exact
   multiplication/division by powers of 2.

   These functions will set *rop* to, respectively:

   * :math:`x \times 2^n` (``mul_2`` variants),
   * :math:`\frac{x}{2^n}` (``div_2`` variants).

   The precision of the result will be equal to the precision of *x*.
   The computation will be exact (that is, no rounding takes place).

   :param rop: the return value.
   :param x: the operand.
   :param n: the power of 2.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::mul_2ui(T &&x, unsigned long n)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::mul_2si(T &&x, long n)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::div_2ui(T &&x, unsigned long n)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::div_2si(T &&x, long n)

   .. versionadded:: 0.19

   Binary :cpp:class:`~mppp::real` primitives for exact
   multiplication/division by powers of 2.

   These functions will return, respectively:

   * :math:`x \times 2^n` (``mul_2`` variants),
   * :math:`\frac{x}{2^n}` (``div_2`` variants).

   The precision of the result will be equal to the precision of *x*.
   The computation will be exact (that is, no rounding takes place).

   :param x: the operand.
   :param n: the power of 2.

   :return: *x* multiplied/divided by :math:`2^n`.

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

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::exp(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::exp2(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::exp10(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::expm1(mppp::real &rop, T &&x)

   Binary exponentials.

   These functions will set *rop* to, respectively,

   * :math:`e^x`,
   * :math:`2^x`,
   * :math:`10^x`,
   * :math:`e^x - 1`.

   The precision of the result will be equal to the precision of *x*.

   :param rop: the return value.
   :param x: the exponent.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::exp(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::exp2(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::exp10(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::expm1(T &&x)

   Unary exponentials.

   These functions will return, respectively,

   * :math:`e^x`,
   * :math:`2^x`,
   * :math:`10^x`,
   * :math:`e^x - 1`.

   The precision of the result will be equal to the precision of *x*.

   :param x: the exponent.

   :return: the exponential of *x*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::log(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::log2(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::log10(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::log1p(mppp::real &rop, T &&x)

   Binary logarithms.

   These functions will set *rop* to, respectively,

   * :math:`\log x`,
   * :math:`\log_2 x`,
   * :math:`\log_{10} x`,
   * :math:`\log\left( 1+x \right)`.

   The precision of the result will be equal to the precision of *x*.

   :param rop: the return value.
   :param x: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::log(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::log2(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::log10(T &&x)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::log1p(T &&x)

   Unary logarithms.

   These functions will return, respectively,

   * :math:`\log x`,
   * :math:`\log_2 x`,
   * :math:`\log_{10} x`,
   * :math:`\log\left( 1+x \right)`.

   The precision of the result will be equal to the precision of *x*.

   :param x: the operand.

   :return: the logarithm of *x*.

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

Polylogarithms
~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::li2(mppp::real &rop, T &&x)

   Binary dilogarithm.

   This function will set *rop* to :math:`\operatorname{Li}_2\left( x \right)`.
   The precision of the result will be equal to the precision of *x*.
   If :math:`x \geq 1`, *rop* will be set to NaN.

   :param rop: the return value.
   :param x: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::li2(T &&x)

   Unary dilogarithm.

   This function will return :math:`\operatorname{Li}_2\left( x \right)`.
   The precision of the result will be equal to the precision of *x*.
   If :math:`x \geq 1`, NaN will be returned.

   :param x: the argument.

   :return: the dilogarithm of *x*.

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
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::zeta(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::CvrReal T> mppp::real &mppp::ai(mppp::real &rop, T &&op)

   Other binary special functions.

   These functions will set *rop* to, respectively,

   * the exponential integral,
   * the Riemann Zeta function,
   * the Airy function,

   of *op*. The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::eint(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::zeta(T &&r)
.. cpp:function:: template <mppp::CvrReal T> mppp::real mppp::ai(T &&r)

   Other unary special functions.

   These functions will return, respectively,

   * the exponential integral,
   * the Riemann Zeta function,
   * the Airy function,

   of *r*. The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the exponential integral, Riemann Zeta function or Airy function of *r*.

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
   :cpp:class:`~mppp::real` instances with, respectively, 128, 256, 512
   and 1024 bits of precision. Floating-point literals in decimal and
   hexadecimal format are supported.

   .. seealso::

      https://en.cppreference.com/w/cpp/language/floating_literal

   :exception std\:\:invalid_argument: if the input sequence of characters is not
     a valid floating-point literal (as defined by the C++ standard).
