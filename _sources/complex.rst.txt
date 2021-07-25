Multiprecision complex numbers
==============================

.. note::

   The functionality described in this section is available only if mp++ was configured
   with the ``MPPP_WITH_MPC`` option enabled (see the :ref:`installation instructions <installation>`).

.. versionadded:: 0.20

*#include <mp++/complex.hpp>*

The complex class
-----------------

.. cpp:class:: mppp::complex

   Multiprecision complex class.

   This class represents arbitrary-precision complex values as real-imaginary pairs encoded in a
   binary floating-point format.
   It acts as a wrapper around the MPC :cpp:type:`mpc_t` type, pairing multiprecision significands
   (whose size can be set at runtime) to fixed-size exponents. In other words, the real and imaginary parts
   of :cpp:class:`~mppp::complex`
   values can have an arbitrary number of binary digits of precision (limited only by the available memory),
   but the exponent range is limited. The real and imaginary parts of a :cpp:class:`~mppp::complex` always
   have the same precision.

   :cpp:class:`~mppp::complex` aims to behave like a floating-point C++ type whose precision is a runtime property
   of the class instances rather than a compile-time property of the type. Because of this, the way precision
   is handled in :cpp:class:`~mppp::complex` differs from the way it is managed in MPC. The most important difference
   is that in operations involving :cpp:class:`~mppp::complex` the precision of the result is usually determined
   by the precision of the operands, whereas in MPC the precision of the operation is determined by the precision
   of the return value (which is always passed as the first function parameter in the MPC API). For instance,
   in the following code,

   .. code-block:: c++

      auto c = complex{5, complex_prec_t(200)} + real{6, complex_prec_t(150)};

   the first operand has a value of 5 and precision of 200 bits, while the second operand has a value of 6 and precision
   150 bits. The precision of the result ``c`` will be
   the maximum precision among the two operands, that is, 200 bits.

   The real and imaginary parts of a :cpp:class:`~mppp::complex` always have the same precision.
   The precision of a :cpp:class:`~mppp::complex` can be set at construction, or it can be changed later via functions
   such as :cpp:func:`mppp::complex::set_prec()`, :cpp:func:`mppp::complex::prec_round()`, etc. By default,
   the precision of a :cpp:class:`~mppp::complex` is automatically deduced upon construction following a set of heuristics
   aimed at ensuring that the constructed :cpp:class:`~mppp::complex` preserves the value used for initialisation,
   if possible.
   For instance, by default the construction of a :cpp:class:`~mppp::complex` from a 32 bit integer will yield a
   :cpp:class:`~mppp::complex` with a precision of 32 bits. This behaviour can be altered by specifying explicitly
   the desired precision value.

   Most of the functionality is exposed via plain :ref:`functions <complex_functions>`, with the
   general convention that the functions are named after the corresponding MPC functions minus the leading ``mpc_``
   prefix. For instance, the MPC call

   .. code-block:: c++

      mpc_add(rop, a, b, MPC_RNDNN);

   that writes the result of ``a + b``, rounded to nearest, into ``rop``, becomes simply

   .. code-block:: c++

      add(rop, a, b);

   where the ``add()`` function is resolved via argument-dependent lookup. Function calls with overlapping arguments
   are allowed, unless noted otherwise. Unless otherwise specified, the :cpp:class:`~mppp::complex` API always
   rounds to nearest (that is, the ``MPC_RNDNN`` rounding mode is used).

   Various :ref:`overloaded operators <complex_operators>` are provided. The arithmetic operators always return
   a :cpp:class:`~mppp::complex` result.

   Member functions are provided to access directly the internal :cpp:type:`mpc_t` instance (see
   :cpp:func:`mppp::complex::get_mpc_t()` and :cpp:func:`mppp::complex::_get_mpc_t()`), so that
   it is possible to use transparently the MPC API with :cpp:class:`~mppp::complex` objects.

   A :ref:`tutorial <tutorial_complex>` showcasing various features of :cpp:class:`~mppp::complex`
   is available.

   .. cpp:function:: complex()

      Default constructor.

      The real and imaginary parts will both be initialised to positive zero, the precision will be
      the value returned by :cpp:func:`mppp::real_prec_min()`.

   .. cpp:function:: complex(const complex &other)
   .. cpp:function:: complex(complex &&other) noexcept

      Copy and move constructors.

      The copy constructor performs an exact deep copy of the input object.

      After move construction, the only valid operations on *other* are
      destruction, copy/move assignment and the invocation of the :cpp:func:`~mppp::complex::is_valid()`
      member function. After re-assignment, *other* can be used normally again.

      :param other: the construction argument.

   .. cpp:function:: explicit complex(const complex &other, complex_prec_t p)
   .. cpp:function:: explicit complex(complex &&other, complex_prec_t p)

      Copy/move constructors with custom precision.

      These constructors will set *this* to the value of *other* with precision *p*. If *p*
      is smaller than the precision of *other*, a rounding operation will be performed,
      otherwise the value will be copied exactly.

      After move construction, the only valid operations on *other* are
      destruction, copy/move assignment and the invocation of the :cpp:func:`~mppp::complex::is_valid()`
      member function. After re-assignment, *other* can be used normally again.

      :param other: the construction argument.
      :param p: the desired precision.

      :exception std\:\:invalid_argument: if *p* is outside the range established by
        :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`.

   .. cpp:function:: template <complex_interoperable T> complex(T &&x)
   .. cpp:function:: template <complex_interoperable T> explicit complex(T &&x, complex_prec_t p)

      Generic constructors.

      The generic constructors will set ``this`` to the value of *x*.

      The variant with the *p* argument will set the precision of ``this``
      exactly to *p*.

      The variant without the *p* argument will set the
      precision of ``this`` according to the following heuristics:

      * if ``T`` is :cpp:class:`~mppp::real`, then the precision is set to
        the precision of *x* (as returned by :cpp:func:`mppp::real::get_prec()`),
      * if ``T`` is real-valued, then the precision is set following the same
        heuristics described in the generic constructor of :cpp:class:`~mppp::real`,
      * if ``T`` is complex-valued, then the precision is set to the maximum
        between the precisions of the real and imaginary parts of *x* (which are deduced
        following the same
        heuristics described in the generic constructor of :cpp:class:`~mppp::real`).

      :param x: the construction argument.
      :param p: the desired precision.

      :exception unspecified: any exception raised by the invoked :cpp:class:`~mppp::real`
        constructor.

   .. cpp:function:: template <rv_complex_interoperable T, rv_complex_interoperable U> explicit complex(T &&x, U &&y, complex_prec_t p)
   .. cpp:function:: template <rv_complex_interoperable T, rv_complex_interoperable U> explicit complex(T &&x, U &&y)

      Generic constructors from real and imaginary parts.

      These constructors will set ``this`` to :math:`x+\imath y`.

      The variant with the *p* argument will set the precision of ``this``
      exactly to *p*.

      Otherwise, the precision of ``this`` will be the maximum among the deduced precisions
      of *x* and *y*. The precision deduction rules are the same explained in the generic
      constructors of :cpp:class:`~mppp::real`. If *x* and/or *y* are :cpp:class:`~mppp::real`,
      the deduced precision is the output of :cpp:func:`mppp::real::get_prec()`.

      :param x: the real part of the value that will be used for the initialisation.
      :param y: the imaginary part of the value that will be used for the initialisation.
      :param p: the desired precision.

      :exception unspecified: any exception raised by the invoked :cpp:class:`~mppp::real`
        constructor.

   .. cpp:function:: template <string_type T> explicit complex(const T &s, int base, complex_prec_t p)
   .. cpp:function:: template <string_type T> explicit complex(const T &s, complex_prec_t p)

      Constructors from string, base and precision.

      These constructors will initialise ``this`` from the :cpp:concept:`~mppp::string_type` *s*,
      which is interpreted as a complex number in base *base*. *base* must be either zero (in which case the base
      will be automatically deduced) or a number in the :math:`\left[ 2,62 \right]` range.
      The accepted string formats are:

      * a single floating-point number (e.g., ``1.234``),
      * a single floating-point number surrounded by round brackets
        (e.g., ``(1.234)``),
      * a pair of floating-point numbers, surrounded by round brackets and
        separated by a comma (e.g., ``(1.234, 4.567)``).

      The allowed floating-point representations (for both the real and imaginary part)
      are described in the documentation of the constructor from string of
      :cpp:class:`~mppp::real`.

      The precision of ``this`` will be set to *p*.

      The second constructor calls the first one with a *base* value of 10.

      :param s: the input string.
      :param base: the base used in the string representation.
      :param p: the desired precision.

      :exception std\:\:invalid_argument: if *base* is not zero and not in the
        :math:`\left[ 2,62 \right]` range, or *s* cannot be interpreted as a complex number.

      :exception unspecified: any exception thrown by the constructor of
        :cpp:class:`~mppp::real` from string.

   .. cpp:function:: explicit complex(const char *begin, const char *end, int base, complex_prec_t p)
   .. cpp:function:: explicit complex(const char *begin, const char *end, complex_prec_t p)

      Constructors from range of characters, base and precision.

      The first constructor will initialise ``this`` from the content of the input half-open range,
      which is interpreted as the string representation of a complex value in base ``base``.

      Internally, the constructor will copy the content of the range to a local buffer, add a
      string terminator, and invoke the constructor from string, base and precision.

      The second constructor calls the first one with a *base* value of 10.

      :param begin: the start of the input range.
      :param end: the end of the input range.
      :param base: the base used in the string representation.
      :param p: the desired precision.

      :exception unspecified: any exception thrown by the constructor from string, or by memory
        allocation errors in standard containers.

   .. cpp:function:: explicit complex(const mpc_t c)

      Constructor from an :cpp:type:`mpc_t`.

      This constructor will initialise ``this`` with an exact deep copy of *c*.

      .. warning::

         It is the user's responsibility to ensure that *c* has been correctly initialised
         with a precision which is:

         * the same for the real and imaginary parts,
         * within the bounds established by :cpp:func:`mppp::real_prec_min()`
           and :cpp:func:`mppp::real_prec_max()`.

      :param c: the :cpp:type:`mpc_t` that will be deep-copied.

   .. cpp:function:: explicit complex(mpc_t &&c)

      Move constructor from an :cpp:type:`mpc_t`.

      This constructor will initialise ``this`` with a shallow copy of *c*.

      .. warning::

         It is the user's responsibility to ensure that *c* has been correctly initialised
         with a precision which is:

         * the same for the real and imaginary parts,
         * within the bounds established by :cpp:func:`mppp::real_prec_min()`
           and :cpp:func:`mppp::real_prec_max()`.

         Additionally, the user must ensure that, after construction, ``mpc_clear()`` is never
         called on *c*: the resources previously owned by *c* are now owned by ``this``, which
         will take care of releasing them when the destructor is called.

      .. note::

         Due to a compiler bug, this constructor is not available on Microsoft Visual Studio.

      :param c: the :cpp:type:`mpc_t` that will be moved.

   .. cpp:function:: ~complex()

      Destructor.

      The destructor will run sanity checks in debug mode.

   .. cpp:function:: complex &operator=(const complex &other)
   .. cpp:function:: complex &operator=(complex &&other) noexcept

      Copy and move assignment operators.

      :param other: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: template <complex_interoperable T> complex &operator=(T &&x)

      The generic assignment operator will set ``this`` to the value of *x*.

      The precision of ``this`` will be set according to the same
      heuristics described in the generic constructor.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the generic assignment operator
        of :cpp:class:`~mppp::real`.

   .. cpp:function:: complex &operator=(const mpc_t c)

      Copy assignment from :cpp:type:`mpc_t`.

      This operator will set ``this`` to a deep copy of *c*.

      .. warning::

         It is the user's responsibility to ensure that *c* has been correctly initialised
         with a precision which is:

         * the same for the real and imaginary parts,
         * within the bounds established by :cpp:func:`mppp::real_prec_min()`
           and :cpp:func:`mppp::real_prec_max()`.

      :param c: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: complex &operator=(mpc_t &&c)

      Move assignment from :cpp:type:`mpc_t`.

      This operator will set ``this`` to a shallow copy of *c*.

      .. warning::

         It is the user's responsibility to ensure that *c* has been correctly initialised
         with a precision which is:

         * the same for the real and imaginary parts,
         * within the bounds established by :cpp:func:`mppp::real_prec_min()`
           and :cpp:func:`mppp::real_prec_max()`.

         Additionally, the user must ensure that, after the assignment, ``mpc_clear()`` is never
         called on *c*: the resources previously owned by *c* are now owned by ``this``, which
         will take care of releasing them when the destructor is called.

      .. note::

         Due to a compiler bug, this operator is not available on Microsoft Visual Studio.

      :param c: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: bool is_valid() const noexcept

      Check validity.

      A :cpp:class:`~mppp::complex` becomes invalid after it is used
      as an argument to the move constructor, or after the extraction
      of the real/imaginary parts via the move overload
      of :cpp:func:`~mppp::get_real_imag()`.

      :return: ``true`` if ``this`` is valid, ``false`` otherwise.

   .. cpp:function:: complex &set(const complex &other)

      Set to another :cpp:class:`~mppp::complex`.

      This member function will set ``this`` to the value of *other*. Contrary to the copy assignment operator,
      the precision of the assignment is dictated by the precision of ``this``, rather than
      the precision of *other*. Consequently, the precision of ``this`` will not be altered by the
      assignment, and a rounding might occur, depending on the values
      and the precisions of the operands.

      This function is a thin wrapper around the ``mpc_set()`` assignment function from the MPC API.

      :param other: the value to which ``this`` will be set.

      :return: a reference to ``this``.

   .. cpp:function:: template <complex_interoperable T> complex &set(const T &x)

      Generic setter.

      This member function will set ``this`` to the value of *x*. Contrary to the generic assignment operator,
      the precision of the assignment is dictated by the precision of ``this``, rather than
      being deduced from the type and value of *x*. Consequently, the precision of ``this`` will not be altered
      by the assignment, and a rounding might occur, depending on the operands.

      :param x: the value to which ``this`` will be set.

      :return: a reference to ``this``.

   .. cpp:function:: template <string_type T> complex &set(const T &s, int base = 10)

      Setter to string.

      This member function will set ``this`` to the value represented by *s*, which will
      be interpreted as a complex number in base *base* (the expected string representations
      for a complex number are detailed in the documentation of the constructor from string).
      *base* must be either 0 (in which case the base is
      automatically deduced), or a value in the :math:`\left[ 2,62 \right]` range.
      The precision of the assignment is dictated by the
      precision of ``this``, and a rounding might thus occur.

      If *s* is not a valid representation of a complex number in base *base*, the real and imaginary
      parts of ``this`` will be set to NaN and an error will be raised.

      :param s: the string to which ``this`` will be set.
      :param base: the base used in the string representation.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if *s* cannot be parsed as a complex value, or if the value
        of *base* is invalid.
      :exception unspecified: any exception thrown by memory allocation errors in standard containers.

   .. cpp:function:: complex &set(const char *begin, const char *end, int base = 10)

      Set to character range.

      This setter will set ``this`` to the content of the input half-open range,
      which is interpreted as the string representation of a complex value in base *base*.

      Internally, the setter will copy the content of the range to a local buffer, add a
      string terminator, and invoke the setter to string.

      :param begin: the start of the input range.
      :param end: the end of the input range.
      :param base: the base used in the string representation.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the setter to string, or by memory
        allocation errors in standard containers.

   .. cpp:function:: complex &set(const mpc_t c)

      Set to an :cpp:type:`mpc_t`.

      This member function will set ``this`` to the value of *c*. Contrary to the corresponding assignment operator,
      the precision of the assignment is dictated by the precision of ``this``, rather than
      the precision of *c*. Consequently, the precision of ``this`` will not be altered by the
      assignment, and a rounding might occur, depending on the values
      and the precisions of the operands.

      This function is a thin wrapper around the ``mpc_set()`` assignment function from the MPC API.

      .. warning::

         It is the user's responsibility to ensure that *c* has been correctly initialised
         with a precision which is:

         * the same for the real and imaginary parts,
         * within the bounds established by :cpp:func:`mppp::real_prec_min()`
           and :cpp:func:`mppp::real_prec_max()`.

      :param c: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: complex &set_nan()

      Set to NaN.

      This member function will set both the real and imaginary parts of ``this``
      to NaN.

      :return: a reference to ``this``.

   .. cpp:function:: const mpc_struct_t *get_mpc_t() const
   .. cpp:function:: mpc_struct_t *_get_mpc_t()

      Getters for the internal :cpp:type:`mpc_t` instance.

      These member functions will return a const or mutable pointer
      to the internal :cpp:type:`mpc_t` instance.

      .. warning::

         When using the mutable getter, it is the user's responsibility to ensure
         that the internal MPC structure is kept in a state which respects the invariants
         of the :cpp:class:`~mppp::complex` class. Specifically, the precision value
         must be the same for the real and imaginary parts and
         within the bounds established by :cpp:func:`mppp::real_prec_min()` and
         :cpp:func:`mppp::real_prec_max()`, and upon destruction a :cpp:class:`~mppp::complex`
         object must contain a valid :cpp:type:`mpc_t` object.

      :return: a const or mutable pointer to the internal MPC structure.

   .. cpp:function:: bool zero_p() const
   .. cpp:function:: bool inf_p() const
   .. cpp:function:: bool number_p() const
   .. cpp:function:: bool is_one() const

      Detect special values.

      These member functions will return ``true`` if ``this`` is, respectively:

      * zero,
      * a complex infinity (that is, at least one component of ``this``
        is an infinity),
      * a finite number (that is, both components of ``this`` are finite
        numbers),
      * one,

      ``false`` otherwise.

      .. versionadded:: 0.21

         The ``inf_p()`` and ``number_p()`` functions.

      :return: the result of the detection.

   .. cpp:function:: mpfr_prec_t get_prec() const

      Precision getter.

      :return: the precision of ``this``.

   .. cpp:function:: complex &set_prec(mpfr_prec_t p)

      Destructively set the precision

      This member function will set the precision of ``this`` to exactly *p* bits. The value
      of the real and imaginary parts of ``this`` will be set to NaN.

      :param p: the desired precision.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if *p* is outside the range established by
        :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`.

   .. cpp:function:: complex &prec_round(mpfr_prec_t p)

      Set the precision maintaining the current value.

      This member function will set the precision of ``this`` to exactly *p* bits. If *p*
      is smaller than the current precision of ``this``, a rounding operation will be performed,
      otherwise the current value will be preserved exactly.

      :param p: the desired precision.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if *p* is outside the range established by
        :cpp:func:`mppp::real_prec_min()` and :cpp:func:`mppp::real_prec_max()`.

   .. cpp:function:: template <complex_convertible T> explicit operator T() const

      Generic conversion operator.

      This operator will convert ``this`` to ``T``.

      Conversion to ``bool`` returns ``false`` if ``this`` is zero, ``true`` otherwise.

      Conversion to other real-valued :cpp:concept:`~mppp::complex_convertible` types
      will attempt to convert the real part of ``this`` to ``T`` via the cast operator
      of :cpp:class:`~mppp::real` (unless ``T`` is :cpp:class:`~mppp::real`, in which case a copy
      of the real part of ``this`` will be returned). If the imaginary part of ``this`` is nonzero,
      a domain error will be raised.

      Conversion to complex-valued :cpp:concept:`~mppp::complex_convertible` types
      is also implemented on top of the conversion operator of :cpp:class:`~mppp::real`.

      :return: ``this`` converted to ``T``.

      :exception std\:\:domain_error: if ``T`` is a real-valued type other than ``bool`` and
        the imaginary part of ``this`` is not zero.
      :exception unspecified: any exception raised by the cast operator of :cpp:class:`~mppp::real`.

   .. cpp:function:: template <complex_convertible T> bool get(T &rop) const

      Generic conversion function.

      This member function, similarly to the conversion operator, will convert ``this`` to
      ``T``, storing the result of the conversion into *rop*. Differently
      from the conversion operator, this function does not raise any exception: if the conversion is successful, the
      function will return ``true``, otherwise the function will return ``false``. If the conversion fails,
      *rop* will not be altered.

      :param rop: the variable which will store the result of the conversion.

      :return: ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail in the ways
        specified in the documentation of the conversion operator.

   .. cpp:function:: std::string to_string(int base = 10) const

      Conversion to string.

      This member function will convert ``this`` to a string representation in base *base*. The returned string is guaranteed
      to produce exactly the original value when used in one of the constructors from string of
      :cpp:class:`~mppp::complex` (provided that the original precision and base are used in the construction).

      :param base: the base to be used for the string representation.

      :return: ``this`` converted to a string.

      :exception unspecified: any exception thrown by :cpp:func:`mppp::real::to_string()`.

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

   .. cpp:function:: std::pair<real, real> get_real_imag() const &
   .. cpp:function:: std::pair<real, real> get_real_imag() &&

      Copy or move out the real and imaginary parts.

      After the invocation of the second overload, the only valid operations on ``this`` are
      destruction, copy/move assignment and the invocation of the :cpp:func:`~mppp::complex::is_valid()`
      member function. After re-assignment, ``this`` can be used normally again.

      :return: a pair containing the real and imaginary parts of ``this``, represented
        as :cpp:class:`~mppp::real` objects.

   .. cpp:function:: re_cref real_cref() const
   .. cpp:function:: im_cref imag_cref() const
   .. cpp:function:: re_ref real_ref()
   .. cpp:function:: im_ref imag_ref()

      .. note::

        These member functions are available only if at least C++17 is being used.

      These member functions facilitate the creation of ``const`` or mutable reference
      wrappers to the real and imaginary parts of ``this``.

      See the documentation of :cpp:class:`~mppp::complex::re_cref`,
      :cpp:class:`~mppp::complex::im_cref`, :cpp:class:`~mppp::complex::re_ref`
      and :cpp:class:`~mppp::complex::im_ref` for information on how to
      use these classes.

      :return: a ``const`` or mutable reference wrapper to the real or
        imaginary part of ``this``.

   .. cpp:function:: complex &neg()
   .. cpp:function:: complex &conj()
   .. cpp:function:: complex &proj()
   .. cpp:function:: complex &abs()
   .. cpp:function:: complex &norm()
   .. cpp:function:: complex &arg()
   .. cpp:function:: complex &sqr()
   .. cpp:function:: complex &mul_i(int s = 0)
   .. cpp:function:: complex &inv()

      .. note::

         The ``inv()`` function is available only if mp++ was
         configured with the ``MPPP_WITH_ARB`` option enabled.

      In-place basic aritmetic functions.

      These member functions will set ``this`` to, respectively:

      * :math:`-z`,
      * :math:`\overline{z}`,
      * the projection of :math:`z` into Riemann sphere,
      * :math:`\left| z \right|`,
      * :math:`\left| z \right|^2`,
      * :math:`\arg z`,
      * :math:`z^2`,
      * :math:`\imath z` (if :math:`s\geq 0`) or :math:`-\imath z` (if :math:`s < 0`),
      * :math:`1/z`,

      where :math:`z` is the current value of ``this``.

      The ``inv()`` function follows the conventions laid out in Annex G
      of the C99 standard when ``this`` is zero or an infinity.

      .. versionadded:: 0.21

         The ``inv()`` function.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if the conversion between Arb and MPC types
        fails because of (unlikely) overflow conditions.

   .. cpp:function:: complex &sqrt()
   .. cpp:function:: complex &rec_sqrt()

      .. note::

         The ``rec_sqrt()`` function is available only if mp++ was
         configured with the ``MPPP_WITH_ARB`` option enabled.

      In-place roots.

      These member functions will set ``this`` to, respectively:

      * :math:`\sqrt{z}`,
      * :math:`1 / \sqrt{z}`,

      where :math:`z` is the current value of ``this``.

      The ``rec_sqrt()`` function follows the conventions laid out in Annex G
      of the C99 standard when ``this`` is zero or an infinity.

      .. versionadded:: 0.21

         The ``rec_sqrt()`` function.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if the conversion between Arb and MPC types
        fails because of (unlikely) overflow conditions.

   .. cpp:function:: complex &exp()
   .. cpp:function:: complex &log()
   .. cpp:function:: complex &log10()

      In-place logarithms and exponentials.

      These member functions will set ``this`` to, respectively:

      * :math:`e^z`,
      * :math:`\log z`,
      * :math:`\log_{10} z`,

      where :math:`z` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: complex &sin()
   .. cpp:function:: complex &cos()
   .. cpp:function:: complex &tan()
   .. cpp:function:: complex &asin()
   .. cpp:function:: complex &acos()
   .. cpp:function:: complex &atan()

      In-place (inverse) trigonometric functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\sin{z}`,
      * :math:`\cos{z}`,
      * :math:`\tan{z}`,
      * :math:`\arcsin{z}`,
      * :math:`\arccos{z}`,
      * :math:`\arctan{z}`,

      where :math:`z` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: complex &sinh()
   .. cpp:function:: complex &cosh()
   .. cpp:function:: complex &tanh()
   .. cpp:function:: complex &asinh()
   .. cpp:function:: complex &acosh()
   .. cpp:function:: complex &atanh()

      In-place (inverse) hyperbolic functions.

      These member functions will set ``this`` to, respectively:

      * :math:`\sinh{z}`,
      * :math:`\cosh{z}`,
      * :math:`\tanh{z}`,
      * :math:`\operatorname{arcsinh}{z}`,
      * :math:`\operatorname{arccosh}{z}`,
      * :math:`\operatorname{arctanh}{z}`,

      where :math:`z` is the current value of ``this``.

      :return: a reference to ``this``.

   .. cpp:function:: complex &agm1()

      .. versionadded:: 0.21

      .. note::

         This function is available only if mp++ was
         configured with the ``MPPP_WITH_ARB`` option enabled.

      Arithmetic-geometric mean.

      This member function will set ``this`` to
      :math:`\operatorname{agm}\left( 1, z \right)`,
      where :math:`z` is the current value of ``this``.

      :return: a reference to ``this``.

      :exception std\:\:invalid_argument: if the conversion between Arb and MPC types
        fails because of (unlikely) overflow conditions.

Types
-----

.. cpp:type:: mpc_t

   This is the type used by the MPC library to represent multiprecision complex numbers.
   It is defined as an array of size 1 of an unspecified structure
   (see :cpp:type:`~mppp::mpc_struct_t`).

.. cpp:type:: mppp::mpc_struct_t = std::remove_extent_t<mpc_t>

   The C structure used by MPC to represent arbitrary-precision complex numbers.
   The MPC type :cpp:type:`mpc_t` is defined as an array of size 1 of this structure.

.. cpp:enum-class:: mppp::complex_prec_t : mpfr_prec_t

   A strongly-typed counterpart to :cpp:type:`mpfr_prec_t`, used in the constructors of
   :cpp:class:`~mppp::complex` in order to avoid ambiguities during overload resolution.

Concepts
--------

.. cpp:concept:: template <typename T> mppp::cvr_complex

   This concept is satisfied if the type ``T``, after the removal of reference and cv qualifiers,
   is the same as :cpp:class:`mppp::complex`.

.. cpp:concept:: template <typename T> mppp::rv_complex_interoperable

   This concept is satisfied if ``T``, after the removal of reference and cv qualifiers,
   is a real-valued type that can interoperate with :cpp:class:`~mppp::complex`.
   Specifically, this concept will be ``true`` if ``T``, after the removal of reference and cv qualifiers,
   is either:

   * a :cpp:concept:`~mppp::cpp_arithmetic` type, or
   * an :cpp:class:`~mppp::integer`, or
   * a :cpp:class:`~mppp::rational`, or
   * :cpp:class:`~mppp::real128`, or
   * :cpp:class:`~mppp::real`.

.. cpp:concept:: template <typename T> mppp::complex_interoperable

   This concept is satisfied if ``T``, after the removal of reference and cv qualifiers,
   is a type that can interoperate with :cpp:class:`~mppp::complex`.
   Specifically, this concept will be ``true`` if ``T``, after the removal of reference and cv qualifiers,
   is either:

   * an :cpp:concept:`~mppp::rv_complex_interoperable` type, or
   * a :cpp:concept:`~mppp::cpp_complex` type, or
   * :cpp:class:`~mppp::complex128`.

.. cpp:concept:: template <typename T> mppp::complex_convertible

   This concept is satisfied if ``T`` is a type which a :cpp:class:`~mppp::complex`
   can be converted to. Specifically, this concept will be true if ``T``
   is a :cpp:concept:`~mppp::complex_interoperable` type which is not a reference
   or cv qualified.

.. cpp:concept:: template <typename... Args> mppp::complex_set_args

   This concept is satisfied if the types in the parameter pack ``Args``
   can be used as argument types in one of the :cpp:func:`mppp::complex::set()` member function overloads.
   In other words, this concept is satisfied if the expression

   .. code-block:: c++

      c.set(x, y, z, ...);

   is valid (where ``c`` is a non-const :cpp:class:`~mppp::complex` and ``x``, ``y``, ``z``, etc. are const
   references to the types in ``Args``).

.. cpp:concept:: template <typename T, typename U> mppp::complex_eq_op_types

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   equality/inequality :ref:`operators <complex_operators>`
   involving :cpp:class:`~mppp::complex`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` both satisfy :cpp:concept:`~mppp::cvr_complex`, or
   * one type satisfies :cpp:concept:`~mppp::cvr_complex` and the other type
     satisfies :cpp:concept:`~mppp::complex_interoperable`.

