.. _real128_reference:

Quadruple-precision floats
==========================

.. note::

   The functionality described in this section is available only if mp++ was configured
   with the ``MPPP_WITH_QUADMATH`` option enabled (see the :ref:`installation instructions <installation>`).

.. versionadded:: 0.5

*#include <mp++/real128.hpp>*

The real128 class
-----------------

.. cpp:class:: mppp::real128

   Quadruple-precision floating-point class.

   This class represents real values encoded in the quadruple-precision IEEE 754 floating-point format
   (which features up to 36 decimal digits of precision).
   The class is a thin wrapper around the :cpp:type:`__float128` type and the quadmath library, available in GCC
   and recent Clang versions on most modern platforms, on top of which it provides the following additions:

   * interoperability with other mp++ classes,
   * consistent behaviour with respect to the conventions followed elsewhere in mp++ (e.g., values are
     default-initialised to zero rather than to indefinite values, conversions must be explicit, etc.),
   * enhanced compile-time (``constexpr``) capabilities,
   * a generic C++ API.

   Most of the functionality is exposed via plain :ref:`functions <real128_functions>`, with the
   general convention that the functions are named after the corresponding quadmath functions minus the trailing ``q``
   suffix. For instance, the quadmath code

   .. code-block:: c++

      __float128 a = 1;
      auto b = ::sinq(a);

   that computes the sine of 1 in quadruple precision, storing the result in ``b``, becomes in mp++

   .. code-block:: c++

      real128 a{1};
      auto b = sin(a);

   where the ``sin()`` function is resolved via argument-dependent lookup.

   Various :ref:`overloaded operators <real128_operators>` are provided.
   The common arithmetic operators (``+``, ``-``, ``*`` and ``/``) always return :cpp:class:`~mppp::real128`
   as a result, promoting at most one operand to :cpp:class:`~mppp::real128` before actually performing
   the computation. Similarly, the relational operators, ``==``, ``!=``, ``<``, ``>``, ``<=`` and ``>=`` will promote at
   most one argument to :cpp:class:`~mppp::real128` before performing the comparison. Alternative comparison functions
   treating NaNs specially are provided for use in the C++ standard library (and wherever strict weak ordering relations
   are needed).

   The :cpp:class:`~mppp::real128` class is a `literal type
   <https://en.cppreference.com/w/cpp/named_req/LiteralType>`__, and, whenever possible, operations involving
   :cpp:class:`~mppp::real128` are marked as ``constexpr``. Some functions which are not ``constexpr`` in the quadmath
   library have been reimplemented as ``constexpr`` functions via compiler builtins.

   A :ref:`tutorial <tutorial_real128>` showcasing various features of :cpp:class:`~mppp::real128`
   is available.

   .. seealso::
      https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html

      https://gcc.gnu.org/onlinedocs/libquadmath/

   .. cpp:member:: __float128 m_value

      The internal value.

      This class member gives direct access to the :cpp:type:`__float128` instance stored
      inside a :cpp:class:`~mppp::real128`.

   .. cpp:function:: constexpr real128()

      Default constructor.

      The default constructor will set ``this`` to zero.

   .. cpp:function:: real128(const real128 &) = default
   .. cpp:function:: real128(real128 &&) = default

      :cpp:class:`~mppp::real128` is trivially copy and
      move constructible.

   .. cpp:function:: constexpr explicit real128(__float128 x)

      Constructor from a :cpp:type:`__float128`.

      This constructor will initialise the internal :cpp:type:`__float128`
      value to *x*.

      :param x: the :cpp:type:`__float128` that will be assigned to the internal value.

   .. cpp:function:: template <Real128Interoperable T> constexpr explicit real128(const T &x)

      Constructor from interoperable types.

      This constructor will initialise the internal value to *x*.
      Depending on the value and type of *x*, ``this`` may not be exactly equal
      to *x* after initialisation (e.g., if *x* is a very large
      :cpp:class:`~mppp::integer`).

      :param x: the value that will be used for the initialisation.

      :exception std\:\:overflow_error: in case of (unlikely) overflow errors during initialisation.

   .. cpp:function:: template <StringType T> explicit real128(const T &s)

      Constructor from string.

      This constructor will initialise ``this`` from the :cpp:concept:`~mppp::StringType` *s*.
      The accepted string formats are detailed in the quadmath library's documentation
      (see the link below). Leading whitespaces are accepted (and ignored), but trailing whitespaces
      will raise an error.

      .. seealso::
         https://gcc.gnu.org/onlinedocs/libquadmath/strtoflt128.html

      :param s: the string that will be used to initialise ``this``.

      :exception std\:\:invalid_argument: if *s* does not represent a valid quadruple-precision
        floating-point value.
      :exception unspecified: any exception thrown by memory errors in standard containers.

   .. cpp:function:: explicit real128(const char *begin, const char *end)

      Constructor from range of characters.

      This constructor will initialise ``this`` from the content of the input half-open range, which is interpreted
      as the string representation of a floating-point value.

      Internally, the constructor will copy the content of the range to a local buffer, add a string terminator, and
      invoke the constructor from string.

      :param begin: the begin of the input range.
      :param end: the end of the input range.

      :exception unspecified: any exception thrown by the constructor from string or by memory errors in standard
        containers.

   .. cpp:function:: real128 &operator=(const real128 &) = default
   .. cpp:function:: real128 &operator=(real128 &&) = default

      :cpp:class:`~mppp::real128` is trivially copy and
      move assignable.

   .. cpp:function:: constexpr real128 &operator=(const __float128 &x)

      .. note::

        This operator is marked as ``constexpr`` only if at least C++14 is being used.

      Assignment from a :cpp:type:`__float128`.

      :param x: the :cpp:type:`__float128` that will be assigned to the internal value.

      :return: a reference to ``this``.

   .. cpp:function:: template <Real128Interoperable T> constexpr real128 &operator=(const T &x)

      .. note::

        This operator is marked as ``constexpr`` only if at least C++14 is being used.

      Assignment from interoperable types.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the construction of a
        :cpp:class:`~mppp::real128` from *x*.

   .. cpp:function:: template <StringType T> real128 &operator=(const T &s)

      Assignment from string.

      The body of this operator is equivalent to:

      .. code-block:: c++

         return *this = real128{s};

      That is, a temporary :cpp:class:`~mppp::real128` is constructed from *s*
      and it is then move-assigned to ``this``.

      :param s: the string that will be used for the assignment.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the constructor from string.

   .. cpp:function:: constexpr explicit operator __float128() const

      Conversion to :cpp:type:`__float128`.

      :return: a copy of the :cpp:type:`__float128` value stored internally.

   .. cpp:function:: template <Real128Interoperable T> constexpr explicit operator T() const

      Conversion operator to interoperable types.

      This operator will convert ``this`` to a :cpp:concept:`~mppp::Real128Interoperable` type.

      Conversion to C++ types is implemented via direct cast, and thus no checks are
      performed to ensure that the value of ``this`` can be represented by the target type.

      Conversion to :cpp:class:`~mppp::rational`, if successful, is exact.

      Conversion to integral types will produce the truncated counterpart of ``this``.

      :return: ``this`` converted to ``T``.

      :exception std\:\:domain_error: if ``this`` represents a non-finite value and ``T``
        is :cpp:class:`~mppp::integer` or :cpp:class:`~mppp::rational`.

   .. cpp:function:: template <Real128Interoperable T> constexpr bool get(T &rop) const

      .. note::

        This member function is marked as ``constexpr`` only if at least C++14 is being used.

      Conversion member function to interoperable types.

      This member function, similarly to the conversion operator, will convert ``this`` to a
      :cpp:concept:`~mppp::Real128Interoperable` type, storing the result of the conversion into *rop*.
      Differently from the conversion operator, this function does not raise any exception: if the conversion is
      successful, the function will return ``true``, otherwise the function will return ``false``. If the
      conversion fails, *rop* will not be altered. The conversion can fail only if ``T`` is either
      :cpp:class:`~mppp::integer` or :cpp:class:`~mppp::rational`, and ``this`` represents a non-finite value.

      :param rop: the variable which will store the result of the conversion.

      :return: ``true`` if the conversion succeeds, ``false`` otherwise.

   .. cpp:function:: std::string to_string() const

      Convert to string.

      This member function will convert ``this`` to a decimal string representation in scientific format.
      The number of significant digits in the output (36) guarantees that a :cpp:class:`~mppp::real128`
      constructed from the returned string will have a value identical to the value of ``this``.

      The implementation uses the ``quadmath_snprintf()`` function from the quadmath library.

      .. seealso::
         https://gcc.gnu.org/onlinedocs/libquadmath/quadmath_005fsnprintf.html

      :return: a decimal string representation of ``this``.

      :exception std\:\:runtime_error: if the internal call to the ``quadmath_snprintf()`` function fails.

   .. cpp:function:: std::tuple<std::uint_least8_t, std::uint_least16_t, std::uint_least64_t, std::uint_least64_t> get_ieee() const

      Get the IEEE representation of the value.

      This member function will return a tuple containing the IEEE quadruple-precision floating-point representation
      of the value. The returned tuple elements are, in order:

      * the sign of the value (1 for a negative sign bit, 0 for a positive sign bit),
      * the exponent (a 15-bit unsigned value),
      * the high part of the significand (a 48-bit unsigned value),
      * the low part of the significand (a 64-bit unsigned value).

      .. seealso::
         https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format

      :return: a tuple containing the IEEE quadruple-precision floating-point representation of the value stored
        in ``this``.

   .. cpp:function:: bool signbit() const

      Sign bit.

      This member function will return the value of the sign bit of ``this``. That is, if ``this``
      is not a NaN the function will return ``true`` if ``this`` is negative or :math:`-0`,
      ``false`` otherwise.
      If ``this`` is NaN, the sign bit of the NaN value will be returned.

      :return: ``true`` if the sign bit of ``this`` is set, ``false`` otherwise.

   .. cpp:function:: constexpr int fpclassify() const

      Categorise the floating point value.

      This member function will categorise the floating-point value of ``this`` into the 5 categories,
      represented as ``int`` values, defined by the standard:

      * ``FP_NAN`` for NaN,
      * ``FP_INFINITE`` for infinite,
      * ``FP_NORMAL`` for normal values,
      * ``FP_SUBNORMAL`` for subnormal values,
      * ``FP_ZERO`` for zero.

      :return: the category to which the value of ``this`` belongs.

   .. cpp:function:: constexpr bool isnan() const
   .. cpp:function:: constexpr bool isinf() const
   .. cpp:function:: constexpr bool finite() const

      Detect NaN, infinity or finite value.

      :return: ``true`` is the value of ``this`` is, respectively,
        NaN, an infinity or finite, ``false`` otherwise.

   .. cpp:function:: constexpr real128 &abs()

      .. note::

        This operator is marked as ``constexpr`` only if at least C++14 is being used.

      In-place absolute value.

      This member function will set ``this`` to its absolute value.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &sqrt()
   .. cpp:function:: real128 &cbrt()

      In-place roots.

      These member functions will set ``this`` to, respectively:

      * :math:`\sqrt{x}`,
      * :math:`\sqrt[3]{x}`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &sin()
   .. cpp:function:: real128 &cos()
   .. cpp:function:: real128 &tan()

      In-place trigonometric functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\sin{x}`,
      * :math:`\cos{x}`,
      * :math:`\tan{x}`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &asin()
   .. cpp:function:: real128 &acos()
   .. cpp:function:: real128 &atan()

      In-place inverse trigonometric functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\arcsin{x}`,
      * :math:`\arccos{x}`,
      * :math:`\arctan{x}`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &sinh()
   .. cpp:function:: real128 &cosh()
   .. cpp:function:: real128 &tanh()

      In-place hyperbolic functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\sinh{x}`,
      * :math:`\cosh{x}`,
      * :math:`\tanh{x}`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &asinh()
   .. cpp:function:: real128 &acosh()
   .. cpp:function:: real128 &atanh()

      In-place inverse hyperbolic functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\operatorname{arcsinh}{x}`,
      * :math:`\operatorname{arccosh}{x}`,
      * :math:`\operatorname{arctanh}{x}`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &exp()
   .. cpp:function:: real128 &log()
   .. cpp:function:: real128 &log10()
   .. cpp:function:: real128 &log2()

      In-place logarithms and exponentials.

      These member functions will set ``this`` to, respectively:

      * :math:`e^x`,
      * :math:`\log{x}`,
      * :math:`\log_{10}{x}`,
      * :math:`\log_2{x}`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &lgamma()

      In-place logarithm of the gamma function.

      This member function will set ``this`` to :math:`\log\Gamma\left( x \right)`,
      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &erf()

      In-place error function.

      This member function will set ``this`` to :math:`\operatorname{erf}\left( x \right)`,
      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

