.. _rational_reference:

Multiprecision rationals
========================

.. versionadded:: 0.3

*#include <mp++/rational.hpp>*

The rational class
------------------

.. cpp:class:: template <std::size_t SSize> mppp::rational

   Multiprecision rational class.

   This class represents arbitrary-precision rationals. Internally, the class stores a pair of
   :cpp:class:`integers <mppp::integer>` with static size ``SSize`` as the numerator and denominator.
   Rational numbers are represented in the usual canonical form:

   * numerator and denominator are coprime,
   * the denominator is always strictly positive.

   Most of the functionality is exposed via plain :ref:`functions <rational_functions>`, with the
   general convention that the functions are named after the corresponding GMP functions minus the leading ``mpq_``
   prefix. For instance, the GMP call

   .. code-block:: c++

      mpq_add(rop,a,b);

   that writes the result of ``a + b`` into ``rop`` becomes simply

   .. code-block:: c++

      add(rop,a,b);

   where the ``add()`` function is resolved via argument-dependent lookup. Function calls with overlapping arguments
   are allowed, unless noted otherwise.

   Various :ref:`overloaded operators <rational_operators>` are provided.

   The :cpp:class:`~mppp::rational` class allows to access and manipulate directly the numerator and denominator
   via the :cpp:func:`~mppp::rational::get_num()`, :cpp:func:`~mppp::rational::get_den()`,
   :cpp:func:`~mppp::rational::_get_num()` and :cpp:func:`~mppp::rational::_get_den()` member functions, so that it is
   possible to use :cpp:class:`~mppp::integer` functions directly on numerator and denominator. The mutable getters'
   names :cpp:func:`~mppp::rational::_get_num()` and :cpp:func:`~mppp::rational::_get_den()` are prefixed with an
   underscore ``_`` to highlight their potentially dangerous nature: it is the user's responsibility to ensure that the
   canonical form of the rational is preserved after altering the numerator and/or the denominator via the mutable
   getters.

   A :ref:`tutorial <tutorial_rational>` showcasing various features of :cpp:class:`~mppp::rational`
   is available.

   .. cpp:type:: int_t = integer<SSize>

      Underlying integral type.

      This is the :cpp:class:`~mppp::integer` type used for the representation of numerator and
      denominator.

   .. cpp:member:: static constexpr std::size_t ssize = SSize

      Alias for the static size.

   .. cpp:function:: rational()

      The default constructor initialises to zero.

   .. cpp:function:: rational(const rational &other)
   .. cpp:function:: rational(rational &&other) noexcept

      Copy and move constructors.

      The copy constructor will perform a copy of numerator
      and denominator. The move constructor will leave *other* in an unspecified but valid state.

      :param other: the construction argument.

   .. cpp:function:: template <RationalCvrInteroperable<SSize> T> explicit rational(T &&x)

      Generic constructor.

      This constructor will initialize a rational with the value *x*. The construction will fail if either:

      * *x* is a non-finite floating-point value, or,
      * *x* is a complex value whose imaginary part is not zero
        or whose real part is not finite.

      :param x: the value that will be used to initialize ``this``.

      :exception std\:\:domain_error: if *x* is a non-finite floating-point value, or
        a complex value with non-zero imaginary part or non-finite real part.

   .. cpp:function:: template <RationalCvrIntegralInteroperable<SSize> T, RationalCvrIntegralInteroperable<SSize> U> explicit rational(T &&n, U &&d, bool make_canonical = true)

      Constructor from numerator and denominator.

      The input value *n* will be used to initialise
      the numerator, while *d* will be used to initialise the denominator. If *make_canonical* is ``true`` (the
      default), then :cpp:func:`~mppp::rational::canonicalise()` will be called after the construction of numerator and
      denominator.

      .. warning::

         If this constructor is called with *make_canonical* set to ``false``, it will be the user's responsibility
         to ensure that ``this`` is canonicalised before using it with other mp++ functions. Failure to do
         so will result in undefined behaviour.

      :param n: the numerator.
      :param d: the denominator.
      :param make_canonical: if ``true``, the rational will be canonicalised after the construction
        of numerator and denominator.

      :exception zero_division_error: if the denominator is zero.

   .. cpp:function:: template <StringType T> explicit rational(const T &s, int base = 10)

      Constructor from string.

      This constructor will initialize ``this`` from the :cpp:concept:`~mppp::StringType` *s*, which must represent
      a rational value in base *base*. The expected format is either a numerator-denominator pair separated
      by the division operator ``/``, or just a numerator (in which case the denominator will be set to one).
      The format for numerator and denominator is described in the documentation of the constructor from string
      of :cpp:class:`~mppp::integer`.

      :param s: the input string.
      :param base: the base used in the string representation.

      :exception zero_division_error: if the denominator is zero.
      :exception unspecified: any exception thrown by the string constructor of :cpp:class:`~mppp::integer`, or by
        memory errors in standard containers.

   .. cpp:function:: explicit rational(const char *begin, const char *end, int base = 10)

      Constructor from a range of characters.

      This constructor will initialise ``this`` from the content of the input half-open range,
      which is interpreted as the string representation of a rational in base *base*.

      Internally, the constructor will copy the content of the range to a local buffer, add a
      string terminator, and invoke the constructor from string.

      :param begin: the begin of the input range.
      :param end: the end of the input range.
      :param base: the base used in the string representation.

      :exception unspecified: any exception thrown by the constructor from string, or by
        memory errors in standard containers.

   .. cpp:function:: explicit rational(const mpz_t n)

      Constructor from a GMP integer.

      This constructor will initialise the numerator of ``this`` with a copy of
      the input GMP integer *n*, and the denominator to 1.

      .. warning::

         It is the user's responsibility to ensure that *n* has been correctly initialized. Calling this constructor
         with an uninitialized *n* results in undefined behaviour.

      :param n: the input GMP integer.

   .. cpp:function:: explicit rational(const mpq_t q)

      Constructor from a GMP rational.

      This constructor will initialise the numerator and denominator of ``this`` with copies
      of those of the GMP rational *q*.

      .. warning::

         It is the user's responsibility to ensure that *q* has been correctly initialized. Calling this constructor
         with an uninitialized *q* results in undefined behaviour.

         This constructor will **not**
         canonicalise ``this``: numerator and denominator are constructed as-is from *q*.

      :param q: the input GMP rational.

   .. cpp:function:: explicit rational(mpq_t &&q)

      Move constructor from a GMP rational.

      This constructor will move the numerator and denominator of the GMP rational *q* into ``this``.

      .. warning::

         It is the user's responsibility to ensure that *q* has been correctly initialized. Calling this constructor
         with an uninitialized *q* results in undefined behaviour.

         This constructor will **not**
         canonicalise ``this``: numerator and denominator are constructed as-is from *q*.

         The user must ensure that, after construction, ``mpq_clear()`` is never
         called on *q*: the resources previously owned by *q* are now owned by ``this``, which
         will take care of releasing them when the destructor is called.

      .. note::

         Due to a compiler bug, this constructor is not available on Microsoft Visual Studio.

      :param q: the input GMP rational.

   .. cpp:function:: rational &operator=(const rational &other)
   .. cpp:function:: rational &operator=(rational &&other) noexcept

      Copy and move assignment.

      After move-assignment, *other* is left in an unspecified but valid state.

      :param other: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: template <RationalCvrInteroperable<SSize> T> rational &operator=(T &&x)

      Generic assignment operator.

      This operator will assign *x* to ``this``.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the generic constructor.

   .. cpp:function:: template <StringType T> rational &operator=(const T &s)

      Assignment from string.

      The body of this operator is equivalent to:

      .. code-block:: c++

         return *this = rational{s};

      That is, a temporary rational is constructed from *s* and it is then
      move-assigned to ``this``.

      :param s: the string that will be used for the assignment.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the constructor from string.

   .. cpp:function:: rational &operator=(const mpz_t n)

      Assignment from a GMP integer.

      This assignment operator will copy into the numerator of ``this`` the value of the GMP integer *n*,
      and it will set the denominator to 1 via :cpp:func:`mppp::integer::set_one()`.

      .. warning::

         It is the user's responsibility to ensure that *n* has been correctly initialized. Calling this operator
         with an uninitialized *n* results in undefined behaviour.

         No aliasing is allowed:
         the data in *n* must be completely distinct from the data in ``this``.

      :param n: the input GMP integer.

      :return: a reference to ``this``.

   .. cpp:function:: rational &operator=(const ::mpq_t q)

      Assignment from a GMP rational.

      This assignment operator will copy into ``this`` the value of the GMP rational *q*.

      .. warning::

         It is the user's responsibility to ensure that *q* has been correctly initialized. Calling this operator
         with an uninitialized *q* results in undefined behaviour.

         This operator will **not** canonicalise
         the assigned value: numerator and denominator are assigned as-is from *q*.

         No aliasing is allowed:
         the data in *q* must be completely distinct from the data in ``this``.

      :param q: the input GMP rational.

      :return: a reference to ``this``.

   .. cpp:function:: rational &operator=(mpq_t &&q)

      Move assignment from a GMP rational.

      This assignment operator will move the GMP rational *q* into ``this``.

      .. warning::

         It is the user's responsibility to ensure that *q* has been correctly initialized. Calling this operator
         with an uninitialized *q* results in undefined behaviour.

         This operator will **not** canonicalise
         the assigned value: numerator and denominator are assigned as-is from *q*.

         No aliasing is allowed:
         the data in *q* must be completely distinct from the data in ``this``.

         The user must ensure that, after the assignment, ``mpq_clear()`` is never
         called on *q*: the resources previously owned by *q* are now owned by ``this``, which
         will take care of releasing them when the destructor is called.

      .. note::

         Due to a compiler bug, this operator is not available on Microsoft Visual Studio.

      :param q: the input GMP rational.

      :return: a reference to ``this``.

