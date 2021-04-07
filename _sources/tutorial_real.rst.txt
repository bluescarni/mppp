.. _tutorial_real:

Multiprecision float tutorial
=============================

:cpp:class:`~mppp::real` is a floating-point
class in which the number of digits of precision in the significand can
be set at runtime to any value (limited only by the available
memory).

:cpp:class:`~mppp::real` wraps the
:cpp:type:`mpfr_t` type from the MPFR library,
and it is designed to behave like a standard C++ floating-point type
in which the precision is not a compile-time property of the class,
but rather a runtime property of the class instances.

Construction and precision handling
-----------------------------------

The precision of a :cpp:class:`~mppp::real` is measured in the number
of bits in the significand, and it can be set at
construction or changed at a later stage. A default-constructed
:cpp:class:`~mppp::real` will have the smallest possible precision
(see :cpp:func:`~mppp::real_prec_min()`), and a value of zero.
When a :cpp:class:`~mppp::real` is constructed from another
numeric type, its precision is automatically deduced so that
the value of the source object is preserved, if possible.
The copy constructor and the copy assignment operator of
:cpp:class:`~mppp::real` copy both the value and the precision,
so that the result of the operation is an exact copy of the source
operand.

Let's see a few examples:

.. code-block:: c++

   real x0;                // x0 is inited to zero with minimum precision.

   real x1{42};            // x1 is inited to 42, the precision is set to
                           // the bit width of the 'int' type
                           // (typically 32).

   real x2{42, 128};       // x2 is inited to 42, the precision is set
                           // explicitly to 128 bits.

   real x3{1 / 3_q1};      // x3 is inited from the rational value
                           // '1/3'. The precision is set to the sum
                           // of the bit widths of numerator and
                           // denominator (which will total 128 bits
                           // on most platforms). Because '1/3'
                           // cannot be represented exactly in a binary
                           // floating-point format, x3 will contain
                           // the 128-bit approximation of '1/3'.

   real x4{1 / 3_q1, 256}; // x4 is inited to the 256-bit
                           // approximation of the rational
                           // value '1/3'.

   real x5{x4};            // x5 is an exact copy (i.e., both
                           // value and precision) of x4.

   x5 = x2;                // x5 is now an exact copy of x2.

   x5 = 1.25;              // x5 now contains the value '1.25',
                           // and its precision is set to the number
                           // of binary digits in the significand of
                           // the C++ 'double' type (typically 53).

   // Fetch and print the precision of x2 (i.e., 128).
   std::cout << x2.get_prec() << '\n';

   // Reduce the precision of x2 to 4 bits, while
   // rounding to nearest the original value. I.e.,
   // x2 will end up containing the 4-bit approximation
   // of the value '42'.
   x2.prec_round(4);
   assert(x2.get_prec() == 4);
   assert(x2 == real{42, 4});

   // Destructively extend the precision of x2
   // to 1024 bits. The value will be set to NaN.
   x2.set_prec(1024);
   assert(x2.nan_p());

The constructors from string currently always require the
precision to be passed explicitly (this restriction may
be lifted in the future):

.. code-block:: c++

   real x0{"1.1", 512};          // x0 is set to the 512-bit approximation
                                 // of '1.1'.

   real x1{"0x1.5p-1", 16, 512}; // Construction from other bases is also
                                 // possible (here base 16 is used).

In additions to the :ref:`constructors <tutorial_constr>` common to
all of mp++'s classes, :cpp:class:`~mppp::real` features additional
specialised constructors:

.. code-block:: c++

   real x0{real_kind::inf, -1, 64}; // x0 is set to -infinity with 64
                                    // bits of precision.

   real x1{-4, 8, 112};             // x1 is set to -4*2**8 with 112
                                    // bits of precision.

Sometimes it is useful to be able to set a :cpp:class:`~mppp::real`
to a specific value *without* changing its precision. For this
purpose, :cpp:class:`~mppp::real` provides the :cpp:func:`~mppp::real::set()`
family of functions:

.. code-block:: c++

   real x0{real_kind::zero, 112}; // Create a positive zero with 112 bits of precision.

   real x1{"1.1", 256};           // x1 is the 256-bit approximation of '1.1'.
   x0.set(x1);                    // x0 will be set to x1 rounded to nearest
                                  // to 112 bits.

   x0.set(2 / 3_q1);              // x0 will be set to the 112-bit approximation
                                  // of the fraction '2/3'.

   x0.set("2.1");                 // x0 will be set to the 112-bit approximation
                                  // of the value '2.1'.

Specialised setter functions are also available:

.. code-block:: c++

   real x0{real_kind::zero, 112}; // Create a positive zero with 112 bits of precision.

   x0.set_inf();                  // Set x0 to +infinity, precision is not altered.
   x0.set_inf(-1);                // Set x0 to -infinity, precision is not altered.

   set_ui_2exp(x0, 4, -5);        // Set x0 to the 112-bit approximation of
                                  // 4*2**(-5).

Precision propagation
---------------------

In the C++ language, mixed-precision floating-point operations promote the lower-precision operand
to the higher-precision type. For instance, in the following code snippet,

.. code-block:: c++

   float x1 = 1;
   double x2 = 2;

   auto ret = x1 + x2;

the addition is computed in ``double`` precision, and the result ``ret``
will be of type ``double``.

:cpp:class:`~mppp::real` adopts a similar principle: in functions (including
overloaded operators) accepting two or more :cpp:class:`~mppp::real` arguments
in input, the precision at which the operation is performed (and the precision of the result)
is the maximum precision among the operands.

Let's see a couple of examples:

.. code-block:: c++

   real x1{"1.1", 128};          // 128-bit approximation of '1.1'.
   real x2{42, 10};              // x2 contains the value 42, represented exactly
                                 // by a 10-bit significand.

   auto x3 = x1 + x2;            // x3 is the result of the addition of
                                 // x1 and x2, computed at 128 bits of precision.
   assert(x3.get_prec() == 128);

   real x4{"2.3", 256};          // 256-bit approximation of '2.3'.

   auto x5 = pow(x3, x4);        // x5 is the result of x3 raised to the
                                 // power of x4, computed at 256 bits of precision.
   assert(x5.get_prec() == 256);

The same idea extends to operations mixing :cpp:class:`~mppp::real` and
non-:cpp:class:`~mppp::real` types, where the "precision" of
non-:cpp:class:`~mppp::real` types
is determined following a set of heuristics detailed in the documentation
of the generic constructor of :cpp:class:`~mppp::real`.

Let's see a few concrete examples:

.. code-block:: c++

   real x1{1, 4};                   // 4-bit representation of the value '1'.

   auto x2 = x1 + 41;               // The deduced precision of the literal '41' is the
                                    // bit width of the 'int' type (typically 32). Hence,
                                    // the addition will be performed with 32 bits of
                                    // precision (because 32 > 4).
   assert(x2.get_prec() == 32);

   auto x3 = pow(x2, 0.5);          // The deduced precision of the literal '0.5' is the
                                    // bit width of the significand of the 'double' type
                                    // (typically 53). Hence, the exponentiation will be
                                    // performed with 53 bits of precision (because 53 > 32).
   assert(x3.get_prec() == 53);

   auto x4 = atan2(real128{2}, x3); // The deduced precision of a real128
                                    // is 113 (i.e., the number of bits in the
                                    // significand). Hence, the inverse tangent
                                    // will be computed with 113 bits of
                                    // precision (because 113 > 53).
   assert(x4.get_prec() == 113);

Move awareness
--------------

One of the major sources of inefficiency when working with :cpp:class:`~mppp::real` objects,
especially at lower precisions, is the memory allocation overhead. The problem is
particularly evident when creating complex mathematical expressions involving
:cpp:class:`~mppp::real` objects.

Consider the following example:

.. code-block:: c++

   real horner_6(const real &x)
   {
      real a[7] = {real{1.}, real{2.}, real{3.}, real{4.}, real{5.}, real{6.}, real{7.}};

      return (((((a[6] * x + a[5]) * x + a[4]) * x + a[3]) * x + a[2]) * x + a[1]) * x + a[0];
   }

This is the evaluation of the polynomial of degree 6

.. math::

   1 + 2x + 3x^3 + 4x^3 + 5x^4 + 6x^5 + 7x^6

for some :cpp:class:`~mppp::real` :math:`x` via
`Horner's method <https://en.wikipedia.org/wiki/Horner%27s_method>`__.
The polynomial coefficients are stored in the array ``a``.
Let's focus on the expression in the return statement:

.. code-block:: c++

   (((((a[6] * x + a[5]) * x + a[4]) * x + a[3]) * x + a[2]) * x + a[1]) * x + a[0]

Due to the way operators are parsed in the C++ language,
this expression is decomposed in multiple subexpressions,
which are then formally chained together in the following fashion:

.. code-block:: c++

   auto tmp0 = a[6] * x;
   auto tmp1 = tmp0 + a[5];
   auto tmp2 = tmp1 * x;
   auto tmp3 = tmp2 + a[4];
   // ... and so on.

In other words, the evaluation of the expression above results
in the creation of 12 temporary :cpp:class:`~mppp::real` objects. It's easy
to see that the creation of these temporary variables is not really
necessary if one, instead of using overloaded binary operators,
employs either the ternary mathematical primitives provided by
:cpp:class:`~mppp::real`, or, equivalently, in-place operators:

.. code-block:: c++

   // Create the return value.
   real ret{a[6]};

   // Accumulate the result into
   // ret step-by-step.
   mul(ret, ret, x);    // or: ret *= x;
   add(ret, ret, a[5]); // or: ret += a[5];
   mul(ret, ret, x);    // or: ret *= x;
   add(ret, ret, a[4]); // or: ret += a[4];
   // ... and so on.

While this approach is valid and efficient, it is quite verbose,
and, arguably, code clarity suffers.

Luckily, it turns out that such a complication is not really necessary,
because all the operators and functions in the :cpp:class:`~mppp::real`
API are *move-aware*. This means that the real API is able to detect
when a :cpp:class:`~mppp::real` argument is a temporary (technically,
an *rvalue*) and it is able to re-use the memory provided by such
a temporary to construct the result of the operation.

For instance, consider the subexpression:

.. code-block:: c++

   a[6] * x + a[5]

Here the multiplication ``a[6] * x`` produces a new :cpp:class:`~mppp::real`, which
is then added to ``a[5]``. The binary addition operator of :cpp:class:`~mppp::real`
recognises that the first argument is an rvalue, and accordingly it will use the
memory provided by the result of ``a[6] * x`` to create the result of the full
subexpression, thus avoiding an unnecessary memory allocation.

The end result is that in the expression

.. code-block:: c++

   (((((a[6] * x + a[5]) * x + a[4]) * x + a[3]) * x + a[2]) * x + a[1]) * x + a[0]

only a **single** memory allocation is performed, instead of 12.

Moreover, it turns out that in this specific case it is even possible to elide that
last memory allocation. Observe that in the expression above, the only memory
allocation originates from the first subexpression encountered by the compiler, that is:

.. code-block:: c++

   a[6] * x

In this subexpression there are no rvalues whose memory can be pilfered. However, we can notice how the variable
``a[6]`` from the coefficient array is never used again in the rest of the ``horner_6()``
function. We can then "cast" ``a[6]`` to an rvalue via the ``std::move()`` utility function:

.. code-block:: c++

   std::move(a[6]) * x

Now the memory necessary to represent the result of the multiplication will be "stolen" from
``a[6]``. We have thus avoided the last remaining memory allocation.

.. warning::

   Extreme care must be taken when using manually ``std::move()`` on a :cpp:class:`~mppp::real`
   object. Only a very narrow set of operations is valid on a moved-from :cpp:class:`~mppp::real`
   (see the documentation of the move constructor), and any other operation will result
   in undefined behaviour.

   Generally speaking, use-after-move is considered a pattern to
   avoid in modern C++. Static analysis tools such as `clang-tidy <https://clang.llvm.org/extra/clang-tidy/>`__
   are very effective at detecting use-after-move occurrences in source code.

It is important to emphasise how move-awareness in the :cpp:class:`~mppp::real` API is not
limited to mathematical operators: all functions accepting one or more :cpp:class:`~mppp::real`
objects as input arguments are move-aware.

Special functions
-----------------

One of the design goals of the :cpp:class:`~mppp::real` class is to provide a
comprehensive library of special functions. :cpp:class:`~mppp::real` currently
wraps all the special functions provided by the MPFR library, including:

* roots and exponentiation,
* (inverse) trigonometric functions,
* (inverse) hyperbolic functions,
* logarithms and exponentials,
* Gamma functions,
* Bessel functions,
* error functions,

and many more. Additionally, :cpp:class:`~mppp::real` can also (optionally) use
the `Arb <https://arblib.org/>`__ library to provide additional special functions
not available in MPFR.

Constants
---------

The MPFR API provides the ability to compute a handful of
constants in arbitrary precision. This capability is
:ref:`exposed <real_constants>`
in the :cpp:class:`~mppp::real` API. Here's an example
with the :math:`\pi` constant:

.. code-block:: c++

   real r0 = real_pi(42);        // r0 is the 42-bit approximation of pi,
                                 // that is, 3.14159265359012.

   real r1{real_kind::zero, 10}; // Init a real zero with 10 bits of precision.

   real_pi(r1);                  // r1 is now set to the 10-bit approximation of
                                 // pi, that is, 3.1406.

Comparisons
-----------

In addition to the :ref:`common comparison operators <tutorial_commonops>`
available for all of mp++'s multiprecision classes,
:cpp:class:`~mppp::real` supports additional comparison functions.

For instance, it is possible to detect special :cpp:class:`~mppp::real` values:

.. code-block:: c++

   // Check that default construction
   // initialises to zero.
   real r0;
   assert(r0.zero_p());

   // Detection of non-finite values.
   real r1{"inf", 42};
   assert(r1.inf_p());
   real r2{"nan", 42};
   assert(r2.nan_p());

   // Check if a real is an exact
   // integral value.
   real r3{3};
   assert(r3.integer_p());

:cpp:class:`~mppp::real` values can be compared by absolute value:

.. code-block:: c++

   assert(cmpabs(real{1}, real{-1}) == 0); // |1| == |-1|.
   assert(cmpabs(real{-2}, real{1}) > 0);  // |-2| > |1|.
   assert(cmpabs(real{0}, real{-1}) < 0);  // |0| < |-1|.

And they can also be compared efficiently to integral multiples
of powers of 2:

.. code-block:: c++

   assert(cmp_ui_2exp(real{50}, 3, 4) > 0); // 50 > 3*2**4.

The :cpp:class:`~mppp::real` API also provides comparison
functions that handle NaN values in a special way. Specifically,
these functions consider all NaN values equal to each other
and greater than non-NaN values:

.. code-block:: c++

   assert(real_equal_to(real{"nan", 10}, real{"nan", 20})); // NaN == NaN.
   assert(real_gt(real{"nan", 10}, real{100}));             // NaN > 100.
   assert(real_lt(real{"inf", 20}, real{"nan", 10}));       // inf < NaN.

Interacting with the MPFR API
-----------------------------

Because :cpp:class:`~mppp::real` wraps an :cpp:type:`mpfr_t`
instance, it is trivial to use a :cpp:class:`~mppp::real` in the MPFR API.
Two member functions are provided for direct access to the internal
:cpp:type:`mpfr_t` instance:

* :cpp:func:`mppp::real::get_mpfr_t()`, which returns a ``const`` :cpp:type:`mpfr_t`,
* :cpp:func:`mppp::real::_get_mpfr_t()`, which returns a mutable :cpp:type:`mpfr_t`.

When using :cpp:func:`mppp::real::_get_mpfr_t()`, it is the user's responsibility to ensure
that the internal :cpp:type:`mpfr_t` is kept in a state which respects the invariants
of the :cpp:class:`~mppp::real` class. Specifically, the precision value
must be in the bounds established by :cpp:func:`mppp::real_prec_min()` and
:cpp:func:`mppp::real_prec_max()`, and, upon destruction, a :cpp:class:`~mppp::real`
object must contain a valid :cpp:type:`mpfr_t` object.

Additionally, a variety of constructors, assignment operators and setters from
:cpp:type:`mpfr_t` are also available.

Serialisation
-------------

.. versionadded:: 0.22

mp++ provides a simple :ref:`API for the (de)serialisation <real_s11n>` of :cpp:class:`~mppp::real` objects
into/from memory buffers and C++ streams. Possible uses of the serialisation API include persistence (e.g.,
saving/loading :cpp:class:`~mppp::real` values to/from a file), the transmission of :cpp:class:`~mppp::real` objects over
the network (e.g., in distributed computing applications), inter-process communication, etc. The API consists of two main
overloaded functions, :cpp:func:`mppp::real::binary_save()` and :cpp:func:`mppp::real::binary_load()` (plus their
free-function counterparts).

Because the API is identical, we refer to the tutorial section on
:ref:`integer serialisation <tutorial_integer_s11n>` for usage examples.

As already pointed out in the :ref:`integer serialisation section <tutorial_integer_s11n>`,
the following points must be emphasised:

* although mp++ does run some consistency checks during deserialisation, the API is not built to protect
  against maliciously-crafted data. Users are thus advised not to load data from untrusted sources;
* the current binary serialisation format is compiler, platform and architecture specific, it is not portable
  and it might be subject to changes in future versions of mp++. Users are thus advised not to use
  the binary serialisation format for long-term persistence or as a data exchange format: for such
  purposes, it is better to use the string representation of :cpp:class:`~mppp::real` objects.

User-defined literals
---------------------

User-defined literals are available for :cpp:class:`~mppp::real`.
The :ref:`literals <real_literals>`
are defined within
the inline namespace ``mppp::literals``, they support
decimal and hexadecimal representations, and they currently
allow to construct :cpp:class:`~mppp::real` instances
with 128, 256, 512 and 1024 bits of precision:

.. code-block:: c++

   using namespace mppp::literals;

   auto r1 = 123.456_r128;   // r1 contains the 128-bit
                             // approximation of 123.456 (that is,
                             // 123.45599999999999999999999999999999999988).

   auto r2 = 4.2e1_r256;     // Scientific notation can be used.

   auto r3 = 0x1.12p-1_r512; // Hexadecimal floats are supported too.

.. seealso::

   https://en.cppreference.com/w/cpp/language/floating_literal