Types
-----

.. cpp:type:: __float128

   A quadruple-precision floating-point type available in recent versions of the GCC and Clang compilers.
   This is the type wrapped by the :cpp:class:`~mppp::real128` class.

   .. seealso::

      https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html

Concepts
--------

.. cpp:concept:: template <typename T> mppp::Real128Interoperable

   This concept is satisfied by types that can interoperate with :cpp:class:`~mppp::real128`.
   Specifically, this concept is satisfied if either:

   * ``T`` is :cpp:class:`~mppp::integer`, or
   * ``T`` is :cpp:class:`~mppp::rational`, or
   * on GCC, ``T`` satisfies :cpp:concept:`mppp::CppInteroperable`, or
   * on Clang, ``T`` satisfies :cpp:concept:`mppp::CppInteroperable`,
     except if ``T`` is ``long double``.

.. cpp:concept:: template <typename T, typename U> mppp::Real128OpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <real128_operators>`
   involving :cpp:class:`~mppp::real128` and other types. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::real128`, or
   * one type is :cpp:class:`~mppp::real128` and the other is a :cpp:concept:`~mppp::Real128Interoperable` type.

.. _real128_functions:

Functions
---------

.. _real128_conversion:

Conversion
~~~~~~~~~~

.. cpp:function:: template <mppp::Real128Interoperable T> constexpr bool mppp::get(T &rop, const mppp::real128 &x)

   .. note::

     This function is marked as ``constexpr`` only if at least C++14 is being used.

   Conversion function.

   This function will convert the input :cpp:class:`~mppp::real128` *x* to a
   :cpp:concept:`~mppp::Real128Interoperable` type, storing the result of the conversion into *rop*.
   If the conversion is successful, the function
   will return ``true``, otherwise the function will return ``false``. If the conversion fails, *rop* will
   not be altered. The conversion can fail only if ``T``
   is either :cpp:class:`~mppp::integer` or :cpp:class:`~mppp::rational`, and *x*
   represents a non-finite value.

   :param rop: the variable which will store the result of the conversion.
   :param x: the input value.

   :return: ``true`` if the conversion succeeds, ``false`` otherwise.

