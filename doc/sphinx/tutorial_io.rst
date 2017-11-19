Input and output
================

All of mp++'s multiprecision classes support stream insertion and extraction. E.g., values can be printed
to screen in the canonical C++ way:

.. code-block:: c++

   std::cout << int_t{42} << '\n';        // "42"
   std::cout << rat_t{-6/7} << '\n';      // "-6/7"
   std::cout << real128{"1.1"} << '\n';   // "1.10000000000000000000000000000000008e+00"
   std::cout << real{"1.3", 150} << '\n'; // "1.3000000000000000000000000000000000000000000006"

.. note::

   A current limitation of all stream insertion operators is that they ignore any formatting flag that may be set in the stream
   object. This limitation will be lifted in the future.

Stream extraction is equivalent to construction from string:

.. code-block:: c++

   std::istringstream iss;

   int_t n;
   iss.str("123");
   iss >> n;
   assert(n == 123);
   iss.clear();

   rat_t q;
   iss.str("1/3");
   iss >> q;
   assert((q == rat_t{1, 3}));

All of mp++'s multiprecision classes also provide ``to_string()`` member functions that convert the multiprecision
values into string representations (see, e.g., :cpp:func:`mppp::integer::to_string()`, :cpp:func:`mppp::rational::to_string()`,
etc.). These member functions always return an "exact" string representation of the multiprecision value: feeding back
the string representation to a constructor from string will initialise a value identical to the original one.
