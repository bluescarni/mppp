.. _tutorial_integer:

Integer tutorial
================

:cpp:class:`~mppp::integer` was the first class implemented in mp++, and as a result it is currently
the most featureful and optimised among mp++'s multiprecision classes.

One of the first things that can be noticed about :cpp:class:`~mppp::integer` is that the class is parametrised over
an integral value ``SSize``, called the *static size*. This value represents the number of *limbs* that will be
stored directly within an :cpp:class:`~mppp::integer` object before resorting to dynamic memory allocation.
A limb, in turn, is the part of a multiprecision integer that fits in a single machine word. In the same way
an integral value in base 10 is represented as a sequence of digits in the :math:`\left[0,9\right]` range,
an :cpp:class:`~mppp::integer` object is represented by a sequence of limbs, each of which, on current platforms, has a range
of either :math:`\left[0,2^{32}-1\right]` (for 32-bit architectures) or :math:`\left[0,2^{64}-1\right]`
(for 64-bit architectures). Thus, for instance, if ``SSize`` is set to 2 on a 64-bit system,
:cpp:class:`~mppp::integer` is able to represent values in the :math:`\left(-2^{128},2^{128}\right)` range
without resorting to dynamic memory allocation. In general, a value of 1 is a good default choice for
the ``SSize`` parameter.

In addition to the :ref:`common operators <tutorial_commonops>` available for all of mp++'s multiprecision classes,
:cpp:class:`~mppp::integer` supports the following additional operators:

* the modulo operator ``%``, which computes the remainder of integer division,
* the bitshifting operators ``<<`` and ``>>``,
* the bitwise logical operators ``~``, ``&``, ``|`` and ``^``.

In addition to the binary versions of these operators, the in-place versions are also available. Lower level ternary
primitives are also provided for those situations in which it is desirable to pass the return value as a function
parameter, rather than creating a new return value (see the :ref:`API overview <tutorial_api>`).
For consistency with C++11, the ``%`` operator returns a remainder with the same sign as the dividend. The bit-shifting
operators ``<<`` and ``>>`` correspond respectively to multiplication and division by a power of 2. The bitwise logical
operators behave as-if :cpp:class:`~mppp::integer` used a two's complement representation (even if internally
a sign-magnitude representation is used instead).

Among the features specific to :cpp:class:`~mppp::integer` we find:

* specialised arithmetic primitives, including fused multiply-add/sub and addition/subtraction with C++ unsigned
  integrals,
* functionality related to prime numbers (:cpp:func:`~mppp::probab_prime_p()`, :cpp:func:`~mppp::nextprime()`, ...),
* parity detection (:cpp:func:`~mppp::even_p()`, :cpp:func:`~mppp::odd_p()`),
* various :ref:`division functions <integer_division>`,
* :ref:`number-theoretic functions <integer_ntheory>` (GCD, factorial, binomial coefficient, ...),
* integer :ref:`roots <integer_roots>` and :ref:`exponentiation <integer_exponentiation>`,
* hashing utilities (including a specialisation of ``std::hash``, so that it is possible to use
  :cpp:class:`~mppp::integer` in standard unordered containers out of the box).

Many of these features available in multiple overloads, and both as free and member functions.