.. cpp:function:: mppp::real128 mppp::frexp(const mppp::real128 &x, int *exp)

   Decompose a :cpp:class:`~mppp::real128` into a normalized fraction and an integral power of two.

   If *x* is zero, this function will return zero and store zero in *exp*. Otherwise,
   this function will return a :cpp:class:`~mppp::real128` :math:`r` with an absolute value in the
   :math:`\left[0.5,1\right)` range, and it will store an integer value :math:`n` in *exp*
   such that :math:`r \times 2^n` equals to :math:`x`. If *x* is a non-finite value, the return
   value will be *x* and an unspecified value will be stored in *exp*.

   :param x: the input :cpp:class:`~mppp::real128`.
   :param exp: a pointer to the value that will store the exponent.

   :return: the binary significand of *x*.

.. _real128_arithmetic:

Arithmetic
~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::fma(const mppp::real128 &x, const mppp::real128 &y, const mppp::real128 &z)

   Fused multiply-add.

   This function will return :math:`\left(x \times y\right) + z` as if calculated to infinite precision and
   rounded once.

   :param x: the first factor.
   :param y: the second factor.
   :param z: the addend.

   :return: :math:`\left(x \times y\right) + z`.

.. cpp:function:: constexpr mppp::real128 mppp::abs(const mppp::real128 &x)

   Absolute value.

   :param x: the :cpp:class:`~mppp::real128` whose absolute value will be computed.

   :return: :math:`\left| x \right|`.