Types
-----

.. cpp:type:: mpq_t

   This is the type used by the GMP library to represent multiprecision rationals.
   It is defined as an array of size 1 of an unspecified structure.

   .. seealso::

      https://gmplib.org/manual/Nomenclature-and-Types.html#Nomenclature-and-Types

Concepts
--------

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::RationalInteroperable

   This concept is satisfied if the type ``T`` can interoperate with a :cpp:class:`~mppp::rational`
   with static size ``SSize``. Specifically, this concept will be ``true`` if ``T`` satisfies
   :cpp:concept:`~mppp::CppInteroperable` or :cpp:concept:`~mppp::CppComplex`,
   or it is an :cpp:class:`~mppp::integer` with static size ``SSize``.

   A corresponding boolean type trait called ``is_rational_interoperable`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::RationalCvrInteroperable

   This concept is satisfied if the type ``T``, after the removal of reference and cv qualifiers,
   satisfies :cpp:concept:`~mppp::RationalInteroperable`.

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::RationalIntegralInteroperable

   This concept is satisfied if either:

   * ``T`` satisfies :cpp:concept:`~mppp::CppIntegralInteroperable`, or
   * ``T`` is an :cpp:class:`~mppp::integer` with static size ``SSize``.

   A corresponding boolean type trait called ``is_rational_integral_interoperable`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::RationalCvrIntegralInteroperable

   This concept is satisfied if the type ``T``, after the removal of reference and cv qualifiers,
   satisfies :cpp:concept:`~mppp::RationalIntegralInteroperable`.