.. cpp:concept:: template <typename T, typename U> mppp::complex_op_types

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <complex_operators>` and :ref:`functions <complex_functions>`
   involving :cpp:class:`~mppp::complex`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` satisfy :cpp:concept:`~mppp::complex_eq_op_types`, or
   * one type satisfies :cpp:concept:`~mppp::cvr_real` and the other type,
     after the removal of reference and cv qualifiers, either satisfies
     :cpp:concept:`~mppp::cpp_complex` or it is :cpp:class:`~mppp::complex128`.

.. cpp:concept:: template <typename T, typename U> mppp::complex_in_place_op_types

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic in-place :ref:`operators <complex_operators>`
   involving :cpp:class:`~mppp::complex`. Specifically, the concept will be ``true`` if
   ``T`` and ``U`` satisfy :cpp:concept:`~mppp::complex_op_types` and ``T``, after the removal
   of reference, is not const.

.. _complex_functions:

Functions
---------

.. _complex_components_access:

Components access
~~~~~~~~~~~~~~~~~

.. cpp:function:: std::pair<mppp::real, mppp::real> mppp::get_real_imag(const mppp::complex &c)
.. cpp:function:: std::pair<mppp::real, mppp::real> mppp::get_real_imag(mppp::complex &&c)

   Copy or move out the real and imaginary parts of a :cpp:class:`~mppp::complex`.

   After the invocation of the second overload, the only valid operations on *c* are
   destruction, copy/move assignment and the invocation of the :cpp:func:`~mppp::complex::is_valid()`
   member function. After re-assignment, *c* can be used normally again.

   :param c: the input parameter.

   :return: a pair containing the real and imaginary parts of *c*, represented
      as :cpp:class:`~mppp::real` objects.

