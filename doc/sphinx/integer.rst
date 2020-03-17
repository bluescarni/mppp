.. _integer_reference:

Multiprecision integers
=======================

*#include <mp++/integer.hpp>*

The integer class
-----------------

.. cpp:class:: template <std::size_t SSize> mppp::integer

   Multiprecision integer class.

   This class represents arbitrary-precision signed integers. It acts as a wrapper around the GMP :cpp:type:`mpz_t` type, with
   a small value optimisation: integers whose size is up to ``SSize`` limbs are stored directly in the storage
   occupied by the :cpp:class:`~mppp::integer` object, without resorting to dynamic memory allocation. The value of
   ``SSize`` must be at least 1 and less than an implementation-defined upper limit. On most modern architectures,
   a limb contains either 32 or 64 bits of data. Thus, for instance, if ``SSize`` is set to 2 on a 64-bit system,
   the small value optimisation will be employed for all integral values less than :math:`2^{64 \times 2} = 2^{128}`.

   When the value of an :cpp:class:`~mppp::integer` is stored directly within the object, the *storage type* of the
   integer is said to be *static*. When the limb size of the integer exceeds the maximum value ``SSize``, the storage
   type becomes *dynamic*. The transition from static to dynamic storage happens transparently whenever the integer
   value becomes large enough. The demotion from dynamic to static storage usually needs to be requested explicitly.
   For values of ``SSize`` of 1 and 2, optimised implementations of basic arithmetic operations are employed,
   if supported by the target architecture and if the storage type is static. For larger values of ``SSize``,
   the ``mpn_`` low-level functions of the GMP API are used if the storage type is static. If the storage type is
   dynamic, the usual ``mpz_`` functions from the GMP API are used.

   Most of the functionality is exposed via plain :ref:`functions <integer_functions>`, with the
   general convention that the functions are named after the corresponding GMP functions minus the leading ``mpz_``
   prefix. For instance, the GMP call

   .. code-block:: c++

      mpz_add(rop,a,b);

   that writes the result of ``a + b`` into ``rop`` becomes simply

   .. code-block:: c++

      add(rop,a,b);

   where the ``add()`` function is resolved via argument-dependent lookup. Function calls with overlapping arguments
   are allowed, unless noted otherwise. Various :ref:`overloaded operators <integer_operators>` are also provided.

   Several facilities for interfacing with the GMP library are provided. Specifically, :cpp:class:`~mppp::integer`
   features:

   * a constructor and an assignment operator from the GMP integer type :cpp:type:`mpz_t`,
   * a :cpp:func:`~mppp::integer::get_mpz_t()` method that promotes ``this`` to dynamic
     storage and returns a pointer to the internal :cpp:type:`mpz_t` instance,
   * an :cpp:class:`mpz_view` class, an instance of which can be requested via the :cpp:func:`~mppp::integer::get_mpz_view()`
     method, which allows to use :cpp:class:`~mppp::integer` in the GMP API as a drop-in replacement for
     ``const`` :cpp:type:`mpz_t` function arguments.

   The :cpp:class:`mpz_view` class represent a read-only view of an integer object which is implicitly convertible to the type
   ``const`` :cpp:type:`mpz_t` and which is thus usable as an argument to GMP functions. For example:

   .. code-block:: c++

      mpz_t m;
      mpz_init_set_si(m,1); // Create an mpz_t with the value 1.
      integer<1> n{1}; // Initialize an integer with the value 1.
      mpz_add(m,m,n.get_mpz_view()); // Compute the result of n + m and store
                                     // it in m using the GMP API.

   See the documentation of :cpp:func:`~mppp::integer::get_mpz_view()` for more details about the :cpp:class:`mpz_view` class.
   Via the GMP interfacing facilities, it is thus possible to use directly the GMP C API with
   :cpp:class:`~mppp::integer` objects whenever necessary (e.g., if a GMP function has not been wrapped yet by mp++).

   The :cpp:class:`~mppp::integer` class supports a simple binary serialisation API, through member functions
   such as :cpp:func:`~mppp::integer::binary_save()` and :cpp:func:`~mppp::integer::binary_load()`, and the
   corresponding :ref:`free function overloads <integer_s11n>`.

   A :ref:`tutorial <tutorial_integer>` showcasing various features of :cpp:class:`~mppp::integer`
   is available.

   .. cpp:member:: static constexpr std::size_t ssize = SSize

      Alias for the static size.

   .. cpp:function:: integer()

      Default constructor.

      The default constructor initialises an integer with static storage type and value 0.

   .. cpp:function:: integer(const integer &other)

      Copy constructor.

      The copy constructor deep-copies *other* into ``this``, copying the original
      storage type as well.

      :param other: the object that will be copied into ``this``.

   .. cpp:function:: integer(integer &&other) noexcept

      Move constructor.

      The move constructor will leave *other* in an unspecified but valid state.
      The storage type of ``this`` will be the same as *other*'s.

      :param other: the object that will be moved into ``this``.

   .. cpp:function:: explicit integer(const mp_limb_t *p, std::size_t size)

      Constructor from an array of limbs.

      This constructor will initialise ``this`` with the content of the array
      sized *size* starting at *p*. The array is expected to contain
      the limbs of the desired value for ``this``, ordered from the least significant
      to the most significant.

      For instance, the following code:

      .. code-block:: c++

         mp_limb_t arr[] = {5,6,7};
         integer<1> n{arr, 3};

      will initialise ``n`` to the value :math:`5 + 6 \times 2^{N} + 7 \times 2^{2N}`,
      where :math:`N` is the compile-time GMP constant ``GMP_NUMB_BITS`` representing the number of
      value bits in a limb (typically 64 or 32, depending on the platform).

      This constructor always initialises ``this`` to a non-negative value,
      and it requires the most significant limb of *p* to be nonzero. It also requires
      every member of the input array not to be greater than the ``GMP_NUMB_MAX`` GMP constant.
      If *size* is zero, ``this`` will be initialised to zero without ever dereferencing *p*.

      .. warning::

         The effects of this constructor are highly dependent on the platform
         currently in use and also on the build configuration of the GMP library.
         Do not use it for portable initialisation.

      .. seealso::
         https://gmplib.org/manual/Low_002dlevel-Functions.html#Low_002dlevel-Functions


      :param p: a pointer to the beginning of the limbs array.
      :param size: the size of the limbs array.

      :exception std\:\:invalid_argument: if the last element of the *p* array is zero,
        or if at least one element of the *p* array is greater than ``GMP_NUMB_MAX``.
      :exception std\:\:overflow_error: if *size* is larger than an implementation-defined limit.

   .. cpp:function:: explicit integer(integer_bitcnt_t nbits)

      Constructor from number of bits.

      This constructor will initialise ``this`` to zero, allocating enough memory
      to represent a value with a magnitude of *nbits* binary digits. The storage type
      will be static if *nbits* is small enough, dynamic otherwise. For instance, the code

      .. code-block:: c++

         integer n{integer_bitcnt_t(64)};

      will initialise an integer ``n`` with value zero and enough storage for a 64-bit value.

      :param nbits: the number of bits of storage that will be allocated.

      :exception std\:\:overflow_error: if the value of *nbits* is larger than an
        implementation-defined limit.

   .. cpp:function:: template <CppInteroperable T> explicit integer(const T &x)

      Generic constructor from a C++ fundamental type.

      This constructor will initialize an integer with the value of *x*.
      The initialization is always successful if *x* is an integral value
      (construction from ``bool`` yields 1 for ``true``, 0 for ``false``).
      If *x* is a floating-point value, the construction will fail if *x*
      is not finite. Construction from a floating-point type yields the
      truncated counterpart of the input value.

      :param x: value that will be used to initialize ``this``.

      :exception std\:\:domain_error: if *x* is a non-finite floating-point value.

   .. cpp:function:: template <CppComplex T> explicit integer(const T &c)

      .. versionadded:: 0.19

      Generic constructor from a C++ complex type.

      This constructor will initialize an integer with the value of *c*. The initialization is
      successful only if the imaginary part of *c* is zero and the real part of *c* is finite.

      :param c: value that will be used to initialize ``this``.

      :exception std\:\:domain_error: if the imaginary part of *c* is not zero or if
        the real part of *c* is not finite.

   .. cpp:function:: template <StringType T> explicit integer(const T &s, int base = 10)

      Constructor from string.

      This constructor will initialize ``this`` from the :cpp:concept:`~mppp::StringType` *s*, which must represent
      an integer value in base *base*. The expected format is the same as specified by the ``mpz_set_str()``
      GMP function. *base* may vary from 2 to 62, or be zero. In the latter case, the base is inferred
      from the leading characters of the string.

      .. seealso::

         https://gmplib.org/manual/Assigning-Integers.html

      :param s: the input string.
      :param base: the base used in the string representation.

      :exception std\:\:invalid_argument: if the *base* parameter is invalid or if *s* is not a
        valid string representation of an integer in the specified base.
      :exception unspecified: any exception thrown by memory allocation errors in
        standard containers.

   .. cpp:function:: explicit integer(const char *begin, const char *end, int base = 10)

      Constructor from range of characters.

      This constructor will initialise ``this`` from the content of the input half-open range,
      which is interpreted as the string representation of an integer in base *base*.

      Internally, the constructor will copy the content of the range to a local buffer, add a
      string terminator, and invoke the constructor from string.

      :param begin: the begin of the input range.
      :param end: the end of the input range.
      :param base: the base used in the string representation.

      :exception unspecified: any exception thrown by the constructor from string, or by memory
        allocation errors in standard containers.

   .. cpp:function:: explicit integer(const mpz_t n)

      Copy constructor from :cpp:type:`mpz_t`.

      This constructor will initialize ``this`` with the value of the GMP integer *n*.
      The storage type of ``this`` will be static if *n* fits in the static storage,
      otherwise it will be dynamic.

      .. warning::

         It is the user's responsibility to ensure that *n* has been correctly
         initialized. Calling this constructor with an uninitialized *n*
         results in undefined behaviour.

      :param n: the input GMP integer.

   .. cpp:function:: explicit integer(mpz_t &&n)

      Move constructor from :cpp:type:`mpz_t`.

      This constructor will initialize ``this`` with the value of the
      GMP integer *n*, transferring the state of *n* into ``this``.
      The storage type of ``this`` will be static if *n* fits in the
      static storage, otherwise it will be dynamic.

      .. warning::

         It is the user's responsibility to ensure that *n* has been
         correctly initialized. Calling this constructor
         with an uninitialized *n* results in undefined behaviour.

         Additionally, the user must ensure that, after construction,
         ``mpz_clear()`` is never called on *n*: the resources previously
         owned by *n* are now owned by ``this``, which
         will take care of releasing them when the destructor is called.

      .. note::

         Due to a compiler bug, this constructor is not available on Microsoft Visual Studio.

      :param n: the input GMP integer.

   .. cpp:function:: integer &operator=(const integer &other)

      Copy assignment operator.

      This operator will perform a deep copy of *other*, copying its storage type as well.

      :param other: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: integer &operator=(integer &&other)

      Move assignment operator.

      After the move, *other* will be in an unspecified but valid state,
      and the storage type of ``this`` will be
      *other*'s original storage type.

      :param other: the assignment argument.

      :return: a reference to ``this``.

   .. cpp:function:: template <CppInteroperable T> integer &operator=(const T &x)

      Generic assignment operator from a fundamental C++ type.

      This operator will assign *x* to ``this``. The storage type of ``this`` after the assignment
      will depend only on the value of *x* (that is, the storage type will be static if the value of *x*
      is small enough, dynamic otherwise). Assignment from floating-point types will assign the truncated
      counterpart of *x*.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception std\:\:domain_error: if *x* is a non-finite floating-point value.

   .. cpp:function:: template <CppComplex T> integer &operator=(const T &c)

      .. versionadded:: 0.19

      Generic assignment operator from a complex C++ type.

      This operator will assign *c* to ``this``. The storage type of ``this`` after the assignment
      will depend only on the value of *c* (that is, the storage type will be static if the value of *c*
      is small enough, dynamic otherwise). The assignment will be successful only if
      the imaginary part of *c* is zero and the real part of *c* is finite.

      :param c: the assignment argument.

      :return: a reference to ``this``.

      :exception std\:\:domain_error: if the imaginary part of *c* is not zero or if
        the real part of *c* is not finite.

   .. cpp:function:: template <StringType T> integer &operator=(const T &s)

      Assignment from string.

      The body of this operator is equivalent to:

      .. code-block:: c++

         return *this = integer{s};

      That is, a temporary integer is constructed from the :cpp:concept:`~mppp::StringType`
      *s* and it is then move-assigned to ``this``.

      :param s: the string that will be used for the assignment.

      :return: a reference to ``this``.

      :exception unspecified: any exception thrown by the constructor from string.

   .. cpp:function:: integer &operator=(const mpz_t n)

      Copy assignment from :cpp:type:`mpz_t`.

      This assignment operator will copy into ``this`` the value of the GMP integer *n*.
      The storage type of ``this`` after the assignment will be static if *n* fits in
      the static storage, otherwise it will be dynamic.

      .. warning::

         It is the user's responsibility to ensure that *n* has been correctly initialized. Calling this operator
         with an uninitialized *n* results in undefined behaviour. Also, no aliasing is allowed: the data in *n*
         must be completely distinct from the data in ``this`` (e.g., if *n* is an :cpp:class:`~mppp::integer::mpz_view`
         of ``this`` then it might point to internal data of ``this``, and the behaviour of this operator will thus be undefined).

      :param n: the input GMP integer.

      :return: a reference to ``this``.

   .. cpp:function:: integer &operator=(mpz_t &&n)

      Move assignment from :cpp:type:`mpz_t`.

      This assignment operator will move into ``this`` the GMP integer *n*. The storage type of ``this``
      after the assignment will be static if *n* fits in the static storage, otherwise it will be dynamic.

      .. warning::

         It is the user's responsibility to ensure that *n* has been correctly initialized. Calling this operator
         with an uninitialized *n* results in undefined behaviour. Also, no aliasing is allowed: the data in *n*
         must be completely distinct from the data in ``this`` (e.g., if *n* is an :cpp:class:`~mppp::integer::mpz_view`
         of ``this`` then it might point to internal data of ``this``, and the behaviour of this operator will thus be undefined).

         Additionally, the user must ensure that, after the assignment, ``mpz_clear()`` is never
         called on *n*: the resources previously owned by *n* are now owned by ``this``, which
         will take care of releasing them when the destructor is called.

      .. note::

         Due to a compiler bug, this operator is not available on Microsoft Visual Studio.

      :param n: the input GMP integer.

      :return: a reference to ``this``.

   .. cpp:function:: integer &set_zero()
   .. cpp:function:: integer &set_one()
   .. cpp:function:: integer &set_negative_one()

      Set to :math:`0`, :math:`1` or :math:`-1`.

      After calling these member functions, the storage type of ``this`` will be static.

      .. note::

         These are specialised higher-performance alternatives to the assignment operators.

      :return: a reference to ``this``.

   .. cpp:function:: bool is_static() const
   .. cpp:function:: bool is_dynamic() const

      Query the storage type currently in use.

      :return: ``true`` if the current storage type is static (resp. dynamic),
        ``false`` otherwise.

   .. cpp:function:: std::string to_string(int base = 10) const

      Conversion to string.

      This member function will convert ``this`` into a string in base *base*
      using the GMP function ``mpz_get_str()``.

      .. seealso::

         https://gmplib.org/manual/Converting-Integers.html

      :param base: the desired base.

      :return: a string representation of ``this``.

      :exception std\:\:invalid_argument: if *base* is smaller than 2 or greater than 62.

   .. cpp:function:: template <CppInteroperable T> explicit operator T() const

      Generic conversion operator to a C++ fundamental type.

      This operator will convert ``this`` to a :cpp:concept:`~mppp::CppInteroperable` type.
      Conversion to ``bool`` yields ``false`` if ``this`` is zero,
      ``true`` otherwise. Conversion to other integral types yields the exact result, if representable by the target
      :cpp:concept:`~mppp::CppInteroperable` type. Conversion to floating-point types might yield inexact values and
      infinities.

      :return: ``this`` converted to the target type.

      :exception std\:\:overflow_error: if the target type is an integral type and the value of
        ``this`` cannot be represented by it.

   .. cpp:function:: template <CppComplex T> explicit operator T() const

      .. versionadded:: 0.19

      Generic conversion operator to a C++ complex type.

      This operator will convert ``this`` to a :cpp:concept:`~mppp::CppComplex` type.
      The conversion might yield inexact values and infinities.

      :return: ``this`` converted to the target type.

   .. cpp:function:: template <CppInteroperable T> bool get(T &rop) const

      Generic conversion member function to a C++ fundamental type.

      This member function, similarly to the conversion operator, will convert ``this`` to a
      :cpp:concept:`~mppp::CppInteroperable` type, storing the result of the conversion into *rop*. Differently
      from the conversion operator, this member function does not raise any exception: if the conversion is successful,
      the member function will return ``true``, otherwise the member function will return ``false``. If the conversion
      fails, *rop* will not be altered.

      :param rop: the variable which will store the result of the conversion.

      :return: ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail only if *rop* is
        a C++ integral which cannot represent the value of ``this``.

   .. cpp:function:: template <CppComplex T> bool get(T &rop) const

      .. versionadded:: 0.19

      Generic conversion member function to a C++ complex type.

      This member function, similarly to the conversion operator, will convert ``this`` to a
      :cpp:concept:`~mppp::CppComplex` type, storing the result of the conversion into *rop*.
      The conversion is always successful, and this member function
      will always return ``true``.

      :param rop: the variable which will store the result of the conversion.

      :return: ``true``.

   .. cpp:function:: bool promote()

      Promote to dynamic storage.

      This member function will promote the storage type of ``this`` from static to dynamic.

      :return: ``false`` if the storage type of ``this`` is already dynamic and no promotion
        takes place, ``true`` otherwise.

   .. cpp:function:: bool demote()

      Demote to static storage.

      This member function will demote the storage type of ``this`` from dynamic to static.

      :return: ``false`` if the storage type of ``this`` is already static and no demotion
        takes place, or if the current value of ``this`` does not fit in static storage,
        ``true`` otherwise.

   .. cpp:function:: std::size_t nbits() const
   .. cpp:function:: std::size_t size() const

      Size in bits or limbs.

      :return: the number of bits/limbs needed to represent ``this``. If ``this``
        is zero, zero will be returned.

      :exception std\:\:overflow_error: if the size in bits of ``this`` is
        larger than an implementation-defined value.

   .. cpp:function:: int sgn() const

      Sign.

      :return: :math:`0` if ``this`` is zero, :math:`1` if ``this`` is positive,
        :math:`-1` if ``this`` is negative.

   .. cpp:class:: mpz_view

      Read-only view onto an :cpp:type:`mpz_t`.

      This is a proxy class with an implicit conversion operator to a ``const`` pointer
      to the ``struct`` underlying :cpp:type:`mpz_t`. Thus, objects of this class can be
      passed as read-only parameters to GMP functions.

      In addition to the implicit conversion operator, a ``get()`` member function
      provides the same functionality (i.e., conversion to a ``const`` pointer
      to the ``struct`` underlying :cpp:type:`mpz_t`).

      Objects of this class can only be constructed by :cpp:func:`mppp::integer::get_mpz_view()`,
      or move-constructed. All assignment operators are disabled.

   .. cpp:function:: mpz_view get_mpz_view() const

      Get a GMP-compatible read-only view of ``this``.

      This member function will return an :cpp:class:`mpz_view` object containing
      a read-only GMP-compatible representation of the value stored in ``this``.
      That is, the return value of this function can be used in the GMP API
      where a ``const`` :cpp:type:`mpz_t` parameter is expected.

      .. warning::

         Because the returned object is a non-owning view of ``this``,
         it is important to keep in mind the following facts in order
         to avoid undefined behaviour at runtime:

         * the returned object and the pointer returned by its conversion operator
           might reference internal data belonging to ``this``, and they can
           thus be safely used only during the lifetime of ``this``;
         * the lifetime of the pointer returned by the conversion operator
           of the returned object is tied to the lifetime of the
           returned object itself (that is, if the :cpp:class:`mpz_view` object is
           destroyed,
           any pointer previously returned by its conversion operator becomes invalid);
         * any modification to ``this`` will also invalidate the view and the pointer.

      :return: an :cpp:class:`mpz_view` of ``this``.

