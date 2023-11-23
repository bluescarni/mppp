.. _complex128_reference:

Quadruple-precision complex numbers
===================================

.. note::

   The functionality described in this section is available only if mp++ was configured
   with the ``MPPP_WITH_QUADMATH`` option enabled (see the :ref:`installation instructions <installation>`).

.. versionadded:: 0.20

*#include <mp++/complex128.hpp>*

The complex128 class
--------------------

.. cpp:class:: mppp::complex128

   Quadruple-precision complex floating-point class.

   This class represents complex numbers as pairs of quadruple-precision IEEE 754
   floating-point values.
   The class is a thin wrapper around the :cpp:type:`__complex128` type and the quadmath library, available on GCC,
   Clang and the Intel compiler on most modern platforms, on top of which it provides the following additions:

   * interoperability with other mp++ classes,
   * consistent behaviour with respect to the conventions followed elsewhere in mp++ (e.g., values are
     default-initialised to zero rather than to indefinite values, conversions must be explicit, etc.),
   * enhanced compile-time (``constexpr``) capabilities,
   * a generic C++ API.

   Most of the functionality is exposed via plain :ref:`functions <complex128_functions>`, with the
   general convention that the functions are named after the corresponding quadmath functions minus the
   leading ``c`` prefix and the trailing ``q`` suffix. For instance, the quadmath code

   .. code-block:: c++

      __complex128 a{1, 2};
      auto b = ::csinq(a);

   that computes the sine of :math:`1+2\imath` in quadruple precision, storing the result in ``b``, becomes in mp++

   .. code-block:: c++

      complex128 a{1, 2};
      auto b = sin(a);

   where the ``sin()`` function is resolved via argument-dependent lookup.

   Various :ref:`overloaded operators <complex128_operators>` are provided.

   The :cpp:class:`~mppp::complex128` class is a `literal type
   <https://en.cppreference.com/w/cpp/named_req/LiteralType>`__, and, whenever possible, operations involving
   :cpp:class:`~mppp::complex128` are marked as ``constexpr``. Some functions which are not ``constexpr`` in the quadmath
   library have been reimplemented as ``constexpr`` functions via compiler builtins.

   A :ref:`tutorial <tutorial_complex128>` showcasing various features of :cpp:class:`~mppp::complex128`
   is available.

   .. seealso::
      https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html

      https://gcc.gnu.org/onlinedocs/libquadmath/

   .. cpp:member:: __complex128 m_value

      The internal value.

      This class member gives direct access to the :cpp:type:`__complex128` instance stored
      inside a :cpp:class:`~mppp::complex128`.

   .. cpp:type:: value_type = real128

      Alias to mimick the interface of ``std::complex``.

   .. cpp:function:: constexpr complex128()

      Default constructor.

      The default constructor will set ``this`` to zero.

   .. cpp:function:: complex128(const complex128 &) = default
   .. cpp:function:: complex128(complex128 &&) = default

      :cpp:class:`~mppp::complex128` is trivially copy and
      move constructible.

   .. cpp:function:: constexpr explicit complex128(const __complex128 &c)

      Constructor from a :cpp:type:`__complex128`.

      This constructor will initialise the internal :cpp:type:`__complex128`
      value to *c*.

      :param c: the :cpp:type:`__complex128` that will be assigned to the internal value.

   .. cpp:function:: template <complex128_interoperable T> constexpr complex128(const T &x)

      Constructor from real-valued interoperable types.

      .. note::

         This constructor is ``explicit`` if ``T`` is :cpp:class:`~mppp::real`.

      This constructor will initialise the internal value to *x*.
      Depending on the value and type of *x*, ``this`` may not be exactly equal
      to *x* after initialisation (e.g., if *x* is a very large
      :cpp:class:`~mppp::integer`).

      :param x: the value that will be used for the initialisation.

      :exception unspecified: any exception raised by casting ``T`` to :cpp:class:`~mppp::real128`.

   .. cpp:function:: template <complex128_interoperable T, complex128_interoperable U> constexpr explicit complex128(const T &x, const U &y)

      Constructor from real and imaginary parts.

      This constructor will initialise the internal value to :math:`x+\imath y`.
      Depending on the value and type of *x* and *y*, ``this`` may not be exactly equal
      to :math:`x+\imath y` after initialisation (e.g., if *x* and *y* are very large
      :cpp:class:`~mppp::integer` values).

      :param x: the real part of the value that will be used for the initialisation.
      :param y: the imaginary part of the value that will be used for the initialisation.

      :exception unspecified: any exception raised by casting ``T`` to :cpp:class:`~mppp::real128`.

   .. cpp:function:: template <real128_cpp_complex T> constexpr complex128(const T &c)

      .. note::

        This constructor is ``constexpr`` only if at least C++14 is being used.

      Constructor from ``std::complex``.

      :param x: the complex value that will be used for the initialisation.

   .. cpp:function:: template <string_type T> explicit complex128(const T &s)

      Constructor from string.

      This constructor will initialise ``this`` from the :cpp:concept:`~mppp::string_type` *s*.
      The accepted string formats are:

      * a single floating-point number (e.g., ``1.234``),
      * a single floating-point number surrounded by round brackets
        (e.g., ``(1.234)``),
      * a pair of floating-point numbers, surrounded by round brackets and
        separated by a comma (e.g., ``(1.234, 4.567)``).

      The allowed floating-point representations (for both the real and imaginary part)
      are described in the documentation of the constructor from string of
      :cpp:class:`~mppp::real128`.

      :param s: the string that will be used to initialise ``this``.

      :exception std\:\:invalid_argument: if *s* does not represent a valid quadruple-precision
        complex floating-point value.
      :exception unspecified: any exception thrown by memory errors in standard containers.

   .. cpp:function:: explicit complex128(const char *begin, const char *end)

      Constructor from a range of characters.

      This constructor will initialise ``this`` from the content of the input half-open range, which is interpreted
      as the string representation of a complex value.

      Internally, the constructor will copy the content of the range to a local buffer, add a string terminator, and
      invoke the constructor from string.

      :param begin: the begin of the input range.
      :param end: the end of the input range.

      :exception unspecified: any exception thrown by the constructor from string or by memory errors in standard
        containers.

   .. cpp:function:: complex128 &operator=(const complex128 &) = default
   .. cpp:function:: complex128 &operator=(complex128 &&) = default

      :cpp:class:`~mppp::complex128` is trivially copy and
      move assignable.

   .. cpp:function:: constexpr complex128 &operator=(const __complex128 &c)

      .. note::

        This operator is ``constexpr`` only if at least C++14 is being used.

      Assignment operator from :cpp:type:`__complex128`.

      :param c: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function::  template <complex128_interoperable T> constexpr complex128 &operator=(const T &x)

      .. note::

        This operator is ``constexpr`` only if at least C++14 is being used.

      Assignment from interoperable types.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the construction of a
        :cpp:class:`~mppp::complex128` from *x*.

   .. cpp:function:: template <real128_cpp_complex T> constexpr complex128 &operator=(const T &c)

      .. note::

        This operator is ``constexpr`` only if at least C++14 is being used.

      Assignment from complex C++ types.

      :param c: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: complex128 &operator=(const complex &c)

      .. note::

         This operator is available only if mp++ was configured with the ``MPPP_WITH_MPC`` option enabled.

      .. versionadded:: 0.20

      Assignment operator from :cpp:class:`~mppp::complex`.

      This operator is formally equivalent to converting *c* to
      :cpp:class:`~mppp::complex128` and then move-assigning the result
      to ``this``.

      :param c: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: template <string_type T> complex128 &operator=(const T &s)

      Assignment from string.

      The accepted string formats are the same explained in the constructor
      from string.

      :param s: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the constructor from string.

   .. cpp:function:: constexpr real128 real() const
   .. cpp:function:: constexpr real128 imag() const

      Getters for the real and imaginary parts.

      :return: a copy of the real or imaginary part of ``this``.

   .. cpp:function:: constexpr complex128 &set_real(const real128 &x)
   .. cpp:function:: constexpr complex128 &set_imag(const real128 &x)

      .. note::

        These functions are ``constexpr`` only if at least C++14 is being used.

      Setters for the real and imaginary parts.

      :param x: the desired value for the real or imaginary part of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: constexpr explicit operator __complex128() const

      Conversion to :cpp:type:`__complex128`.

      :return: a copy of :cpp:member:`~mppp::complex128::m_value`.

   .. cpp:function:: template <complex128_interoperable T> constexpr explicit operator T() const

      Conversion to real-valued interoperable types.

      Conversion to non-:cpp:class:`~mppp::real` types uses the cast operator
      of :cpp:class:`~mppp::real128`. Conversion to :cpp:class:`~mppp::real`
      invokes the :cpp:class:`~mppp::real` constructor from :cpp:class:`~mppp::real128`,
      and thus produces a return value with a precision of 113 bits.

      :return: ``this`` converted to the type ``T``.

      :exception std\:\:domain_error: if the imaginary part of ``this`` is not zero.
      :exception unspecified: any exception thrown by the conversion operator of
        :cpp:class:`~mppp::real128`.

   .. cpp:function:: template <real128_cpp_complex T> constexpr explicit operator T() const

      .. note::

        This operator is ``constexpr`` only if at least C++14 is being used.

      Conversion to complex C++ types.

      :return: ``this`` converted to the type ``T``.

   .. cpp:function:: template <complex128_interoperable T> constexpr bool get(T &rop) const
   .. cpp:function:: template <real128_cpp_complex T> constexpr bool get(T &rop) const

      .. note::

        The first overload is ``constexpr`` only if at least C++14 is being used.
        The second overload is ``constexpr`` only if at least C++20 is being used.

      Conversion member functions to interoperable and complex C++ types.

      These member functions, similarly to the conversion operator, will convert ``this`` to
      ``T``, storing the result of the conversion into *rop*.
      Differently from the conversion operator, these functions do not raise any exception: if the conversion is
      successful, the functions will return ``true``, otherwise the functions will return ``false``. If the
      conversion fails, *rop* will not be altered.

      The conversion can fail only in the first overload,
      if either:

      * the imaginary part of ``this`` is not zero, or
      * the conversion of the real part of ``this`` to ``T`` (where ``T`` is neither
        :cpp:class:`~mppp::real128` nor :cpp:class:`~mppp::real`) via
        :cpp:func:`mppp::real128::get()` returns ``false``.

      :param rop: the variable which will store the result of the conversion.

      :return: ``true`` if the conversion succeeds, ``false`` otherwise.

   .. cpp:function:: std::string to_string() const

      Convert to string.

      This member function will convert ``this`` to a decimal string representation in scientific format.
      The number of significant digits in the output (36) guarantees that a :cpp:class:`~mppp::complex128`
      constructed from the returned string will have a value identical to the value of ``this``.

      The string format consists of the real and imaginary parts of ``this`` (as returned
      by :cpp:func:`mppp::real128::to_string()`), separated by a comma
      and enclosed by round brackets.

      :return: a decimal string representation of ``this``.

      :exception unspecified: any exception thrown by :cpp:func:`mppp::real128::to_string()`
        or by the public interface of output streams.

   .. cpp:function:: complex128 &abs()
   .. cpp:function:: complex128 &arg()
   .. cpp:function:: constexpr complex128 &conj()
   .. cpp:function:: complex128 &proj()

      .. note::

        :cpp:func:`~mppp::complex128::conj()` is ``constexpr`` only if at least C++14 is being used.

      In-place absolute value, argument, complex conjugate and projection into Riemann sphere.

      These member functions will set ``this`` to, respectively:

      * :math:`\left| z \right|`,
      * :math:`\arg z`,
      * :math:`\overline{z}`,
      * the projection of :math:`z` into Riemann sphere,

      where :math:`z` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: complex128 &sqrt()

      Square root.

      This member function will set ``this`` to :math:`\sqrt{z}`,
      where :math:`z` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: complex128 &sin()
   .. cpp:function:: complex128 &cos()
   .. cpp:function:: complex128 &tan()

      In-place trigonometric functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\sin z`,
      * :math:`\cos z`,
      * :math:`\tan z`,

      where :math:`z` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: complex128 &asin()
   .. cpp:function:: complex128 &acos()
   .. cpp:function:: complex128 &atan()

      In-place inverse trigonometric functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\arcsin z`,
      * :math:`\arccos z`,
      * :math:`\arctan z`,

      where :math:`z` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: complex128 &sinh()
   .. cpp:function:: complex128 &cosh()
   .. cpp:function:: complex128 &tanh()

      In-place hyperbolic functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\sinh z`,
      * :math:`\cosh z`,
      * :math:`\tanh z`,

      where :math:`z` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: complex128 &asinh()
   .. cpp:function:: complex128 &acosh()
   .. cpp:function:: complex128 &atanh()

      In-place inverse hyperbolic functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\operatorname{arcsinh} z`,
      * :math:`\operatorname{arccosh} z`,
      * :math:`\operatorname{arctanh} z`,

      where :math:`z` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: complex128 &exp()
   .. cpp:function:: complex128 &log()
   .. cpp:function:: complex128 &log10()

      Exponentials and logarithms.

      These member functions will set ``this`` to, respectively:

      * :math:`e^z`,
      * :math:`\log z`,
      * :math:`\log_{10} z`,

      where :math:`z` is the current value of ``this``.

      :return: a reference to ``this``.

