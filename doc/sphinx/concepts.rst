Common concepts
===============

.. note::

   Generic functions and classes in mp++ support `concepts <https://en.wikipedia.org/wiki/Concepts_(C%2B%2B)>`__
   to constrain the types with which they can be used. C++ concepts are not (yet) part of the standard, and they are
   currently available only in GCC 6 and later (with the ``-fconcepts`` compilation flag). When used with compilers which do not
   support concepts natively, mp++ will employ a concept emulation layer in order to provide the same functionality as native
   C++ concepts.

   Since the syntax of native C++ concepts is clearer than that of the concept emulation layer, the mp++ documentation describes
   and refers to concepts in their native C++ form. As long as concepts are not part of the C++ standard, mp++'s concepts
   will be subject to breaking changes, and they should not be regarded as part of the public mp++ API.

*#include <mp++/concepts.hpp>*

.. cpp:concept:: template <typename T> mppp::CppInteroperable

   This concept is satisfied by any C++ fundamental type with which the multiprecision classes (such as :cpp:class:`~mppp::integer`,
   :cpp:class:`~mppp::rational`, etc.) can interoperate. The full list of types satisfying this concept includes:

   * all the fundamental C++ integral types,
   * ``float`` and ``double``.

   ``long double`` is also included, but only if mp++ was configured with the ``MPPP_WITH_MPFR`` option enabled
   (see the :ref:`installation instructions <installation>`).

   The GCC-style extended 128-bit integral types ``__int128_t`` and ``__uint128_t`` are included as well, if supported
   on the current platform/compiler combination (see also the :c:macro:`MPPP_HAVE_GCC_INT128` definition).

   A corresponding boolean type trait called ``is_cpp_interoperable`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T> mppp::CppIntegralInteroperable

   This concept is satisfied if ``T`` is an integral :cpp:concept:`~mppp::CppInteroperable` type.

   A corresponding boolean type trait called ``is_cpp_integral_interoperable`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T> mppp::CppUnsignedIntegralInteroperable

   This concept is satisfied if ``T`` is an unsigned integral :cpp:concept:`~mppp::CppInteroperable` type.

   A corresponding boolean type trait called ``is_cpp_unsigned_integral_interoperable`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T> mppp::CppFloatingPointInteroperable

   This concept is satisfied if ``T`` is a floating-point :cpp:concept:`~mppp::CppInteroperable` type.

   A corresponding boolean type trait called ``is_cpp_floating_point_interoperable`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T> mppp::StringType

   This concept is satisfied by C++ string-like types. Specifically, the concept will be true if ``T``,
   after the removal of cv qualifiers, is one of the following types:

   * ``std::string``,
   * a pointer to (possibly cv qualified) ``char``,
   * a ``char`` array of any size.

   Additionally, if at least C++17 is being used, the concept is satisfied also by ``std::string_view``.
