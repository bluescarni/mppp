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
   The class is a thin wrapper around the :cpp:type:`__float128` type and the quadmath library, available on GCC,
   Clang and the Intel compiler on most modern platforms, on top of which it provides the following additions:

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

   Various :ref:`overloaded operators <real128_operators>` are provided. Alternative comparison functions
   treating NaNs specially are also provided for use in the C++ standard library (and wherever strict weak ordering relations
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

   .. cpp:function:: constexpr explicit real128(const __float128 &x)

      Constructor from a :cpp:type:`__float128`.

      This constructor will initialise the internal :cpp:type:`__float128`
      value to *x*.

      :param x: the :cpp:type:`__float128` that will be assigned to the internal value.

   .. cpp:function:: template <real128_interoperable T> constexpr real128(const T &x)

      Constructor from interoperable types.

      This constructor will initialise the internal value to *x*.
      Depending on the value and type of *x*, ``this`` may not be exactly equal
      to *x* after initialisation (e.g., if *x* is a very large
      :cpp:class:`~mppp::integer`).

      :param x: the value that will be used for the initialisation.

      :exception std\:\:overflow_error: in case of (unlikely) overflow errors during initialisation.

   .. cpp:function:: template <real128_cpp_complex T> constexpr explicit real128(const T &c)

      .. note::

        This constructor is ``constexpr`` only if at least C++14 is being used.

      .. versionadded:: 0.20

      Constructor from complex C++ types.

      The initialisation is is successful only if the imaginary part of *c* is zero.

      :param c: the input complex value.

      :exception std\:\:domain_error: if the imaginary part of *c* is not zero.

   .. cpp:function:: template <string_type T> explicit real128(const T &s)

      Constructor from string.

      This constructor will initialise ``this`` from the :cpp:concept:`~mppp::string_type` *s*.
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

        This operator is ``constexpr`` only if at least C++14 is being used.

      Assignment from a :cpp:type:`__float128`.

      :param x: the :cpp:type:`__float128` that will be assigned to the internal value.

      :return: a reference to ``this``.

   .. cpp:function:: template <real128_interoperable T> constexpr real128 &operator=(const T &x)

      .. note::

        This operator is ``constexpr`` only if at least C++14 is being used.

      Assignment from interoperable types.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the construction of a
        :cpp:class:`~mppp::real128` from *x*.

   .. cpp:function:: template <real128_cpp_complex T> constexpr real128 &operator=(const T &c)

      .. note::

        This operator is ``constexpr`` only if at least C++14 is being used.

      .. versionadded:: 0.20

      Assignment from complex C++ types.

      :param c: the assignment argument.

      :return: a reference to ``this``.

      :exception std\:\:domain_error: if the imaginary part of *c* is not zero.

   .. cpp:function:: real128 &operator=(const real &x)
   .. cpp:function:: constexpr real128 &operator=(const complex128 &x)
   .. cpp:function:: real128 &operator=(const complex &x)

      .. note::

         The :cpp:class:`~mppp::real` overload is available only if mp++ was configured with the
         ``MPPP_WITH_MPFR`` option enabled. The :cpp:class:`~mppp::complex` overload
         is available only if mp++ was configured with the ``MPPP_WITH_MPC`` option enabled.

      .. note::

        The :cpp:class:`~mppp::complex128` overload is ``constexpr`` only if at least C++14 is being used.

      .. versionadded:: 0.20

      Assignment operators from other mp++ classes.

      These operators are formally equivalent to converting *x* to
      :cpp:class:`~mppp::real128` and then move-assigning the result
      to ``this``.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception raised by the conversion of *x*
        to :cpp:class:`~mppp::real128`.

   .. cpp:function:: template <string_type T> real128 &operator=(const T &s)

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

   .. cpp:function:: template <real128_interoperable T> constexpr explicit operator T() const

      Conversion operator to interoperable types.

      This operator will convert ``this`` to a :cpp:concept:`~mppp::real128_interoperable` type.

      Conversion to C++ types is implemented via direct cast, and thus no checks are
      performed to ensure that the value of ``this`` can be represented by the target type.

      Conversion to :cpp:class:`~mppp::rational`, if successful, is exact.

      Conversion to integral types will produce the truncated counterpart of ``this``.

      :return: ``this`` converted to ``T``.

      :exception std\:\:domain_error: if ``this`` represents a non-finite value and ``T``
        is :cpp:class:`~mppp::integer` or :cpp:class:`~mppp::rational`.

   .. cpp:function:: template <real128_cpp_complex T> constexpr explicit operator T() const

      .. note::

        This operator is ``constexpr`` only if at least C++14 is being used.

      .. versionadded:: 0.20

      Conversion to complex C++ types.

      :return: ``this`` converted to the type ``T``.

   .. cpp:function:: template <real128_interoperable T> constexpr bool get(T &rop) const
   .. cpp:function:: template <real128_cpp_complex T> constexpr bool get(T &rop) const

      .. note::

        The first overload is ``constexpr`` only if at least C++14 is being used.
        The second overload is ``constexpr`` only if at least C++20 is being used.

      Conversion member functions to interoperable and complex C++ types.

      These member functions, similarly to the conversion operator, will convert ``this`` to
      ``T``, storing the result of the conversion into *rop*.
      Differently from the conversion operator, these functions do not raise any exception: if the conversion is
      successful, the functions will return ``true``, otherwise the functions will return ``false``. If the
      conversion fails, *rop* will not be altered. The conversion can fail only if ``T`` is either
      :cpp:class:`~mppp::integer` or :cpp:class:`~mppp::rational`, and ``this`` represents a non-finite value.

      .. versionadded:: 0.20

         The conversion function to complex C++ types.

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

   .. cpp:function:: int ilogb() const
   .. cpp:function:: real128 logb() const

      .. note::

         The ``logb()`` function is available when using ``libquadmath``
         from GCC 6 onwards.

      .. versionadded:: 0.21

      :return: the unbiased exponent of ``this``, as an ``int`` or as a :cpp:class:`~mppp::real128`.

   .. cpp:function:: bool signbit() const

      Sign bit.

      This member function will return the value of the sign bit of ``this``. That is, if ``this``
      is not a NaN the function will return ``true`` if ``this`` is negative or :math:`-0`,
      ``false`` otherwise.
      If ``this`` is NaN, the sign bit of the NaN value will be returned.

      :return: ``true`` if the sign bit of ``this`` is set, ``false`` otherwise.

   .. cpp:function:: constexpr int fpclassify() const

      .. note::

         This function is not ``constexpr`` if the Intel C++ compiler
         is being used.

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
   .. cpp:function:: constexpr bool isfinite() const
   .. cpp:function:: constexpr bool isnormal() const

      .. note::

         These functions are not ``constexpr`` if the Intel C++ compiler
         is being used.

      Detect NaN, infinity, finite value (``finite()`` and ``isfinite()``) or
      normal value.

      .. versionadded:: 0.22

         ``isfinite()`` and ``isnormal()``.

      :return: ``true`` is the value of ``this`` is, respectively,
        NaN, an infinity, finite or normal, ``false`` otherwise.

   .. cpp:function:: constexpr real128 &abs()
   .. cpp:function:: constexpr real128 &fabs()

      .. note::

        These functions are ``constexpr`` only if at least C++14 is being used.

      .. note::

         These functions are not ``constexpr`` if the Intel C++ compiler
         is being used.

      In-place absolute value.

      These member functions will set ``this`` to its absolute value.

      .. versionadded:: 0.23

         The ``fabs()`` overload.

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
   .. cpp:function:: real128 &exp2()
   .. cpp:function:: real128 &expm1()
   .. cpp:function:: real128 &log()
   .. cpp:function:: real128 &log10()
   .. cpp:function:: real128 &log2()
   .. cpp:function:: real128 &log1p()

      .. note::

         The ``exp2()`` function is available when using ``libquadmath``
         from GCC 9 onwards.

      In-place logarithms and exponentials.

      These member functions will set ``this`` to, respectively:

      * :math:`e^x`,
      * :math:`2^x`,
      * :math:`e^x - 1`,
      * :math:`\log{x}`,
      * :math:`\log_{10}{x}`,
      * :math:`\log_2{x}`,
      * :math:`\log{\left( 1 + x \right)}`,

      where :math:`x` is the current value of ``this``.

      .. versionadded:: 0.21

         The ``exp2()``, ``expm1()`` and ``log1p()`` functions.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &lgamma()
   .. cpp:function:: real128 &tgamma()

      In-place gamma functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\log\Gamma\left( x \right)`,
      * :math:`\Gamma\left( x \right)`,

      where :math:`x` is the current value of ``this``.

      .. versionadded:: 0.21

         The ``tgamma()`` function.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &j0()
   .. cpp:function:: real128 &j1()
   .. cpp:function:: real128 &y0()
   .. cpp:function:: real128 &y1()

      .. versionadded:: 0.21

      In-place Bessel functions of the first and second kind.

      These member functions will set ``this`` to, respectively:

      * :math:`J_0\left( x \right)`,
      * :math:`J_1\left( x \right)`,
      * :math:`Y_0\left( x \right)`,
      * :math:`Y_1\left( x \right)`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &erf()
   .. cpp:function:: real128 &erfc()

      In-place error functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\operatorname{erf}\left( x \right)`,
      * :math:`\operatorname{erfc}\left( x \right)`,

      where :math:`x` is the current value of ``this``.

      .. versionadded:: 0.21

         The ``erfc()`` function.

      :return: a reference to ``this``.

   .. cpp:function:: real128 &ceil()
   .. cpp:function:: real128 &floor()
   .. cpp:function:: real128 &nearbyint()
   .. cpp:function:: real128 &rint()
   .. cpp:function:: real128 &round()
   .. cpp:function:: real128 &trunc()

      .. versionadded:: 0.21

      Integer rounding functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\left\lceil x \right\rceil`,
      * :math:`\left\lfloor x \right\rfloor`,
      * the nearest integer value to *x*, according to the current rounding mode,
        without raising the ``FE_INEXACT`` exception,
      * the nearest integer value to *x*, according to the current rounding mode,
        possibly raising the ``FE_INEXACT`` exception,
      * the nearest integer value to *x* rounding halfway cases away from zero,
      * :math:`\operatorname{trunc}\left( x \right)`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

Types
-----

.. cpp:type:: __float128

   A quadruple-precision floating-point type available on GCC, Clang and
   the Intel compiler.
   This is the type wrapped by the :cpp:class:`~mppp::real128` class.

   .. seealso::

      https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html

Concepts
--------

.. cpp:concept:: template <typename T> mppp::real128_interoperable

   This concept is satisfied by real-valued types that can
   interoperate with :cpp:class:`~mppp::real128`.
   Specifically, this concept is satisfied if either:

   * ``T`` is :cpp:class:`~mppp::integer`, or
   * ``T`` is :cpp:class:`~mppp::rational`, or
   * on GCC, the Intel compiler and Clang>=7, ``T`` satisfies
     :cpp:concept:`mppp::cpp_arithmetic`, or
   * on Clang<7, ``T`` satisfies :cpp:concept:`mppp::cpp_arithmetic`, except if
     ``T`` is ``long double``.

.. cpp:concept:: template <typename T> mppp::real128_cpp_complex

   .. versionadded:: 0.20

   This concept is satisfied by complex C++ types that can
   interoperate with :cpp:class:`~mppp::real128`.
   Specifically, this concept is satisfied if either:

   * on GCC, the Intel compiler and Clang>=7, ``T`` satisfies
     :cpp:concept:`mppp::cpp_complex`, or
   * on Clang<7, ``T`` satisfies :cpp:concept:`mppp::cpp_complex`, except if
     ``T`` is ``std::complex<long double>``.

.. cpp:concept:: template <typename T, typename U> mppp::real128_op_types

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <real128_operators>`
   involving :cpp:class:`~mppp::real128` and other types. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::real128`, or
   * one type is :cpp:class:`~mppp::real128` and the other is a :cpp:concept:`~mppp::real128_interoperable` type.

.. cpp:concept:: template <typename T, typename U> mppp::real128_eq_op_types

   .. versionadded:: 0.20

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary equality and inequality operators
   involving :cpp:class:`~mppp::real128` and other types. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` satisfy :cpp:concept:`~mppp::real128_op_types`, or
   * one type is :cpp:class:`~mppp::real128` and the other is a :cpp:concept:`~mppp::real128_cpp_complex` type.

.. _real128_functions:

Functions
---------

.. _real128_conversion:

Conversion
~~~~~~~~~~

.. cpp:function:: template <mppp::real128_interoperable T> constexpr bool mppp::get(T &rop, const mppp::real128 &x)
.. cpp:function:: template <mppp::real128_cpp_complex T> constexpr bool mppp::get(T &rop, const mppp::real128 &x)

   .. note::

      The first overload is ``constexpr`` only if at least C++14 is being used.
      The second overload is ``constexpr`` only if at least C++20 is being used.

   Conversion functions.

   These functions will convert the input :cpp:class:`~mppp::real128` *x* to
   ``T``, storing the result of the conversion into *rop*.
   If the conversion is successful, the functions
   will return ``true``, otherwise the functions will return ``false``. If the conversion fails, *rop* will
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

.. cpp:function:: int mppp::ilogb(const mppp::real128 &x)
.. cpp:function:: real128 mppp::logb(const mppp::real128 &x)

   .. note::

      The ``logb()`` function is available when using ``libquadmath``
      from GCC 6 onwards.

   .. versionadded:: 0.21

   Unbiased exponent.

   :param x: the input argument.

   :return: the unbiased exponent of *x*, as an ``int`` or as a :cpp:class:`~mppp::real128`.

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
.. cpp:function:: constexpr mppp::real128 mppp::fabs(const mppp::real128 &x)

   .. note::

      These functions are not ``constexpr`` if the Intel C++ compiler
      is being used.

   Absolute value.

   .. versionadded:: 0.23

      The ``fabs()`` overload.

   :param x: the :cpp:class:`~mppp::real128` whose absolute value will be computed.

   :return: :math:`\left| x \right|`.

.. cpp:function:: mppp::real128 mppp::scalbn(const mppp::real128 &x, int n)
.. cpp:function:: mppp::real128 mppp::scalbln(const mppp::real128 &x, long n)
.. cpp:function:: mppp::real128 mppp::ldexp(const mppp::real128 &x, int n)

   Multiply by power of 2.

   .. versionadded:: 0.21

      The ``ldexp()`` function.

   :param x: the input :cpp:class:`~mppp::real128`.
   :param n: the power of 2 by which *x* will be multiplied.

   :return: :math:`x \times 2^n`.

.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> mppp::real128 mppp::fdim(const T &x, const U &y)

   .. versionadded:: 0.21

   Positive difference.

   This function returns the positive difference between *x* and *y*.
   That is, if :math:`x>y`, returns :math:`x-y`, otherwise returns :math:`+0`.
   Internally, the implementation uses the ``fdimq()`` function from the quadmath library,
   after the conversion of one of the operands to :cpp:class:`~mppp::real128`
   (if necessary).

   :param x: the first argument.
   :param y: the second argument.

   :return: the positive difference of *x* and *y*.

.. _real128_comparison:

Comparison
~~~~~~~~~~

.. cpp:function:: bool mppp::signbit(const mppp::real128 &x)

   Sign bit.

   :param x: the input value.

   :return: the sign bit of *x* (as returned by :cpp:func:`mppp::real128::signbit()`).

.. cpp:function:: constexpr int mppp::fpclassify(const mppp::real128 &x)

   .. note::

      This function is not ``constexpr`` if the Intel C++ compiler
      is being used.

   Categorise a :cpp:class:`~mppp::real128`.

   :param x: the value whose floating-point category will be returned.

   :return: the category of the value of *x*, as established by :cpp:func:`mppp::real128::fpclassify()`.

.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> mppp::real128 mppp::fmax(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> mppp::real128 mppp::fmin(const T &x, const U &y)

   .. versionadded:: 0.21

   Max/min.

   These functions will return, respectively, the maximum and minimum of the two input operands.
   NaNs are treated as missing data (between a NaN and a numeric value, the numeric value is chosen).
   Internally, the implementation uses the ``fmaxq()`` and ``fminq()`` functions from the quadmath library,
   after the conversion of one of the operands to :cpp:class:`~mppp::real128`
   (if necessary).

   :param x: the first argument.
   :param y: the second argument.

   :return: the maximum and minimum of the two input operands.

.. cpp:function:: constexpr bool mppp::isnan(const mppp::real128 &x)
.. cpp:function:: constexpr bool mppp::isinf(const mppp::real128 &x)
.. cpp:function:: constexpr bool mppp::finite(const mppp::real128 &x)
.. cpp:function:: constexpr bool mppp::isfinite(const mppp::real128 &x)
.. cpp:function:: constexpr bool mppp::isnormal(const mppp::real128 &x)

   .. note::

      These functions are not ``constexpr`` if the Intel C++ compiler
      is being used.

   Detect special values.

   These functions will return ``true`` is *x* is, respectively:

   * NaN,
   * an infinity,
   * a finite value (``finite()`` and ``isfinite()``),
   * a normal value,

   and ``false`` otherwise.

   .. versionadded:: 0.22

      ``isfinite()`` and ``isnormal()``.

   :param x: the input value.

   :return: a boolean flag indicating if *x* is NaN, an infinity, a finite value or a normal value.

.. cpp:function:: constexpr bool mppp::real128_equal_to(const mppp::real128 &x, const mppp::real128 &y)

   .. note::

      This function is not ``constexpr`` if the Intel C++ compiler
      is being used.

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

   .. note::

      This function is not ``constexpr`` if the Intel C++ compiler
      is being used.

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

   .. note::

      This function is not ``constexpr`` if the Intel C++ compiler
      is being used.

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

.. cpp:function:: mppp::real128 mppp::sqrt(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::cbrt(const mppp::real128 &x)

   Root functions.

   These functions will return, respectively:

   * :math:`\sqrt{x}`,
   * :math:`\sqrt[3]{x}`.

   :param x: the input argument.

   :return: the square or cubic root of *x*.

.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> mppp::real128 mppp::hypot(const T &x, const U &y)

   Euclidean distance.

   This function will return :math:`\sqrt{x^2+y^2}`.
   The calculation is performed without undue overflow or underflow during the intermediate
   steps of the calculation.
   Internally,
   the implementation uses the ``hypotq()`` function from the quadmath library,
   after the conversion of one of the operands to :cpp:class:`~mppp::real128`
   (if necessary).

   .. versionadded:: 0.21

      Support for types other than :cpp:class:`~mppp::real128`.

   :param x: the first argument.
   :param y: the second argument.

   :return: :math:`\sqrt{x^2+y^2}`.

.. _real128_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> mppp::real128 mppp::pow(const T &x, const U &y)

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

.. cpp:function:: mppp::real128 mppp::sin(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::cos(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::tan(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::asin(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::acos(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::atan(const mppp::real128 &x)

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

.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> mppp::real128 mppp::atan2(const T &y, const U &x)

   .. versionadded:: 0.21

   Two-arguments arctangent.

   This function will compute :math:`\arctan\left( y,x \right)`. Internally,
   the implementation uses the ``atan2q()`` function from the quadmath library,
   after the conversion of one of the operands to :cpp:class:`~mppp::real128`
   (if necessary).

   :param y: the sine argument.
   :param x: the cosine argument.

   :return: :math:`\arctan\left( y,x \right)`.

.. cpp:function:: void mppp::sincos(const mppp::real128 &x, mppp::real128 *s, mppp::real128 *c)

   .. versionadded:: 0.21

   Simultaneous sine and cosine.

   This function will set the variables pointed to by *s* and *c* to, respectively,
   :math:`\sin x` and :math:`\cos x`.

   :param x: the input argument.
   :param s: a pointer to the sine return value.
   :param c: a pointer to the cosine return value.

.. _real128_hyper:

Hyperbolic functions
~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::sinh(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::cosh(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::tanh(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::asinh(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::acosh(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::atanh(const mppp::real128 &x)

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

.. cpp:function:: mppp::real128 mppp::exp(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::exp2(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::expm1(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::log(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::log10(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::log2(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::log1p(const mppp::real128 &x)

   .. note::

      The ``exp2()`` function is available when using ``libquadmath``
      from GCC 9 onwards.

   Logarithms and exponentials.

   These functions will return, respectively:

   * :math:`e^x`,
   * :math:`2^x`,
   * :math:`e^x - 1`,
   * :math:`\log{x}`,
   * :math:`\log_{10}{x}`,
   * :math:`\log_2{x}`,
   * :math:`\log{\left( 1 + x \right)}`.

   .. versionadded:: 0.21

      The ``exp2()``, ``expm1()`` and ``log1p()`` functions.

   :param x: the input value.

   :return: a logarithm/exponential of *x*.

.. _real128_gamma:

Gamma functions
~~~~~~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::lgamma(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::tgamma(const mppp::real128 &x)

   Gamma functions.

   These functions will return, respectively:

   * :math:`\log\Gamma\left( x \right)`,
   * :math:`\Gamma\left( x \right)`.

   .. versionadded:: 0.21

      The ``tgamma()`` function.

   :param x: the input value.

   :return: the result of the operation.

Bessel functions
~~~~~~~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::j0(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::j1(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::jn(int n, const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::y0(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::y1(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::yn(int n, const mppp::real128 &x)

   .. versionadded:: 0.21

   Bessel functions of the first and second kind of integral order.

   These functions will return, respectively,

   * :math:`J_0\left( x \right)`,
   * :math:`J_1\left( x \right)`,
   * :math:`J_n\left( x \right)`,
   * :math:`Y_0\left( x \right)`,
   * :math:`Y_1\left( x \right)`,
   * :math:`Y_n\left( x \right)`.

   :param n: the order of the Bessel function.
   :param x: the argument.

   :return: a Bessel function of *x*.

.. _real128_miscfuncts:

Other special functions
~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::erf(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::erfc(const mppp::real128 &x)

   Error functions.

   These functions will return, respectively:

   * :math:`\operatorname{erf}\left( x \right)`,
   * :math:`\operatorname{erfc}\left( x \right)`.

   .. versionadded:: 0.21

      The ``erfc()`` function.

   :param x: the input value.

   :return: the (complementary) error function of :math:`x`.

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

.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> mppp::real128 mppp::copysign(const T &x, const U &y)

   .. versionadded:: 0.21

   Copy sign.

   This function composes a floating point value with the magnitude of *x* and the sign of *y*.
   Internally, the implementation uses the ``copysignq()`` function from the quadmath library,
   after the conversion of one of the operands to :cpp:class:`~mppp::real128`
   (if necessary).

   :param x: the first argument.
   :param y: the second argument.

   :return: a value with the magnitude of *x* and the sign of *y*.

Integer and remainder-related functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::ceil(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::floor(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::nearbyint(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::rint(const mppp::real128 &x)
.. cpp:function:: long long mppp::llrint(const mppp::real128 &x)
.. cpp:function:: long mppp::lrint(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::round(const mppp::real128 &x)
.. cpp:function:: long long mppp::llround(const mppp::real128 &x)
.. cpp:function:: long mppp::lround(const mppp::real128 &x)
.. cpp:function:: mppp::real128 mppp::trunc(const mppp::real128 &x)

   .. versionadded:: 0.21

   Integer rounding functions.

   These member functions will return, respectively:

   * :math:`\left\lceil x \right\rceil`,
   * :math:`\left\lfloor x \right\rfloor`,
   * the nearest integer value to *x*, according to the current rounding mode,
     without raising the ``FE_INEXACT`` exception,
   * the nearest integer value to *x*, according to the current rounding mode,
     possibly raising the ``FE_INEXACT`` exception, represented as a:

     * :cpp:class:`~mppp::real128` (``rint()``),
     * ``long long`` (``llrint()``),
     * ``long`` (``lrint()``),

   * the nearest integer value to *x* rounding halfway cases away from zero,
     represented as a:

     * :cpp:class:`~mppp::real128` (``round()``),
     * ``long long`` (``llround()``),
     * ``long`` (``lround()``),

   * :math:`\operatorname{trunc}\left( x \right)`.

   :param x: the input argument.

   :return: the result of the operation.

.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> mppp::real128 mppp::fmod(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> mppp::real128 mppp::remainder(const T &x, const U &y)

   .. versionadded:: 0.21

   Floating modulus and remainder.

   These functions will return, respectively, the floating modulus and the remainder of the
   division :math:`x/y`.

   The floating modulus is :math:`x - n\times y`, where :math:`n` is :math:`x/y` with its fractional part truncated.

   The remainder is :math:`x - m\times y`, where :math:`m` is the integral value nearest the exact value
   :math:`x/y`.

   Special values are handled as described in the C99 standard.

   Internally, the implementation uses the ``fmodq()`` and ``remainderq()`` functions from the quadmath library,
   after the conversion of one of the operands to :cpp:class:`~mppp::real128`
   (if necessary).

   :param x: the numerator.
   :param y: the denominator.

   :return: the floating modulus or remainder of :math:`x/y`.

.. cpp:function:: mppp::real128 mppp::remquo(const mppp::real128 &x, const mppp::real128 &y, int *quo)

   .. versionadded:: 0.21

   Remainder and quotient.

   This function will return the remainder of the division :math:`x/y`. Additionally,
   it will store the sign and at least three of the least significant bits of :math:`x/y` in *quo*.

   :param x: the numerator.
   :param y: the denominator.
   :param quo: a pointer to the quotient return value.

   :return: the remainder of :math:`x/y`.

.. cpp:function:: mppp::real128 mppp::modf(const mppp::real128 &x, mppp::real128 *iptr)

   .. versionadded:: 0.21

   Decompose in integral and fractional parts.

   This function will return the fractional part of *x*, and it will store
   the integral part of *x* into the variable pointed to by *iptr*.

   :param x: the input argument.
   :param iptr: a pointer to the return value for the integral part.

   :return: the fractional part of *x*.

.. _real128_io:

Input/Output
~~~~~~~~~~~~

.. cpp:function:: std::ostream &mppp::operator<<(std::ostream &os, const mppp::real128 &x)

   Output stream operator.

   This function will direct to the output stream *os* the input :cpp:class:`~mppp::real128` *x*.

   :param os: the target stream.
   :param x: the input :cpp:class:`~mppp::real128`.

   :return: a reference to *os*.

   :exception std\:\:overflow_error: in case of (unlikely) overflow errors.
   :exception std\:\:invalid_argument: if the quadmath printing primitive ``quadmath_snprintf()`` returns an error code.
   :exception unspecified: any exception raised by the public interface of ``std::ostream`` or by memory allocation errors.


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

.. cpp:function:: constexpr mppp::real128 mppp::operator+(const mppp::real128 &x)
.. cpp:function:: constexpr mppp::real128 mppp::operator-(const mppp::real128 &x)

   Identity and negation.

   :param x: the argument.

   :return: :math:`x` and :math:`-x` respectively.

.. cpp:function:: constexpr mppp::real128 &mppp::operator++(mppp::real128 &x)
.. cpp:function:: constexpr mppp::real128 &mppp::operator--(mppp::real128 &x)

   .. note::

      These operators are ``constexpr`` only if at least C++14 is being used.

   Prefix increment and decrement.

   :param x: the argument.

   :return: a reference to *x* after it has been incremented/decremented by one.

.. cpp:function:: constexpr mppp::real128 mppp::operator++(mppp::real128 &x, int)
.. cpp:function:: constexpr mppp::real128 mppp::operator--(mppp::real128 &x, int)

   .. note::

      These operators are ``constexpr`` only if at least C++14 is being used.

   Suffix increment and decrement.

   :param x: the argument.

   :return: a copy of *x* before the increment/decrement.

.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr mppp::real128 mppp::operator+(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr mppp::real128 mppp::operator-(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr mppp::real128 mppp::operator*(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr mppp::real128 mppp::operator/(const T &x, const U &y)

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

.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr T &mppp::operator+=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr T &mppp::operator-=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr T &mppp::operator*=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr T &mppp::operator/=(T &x, const U &y)

   .. note::

      These operators are ``constexpr`` only if at least C++14 is being used.

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

.. cpp:function:: template <typename T, mppp::real128_eq_op_types<T> U> constexpr bool mppp::operator==(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_eq_op_types<T> U> constexpr bool mppp::operator!=(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr bool mppp::operator<(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr bool mppp::operator>(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr bool mppp::operator<=(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::real128_op_types<T> U> constexpr bool mppp::operator>=(const T &x, const U &y)

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

   .. versionadded:: 0.20

      Equality and inequality comparison with :cpp:concept:`~mppp::real128_cpp_complex` types.

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

.. cpp:function:: constexpr mppp::real128 mppp::real128_max()
.. cpp:var:: constexpr mppp::real128 mppp::max_128

   The maximum positive finite value representable by :cpp:class:`~mppp::real128`.

.. cpp:function:: constexpr mppp::real128 mppp::real128_min()
.. cpp:var:: constexpr mppp::real128 mppp::min_128

   The minimum positive value representable by :cpp:class:`~mppp::real128`
   with full precision.

.. cpp:function:: constexpr mppp::real128 mppp::real128_epsilon()
.. cpp:var:: constexpr mppp::real128 mppp::epsilon_128

   The difference between 1 and the next larger number representable
   by :cpp:class:`~mppp::real128` (:math:`2^{-112}`).

.. cpp:function:: constexpr mppp::real128 mppp::real128_denorm_min()
.. cpp:var:: constexpr mppp::real128 mppp::denorm_min_128

   The smallest positive denormalized number representable by
   :cpp:class:`~mppp::real128`.

.. cpp:function:: constexpr mppp::real128 mppp::real128_inf()
.. cpp:var:: constexpr mppp::real128 mppp::inf_128
.. cpp:function:: constexpr mppp::real128 mppp::real128_nan()
.. cpp:var:: constexpr mppp::real128 mppp::nan_128

   Positive infinity and NaN.

.. cpp:function:: constexpr mppp::real128 mppp::real128_pi()
.. cpp:var:: constexpr mppp::real128 mppp::pi_128

   Quadruple-precision :math:`\pi` constant.

.. cpp:function:: constexpr mppp::real128 mppp::real128_e()
.. cpp:var:: constexpr mppp::real128 mppp::e_128

   Quadruple-precision :math:`\text{e}` constant (Euler's number).

.. cpp:function:: constexpr mppp::real128 mppp::real128_sqrt2()
.. cpp:var:: constexpr mppp::real128 mppp::sqrt2_128

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

.. _real128_literals:

User-defined literals
---------------------

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
