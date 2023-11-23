.. _tutorial_commonops:

Common operators
================

After having explained how mp++'s :ref:`type coercion <tutorial_numtower>` works, we can now move on to
describe how mp++'s classes can be used to perform mathematical computations.

All of mp++'s multiprecision classes support the four basic arithmetic operations (:math:`+`, :math:`-`,
:math:`\times` and :math:`\div`) via overloaded binary operators:

.. code-block:: c++

   auto r1 = int_t{4} + int_t{3};
   assert(r1 == 7);

   auto r2 = rat_t{4, 3} - rat_t{2, 5};
   assert(r2 == rat_t{14, 15});

   auto r3 = real128{5} * real128{2};
   assert(r3 == 10.);

   auto r4 = real{5} / real{2};
   assert(r4 == 2.5);

It is of course possible to mix operands of different types, and the type of the result will be determined
by the :ref:`type coercion rules <tutorial_numtower>` explained earlier:

.. code-block:: c++

   auto r1 = int_t{4} + 3;        // r1 will be of type int_t.
   assert(r1 == 7);

   auto r2 = -0.75 + rat_t{1, 2}; // r2 will be of type double.
   assert(r2 == -.25);

   auto r3 = real128{5} * 2;      // r3 will be of type real128.
   assert(r3 == 10.);

   auto r4 = real{5} / int_t{2};  // r4 will be of type real.
   assert(r4 == 2.5);

The behaviour of the division operator varies depending on the types involved. If only
integral types are involved, division truncates, and division by zero throws
a :cpp:class:`~mppp::zero_division_error` exception:

.. code-block:: c++

   auto r1 = int_t{5} / 2; // Integral division truncates.
   assert(r1 == 2);

   int_t{1} / 0;           // This will throw a zero_division_error exception.

If floating-point types are involved, division by zero is allowed and results in infinity:

.. code-block:: c++

   auto r1 = real128{1} / 0; // Floating-point division by zero generates an infinity.
   assert(isinf(r1))

Rational division is always exact, unless the divisor is zero:

.. code-block:: c++

   auto r1 = rat_t{3} / 4;
   assert(r1 == rat_t{3, 4});

   rat_t{2} / 0;              // This will throw a zero_division_error exception.

The in-place versions of the binary operators are available as well. Given a binary operator, its
corresponding in-place counterpart behaves as a binary operation followed by assignment:

.. code-block:: c++

   int_t r1{4};
   r1 += 5;                    // Equivalent to: r1 = r1 + 5
   assert(r1 == 9);

   rat_t r2{4, 3};
   r2 -= 1.5;                  // Equivalent to: r2 = r2 - 1.5
   assert(r2 == rat_t{-1, 6});

   real128 r3{5};
   r3 *= rat_t{1, 2};          // Equivalent to: r3 = r3 * rat_t{1, 2}
   assert(r3 == 2.5);

   real r4{42};
   r4 /= real128{0};           // Equivalent to: r4 = r4 / real128{0}
   assert(isinf(r4));

It is also possible to use fundamental C++ types on the left-hand side of in-place operators:

.. code-block:: c++

   int n = 5;
   n += int_t{5};
   assert(n == 10);

   n -= rat_t{3, 4}
   assert(n == 9);

   double x = 1.5;
   x *= real128{2};
   assert(x == 3.);

   x /= real{3};
   assert(x == 1.);

The identity, negation, pre/post increment/decrement operators are also supported for all of
mp++'s multiprecision classes:

.. code-block:: c++

   int_t n;
   assert(++n == 1);
   n++;
   assert(n == 2);
   assert(--n == 1);
   n--;
   assert(n == 0);

   rat_t q{1, 2};
   assert(+q == q);
   assert(-q == rat_t{-1, 2});

All of mp++'s multiprecision classes are contextually convertible to ``bool``, following the usual rule
that nonzero values convert to ``true`` and zero values convert to ``false``:

.. code-block:: c++

   int_t n{3};
   if (n) {
      std::cout << "n is nonzero!\n";
   }

   rat_t q{0};
   if (!q) {
      std::cout << "q is zero!\n";
   }

   real r{1.23};
   if (!!r) {
      std::cout << "r is nonzero!\n";
   }

In addition to the common arithmetic operators, all of mp++'s multiprecision classes support the relational
operators :math:`=` and :math:`\neq`. Real-valued types also support the comparison operators
:math:`>`, :math:`\geq`, :math:`<` and :math:`\leq`. Any combination
of multiprecision and numerical C++ types is supported:

.. code-block:: c++

   assert(int_t{42} == 42);
   assert(3 != rat_t{1, 3});
   assert(0.9 < int_t{1});
   assert(real128{15} <= int_t{15});
   assert(real{"inf", 100} > rat_t{123, 456});
   assert(int_t{4} >= rat_t{16, 4});

The comparison operators treat NaN values in the standard way: comparing NaN to any other value returns always
``false``, apart from the :math:`\neq` operator which always returns ``true`` when NaN is involved. For the floating-point multiprecision
classes, custom comparison functions with special NaN handling are also available (e.g., :cpp:func:`~mppp::real_lt()`,
:cpp:func:`~mppp::real128_equal_to()`, etc.). These functions can be used as replacements for the comparison operators
in facilities of the standard library such as ``std::sort()``, ``std::set``, etc.