.. cpp:class:: mppp::complex::re_cref

   ``const`` reference wrapper for the
   real part of a :cpp:class:`~mppp::complex`.

   After construction from a :cpp:class:`~mppp::complex`,
   an instance of this class can be used to perform read-only operations
   on the real part of the :cpp:class:`~mppp::complex` number
   via the :cpp:class:`~mppp::real` API.

   Here's a typical usage pattern:

   .. code-block:: c++

      complex c{1, 2};

      {
         // Place the re_cref object in a new scope
         // in order to minimise its lifetime.
         complex::re_cref re{c};

         // We can use 're' as-if it was a pointer
         // to an mppp::real.
         std::cout << "The real part of c is " << *re << '\n';
         std::cout << "Is the real part of c one? " << re->is_one() << '\n';
      }

   Because :cpp:class:`~mppp::complex::re_cref` shares data
   with a :cpp:class:`~mppp::complex`, special care should
   be taken in order to avoid undefined behaviour. In particular:

   * the lifetime of a :cpp:class:`~mppp::complex::re_cref`
     is tied to the lifetime of the :cpp:class:`~mppp::complex`
     from which it was constructed. Thus, a
     :cpp:class:`~mppp::complex::re_cref` must always be destroyed
     before the destruction of the :cpp:class:`~mppp::complex`
     from which it was constructed;
   * during the lifetime of a :cpp:class:`~mppp::complex::re_cref`,
     only ``const`` operations on the :cpp:class:`~mppp::complex`
     from which it was constructed are allowed.

   The easiest way to avoid mistakes is to follow these guidelines:

   * create a :cpp:class:`~mppp::complex::re_cref` object in a new scope,
   * in the new scope, don't perform non-``const`` operations on the
     :cpp:class:`~mppp::complex` from which the
     :cpp:class:`~mppp::complex::re_cref` object was created,
   * exit the new scope as soon as you are finished interacting
     with the :cpp:class:`~mppp::complex::re_cref` object.

   When using at least C++17, the creation of a :cpp:class:`~mppp::complex::re_cref`
   can also be accomplished via the :cpp:func:`mppp::complex::real_cref()`
   and :cpp:func:`mppp::real_cref()` functions (which slightly reduce
   the amount of typing).

   .. cpp:function:: explicit re_cref(const complex &c)

      Constructor.

      This is the only constructor available for a :cpp:class:`~mppp::complex::re_cref`
      object. It will create a reference to the real part of *c*.

      :param c: the input :cpp:class:`~mppp::complex`.

   .. cpp:function:: re_cref(const re_cref &) = delete
   .. cpp:function:: re_cref(re_cref &&) = delete
   .. cpp:function:: re_cref &operator=(const re_cref &) = delete
   .. cpp:function:: re_cref &operator=(re_cref &&) = delete

      :cpp:class:`~mppp::complex::re_cref` cannot be copied or moved.

   .. cpp:function:: const real &operator*() const
   .. cpp:function:: const real *operator->() const

      Dereference and arrow operators.

      These operators allow to interact with a :cpp:class:`~mppp::complex::re_cref`
      as-if it was a ``const`` pointer to a :cpp:class:`~mppp::real` object.

      :return: a ``const`` reference/pointer to a :cpp:class:`~mppp::real` object.