Types
-----

.. cpp:type:: mpz_t

   This is the type used by the GMP library to represent multiprecision integers.
   It is defined as an array of size 1 of an unspecified structure.

   .. seealso::

      https://gmplib.org/manual/Nomenclature-and-Types.html#Nomenclature-and-Types

.. cpp:type:: mp_limb_t

   This type is defined by the GMP library. It is used to represents a limb, that is,
   the part of a multiprecision integer that fits in a single machine word. This is an
   unsigned integral type, typically 64 or 32 bits wide.

   .. seealso::

      https://gmplib.org/manual/Nomenclature-and-Types.html#Nomenclature-and-Types

.. cpp:type:: mp_bitcnt_t

   This type is defined by the GMP library. It is an unsigned integral type used to count bits in a multiprecision
   number.

   .. seealso::

      https://gmplib.org/manual/Nomenclature-and-Types.html#Nomenclature-and-Types

.. cpp:enum-class:: mppp::integer_bitcnt_t : mp_bitcnt_t

   A strongly-typed counterpart to :cpp:type:`mp_bitcnt_t`, used in the constructor of :cpp:class:`~mppp::integer`
   from number of bits.

Concepts
--------

.. cpp:concept:: template <typename T, typename U> mppp::IntegerOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <integer_operators>` and :ref:`functions <integer_functions>`
   involving :cpp:class:`~mppp::integer`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::integer` with the same static size ``SSize``, or
   * one type is an :cpp:class:`~mppp::integer` and the other is a :cpp:concept:`~mppp::CppInteroperable`
     or :cpp:concept:`~mppp::CppComplex` type.

   Note that the modulo, bit-shifting and bitwise logic operators have additional restrictions.

   A corresponding boolean type trait called ``are_integer_op_types`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, typename U> mppp::IntegerRealOpTypes

   This concept will be ``true`` if:

   * ``T`` and ``U`` satisfy :cpp:concept:`~mppp::IntegerOpTypes`, and
   * neither ``T`` nor ``U`` satisfy :cpp:concept:`~mppp::CppComplex`.

   A corresponding boolean type trait called ``are_integer_real_op_types`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, typename U> mppp::IntegerIntegralOpTypes

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <integer_operators>` and :ref:`functions <integer_functions>`
   involving :cpp:class:`~mppp::integer` and C++ integral types. Specifically, the concept will be ``true``
   if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::integer` with the same static size, or
   * one type is an :cpp:class:`~mppp::integer` and the other is a :cpp:concept:`~mppp::CppIntegralInteroperable` type.

   A corresponding boolean type trait called ``are_integer_integral_op_types`` is also available (even if the compiler does
   not support concepts).

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::IntegerBinarySaveDest

   This concept is satisfied if ``T`` is a type into which the serialised binary representation of an
   :cpp:class:`~mppp::integer` with static size ``SSize`` can be written. In other words, the concept is satisfied if
   an object of type ``T`` can be passed as an argument to one of the :cpp:func:`mppp::integer::binary_save()` overloads.