Types
-----

.. cpp:type:: __complex128
.. cpp:type:: mppp::cplex128 = __complex128

   :cpp:type:`__complex128` is a quadruple-precision complex floating-point type
   available on GCC, Clang and the Intel compiler.
   This is the type wrapped by the :cpp:class:`~mppp::complex128` class.

   Because :cpp:type:`__complex128` is defined in the ``<quadmath.h>`` header, mp++ also provides
   an alias for :cpp:type:`__complex128` called :cpp:type:`mppp::cplex128`, so that users
   of the library need not to include ``<quadmath.h>`` (which can be problematic on non-GCC
   compilers).

   .. seealso::

      https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html

Concepts
--------

.. cpp:concept:: template <typename T> mppp::complex128_interoperable

   This concept is satisfied by real-valued types that can interoperate
   with :cpp:class:`~mppp::complex128`. Specifically, this concept is
   satisfied if either:

   * ``T`` satisfies :cpp:concept:`~mppp::real128_interoperable`, or
   * ``T`` is :cpp:class:`~mppp::real128`, or
   * ``T`` is :cpp:class:`~mppp::real`.

.. cpp:concept:: template <typename T, typename U> mppp::complex128_op_types

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`functions <complex128_functions>` and :ref:`operators <complex128_operators>`
   involving :cpp:class:`~mppp::complex128` and other types. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::complex128`, or
   * one type is :cpp:class:`~mppp::complex128` and the other is either:

     * a :cpp:concept:`~mppp::real128_interoperable` type, or
     * :cpp:class:`~mppp::real128`, or
     * a :cpp:concept:`~mppp::real128_cpp_complex` type, or

   * one type is :cpp:class:`~mppp::real128` and the other type is
     a :cpp:concept:`~mppp::real128_cpp_complex` type.

.. cpp:concept:: template <typename T, typename U> mppp::complex128_cmp_op_types

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic comparison :ref:`operators <complex128_operators>`
   involving :cpp:class:`~mppp::complex128` and other types. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::complex128`, or
   * one type is :cpp:class:`~mppp::complex128` and the other is either:

     * a :cpp:concept:`~mppp::complex128_interoperable` type, or
     * a :cpp:concept:`~mppp::real128_cpp_complex` type.

