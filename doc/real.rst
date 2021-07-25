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

   :cpp:class:`~mppp::real` aims to behave like a floating-point C++ type whose precision is a runtime property
   of the class instances rather than a compile-time property of the type. Because of this, the way precision
   is handled in :cpp:class:`~mppp::real` differs from the way it is managed in MPFR. The most important difference
   is that in operations involving :cpp:class:`~mppp::real` the precision of the result is usually determined
   by the precision of the operands, whereas in MPFR the precision of the operation is determined by the precision
   of the return value (which is always passed as the first function parameter in the MPFR API). For instance,
   in the following code,

   .. code-block:: c++

      auto x = real{5, 200} + real{6, 150};

   the first operand has a value of 5 and precision of 200 bits, while the second operand has a value of 6 and precision
   150 bits. The precision of the result ``x`` will be
   the maximum precision among the two operands, that is, 200 bits.

   The precision of a :cpp:class:`~mppp::real` can be set at construction, or it can be changed later via functions
   such as :cpp:func:`mppp::real::set_prec()`, :cpp:func:`mppp::real::prec_round()`, etc. By default,
   the precision of a :cpp:class:`~mppp::real` is automatically deduced upon construction following a set of heuristics
   aimed at ensuring that the constructed :cpp:class:`~mppp::real` preserves the value used for initialisation,
   if possible.
   For instance, by default the construction of a :cpp:class:`~mppp::real` from a 32 bit integer will yield a
   :cpp:class:`~mppp::real` with a precision of 32 bits. This behaviour can be altered by specifying explicitly
   the desired precision value.

   Most of the functionality is exposed via plain :ref:`functions <real_functions>`, with the
   general convention that the functions are named after the corresponding MPFR functions minus the leading ``mpfr_``
   prefix. For instance, the MPFR call

   .. code-block:: c++

      mpfr_add(rop, a, b, MPFR_RNDN);

   that writes the result of ``a + b``, rounded to nearest, into ``rop``, becomes simply

   .. code-block:: c++

      add(rop, a, b);

   where the ``add()`` function is resolved via argument-dependent lookup. Function calls with overlapping arguments
   are allowed, unless noted otherwise. Unless otherwise specified, the :cpp:class:`~mppp::real` API always
   rounds to nearest (that is, the ``MPFR_RNDN`` rounding mode is used).

   Various :ref:`overloaded operators <real_operators>` are provided. The arithmetic operators always return
   a :cpp:class:`~mppp::real` result. Alternative comparison functions
   treating NaNs specially are provided for use in the C++ standard library (and wherever strict weak ordering relations
   are needed).

   Member functions are provided to access directly the internal :cpp:type:`mpfr_t` instance (see
   :cpp:func:`mppp::real::get_mpfr_t()` and :cpp:func:`mppp::real::_get_mpfr_t()`), so that
   it is possible to use transparently the MPFR API with :cpp:class:`~mppp::real` objects.

   The :cpp:class:`~mppp::real` class supports a simple binary serialisation API, through member functions
   such as :cpp:func:`~mppp::real::binary_save()` and :cpp:func:`~mppp::real::binary_load()`, and the
   corresponding :ref:`free function overloads <real_s11n>`.

   A :ref:`tutorial <tutorial_real>` showcasing various features of :cpp:class:`~mppp::real`
   is available.

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
   .. cpp:function:: explicit real(real &&other, mpfr_prec_t p)

      Copy/move constructors with custom precision.

      These constructors will set ``this`` to the value of *other* with precision *p*. If *p*
      is smaller than the precision of *other*, a rounding operation will be performed,
      otherwise the value will be copied exactly.

      After move construction, the only valid operations on *other* are
      destruction, copy/move assignment and the invocation of the :cpp:func:`~mppp::real::is_valid()`
      member function. After re-assignment, *other* can be used normally again.

      .. versionadded:: 0.20

         The move overload.

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

      If *k* is not one of :cpp:enumerator:`~mppp::real_kind::nan`,
      :cpp:enumerator:`~mppp::real_kind::inf` or
      :cpp:enumerator:`~mppp::real_kind::zero`, an error will be raised.

      :param k: the desired special value.
      :param sign: the desired sign for ``this``.
      :param p: the desired precision for ``this``.

      :exception std\:\:invalid_argument: if *p* is outside the range established by
        :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`,
        or *k* is an invalid enumerator.

   .. cpp:function:: template <std::size_t SSize> explicit real(const integer<SSize> &n, mpfr_exp_t e, mpfr_prec_t p)
   .. cpp:function:: explicit real(unsigned long n, mpfr_exp_t e, mpfr_prec_t p)
   .. cpp:function:: explicit real(long n, mpfr_exp_t e, mpfr_prec_t p)

      .. versionadded:: 0.20

      Constructors from an integral multiple of a power of two.

      These constructors will set ``this`` to :math:`n\times 2^e` with a precision of *p*.

      :param n: the integral multiple.
      :param e: the power of 2.
      :param p: the desired precision.

      :exception std\:\:invalid_argument: if *p* is outside the range established by
        :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`.

   .. cpp:function:: template <real_interoperable T> real(const T &x)
   .. cpp:function:: template <real_interoperable T> explicit real(const T &x, mpfr_prec_t p)

      Generic constructors.

      The generic constructors will set ``this`` to the value of *x*.

      The variant with the *p* argument will set the precision of ``this``
      exactly to *p*.

      The variant without the *p* argument will set the
      precision of ``this`` according to the following
      heuristics:

      * if *x* is an integral C++ type ``I``, then the precision is set to the bit width of ``I``;
      * if *x* is a floating-point C++ type ``F``, then the precision is set to the number of binary digits
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

   .. cpp:function:: template <cpp_complex T> explicit real(const T &c)
   .. cpp:function:: template <cpp_complex T> explicit real(const T &c, mpfr_prec_t p)

      .. versionadded:: 0.20

      Constructors from complex C++ types.

      These constructors will set ``this`` to the real part of *c*. If the imaginary part
      of *c* is not zero, an error will be raised.

      The precision of ``this`` will be set exactly to *p*, if provided. Otherwise, the precision
      will be set following the same heuristics explained in the generic constructor for the
      real-valued floating-point type underlying ``T``.

      :param c: the construction argument.
      :param p: the desired precision.

      :exception std\:\:domain_error: if the imaginary part of *c* is not zero.
      :exception std\:\:invalid_argument: if *p* is outside the range established by
        :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`.

   .. cpp:function:: template <string_type T> explicit real(const T &s, int base, mpfr_prec_t p)
   .. cpp:function:: template <string_type T> explicit real(const T &s, mpfr_prec_t p)

      Constructors from string, base and precision.

      The first constructor will set ``this`` to the value represented by the :cpp:concept:`~mppp::string_type` *s*, which
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

         * *base* is not zero and not in the :math:`\left[ 2,62 \right]` range,
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

      Constructor from an :cpp:type:`mpfr_t`.

      This constructor will initialise ``this`` with an exact deep copy of *x*.

      .. warning::

         It is the user's responsibility to ensure that *x* has been correctly initialised
         with a precision within the bounds established by :cpp:func:`mppp::real_prec_min()`
         and :cpp:func:`mppp::real_prec_max()`.

      :param x: the :cpp:type:`mpfr_t` that will be deep-copied.

   .. cpp:function:: explicit real(mpfr_t &&x)

      Move constructor from an :cpp:type:`mpfr_t`.

      This constructor will initialise ``this`` with a shallow copy of *x*.

      .. warning::

         It is the user's responsibility to ensure that *x* has been correctly initialised
         with a precision within the bounds established by :cpp:func:`mppp::real_prec_min()`
         and :cpp:func:`mppp::real_prec_max()`.

         Additionally, the user must ensure that, after construction, ``mpfr_clear()`` is never
         called on *x*: the resources previously owned by *x* are now owned by ``this``, which
         will take care of releasing them when the destructor is called.

      .. note::

         Due to a compiler bug, this constructor is not available on Microsoft Visual Studio.

      :param x: the :cpp:type:`mpfr_t` that will be moved.

   .. cpp:function:: ~real()

      Destructor.

      The destructor will run sanity checks in debug mode.

   .. cpp:function:: real &operator=(const real &other)
   .. cpp:function:: real &operator=(real &&other) noexcept

      Copy and move assignment operators.

      :param other: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: template <real_interoperable T> real &operator=(const T &x)

      The generic assignment operator will set ``this`` to the value of *x*.

      The precision of ``this`` will be set according to the same
      heuristics described in the generic constructor.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception std\:\:overflow_error: if an overflow occurs in the computation of
        the automatically-deduced precision.

   .. cpp:function:: template <cpp_complex T> real &operator=(const T &c)

      .. versionadded:: 0.20

      Assignment from complex C++ types.

      This operator will first attempt to convert *c* to :cpp:class:`~mppp::real`,
      and it will then assign the result of the conversion to ``this``.

      :param c: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the conversion of *c*
        to :cpp:class:`~mppp::real`.

   .. cpp:function:: real &operator=(const complex128 &x)
   .. cpp:function:: real &operator=(const complex &x)

      .. note::

         The :cpp:class:`~mppp::complex128` overload is available only if mp++ was configured with the
         ``MPPP_WITH_QUADMATH`` option enabled. The :cpp:class:`~mppp::complex` overload
         is available only if mp++ was configured with the ``MPPP_WITH_MPC`` option enabled.

      .. versionadded:: 0.20

      Assignment operators from other mp++ classes.

      These operators are formally equivalent to converting *x* to
      :cpp:class:`~mppp::real` and then move-assigning the result
      to ``this``.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception raised by the conversion of *x*
        to :cpp:class:`~mppp::real`.

   .. cpp:function:: real &operator=(const mpfr_t x)

      Copy assignment from :cpp:type:`mpfr_t`.

      This operator will set ``this`` to a deep copy of *x*.

      .. warning::

         It is the user's responsibility to ensure that *x* has been correctly initialised
         with a precision within the bounds established by :cpp:func:`mppp::real_prec_min()`
         and :cpp:func:`mppp::real_prec_max()`.

      :param x: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: real &operator=(mpfr_t &&x)

      Move assignment from :cpp:type:`mpfr_t`.

      This operator will set ``this`` to a shallow copy of *x*.

      .. warning::

         It is the user's responsibility to ensure that *x* has been correctly initialised
         with a precision within the bounds established by :cpp:func:`mppp::real_prec_min()`
         and :cpp:func:`mppp::real_prec_max()`.

         Additionally, the user must ensure that, after the assignment, ``mpfr_clear()`` is never
         called on *x*: the resources previously owned by *x* are now owned by ``this``, which
         will take care of releasing them when the destructor is called.

      .. note::

         Due to a compiler bug, this operator is not available on Microsoft Visual Studio.

      :param x: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: bool is_valid() const noexcept

      Check validity.

      A :cpp:class:`~mppp::real` becomes invalid after it is used
      as an argument to the move constructor.

      :return: ``true`` if ``this`` is valid, ``false`` otherwise.

   .. cpp:function:: real &set(const real &other)

      Set to another :cpp:class:`~mppp::real`.

      This member function will set ``this`` to the value of *other*. Contrary to the copy assignment operator,
      the precision of the assignment is dictated by the precision of ``this``, rather than
      the precision of *other*. Consequently, the precision of ``this`` will not be altered by the
      assignment, and a rounding might occur, depending on the values
      and the precisions of the operands.

      This function is a thin wrapper around the ``mpfr_set()`` assignment function from the MPFR API.

      .. seealso::

         https://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions

      :param other: the value to which ``this`` will be set.

      :return: a reference to ``this``.

   .. cpp:function:: template <real_interoperable T> real &set(const T &x)

      Generic setter.

      This member function will set ``this`` to the value of *x*. Contrary to the generic assignment operator,
      the precision of the assignment is dictated by the precision of ``this``, rather than
      being deduced from the type and value of *x*. Consequently, the precision of ``this`` will not be altered
      by the assignment, and a rounding might occur, depending on the operands.

      This function is a thin wrapper around various ``mpfr_set_*()``
      assignment functions from the MPFR API.

      .. seealso::

         https://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions

      :param x: the value to which ``this`` will be set.

      :return: a reference to ``this``.

   .. cpp:function:: template <cpp_complex T> real &set(const T &c)

      .. versionadded:: 0.20

      Setter to complex C++ types.

      This member function will set ``this`` to the value of *c*. Contrary to the generic assignment operator,
      the precision of the assignment is dictated by the precision of ``this``, rather than
      being deduced from the type and value of *c*. Consequently, the precision of ``this`` will not be altered
      by the assignment, and a rounding might occur, depending on the operands.

      If the imaginary part of *c* is not zero, an error will be raised.

      :param c: the value to which ``this`` will be set.

      :return: a reference to ``this``.

      :exception std\:\:domain_error: if the imaginary part of *c* is not zero.

   .. cpp:function:: template <string_type T> real &set(const T &s, int base = 10)

      Setter to string.

      This member function will set ``this`` to the value represented by *s*, which will
      be interpreted as a floating-point number in base *base*. *base* must be either 0 (in which case the base is
      automatically deduced), or a value in the :math:`\left[ 2,62 \right]` range.
      The precision of the assignment is dictated by the
      precision of ``this``, and a rounding might thus occur.

      If *s* is not a valid representation of a floating-point number in base *base*, ``this``
      will be set to NaN and an error will be raised.

      This function is a thin wrapper around the ``mpfr_set_str()`` assignment function from the MPFR API.

      .. seealso::

         https://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions

      :param s: the string to which ``this`` will be set.
      :param base: the base used in the string representation.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if *s* cannot be parsed as a floating-point value, or if the value
        of *base* is invalid.
      :exception unspecified: any exception thrown by memory allocation errors in standard containers.

   .. cpp:function:: real &set(const char *begin, const char *end, int base = 10)

      Set to character range.

      This setter will set ``this`` to the content of the input half-open range,
      which is interpreted as the string representation of a floating-point value in base *base*.

      Internally, the setter will copy the content of the range to a local buffer, add a
      string terminator, and invoke the setter to string.

      :param begin: the start of the input range.
      :param end: the end of the input range.
      :param base: the base used in the string representation.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the setter to string, or by memory
        allocation errors in standard containers.

   .. cpp:function:: real &set(const mpfr_t x)

      Set to an :cpp:type:`mpfr_t`.

      This member function will set ``this`` to the value of *x*. Contrary to the corresponding assignment operator,
      the precision of the assignment is dictated by the precision of ``this``, rather than
      the precision of *x*. Consequently, the precision of ``this`` will not be altered by the
      assignment, and a rounding might occur, depending on the values
      and the precisions of the operands.

      This function is a thin wrapper around the ``mpfr_set()`` assignment function from the MPFR API.

      .. warning::

         It is the user's responsibility to ensure that *x* has been correctly initialised.

      .. seealso::

         https://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions

      :param x: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: real &set_nan()
   .. cpp:function:: real &set_inf(int sign = 0)
   .. cpp:function:: real &set_zero(int sign = 0)

      Set to special values.

      These member functions will set ``this`` to, respectively:

      * NaN (with an unspecified sign bit),
      * infinity (with positive sign if *sign* is nonnegative,
        negative sign otherwise),
      * zero (with positive sign if *sign* is nonnegative,
        negative sign otherwise).

      The precision of ``this`` will not be altered.

      :param sign: the sign of the special value (positive if *sign* is nonnegative,
        negative otherwise).

      :return: a reference to ``this``.

   .. cpp:function:: const mpfr_struct_t *get_mpfr_t() const
   .. cpp:function:: mpfr_struct_t *_get_mpfr_t()

      Getters for the internal :cpp:type:`mpfr_t` instance.

      These member functions will return a const or mutable pointer
      to the internal :cpp:type:`mpfr_t` instance.

      .. warning::

         When using the mutable getter, it is the user's responsibility to ensure
         that the internal MPFR structure is kept in a state which respects the invariants
         of the :cpp:class:`~mppp::real` class. Specifically, the precision value
         must be in the bounds established by :cpp:func:`mppp::real_prec_min()` and
         :cpp:func:`mppp::real_prec_max()`, and upon destruction a :cpp:class:`~mppp::real`
         object must contain a valid :cpp:type:`mpfr_t` object.

      :return: a const or mutable pointer to the internal MPFR structure.

   .. cpp:function:: bool nan_p() const
   .. cpp:function:: bool inf_p() const
   .. cpp:function:: bool number_p() const
   .. cpp:function:: bool zero_p() const
   .. cpp:function:: bool regular_p() const
   .. cpp:function:: bool integer_p() const
   .. cpp:function:: bool is_one() const

      Detect special values.

      These member functions will return ``true`` if ``this`` is, respectively:

      * NaN,
      * an infinity,
      * a finite number,
      * zero,
      * a regular number (i.e., not NaN, infinity or zero),
      * an integral value,
      * one,

      ``false`` otherwise.

      :return: the result of the detection.

   .. cpp:function:: int sgn() const

      Sign detection.

      :return: a positive value if ``this`` is positive, zero if ``this`` is zero,
        a negative value if ``this`` is negative.

      :exception std\:\:domain_error: if ``this`` is NaN.

   .. cpp:function:: bool signbit() const

      Get the sign bit.

      The sign bit is set if ``this`` is negative, -0, or a NaN whose representation has its sign bit set.

      :return: the sign bit of ``this``.

   .. cpp:function:: mpfr_prec_t get_prec() const

      Precision getter.

      :return: the precision of ``this``.

   .. cpp:function:: real &set_prec(mpfr_prec_t p)

      Destructively set the precision

      This member function will set the precision of ``this`` to exactly *p* bits. The value
      of ``this`` will be set to NaN.

      :param p: the desired precision.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if *p* is outside the range established by
        :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`.

   .. cpp:function:: real &prec_round(mpfr_prec_t p)

      Set the precision maintaining the current value.

      This member function will set the precision of ``this`` to exactly *p* bits. If *p*
      is smaller than the current precision of ``this``, a rounding operation will be performed,
      otherwise the current value will be preserved exactly.

      :param p: the desired precision.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if *p* is outside the range established by
        :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`.

   .. cpp:function:: template <real_interoperable T> explicit operator T() const

      Generic conversion operator.

      This operator will convert ``this`` to ``T``. The conversion
      proceeds as follows:

      * if ``T`` is ``bool``, then the conversion returns ``false`` if ``this`` is zero, ``true`` otherwise
        (including if ``this`` is NaN);
      * if ``T`` is an integral C++ type other than ``bool``, the conversion will yield the truncated counterpart
        of ``this`` (i.e., the conversion rounds to zero). The conversion may fail due to overflow or domain errors
        (i.e., when trying to convert non-finite values);
      * if ``T`` if a floating-point C++ type, the conversion calls directly the low-level MPFR functions (e.g.,
        ``mpfr_get_d()``), and might yield infinities for finite input values;
      * if ``T`` is :cpp:class:`~mppp::integer`, the conversion rounds to zero and might fail due to domain errors,
        but it will never overflow;
      * if ``T`` is :cpp:class:`~mppp::rational`, the conversion may fail if ``this`` is not finite or if the
        conversion produces an overflow in the manipulation of the exponent of ``this`` (that is, if
        the absolute value of ``this`` is very large or very small). If the conversion succeeds, it will be exact;
      * if ``T`` is :cpp:class:`~mppp::real128`, the conversion might yield infinities for finite input values.

      :return: ``this`` converted to ``T``.

      :exception std\:\:domain_error: if ``this`` is not finite and the target type cannot represent non-finite numbers.
      :exception std\:\:overflow_error: if the conversion results in overflow.

   .. cpp:function:: template <cpp_complex T> explicit operator T() const

      .. versionadded:: 0.20

      Conversion to complex C++ types.

      The real part of the return value is constructed by converting ``this`` to the value type
      of ``T``. The imaginary part of the return value is set to zero.

      :return: ``this`` converted to ``T``.

   .. cpp:function:: template <real_interoperable T> bool get(T &rop) const

      Generic conversion function.

      This member function, similarly to the conversion operator, will convert ``this`` to
      ``T``, storing the result of the conversion into *rop*. Differently
      from the conversion operator, this function does not raise any exception: if the conversion is successful, the
      function will return ``true``, otherwise the function will return ``false``. If the conversion fails,
      *rop* will not be altered.

      :param rop: the variable which will store the result of the conversion.

      :return: ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail in the ways
        specified in the documentation of the conversion operator.

   .. cpp:function:: template <cpp_complex T> bool get(T &rop) const

      .. versionadded:: 0.20

      Conversion function to complex C++ types.

      The conversion is always successful.

      :param rop: the variable which will store the result of the conversion.

      :return: ``true``.

   .. cpp:function:: std::string to_string(int base = 10) const

      Conversion to string.

      This member function will convert ``this`` to a string representation in base *base*. The returned string is guaranteed
      to produce exactly the original value when used in one of the constructors from string of
      :cpp:class:`~mppp::real` (provided that the original precision and base are used in the construction).

      :param base: the base to be used for the string representation.

      :return: ``this`` converted to a string.

      :exception std\:\:invalid_argument: if *base* is not in the :math:`\left[ 2,62 \right]` range.
      :exception std\:\:runtime_error: if the call to the ``mpfr_get_str()`` function of the MPFR API fails.

   .. cpp:function:: std::size_t get_str_ndigits(int base = 10) const

      .. versionadded:: 0.25

      .. note::

         This function is available from MPFR 4.1 onwards.

      Minimum number of digits necessary for round-tripping.

      This member function will return the minimum number of digits
      necessary to ensure that the current value of ``this``
      can be recovered exactly from
      a string representation in the given *base*.

      :param base: the base to be used for the string representation.

      :return: the minimum number of digits necessary for round-tripping.

      :exception std\:\:invalid_argument: if *base* is not in the :math:`\left[ 2,62 \right]` range.

   .. cpp:function:: real &neg()
   .. cpp:function:: real &abs()

      In-place negation and absolute value.

      :return: a reference to ``this``.

   .. cpp:function:: real &sqr()

      .. versionadded:: 0.19

      Square ``this`` in place.

      :return: a reference to ``this``.

   .. cpp:function:: real &sqrt()
   .. cpp:function:: real &rec_sqrt()
   .. cpp:function:: real &sqrt1pm1()
   .. cpp:function:: real &cbrt()

      .. note::

         The :cpp:func:`~mppp::real::sqrt1pm1()` function is available only if mp++ was
         configured with the ``MPPP_WITH_ARB`` option enabled.

      In-place roots.

      These member functions will set ``this`` to, respectively:

      * :math:`\sqrt{x}`,
      * :math:`\frac{1}{\sqrt{x}}`,
      * :math:`\sqrt{1+x}-1`,
      * :math:`\sqrt[3]{x}`,

      where :math:`x` is the current value of ``this``.

      .. versionadded:: 0.12

         The :cpp:func:`~mppp::real::rec_sqrt()` and
         :cpp:func:`~mppp::real::cbrt()` functions.

      .. versionadded:: 0.19

         The :cpp:func:`~mppp::real::sqrt1pm1()` function.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
        fails because of (unlikely) overflow conditions.

   .. cpp:function:: real &sin()
   .. cpp:function:: real &cos()
   .. cpp:function:: real &tan()
   .. cpp:function:: real &sec()
   .. cpp:function:: real &csc()
   .. cpp:function:: real &cot()
   .. cpp:function:: real &sin_pi()
   .. cpp:function:: real &cos_pi()
   .. cpp:function:: real &tan_pi()
   .. cpp:function:: real &cot_pi()
   .. cpp:function:: real &sinc()
   .. cpp:function:: real &sinc_pi()

      .. note::

         The :cpp:func:`~mppp::real::sin_pi()`, :cpp:func:`~mppp::real::cos_pi()`,
         :cpp:func:`~mppp::real::tan_pi()`, :cpp:func:`~mppp::real::cot_pi()`,
         :cpp:func:`~mppp::real::sinc()` and :cpp:func:`~mppp::real::sinc_pi()`
         functions are available only if mp++ was
         configured with the ``MPPP_WITH_ARB`` option enabled.

      In-place trigonometric functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\sin{x}`,
      * :math:`\cos{x}`,
      * :math:`\tan{x}`,
      * :math:`\sec{x}`,
      * :math:`\csc{x}`,
      * :math:`\cot{x}`,
      * :math:`\sin\left( \pi x \right)`,
      * :math:`\cos\left( \pi x \right)`,
      * :math:`\tan\left( \pi x \right)`,
      * :math:`\cot\left( \pi x \right)`,
      * :math:`\frac{\sin\left( x \right)}{x}`,
      * :math:`\frac{\sin\left( \pi x \right)}{\pi x}`.

      where :math:`x` is the current value of ``this``.

      .. versionadded:: 0.19

         The :cpp:func:`~mppp::real::sin_pi()`, :cpp:func:`~mppp::real::cos_pi()`,
         :cpp:func:`~mppp::real::tan_pi()`, :cpp:func:`~mppp::real::cot_pi()`,
         :cpp:func:`~mppp::real::sinc()` and :cpp:func:`~mppp::real::sinc_pi()`
         functions.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
        fails because of (unlikely) overflow conditions.

   .. cpp:function:: real &acos()
   .. cpp:function:: real &asin()
   .. cpp:function:: real &atan()

      In-place inverse trigonometric functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\arccos{x}`,
      * :math:`\arcsin{x}`,
      * :math:`\arctan{x}`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real &sinh()
   .. cpp:function:: real &cosh()
   .. cpp:function:: real &tanh()
   .. cpp:function:: real &sech()
   .. cpp:function:: real &csch()
   .. cpp:function:: real &coth()

      In-place hyperbolic functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\sinh{x}`,
      * :math:`\cosh{x}`,
      * :math:`\tanh{x}`,
      * :math:`\operatorname{sech}{x}`,
      * :math:`\operatorname{csch}{x}`,
      * :math:`\coth{x}`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real &acosh()
   .. cpp:function:: real &asinh()
   .. cpp:function:: real &atanh()

      In-place inverse hyperbolic functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\operatorname{arccosh}{x}`,
      * :math:`\operatorname{arcsinh}{x}`,
      * :math:`\operatorname{arctanh}{x}`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real &exp()
   .. cpp:function:: real &exp2()
   .. cpp:function:: real &exp10()
   .. cpp:function:: real &expm1()
   .. cpp:function:: real &log()
   .. cpp:function:: real &log2()
   .. cpp:function:: real &log10()
   .. cpp:function:: real &log1p()

      In-place exponentials and logarithms.

      These member functions will set ``this`` to, respectively:

      * :math:`e^x`,
      * :math:`2^x`,
      * :math:`10^x`,
      * :math:`e^x-1`,
      * :math:`\log x`,
      * :math:`\log_2 x`,
      * :math:`\log_{10} x`,
      * :math:`\log\left( 1+x\right)`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real &gamma()
   .. cpp:function:: real &lngamma()
   .. cpp:function:: real &lgamma()
   .. cpp:function:: real &digamma()

      In-place gamma functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\Gamma\left( x \right)`,
      * :math:`\log \Gamma\left( x \right)`,
      * :math:`\log \left|\Gamma\left( x \right)\right|`,
      * :math:`\psi\left( x \right)`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real &j0()
   .. cpp:function:: real &j1()
   .. cpp:function:: real &y0()
   .. cpp:function:: real &y1()

      In-place Bessel functions of the first and second kind.

      These member functions will set ``this`` to, respectively:

      * :math:`J_0\left( x \right)`,
      * :math:`J_1\left( x \right)`,
      * :math:`Y_0\left( x \right)`,
      * :math:`Y_1\left( x \right)`,

      where :math:`x` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: real &eint()
   .. cpp:function:: real &li2()
   .. cpp:function:: real &zeta()
   .. cpp:function:: real &erf()
   .. cpp:function:: real &erfc()
   .. cpp:function:: real &ai()
   .. cpp:function:: real &lambert_w0()
   .. cpp:function:: real &lambert_wm1()

      .. note::

         The :cpp:func:`~mppp::real::lambert_w0()` and :cpp:func:`~mppp::real::lambert_wm1()`
         functions are available only if mp++ was
         configured with the ``MPPP_WITH_ARB`` option enabled.

      Other special functions, in-place variants.

      These member functions will set ``this`` to, respectively:

      * :math:`\operatorname{Ei}\left( x \right)`,
      * :math:`\operatorname{Li}_2\left( x \right)`,
      * :math:`\zeta\left( x \right)`,
      * :math:`\operatorname{erf}\left( x \right)`,
      * :math:`\operatorname{erfc}\left( x \right)`,
      * :math:`\operatorname{Ai}\left( x \right)`,
      * the Lambert W functions :math:`W_0\left( x \right)` and :math:`W_{-1}\left( x \right)`,

      where :math:`x` is the current value of ``this``.

      .. versionadded:: 0.24

         The :cpp:func:`~mppp::real::lambert_w0()` and :cpp:func:`~mppp::real::lambert_wm1()`
         functions.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
        fails because of (unlikely) overflow conditions.

   .. cpp:function:: real &ceil()
   .. cpp:function:: real &floor()
   .. cpp:function:: real &round()
   .. cpp:function:: real &roundeven()
   .. cpp:function:: real &trunc()
   .. cpp:function:: real &frac()

      .. note::

         The ``roundeven()`` function is available from MPFR 4 onwards.

      In-place integer and remainder-related functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\left\lceil x \right\rceil`,
      * :math:`\left\lfloor x \right\rfloor`,
      * *x* rounded to nearest (IEEE ``roundTiesToAway`` mode),
      * *x* rounded to nearest (IEEE ``roundTiesToEven`` mode),
      * :math:`\operatorname{trunc}\left( x \right)`,
      * the fractional part of *x*,

      where :math:`x` is the current value of ``this``.

      .. versionadded:: 0.21

         The ``ceil()``, ``floor()``, ``round()``, ``roundeven()`` and ``frac()``
         functions.

      :return: a reference to ``this``.

      :exception std\:\:domain_error: if ``this`` represents a NaN value.

   .. cpp:function:: std::size_t binary_size() const

      .. versionadded:: 0.22

      Size of the serialised binary representation.

      This member function will return a value representing the number of bytes necessary
      to serialise ``this`` into a memory buffer in binary format via one of the available
      :cpp:func:`~mppp::real::binary_save()` overloads. The returned value
      is platform-dependent.

      :return: the number of bytes needed for the binary serialisation of ``this``.

      :exception std\:\:overflow_error: if the size in limbs of ``this`` is larger than an
        implementation-defined limit.

   .. cpp:function:: std::size_t binary_save(char *dest) const
   .. cpp:function:: std::size_t binary_save(std::vector<char> &dest) const
   .. cpp:function:: template <std::size_t S> std::size_t binary_save(std::array<char, S> &dest) const
   .. cpp:function:: std::size_t binary_save(std::ostream &dest) const

      .. versionadded:: 0.22

      Serialise into a memory buffer or an output stream.

      These member functions will write into *dest* a binary representation of ``this``. The serialised
      representation produced by these member functions can be read back with one of the
      :cpp:func:`~mppp::real::binary_load()` overloads.

      For the first overload, *dest* must point to a memory area whose size is at least equal to the value returned
      by :cpp:func:`~mppp::real::binary_size()`, otherwise the behaviour will be undefined.
      *dest* does not have any special alignment requirements.

      For the second overload, the size of *dest* must be at least equal to the value returned by
      :cpp:func:`~mppp::real::binary_size()`. If that is not the case, *dest* will be resized
      to :cpp:func:`~mppp::real::binary_size()`.

      For the third overload, the size of *dest* must be at least equal to the value returned by
      :cpp:func:`~mppp::real::binary_size()`. If that is not the case, no data
      will be written to *dest* and zero will be returned.

      For the fourth overload, if the serialisation is successful (that is, no stream error state is ever detected
      in *dest* after write
      operations), then the binary size of ``this`` (that is, the number of bytes written into *dest*) will be
      returned. Otherwise, zero will be returned. Note that a return value of zero does not necessarily imply that no
      bytes were written into *dest*, just that an error occurred at some point during the serialisation process.

      .. warning::

         The binary representation produced by these member functions is compiler, platform and architecture
         specific, and it is subject to possible breaking changes in future versions of mp++. Thus,
         it should not be used as an exchange format or for long-term data storage.

      :param dest: the output buffer or stream.

      :return: the number of bytes written into ``dest`` (i.e., the output of :cpp:func:`~mppp::real::binary_size()`,
        if the serialisation was successful).

      :exception std\:\:overflow_error: in case of (unlikely) overflow errors.
      :exception unspecified: any exception thrown by :cpp:func:`~mppp::real::binary_size()`, by memory errors in
        standard containers, or by the public interface of ``std::ostream``.

   .. cpp:function:: std::size_t binary_load(const char *src)
   .. cpp:function:: std::size_t binary_load(const std::vector<char> &src)
   .. cpp:function:: template <std::size_t S> std::size_t binary_load(const std::array<char, S> &src)
   .. cpp:function:: std::size_t binary_load(std::istream &src)

      .. versionadded:: 0.22

      Deserialise from a memory buffer or an input stream.

      These member functions will load into ``this`` the content of the memory buffer or input stream
      *src*, which must contain the serialised representation of a :cpp:class:`~mppp::real`
      produced by one of the :cpp:func:`~mppp::real::binary_save()` overloads.

      For the first overload, *src* does not have any special alignment requirements.

      For the second and third overloads, the serialised representation of the
      :cpp:class:`~mppp::real` must start at the beginning of *src*,
      but it can end before the end of *src*. Data
      past the end of the serialised representation of the :cpp:class:`~mppp::real`
      will be ignored.

      For the fourth overload, the serialised representation of the :cpp:class:`~mppp::real`
      must start at
      the current position of *src*, but *src* can contain other data before and after
      the serialised :cpp:class:`~mppp::real` value. Data
      past the end of the serialised representation of the :cpp:class:`~mppp::real`
      will be ignored. If a stream error state is detected at any point of the deserialisation
      process after a read operation, zero will be returned and ``this`` will not have been modified.
      Note that a return value of zero does not necessarily imply that no
      bytes were read from *src*, just that an error occurred at some point during the serialisation process.

      .. warning::

         Although these member functions perform a few consistency checks on the data in *src*,
         they cannot ensure complete safety against maliciously-crafted data. Users are
         advised to use these member functions only with trusted data.

      :param src: the source memory buffer or stream.

      :return: the number of bytes read from *src* (that is, the output of :cpp:func:`~mppp::real::binary_size()`
        after the deserialisation into ``this`` has successfully completed).

      :exception std\:\:overflow_error: in case of (unlikely) overflow errors.
      :exception std\:\:invalid_argument: if invalid data is detected in *src*.
      :exception unspecified: any exception thrown by memory errors in standard containers,
        the public interface of ``std::istream``, :cpp:func:`~mppp::real::binary_size()`
        or :cpp:func:`~mppp::real::set_prec()`.

Types
-----

.. cpp:type:: mpfr_t

   This is the type used by the MPFR library to represent multiprecision floats.
   It is defined as an array of size 1 of an unspecified structure
   (see :cpp:type:`~mppp::mpfr_struct_t`).

   .. seealso::

      https://www.mpfr.org/mpfr-current/mpfr.html#Nomenclature-and-Types

.. cpp:type:: mppp::mpfr_struct_t = std::remove_extent_t<mpfr_t>

   The C structure used by MPFR to represent arbitrary-precision floats.
   The MPFR type :cpp:type:`mpfr_t` is defined as an array of size 1 of this structure.

.. cpp:type:: mpfr_prec_t

   An integral type defined by the MPFR library, used to represent the precision of :cpp:type:`mpfr_t`
   and (by extension) :cpp:class:`~mppp::real` objects.

.. cpp:type:: mpfr_exp_t

   An integral type defined by the MPFR library, used to represent the exponent of :cpp:type:`mpfr_t`
   and (by extension) :cpp:class:`~mppp::real` objects.

.. cpp:enum-class:: mppp::real_kind : std::underlying_type<mpfr_kind_t>::type

   This scoped enum is used to initialise a :cpp:class:`~mppp::real` with
   one of the three special values NaN, infinity or zero.

   .. cpp:enumerator:: nan = MPFR_NAN_KIND
   .. cpp:enumerator:: inf = MPFR_INF_KIND
   .. cpp:enumerator:: zero = MPFR_ZERO_KIND

.. seealso::

   https://www.mpfr.org/mpfr-current/mpfr.html#Nomenclature-and-Types

Concepts
--------

.. cpp:concept:: template <typename T> mppp::cvr_real

   This concept is satisfied if the type ``T``, after the removal of reference and cv qualifiers,
   is the same as :cpp:class:`mppp::real`.

.. cpp:concept:: template <typename T> mppp::real_interoperable

   This concept is satisfied if the type ``T`` can interoperate with :cpp:class:`~mppp::real`.
   Specifically, this concept will be ``true`` if ``T`` is either:

   * a :cpp:concept:`~mppp::cpp_arithmetic` type, or
   * an :cpp:class:`~mppp::integer`, or
   * a :cpp:class:`~mppp::rational`, or
   * :cpp:class:`~mppp::real128`.

.. cpp:concept:: template <typename... Args> mppp::real_set_args

   This concept is satisfied if the types in the parameter pack ``Args``
   can be used as argument types in one of the :cpp:func:`mppp::real::set()` member function overloads.
   In other words, this concept is satisfied if the expression

   .. code-block:: c++

      r.set(x, y, z, ...);

   is valid (where ``r`` is a non-const :cpp:class:`~mppp::real` and ``x``, ``y``, ``z``, etc. are const
   references to the types in ``Args``).

.. cpp:concept:: template <typename T, typename U> mppp::real_op_types

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <real_operators>` and :ref:`functions <real_functions>`
   involving :cpp:class:`~mppp::real`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` both satisfy :cpp:concept:`~mppp::cvr_real`,
   * one type satisfies :cpp:concept:`~mppp::cvr_real` and the other type, after the removal of reference
     and cv qualifiers, satisfies :cpp:concept:`~mppp::real_interoperable`.

.. cpp:concept:: template <typename T, typename U> mppp::real_in_place_op_types

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic in-place :ref:`operators <real_operators>`
   involving :cpp:class:`~mppp::real`. Specifically, the concept will be ``true`` if
   ``T`` and ``U`` satisfy :cpp:concept:`~mppp::real_op_types` and ``T``, after the removal
   of reference, is not const.

.. cpp:concept:: template <typename T, typename U> mppp::real_eq_op_types

   .. versionadded:: 0.20

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary equality and inequality operators
   involving :cpp:class:`~mppp::real` and other types. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` satisfy :cpp:concept:`~mppp::real_op_types`, or
   * one type is :cpp:class:`~mppp::real` and the other is a :cpp:concept:`~mppp::cpp_complex` type.

