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
   of the return value (which is always passed as the first function parameter in the MPC API).

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

   .. cpp:function:: explicit complex(const complex &other, mpfr_prec_t p)
   .. cpp:function:: explicit complex(complex &&other, mpfr_prec_t p)

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

   .. cpp:function:: template <complex_interoperable T> explicit complex(T &&x, complex_prec_t p)
   .. cpp:function:: template <complex_interoperable T> explicit complex(T &&x)

      Generic constructors.

      The generic constructors will set ``this`` to the value of *x*.

      The variant with the *p* argument will set the precision of ``this``
      exactly to *p*.

      The variant without the *p* argument will set the
      precision of ``this`` according to the heuristics explained in the
      generic constructors of :cpp:class:`~mppp::real` (or exactly to
      the output of :cpp:func:`mppp::real::get_prec()` if *x* is a
      :cpp:class:`~mppp::real`).

      :param x: the construction argument.
      :param p: the desired precision.

      :exception unspecified: any exception raised by the invoked :cpp:class:`~mppp::real`
        constructor.

   .. cpp:function:: template <rv_complex_interoperable T, rv_complex_interoperable U> explicit complex(T &&x, U &&y)
   .. cpp:function:: template <rv_complex_interoperable T, rv_complex_interoperable U> explicit complex(T &&x, U &&y, complex_prec_t p)
   .. cpp:function:: template <string_type T, rv_complex_interoperable U> explicit complex(const T &x, U &&y, complex_prec_t p)
   .. cpp:function:: template <rv_complex_interoperable T, string_type U> explicit complex(T &&x, const U &y, complex_prec_t p)
   .. cpp:function:: template <string_type T, string_type U> explicit complex(const T &x, const U &y, complex_prec_t p)

      Generic constructors from real and imaginary parts, in numerical or string format.

      These constructors will set ``this`` to :math:`x+\imath y`.

      The variants with a precision argument
      will set the precision of ``this`` exactly to *p*.

      Otherwise, the precision of ``this`` will be the maximum among the deduced precisions
      of *x* and *y*. The precision deduction rules are the same explained in the generic
      constructors of :cpp:class:`~mppp::real`. If *x* and/or *y* are :cpp:class:`~mppp::real`,
      the deduced precision is the output of :cpp:func:`mppp::real::get_prec()`.

      The string variants expect a representation in base 10.

      :param x: the real part of the value that will be used for the initialisation.
      :param y: the imaginary part of the value that will be used for the initialisation.
      :param p: the desired precision.

      :exception unspecified: any exception raised by the invoked :cpp:class:`~mppp::real`
        constructor.

   .. cpp:function:: template <string_type T> explicit complex(const T &s, int base, mpfr_prec_t p)
   .. cpp:function:: template <string_type T> explicit complex(const T &s, mpfr_prec_t p)

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

   .. cpp:function:: explicit complex(const char *begin, const char *end, int base, mpfr_prec_t p)
   .. cpp:function:: explicit complex(const char *begin, const char *end, mpfr_prec_t p)

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

      :param c: the :cpp:type:`mpc_t` that will be moved.

   .. cpp:function:: ~complex()

      Destructor.

      The destructor will run sanity checks in debug mode.

   .. cpp:function:: complex &operator=(const complex &other)
   .. cpp:function:: complex &operator=(complex &&other) noexcept

      Copy and move assignment operators.

      :param other: the assignment argument.

      :return: a reference to ``this``.

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

   A strongly-typed counterpart to :cpp:type:`mpfr_prec_t`, used in certain constructors of
   :cpp:class:`~mppp::complex` in order to avoid ambiguities during overload resolution.

Concepts
--------

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