.. _complex128_functions:

Functions
---------

Real/imaginary parts
~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: constexpr mppp::real128 mppp::creal(const mppp::complex128 &c)
.. cpp:function:: constexpr mppp::real128 mppp::cimag(const mppp::complex128 &c)

   Getters for the real/imaginary part.

   :param c: the input argument.

   :return: the real/imaginary part of *c*.

.. cpp:function:: constexpr mppp::complex128 &mppp::set_real(mppp::complex128 &c, const mppp::real128 &x)
.. cpp:function:: constexpr mppp::complex128 &mppp::set_imag(mppp::complex128 &c, const mppp::real128 &x)

   .. note::

      These functions are ``constexpr`` only if at least C++14 is being used.

   Setters for the real/imaginary part.

   :param c: the :cpp:class:`~mppp::complex128` whose real/imaginary part will be set.
   :param x: the desired value for the real/imaginary part of *c*.

   :return: a reference to *c*.

Conversion
~~~~~~~~~~

.. cpp:function:: template <mppp::complex128_interoperable T> constexpr bool mppp::get(T &rop, const mppp::complex128 &c)
.. cpp:function:: template <mppp::real128_cpp_complex T> constexpr bool mppp::get(T &rop, const mppp::complex128 &c)

   .. note::

      The first overload is ``constexpr`` only if at least C++14 is being used.
      The second overload is ``constexpr`` only if at least C++20 is being used.

   Conversion functions to interoperable and complex C++ types.

   These functions, similarly to the conversion operator of :cpp:class:`~mppp::complex128`, will convert *c* to
   ``T``, storing the result of the conversion into *rop*.
   Differently from the conversion operator, these functions do not raise any exception: if the conversion is
   successful, the functions will return ``true``, otherwise the functions will return ``false``. If the
   conversion fails, *rop* will not be altered.

   The conversion can fail only in the first overload,
   if either:

   * the imaginary part of *c* is not zero, or
   * the conversion of the real part of *c* to ``T`` (where ``T`` is neither
     :cpp:class:`~mppp::real128` nor :cpp:class:`~mppp::real`) via
     :cpp:func:`mppp::real128::get()` returns ``false``.

   :param rop: the variable which will store the result of the conversion.
   :param c: the value that will be converted to ``T``.

   :return: ``true`` if the conversion succeeds, ``false`` otherwise.

