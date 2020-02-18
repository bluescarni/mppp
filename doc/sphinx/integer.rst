.. _integer_reference:

Multiprecision integers
=======================

*#include <mp++/integer.hpp>*

The ``integer`` class
---------------------

.. doxygenclass:: mppp::integer
   :members:

Types
-----

.. cpp:type:: mp_limb_t

   This type is defined by the GMP library. It is used to represents a limb, that is,
   the part of a multiprecision integer that fits in a single machine word. This is an
   unsigned integral type, typically 64 or 32 bits wide.

   .. seealso::

      https://gmplib.org/manual/Nomenclature-and-Types.html#Nomenclature-and-Types

.. cpp:type:: mp_bitcnt_t

   This type is defined by the GMP library. It is an unsigned integral type used to count bits in a multiprecision
   number.

   .. seealso::

      https://gmplib.org/manual/Nomenclature-and-Types.html#Nomenclature-and-Types

.. cpp:enum-class:: mppp::integer_bitcnt_t : mp_bitcnt_t

   A strongly-typed counterpart to :cpp:type:`mp_bitcnt_t`, used in the constructor of :cpp:class:`~mppp::integer`
   from number of bits.

Concepts
--------

.. cpp:concept:: template <typename T, typename U> mppp::IntegerOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <integer_operators>` and :ref:`functions <integer_functions>`
   involving :cpp:class:`~mppp::integer`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::integer` with the same static size ``SSize``, or
   * one type is an :cpp:class:`~mppp::integer` and the other is a :cpp:concept:`~mppp::CppInteroperable`
     or :cpp:concept:`~mppp::CppComplex` type.

   Note that the modulo, bit-shifting and bitwise logic operators have additional restrictions.

   A corresponding boolean type trait called ``are_integer_op_types`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, typename U> mppp::IntegerIntegralOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <integer_operators>` and :ref:`functions <integer_functions>`
   involving :cpp:class:`~mppp::integer` and C++ integral types. Specifically, the concept will be ``true``
   if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::integer` with the same static size, or
   * one type is an :cpp:class:`~mppp::integer` and the other is a :cpp:concept:`~mppp::CppIntegralInteroperable` type.

   A corresponding boolean type trait called ``are_integer_integral_op_types`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::IntegerBinarySaveDest

   This concept is satisfied if ``T`` is a type into which the serialised binary representation of an
   :cpp:class:`~mppp::integer` with static size ``SSize`` can be written. In other words, the concept is satisfied if
   an object of type ``T`` can be passed as an argument to one of the :cpp:func:`mppp::integer::binary_save()` overloads.

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::IntegerBinaryLoadSrc

   This concept is satisfied if ``T`` is a type from which the serialised binary representation of an
   :cpp:class:`~mppp::integer` with static size ``SSize`` can be loaded. In other words, the concept is satisfied if
   an object of type ``T`` can be passed as an argument to one of the :cpp:func:`mppp::integer::binary_load()` overloads.

.. _integer_functions:

Functions
---------

Much of the functionality of the :cpp:class:`~mppp::integer` class
is exposed via plain functions. These functions
mimic the `GMP API <https://gmplib.org/manual/Integer-Functions.html>`__ where appropriate, but a variety of
convenience/generic overloads is provided as well.

.. _integer_assignment:

Assignment
~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::set_zero(mppp::integer<SSize> &n)

   Set to zero.

   After calling this function, the storage type of *n* will be static and its value will be zero.

   .. note::

      This is a specialised higher-performance alternative to the assignment operator.

   :param n: the argument.

   :return: a reference to *n*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::set_one(mppp::integer<SSize> &n)

   Set to one.

   After calling this function, the storage type of *n* will be static and its value will be one.

   .. note::

      This is a specialised higher-performance alternative to the assignment operator.

   :param n: the argument.

   :return: a reference to *n*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::set_negative_one(mppp::integer<SSize> &n)

   Set to minus one.

   After calling this function, the storage type of *n* will be static and its value will be minus one.

   .. note::

      This is a specialised higher-performance alternative to the assignment operator.

   :param n: the argument.

   :return: a reference to *n*.

