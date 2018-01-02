.. _tutorial_integer:

Integer tutorial
================

:cpp:class:`~mppp::integer` was the first class implemented in mp++, and as a result it is currently
the most featureful and optimised among mp++'s multiprecision classes.

One of the first things that can be noticed about :cpp:class:`~mppp::integer` is that the class is parametrised over
an integral value ``SSize``, called the *static size*. This compile-time constant represents the number of *limbs* that will be
stored directly within an :cpp:class:`~mppp::integer` object before resorting to dynamic memory allocation.
A limb is the part of a multiprecision integer that fits in a single word-sized unsigned integral type:
in the same way an integral value in base 10 is represented as a sequence of digits in the :math:`\left[0,9\right]` range,
an :cpp:class:`~mppp::integer` object is represented internally by an array of 32-bit or 64-bit
unsigned C++ integral values. Thus, for instance, if ``SSize`` is set to 2 on a 64-bit system,
:cpp:class:`~mppp::integer` is able to represent values in the :math:`\left(-2^{128},2^{128}\right)` range
without resorting to dynamic memory allocation. In general, an ``SSize`` value of 1 is a good default choice for many
use cases.

In addition to the :ref:`common operators <tutorial_commonops>` available for all of mp++'s multiprecision classes,
:cpp:class:`~mppp::integer` supports the following additional operators:

* the modulo operator ``%``, which computes the remainder of integer division,
* the bitshifting operators ``<<`` and ``>>``,
* the bitwise logical operators ``~``, ``&``, ``|`` and ``^``.

In addition to the binary versions of these operators, the in-place versions are also available. Lower level ternary
primitives (e.g., :cpp:func:`~mppp::mul_2exp()`, :cpp:func:`~mppp::tdiv_q_2exp()`, :cpp:func:`~mppp::bitwise_ior()`, etc.)
are also provided for those situations in which it is desirable to pass the return value as a function
parameter, rather than creating a new return value (as explained earlier in the :ref:`API overview <tutorial_api>`).
For consistency with C++11, the ``%`` operator returns a remainder with the same sign as the dividend. The bit-shifting
operators ``<<`` and ``>>`` correspond respectively to multiplication and division by a power of 2. The bitwise logical
operators behave as-if :cpp:class:`~mppp::integer` used a two's complement representation (even if internally
a sign-magnitude representation is used instead).

Among the features specific to :cpp:class:`~mppp::integer` we find:

* specialised arithmetic primitives (fused multiply-add/sub, addition/subtraction with C++ unsigned
  integrals, ...),
* functionality related to prime numbers (:cpp:func:`~mppp::probab_prime_p()`, :cpp:func:`~mppp::nextprime()`, ...),
* parity detection (:cpp:func:`~mppp::even_p()`, :cpp:func:`~mppp::odd_p()`),
* several additional :ref:`division functions <integer_division>`,
* :ref:`number-theoretic functions <integer_ntheory>` (GCD, factorial, binomial coefficient, ...),
* integer :ref:`roots <integer_roots>` and :ref:`exponentiation <integer_exponentiation>`,
* hashing (including a specialisation of ``std::hash``, so that it is possible to use
  :cpp:class:`~mppp::integer` in standard unordered containers out of the box),
* various utility functions specific to :cpp:class:`~mppp::integer` objects (detect size in bits/limbs,
  detect and/or promote/demote storage type, ...).

Many of these features, which are documented in detail in the :ref:`integer reference <integer_reference>`, are available
in multiple overloads, often both as free and member functions.

Interacting with the GMP API
----------------------------

:cpp:class:`~mppp::integer` provides a variety of ways for interfacing with the `GMP <https://gmplib.org/>`__ library.
There are a few reasons why one would want to use :cpp:class:`~mppp::integer` in conjunction with the GMP API, such as:

* the necessity of using functions from the GMP API which have not (yet) been wrapped/implemented by mp++,
* passing data from/to mp++ to/from another GMP-based multiprecision library.

To start with, :cpp:class:`~mppp::integer` is constructible and assignable from ``mpz_t`` objects:

.. code-block:: c++

   mpz_t m;
   mpz_init_set_si(m, -4);  // Init an mpz_t with the value -4.

   int_t n1{m};             // Init an int_t from the mpz_t.
   assert(n1 == -4);        // Verify that the value is correct.

   int_t n2;
   n2 = m;                  // Assign the mpz_t to another int_t.
   assert(n2 == -4);        // Verify that the value is correct.

   mpz_clear(m);            // Clear the mpz_t.

Second, it is possible to get a reference to an ``mpz_t`` from an :cpp:class:`~mppp::integer`
via the :cpp:func:`~mppp::integer::get_mpz_t()` member function. This member function will
first switch the calling :cpp:class:`~mppp::integer` to dynamic storage (if the calling
:cpp:class:`~mppp::integer` is not already employing dynamic storage), and it will then return
a raw non-owning pointer which can be used both as a const and mutable parameter in the GMP API.
For example:

.. code-block:: c++

   mpz_t b;
   mpz_init_set_si(b, -4);                   // Init an mpz_t with the value -4.

   int_t a, c{2};                            // Init two integers.

   mpz_add(a.get_mpz_t(), b, c.get_mpz_t()); // Compute b + c via the GMP API, storing the result in a.

   assert(a == -2);                          // Verify that the result is correct.

   mpz_clear(b);                             // Clear the mpz_t.

It is important to emphasise that :cpp:func:`~mppp::integer::get_mpz_t()` forces the use of dynamic storage,
thus incurring in a potential performance hit. If only const access is needed, a better alternative to
:cpp:func:`~mppp::integer::get_mpz_t()` is the :cpp:func:`~mppp::integer::get_mpz_view()` member function.
:cpp:func:`~mppp::integer::get_mpz_view()` returns a read-only view of
the calling :cpp:class:`~mppp::integer` which is implicitly convertible to a ``const mpz_t``, and which can thus be
used as a non-mutable function parameter in the GMP API. The creation of the read-only view is lightweight,
and, crucially, it does not force the use of dynamic storage in the calling :cpp:class:`~mppp::integer`.
We can slightly modify to previous example to use a read-only view as the third parameter in the ``mpz_add()`` call,
and verify that the creation of the read-only view did not trigger a promotion from static to dynamic storage:

.. code-block:: c++

   mpz_t b;
   mpz_init_set_si(b, -4);                      // Init an mpz_t with the value -4.

   int_t a, c{2};                               // Init two integers.

   mpz_add(a.get_mpz_t(), b, c.get_mpz_view()); // Compute b + c via the GMP API, storing the result in a.

   assert(a == -2);                             // Verify that the result is correct.
   assert(c.is_static());                       // Verify that c is still using static storage.

   mpz_clear(b);                                // Clear the mpz_t.

It must be noted that both :cpp:func:`~mppp::integer::get_mpz_t()` and :cpp:func:`~mppp::integer::get_mpz_view()`
have to be used carefully, as they return non-owning objects which can easily lead to dangling pointers or references, if misused.
The documentation of the two functions explains in detail some of the potential pitfalls that users need to be aware of.

Serialisation
-------------

.. versionadded:: 0.7

mp++ provides a simple :ref:`API for the (de)serialisation <integer_s11n>` of :cpp:class:`~mppp::integer` objects
into/from memory buffers and C++ streams. Possible uses of the serialisation API include persistence (e.g.,
saving/loading :cpp:class:`~mppp::integer` values to/from a file), the transmission of :cpp:class:`~mppp::integer` objects over
the network (e.g., in distributed computing applications), inter-process communication, etc. The API consists of two main
functions, :cpp:func:`mppp::integer::binary_save()` and :cpp:func:`mppp::integer::binary_load()` (plus their
free-function overloads).

Let's see a few examples of the serialisation API in action:

.. code-block:: c++

   int_t a{42}, b;
   char buffer[1024];     // Provide ample storage for serialisation.

   a.binary_save(buffer); // Serialise a into the buffer.

   b.binary_load(buffer); // Deserialise the content of the buffer into b.

   assert(b == a);        // Check that the original value is recovered.