.. cpp:concept:: template <typename T, std::size_t SSize> mppp::IntegerBinaryLoadSrc

   This concept is satisfied if ``T`` is a type from which the serialised binary representation of an
   :cpp:class:`~mppp::integer` with static size ``SSize`` can be loaded. In other words, the concept is satisfied if
   an object of type ``T`` can be passed as an argument to one of the :cpp:func:`mppp::integer::binary_load()` overloads.

.. _integer_functions:

Functions
---------

Much of the functionality of the :cpp:class:`~mppp::integer` class
is exposed via plain functions. These functions
mimic the `GMP API <https://gmplib.org/manual/Integer-Functions.html>`__ where appropriate, but a variety of
convenience/generic overloads is provided as well.

.. _integer_assignment:

Assignment
~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::set_zero(mppp::integer<SSize> &n)

   Set to zero.

   After calling this function, the storage type of *n* will be static and its value will be zero.

   .. note::

      This is a specialised higher-performance alternative to the assignment operator.

   :param n: the argument.

   :return: a reference to *n*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::set_one(mppp::integer<SSize> &n)

   Set to one.

   After calling this function, the storage type of *n* will be static and its value will be one.

   .. note::

      This is a specialised higher-performance alternative to the assignment operator.

   :param n: the argument.

   :return: a reference to *n*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::set_negative_one(mppp::integer<SSize> &n)

   Set to minus one.

   After calling this function, the storage type of *n* will be static and its value will be minus one.

   .. note::

      This is a specialised higher-performance alternative to the assignment operator.

   :param n: the argument.

   :return: a reference to *n*.

