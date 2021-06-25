.. _tutorial_real128:

Quadruple-precision float tutorial
==================================

The :cpp:class:`~mppp::real128` class is a thin wrapper around
the :cpp:type:`__float128` type
available on GCC, Clang and the Intel compiler on some platforms [#f1]_.

:cpp:type:`__float128` is an implementation the
`quadruple-precision IEEE 754 binary floating-point standard <https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format>`__,
which provides up to 36 decimal digits of precision.
On most platforms, :cpp:type:`__float128` is implemented
in software, and thus it is typically an order of magnitude
slower than the standard floating-point C++ types. A notable
exception are recent versions of the PowerPC architecture,
which provide hardware-accelerated quadruple-precision
floating-point arithmetic.
Note that, even with software implementations, :cpp:class:`~mppp::real128`
can be expected to be noticeably faster than :cpp:class:`~mppp::real`.
:cpp:class:`~mppp::real128` is available in mp++ if
the library is configured with the
``MPPP_WITH_QUADMATH`` option enabled
(see the :ref:`installation instructions <installation>`).

.. note::

   On Clang<7, :cpp:type:`__float128` cannot be used in mixed-mode
   operations with ``long double``. Accordingly,
   :cpp:class:`~mppp::real128` will disable interoperability with
   ``long double`` if Clang<7 is being used.

As a thin wrapper, :cpp:class:`~mppp::real128` adds a few extra features
on top of what :cpp:type:`__float128` already provides. Specifically, :cpp:class:`~mppp::real128`:

* can interact with the other mp++ classes,
* can be constructed from string-like objects,
* supports the standard C++ ``iostream`` facilities.

Like :cpp:type:`__float128`, :cpp:class:`~mppp::real128` is a
`literal type <https://en.cppreference.com/w/cpp/named_req/LiteralType>`__, and thus it can be used
for ``constexpr`` compile-time computations. Additionally, :cpp:class:`~mppp::real128`
implements as ``constexpr`` constructs a variety of functions which are not ``constexpr``
for :cpp:type:`__float128`.

.. note::

   The Intel compiler does not implement certain :cpp:type:`__float128`
   floating-point primitives
   as constant expressions. As a result, a few :cpp:class:`~mppp::real128`
   functions which are ``constexpr`` on GCC and Clang are not ``constexpr``
   when using the Intel compiler. These occurrences are marked in the API
   reference.

In addition to the features common to all mp++ classes, the :cpp:class:`~mppp::real128` API provides
a few additional capabilities:

* construction/conversion from/to :cpp:type:`__float128`:

  .. code-block:: c++

     real128 r{__float128(42)};                // Construction from a __float128.
     assert(r == 42);
     assert(static_cast<__float128>(r) == 42); // Conversion to __float128.

* direct access to the internal :cpp:type:`__float128` instance (via the public :cpp:member:`~mppp::real128::m_value`
  data member):

  .. code-block:: c++

     real128 r{1};
     r.m_value += 1;                 // Modify directly the internal __float128 member.
     assert(r == 2);

     r.m_value = 0;
     assert(::cosq(r.m_value) == 1); // Call a libquadmath function directly on the internal member.

* a variety of mathematical :ref:`functions <real128_functions>` wrapping the
  `libquadmath library routines <https://gcc.gnu.org/onlinedocs/libquadmath/Math-Library-Routines.html#Math-Library-Routines>`__.
  Note that the :cpp:class:`~mppp::real128` function names drop the suffix ``q`` appearing in the names of the libquadmath routines, and, as usual
  in mp++, they are supposed to be found via ADL. Member function overloads for the unary functions are also available:

  .. code-block:: c++

     real128 r{42};

     // Trigonometry.
     assert(cos(r) == ::cosq(r.m_value));
     assert(sin(r) == ::sinq(r.m_value));

     // Logarithms and exponentials.
     assert(exp(r) == ::expq(r.m_value));
     assert(log10(r) == ::log10q(r.m_value));

     // Etc.
     assert(lgamma(r) == ::lgammaq(r.m_value));
     assert(erf(r) == ::erfq(r.m_value));

     // Member function overloads.
     auto tmp = cos(r);
     assert(r.cos() == tmp); // NOTE: r.cos() will set r to its cosine.
     tmp = sin(r);
     assert(r.sin() == tmp); // NOTE: r.sin() will set r to its sine.

* NaN-friendly hashing and comparison functions, for use in standard algorithms and containers;
* a :ref:`specialisation <real128_std_specs>` of the ``std::numeric_limits`` class template;
* a selection of quadruple-precision compile-time :ref:`mathematical constants <real128_constants>`.

The :ref:`real128 reference <real128_reference>` contains the detailed description of all the features
provided by :cpp:class:`~mppp::real128`.

.. rubric:: Footnotes

.. [#f1] Note that on some platforms (e.g., Linux ARM64) a quadruple-precision floating-point type
     is available as the standard C++ ``long double`` type. On such platforms, :cpp:type:`__float128`
     is not available and neither is :cpp:class:`~mppp::real128`.

User-defined literal
--------------------

.. versionadded:: 0.19

A user-defined literal is available to construct
:cpp:class:`mppp::real128` instances.
The :ref:`literal <real128_literals>`
is defined within
the inline namespace ``mppp::literals``, and it supports
decimal and hexadecimal representations:

.. code-block:: c++

   using namespace mppp::literals;

   auto r1 = 123.456_rq;   // r1 contains the quadruple-precision
                           // approximation of 123.456 (that is,
                           // 123.455999999999999999999999999999998).

   auto r2 = 4.2e1_rq;     // Scientific notation can be used.

   auto r3 = 0x1.12p-1_rq; // Hexadecimal floats are supported too.

.. seealso::

   https://en.cppreference.com/w/cpp/language/floating_literal