.. cpp:function:: template <std::size_t SSize> void mppp::swap(mppp::integer<SSize> &n1, mppp::integer<SSize> &n2) noexcept

   .. versionadded:: 0.15

   Swap.

   This function will efficiently swap the values of *n1* and *n2*.

   :param n1: the first argument.
   :param n2: the second argument.

.. _integer_conversion:

Conversion
~~~~~~~~~~

.. cpp:function:: template <mppp::CppInteroperable T, std::size_t SSize> bool mppp::get(T &rop, const mppp::integer<SSize> &n)

   Generic conversion function from :cpp:class:`~mppp::integer` to C++ fundamental types.

   This function will convert the input :cpp:class:`~mppp::integer` *n* to a
   :cpp:concept:`~mppp::CppInteroperable` type, storing the result of the conversion into *rop*.
   If the conversion is successful, the function
   will return ``true``, otherwise the function will return ``false``. If the conversion fails, *rop* will
   not be altered.

   :param rop: the variable which will store the result of the conversion.
   :param n: the input :cpp:class:`~mppp::integer`.

   :return: ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail only if *rop* is
     a C++ integral which cannot represent the value of *n*.

.. cpp:function:: template <mppp::CppComplex T, std::size_t SSize> bool mppp::get(T &rop, const mppp::integer<SSize> &n)

   .. versionadded:: 0.19

   Generic conversion function from :cpp:class:`~mppp::integer` to C++ complex types.

   This function will convert the input :cpp:class:`~mppp::integer` *n* to a
   :cpp:concept:`~mppp::CppComplex` type, storing the result of the conversion into *rop*.
   The conversion is always successful, and this function will always return ``true``.

   :param rop: the variable which will store the result of the conversion.
   :param n: the input :cpp:class:`~mppp::integer`.

   :return: ``true``.

.. _integer_arithmetic:

Arithmetic
~~~~~~~~~~

.. doxygengroup:: integer_arithmetic
   :content-only:

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::sqr(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n)

   .. versionadded:: 0.18

   Binary :cpp:class:`~mppp::integer` squaring.

   This function will set *rop* to the square of *n*.

   :param rop: the return value.
   :param n: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::sqr(const mppp::integer<SSize> &n)

   .. versionadded:: 0.18

   Unary :cpp:class:`~mppp::integer` squaring.

   This function will return the square of *n*.

   :param n: the argument.

   :return: the square of *n*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::sqrm(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n, const mppp::integer<SSize> &mod)

   .. versionadded:: 0.18

   Ternary modular :cpp:class:`~mppp::integer` squaring.

   This function will set *rop* to the square of *n* modulo *mod*.

   :param rop: the return value.
   :param n: the argument.
   :param mod: the modulus.

   :return: a reference to *rop*.

   :exception mppp\:\:zero_division_error: if *mod* is zero.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::sqrm(const mppp::integer<SSize> &n, const mppp::integer<SSize> &mod)

   .. versionadded:: 0.18

   Binary modular :cpp:class:`~mppp::integer` squaring.

   This function will return the square of *n* modulo *mod*.

   :param n: the argument.
   :param mod: the modulus.

   :return: the square of *n* modulo *mod*.
   :exception mppp\:\:zero_division_error: if *mod* is zero.

.. _integer_division:

Division
~~~~~~~~

.. doxygengroup:: integer_division
   :content-only:

.. _integer_comparison:

Comparison
~~~~~~~~~~

.. doxygengroup:: integer_comparison
   :content-only:

.. _integer_logic:

Logic and bit fiddling
~~~~~~~~~~~~~~~~~~~~~~

.. versionadded:: 0.6

.. doxygengroup:: integer_logic
   :content-only:

.. _integer_ntheory:

Number theoretic functions
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygengroup:: integer_ntheory
   :content-only:

.. _integer_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::pow_ui(mppp::integer<SSize> &rop, const mppp::integer<SSize> &base, unsigned long exp)

   Ternary :cpp:class:`~mppp::integer` exponentiation.

   This function will set *rop* to ``base**exp``.

   :param rop: the return value.
   :param base: the base.
   :param exp: the exponent.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::pow_ui(const mppp::integer<SSize> &base, unsigned long exp)

   Binary :cpp:class:`~mppp::integer` exponentiation.

   :param base: the base.
   :param exp: the exponent.

   :return: ``base**exp``.

.. cpp:function:: template <typename T, typename U> auto mppp::pow(const T &base, const U &exp)

   .. note::

      This function participates in overload resolution only if ``T`` and ``U`` satisfy
      the :cpp:concept:`~mppp::IntegerOpTypes` concept.

   Generic binary :cpp:class:`~mppp::integer` exponentiation.

   This function will raise *base* to the power *exp*, and return the result. If one of the arguments
   is a floating-point or complex value, then the result will be computed via ``std::pow()`` and it will
   also be a floating-point or complex value. Otherwise, the result will be an :cpp:class:`~mppp::integer`.
   In case of a negative integral exponent and integral base, the result will be zero unless
   the absolute value of ``base`` is 1.

   :param base: the base.
   :param exp: the exponent.

   :return: ``base**exp``.

   :exception std\:\:overflow_error: if *base* and *exp* are integrals and *exp* is non-negative and outside the range
     of ``unsigned long``.
   :exception mppp\:\:zero_division_error: if *base* and *exp* are integrals and *base* is zero and *exp* is negative.

.. _integer_roots:

Roots
~~~~~

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::sqrt(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n)

   Binary :cpp:class:`~mppp::integer` square root.

   This function will set *rop* to the truncated integer part of the square root of *n*.

   :param rop: the return value.
   :param n: the argument.

   :return: a reference to *rop*.

   :exception std\:\:domain_error: if *n* is negative.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::sqrt(const mppp::integer<SSize> &n)

   Unary :cpp:class:`~mppp::integer` square root.

   This function will return the truncated integer part of the square root of *n*.

   :param n: the argument.

   :return: the integer square root of *n*.

   :exception std\:\:domain_error: if *n* is negative.

.. cpp:function:: template <std::size_t SSize> void mppp::sqrtrem(mppp::integer<SSize> &rop, mppp::integer<SSize> &rem, const mppp::integer<SSize> &n)

   .. versionadded:: 0.12

   :cpp:class:`~mppp::integer` square root with remainder.

   This function will set *rop* to the truncated integer part of the square root of *n*, and *rem* to the remainder of the operation.
   That is, *rem* will be equal to ``n-rop*rop``, and it will be zero if *n* is a perfect square.

   *rop* and *rem* must be distinct objects.

   :param rop: the first return value (i.e., the integer square root of *n*).
   :param rem: the second return value (i.e., the remainder of the operation).
   :param n: the argument.

   :exception std\:\:domain_error: if *n* is negative.
   :exception std\:\:invalid_argument: if *rop* and *rem* are the same object.

.. cpp:function:: template <std::size_t SSize> bool mppp::perfect_square_p(const mppp::integer<SSize> &n)

   .. versionadded:: 0.12

   Detect perfect square.

   This function returns ``true`` if *n* is a perfect square, ``false`` otherwise.

   :param n: the argument.

   :return: ``true`` if *n* is a perfect square, ``false`` otherwise.

.. cpp:function:: template <std::size_t SSize> bool mppp::root(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n, unsigned long m)

   .. versionadded:: 0.12

   Ternary :math:`m`-th root.

   This function will set *rop* to the truncated integer part of the :math:`m`-th root of *n*. The return value will be ``true`` if the
   computation is exact, ``false`` otherwise.

   :param rop: the return value.
   :param n: the argument.
   :param m: the degree of the root.

   :return: ``true`` if the computation is exact, ``false`` otherwise.

   :exception std\:\:domain_error: if *m* is even and *n* is negative, or if *m* is zero.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::root(const mppp::integer<SSize> &n, unsigned long m)

   .. versionadded:: 0.12

   Binary :math:`m`-th root.

   This function will return the truncated integer part of the :math:`m`-th root of *n*.

   :param n: the argument.
   :param m: the degree of the root.

   :return: the truncated integer part of the :math:`m`-th root of *n*.

   :exception std\:\:domain_error: if *m* is even and *n* is negative, or if *m* is zero.