.. cpp:function:: template <std::size_t SSize> void mppp::swap(mppp::integer<SSize> &n1, mppp::integer<SSize> &n2) noexcept

   .. versionadded:: 0.15

   Swap.

   This function will efficiently swap the values of *n1* and *n2*.

   :param n1: the first argument.
   :param n2: the second argument.

.. _integer_conversion:

Conversion
~~~~~~~~~~

.. cpp:function:: template <mppp::CppInteroperable T, std::size_t SSize> bool mppp::get(T &rop, const mppp::integer<SSize> &n)

   Generic conversion function from :cpp:class:`~mppp::integer` to C++ fundamental types.

   This function will convert the input :cpp:class:`~mppp::integer` *n* to a
   :cpp:concept:`~mppp::CppInteroperable` type, storing the result of the conversion into *rop*.
   If the conversion is successful, the function
   will return ``true``, otherwise the function will return ``false``. If the conversion fails, *rop* will
   not be altered.

   :param rop: the variable which will store the result of the conversion.
   :param n: the input :cpp:class:`~mppp::integer`.

   :return: ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail only if *rop* is
     a C++ integral which cannot represent the value of *n*.

.. cpp:function:: template <mppp::CppComplex T, std::size_t SSize> bool mppp::get(T &rop, const mppp::integer<SSize> &n)

   .. versionadded:: 0.19

   Generic conversion function from :cpp:class:`~mppp::integer` to C++ complex types.

   This function will convert the input :cpp:class:`~mppp::integer` *n* to a
   :cpp:concept:`~mppp::CppComplex` type, storing the result of the conversion into *rop*.
   The conversion is always successful, and this function will always return ``true``.

   :param rop: the variable which will store the result of the conversion.
   :param n: the input :cpp:class:`~mppp::integer`.

   :return: ``true``.