.. cpp:concept:: template <typename T, typename U> mppp::RationalOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <rational_operators>` and :ref:`functions <rational_functions>`
   involving :cpp:class:`~mppp::rational`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::rational` with the same static size ``SSize``, or
   * one type is a :cpp:class:`~mppp::rational` and the other is a :cpp:concept:`~mppp::RationalInteroperable`
     type.

   A corresponding boolean type trait called ``are_rational_op_types`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, typename U> mppp::RationalRealOpTypes

   This concept will be ``true`` if:

   * ``T`` and ``U`` satisfy :cpp:concept:`~mppp::RationalOpTypes`, and
   * neither ``T`` nor ``U`` satisfy :cpp:concept:`~mppp::CppComplex`.

   A corresponding boolean type trait called ``are_rational_real_op_types`` is also available (even if the compiler does
   not support concepts).

.. _rational_functions:

Functions
---------

Much of the functionality of the :cpp:class:`~mppp::rational` class is exposed
via plain functions. These functions
mimic the `GMP API <https://gmplib.org/manual/Rational-Number-Functions.html>`__ where appropriate, but a variety of
convenience/generic overloads is provided as well.

.. _rational_assignment:

Assignment
~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> void mppp::swap(mppp::rational<SSize> &q1, mppp::rational<SSize> &q2) noexcept

   .. versionadded:: 0.15

   Swap.

   This function will efficiently swap the values of *q1* and *q2*.

   :param q1: the first argument.
   :param q2: the second argument.

