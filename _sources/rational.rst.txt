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

   .. cpp:function:: template <rational_cvr_interoperable<SSize> T> explicit rational(T &&x)

      Generic constructor.

      .. note::

         This constructor is not ``explicit`` if ``T`` satisfies :cpp:concept:`rational_integral_interoperable`.

      This constructor will initialize a rational with the value *x*. The construction will fail if either:

      * *x* is a non-finite floating-point value, or,
      * *x* is a complex value whose imaginary part is not zero
        or whose real part is not finite.

      :param x: the value that will be used to initialize ``this``.

      :exception std\:\:domain_error: if *x* is a non-finite floating-point value, or
        a complex value with non-zero imaginary part or non-finite real part.

   .. cpp:function:: template <rational_cvr_integral_interoperable<SSize> T, rational_cvr_integral_interoperable<SSize> U> explicit rational(T &&n, U &&d, bool make_canonical = true)

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

   .. cpp:function:: template <string_type T> explicit rational(const T &s, int base = 10)

      Constructor from string.

      This constructor will initialize ``this`` from the :cpp:concept:`~mppp::string_type` *s*, which must represent
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

   .. cpp:function:: template <rational_cvr_interoperable<SSize> T> rational &operator=(T &&x)

      Generic assignment operator.

      This operator will assign *x* to ``this``.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the generic constructor.

   .. cpp:function:: rational &operator=(const real128 &x)
   .. cpp:function:: rational &operator=(const real &x)
   .. cpp:function:: rational &operator=(const complex128 &x)
   .. cpp:function:: rational &operator=(const complex &x)

      .. note::

         The :cpp:class:`~mppp::real128` and :cpp:class:`~mppp::complex128`
         overloads are available only if mp++ was configured with the ``MPPP_WITH_QUADMATH``
         option enabled. The :cpp:class:`~mppp::real` overload
         is available only if mp++ was configured with the ``MPPP_WITH_MPFR`` option enabled.
         The :cpp:class:`~mppp::complex` overload
         is available only if mp++ was configured with the ``MPPP_WITH_MPC`` option enabled.

      .. versionadded:: 0.20

      Assignment operators from other mp++ classes.

      These operators are formally equivalent to converting *x* to
      :cpp:class:`~mppp::rational` and then move-assigning the result
      to ``this``.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception raised by the conversion of *x*
        to :cpp:class:`~mppp::rational`.

   .. cpp:function:: template <string_type T> rational &operator=(const T &s)

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

   .. cpp:function:: std::string to_string(int base = 10) const

      Convert to string.

      This operator will return a string representation of ``this`` in base *base*.
      The string format consists of the numerator, followed by the division operator ``/`` and the
      denominator, but only if the denominator is not unitary. Otherwise, only the numerator will be
      represented in the returned string.

      :param base: the desired base for the string representation.

      :return: a string representation for ``this``.

      :exception unspecified: any exception thrown by :cpp:func:`mppp::integer::to_string()`.

   .. cpp:function:: template <rational_interoperable<SSize> T> explicit operator T() const

      Generic conversion operator.

      This operator will convert ``this`` to ``T``.
      Conversion to ``bool`` yields ``false`` if ``this`` is zero,
      ``true`` otherwise. Conversion to other integral types and to :cpp:type:`~mppp::rational::int_t`
      yields the result of the truncated division of the numerator by the denominator, if representable by the target
      type. Conversion to floating-point and complex types might yield inexact values and infinities.

      :return: ``this`` converted to the target type.

      :exception std\:\:overflow_error: if the target type is an integral type and the
        value of ``this`` overflows its range.

   .. cpp:function:: template <rational_interoperable<SSize> T> bool get(T &rop) const

      Generic conversion member function.

      This member function, similarly to the conversion operator, will convert ``this`` to
      ``T``, storing the result of the conversion into *rop*. Differently
      from the conversion operator, this member function does not raise any exception: if the conversion is successful,
      the member function will return ``true``, otherwise the member function will return ``false``. If the conversion
      fails, *rop* will not be altered.

      :param rop: the variable which will store the result of the conversion.

      :return: ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail only if ``T`` is
         an integral C++ type which cannot represent the truncated value of ``this``.

   .. cpp:function:: const int_t &get_num() const
   .. cpp:function:: const int_t &get_den() const
   .. cpp:function:: int_t &_get_num()
   .. cpp:function:: int_t &_get_den()

      Getters for numerator and denominator.

      .. warning::

         It is the user's responsibility to ensure that, after changing the numerator
         or denominator via one of the mutable getters, the rational is kept in canonical form.

      :return: a (const) reference to the numerator or denominator.

   .. cpp:function:: rational &canonicalise()

      Canonicalise.

      This member function will put ``this`` in canonical form. In particular, this member function
      will make sure that:

      * the numerator and denominator are coprime (dividing them by their GCD,
        if necessary),
      * the denominator is strictly positive.

      In general, it is not necessary to call explicitly this member function, as the public
      API of :cpp:class:`~mppp::rational` ensures that rationals are kept in canonical
      form. Calling this member function, however, might be necessary if the numerator and/or denominator
      are modified manually, or when constructing/assigning from non-canonical :cpp:type:`mpq_t`
      values.

      :return: a reference to ``this``.

   .. cpp:function:: bool is_canonical() const

      Check canonical form.

      :return: ``true`` if ``this`` is the canonical form for rational numbers, ``false`` otherwise.

   .. cpp:function:: int sgn() const

      Sign function.

      :return: 0 if ``this`` is zero, 1 if ``this`` is positive, -1 if ``this`` is negation.

   .. cpp:function:: rational &neg()

      Negate in-place.

      This function will set ``this`` to its negative.

      :return: a reference to ``this``.

   .. cpp:function:: rational &abs()

      In-place absolute value.

      This function will set ``this`` to its absolute value.

      :return: a reference to ``this``.

   .. cpp:function:: rational &inv()

      Invert in-place.

      This function will set ``this`` to its inverse.

      :return: a reference to ``this``.

      :exception zero_division_error: if ``this`` is zero.

   .. cpp:function:: bool is_zero() const
   .. cpp:function:: bool is_one() const
   .. cpp:function:: bool is_negative_one() const

      Test for special values.

      :return: ``true`` if ``this`` is, respectively, :math:`0`, :math:`1` or :math:`-1`,
        ``false`` otherwise.