.. cpp:class:: mppp::complex::im_cref

   ``const`` reference wrapper for the imaginary part of a :cpp:class:`~mppp::complex`.

   The API is identical to :cpp:class:`~mppp::complex::re_cref`.

.. cpp:class:: mppp::complex::re_ref

   Mutable reference wrapper for the real part of a :cpp:class:`~mppp::complex`.

   After construction from a :cpp:class:`~mppp::complex`,
   an instance of this class can be used to perform non-``const`` operations
   on the real part of the :cpp:class:`~mppp::complex` number
   via the :cpp:class:`~mppp::real` API.

   Here's a typical usage pattern:

   .. code-block:: c++

      complex c{1, 2};

      {
         // Place the re_ref object in a new scope
         // in order to minimise its lifetime.
         complex::re_ref re{c};

         // We can use 're' as-if it was a pointer
         // to an mppp::real.
         *re += 5;
      }

      std::cout << "c is now: " << c << '\n'; // Will print '(6.000..., 2.000...)'.

   On construction, :cpp:class:`~mppp::complex::re_ref` takes ownership of the real
   part of a :cpp:class:`~mppp::complex`. On destruction,
   :cpp:class:`~mppp::complex::re_ref` will automatically transfer its content back
   to the real part of the :cpp:class:`~mppp::complex` from
   which it was constructed. In order to avoid misuses, it is thus
   important to keep the following in mind:

   * the lifetime of a :cpp:class:`~mppp::complex::re_ref`
     is tied to the lifetime of the :cpp:class:`~mppp::complex`
     from which it was constructed. Thus, a
     :cpp:class:`~mppp::complex::re_ref` must always be destroyed
     before the destruction of the :cpp:class:`~mppp::complex`
     from which it was constructed;
   * during the lifetime of a :cpp:class:`~mppp::complex::re_ref`,
     neither ``const`` nor mutable operations can be performed
     on the :cpp:class:`~mppp::complex` from which it was constructed.
     The **only** exception to this rule is that it is allowed
     to create an :cpp:class:`~mppp::complex::im_cref` or an
     :cpp:class:`~mppp::complex::im_ref` to the :cpp:class:`~mppp::complex`
     (so that it is possible to operate on both the real and imaginary parts
     of a :cpp:class:`~mppp::complex` at the same time);
   * it is the user's responsibility to ensure that any modification to the
     precision of the real part is propagated to the imaginary part
     of the :cpp:class:`~mppp::complex` (so that the :cpp:class:`~mppp::complex`
     class invariant dictating that the real and imaginary parts must always
     have the same precision is preserved).

   The easiest way to avoid mistakes is to follow these guidelines:

   * create a :cpp:class:`~mppp::complex::re_ref` object in a new scope,
   * in the new scope, never reuse the :cpp:class:`~mppp::complex`
     from which the :cpp:class:`~mppp::complex::re_ref` object was created.
     The **only** exception is that you can create an
     :cpp:class:`~mppp::complex::im_cref` or an
     :cpp:class:`~mppp::complex::im_ref` to the same :cpp:class:`~mppp::complex`;
   * ensure that any modification to the precision of the real part is propagated
     to the imaginary part;
   * exit the new scope as soon as you are finished interacting
     with the :cpp:class:`~mppp::complex::re_ref` object.

   When using at least C++17, the creation of a :cpp:class:`~mppp::complex::re_ref`
   can also be accomplished via the :cpp:func:`mppp::complex::real_ref()`
   and :cpp:func:`mppp::real_ref()` functions (which slightly reduce
   the amount of typing).

   .. cpp:function:: explicit re_ref(complex &c)

      Constructor.

      This is the only constructor available for a :cpp:class:`~mppp::complex::re_ref`
      object. It will take ownership of the real part of a :cpp:class:`~mppp::complex`.

      :param c: the input :cpp:class:`~mppp::complex`.

   .. cpp:function:: re_ref(const re_ref &) = delete
   .. cpp:function:: re_ref(re_ref &&) = delete
   .. cpp:function:: re_ref &operator=(const re_ref &) = delete
   .. cpp:function:: re_ref &operator=(re_ref &&) = delete

      :cpp:class:`~mppp::complex::re_ref` cannot be copied or moved.

   .. cpp:function:: ~re_ref()

      The destructor will transfer the (possibly modified) real part
      back to the :cpp:class:`~mppp::complex`
      from which the :cpp:class:`~mppp::complex::re_ref` object was created.

   .. cpp:function:: real &operator*()
   .. cpp:function:: real *operator->()

      Dereference and arrow operators.

      These operators allow to interact with a :cpp:class:`~mppp::complex::re_ref`
      as-if it was a pointer to a :cpp:class:`~mppp::real` object.

      :return: a reference/pointer to a :cpp:class:`~mppp::real` object.

