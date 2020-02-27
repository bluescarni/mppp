.. _real128_reference:

Quadruple-precision floats
==========================

.. note::

   The functionality described in this section is available only if mp++ was configured
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

   A quadruple-precision floating-point type available in recent versions of the GCC and Clang compilers.
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