.. _integer_arithmetic:

Arithmetic
~~~~~~~~~~

.. doxygengroup:: integer_arithmetic
   :content-only:

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::sqr(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n)

   .. versionadded:: 0.18

   Binary :cpp:class:`~mppp::integer` squaring.

   This function will set *rop* to the square of *n*.

   :param rop: the return value.
   :param n: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::sqr(const mppp::integer<SSize> &n)

   .. versionadded:: 0.18

   Unary :cpp:class:`~mppp::integer` squaring.

   This function will return the square of *n*.

   :param n: the argument.

   :return: the square of *n*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::sqrm(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n, const mppp::integer<SSize> &mod)

   .. versionadded:: 0.18

   Ternary modular :cpp:class:`~mppp::integer` squaring.

   This function will set *rop* to the square of *n* modulo *mod*.

   :param rop: the return value.
   :param n: the argument.
   :param mod: the modulus.

   :return: a reference to *rop*.

   :exception mppp\:\:zero_division_error: if *mod* is zero.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::sqrm(const mppp::integer<SSize> &n, const mppp::integer<SSize> &mod)

   .. versionadded:: 0.18

   Binary modular :cpp:class:`~mppp::integer` squaring.

   This function will return the square of *n* modulo *mod*.

   :param n: the argument.
   :param mod: the modulus.

   :return: the square of *n* modulo *mod*.
   :exception mppp\:\:zero_division_error: if *mod* is zero.

