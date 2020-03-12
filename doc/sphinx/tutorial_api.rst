.. _tutorial_api:

API overview
============

In addition to the :ref:`common arithmetical and relational operators <tutorial_commonops>` introduced
earlier, mp++'s API provides a rich set of functions.
Generally speaking, the mp++ API operates on two levels:

* a low-level API closely following the APIs of the multiprecision libraries
  on top of which mp++ is built,
* a higher-level API more focused on consistency and user-friendliness.

The intent is to provide users with the flexibility of switching between low-level primitives
(e.g., when performance is critical) and higher-level, more readable and user-friendly code.

To give an idea of the type of tradeoffs involved in the choice between low-level and high-level
API, consider the simple task of adding two multiprecision integers ``a`` and ``b``. This can be accomplished
either via the overloaded binary operator for :cpp:class:`~mppp::integer` (as seen in the previous section), or via the
lower-level ``add()`` primitive (which is documented in the :ref:`integer reference <integer_arithmetic>`):

.. code-block:: c++

   int_t a{2}, b{4};

   auto c1 = a + b; // Binary operator.

   int_t c2;
   add(c2, a, b);   // Low-level add() primitive.

   assert(c1 == c2);

Similarly to the ``mpz_add()`` function from the GMP API, mp++'s ``add()`` is a ternary function taking as first
parameter the return value (by reference), and as second and third parameters the operands. Although the end result is identical,
there is a fundamental difference in terms of the number of operations performed by the two approaches: the binary
operator returns a new value, which is then used to move-initialise ``c1`` [#f1]_, while the ``add()`` function writes
the result of the addition *directly* into an existing value. While, in practice, the difference might not
matter in the simple example above, it will start to matter in less trivial cases:

.. code-block:: c++

   std::vector<int_t> v = {...};

   int_t sum1;
   for (const auto &n: v) {
     sum1 = sum1 + n;    // Binary operator.
   }

   int_t sum2;
   for (const auto &n: v) {
     add(sum2, sum2, n); // Low-level add() function.
   }

   assert(sum1 == sum2);

In this example, we are computing the sum of the integral values held in a vector ``v``. When using the binary operator,
at each step of the iteration a new temporary :cpp:class:`~mppp::integer` is constructed by the expression ``sum1 + n``, and
this temporary value is then move-assigned to ``sum1``. When using the ``add()`` function, the creation of the temporary is
avoided altogether as the result of the addition is written directly into the accumulator ``sum2``. Thus, the first version
of the loop will necessarily be less efficient than the second one. Note that, in this specific case, we can recover optimal
performance while maintaining a nice syntax by replacing the binary operator with an in-place operator (which will avoid the
creation of unnecessary temporaries):

.. code-block:: c++

   int_t sum1;
   for (const auto &n: v) {
     sum1 += n; // In-place addition operator.
   }

Let's see another example:

.. code-block:: c++

   std::vector<int_t> v = {...};

   int_t gcd1;
   for (const auto &n: v) {
     gcd1 = gcd(gcd1, n); // Binary gcd() function.
   }

   int_t gcd2;
   for (const auto &n: v) {
     gcd(gcd2, gcd2, n);  // Ternary gcd() function.
   }

   assert(gcd1 == gcd2);

Here we are computing the GCD of the integers stored in the vector ``v``. mp++ provides two overloads for the :cpp:func:`~mppp::gcd()` function:

* a binary overload, taking as input the two operands, and returning their GCD,
* a ternary overload, taking the return value as first parameter and the two operands as second
  and third parameters.

Like in the previous example, the ternary overload avoids the creation and subsequent assignment of a temporary value, and will thus perform
better. The binary GCD overload, on the other hand, is easier to use (no need to prepare a return value beforehand) and closer
to a functional style. The presence of binary and ternary overloads for the same functionality is not restricted to :cpp:func:`~mppp::gcd()`,
but it is a common feature for many of mp++'s binary functions and operators.

For unary functions and operators, there's an additional degree of freedom in the API. Unary functions in mp++ are often provided with the
following set of overloads:

* an in-place nullary member function,
* a functional-style unary function,
* a GMP-style binary function.

As a concrete example, let's take a look at different ways of computing the absolute value of an integer:

.. code-block:: c++

   int_t n1{-5};
   n1.abs();              // In-place nullary member function.
   assert(n1 == 5);

   int_t n2{-5};
   auto n2_abs = abs(n2); // Unary function.
   assert(n2_abs == 5);

   int_t n3{-5}, n3_abs;
   abs(n3_abs, n3);       // GMP-style binary function.
   assert(n3_abs == 5);

The :cpp:func:`mppp::integer::abs()` member function computes and stores the absolute value directly into the calling object.
The unary function (much like ``std::abs()``) takes as input an integer and returns its absolute value. The GMP-style
binary ``abs()`` function stores into the first argument the absolute value of the second argument.

The nullary member function overload is provided to cater to the common use case in which a value is mutated in-place
through a unary operation (e.g., ``n = abs(n)``). Since the nullary member function overloads return a reference
to the calling object, they can be chained to perform a sequence of in-place operations on the same value:

.. code-block:: c++

   int_t n1{-16};
   n1.abs().sqrt().neg(); // Equivalent to: n1 = neg(sqrt(abs(n1)))
   assert(n1 == -4);

.. rubric:: Footnotes

.. [#f1] Of course, copy elision in this specific case will most likely eliminate any move operation. But, for the sake
         of argument, let's pretend that it will not :)