Types
-----

.. cpp:type:: mpq_t

   This is the type used by the GMP library to represent multiprecision rationals.
   It is defined as an array of size 1 of an unspecified structure.

   .. seealso::

      https://gmplib.org/manual/Nomenclature-and-Types.html#Nomenclature-and-Types

Concepts
--------

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::rational_interoperable

   This concept is satisfied if the type ``T`` can interoperate with a :cpp:class:`~mppp::rational`
   with static size ``SSize``. Specifically, this concept will be ``true`` if ``T`` satisfies
   :cpp:concept:`~mppp::integer_cpp_arithmetic` or :cpp:concept:`~mppp::integer_cpp_complex`,
   or it is an :cpp:class:`~mppp::integer` with static size ``SSize``.

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::rational_cvr_interoperable

   This concept is satisfied if the type ``T``, after the removal of reference and cv qualifiers,
   satisfies :cpp:concept:`~mppp::rational_interoperable`.

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::rational_integral_interoperable

   This concept is satisfied if either:

   * ``T`` satisfies :cpp:concept:`~mppp::cpp_integral`, or
   * ``T`` is an :cpp:class:`~mppp::integer` with static size ``SSize``.

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::rational_cvr_integral_interoperable

   This concept is satisfied if the type ``T``, after the removal of reference and cv qualifiers,
   satisfies :cpp:concept:`~mppp::rational_integral_interoperable`.

.. cpp:concept:: template <typename T, typename U> mppp::rational_op_types

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <rational_operators>` and :ref:`functions <rational_functions>`
   involving :cpp:class:`~mppp::rational`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::rational` with the same static size ``SSize``, or
   * one type is a :cpp:class:`~mppp::rational` and the other is a :cpp:concept:`~mppp::rational_interoperable`
     type.

.. cpp:concept:: template <typename T, typename U> mppp::rational_real_op_types

   This concept will be ``true`` if:

   * ``T`` and ``U`` satisfy :cpp:concept:`~mppp::rational_op_types`, and
   * neither ``T`` nor ``U`` satisfy :cpp:concept:`~mppp::integer_cpp_complex`.

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

.. cpp:function:: template <std::size_t SSize, mppp::rational_interoperable<SSize> T> bool mppp::get(T &rop, const mppp::rational<SSize> &q)

   Generic conversion function.

   This function will convert the input :cpp:class:`~mppp::rational` *q* to a
   :cpp:concept:`~mppp::rational_interoperable` type, storing the result of the conversion into *rop*.
   If the conversion is successful, the function
   will return ``true``, otherwise the function will return ``false``. If the conversion fails, *rop* will
   not be altered.

   :param rop: the variable which will store the result of the conversion.
   :param q: the input argument.

   :return: ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail only if ``T`` is
     an integral C++ type which cannot represent the truncated value of *q*.