.. _integer_division:

Division
~~~~~~~~

.. doxygengroup:: integer_division
   :content-only:

.. _integer_comparison:

Comparison
~~~~~~~~~~

.. doxygengroup:: integer_comparison
   :content-only:

.. _integer_logic:

Logic and bit fiddling
~~~~~~~~~~~~~~~~~~~~~~

.. versionadded:: 0.6

.. doxygengroup:: integer_logic
   :content-only:

.. _integer_ntheory:

Number theoretic functions
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygengroup:: integer_ntheory
   :content-only:

.. _integer_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::pow_ui(mppp::integer<SSize> &rop, const mppp::integer<SSize> &base, unsigned long exp)

   Ternary :cpp:class:`~mppp::integer` exponentiation.

   This function will set *rop* to ``base**exp``.

   :param rop: the return value.
   :param base: the base.
   :param exp: the exponent.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::pow_ui(const mppp::integer<SSize> &base, unsigned long exp)

   Binary :cpp:class:`~mppp::integer` exponentiation.

   :param base: the base.
   :param exp: the exponent.

   :return: ``base**exp``.

.. cpp:function:: template <typename T, typename U> auto mppp::pow(const T &base, const U &exp)

   .. note::

      This function participates in overload resolution only if ``T`` and ``U`` satisfy
      the :cpp:concept:`~mppp::IntegerOpTypes` concept.

   Generic binary :cpp:class:`~mppp::integer` exponentiation.

   This function will raise *base* to the power *exp*, and return the result. If one of the arguments
   is a floating-point or complex value, then the result will be computed via ``std::pow()`` and it will
   also be a floating-point or complex value. Otherwise, the result will be an :cpp:class:`~mppp::integer`.
   In case of a negative integral exponent and integral base, the result will be zero unless
   the absolute value of ``base`` is 1.

   :param base: the base.
   :param exp: the exponent.

   :return: ``base**exp``.

   :exception std\:\:overflow_error: if *base* and *exp* are integrals and *exp* is non-negative and outside the range
     of ``unsigned long``.
   :exception mppp\:\:zero_division_error: if *base* and *exp* are integrals and *base* is zero and *exp* is negative.