.. _real_functions:

Functions
---------

.. _real_prec:

Precision handling
~~~~~~~~~~~~~~~~~~

.. cpp:function:: mpfr_prec_t mppp::get_prec(const mppp::real &r)

   Get the precision of a :cpp:class:`~mppp::real`.

   :param r: the input argument.

   :return: the precision of *r*.

.. cpp:function:: void mppp::set_prec(mppp::real &r, mpfr_prec_t p)
.. cpp:function:: void mppp::prec_round(mppp::real &r, mpfr_prec_t p)

   Set the precision of a :cpp:class:`~mppp::real`.

   The first variant will set the precision of *r* to exactly *p* bits. The value
   of *r* will be set to NaN.

   The second variant will preserve the current value of *r*, performing
   a rounding operation if *p* is less than the current precision of *r*.

   :param r: the input argument.
   :param p: the desired precision.

   :exception unspecified: any exception thrown by :cpp:func:`mppp::real::set_prec()`
     or :cpp:func:`mppp::real::prec_round()`.

.. cpp:function:: constexpr mpfr_prec_t mppp::real_prec_min()
.. cpp:function:: constexpr mpfr_prec_t mppp::real_prec_max()

   Minimum/maximum precisions for a :cpp:class:`~mppp::real`.

   These compile-time constants represent the minimum/maximum valid precisions
   for a :cpp:class:`~mppp::real`. The returned values are guaranteed to be, respectively,
   not less than the ``MPFR_PREC_MIN`` MPFR constant and not greater than
   the ``MPFR_PREC_MAX`` MPFR constant.

   :return: the minimum/maximum valid precisions for a :cpp:class:`~mppp::real`.