.. cpp:function:: template <std::size_t SSize> void mppp::rootrem(mppp::integer<SSize> &rop, mppp::integer<SSize> &rem, const mppp::integer<SSize> &n, unsigned long m)

   .. versionadded:: 0.12

   :math:`m`-th root with remainder.

   This function will set *rop* to the truncated integer part of the :math:`m`-th root of *n*, and *rem* to the remainder
   of the operation. That is, *rem* will be equal to ``n-rop**m``, and it will be zero if *n* is a perfect power.

   :param rop: the first return value (i.e., the :math:`m`-th root root of *n*).
   :param rem: the second return value (i.e., the remainder of the operation).
   :param n: the argument.
   :param m: the degree of the root.

   :exception std\:\:domain_error: if *m* is even and *n* is negative, or if *m* is zero.

.. cpp:function:: template <std::size_t SSize> bool mppp::perfect_power_p(const mppp::integer<SSize> &n)

   .. versionadded:: 0.12

   Detect perfect power.

   This function will return ``true`` if *n* is a perfect power, that is, if there exist integers :math:`a` and :math:`b`,
   with :math:`b>1`, such that *n* equals :math:`a^b`.  Otherwise, the function will return ``false``.

   :param n: the argument.

   :return: ``true`` if *n* is a perfect power, ``false`` otherwise.

.. _integer_io:

Input/Output
~~~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> std::ostream &mppp::operator<<(std::ostream &os, const mppp::integer<SSize> &n)

   Stream insertion operator.

   This function will direct to the output stream *os* the input :cpp:class:`~mppp::integer` *n*.

   :param os: the output stream.
   :param n: the input :cpp:class:`~mppp::integer`.

   :return: a reference to *os*.

   :exception std\:\:overflow_error: in case of (unlikely) overflow errors.
   :exception unspecified: any exception raised by the public interface of ``std::ostream`` or by memory allocation errors.

.. _integer_s11n:

Serialisation
~~~~~~~~~~~~~

.. versionadded:: 0.7

.. doxygengroup:: integer_s11n
   :content-only:

.. _integer_other:

Other
~~~~~

.. doxygengroup:: integer_other
   :content-only:

.. _integer_operators:

Mathematical operators
----------------------

Overloaded operators are provided for convenience.
Their interface is generic, and their implementation
is typically built on top of basic :ref:`functions <integer_functions>`.

.. doxygengroup:: integer_operators
   :content-only:

.. _integer_std_specialisations:

Standard library specialisations
--------------------------------

.. cpp:class:: template <std::size_t SSize> std::hash<mppp::integer<SSize>>

   Specialisation of ``std::hash`` for :cpp:class:`mppp::integer`.

   .. cpp:type:: public argument_type = mppp::integer<SSize>
   .. cpp:type:: public result_type = std::size_t

   .. note::

      The :cpp:type:`argument_type` and :cpp:type:`result_type` type aliases are defined only until C++14.

   .. cpp:function:: public std::size_t operator()(const mppp::integer<SSize> &n) const

      :param n: the input :cpp:class:`mppp::integer`.

      :return: a hash value for *n*.

.. _integer_literals:

User-defined literals
---------------------

.. versionadded:: 0.18

.. cpp:function:: template <char... Chars> mppp::integer<1> mppp::literals::operator"" _z1()
.. cpp:function:: template <char... Chars> mppp::integer<2> mppp::literals::operator"" _z2()
.. cpp:function:: template <char... Chars> mppp::integer<3> mppp::literals::operator"" _z3()

   User-defined integer literals.

   These numeric literal operator templates can be used to construct
   :cpp:class:`mppp::integer` instances with, respectively, 1, 2 and 3
   limbs of static storage. Literals in binary, octal, decimal and
   hexadecimal format are supported.

   .. seealso::

      https://en.cppreference.com/w/cpp/language/integer_literal

   :exception std\:\:invalid_argument: if the input sequence of characters is not
     a valid integer literal (as defined by the C++ standard).