.. cpp:class:: mppp::complex::im_ref

   Mutable reference wrapper for the imaginary part of a :cpp:class:`~mppp::complex`.

   The API is identical to :cpp:class:`~mppp::complex::re_ref`.

.. cpp:function:: mppp::complex::re_cref mppp::real_cref(const mppp::complex &c)
.. cpp:function:: mppp::complex::im_cref mppp::imag_cref(const mppp::complex &c)
.. cpp:function:: mppp::complex::re_ref mppp::real_ref(mppp::complex &c)
.. cpp:function:: mppp::complex::im_ref mppp::imag_ref(mppp::complex &c)

   .. note::

      These functions are available only if at least C++17 is being used.

   These functions facilitate the creation of ``const`` or mutable reference
   wrappers to the real and imaginary parts of *c*.

   See the documentation of :cpp:class:`~mppp::complex::re_cref`,
   :cpp:class:`~mppp::complex::im_cref`, :cpp:class:`~mppp::complex::re_ref`
   and :cpp:class:`~mppp::complex::im_ref` for information on how to
   use these classes.

   :param c: the input parameter.

   :return: a ``const`` or mutable reference wrapper to the real or
      imaginary part of *c*.

Precision handling
~~~~~~~~~~~~~~~~~~

.. cpp:function:: mpfr_prec_t mppp::get_prec(const mppp::complex &c)

   Get the precision of a :cpp:class:`~mppp::complex`.

   :param c: the input argument.

   :return: the precision of *c*.