.. cpp:function:: mppp::real128 mppp::scalbn(const mppp::real128 &x, int n)
.. cpp:function:: mppp::real128 mppp::scalbln(const mppp::real128 &x, long n)

   Multiply by power of 2.

   :param x: the input :cpp:class:`~mppp::real128`.
   :param n: the power of 2 by which *x* will be multiplied.

   :return: :math:`x \times 2^n`.

.. _real128_comparison:

Comparison
~~~~~~~~~~

.. cpp:function:: bool mppp::signbit(const mppp::real128 &x)

   Sign bit.

   :param x: the input value.

   :return: the sign bit of *x* (as returned by :cpp:func:`mppp::real128::signbit()`).

.. cpp:function:: constexpr int mppp::fpclassify(const mppp::real128 &x)

   Categorise a :cpp:class:`~mppp::real128`.

   :param x: the value whose floating-point category will be returned.

   :return: the category of the value of *x*, as established by :cpp:func:`mppp::real128::fpclassify()`.

.. cpp:function:: constexpr bool mppp::isnan(const mppp::real128 &x)
.. cpp:function:: constexpr bool mppp::isinf(const mppp::real128 &x)
.. cpp:function:: constexpr bool mppp::finite(const mppp::real128 &x)

   Detect special values.

   These functions will return ``true`` is *x* is, respectively:

   * NaN,
   * an infinity,
   * a finite value,

   and ``false`` otherwise.

   :param x: the input value.

   :return: a boolean flag indicating if *x* is NaN, an infinity or a finite value.