.. _real_assignment:

Assignment
~~~~~~~~~~

.. cpp:function:: template <mppp::real_set_args... Args> mppp::real &mppp::set(mppp::real &r, const Args &... args)

   Generic setter.

   This function will use the arguments *args* to set the value of the :cpp:class:`~mppp::real` *r*,
   using one of the available :cpp:func:`mppp::real::set()` overloads. That is,
   the body of this function is equivalent to

   .. code-block:: c++

      return r.set(args...);

   The input arguments must satisfy the :cpp:concept:`mppp::real_set_args` concept.

   :param r: the return value.
   :param args: the arguments that will be passed to :cpp:func:`mppp::real::set()`.

   :return: a reference to *r*.

   :exception unspecified: any exception thrown by the invoked :cpp:func:`mppp::real::set()` overload.

.. cpp:function:: mppp::real &mppp::set_ui_2exp(mppp::real &r, unsigned long n, mpfr_exp_t e)
.. cpp:function:: mppp::real &mppp::set_si_2exp(mppp::real &r, long n, mpfr_exp_t e)
.. cpp:function:: template <std::size_t SSize> mppp::real &mppp::set_z_2exp(mppp::real &r, const mppp::integer<SSize> &n, mpfr_exp_t e)

   Set to :math:`n\times 2^e`.

   These functions will set *r* to :math:`n\times 2^e`. The precision of *r*
   will not be altered. If *n* is zero, the result will be positive zero.

   .. versionadded:: 0.20

      The :cpp:func:`~mppp::set_ui_2exp()` and :cpp:func:`~mppp::set_si_2exp()`
      functions.

   :param r: the return value.
   :param n: the input integer multiplier.
   :param e: the exponent.

   :return: a reference to *r*.