.. _integer_roots:

Roots
~~~~~

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::sqrt(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n)

   Binary :cpp:class:`~mppp::integer` square root.

   This function will set *rop* to the truncated integer part of the square root of *n*.

   :param rop: the return value.
   :param n: the argument.

   :return: a reference to *rop*.

   :exception std\:\:domain_error: if *n* is negative.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::sqrt(const mppp::integer<SSize> &n)

   Unary :cpp:class:`~mppp::integer` square root.

   This function will return the truncated integer part of the square root of *n*.

   :param n: the argument.

   :return: the integer square root of *n*.

   :exception std\:\:domain_error: if *n* is negative.

.. cpp:function:: template <std::size_t SSize> void mppp::sqrtrem(mppp::integer<SSize> &rop, mppp::integer<SSize> &rem, const mppp::integer<SSize> &n)

   .. versionadded:: 0.12

   :cpp:class:`~mppp::integer` square root with remainder.

   This function will set *rop* to the truncated integer part of the square root of *n*, and *rem* to the remainder of the operation.
   That is, *rem* will be equal to ``n-rop*rop``, and it will be zero if *n* is a perfect square.

   *rop* and *rem* must be distinct objects.

   :param rop: the first return value (i.e., the integer square root of *n*).
   :param rem: the second return value (i.e., the remainder of the operation).
   :param n: the argument.

   :exception std\:\:domain_error: if *n* is negative.
   :exception std\:\:invalid_argument: if *rop* and *rem* are the same object.

.. cpp:function:: template <std::size_t SSize> bool mppp::perfect_square_p(const mppp::integer<SSize> &n)

   .. versionadded:: 0.12

   Detect perfect square.

   This function returns ``true`` if *n* is a perfect square, ``false`` otherwise.

   :param n: the argument.

   :return: ``true`` if *n* is a perfect square, ``false`` otherwise.