.. cpp:function:: constexpr bool mppp::real128_equal_to(const mppp::real128 &x, const mppp::real128 &y)

   Equality predicate with special NaN handling.

   If both *x* and *y* are not NaN, this function is identical to the equality operator.
   Otherwise, this function will return ``true``
   if both operands are NaN, ``false`` otherwise.

   In other words, this function behaves like an equality operator which considers all NaN
   values equal to each other.

   :param x: the first operand.
   :param y: the second operand.

   :return: ``true`` if :math:`x = y` (including the case in which both operands are NaN),
     ``false`` otherwise.

.. cpp:function:: constexpr bool mppp::real128_lt(const mppp::real128 &x, const mppp::real128 &y)

   Less-than predicate with special NaN handling.

   If both *x* and *y* are not NaN, this function is identical to the less-than operator.
   If at least one operand is NaN, this function will return ``true``
   if *x* is not NaN, ``false`` otherwise.

   In other words, this function behaves like a less-than operator which considers NaN values
   greater than non-NaN values. This function can be used as a comparator in various facilities of the
   standard library (e.g., ``std::sort()``, ``std::set``, etc.).

   :param x: the first operand.
   :param y: the second operand.

   :return: ``true`` if :math:`x < y` (with NaN values considered greather than non-NaN values),
     ``false`` otherwise.

.. cpp:function:: constexpr bool mppp::real128_gt(const mppp::real128 &x, const mppp::real128 &y)

   Greater-than predicate with special NaN handling.

   If both *x* and *y* are not NaN, this function is identical to the greater-than operator.
   If at least one operand is NaN, this function will return ``true``
   if *y* is not NaN, ``false`` otherwise.

   In other words, this function behaves like a greater-than operator which considers NaN values
   greater than non-NaN values. This function can be used as a comparator in various facilities of the
   standard library (e.g., ``std::sort()``, ``std::set``, etc.).

   :param x: the first operand.
   :param y: the second operand.

   :return: ``true`` if :math:`x > y` (with NaN values considered greather than non-NaN values),
     ``false`` otherwise.

.. _real128_roots:

Roots
~~~~~