.. cpp:function:: mppp::real &mppp::set_nan(mppp::real &r)
.. cpp:function:: mppp::real &mppp::set_inf(mppp::real &r, int sign = 0)
.. cpp:function:: mppp::real &mppp::set_zero(mppp::real &r, int sign = 0)

   Set to NaN, infinity or zero.

   The precision of *r* will not be altered. When setting to infinity
   or zero, the sign bit will be positive if *sign*
   is nonnegative, negative otherwise. When setting to NaN, the sign
   bit is unspecified.

   :param r: the input argument.
   :param sign: the sign of the infinity or zero to which *r* will be set.

   :return: a reference to *r*.

.. cpp:function:: void mppp::swap(mppp::real &a, mppp::real &b) noexcept

   Swap efficiently *a* and *b*.

   :param a: the first argument.
   :param b: the second argument.

.. _real_conversion:

Conversion
~~~~~~~~~~

.. cpp:function:: template <mppp::real_interoperable T> bool mppp::get(T &rop, const mppp::real &x)

   Generic conversion function.

   This function will convert the input :cpp:class:`~mppp::real` *x* to
   ``T``, storing the result of the conversion into *rop*.
   If the conversion is successful, the function
   will return ``true``, otherwise the function will return ``false``. If the conversion fails, *rop* will
   not be altered.

   :param rop: the variable which will store the result of the conversion.
   :param x: the input argument.

   :return: ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail in the ways
      specified in the documentation of the conversion operator for :cpp:class:`~mppp::real`.

