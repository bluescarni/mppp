.. _tutorial_api:

API overview
============

The mp++ API operates on two broad levels:

* a low-level, performance-focused API mimicking the APIs of the multiprecision libraries
  on top of which mp++ is built,
* a high-level API focused on consistency and user-friendliness.

The low-level API should be preferred when maximum performance is required, whereas for a more
"casual" use of the library the high-level API should provide a better user experience with
adequate performance.

To give an idea of the type of tradeoffs involved in the choice between low-level and high-level
API, consider the simple task of adding two multiprecision integers ``a`` and ``b``. This can be accomplished
either via the overloaded binary operator for :cpp:class:`~mppp::integer`, or via the
lower-level ``add()`` function:

.. code-block:: c++

   int_t a{2}, b{4};

   auto c1 = a + b; // Binary operator.

   int_t c2;
   add(c2, a, b);   // Low-level add() function.

   assert(c1 == c2);

Similarly to the ``mpz_add()`` function from the GMP API, mp++'s ``add()`` is a ternary function taking as first
parameter the return value, and as second and third parameters the operands. Although the end result is identical,
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
of the loop will necessarily be less efficient than the second one.

In this case, we can recover optimal performance while maintaining a nice syntax by replacing the binary operator with
an in-place operator:

.. code-block:: c++

   int_t sum1;
   for (const auto &n: v) {
     sum1 += n; // In-place addition operator.
   }

This will avoid the creation of needless temporaries.

.. rubric:: Footnotes

.. [#f1] Of course, copy elision in this specific case will most likely eliminate any move operation. But, for the sake
         of argument, let's pretend that it will not :)