.. cpp:function:: void mppp::set_prec(mppp::complex &c, mpfr_prec_t p)
.. cpp:function:: void mppp::prec_round(mppp::complex &c, mpfr_prec_t p)

   Set the precision of a :cpp:class:`~mppp::complex`.

   The first variant will set the precision of *c* to exactly *p* bits. The value
   of *c* will be set to NaN.

   The second variant will preserve the current value of *c*, performing
   a rounding operation if *p* is less than the current precision of *c*.

   :param c: the input argument.
   :param p: the desired precision.

   :exception unspecified: any exception thrown by :cpp:func:`mppp::complex::set_prec()`
     or :cpp:func:`mppp::complex::prec_round()`.

Assignment
~~~~~~~~~~

.. cpp:function:: template <mppp::complex_set_args... Args> mppp::complex &mppp::set(mppp::complex &c, const Args &... args)

   Generic setter.

   This function will use the arguments *args* to set the value of the :cpp:class:`~mppp::complex` *c*,
   using one of the available :cpp:func:`mppp::complex::set()` overloads. That is,
   the body of this function is equivalent to

   .. code-block:: c++

      return c.set(args...);

   The input arguments must satisfy the :cpp:concept:`mppp::complex_set_args` concept.

   :param c: the return value.
   :param args: the arguments that will be passed to :cpp:func:`mppp::complex::set()`.

   :return: a reference to *c*.

   :exception unspecified: any exception thrown by the invoked :cpp:func:`mppp::complex::set()` overload.

.. cpp:function:: mppp::complex &mppp::set_nan(mppp::complex &c)

   Set to NaN.

   This function will set both the real and imaginary parts of *c* to NaN.

   :param c: the input argument.

   :return: a reference to *c*.

.. cpp:function:: mppp::complex &mppp::set_rootofunity(mppp::complex &c, unsigned long n, unsigned long k)

   Set to a power of the standard primitive n-th root of unity.

   This function will set *c* to :math:`e^{2\pi\imath k/n}`. The precision
   of *c* will not be altered.

   :param c: the input argument.
   :param n: the root order.
   :param k: the exponent.

.. cpp:function:: void mppp::swap(mppp::complex &a, mppp::complex &b) noexcept

   Swap efficiently *a* and *b*.

   :param a: the first argument.
   :param b: the second argument.

Conversion
~~~~~~~~~~

.. cpp:function:: template <mppp::complex_convertible T> bool mppp::get(T &rop, const mppp::complex &c)

   Generic conversion function.

   This function will convert the input :cpp:class:`~mppp::complex` *c* to
   ``T``, storing the result of the conversion into *rop*.
   If the conversion is successful, the function
   will return ``true``, otherwise the function will return ``false``. If the conversion fails, *rop* will
   not be altered.

   :param rop: the variable which will store the result of the conversion.
   :param c: the input argument.

   :return: ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail in the ways
      specified in the documentation of the conversion operator for :cpp:class:`~mppp::complex`.

.. cpp:function:: std::size_t mppp::get_str_ndigits(const mppp::complex &c, int base = 10)

   .. versionadded:: 0.25

   .. note::

      This function is available from MPFR 4.1 onwards.

   Minimum number of digits necessary for round-tripping.

   This member function will return the minimum number of digits
   necessary to ensure that the current value of *c*
   can be recovered exactly from
   a string representation in the given *base*.

   :param c: the input value.
   :param base: the base to be used for the string representation.

   :return: the minimum number of digits necessary for round-tripping.

   :exception std\:\:invalid_argument: if *base* is not in the :math:`\left[ 2,62 \right]` range.

Arithmetic
~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_complex T, mppp::cvr_complex U> mppp::complex &mppp::add(mppp::complex &rop, T &&a, U &&b)
.. cpp:function:: template <mppp::cvr_complex T, mppp::cvr_complex U> mppp::complex &mppp::sub(mppp::complex &rop, T &&a, U &&b)
.. cpp:function:: template <mppp::cvr_complex T, mppp::cvr_complex U> mppp::complex &mppp::mul(mppp::complex &rop, T &&a, U &&b)
.. cpp:function:: template <mppp::cvr_complex T, mppp::cvr_complex U> mppp::complex &mppp::div(mppp::complex &rop, T &&a, U &&b)

   Basic :cpp:class:`~mppp::complex` binary arithmetic.

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