.. cpp:function:: template <mppp::cpp_complex T> bool mppp::get(T &rop, const mppp::real &x)

   .. versionadded:: 0.20

   Conversion to complex C++ types.

   The conversion is always successful.

   :param rop: the variable which will store the result of the conversion.
   :param x: the input argument.

   :return: ``true``.

.. cpp:function:: template <std::size_t SSize> mpfr_exp_t mppp::get_z_2exp(mppp::integer<SSize> &n, const mppp::real &r)

   Extract significand and exponent.

   This function will extract the scaled significand of *r* into *n*, and return the
   exponent *e* such that :math:`r = n\times 2^e`.

   If *r* is not finite, an error will be raised.

   :param n: the :cpp:class:`~mppp::integer` that will contain the scaled significand of *r*.
   :param r: the input argument.

   :return: the exponent *e* such that :math:`r = n\times 2^e`.

   :exception std\:\:domain_error: if *r* is not finite.
   :exception std\:\:overflow_error: if the output exponent is larger than an implementation-defined
     value.

.. cpp:function:: std::size_t mppp::get_str_ndigits(const mppp::real &r, int base = 10)

   .. versionadded:: 0.25

   .. note::

      This function is available from MPFR 4.1 onwards.

   Minimum number of digits necessary for round-tripping.

   This member function will return the minimum number of digits
   necessary to ensure that the current value of *r*
   can be recovered exactly from
   a string representation in the given *base*.

   :param r: the input value.
   :param base: the base to be used for the string representation.

   :return: the minimum number of digits necessary for round-tripping.

   :exception std\:\:invalid_argument: if *base* is not in the :math:`\left[ 2,62 \right]` range.

.. _real_arithmetic:

Arithmetic
~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::add(mppp::real &rop, T &&a, U &&b)
.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::sub(mppp::real &rop, T &&a, U &&b)
.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::mul(mppp::real &rop, T &&a, U &&b)
.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::div(mppp::real &rop, T &&a, U &&b)

   Ternary basic :cpp:class:`~mppp::real` arithmetics.

   These functions will set *rop* to, respectively:

   * :math:`a+b`,
   * :math:`a-b`,
   * :math:`a \times b`,
   * :math:`\frac{a}{b}`.

   The precision of the result will be set to the largest precision among the operands.

   :param rop: the return value.
   :param a: the first operand.
   :param b: the second operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U, mppp::cvr_real V> mppp::real &mppp::fma(mppp::real &rop, T &&a, U &&b, V &&c)
.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U, mppp::cvr_real V> mppp::real &mppp::fms(mppp::real &rop, T &&a, U &&b, V &&c)

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

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U, mppp::cvr_real V> mppp::real mppp::fma(T &&a, U &&b, V &&c)
.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U, mppp::cvr_real V> mppp::real mppp::fms(T &&a, U &&b, V &&c)

   Ternary :cpp:class:`~mppp::real` multiply-add/sub.

   These functions will return, respectively:

   * :math:`a \times b + c`,
   * :math:`a \times b - c`.

   The precision of the result will be the largest precision among the operands.

   :param a: the first operand.
   :param b: the second operand.
   :param c: the third operand.

   :return: :math:`a \times b \pm c`.

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::neg(mppp::real &rop, T &&x)

   Binary :cpp:class:`~mppp::real` negation.

   This function will set *rop* to :math:`-x`. The precision of the result will be
   equal to the precision of *x*.

   :param rop: the return value.
   :param x: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::neg(T &&x)

   Unary :cpp:class:`~mppp::real` negation.

   This function will return :math:`-x`. The precision of the result will be
   equal to the precision of *x*.

   :param x: the operand.

   :return: :math:`-x`.

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::abs(mppp::real &rop, T &&x)

   Binary :cpp:class:`~mppp::real` absolute value.

   This function will set *rop* to :math:`\left| x \right|`. The precision of the result will be
   equal to the precision of *x*.

   :param rop: the return value.
   :param x: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::abs(T &&x)

   Unary :cpp:class:`~mppp::real` absolute value.

   This function will return :math:`\left| x \right|`. The precision of the result will be
   equal to the precision of *x*.

   :param x: the operand.

   :return: :math:`\left| x \right|`.

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::mul_2ui(mppp::real &rop, T &&x, unsigned long n)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::mul_2si(mppp::real &rop, T &&x, long n)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::div_2ui(mppp::real &rop, T &&x, unsigned long n)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::div_2si(mppp::real &rop, T &&x, long n)

   .. versionadded:: 0.19

   Ternary :cpp:class:`~mppp::real` primitives for
   multiplication/division by powers of 2.

   These functions will set *rop* to, respectively:

   * :math:`x \times 2^n` (``mul_2`` variants),
   * :math:`\frac{x}{2^n}` (``div_2`` variants).

   The precision of the result will be equal to the precision of *x*.

   :param rop: the return value.
   :param x: the operand.
   :param n: the power of 2.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::mul_2ui(T &&x, unsigned long n)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::mul_2si(T &&x, long n)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::div_2ui(T &&x, unsigned long n)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::div_2si(T &&x, long n)

   .. versionadded:: 0.19

   Binary :cpp:class:`~mppp::real` primitives for
   multiplication/division by powers of 2.

   These functions will return, respectively:

   * :math:`x \times 2^n` (``mul_2`` variants),
   * :math:`\frac{x}{2^n}` (``div_2`` variants).

   The precision of the result will be equal to the precision of *x*.

   :param x: the operand.
   :param n: the power of 2.

   :return: *x* multiplied/divided by :math:`2^n`.

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::sqr(mppp::real &rop, T &&op)

   .. versionadded:: 0.19

   Binary :cpp:class:`~mppp::real` squaring.

   This function will compute the square of *op* and store it
   into *rop*. The precision of the result will be equal to the precision
   of *op*.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::sqr(T &&r)

   .. versionadded:: 0.19

   Unary :cpp:class:`~mppp::real` squaring.

   This function will compute and return the square of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the operand.

   :return: the square of *r*.

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::dim(mppp::real &rop, T &&x, U &&y)

   .. versionadded:: 0.21

   Ternary positive difference.

   This function will set *rop* to the positive difference of *x* and *y*.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param x: the first operand.
   :param y: the second operand.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::dim(T &&x, U &&y)

   .. versionadded:: 0.21

   Binary positive difference.

   This function will compute and return the positive difference of *x* and *y*.
   The precision of the result will be set to the largest precision among the operands.

   :param x: the first operand.
   :param y: the second operand.

   :return: the positive difference of *x* and *y*.

.. _real_comparison:

Comparison
~~~~~~~~~~

.. cpp:function:: bool mppp::nan_p(const mppp::real &r)
.. cpp:function:: bool mppp::inf_p(const mppp::real &r)
.. cpp:function:: bool mppp::number_p(const mppp::real &r)
.. cpp:function:: bool mppp::zero_p(const mppp::real &r)
.. cpp:function:: bool mppp::regular_p(const mppp::real &r)
.. cpp:function:: bool mppp::integer_p(const mppp::real &r)
.. cpp:function:: bool mppp::is_one(const mppp::real &r)

   Detect special values.

   These functions will return ``true`` if *r* is, respectively:

   * NaN,
   * an infinity,
   * a finite number,
   * zero,
   * a regular number (i.e., not NaN, infinity or zero),
   * an integral value,
   * one,

   ``false`` otherwise.

   :param r: the input argument.

   :return: the result of the detection.

.. cpp:function:: int mppp::sgn(const mppp::real &r)
.. cpp:function:: bool mppp::signbit(const mppp::real &r)

   Detect sign or sign bit.

   The sign is returned as a positive value if *r* is positive,
   zero if *r* is zero, a negative value if *r* is negative.

   The sign bit is ``true`` if *r* is negative, -0, or a NaN whose representation
   has its sign bit set, ``false`` otherwise.

   :param r: the input argument.

   :return: the sign or sign bit of *r*.

   :exception unspecified: any exception raised by :cpp:func:`mppp::real::sgn()`.

.. cpp:function:: int mppp::cmp(const mppp::real &a, const mppp::real &b)

   Three-way comparison.

   This function will compare *a* and *b*, returning:

   * zero if :math:`a=b`,
   * a negative value if :math:`a<b`,
   * a positive value if :math:`a>b`.

   If at least one NaN value is involved in the comparison, an error will be raised.

   This function is useful to distinguish the three possible cases. The comparison operators
   are recommended instead if it is needed to distinguish only two cases.

   :param a: the first operand.
   :param b: the second operand.

   :return: an integral value expressing how *a* compares to *b*.

   :exception std\:\:domain_error: if at least one of the operands is NaN.

.. cpp:function:: int mppp::cmpabs(const mppp::real &a, const mppp::real &b)

   .. versionadded:: 0.20

   Three-way comparison of absolute values.

   This function will compare *a* and *b*, returning:

   * zero if :math:`\left|a\right|=\left|b\right|`,
   * a negative value if :math:`\left|a\right|<\left|b\right|`,
   * a positive value if :math:`\left|a\right|>\left|b\right|`.

   If at least one NaN value is involved in the comparison, an error will be raised.

   :param a: the first operand.
   :param b: the second operand.

   :return: an integral value expressing how the absolute values of *a* and *b* compare.

   :exception std\:\:domain_error: if at least one of the operands is NaN.

.. cpp:function:: int mppp::cmp_ui_2exp(const mppp::real &a, unsigned long n, mpfr_exp_t e)
.. cpp:function:: int mppp::cmp_si_2exp(const mppp::real &a, long n, mpfr_exp_t e)

   .. versionadded:: 0.20

   Comparison with integral multiples of powers of 2.

   This function will compare *a* to :math:`n\times 2^e`, returning:

   * zero if :math:`a=n\times 2^e`,
   * a negative value if :math:`a<n\times 2^e`,
   * a positive value if :math:`a>n\times 2^e`.

   If *a* is NaN, an error will be raised.

   :param a: the first operand.
   :param n: the integral multiplier.
   :param e: the power of 2.

   :return: an integral value expressing how *a* compares to :math:`n\times 2^e`.

   :exception std\:\:domain_error: if *a* is NaN.

.. cpp:function:: bool mppp::real_equal_to(const mppp::real &a, const mppp::real &b)

   Equality predicate with special handling for NaN.

   If both *a* and *b* are not NaN, this function is identical to the equality operator for
   :cpp:class:`~mppp::real`. If at least one operand is NaN, this function will return ``true``
   if both operands are NaN, ``false`` otherwise.

   In other words, this function behaves like an equality operator which considers all NaN
   values equal to each other.

   :param a: the first operand.
   :param b: the second operand.

   :return: ``true`` if :math:`a = b` (including the case in which both operands are NaN),
     ``false`` otherwise.