.. cpp:function:: template <std::size_t SSize> bool mppp::root(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n, unsigned long m)

   .. versionadded:: 0.12

   Ternary :math:`m`-th root.

   This function will set *rop* to the truncated integer part of the :math:`m`-th root of *n*. The return value will be ``true`` if the
   computation is exact, ``false`` otherwise.

   :param rop: the return value.
   :param n: the argument.
   :param m: the degree of the root.

   :return: ``true`` if the computation is exact, ``false`` otherwise.

   :exception std\:\:domain_error: if *m* is even and *n* is negative, or if *m* is zero.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::root(const mppp::integer<SSize> &n, unsigned long m)

   .. versionadded:: 0.12

   Binary :math:`m`-th root.

   This function will return the truncated integer part of the :math:`m`-th root of *n*.

   :param n: the argument.
   :param m: the degree of the root.

   :return: the truncated integer part of the :math:`m`-th root of *n*.

   :exception std\:\:domain_error: if *m* is even and *n* is negative, or if *m* is zero.

.. cpp:function:: template <std::size_t SSize> void mppp::rootrem(mppp::integer<SSize> &rop, mppp::integer<SSize> &rem, const mppp::integer<SSize> &n, unsigned long m)

   .. versionadded:: 0.12

   :math:`m`-th root with remainder.

   This function will set *rop* to the truncated integer part of the :math:`m`-th root of *n*, and *rem* to the remainder
   of the operation. That is, *rem* will be equal to ``n-rop**m``, and it will be zero if *n* is a perfect power.

   :param rop: the first return value (i.e., the :math:`m`-th root root of *n*).
   :param rem: the second return value (i.e., the remainder of the operation).
   :param n: the argument.
   :param m: the degree of the root.

   :exception std\:\:domain_error: if *m* is even and *n* is negative, or if *m* is zero.

.. cpp:function:: template <std::size_t SSize> bool mppp::perfect_power_p(const mppp::integer<SSize> &n)

   .. versionadded:: 0.12

   Detect perfect power.

   This function will return ``true`` if *n* is a perfect power, that is, if there exist integers :math:`a` and :math:`b`,
   with :math:`b>1`, such that *n* equals :math:`a^b`.  Otherwise, the function will return ``false``.

   :param n: the argument.

   :return: ``true`` if *n* is a perfect power, ``false`` otherwise.

.. _integer_io:

Input/Output
~~~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> std::ostream &mppp::operator<<(std::ostream &os, const mppp::integer<SSize> &n)

   Stream insertion operator.

   This function will direct to the output stream *os* the input :cpp:class:`~mppp::integer` *n*.

   :param os: the output stream.
   :param n: the input :cpp:class:`~mppp::integer`.

   :return: a reference to *os*.

   :exception std\:\:overflow_error: in case of (unlikely) overflow errors.
   :exception unspecified: any exception raised by the public interface of ``std::ostream`` or by memory allocation errors.

.. _integer_s11n:

Serialisation
~~~~~~~~~~~~~

.. versionadded:: 0.7

.. doxygengroup:: integer_s11n
   :content-only:

.. _integer_other:

Other
~~~~~

.. doxygengroup:: integer_other
   :content-only:

.. _integer_operators:

Mathematical operators
----------------------

Overloaded operators are provided for convenience.
Their interface is generic, and their implementation
is typically built on top of basic :ref:`functions <integer_functions>`.

.. doxygengroup:: integer_operators
   :content-only:

.. _integer_std_specialisations:

Standard library specialisations
--------------------------------

.. cpp:class:: template <std::size_t SSize> std::hash<mppp::integer<SSize>>

   Specialisation of ``std::hash`` for :cpp:class:`mppp::integer`.

   .. cpp:type:: public argument_type = mppp::integer<SSize>
   .. cpp:type:: public result_type = std::size_t

   .. note::

      The :cpp:type:`argument_type` and :cpp:type:`result_type` type aliases are defined only until C++14.

   .. cpp:function:: public std::size_t operator()(const mppp::integer<SSize> &n) const

      :param n: the input :cpp:class:`mppp::integer`.

      :return: a hash value for *n*.

.. _integer_literals:

User-defined literals
---------------------

.. versionadded:: 0.18

.. cpp:function:: template <char... Chars> mppp::integer<1> mppp::literals::operator"" _z1()
.. cpp:function:: template <char... Chars> mppp::integer<2> mppp::literals::operator"" _z2()
.. cpp:function:: template <char... Chars> mppp::integer<3> mppp::literals::operator"" _z3()

   User-defined integer literals.

   These numeric literal operator templates can be used to construct
   :cpp:class:`~mppp::integer` instances with, respectively, 1, 2 and 3
   limbs of static storage. Literals in binary, octal, decimal and
   hexadecimal format are supported.

   .. seealso::

      https://en.cppreference.com/w/cpp/language/integer_literal

   :exception std\:\:invalid_argument: if the input sequence of characters is not
     a valid integer literal (as defined by the C++ standard).