.. cpp:function:: template <mppp::cvr_complex T, mppp::cvr_complex U, mppp::cvr_complex V> mppp::complex &mppp::fma(mppp::complex &rop, T &&a, U &&b, V &&c)

   Quaternary :cpp:class:`~mppp::complex` multiply-add.

   This function will set *rop* to :math:`a \times b + c`.

   The precision of the result will be set to the largest precision among the operands.

   :param rop: the return value.
   :param a: the first operand.
   :param b: the second operand.
   :param c: the third operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_complex T, mppp::cvr_complex U, mppp::cvr_complex V> mppp::complex mppp::fma(T &&a, U &&b, V &&c)

   Ternary :cpp:class:`~mppp::complex` multiply-add.

   This function will return :math:`a \times b + c`.

   The precision of the result will be the largest precision among the operands.

   :param a: the first operand.
   :param b: the second operand.
   :param c: the third operand.

   :return: :math:`a \times b + c`.

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::neg(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::conj(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::proj(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::sqr(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::mul_i(mppp::complex &rop, T &&z, int s = 0)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::inv(mppp::complex &rop, T &&z)
.. cpp:function:: mppp::real &mppp::abs(mppp::real &rop, const mppp::complex &z)
.. cpp:function:: mppp::real &mppp::norm(mppp::real &rop, const mppp::complex &z)
.. cpp:function:: mppp::real &mppp::arg(mppp::real &rop, const mppp::complex &z)

   .. note::

      The ``inv()`` function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Basic binary arithmetic functions.

   These functions will set *rop* to, respectively:

   * :math:`-z`,
   * :math:`\overline{z}`,
   * the projection of :math:`z` into Riemann sphere,
   * :math:`z^2`,
   * :math:`\imath z` (if :math:`s\geq 0`) or :math:`-\imath z` (if :math:`s < 0`),
   * :math:`1/z`,
   * :math:`\left| z \right|`,
   * :math:`\left| z \right|^2`,
   * :math:`\arg z`.

   The precision of the result will be equal to the precision of *z*.

   The ``inv()`` function follows the conventions laid out in Annex G
   of the C99 standard when *z* is zero or an infinity.

   .. versionadded:: 0.21

      The ``inv()`` function.

   :param rop: the return value.
   :param z: the argument.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPC types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::neg(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::conj(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::proj(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::sqr(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::mul_i(T &&z, int s = 0)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::inv(T &&z)
.. cpp:function:: mppp::real mppp::abs(const mppp::complex &z)
.. cpp:function:: mppp::real mppp::norm(const mppp::complex &z)
.. cpp:function:: mppp::real mppp::arg(const mppp::complex &z)

   .. note::

      The ``inv()`` function is available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Basic unary arithmetic functions.

   These functions will return, respectively:

   * :math:`-z`,
   * :math:`\overline{z}`,
   * the projection of :math:`z` into Riemann sphere,
   * :math:`z^2`,
   * :math:`\imath z` (if :math:`s\geq 0`) or :math:`-\imath z` (if :math:`s < 0`),
   * :math:`1/z`,
   * :math:`\left| z \right|`,
   * :math:`\left| z \right|^2`,
   * :math:`\arg z`.

   The precision of the result will be equal to the precision of *z*.

   The ``inv()`` function follows the conventions laid out in Annex G
   of the C99 standard when *z* is zero or an infinity.

   .. versionadded:: 0.21

      The ``inv()`` function.

   :param z: the argument.

   :return: the result of the operation.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPC types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::mul_2ui(mppp::complex &rop, T &&c, unsigned long n)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::mul_2si(mppp::complex &rop, T &&c, long n)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::div_2ui(mppp::complex &rop, T &&c, unsigned long n)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::div_2si(mppp::complex &rop, T &&c, long n)

   Ternary :cpp:class:`~mppp::complex` primitives for
   multiplication/division by powers of 2.

   These functions will set *rop* to, respectively:

   * :math:`c \times 2^n` (``mul_2`` variants),
   * :math:`\frac{c}{2^n}` (``div_2`` variants).

   The precision of the result will be equal to the precision of *c*.

   :param rop: the return value.
   :param c: the operand.
   :param n: the power of 2.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::mul_2ui(T &&c, unsigned long n)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::mul_2si(T &&c, long n)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::div_2ui(T &&c, unsigned long n)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::div_2si(T &&c, long n)

   Binary :cpp:class:`~mppp::complex` primitives for
   multiplication/division by powers of 2.

   These functions will return, respectively:

   * :math:`c \times 2^n` (``mul_2`` variants),
   * :math:`\frac{c}{2^n}` (``div_2`` variants).

   The precision of the result will be equal to the precision of *c*.

   :param c: the operand.
   :param n: the power of 2.

   :return: *c* multiplied/divided by :math:`2^n`.

Comparison
~~~~~~~~~~

.. cpp:function:: int mppp::cmpabs(const mppp::complex &a, const mppp::complex &b)

   Three-way comparison of absolute values.

   This function will compare *a* and *b*, returning:

   * zero if :math:`\left|a\right|=\left|b\right|`,
   * a negative value if :math:`\left|a\right|<\left|b\right|`,
   * a positive value if :math:`\left|a\right|>\left|b\right|`.

   If at least one NaN component is involved in the comparison, an error will be raised.

   :param a: the first operand.
   :param b: the second operand.

   :return: an integral value expressing how the absolute values of *a* and *b* compare.

   :exception std\:\:domain_error: if at least one of the components of *a* and *b* is NaN.

.. cpp:function:: bool mppp::zero_p(const mppp::complex &c)
.. cpp:function:: bool mppp::inf_p(const mppp::complex &c)
.. cpp:function:: bool mppp::number_p(const mppp::complex &c)
.. cpp:function:: bool mppp::is_one(const mppp::complex &c)

   Detect special values.

   These functions will return ``true`` if *c* is, respectively:

   * zero,
   * a complex infinity (that is, at least one component of *c*
     is an infinity),
   * a finite number (that is, both components of *c* are finite
     numbers),
   * one,

   ``false`` otherwise.

   .. versionadded:: 0.21

      The ``inf_p()`` and ``number_p()`` functions.

   :param c: the input argument.

   :return: the result of the detection.

Roots
~~~~~

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::sqrt(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::rec_sqrt(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::rootn_ui(mppp::complex &rop, T &&z, unsigned long k)

   .. note::

      The ``rec_sqrt()`` and ``rootn_ui()`` functions are available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Binary :cpp:class:`~mppp::complex` roots.

   These functions will set *rop* to, respectively:

   * :math:`\sqrt{z}`,
   * :math:`1 / \sqrt{z}`,
   * :math:`\sqrt[k]{z}`.

   The precision of the result will be equal to the precision of *z*.

   The ``rec_sqrt()`` function follows the conventions laid out in Annex G
   of the C99 standard when *z* is zero or an infinity.

   The ``rootn_ui()`` function computes the principal *k*-th root of *z*. If
   *k* is zero, *rop* will be set to NaN.

   .. versionadded:: 0.21

      The ``rec_sqrt()`` and ``rootn_ui()`` functions.

   :param rop: the return value.
   :param z: the operand.
   :param k: the degree of the root.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPC types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::sqrt(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::rec_sqrt(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::rootn_ui(T &&z, unsigned long k)

   .. note::

      The ``rec_sqrt()`` and ``rootn_ui()`` functions are available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

   Unary :cpp:class:`~mppp::complex` roots.

   These functions will return, respectively:

   * :math:`\sqrt{z}`,
   * :math:`1 / \sqrt{z}`,
   * :math:`\sqrt[k]{z}`.

   The precision of the result will be equal to the precision of *z*.

   The ``rec_sqrt()`` function follows the conventions laid out in Annex G
   of the C99 standard when *z* is zero or an infinity.

   The ``rootn_ui()`` function computes the principal *k*-th root of *z*. If
   *k* is zero, *rop* will be set to NaN.

   .. versionadded:: 0.21

      The ``rec_sqrt()`` and ``rootn_ui()`` functions.

   :param z: the operand.
   :param k: the degree of the root.

   :return: the result of the operation.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPC types
     fails because of (unlikely) overflow conditions.

Exponentiation
~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_complex T, mppp::cvr_complex U> mppp::complex &mppp::pow(mppp::complex &rop, T &&op1, U &&op2)

   Ternary exponentiation.

   This function will set *rop* to *op1* raised to the power of *op2*.
   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param op1: the base.
   :param op2: the exponent.

   :return: a reference to *rop*.

.. cpp:function:: template <typename T, mppp::complex_op_types<T> U> mppp::complex mppp::pow(T &&op1, U &&op2)

   Binary exponentiation.

   This function will compute and return *op1* raised to the power of *op2*.
   The precision of the result will be set to the largest precision among the operands.

   :param op1: the base.
   :param op2: the exponent.

   :return: *op1* raised to the power of *op2*.

Trigonometry
~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::sin(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::cos(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::tan(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::asin(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::acos(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::atan(mppp::complex &rop, T &&z)

   Binary basic (inverse) trigonometric functions.

   These functions will set *rop* to, respectively:

   * :math:`\sin{z}`,
   * :math:`\cos{z}`,
   * :math:`\tan{z}`,
   * :math:`\arcsin{z}`,
   * :math:`\arccos{z}`,
   * :math:`\arctan{z}`.

   The precision of the result will be equal to the precision of *z*.

   :param rop: the return value.
   :param z: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::sin(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::cos(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::tan(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::asin(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::acos(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::atan(T &&z)

   Unary basic (inverse) trigonometric functions.

   These functions will return, respectively:

   * :math:`\sin{z}`,
   * :math:`\cos{z}`,
   * :math:`\tan{z}`,
   * :math:`\arcsin{z}`,
   * :math:`\arccos{z}`,
   * :math:`\arctan{z}`.

   The precision of the result will be equal to the precision of *z*.

   :param z: the argument.

   :return: the trigonometric function of *z*.

.. cpp:function:: template <mppp::cvr_complex T> void mppp::sin_cos(mppp::complex &sop, mppp::complex &cop, T &&op)

   Simultaneous sine and cosine.

   This function will set *sop* and *cop* respectively to the sine and cosine of *op*.
   *sop* and *cop* must be distinct objects. The precision of *sop* and *rop* will be set to the
   precision of *op*.

   :param sop: the sine return value.
   :param cop: the cosine return value.
   :param op: the operand.

   :exception std\:\:invalid_argument: if *sop* and *cop* are the same object.

Hyperbolic functions
~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::sinh(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::cosh(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::tanh(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::asinh(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::acosh(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::atanh(mppp::complex &rop, T &&z)

   Binary basic (inverse) hyperbolic functions.

   These functions will set *rop* to, respectively:

   * :math:`\sinh{z}`,
   * :math:`\cosh{z}`,
   * :math:`\tanh{z}`,
   * :math:`\operatorname{arcsinh}{z}`,
   * :math:`\operatorname{arccosh}{z}`,
   * :math:`\operatorname{arctanh}{z}`.

   The precision of the result will be equal to the precision of *z*.

   :param rop: the return value.
   :param z: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::sinh(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::cosh(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::tanh(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::asinh(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::acosh(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::atanh(T &&z)

   Unary basic (inverse) hyperbolic functions.

   These functions will return, respectively:

   * :math:`\sinh{z}`,
   * :math:`\cosh{z}`,
   * :math:`\tanh{z}`,
   * :math:`\operatorname{arcsinh}{z}`,
   * :math:`\operatorname{arccosh}{z}`,
   * :math:`\operatorname{arctanh}{z}`.

   The precision of the result will be equal to the precision of *z*.

   :param z: the argument.

   :return: the hyperbolic function of *z*.

Logarithms and exponentials
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::exp(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::log(mppp::complex &rop, T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::log10(mppp::complex &rop, T &&z)

   Binary :cpp:class:`~mppp::complex` logarithms and exponentials.

   These functions will set *rop* to, respectively:

   * :math:`e^z`,
   * :math:`\log z`,
   * :math:`\log_{10} z`.

   The precision of the result will be equal to the precision of *z*.

   :param rop: the return value.
   :param z: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::exp(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::log(T &&z)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::log10(T &&z)

   Unary :cpp:class:`~mppp::complex` logarithms and exponentials.

   These functions will return, respectively:

   * :math:`e^z`,
   * :math:`\log z`,
   * :math:`\log_{10} z`.

   The precision of the result will be equal to the precision of *z*.

   :param z: the argument.

   :return: the result of the operation.

Arithmetic-geometric mean
~~~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex &mppp::agm1(mppp::complex &rop, T &&x)
.. cpp:function:: template <mppp::cvr_complex T, mppp::cvr_complex U> mppp::complex &mppp::agm(mppp::complex &rop, T &&x, U &&y)

   .. versionadded:: 0.21

   .. note::

      These functions are available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

      The ``agm()`` function is available only from
      Arb 2.18 onwards.

   :cpp:class:`~mppp::complex` AGM functions.

   These functions will set *rop* to, respectively:

   * :math:`\operatorname{agm}\left( 1, x \right)`,
   * :math:`\operatorname{agm}\left( x, y \right)`.

   The precision of *rop* will be set to the largest precision among the operands.

   :param rop: the return value.
   :param x: the first operand.
   :param y: the second operand.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPC types
     fails because of (unlikely) overflow conditions.

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::agm1(T &&x)
.. cpp:function:: template <typename T, mppp::complex_op_types<T> U> mppp::complex mppp::agm(T &&x, U &&y)

   .. versionadded:: 0.21

   .. note::

      These functions are available only if mp++ was
      configured with the ``MPPP_WITH_ARB`` option enabled.

      The ``agm()`` function is available only from
      Arb 2.18 onwards.

   :cpp:class:`~mppp::complex` AGM functions.

   These functions will return, respectively:

   * :math:`\operatorname{agm}\left( 1, x \right)`,
   * :math:`\operatorname{agm}\left( x, y \right)`.

   The precision of the result will be equal to the highest precision
   among the operands.

   :param x: the first operand.
   :param y: the second operand.

   :return: the result of the operation.

   :exception std\:\:invalid_argument: if the conversion between Arb and MPC types
     fails because of (unlikely) overflow conditions.

Input/Output
~~~~~~~~~~~~

.. cpp:function:: std::ostream &mppp::operator<<(std::ostream &os, const mppp::complex &c)

   Output stream operator.

   This function will direct to the output stream *os* the input :cpp:class:`~mppp::complex` *c*.

   :param os: the target stream.
   :param c: the input argument.

   :return: a reference to *os*.

   :exception std\:\:overflow_error: in case of (unlikely) overflow errors.
   :exception std\:\:invalid_argument: if the MPFR printing primitive ``mpfr_asprintf()`` returns an error code.
   :exception unspecified: any exception raised by the public interface of ``std::ostream`` or by memory allocation errors.

.. _complex_operators:

Mathematical operators
----------------------

.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::operator+(T &&c)
.. cpp:function:: template <mppp::cvr_complex T> mppp::complex mppp::operator-(T &&c)

   Identity and negation operators.

   :param c: the input argument.

   :return: :math:`c` and :math:`-c` respectively.

.. cpp:function:: template <typename T, mppp::complex_op_types<T> U> mppp::complex mppp::operator+(T &&a, U &&b)
.. cpp:function:: template <typename T, mppp::complex_op_types<T> U> mppp::complex mppp::operator-(T &&a, U &&b)
.. cpp:function:: template <typename T, mppp::complex_op_types<T> U> mppp::complex mppp::operator*(T &&a, U &&b)
.. cpp:function:: template <typename T, mppp::complex_op_types<T> U> mppp::complex mppp::operator/(T &&a, U &&b)

   Binary arithmetic operators.

   The precision of the result will be set to the largest precision among the operands.

   :param a: the first operand.
   :param b: the second operand.

   :return: the result of the binary operation.

.. cpp:function:: template <typename U, mppp::complex_in_place_op_types<U> T> T &mppp::operator+=(T &a, U &&b)
.. cpp:function:: template <typename U, mppp::complex_in_place_op_types<U> T> T &mppp::operator-=(T &a, U &&b)
.. cpp:function:: template <typename U, mppp::complex_in_place_op_types<U> T> T &mppp::operator*=(T &a, U &&b)
.. cpp:function:: template <typename U, mppp::complex_in_place_op_types<U> T> T &mppp::operator/=(T &a, U &&b)

   In-place arithmetic operators.

   If *a* is a :cpp:class:`~mppp::complex`, then these operators are equivalent, respectively,
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

   :exception unspecified: any exception thrown by the generic conversion operator of :cpp:class:`~mppp::complex`.

.. cpp:function:: mppp::complex &mppp::operator++(mppp::complex &c)
.. cpp:function:: mppp::complex &mppp::operator--(mppp::complex &c)

   Prefix increment/decrement.

   :param c: the input argument.

   :return: a reference to *c* after the increment/decrement.

.. cpp:function:: mppp::complex mppp::operator++(mppp::complex &c, int)
.. cpp:function:: mppp::complex mppp::operator--(mppp::complex &c, int)

   Suffix increment/decrement.

   :param c: the input argument.

   :return: a copy of *c* before the increment/decrement.

.. cpp:function:: template <typename T, mppp::complex_eq_op_types<T> U> bool mppp::operator==(const T &a, const U &b)
.. cpp:function:: template <typename T, mppp::complex_eq_op_types<T> U> bool mppp::operator!=(const T &a, const U &b)

   Comparison operators.

   These operators will compare *a* and *b*, returning ``true`` if, respectively, :math:`a=b` and :math:`a \neq b`,
   and ``false`` otherwise.

   The comparisons are always exact (i.e., no rounding is involved).

   These operators handle NaN in the same way specified by the IEEE floating-point
   standard.

   :param a: the first operand.
   :param b: the second operand.

   :return: the result of the comparison.

User-defined literals
---------------------

.. cpp:function:: template <char... Chars> mppp::complex mppp::literals::operator"" _icr128()
.. cpp:function:: template <char... Chars> mppp::complex mppp::literals::operator"" _icr256()
.. cpp:function:: template <char... Chars> mppp::complex mppp::literals::operator"" _icr512()
.. cpp:function:: template <char... Chars> mppp::complex mppp::literals::operator"" _icr1024()

   User-defined arbitrary-precision imaginary literals.

   These numeric literal operator templates can be used to construct
   purely imaginary :cpp:class:`~mppp::complex` instances with, respectively, 128, 256, 512
   and 1024 bits of precision. The real part of the return value will be set to zero,
   while the imaginary part will be set to the input floating-point literal. Literals in decimal and
   hexadecimal format are supported.

   .. seealso::

      https://en.cppreference.com/w/cpp/language/floating_literal

   :exception std\:\:invalid_argument: if the input sequence of characters is not
     a valid floating-point literal (as defined by the C++ standard).
