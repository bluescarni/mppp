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
   The class is a thin wrapper around the :cpp:type:`__complex128` type and the quadmath library, available in GCC
   and recent Clang versions on most modern platforms, on top of which it provides the following additions:

   * interoperability with other mp++ classes,
   * consistent behaviour with respect to the conventions followed elsewhere in mp++ (e.g., values are
     default-initialised to zero rather than to indefinite values, conversions must be explicit, etc.),
   * enhanced compile-time (``constexpr``) capabilities,
   * a generic C++ API.

   Most of the functionality is exposed via plain :ref:`functions <complex128_functions>`, with the
   general convention that the functions are named after the corresponding quadmath functions minus the trailing ``q``
   suffix. For instance, the quadmath code

   .. code-block:: c++

      __complex128 a{1, 2};
      auto b = ::csinq(a);

   that computes the sine of :math:`1+2\imath` in quadruple precision, storing the result in ``b``, becomes in mp++

   .. code-block:: c++

      complex128 a{1, 2};
      auto b = sin(a);

   where the ``sin()`` function is resolved via argument-dependent lookup.

   Various :ref:`overloaded operators <complex128_operators>` are provided.
   The common arithmetic operators (``+``, ``-``, ``*`` and ``/``) always return :cpp:class:`~mppp::complex128`
   as a result, promoting at most one operand to :cpp:class:`~mppp::complex128` before actually performing
   the computation. Similarly, the relational operators ``==`` and ``!=`` will promote at
   most one argument to :cpp:class:`~mppp::complex128` before performing the comparison. Alternative comparison functions
   treating NaNs specially are provided.

   .. cpp:member:: __complex128 m_value

      The internal value.

      This class member gives direct access to the :cpp:type:`__complex128` instance stored
      inside a :cpp:class:`~mppp::complex128`.

   .. cpp:function:: constexpr complex128()

      Default constructor.

      The default constructor will set ``this`` to zero.

   .. cpp:function:: complex128(const complex128 &) = default
   .. cpp:function:: complex128(complex128 &&) = default

      :cpp:class:`~mppp::complex128` is trivially copy and
      move constructible.

   .. cpp:function:: constexpr explicit complex128(__complex128 x)

      Constructor from a :cpp:type:`__complex128`.

      This constructor will initialise the internal :cpp:type:`__complex128`
      value to *x*.

      :param x: the :cpp:type:`__complex128` that will be assigned to the internal value.

   .. cpp:function:: template <complex128_interoperable T> constexpr explicit complex128(const T &x)

      Constructor from real-valued interoperable types.

      This constructor will initialise the internal value to *x*.
      Depending on the value and type of *x*, ``this`` may not be exactly equal
      to *x* after initialisation (e.g., if *x* is a very large
      :cpp:class:`~mppp::integer`).

      :param x: the value that will be used for the initialisation.

      :exception unspecified: any exception raised by casting ``T`` to :cpp:class:`~mppp::real128`.

Types
-----

.. cpp:type:: __complex128

   A quadruple-precision complex floating-point type available in recent versions of the GCC and Clang compilers.
   This is the type wrapped by the :cpp:class:`~mppp::complex128` class.

   .. seealso::

      https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html

Concepts
--------

.. cpp:concept:: template <typename T> mppp::complex128_interoperable

   This concept is satisfied by real-valued types that can interoperate
   with :cpp:class:`~mppp::complex128`. Specifically, this concept is
   satisfied if either:

   * ``T`` satisfies :cpp:concept:`~mppp::Real128Interoperable`, or
   * ``T`` is :cpp:class:`~mppp::real128`, or
   * ``T`` is :cpp:class:`~mppp::real`.
