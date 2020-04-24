Common concepts
===============

.. note::

   Generic functions and classes in mp++ support `concepts <https://en.wikipedia.org/wiki/Concepts_(C%2B%2B)>`__
   to constrain the types with which they can be used. C++ concepts are part of the C++20 standard, and they are
   currently available only on GCC>=6, Clang>=10 and MSVC>=16.3. When used with compilers which do not
   support concepts natively, mp++ will employ a concept emulation layer in order to provide the same functionality as native
   C++ concepts.

   Since the syntax of native C++ concepts is clearer than that of the concept emulation layer, the mp++ documentation describes
   and refers to concepts in their native C++ form.

*#include <mp++/concepts.hpp>*

.. cpp:concept:: template <typename T> mppp::cpp_integral

   This concept is satisfied if ``T`` is an integral C++ type.

   The GCC-style extended 128-bit integral types ``__int128_t`` and ``__uint128_t`` are included as well, if supported
   on the current platform/compiler combination (see also the :c:macro:`MPPP_HAVE_GCC_INT128` definition).

.. cpp:concept:: template <typename T> mppp::cpp_unsigned_integral

   This concept is satisfied if ``T`` is an unsigned :cpp:concept:`~mppp::cpp_integral`.

.. cpp:concept:: template <typename T> mppp::cpp_signed_integral

   This concept is satisfied if ``T`` is a signed :cpp:concept:`~mppp::cpp_integral`.

.. cpp:concept:: template <typename T> mppp::cpp_floating_point

   This concept is satisfied if ``T`` is a floating-point C++ type.

.. cpp:concept:: template <typename T> mppp::cpp_arithmetic

   This concept is satisfied if ``T`` is either a :cpp:concept:`~mppp::cpp_integral` or
   a :cpp:concept:`~mppp::cpp_floating_point` type.

.. cpp:concept:: template <typename T> mppp::cpp_complex

   This concept is satisfied if ``T`` is one of the standard complex C++ types:

   * ``std::complex<float>``,
   * ``std::complex<double>``,
   * ``std::complex<long double>``.

.. cpp:concept:: template <typename T> mppp::string_type

   This concept is satisfied by C++ string-like types. Specifically, the concept will be true if ``T``,
   after the removal of cv qualifiers, is one of the following types:

   * ``std::string``,
   * a pointer to (possibly cv qualified) ``char``,
   * a ``char`` array of any size.

   Additionally, if at least C++17 is being used, the concept is satisfied also by ``std::string_view``
   (see also the :c:macro:`MPPP_HAVE_STRING_VIEW` definition).