.. cpp:function:: mppp::real128 mppp::sqrt(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::cbrt(mppp::real128 x)

   Root functions.

   These functions will return, respectively:

   * :math:`\sqrt{x}`,
   * :math:`\sqrt[3]{x}`.

   :param x: the input argument.

   :return: the square or cubic root of *x*.

.. cpp:function:: mppp::real128 mppp::hypot(const mppp::real128 &x, const mppp::real128 &y)

   Euclidean distance.

   This function will return :math:`\sqrt{x^2+y^2}`.
   The calculation is performed without undue overflow or underflow during the intermediate
   steps of the calculation.

   :param x: the first argument.
   :param y: the second argument.

   :return: :math:`\sqrt{x^2+y^2}`.

.. _real128_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. cpp:function:: template <typename T, typename U> mppp::real128 mppp::pow(const T &x, const U &y)

   .. note::

      This function participates in overload resolution only if ``T`` and ``U`` satisfy
      the :cpp:concept:`~mppp::Real128OpTypes` concept.

   This function will compute :math:`x^y`. Internally,
   the implementation uses the ``powq()`` function from the quadmath library,
   after the conversion of one of the operands to :cpp:class:`~mppp::real128`
   (if necessary).

   :param x: the base.
   :param y: the exponent.

   :return: :math:`x^y`.

.. _real128_trig:

Trigonometry
~~~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::sin(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::cos(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::tan(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::asin(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::acos(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::atan(mppp::real128 x)

   Trigonometric functions.

   These functions will return, respectively:

   * :math:`\sin x`,
   * :math:`\cos x`,
   * :math:`\tan x`,
   * :math:`\arcsin x`,
   * :math:`\arccos x`,
   * :math:`\arctan x`.

   :param x: the input value.

   :return: a trigonometric function of *x*.

.. real128_hyper:

Hyperbolic functions
~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::sinh(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::cosh(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::tanh(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::asinh(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::acosh(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::atanh(mppp::real128 x)

   Hyperbolic functions.

   These functions will return, respectively:

   * :math:`\sinh x`,
   * :math:`\cosh x`,
   * :math:`\tanh x`,
   * :math:`\operatorname{arcsinh} x`,
   * :math:`\operatorname{arccosh} x`,
   * :math:`\operatorname{arctanh} x`.

   :param x: the input value.

   :return: a hyperbolic function of *x*.

.. _real128_logexp:

Logarithms and exponentials
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::exp(mppp::real128 x)

   Exponential function.

   :param x: the input value.

   :return: :math:`e^x`.

.. cpp:function:: mppp::real128 mppp::log(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::log10(mppp::real128 x)
.. cpp:function:: mppp::real128 mppp::log2(mppp::real128 x)

   Logarithms.

   These functions will return, respectively:

   * :math:`\log x`,
   * :math:`\log_{10} x`,
   * :math:`\log_2 x`.

   :param x: the input value.

   :return: a logarithm of *x*.

.. _real128_gamma:

Gamma functions
~~~~~~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::lgamma(mppp::real128 x)

   Natural logarithm of the gamma function.

   :param x: the input value.

   :return: :math:`\log\Gamma\left( x \right)`.

.. _real128_miscfuncts:

Other special functions
~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::erf(mppp::real128 x)

   Error function.

   :param x: the input value.

   :return: :math:`\operatorname{erf}\left( x \right)`.

.. _real128_fpmanip:

Floating-point manipulation
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::nextafter(const mppp::real128 &from, const mppp::real128 &to)

   .. versionadded:: 0.14

   This function returns the next representable value of *from* in the direction of *to*.

   If *from* equals to *to*, *to* is returned.

   :param from: the :cpp:class:`~mppp::real128` whose next representable value will be returned.
   :param to: the direction of the next representable value.

   :return: the next representable value of *from* in the direction of *to*.

.. _real128_io:

Input/Output
~~~~~~~~~~~~

.. cpp:function:: std::ostream &mppp::operator<<(std::ostream &os, const mppp::real128 &x)

   Output stream operator.

   This operator will print to the stream *os* the :cpp:class:`~mppp::real128` *x*. The current implementation
   ignores any formatting flag specified in *os*, and the print format will be the one
   described in :cpp:func:`mppp::real128::to_string()`.

   .. warning::
      In future versions of mp++, the behaviour of this operator will change to support the output stream's formatting
      flags. For the time being, users are encouraged to use the ``quadmath_snprintf()`` function from the quadmath
      library if precise and forward-compatible control on the printing format is needed.

   :param os: the target stream.
   :param x: the input :cpp:class:`~mppp::real128`.

   :return: a reference to *os*.

   :exception unspecified: any exception thrown by :cpp:func:`mppp::real128::to_string()`.


Other
~~~~~

.. cpp:function:: std::size_t mppp::hash(const mppp::real128 &x)

   .. versionadded:: 0.12

   Hash function for :cpp:class:`~mppp::real128`.

   All NaN values produce the same hash value. For non-NaN arguments, this function
   guarantees that ``x == y`` implies ``hash(x) == hash(y)``.

   :param x: the argument.

   :return: a hash value for *x*.

.. _real128_operators:

Mathematical operators
----------------------

.. cpp:function:: constexpr mppp::real128 mppp::operator+(mppp::real128 x)
.. cpp:function:: constexpr mppp::real128 mppp::operator-(mppp::real128 x)

   Identity and negation.

   :param x: the argument.

   :return: :math:`x` and :math:`-x` respectively.

.. cpp:function:: constexpr mppp::real128 &mppp::operator++(mppp::real128 &x)
.. cpp:function:: constexpr mppp::real128 &mppp::operator--(mppp::real128 &x)

   .. note::

      These operators are marked as ``constexpr`` only if at least C++14 is being used.

   Prefix increment and decrement.

   :param x: the argument.

   :return: a reference to *x* after it has been incremented/decremented by one.

.. cpp:function:: constexpr mppp::real128 mppp::operator++(mppp::real128 &x, int)
.. cpp:function:: constexpr mppp::real128 mppp::operator--(mppp::real128 &x, int)

   .. note::

      These operators are marked as ``constexpr`` only if at least C++14 is being used.

   Suffix increment and decrement.

   :param x: the argument.

   :return: a copy of *x* before the increment/decrement.

.. cpp:function:: template <typename T, typename U> constexpr mppp::real128 mppp::operator+(const T &x, const U &y)
.. cpp:function:: template <typename T, typename U> constexpr mppp::real128 mppp::operator-(const T &x, const U &y)
.. cpp:function:: template <typename T, typename U> constexpr mppp::real128 mppp::operator*(const T &x, const U &y)
.. cpp:function:: template <typename T, typename U> constexpr mppp::real128 mppp::operator/(const T &x, const U &y)

   .. note::

      These operators participate in overload resolution only if ``T`` and ``U`` satisfy
      the :cpp:concept:`~mppp::Real128OpTypes` concept.

   Binary arithmetic operators.

   These operators will return, respectively:

   * :math:`x+y`,
   * :math:`x-y`,
   * :math:`x \times y`,
   * :math:`x / y`.

   :param x: the first operand.
   :param y: the second operand.

   :return: the result of the binary operation.

   :exception unspecified: any exception thrown by the constructor of :cpp:class:`~mppp::real128`
     from mp++ types.

.. cpp:function:: template <typename T, typename U> constexpr T &mppp::operator+=(T &x, const U &y)
.. cpp:function:: template <typename T, typename U> constexpr T &mppp::operator-=(T &x, const U &y)
.. cpp:function:: template <typename T, typename U> constexpr T &mppp::operator*=(T &x, const U &y)
.. cpp:function:: template <typename T, typename U> constexpr T &mppp::operator/=(T &x, const U &y)

   .. note::

      These operators participate in overload resolution only if ``T`` and ``U`` satisfy
      the :cpp:concept:`~mppp::Real128OpTypes` concept.

   .. note::

      These operators are marked as ``constexpr`` only if at least C++14 is being used.

   In-place arithmetic operators.

   These operators will set *x* to, respectively:

   * :math:`x+y`,
   * :math:`x-y`,
   * :math:`x \times y`,
   * :math:`x / y`.

   :param x: the first operand.
   :param y: the second operand.

   :return: a reference to *x*.

   :exception unspecified: any exception thrown by the corresponding binary operator, or by the conversion
     of :cpp:class:`~mppp::real128` to mp++ types.

.. cpp:function:: template <typename T, typename U> constexpr bool mppp::operator==(const T &x, const U &y)
.. cpp:function:: template <typename T, typename U> constexpr bool mppp::operator!=(const T &x, const U &y)
.. cpp:function:: template <typename T, typename U> constexpr bool mppp::operator<(const T &x, const U &y)
.. cpp:function:: template <typename T, typename U> constexpr bool mppp::operator>(const T &x, const U &y)
.. cpp:function:: template <typename T, typename U> constexpr bool mppp::operator<=(const T &x, const U &y)
.. cpp:function:: template <typename T, typename U> constexpr bool mppp::operator>=(const T &x, const U &y)

   Comparison operators.

   These operators will return ``true`` if, respectively:

   * :math:`x=y`,
   * :math:`x \neq y`,
   * :math:`x < y`,
   * :math:`x > y`,
   * :math:`x \leq y`,
   * :math:`x \geq y`,

   ``false`` otherwise.

   .. note::

     These operators will handle NaN in the same way as the builtin floating-point types.
     For alternative comparison functions that treat NaN specially, please see the
     :ref:`comparison functions section <real128_comparison>`.

   :param x: the first operand.
   :param y: the second operand.

   :return: the result of the comparison.

   :exception unspecified: any exception thrown by the constructor of :cpp:class:`~mppp::real128`
     from mp++ types.

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

.. cpp:function:: constexpr unsigned mppp::real128_sig_digits()
.. cpp:var:: constexpr unsigned mppp::sig_digits_128

   The number of binary digits in the
   significand of a :cpp:class:`~mppp::real128` (113).

.. cpp:function:: constexpr unsigned mppp::real128_max()
.. cpp:var:: constexpr unsigned mppp::max_128

   The maximum positive finite value representable by :cpp:class:`~mppp::real128`.

.. cpp:function:: constexpr unsigned mppp::real128_min()
.. cpp:var:: constexpr unsigned mppp::min_128

   The minimum positive value representable by :cpp:class:`~mppp::real128`
   with full precision.

.. cpp:function:: constexpr unsigned mppp::real128_epsilon()
.. cpp:var:: constexpr unsigned mppp::epsilon_128

   The difference between 1 and the next larger number representable
   by :cpp:class:`~mppp::real128` (:math:`2^{-112}`).

.. cpp:function:: constexpr unsigned mppp::real128_denorm_min()
.. cpp:var:: constexpr unsigned mppp::denorm_min_128

   The smallest positive denormalized number representable by
   :cpp:class:`~mppp::real128`.

.. cpp:function:: constexpr unsigned mppp::real128_inf()
.. cpp:var:: constexpr unsigned mppp::inf_128
.. cpp:function:: constexpr unsigned mppp::real128_nan()
.. cpp:var:: constexpr unsigned mppp::nan_128

   Positive infinity and NaN.

.. cpp:function:: constexpr unsigned mppp::real128_pi()
.. cpp:var:: constexpr unsigned mppp::pi_128

   Quadruple-precision :math:`\pi` constant.

.. cpp:function:: constexpr unsigned mppp::real128_e()
.. cpp:var:: constexpr unsigned mppp::e_128

   Quadruple-precision :math:`\text{e}` constant (Euler's number).

.. cpp:function:: constexpr unsigned mppp::real128_sqrt2()
.. cpp:var:: constexpr unsigned mppp::sqrt2_128

   Quadruple-precision :math:`\sqrt{2}` constant.

.. _real128_std_specs:

Standard library specialisations
--------------------------------

.. cpp:class:: template <> std::numeric_limits<mppp::real128>

   This specialisation exposes the compile-time properties of :cpp:class:`~mppp::real128` as specified by the C++ standard.

   .. cpp:member:: public static constexpr bool is_specialized = true
   .. cpp:member:: public static constexpr int digits = 113
   .. cpp:member:: public static constexpr int digits10 = 33
   .. cpp:member:: public static constexpr int max_digits10 = 36
   .. cpp:member:: public static constexpr bool is_signed = true
   .. cpp:member:: public static constexpr bool is_integer = false
   .. cpp:member:: public static constexpr bool is_exact = false
   .. cpp:member:: public static constexpr int radix = 2
   .. cpp:member:: public static constexpr int min_exponent = -16381
   .. cpp:member:: public static constexpr int min_exponent10 = -4931
   .. cpp:member:: public static constexpr int max_exponent = 16384
   .. cpp:member:: public static constexpr int max_exponent10 = 4931
   .. cpp:member:: public static constexpr bool has_infinity = true
   .. cpp:member:: public static constexpr bool has_quiet_NaN = true
   .. cpp:member:: public static constexpr bool has_signaling_NaN = false
   .. cpp:member:: public static constexpr std::float_denorm_style has_denorm = std::denorm_present
   .. cpp:member:: public static constexpr bool has_denorm_loss = true
   .. cpp:member:: public static constexpr bool is_iec559 = true
   .. cpp:member:: public static constexpr bool is_bounded = false
   .. cpp:member:: public static constexpr bool is_modulo = false
   .. cpp:member:: public static constexpr bool traps = false
   .. cpp:member:: public static constexpr bool tinyness_before = false
   .. cpp:member:: public static constexpr std::float_round_style round_style = std::round_to_nearest

   .. cpp:function:: public static constexpr mppp::real128 min()

      :return: the output of :cpp:func:`mppp::real128_min()`.

   .. cpp:function:: public static constexpr mppp::real128 max()

      :return: the output of :cpp:func:`mppp::real128_max()`.

   .. cpp:function:: public static constexpr mppp::real128 lowest()

      :return: the negative of the output of :cpp:func:`mppp::real128_max()`.

   .. cpp:function:: public static constexpr mppp::real128 epsilon()

      :return: the output of :cpp:func:`mppp::real128_epsilon()`.

   .. cpp:function:: public static constexpr mppp::real128 round_error()

      :return: ``0.5``.

   .. cpp:function:: public static constexpr mppp::real128 infinity()

      :return: the output of :cpp:func:`mppp::real128_inf()`.

   .. cpp:function:: public static constexpr mppp::real128 quiet_NaN()

      :return: the output of :cpp:func:`mppp::real128_nan()`.

   .. cpp:function:: public static constexpr mppp::real128 signaling_NaN()

      :return: ``0``.

   .. cpp:function:: public static constexpr mppp::real128 denorm_min()

     :return: the output of :cpp:func:`mppp::real128_denorm_min()`.

.. _real128_literal:

User-defined literal
--------------------

.. versionadded:: 0.19

.. cpp:function:: template <char... Chars> mppp::real128 mppp::literals::operator"" _rq()

   User-defined quadruple-precision literal.

   This numeric literal operator template can be used to construct
   :cpp:class:`~mppp::real128` instances. Floating-point literals in decimal and
   hexadecimal format are supported.

   .. seealso::

      https://en.cppreference.com/w/cpp/language/floating_literal

   :exception std\:\:invalid_argument: if the input sequence of characters is not
     a valid floating-point literal (as defined by the C++ standard).