.. cpp:function:: bool mppp::real_lt(const mppp::real &a, const mppp::real &b)
.. cpp:function:: bool mppp::real_gt(const mppp::real &a, const mppp::real &b)

   Comparison predicates with special handling for NaN and moved-from :cpp:class:`~mppp::real`.

   These functions behave like less/greater-than operators which consider NaN values
   greater than non-NaN values, and moved-from objects greater than both NaN and non-NaN values.
   These functions can be used as comparators in various facilities of the
   standard library (e.g., ``std::sort()``, ``std::set``, etc.).

   :param a: the first operand.
   :param b: the second operand.

   :return: ``true`` if :math:`a < b` (respectively, :math:`a > b`), following the rules detailed above
    regarding NaN values and moved-from objects, ``false`` otherwise.

.. _real_roots:

Roots
~~~~~

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::sqrt(mppp::real &rop, T &&op)

   Binary :cpp:class:`~mppp::real` square root.

   This function will compute the square root of *op* and store it
   into *rop*. The precision of the result will be equal to the precision
   of *op*.

   If *op* is -0, *rop* will be set to -0. If *op* is negative, *rop* will be set to NaN.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::sqrt(T &&r)

   Unary :cpp:class:`~mppp::real` square root.

   This function will compute and return the square root of *r*.
   The precision of the result will be equal to the precision of *r*.

   If *r* is -0, the result will be -0. If *r* is negative, the result will be NaN.

   :param r: the operand.

   :return: the square root of *r*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::sqrt1pm1(mppp::real &rop, T &&op)

   .. versionadded:: 0.19

   .. note::

      This function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Binary :cpp:class:`~mppp::real` sqrt1pm1.

   This function will compute :math:`\sqrt{1+x}-1`, where :math:`x` is the value of *op*,
   and store the result into *rop*. The precision of the result will be equal to the precision
   of *op*.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::sqrt1pm1(T &&r)

   .. versionadded:: 0.19

   .. note::

      This function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Unary :cpp:class:`~mppp::real` sqrt1pm1.

   This function will compute and return :math:`\sqrt{1+x}-1`, where :math:`x`
   is the value of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the operand.

   :return: the sqrt1pm1 of *r*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::rec_sqrt(mppp::real &rop, T &&op)

   .. versionadded:: 0.12

   Binary :cpp:class:`~mppp::real` reciprocal square root.

   This function will compute the reciprocal square root of *op* and store it into *rop*. The precision
   of the result will be equal to the precision of *op*.

   If *op* is zero, *rop* will be set to a positive infinity (regardless of the sign of *op*).
   If *op* is a positive infinity, *rop* will be set to +0. If *op* is negative, *rop* will be set to NaN.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::rec_sqrt(T &&r)

   .. versionadded:: 0.12

   Unary :cpp:class:`~mppp::real` reciprocal square root.

   This function will compute and return the reciprocal square root of *r*.
   The precision of the result will be equal to the precision of *r*.

   If *r* is zero, a positive infinity will be returned (regardless of the sign of *r*).
   If *r* is a positive infinity, +0 will be returned. If *r* is negative,
   NaN will be returned.

   :param r: the operand.

   :return: the reciprocal square root of *r*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::cbrt(mppp::real &rop, T &&op)

   .. versionadded:: 0.12

   Binary :cpp:class:`~mppp::real` cubic root.

   This function will compute the cubic root of *op* and store it
   into *rop*. The precision of the result will be equal to the precision
   of *op*.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::cbrt(T &&r)

   .. versionadded:: 0.12

   Unary :cpp:class:`~mppp::real` cubic root.

   This function will compute and return the cubic root of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the operand.

   :return: the cubic root of *r*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::rootn_ui(mppp::real &rop, T &&op, unsigned long k)

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

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::rootn_ui(T &&r, unsigned long k)

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

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::pow(mppp::real &rop, T &&op1, U &&op2)

   Ternary exponentiation.

   This function will set *rop* to *op1* raised to the power of *op2*.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param op1: the base.
   :param op2: the exponent.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::pow(T &&op1, U &&op2)

   Binary exponentiation.

   This function will compute and return *op1* raised to the power of *op2*.
   The precision of the result will be set to the largest precision among the operands.

   :param op1: the base.
   :param op2: the exponent.

   :return: *op1* raised to the power of *op2*.

.. _real_trig:

Trigonometry
~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::sin(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::cos(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::tan(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::sec(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::csc(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::cot(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::sin_pi(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::cos_pi(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::tan_pi(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::cot_pi(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::sinc(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::sinc_pi(mppp::real &rop, T &&x)

   .. note::

      The functions ``sin_pi()``, ``cos_pi()``, ``tan_pi()``,
      ``cot_pi()``, ``sinc()`` and ``sinc_pi()`` are available only
      if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

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

   .. versionadded:: 0.19

      The functions ``sin_pi()``, ``cos_pi()``, ``tan_pi()``,
      ``cot_pi()``, ``sinc()`` and ``sinc_pi()``.

   :param rop: the return value.
   :param x: the argument.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::sin(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::cos(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::tan(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::sec(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::csc(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::cot(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::sin_pi(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::cos_pi(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::tan_pi(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::cot_pi(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::sinc(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::sinc_pi(T &&x)

   .. note::

      The functions ``sin_pi()``, ``cos_pi()``, ``tan_pi()``,
      ``cot_pi()``, ``sinc()`` and ``sinc_pi()`` are available only
      if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

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

   .. versionadded:: 0.19

      The functions ``sin_pi()``, ``cos_pi()``, ``tan_pi()``,
      ``cot_pi()``, ``sinc()`` and ``sinc_pi()``.

   :param x: the argument.

   :return: the trigonometric function of *x*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_real T> void mppp::sin_cos(mppp::real &sop, mppp::real &cop, T &&op)

   Simultaneous sine and cosine.

   This function will set *sop* and *cop* respectively to the sine and cosine of *op*.
   *sop* and *cop* must be distinct objects. The precision of *sop* and *rop* will be set to the
   precision of *op*.

   :param sop: the sine return value.
   :param cop: the cosine return value.
   :param op: the operand.

   :exception std\:\:invalid_argument: if *sop* and *cop* are the same object.

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::asin(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::acos(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::atan(mppp::real &rop, T &&op)

   Binary basic inverse trigonometric functions.

   These functions will set *rop* to, respectively, the arcsine, arccosine and
   arctangent of *op*.
   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::asin(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::acos(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::atan(T &&r)

   Unary basic inverse trigonometric functions.

   These functions will return, respectively, the arcsine, arccosine and
   arctangent of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the arcsine, arccosine or arctangent of *r*.

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::atan2(mppp::real &rop, T &&y, U &&x)

   Ternary arctangent-2.

   This function will set *rop* to the arctangent-2 of *y* and *x*.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param y: the sine argument.
   :param x: the cosine argument.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::atan2(T &&y, U &&x)

   Binary arctangent-2.

   This function will compute and return the arctangent-2 of *y* and *x*.
   The precision of the result will be set to the largest precision among the operands.

   :param y: the sine argument.
   :param x: the cosine argument.

   :return: the arctangent-2 of *y* and *x*.

.. _real_hyper:

Hyperbolic functions
~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::sinh(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::cosh(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::tanh(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::sech(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::csch(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::coth(mppp::real &rop, T &&op)

   Binary basic hyperbolic functions.

   These functions will set *rop* to, respectively, the hyperbolic sine, cosine, tangent, secant,
   cosecant and cotangent of *op*.
   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::sinh(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::cosh(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::tanh(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::sech(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::csch(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::coth(T &&r)

   Unary basic hyperbolic functions.

   These functions will return, respectively, the hyperbolic sine, cosine, tangent,
   secant, cosecant and cotangent of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the hyperbolic sine, cosine, tangent, secant, cosecant or cotangent of *r*.

.. cpp:function:: template <mppp::cvr_real T> void mppp::sinh_cosh(mppp::real &sop, mppp::real &cop, T &&op)

   Simultaneous hyperbolic sine and cosine.

   This function will set *sop* and *cop* respectively to the hyperbolic sine and cosine of *op*.
   *sop* and *cop* must be distinct objects. The precision of *sop* and *rop* will be set to the
   precision of *op*.

   :param sop: the hyperbolic sine return value.
   :param cop: the hyperbolic cosine return value.
   :param op: the operand.

   :exception std\:\:invalid_argument: if *sop* and *cop* are the same object.

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::asinh(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::acosh(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::atanh(mppp::real &rop, T &&op)

   Binary basic inverse hyperbolic functions.

   These functions will set *rop* to, respectively, the inverse hyperbolic sine, cosine and
   tangent of *op*.
   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::asinh(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::acosh(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::atanh(T &&r)

   Unary basic inverse hyperbolic functions.

   These functions will return, respectively, the inverse hyperbolic sine, cosine and
   tangent of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the inverse hyperbolic sine, cosine or tangent of *r*.

.. _real_logexp:

Logarithms and exponentials
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::exp(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::exp2(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::exp10(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::expm1(mppp::real &rop, T &&x)

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

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::exp(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::exp2(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::exp10(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::expm1(T &&x)

   Unary exponentials.

   These functions will return, respectively,

   * :math:`e^x`,
   * :math:`2^x`,
   * :math:`10^x`,
   * :math:`e^x - 1`.

   The precision of the result will be equal to the precision of *x*.

   :param x: the exponent.

   :return: the exponential of *x*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::log(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::log2(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::log10(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::log1p(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::log_base_ui(mppp::real &rop, T &&x, unsigned long b)

   .. note::

      The ``log_base_ui()`` function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Binary logarithms.

   These functions will set *rop* to, respectively,

   * :math:`\log x`,
   * :math:`\log_2 x`,
   * :math:`\log_{10} x`,
   * :math:`\log\left( 1+x \right)`,
   * :math:`\log_b x`.

   The precision of the result will be equal to the precision of *x*.

   .. versionadded:: 0.21

      The ``log_base_ui()`` function.

   :param rop: the return value.
   :param x: the operand.
   :param b: the base of the logarithm.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::log(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::log2(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::log10(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::log1p(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::log_base_ui(T &&x, unsigned long b)

   .. note::

      The ``log_base_ui()`` function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Unary logarithms.

   These functions will return, respectively,

   * :math:`\log x`,
   * :math:`\log_2 x`,
   * :math:`\log_{10} x`,
   * :math:`\log\left( 1+x \right)`,
   * :math:`\log_b x`.

   The precision of the result will be equal to the precision of *x*.

   .. versionadded:: 0.21

      The ``log_base_ui()`` function.

   :param x: the operand.
   :param b: the base of the logarithm.

   :return: the logarithm of *x*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::log_hypot(mppp::real &rop, T &&x, U &&y)

   .. versionadded:: 0.19

   .. note::

      This function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Ternary log hypot function.

   This function will set *rop* to :math:`\log\left(\sqrt{x^2+y^2}\right)`.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param x: the first argument.
   :param y: the second argument.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::log_hypot(T &&x, U &&y)

   .. versionadded:: 0.19

   .. note::

      This function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Binary log hypot function.

   This function will compute and return :math:`\log\left(\sqrt{x^2+y^2}\right)`.
   The precision of the result will be set to the largest precision among the operands.

   :param x: the first argument.
   :param y: the second argument.

   :return: the log hypot function of *x* and *y*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

Polylogarithms
~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::li2(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::polylog_si(mppp::real &rop, long s, T &&x)
.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::polylog(mppp::real &rop, T &&s, U &&x)

   .. note::

      The ``polylog_si()`` and ``polylog()`` functions are available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Polylogarithms.

   These functions will set *rop* to, respectively:

   * the dilogarithm :math:`\operatorname{Li}_2\left( x \right)`,
   * the polylogarithm of integer order :math:`s` :math:`\operatorname{Li}_s\left( x \right)`,
   * the polylogarithm of real order :math:`s` :math:`\operatorname{Li}_s\left( x \right)`.

   The precision of the result will be equal to the precision of *x* (for ``li2()`` and ``polylog_si()``) or
   to the largest precision among *s* and *x* (for ``polylog()``).

   .. versionadded:: 0.24

      The ``polylog_si()`` and ``polylog()`` functions.

   :param rop: the return value.
   :param s: the order of the polylogarithm.
   :param x: the argument.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::li2(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::polylog_si(long s, T &&x)
.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::polylog(T &&s, U &&x)

   .. note::

      The ``polylog_si()`` and ``polylog()`` functions are available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Polylogarithms.

   These functions will return, respectively:

   * the dilogarithm :math:`\operatorname{Li}_2\left( x \right)`,
   * the polylogarithm of integer order :math:`s` :math:`\operatorname{Li}_s\left( x \right)`,
   * the polylogarithm of real order :math:`s` :math:`\operatorname{Li}_s\left( x \right)`.

   The precision of the result will be equal to the precision of *x* (for ``li2()`` and ``polylog_si()``) or
   to the largest precision among *s* and *x* (for ``polylog()``).

   .. versionadded:: 0.24

      The ``polylog_si()`` and ``polylog()`` functions.

   :param s: the order of the polylogarithm.
   :param x: the argument.

   :return: the polylogarithm of *x*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. _real_gamma:

Gamma functions
~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::gamma(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::lngamma(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::lgamma(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::digamma(mppp::real &rop, T &&op)

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

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::gamma(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::lngamma(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::lgamma(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::digamma(T &&r)

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

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::gamma_inc(mppp::real &rop, T &&x, U &&y)

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

.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::gamma_inc(T &&x, U &&y)

   .. versionadded:: 0.17

   .. note::

      This function is available from MPFR 4 onwards.

   Binary incomplete Gamma function.

   This function will compute and return the upper incomplete Gamma function of *x* and *y*.
   The precision of the result will be set to the largest precision among the operands.

   :param x: the first argument.
   :param y: the second argument.

   :return: the upper incomplete Gamma function of *x* and *y*

.. _real_bessel:

Bessel functions
~~~~~~~~~~~~~~~~

.. versionadded:: 0.17

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::j0(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::j1(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::jn(mppp::real &rop, long n, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::y0(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::y1(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::yn(mppp::real &rop, long n, T &&x)

   Bessel functions of the first and second kind of integral order.

   These functions will set *rop* to, respectively,

   * :math:`J_0\left( x \right)`,
   * :math:`J_1\left( x \right)`,
   * :math:`J_n\left( x \right)`,
   * :math:`Y_0\left( x \right)`,
   * :math:`Y_1\left( x \right)`,
   * :math:`Y_n\left( x \right)`.

   The precision of the result will be equal to the precision of *x*.

   :param rop: the return value.
   :param n: the order of the Bessel function.
   :param x: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::j0(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::j1(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::jn(long n, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::y0(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::y1(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::yn(long n, T &&x)

   Bessel functions of the first and second kind of integral order.

   These functions will return, respectively,

   * :math:`J_0\left( x \right)`,
   * :math:`J_1\left( x \right)`,
   * :math:`J_n\left( x \right)`,
   * :math:`Y_0\left( x \right)`,
   * :math:`Y_1\left( x \right)`,
   * :math:`Y_n\left( x \right)`.

   The precision of the result will be equal to the precision of *x*.

   :param n: the order of the Bessel function.
   :param r: the argument.

   :return: the Bessel function of *r*.

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::jx(mppp::real &rop, T &&nu, U &&x)
.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::yx(mppp::real &rop, T &&nu, U &&x)

   .. versionadded:: 0.20

   .. note::

      These functions are available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Bessel functions of the first and second kind of real order.

   These functions will set *rop* to, respectively,

   * :math:`J_\nu\left( x \right)`,
   * :math:`Y_\nu\left( x \right)`,

   where :math:`\nu \in \mathbb{R}`. The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param nu: the order of the Bessel function.
   :param x: the argument.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::jx(T &&nu, U &&x)
.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::yx(T &&nu, U &&x)

   .. versionadded:: 0.20

   .. note::

      These functions are available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Bessel functions of the first and second kind of real order.

   These functions will return, respectively,

   * :math:`J_\nu\left( x \right)`,
   * :math:`Y_\nu\left( x \right)`,

   where :math:`\nu \in \mathbb{R}`. The precision of the result will be set to the largest precision among the operands.

   :param nu: the order of the Bessel function.
   :param x: the argument.

   :return: the Bessel function of *x*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. _real_err_func:

Error functions
~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::erf(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::erfc(mppp::real &rop, T &&op)

   Binary error functions.

   These functions will set *rop* to, respectively, the error function and the complementary
   error function of *op*.
   The precision of the result will be equal to the precision of *op*.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::erf(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::erfc(T &&r)

   Unary error functions.

   These functions will return, respectively, the error function and the complementary
   error function of *r*.
   The precision of the result will be equal to the precision of *r*.

   :param r: the argument.

   :return: the error function or the complementary error function of *r*.

.. _real_other_specfunc:

Other special functions
~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::eint(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::zeta(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::ai(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::lambert_w0(mppp::real &rop, T &&op)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::lambert_wm1(mppp::real &rop, T &&op)

   .. note::

      The ``lambert_w0()`` and ``lambert_wm1()`` functions are available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Other binary special functions.

   These functions will set *rop* to, respectively,

   * the exponential integral,
   * the Riemann Zeta function,
   * the Airy function,
   * the Lambert W functions :math:`W_0` and :math:`W_{-1}`,

   of *op*. The precision of the result will be equal to the precision of *op*.

   .. versionadded:: 0.24

      The ``lambert_w0()`` and ``lambert_wm1()`` functions.

   :param rop: the return value.
   :param op: the argument.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::eint(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::zeta(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::ai(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::lambert_w0(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::lambert_wm1(T &&r)

   .. note::

      The ``lambert_w0()`` and ``lambert_wm1()`` functions are available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Other unary special functions.

   These functions will return, respectively,

   * the exponential integral,
   * the Riemann Zeta function,
   * the Airy function,
   * the Lambert W functions :math:`W_0` and :math:`W_{-1}`,

   of *r*. The precision of the result will be equal to the precision of *r*.

   .. versionadded:: 0.24

      The ``lambert_w0()`` and ``lambert_wm1()`` functions.

   :param r: the argument.

   :return: the exponential integral, Riemann Zeta function, Airy function or Lambert W function of *r*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPFR types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::beta(mppp::real &rop, T &&x, U &&y)

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

.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::beta(T &&x, U &&y)

   .. versionadded:: 0.17

   .. note::

      This function is available from MPFR 4 onwards.

   Binary beta function.

   This function will compute and return the beta function of *x* and *y*.
   The precision of the result will be set to the largest precision among the operands.

   :param x: the first argument.
   :param y: the second argument.

   :return: the beta function of *x* and *y*.

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::hypot(mppp::real &rop, T &&x, U &&y)

   Ternary hypot function.

   This function will set *rop* to :math:`\sqrt{x^2+y^2}`.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param x: the first argument.
   :param y: the second argument.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::hypot(T &&x, U &&y)

   Binary hypot function.

   This function will compute and return :math:`\sqrt{x^2+y^2}`.
   The precision of the result will be set to the largest precision among the operands.

   :param x: the first argument.
   :param y: the second argument.

   :return: the hypot function of *x* and *y*.

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::agm(mppp::real &rop, T &&x, U &&y)

   Ternary AGM.

   This function will set *rop* to the arithmetic-geometric mean of *x* and *y*.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param x: the first argument.
   :param y: the second argument.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::agm(T &&x, U &&y)

   Binary AGM.

   This function will compute and return the arithmetic-geometric mean of *x* and *y*.
   The precision of the result will be set to the largest precision among the operands.

   :param x: the first argument.
   :param y: the second argument.

   :return: the AGM of *x* and *y*.

.. _real_intrem:

Integer and remainder related functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::ceil(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::floor(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::round(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::roundeven(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::trunc(mppp::real &rop, T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real &mppp::frac(mppp::real &rop, T &&x)

   .. note::

      The ``roundeven()`` function is available from MPFR 4 onwards.

   Binary integer and remainder-related functions.

   These functions will set *rop* to, respectively:

   * :math:`\left\lceil x \right\rceil`,
   * :math:`\left\lfloor x \right\rfloor`,
   * *x* rounded to nearest (IEEE ``roundTiesToAway`` mode),
   * *x* rounded to nearest (IEEE ``roundTiesToEven`` mode),
   * :math:`\operatorname{trunc}\left( x \right)`,
   * the fractional part of *x*.

   The precision of the result will be equal to the precision
   of *x*.

   .. versionadded:: 0.21

      The ``ceil()``, ``floor()``, ``round()``, ``roundeven()`` and ``frac()``
      functions.

   :param rop: the return value.
   :param x: the operand.

   :return: a reference to *rop*.

   :exception std\:\:domain_error: if *x* is NaN.

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::ceil(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::floor(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::round(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::roundeven(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::trunc(T &&x)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::frac(T &&x)

   .. note::

      The ``roundeven()`` function is available from MPFR 4 onwards.

   Unary integer and remainder-related functions.

   These functions will return, respectively:

   * :math:`\left\lceil x \right\rceil`,
   * :math:`\left\lfloor x \right\rfloor`,
   * *x* rounded to nearest (IEEE ``roundTiesToAway`` mode),
   * *x* rounded to nearest (IEEE ``roundTiesToEven`` mode),
   * :math:`\operatorname{trunc}\left( x \right)`,
   * the fractional part of *x*.

   The precision of the result will be equal to the precision
   of *x*.

   .. versionadded:: 0.21

      The ``ceil()``, ``floor()``, ``round()``, ``roundeven()`` and ``frac()``
      functions.

   :param x: the operand.

   :return: the result of the operation.

   :exception std\:\:domain_error: if *x* is NaN.

.. cpp:function:: template <mppp::cvr_real T> void mppp::modf(mppp::real &iop, mppp::real &fop, T &&op)

   .. versionadded:: 0.21

   Simultaneous integral and fractional parts.

   This function will set *iop* and *fop* respectively to the integral and
   fractional parts of *op*.
   *iop* and *fop* must be distinct objects. The precision of *iop* and *fop* will be set to the
   precision of *op*.

   :param iop: the integral part return value.
   :param fop: the fractional part return value.
   :param op: the operand.

   :exception std\:\:invalid_argument: if *iop* and *fop* are the same object.
   :exception std\:\:domain_error: if *op* is NaN.

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::fmod(mppp::real &rop, T &&x, U &&y)
.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::remainder(mppp::real &rop, T &&x, U &&y)

   .. versionadded:: 0.21

   Ternary floating modulus and remainder.

   These functions will set *rop* to, respectively, the floating modulus and the remainder of the
   division :math:`x/y`.

   The floating modulus is :math:`x - n\times y`, where :math:`n` is :math:`x/y` with its fractional part truncated.

   The remainder is :math:`x - m\times y`, where :math:`m` is the integral value nearest the exact value
   :math:`x/y`.

   Special values are handled as described in the C99 standard.

   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param x: the numerator.
   :param y: the denominator.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::fmod(T &&x, U &&y)
.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::remainder(T &&x, U &&y)

   .. versionadded:: 0.21

   Binary floating modulus and remainder.

   These functions will return, respectively, the floating modulus and the remainder of the
   division :math:`x/y`.

   The floating modulus is :math:`x - n\times y`, where :math:`n` is :math:`x/y` with its fractional part truncated.

   The remainder is :math:`x - m\times y`, where :math:`m` is the integral value nearest the exact value
   :math:`x/y`.

   Special values are handled as described in the C99 standard.

   The precision of the result will be set to the largest precision among the operands.

   :param x: the numerator.
   :param y: the denominator.

   :return: the floating modulus or remainder of :math:`x/y`.

.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::fmodquo(mppp::real &rop, long *q, T &&x, U &&y)
.. cpp:function:: template <mppp::cvr_real T, mppp::cvr_real U> mppp::real &mppp::remquo(mppp::real &rop, long *q, T &&x, U &&y)

   .. note::

      The ``fmodquo()`` function is available from MPFR 4 onwards.

   .. versionadded:: 0.21

   Floating modulus and remainder with quotient.

   These functions will set *rop* to, respectively, the floating modulus and the remainder of the
   division :math:`x/y`. Additionally, they will also store the low significant bits from the quotient
   in the value pointed to by *q* (more precisely the number of bits in a ``long`` minus one),
   with the sign of *x* divided by *y* (except if those low bits are all zero, in which case zero is returned).

   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param q: a pointer to the quotient return value.
   :param x: the numerator.
   :param y: the denominator.

   :return: a reference to *rop*.

.. _real_io:

Input/Output
~~~~~~~~~~~~

.. cpp:function:: std::ostream &mppp::operator<<(std::ostream &os, const mppp::real &r)

   Output stream operator.

   This function will direct to the output stream *os* the input :cpp:class:`~mppp::real` *r*.

   :param os: the target stream.
   :param r: the input argument.

   :return: a reference to *os*.

   :exception std\:\:overflow_error: in case of (unlikely) overflow errors.
   :exception std\:\:invalid_argument: if the MPFR printing primitive ``mpfr_asprintf()`` returns an error code.
   :exception unspecified: any exception raised by the public interface of ``std::ostream`` or by memory allocation errors.

.. _real_s11n:

Serialisation
~~~~~~~~~~~~~

.. versionadded:: 0.22

.. cpp:function:: std::size_t mppp::binary_size(const mppp::real &x)

   Binary size.

   This function is the free function equivalent of the
   :cpp:func:`mppp::real::binary_size()` member function.

   :param x: the input argument.

   :return: the output of :cpp:func:`mppp::real::binary_size()` called on *x*.

   :exception unspecified: any exception thrown by :cpp:func:`mppp::real::binary_size()`.

.. cpp:function:: template <typename T> std::size_t mppp::binary_save(const mppp::real &x, T &&dest)

   Binary serialisation.

   .. note::

      This function participates in overload resolution only if the expression

      .. code-block:: c++

         return x.binary_save(std::forward<T>(dest));

      is well-formed.

   This function is the free function equivalent of the
   :cpp:func:`mppp::real::binary_save()` overloads.

   :param x: the input argument.
   :param dest: the object into which *x* will be serialised.

   :return: the output of the invoked :cpp:func:`mppp::real::binary_save()`
     overload called on *x* with *dest* as argument.

   :exception unspecified: any exception thrown by the invoked :cpp:func:`mppp::real::binary_save()` overload.

.. cpp:function:: template <typename T> std::size_t mppp::binary_load(mppp::real &x, T &&src)

   Binary deserialisation.

   .. note::

      This function participates in overload resolution only if the expression

      .. code-block:: c++

         return x.binary_load(std::forward<T>(src));

      is well-formed.

   This function is the free function equivalent of the
   :cpp:func:`mppp::real::binary_load()` overloads.

   :param x: the output argument.
   :param src: the object containing the serialised :cpp:class:`~mppp::real`
     that will be loaded into *x*.

   :return: the output of the invoked :cpp:func:`mppp::real::binary_load()`
     overload called on *x* with *src* as argument.

   :exception unspecified: any exception thrown by the invoked :cpp:func:`mppp::real::binary_load()` overload.

.. _real_operators:

Mathematical operators
----------------------

.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::operator+(T &&r)
.. cpp:function:: template <mppp::cvr_real T> mppp::real mppp::operator-(T &&r)

   Identity and negation operators.

   :param r: the input argument.

   :return: :math:`r` and :math:`-r` respectively.

.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::operator+(T &&a, U &&b)
.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::operator-(T &&a, U &&b)
.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::operator*(T &&a, U &&b)
.. cpp:function:: template <typename T, mppp::real_op_types<T> U> mppp::real mppp::operator/(T &&a, U &&b)

   Binary arithmetic operators.

   The precision of the result will be set to the largest precision among the operands.

   :param a: the first operand.
   :param b: the second operand.

   :return: the result of the binary operation.

.. cpp:function:: template <typename U, mppp::real_in_place_op_types<U> T> T &mppp::operator+=(T &a, U &&b)
.. cpp:function:: template <typename U, mppp::real_in_place_op_types<U> T> T &mppp::operator-=(T &a, U &&b)
.. cpp:function:: template <typename U, mppp::real_in_place_op_types<U> T> T &mppp::operator*=(T &a, U &&b)
.. cpp:function:: template <typename U, mppp::real_in_place_op_types<U> T> T &mppp::operator/=(T &a, U &&b)

   In-place arithmetic operators.

   If *a* is a :cpp:class:`~mppp::real`, then these operators are equivalent, respectively,
   to the expressions:

   .. code-block:: c++

      a = a + b;
      a = a - b;
      a = a * b;
      a = a / b;

   Otherwise, these operators are equivalent to the expressions:

   .. code-block:: c++

      a = static_cast<T>(a + b);
      a = static_cast<T>(a - b);
      a = static_cast<T>(a * b);
      a = static_cast<T>(a / b);

   :param a: the first operand.
   :param b: the second operand.

   :return: a reference to *a*.

   :exception unspecified: any exception thrown by the generic conversion operator of :cpp:class:`~mppp::real`.

.. cpp:function:: mppp::real &mppp::operator++(mppp::real &x)
.. cpp:function:: mppp::real &mppp::operator--(mppp::real &x)

   Prefix increment/decrement.

   :param x: the input argument.

   :return: a reference to *x* after the increment/decrement.

.. cpp:function:: mppp::real mppp::operator++(mppp::real &x, int)
.. cpp:function:: mppp::real mppp::operator--(mppp::real &x, int)

   Suffix increment/decrement.

   :param x: the input argument.

   :return: a copy of *x* before the increment/decrement.

.. cpp:function:: template <typename T, mppp::real_eq_op_types<T> U> bool mppp::operator==(const T &a, const U &b)
.. cpp:function:: template <typename T, mppp::real_eq_op_types<T> U> bool mppp::operator!=(const T &a, const U &b)
.. cpp:function:: template <typename T, mppp::real_op_types<T> U> bool mppp::operator<(const T &a, const U &b)
.. cpp:function:: template <typename T, mppp::real_op_types<T> U> bool mppp::operator<=(const T &a, const U &b)
.. cpp:function:: template <typename T, mppp::real_op_types<T> U> bool mppp::operator>(const T &a, const U &b)
.. cpp:function:: template <typename T, mppp::real_op_types<T> U> bool mppp::operator>=(const T &a, const U &b)

   Comparison operators.

   These operators will compare *a* and *b*, returning ``true`` if, respectively:

   * :math:`a=b`,
   * :math:`a \neq b`,
   * :math:`a<b`,
   * :math:`a \leq b`,
   * :math:`a>b`,
   * :math:`a \geq b`,

   and ``false`` otherwise.

   The comparisons are always exact (i.e., no rounding is involved).

   These operators handle NaN in the same way specified by the IEEE floating-point
   standard. :ref:`Alternative comparison functions <real_comparison>` treating NaN
   specially are available.

   :param a: the first operand.
   :param b: the second operand.

   :return: the result of the comparison.

.. _real_constants:

Constants
---------

.. cpp:function:: mppp::real mppp::real_pi(mpfr_prec_t p)
.. cpp:function:: mppp::real mppp::real_log2(mpfr_prec_t p)
.. cpp:function:: mppp::real mppp::real_euler(mpfr_prec_t p)
.. cpp:function:: mppp::real mppp::real_catalan(mpfr_prec_t p)

   These functions will return, respectively:

   * the :math:`\pi` constant,
   * the :math:`\log 2` constant,
   * the Euler-Mascheroni constant (0.577…),
   * Catalan's constant (0.915…),

   with a precision of *p*.

   .. versionadded:: 0.21

      The ``real_log2()``, ``real_euler()`` and ``real_catalan()``
      functions.

   :param p: the desired precision.

   :return: an approximation of a constant.

   :exception std\:\:invalid_argument: if *p* is outside the range established by
     :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`.

.. cpp:function:: mppp::real &mppp::real_pi(mppp::real &rop)
.. cpp:function:: mppp::real &mppp::real_log2(mppp::real &rop)
.. cpp:function:: mppp::real &mppp::real_euler(mppp::real &rop)
.. cpp:function:: mppp::real &mppp::real_catalan(mppp::real &rop)

   These functions will set *rop* to, respectively:

   * the :math:`\pi` constant,
   * the :math:`\log 2` constant,
   * the Euler-Mascheroni constant (0.577…),
   * Catalan's constant (0.915…).

   The precision of *rop* will not be altered.

   .. versionadded:: 0.21

      The ``real_log2()``, ``real_euler()`` and ``real_catalan()``
      functions.

   :param rop: the return value.

   :return: a reference to *rop*.

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