Arithmetic
~~~~~~~~~~

.. cpp:function:: mppp::real128 mppp::abs(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::arg(const mppp::complex128 &z)
.. cpp:function:: constexpr mppp::complex128 mppp::conj(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::proj(const mppp::complex128 &z)

   Absolute value, argument, complex conjugate and projection into Riemann sphere.

   These functions will return, respectively:

   * :math:`\left| z \right|`,
   * :math:`\arg z`,
   * :math:`\overline{z}`,
   * the projection of :math:`z` into Riemann sphere.

   :param z: the input value.

   :return: the result of the operation.

Roots
~~~~~

.. cpp:function:: mppp::complex128 mppp::sqrt(const mppp::complex128 &z)

   Square root.

   :param z: the input value.

   :return: :math:`\sqrt{z}`.

Exponentiation
~~~~~~~~~~~~~~

.. cpp:function:: template <typename T, mppp::complex128_op_types<T> U> mppp::complex128 mppp::pow(const T &x, const T &y)

   Exponentiation.

   This function will return :math:`x^y`. The input arguments are converted to :cpp:class:`~mppp::complex128`
   (if necessary) and the result is computed via the exponentiation function from the quadmath library.

   :param x: the base.
   :param y: the exponent.

   :return: :math:`x^y`.

   :exception unspecified: any exception raised by the conversion of *x* and/or *y* to :cpp:class:`~mppp::complex128`.

Trigonometry
~~~~~~~~~~~~

.. cpp:function:: mppp::complex128 mppp::sin(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::cos(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::tan(const mppp::complex128 &z)

   Trigonometric functions.

   These functions will return, respectively:

   * :math:`\sin z`,
   * :math:`\cos z`,
   * :math:`\tan z`.

   :param z: the input value.

   :return: the value of the trigonometric function.

.. cpp:function:: mppp::complex128 mppp::asin(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::acos(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::atan(const mppp::complex128 &z)

   Inverse trigonometric functions.

   These functions will return, respectively:

   * :math:`\arcsin z`,
   * :math:`\arccos z`,
   * :math:`\arctan z`.

   :param z: the input value.

   :return: the value of the inverse trigonometric function.

Hyperbolic functions
~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: mppp::complex128 mppp::sinh(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::cosh(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::tanh(const mppp::complex128 &z)

   Hyperbolic functions.

   These functions will return, respectively:

   * :math:`\sinh z`,
   * :math:`\cosh z`,
   * :math:`\tanh z`.

   :param z: the input value.

   :return: the value of the hyperbolic function.

.. cpp:function:: mppp::complex128 mppp::asinh(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::acosh(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::atanh(const mppp::complex128 &z)

   Inverse hyperbolic functions.

   These functions will return, respectively:

   * :math:`\operatorname{arcsinh} z`,
   * :math:`\operatorname{arccosh} z`,
   * :math:`\operatorname{arctanh} z`.

   :param z: the input value.

   :return: the value of the inverse hyperbolic function.

Exponentials and logarithms
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: mppp::complex128 mppp::exp(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::log(const mppp::complex128 &z)
.. cpp:function:: mppp::complex128 mppp::log10(const mppp::complex128 &z)

   Exponentials and logarithms.

   These functions will return, respectively:

   * :math:`e^z`,
   * :math:`\log z`,
   * :math:`\log_{10} z`.

   :param z: the input value.

   :return: the value of the exponential or logarithm function.

Input/output
~~~~~~~~~~~~

.. cpp:function:: std::ostream &mppp::operator<<(std::ostream &os, const mppp::complex128 &c)

   Output stream operator.

   This function will direct to the output stream *os* the input :cpp:class:`~mppp::complex128` *c*.

   :param os: the target stream.
   :param c: the input :cpp:class:`~mppp::complex128`.

   :return: a reference to *os*.

   :exception std\:\:overflow_error: in case of (unlikely) overflow errors.
   :exception std\:\:invalid_argument: if the quadmath printing primitive ``quadmath_snprintf()`` returns an error code.
   :exception unspecified: any exception raised by the public interface of ``std::ostream`` or by memory allocation errors.

.. _complex128_operators:

Mathematical operators
----------------------

.. cpp:function:: constexpr mppp::complex128 mppp::operator+(const mppp::complex128 &c)
.. cpp:function:: constexpr mppp::complex128 mppp::operator-(const mppp::complex128 &c)

   Identity and negation.

   :param x: the argument.

   :return: :math:`c` and :math:`-c` respectively.

.. cpp:function:: constexpr mppp::complex128 &mppp::operator++(mppp::complex128 &c)
.. cpp:function:: constexpr mppp::complex128 &mppp::operator--(mppp::complex128 &c)

   .. note::

      These operators are ``constexpr`` only if at least C++14 is being used.

   Prefix increment and decrement.

   :param c: the argument.

   :return: a reference to *c* after it has been incremented/decremented by one.

.. cpp:function:: constexpr mppp::complex128 mppp::operator++(mppp::complex128 &c, int)
.. cpp:function:: constexpr mppp::complex128 mppp::operator--(mppp::complex128 &c, int)

   .. note::

      These operators are ``constexpr`` only if at least C++14 is being used.

   Suffix increment and decrement.

   :param c: the argument.

   :return: a copy of *c* before the increment/decrement.

.. cpp:function:: template <typename T, mppp::complex128_op_types<T> U> constexpr mppp::complex128 mppp::operator+(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::complex128_op_types<T> U> constexpr mppp::complex128 mppp::operator-(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::complex128_op_types<T> U> constexpr mppp::complex128 mppp::operator*(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::complex128_op_types<T> U> constexpr mppp::complex128 mppp::operator/(const T &x, const U &y)

   Binary arithmetic operators.

   These operators will return, respectively:

   * :math:`x+y`,
   * :math:`x-y`,
   * :math:`x\times y`,
   * :math:`x / y`.

   :param x: the first operand.
   :param y: the second operand.

   :return: the result of the binary operation.

   :exception unspecified: any exception thrown by the conversion of *x* or *y* to
     :cpp:class:`~mppp::real128` or :cpp:class:`~mppp::complex128`.

.. cpp:function:: template <typename T, mppp::complex128_op_types<T> U> constexpr T &mppp::operator+=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::complex128_op_types<T> U> constexpr T &mppp::operator-=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::complex128_op_types<T> U> constexpr T &mppp::operator*=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::complex128_op_types<T> U> constexpr T &mppp::operator/=(T &x, const U &y)

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
     of the result of the binary operation to ``T``.

.. cpp:function:: template <typename T, mppp::complex128_cmp_op_types<T> U> constexpr bool mppp::operator==(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::complex128_cmp_op_types<T> U> constexpr bool mppp::operator!=(const T &x, const U &y)

   Comparison operators.

   These operators will return ``true`` if, respectively:

   * :math:`x = y`,
   * :math:`x \neq y`,

   ``false`` otherwise.

   :param x: the first operand.
   :param y: the second operand.

   :return: the result of the comparison.

   :exception unspecified: any exception thrown by the comparison operators of :cpp:class:`~mppp::real128`.

User-defined literals
---------------------

.. cpp:function:: template <char... Chars> mppp::complex128 mppp::literals::operator"" _icq()

   User-defined quadruple-precision imaginary literal.

   This operator will return a :cpp:class:`~mppp::complex128` with zero real part
   and imaginary part constructed from the input floating-point literal in decimal
   or hexadecimal format.

   The operator is implemented on top of :cpp:func:`~mppp::literals::operator"" _rq()`.

   .. seealso::

      https://en.cppreference.com/w/cpp/language/floating_literal

   :exception unspecified: any exception thrown by :cpp:func:`~mppp::literals::operator"" _rq()`.