.. _rational_arithmetic:

Arithmetic
~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> &mppp::add(mppp::rational<SSize> &rop, const mppp::rational<SSize> &x, const mppp::rational<SSize> &y)
.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> &mppp::sub(mppp::rational<SSize> &rop, const mppp::rational<SSize> &x, const mppp::rational<SSize> &y)
.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> &mppp::mul(mppp::rational<SSize> &rop, const mppp::rational<SSize> &x, const mppp::rational<SSize> &y)
.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> &mppp::div(mppp::rational<SSize> &rop, const mppp::rational<SSize> &x, const mppp::rational<SSize> &y)

   Ternary arithmetic primitives.

   These functions will set *rop* to, respectively:

   * :math:`x + y`,
   * :math:`x - y`,
   * :math:`x \times y`,
   * :math:`\frac{x}{y}`.

   :param rop: the return value.
   :param x: the first operand.
   :param y: the second operand.

   :return: a reference to *rop*.

   :exception mppp::zero_division_error: if, in a division operation, *y* is zero.

.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> &mppp::neg(mppp::rational<SSize> &rop, const mppp::rational<SSize> &x)
.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> &mppp::abs(mppp::rational<SSize> &rop, const mppp::rational<SSize> &x)

   Binary negation and absolute value.

   These functions will set *rop* to, respectively, :math:`-x` and :math:`\left| x \right|`.

   :param rop: the return value.
   :param x: the input value.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> mppp::neg(const mppp::rational<SSize> &x)
.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> mppp::abs(const mppp::rational<SSize> &x)

   Unary negation and absolute value.

   :param x: the input value.

   :return: :math:`-x` and :math:`\left| x \right|` respectively.

.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> &mppp::inv(mppp::rational<SSize> &rop, const mppp::rational<SSize> &x)

   Binary inversion.

   This function will set *rop* to :math:`x^{-1}`.

   :param rop: the return value.
   :param x: the input value.

   :return: a reference to *rop*.

   :exception unspecified: any exception thrown by :cpp:func:`mppp::rational::inv()`.

.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> mppp::inv(const mppp::rational<SSize> &x)

   Unary inversion.

   :param x: the input value.

   :return: :math:`x^{-1}`.

   :exception unspecified: any exception thrown by :cpp:func:`mppp::rational::inv()`.

.. _rational_comparison:

Comparison
~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> int mppp::cmp(const mppp::rational<SSize> &x, const mppp::rational<SSize> &y)
.. cpp:function:: template <std::size_t SSize> int mppp::cmp(const mppp::rational<SSize> &x, const mppp::integer<SSize> &y)
.. cpp:function:: template <std::size_t SSize> int mppp::cmp(const mppp::integer<SSize> &x, const mppp::rational<SSize> &y)

   Three-way comparisons.

   These functions will return 0 if :math:`x=y`, a negative value if :math:`x<y`
   and a positive value if :math:`x>y`.

   :param x: the first operand.
   :param y: the second operand.

   :return: the result of the comparison.

.. cpp:function:: template <std::size_t SSize> int mppp::sgn(const mppp::rational<SSize> &q)

   Sign function.

   :param q: the input argument.

   :return: 0 if *q* is zero, 1 if *q* is positive, -1 if *q* is negative.

.. cpp:function:: template <std::size_t SSize> bool mppp::is_zero(const mppp::rational<SSize> &q)
.. cpp:function:: template <std::size_t SSize> bool mppp::is_one(const mppp::rational<SSize> &q)
.. cpp:function:: template <std::size_t SSize> bool mppp::is_negative_one(const mppp::rational<SSize> &q)

   Test for special values.

   :param q: the input argument.

   :return: ``true`` if, respectively, :math:`q=0`, :math:`q=1` or :math:`q=-1`, ``false``
     otherwise.

.. _rational_ntheory:

Number theoretic functions
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. versionadded:: 0.8

.. cpp:function:: template <std::size_t SSize, mppp::rational_integral_interoperable<SSize> T> mppp::rational<SSize> mppp::binomial(const mppp::rational<SSize> &x, const T &y)

   Binomial coefficient.

   This function will compute the binomial coefficient :math:`{x \choose y}`. If *x*
   represents an integral value, the calculation is forwarded to the implementation of
   the binomial coefficient for :cpp:class:`~mppp::integer`. Otherwise, an implementation
   based on the falling factorial is employed.

   :param x: the top value.
   :param y: the bottom value.

   :return: :math:`{x \choose y}`.

   :exception unspecified: any exception thrown by the implementation of
     the binomial coefficient for :cpp:class:`~mppp::integer`.

