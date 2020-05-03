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
      generic constructors of :cpp:class:`~mppp::real`.

      :param x: the construction argument.
      :param p: the desired precision.

      :exception unspecified: any exception raised by the invoked :cpp:class:`~mppp::real`
        constructor.

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
   :cpp:class:`~mppp::complex` in order to avoid ambiguities when the input precision may
   be interpreted as an imaginary part instead.

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