.. _rational_conversion:

Conversion
~~~~~~~~~~

.. doxygengroup:: rational_conversion
   :content-only:

.. _rational_arithmetic:

Arithmetic
~~~~~~~~~~

.. doxygengroup:: rational_arithmetic
   :content-only:

.. _rational_comparison:

Comparison
~~~~~~~~~~

.. doxygengroup:: rational_comparison
   :content-only:

.. _rational_ntheory:

Number theoretic functions
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. versionadded:: 0.8

.. doxygengroup:: rational_ntheory
   :content-only:

.. _rational_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. doxygengroup:: rational_exponentiation
   :content-only:

.. _rational_io:

Input/Output
~~~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> std::ostream &mppp::operator<<(std::ostream &os, const mppp::rational<SSize> &q)

   Stream insertion operator.

   This function will direct to the output stream *os* the input :cpp:class:`~mppp::rational` *q*.

   :param os: the output stream.
   :param q: the input :cpp:class:`~mppp::rational`.

   :return: a reference to *os*.

   :exception std\:\:overflow_error: in case of (unlikely) overflow errors.
   :exception unspecified: any exception raised by the public interface of ``std::ostream`` or by memory allocation errors.

.. _rational_other:

Other
~~~~~

.. doxygengroup:: rational_other
   :content-only:

.. _rational_operators:

Mathematical operators
----------------------

Overloaded operators are provided for convenience. Their interface is generic,
and their implementation
is typically built on top of basic :ref:`functions <rational_functions>`.

.. doxygengroup:: rational_operators
   :content-only:

.. _rational_std_specialisations:

Standard library specialisations
--------------------------------

.. cpp:class:: template <std::size_t SSize> std::hash<mppp::rational<SSize>>

   Specialisation of ``std::hash`` for :cpp:class:`mppp::rational`.

   .. cpp:type:: public argument_type = mppp::rational<SSize>
   .. cpp:type:: public result_type = std::size_t

   .. note::

      The :cpp:type:`argument_type` and :cpp:type:`result_type` type aliases are defined only until C++14.

   .. cpp:function:: public std::size_t operator()(const mppp::rational<SSize> &q) const

      :param q: the input :cpp:class:`mppp::rational`.

      :return: a hash value for *q*.

.. _rational_literals:

User-defined literals
---------------------

.. versionadded:: 0.19

.. cpp:function:: template <char... Chars> mppp::rational<1> mppp::literals::operator"" _q1()
.. cpp:function:: template <char... Chars> mppp::rational<2> mppp::literals::operator"" _q2()
.. cpp:function:: template <char... Chars> mppp::rational<3> mppp::literals::operator"" _q3()

   User-defined rational literals.

   These numeric literal operator templates can be used to construct
   :cpp:class:`~mppp::rational` instances with, respectively, 1, 2 and 3
   limbs of static storage. Literals in binary, octal, decimal and
   hexadecimal format are supported.

   Note that only integral values (i.e., rationals with unitary denominator)
   can be constructed via these literals.

   .. seealso::

      https://en.cppreference.com/w/cpp/language/integer_literal

   :exception std\:\:invalid_argument: if the input sequence of characters is not
     a valid integer literal (as defined by the C++ standard).