.. _rational_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> auto mppp::pow(const T &x, const U &y)

   Exponentiation.

   This function will return :math:`x^y`. If one of the arguments
   is a floating-point or complex value, then the result will be computed via ``std::pow()`` and it will also be a
   floating-point or complex value. Otherwise, the result will be a :cpp:class:`~mppp::rational`.

   When floating-point and complex types are not involved, the implementation is based on the integral exponentiation
   of numerator and denominator. Thus, if *y* is a rational value, the exponentiation will be successful
   only in a few special cases (e.g., unitary base, zero exponent, etc.).

   :param x: the base.
   :param y: the exponent.

   :return: :math:`x^y`.

   :exception mppp\:\:zero_division_error: if floating-point or complex types are not involved, and
     *x* is zero and *y* negative.
   :exception std\:\:domain_error: if floating-point or complex types are not involved and *y* is a rational value (except
     in a handful of special cases).

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

.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> &mppp::canonicalise(mppp::rational<SSize> &rop)

   Canonicalise.

   This function will put *rop* in canonical form. Internally, this function will employ
   :cpp:func:`mppp::rational::canonicalise()`.

   :param rop: the rational that will be canonicalised.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> std::size_t mppp::hash(const mppp::rational<SSize> &q)

   Hash value.

   This function will return a hash value for *q*.

   A :ref:`specialisation <rational_std_specialisations>` of the standard ``std::hash`` functor is also provided, so
   that it is possible to use :cpp:class:`~mppp::rational` in standard unordered associative containers out of the box.

   :param q: the rational whose hash value will be computed.

   :return: a hash value for *q*.

.. _rational_operators:

Mathematical operators
----------------------

Overloaded operators are provided for convenience. Their interface is generic,
and their implementation
is typically built on top of basic :ref:`functions <rational_functions>`.

.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> mppp::operator+(const mppp::rational<SSize> &q)
.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> mppp::operator-(const mppp::rational<SSize> &q)

   Identity and negation operators.

   :param q: the input value.

   :return: :math:`q` and :math:`-q` respectively.

.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> auto mppp::operator+(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> auto mppp::operator-(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> auto mppp::operator*(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> auto mppp::operator/(const T &x, const U &y)

   Binary arithmetic operators.

   These operators will return, respectively:

   * :math:`x+y`,
   * :math:`x-y`,
   * :math:`x \times y`,
   * :math:`\frac{x}{y}`.

   The return type of these operators is determined as follows:

   * if the non-:cpp:class:`~mppp::rational` argument is a floating-point or complex value, then the
     type of the result is floating-point or complex; otherwise,
   * the type of the result is :cpp:class:`~mppp::rational`.

   :param x: the first operand.
   :param y: the second operand.

   :return: the result of the operation.

   :exception mppp\:\:zero_division_error: if, in a division not involving floating-point or complex values,
     *y* is zero.

.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> auto mppp::operator+=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> auto mppp::operator-=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> auto mppp::operator*=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> auto mppp::operator/=(T &x, const U &y)

   In-place arithmetic operators.

   These operators will set *x* to, respectively:

   * :math:`x+y`,
   * :math:`x-y`,
   * :math:`x \times y`,
   * :math:`\frac{x}{y}`.

   :param x: the first operand.
   :param y: the second operand.

   :return: a reference to *x*.

   :exception mppp\:\:zero_division_error: if, in a division not involving floating-point or complex values,
     *y* is zero.
   :exception unspecified: any exception thrown by the assignment/conversion operators
     of :cpp:class:`~mppp::rational`.

.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> &mppp::operator++(mppp::rational<SSize> &q)
.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> &mppp::operator--(mppp::rational<SSize> &q)

   Prefix increment/decrement.

   :param q: the input argument.

   :return: a reference to *q* after the increment/decrement.

.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> mppp::operator++(mppp::rational<SSize> &q, int)
.. cpp:function:: template <std::size_t SSize> mppp::rational<SSize> mppp::operator--(mppp::rational<SSize> &q, int)

   Suffix increment/decrement.

   :param q: the input argument.

   :return: a copy of *q* before the increment/decrement.

.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> bool mppp::operator==(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> bool mppp::operator!=(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> bool mppp::operator<(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> bool mppp::operator<=(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> bool mppp::operator>(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::rational_op_types<T> U> bool mppp::operator>=(const T &op1, const U &op2)

   Binary comparison operators.

   :param op1: first argument.
   :param op2: second argument.

   :return: the result of the comparison.

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
