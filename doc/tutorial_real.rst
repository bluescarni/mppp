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

   // Destructively extend the precision of x2
   // to 1024 bits. The value will be set to NaN.
   x2.set_prec(1024);
   assert(x2.nan_p());

The constructors from string currently always require the
precision to be passed explicitly (this restriction might
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

the result ``ret`` will be of type ``double``, and the addition is computed in
``double`` precision.
