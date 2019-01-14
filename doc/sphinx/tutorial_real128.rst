.. _tutorial_real128:

Quadruple-precision float tutorial
==================================

The :cpp:class:`~mppp::real128` class is a thin wrapper around the :cpp:type:`__float128` type
available in GCC and (more recently) clang. :cpp:type:`__float128` is an implementation the
`quadruple-precision IEEE 754 binary floating-point standard <https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format>`__,
which provides up to 36 decimal digits of precision.

:cpp:class:`~mppp::real128` is available in mp++ if the library was configured with the
``MPPP_WITH_QUADMATH`` option enabled (see the :ref:`installation instructions <installation>`).
Note that mp++ needs access to both the ``quadmath.h`` header file and the quadmath library
``libquadmath.so``, which may be installed in non-standard locations. While GCC is typically
able to resolve the correct paths automatically, clang might need assistance
in order to identify the location of these files.

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
