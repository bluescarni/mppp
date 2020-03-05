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

      This method will convert ``this`` to a decimal string representation in scientific format.
      The number of significant digits in the output (36) guarantees that a :cpp:class:`~mppp::real128`
      constructed from the returned string will have a value identical to the value of ``this``.

      The implementation uses the ``quadmath_snprintf()`` function from the quadmath library.

      .. seealso::
         https://gcc.gnu.org/onlinedocs/libquadmath/quadmath_005fsnprintf.html

      :return: a decimal string representation of ``this``.

      :exception std\:\:runtime_error: if the internal call to the ``quadmath_snprintf()`` function fails.

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

.. real128_hyper:

Hyperbolic functions
~~~~~~~~~~~~~~~~~~~~

.. doxygengroup:: real128_hyper
   :content-only:

.. _real128_logexp:

Logarithms and exponentials
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygengroup:: real128_logexp
   :content-only:

.. _real128_gamma:

Gamma functions
~~~~~~~~~~~~~~~

.. doxygengroup:: real128_gamma
   :content-only:

.. _real128_miscfuncts:

Other special functions
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygengroup:: real128_miscfuncts
   :content-only:

.. _real128_fpmanip:

Floating-point manipulation
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: mppp::real128 nextafter(const mppp::real128 &from, const mppp::real128 &to)

   .. versionadded:: 0.14

   This function returns the next representable value of *from* in the direction of *to*.

   If *from* equals to *to*, *to* is returned.

   :param from: the :cpp:class:`~mppp::real128` whose next representable value will be returned.
   :param to: the direction of the next representable value.

   :return: the next representable value of *from* in the direction of *to*.

.. _real128_io:

Input/Output
~~~~~~~~~~~~

.. doxygengroup:: real128_io
   :content-only:

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
   :cpp:class:`mppp::real128` instances. Floating-point literals in decimal and
   hexadecimal format are supported.

   .. seealso::

      https://en.cppreference.com/w/cpp/language/floating_literal

   :exception std\:\:invalid_argument: if the input sequence of characters is not
     a valid floating-point literal (as defined by the C++ standard).
