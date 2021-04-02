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

   .. cpp:function:: template <integer_cpp_arithmetic T> explicit integer(const T &x)

      Generic constructor from arithmetic C++ types.

      .. note::

         This constructor is not ``explicit`` if ``T`` satisfies :cpp:concept:`cpp_integral`.

      This constructor will initialize an integer with the value of *x*.
      The initialisation is always successful if *x* is an integral value
      (construction from ``bool`` yields 1 for ``true``, 0 for ``false``).
      If *x* is a floating-point value, the construction will fail if *x*
      is not finite. Construction from a floating-point type yields the
      truncated counterpart of the input value.

      :param x: value that will be used to initialize ``this``.

      :exception std\:\:domain_error: if *x* is a non-finite floating-point value.

   .. cpp:function:: template <integer_cpp_complex T> explicit integer(const T &c)

      .. versionadded:: 0.19

      Generic constructor from complex C++ types.

      This constructor will initialize an integer with the value of *c*. The initialisation is
      successful only if the imaginary part of *c* is zero and the real part of *c* is finite.

      :param c: value that will be used to initialize ``this``.

      :exception std\:\:domain_error: if the imaginary part of *c* is not zero or if
        the real part of *c* is not finite.

   .. cpp:function:: template <string_type T> explicit integer(const T &s, int base = 10)

      Constructor from string.

      This constructor will initialize ``this`` from *s*, which must represent
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

   .. cpp:function:: template <integer_cpp_arithmetic T> integer &operator=(const T &x)

      Generic assignment operator from arithmetic C++ types.

      This operator will assign *x* to ``this``. The storage type of ``this`` after the assignment
      will depend only on the value of *x* (that is, the storage type will be static if the value of *x*
      is small enough, dynamic otherwise). Assignment from floating-point types will assign the truncated
      counterpart of *x*.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception std\:\:domain_error: if *x* is a non-finite floating-point value.

   .. cpp:function:: template <integer_cpp_complex T> integer &operator=(const T &c)

      .. versionadded:: 0.19

      Generic assignment operator from complex C++ types.

      This operator will assign *c* to ``this``. The storage type of ``this`` after the assignment
      will depend only on the value of *c* (that is, the storage type will be static if the value of *c*
      is small enough, dynamic otherwise). The assignment will be successful only if
      the imaginary part of *c* is zero and the real part of *c* is finite.

      :param c: the assignment argument.

      :return: a reference to ``this``.

      :exception std\:\:domain_error: if the imaginary part of *c* is not zero or if
        the real part of *c* is not finite.

   .. cpp:function:: integer &operator=(const rational<SSize> &x)
   .. cpp:function:: integer &operator=(const real128 &x)
   .. cpp:function:: integer &operator=(const real &x)
   .. cpp:function:: integer &operator=(const complex128 &x)
   .. cpp:function:: integer &operator=(const complex &x)

      .. note::

         The :cpp:class:`~mppp::real128` and :cpp:class:`~mppp::complex128`
         overloads are available only if mp++ was configured with the ``MPPP_WITH_QUADMATH``
         option enabled. The :cpp:class:`~mppp::real` overload
         is available only if mp++ was configured with the ``MPPP_WITH_MPFR`` option enabled.
         The :cpp:class:`~mppp::complex` overload
         is available only if mp++ was configured with the ``MPPP_WITH_MPC`` option enabled.

      .. versionadded:: 0.20

      Assignment operators from other mp++ classes.

      These operators are formally equivalent to converting *x* to
      :cpp:class:`~mppp::integer` and then move-assigning the result
      to ``this``.

      :param x: the assignment argument.

      :return: a reference to ``this``.

      :exception unspecified: any exception raised by the conversion of *x*
        to :cpp:class:`~mppp::integer`.

   .. cpp:function:: template <string_type T> integer &operator=(const T &s)

      Assignment from string.

      The body of this operator is equivalent to:

      .. code-block:: c++

         return *this = integer{s};

      That is, a temporary integer is constructed from
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

   .. cpp:function:: template <integer_cpp_arithmetic T> explicit operator T() const

      Generic conversion operator to arithmetic C++ types.

      This operator will convert ``this`` to ``T``.
      Conversion to ``bool`` yields ``false`` if ``this`` is zero,
      ``true`` otherwise. Conversion to other integral types yields the exact result, if representable by the target
      type. Conversion to floating-point types might yield inexact values and
      infinities.

      :return: ``this`` converted to the target type.

      :exception std\:\:overflow_error: if the target type is an integral type and the value of
        ``this`` cannot be represented by it.

   .. cpp:function:: template <integer_cpp_complex T> explicit operator T() const

      .. versionadded:: 0.19

      Generic conversion operator to complex C++ types.

      This operator will convert ``this`` to ``T``.
      The conversion might yield inexact values and infinities.

      :return: ``this`` converted to the target type.

   .. cpp:function:: template <integer_cpp_arithmetic T> bool get(T &rop) const

      Generic conversion member function to arithmetic C++ types.

      This member function, similarly to the conversion operator, will convert ``this``
      to ``T``, storing the result of the conversion into *rop*. Differently
      from the conversion operator, this member function does not raise any exception: if the conversion is successful,
      the member function will return ``true``, otherwise the member function will return ``false``. If the conversion
      fails, *rop* will not be altered.

      :param rop: the variable which will store the result of the conversion.

      :return: ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail only if ``T`` is
        an integral C++ type which cannot represent the value of ``this``.

   .. cpp:function:: template <integer_cpp_complex T> bool get(T &rop) const

      .. versionadded:: 0.19

      Generic conversion member function to complex C++ types.

      This member function, similarly to the conversion operator, will convert ``this``
      to ``T``, storing the result of the conversion into *rop*.
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

   .. cpp:function:: integer &neg()

      Negate in-place.

      This member function will set ``this`` to ``-this``.

      :return: a reference to ``this``.

   .. cpp:function:: integer &abs()

      In-place absolute value.

      This member function will set ``this`` to its absolute value.

      :return: a reference to ``this``.

   .. cpp:function:: integer &nextprime()

      Compute next prime number in-place.

      This member function will set ``this`` to the first prime number
      greater than the current value.

      :return: a reference to ``this``.

   .. cpp:function:: int probab_prime_p(int reps = 25) const

     Test primality.

     This member function will run a series of probabilistic tests to determine if ``this`` is a prime number.
     It will return 2 if ``this`` is definitely a prime, 1 if ``this`` is probably a prime and 0 if ``this``
     is definitely not-prime.

     :param reps: the number of tests to run.

     :return: an integer indicating if ``this`` is a prime.

     :exception std\:\:invalid_argument: if *reps* is less than 1 or if ``this`` is negative.

   .. cpp:function:: integer &sqrt()

      Integer square root.

      This member function will set ``this`` to its integer square root.

      :return: a reference to ``this``.

      :exception std\:\:domain_error: if ``this`` is negative.

   .. cpp:function:: integer &sqr()

      Integer squaring.

      This member function will set ``this`` to its square.

      :return: a reference to ``this``.

   .. cpp:function:: bool odd_p() const
   .. cpp:function:: bool even_p() const

      Parity detection.

      :return: ``true`` if ``this`` is odd (resp. even), ``false`` otherwise.

   .. cpp:function:: std::remove_extent<mpz_t>::type *get_mpz_t()

      Get a pointer to the dynamic storage.

      This member function will first promote ``this`` to dynamic storage (if ``this`` is not already employing dynamic
      storage), and it will then return a pointer to the internal :cpp:type:`mpz_t`. The returned pointer can be used
      as an argument for the functions of the GMP API.

      .. note::

         The returned pointer is a raw, non-owning pointer tied to the lifetime of ``this``. Calling
         :cpp:func:`~mppp::integer::demote()` or
         assigning an :cpp:class:`~mppp::integer` with static storage to ``this`` will invalidate the returned
         pointer.

      :return: a pointer to the internal GMP integer.

   .. cpp:function:: bool is_zero() const
   .. cpp:function:: bool is_one() const
   .. cpp:function:: bool is_negative_one() const

      Detect special values.

      :return: ``true`` if ``this`` is :math:`0`, :math:`1` or :math:`-1` respectively,
        ``false`` otherwise.

   .. cpp:function:: std::size_t binary_size() const

      Size of the serialised binary representation.

      This member function will return a value representing the number of bytes necessary
      to serialise ``this`` into a memory buffer in binary format via one of the available
      :cpp:func:`~mppp::integer::binary_save()` overloads. The returned value
      is platform-dependent.

      :return: the number of bytes needed for the binary serialisation of ``this``.

      :exception std\:\:overflow_error: if the size in limbs of ``this`` is larger than an
        implementation-defined limit.

   .. cpp:function:: std::size_t binary_save(char *dest) const
   .. cpp:function:: std::size_t binary_save(std::vector<char> &dest) const
   .. cpp:function:: template <std::size_t S> std::size_t binary_save(std::array<char, S> &dest) const
   .. cpp:function:: std::size_t binary_save(std::ostream &dest) const

      Serialise into a memory buffer or an output stream.

      These member functions will write into *dest* a binary representation of ``this``. The serialised
      representation produced by these member functions can be read back with one of the
      :cpp:func:`~mppp::integer::binary_load()` overloads.

      For the first overload, *dest* must point to a memory area whose size is at least equal to the value returned
      by :cpp:func:`~mppp::integer::binary_size()`, otherwise the behaviour will be undefined.
      *dest* does not have any special alignment requirements.

      For the second overload, the size of *dest* must be at least equal to the value returned by
      :cpp:func:`~mppp::integer::binary_size()`. If that is not the case, *dest* will be resized
      to :cpp:func:`~mppp::integer::binary_size()`.

      For the third overload, the size of *dest* must be at least equal to the value returned by
      :cpp:func:`~mppp::integer::binary_size()`. If that is not the case, no data
      will be written to *dest* and zero will be returned.

      For the fourth overload, if the serialisation is successful (that is, no stream error state is ever detected
      in *dest* after write
      operations), then the binary size of ``this`` (that is, the number of bytes written into *dest*) will be
      returned. Otherwise, zero will be returned. Note that a return value of zero does not necessarily imply that no
      bytes were written into *dest*, just that an error occurred at some point during the serialisation process.

      .. warning::

         The binary representation produced by these member functions is compiler, platform and architecture
         specific, and it is subject to possible breaking changes in future versions of mp++. Thus,
         it should not be used as an exchange format or for long-term data storage.

      :param dest: the output buffer or stream.

      :return: the number of bytes written into ``dest`` (i.e., the output of :cpp:func:`~mppp::integer::binary_size()`,
        if the serialisation was successful).

      :exception std\:\:overflow_error: in case of (unlikely) overflow errors.
      :exception unspecified: any exception thrown by :cpp:func:`~mppp::integer::binary_size()`, by memory errors in
        standard containers, or by the public interface of ``std::ostream``.

   .. cpp:function:: std::size_t binary_load(const char *src)
   .. cpp:function:: std::size_t binary_load(const std::vector<char> &src)
   .. cpp:function:: template <std::size_t S> std::size_t binary_load(const std::array<char, S> &src)
   .. cpp:function:: std::size_t binary_load(std::istream &src)

      Deserialise from a memory buffer or an input stream.

      These member functions will load into ``this`` the content of the memory buffer or input stream
      *src*, which must contain the serialised representation of an :cpp:class:`~mppp::integer`
      produced by one of the :cpp:func:`~mppp::integer::binary_save()` overloads.

      For the first overload, *src* does not have any special alignment requirements.

      For the second and third overloads, the serialised representation of the
      :cpp:class:`~mppp::integer` must start at the beginning of *src*,
      but it can end before the end of *src*. Data
      past the end of the serialised representation of the :cpp:class:`~mppp::integer`
      will be ignored.

      For the fourth overload, the serialised representation of the :cpp:class:`~mppp::integer`
      must start at
      the current position of *src*, but *src* can contain other data before and after
      the serialised :cpp:class:`~mppp::integer` value. Data
      past the end of the serialised representation of the :cpp:class:`~mppp::integer`
      will be ignored. If a stream error state is detected at any point of the deserialisation
      process after a read operation, zero will be returned and ``this`` will not have been modified.
      Note that a return value of zero does not necessarily imply that no
      bytes were read from *src*, just that an error occurred at some point during the serialisation process.

      .. warning::

         Although these member functions perform a few consistency checks on the data in *src*,
         they cannot ensure complete safety against maliciously-crafted data. Users are
         advised to use these member functions only with trusted data.

      :param src: the source memory buffer or stream.

      :return: the number of bytes read from *src* (that is, the output of :cpp:func:`~mppp::integer::binary_size()`
        after the deserialisation into ``this`` has successfully completed).

      :exception std\:\:overflow_error: in case of (unlikely) overflow errors.
      :exception std\:\:invalid_argument: if invalid data is detected in *src*.
      :exception unspecified: any exception thrown by memory errors in standard containers,
        the public interface of ``std::istream``, or :cpp:func:`~mppp::integer::binary_size()`.

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

.. cpp:concept:: template <typename T> mppp::integer_cpp_arithmetic

   This concept is satisfied if ``T`` is an arithmetic C++ type compatible with :cpp:class:`~mppp::integer`.

   Specifically, this concept is equivalent to :cpp:concept:`~mppp::cpp_arithmetic` if mp++
   was built with the ``MPPP_WITH_MPFR`` option enabled (see the :ref:`installation instructions <installation>`).
   Otherwise, this concept is equivalent to :cpp:concept:`~mppp::cpp_arithmetic` minus the ``long double`` type.

.. cpp:concept:: template <typename T> mppp::integer_cpp_complex

   This concept is satisfied if ``T`` is a complex C++ type compatible with :cpp:class:`~mppp::integer`.

   Specifically, this concept is equivalent to :cpp:concept:`~mppp::cpp_complex` if mp++
   was built with the ``MPPP_WITH_MPFR`` option enabled (see the :ref:`installation instructions <installation>`).
   Otherwise, this concept is equivalent to :cpp:concept:`~mppp::cpp_complex` minus the
   ``std::complex<long double>`` type.

.. cpp:concept:: template <typename T, typename U> mppp::integer_op_types

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <integer_operators>` and :ref:`functions <integer_functions>`
   involving :cpp:class:`~mppp::integer`. Specifically, the concept will be ``true`` if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::integer` with the same static size ``SSize``, or
   * one type is an :cpp:class:`~mppp::integer` and the other is an :cpp:concept:`~mppp::integer_cpp_arithmetic`
     or an :cpp:concept:`~mppp::integer_cpp_complex` type.

   Note that the modulo, bit-shifting and bitwise logic operators have additional restrictions.

.. cpp:concept:: template <typename T, typename U> mppp::integer_real_op_types

   This concept will be ``true`` if:

   * ``T`` and ``U`` satisfy :cpp:concept:`~mppp::integer_op_types`, and
   * neither ``T`` nor ``U`` satisfy :cpp:concept:`~mppp::integer_cpp_complex`.

.. cpp:concept:: template <typename T, typename U> mppp::integer_integral_op_types

   This concept is satisfied if the types ``T`` and ``U`` are suitable for use in the
   generic binary :ref:`operators <integer_operators>` and :ref:`functions <integer_functions>`
   involving :cpp:class:`~mppp::integer` and integral C++ types. Specifically, the concept will be ``true``
   if either:

   * ``T`` and ``U`` are both :cpp:class:`~mppp::integer` with the same static size, or
   * one type is an :cpp:class:`~mppp::integer` and the other is a :cpp:concept:`~mppp::cpp_integral` type.

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
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::set_one(mppp::integer<SSize> &n)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::set_negative_one(mppp::integer<SSize> &n)

   Set to :math:`0`, :math:`1` or :math:`-1`.

   After calling these functions, the storage type of *n* will be static and its value will be
   :math:`0`, :math:`1` or :math:`-1`.

   .. note::

      These are specialised higher-performance alternatives to the assignment operators.

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

.. cpp:function:: template <mppp::integer_cpp_arithmetic T, std::size_t SSize> bool mppp::get(T &rop, const mppp::integer<SSize> &n)

   Generic conversion function from :cpp:class:`~mppp::integer` to arithmetic C++ types.

   This function will convert the input :cpp:class:`~mppp::integer` *n* to ``T``,
   storing the result of the conversion into *rop*.
   If the conversion is successful, the function
   will return ``true``, otherwise the function will return ``false``. If the conversion fails, *rop* will
   not be altered.

   :param rop: the variable which will store the result of the conversion.
   :param n: the input :cpp:class:`~mppp::integer`.

   :return: ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail only if ``T`` is
     an integral C++ type which cannot represent the value of *n*.

.. cpp:function:: template <mppp::integer_cpp_complex T, std::size_t SSize> bool mppp::get(T &rop, const mppp::integer<SSize> &n)

   .. versionadded:: 0.19

   Generic conversion function from :cpp:class:`~mppp::integer` to complex C++ types.

   This function will convert the input :cpp:class:`~mppp::integer` *n* to ``T``,
   storing the result of the conversion into *rop*.
   The conversion is always successful, and this function will always return ``true``.

   :param rop: the variable which will store the result of the conversion.
   :param n: the input :cpp:class:`~mppp::integer`.

   :return: ``true``.

.. _integer_arithmetic:

Arithmetic
~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::add(mppp::integer<SSize> &rop, const mppp::integer<SSize> &x, const mppp::integer<SSize> &y)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::sub(mppp::integer<SSize> &rop, const mppp::integer<SSize> &x, const mppp::integer<SSize> &y)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::mul(mppp::integer<SSize> &rop, const mppp::integer<SSize> &x, const mppp::integer<SSize> &y)

   Ternary arithmetic primitives.

   These functions will set *rop* to, respectively:

   * :math:`x + y`,
   * :math:`x - y`,
   * :math:`x \times y`.

   :param rop: the return value.
   :param x: the first operand.
   :param y: the second operand.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize, mppp::cpp_unsigned_integral T> mppp::integer<SSize> &mppp::add_ui(mppp::integer<SSize> &rop, const mppp::integer<SSize> &x, const T &y)
.. cpp:function:: template <std::size_t SSize, mppp::cpp_signed_integral T> mppp::integer<SSize> &mppp::add_si(mppp::integer<SSize> &rop, const mppp::integer<SSize> &x, const T &y)
.. cpp:function:: template <std::size_t SSize, mppp::cpp_unsigned_integral T> mppp::integer<SSize> &mppp::sub_ui(mppp::integer<SSize> &rop, const mppp::integer<SSize> &x, const T &y)
.. cpp:function:: template <std::size_t SSize, mppp::cpp_signed_integral T> mppp::integer<SSize> &mppp::sub_si(mppp::integer<SSize> &rop, const mppp::integer<SSize> &x, const T &y)

   Ternary addition/subtraction primitives with integral C++ types.

   These functions, which will set *rop* to :math:`x \pm y`, can be faster
   alternatives to the :cpp:class:`~mppp::integer` addition function
   if *y* fits in a single limb.

   :param rop: the return value.
   :param x: the first operand.
   :param y: the second operand.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::addmul(mppp::integer<SSize> &z, const mppp::integer<SSize> &x, const mppp::integer<SSize> &y)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::submul(mppp::integer<SSize> &z, const mppp::integer<SSize> &x, const mppp::integer<SSize> &y)

   Ternary fused multiply-add/sub.

   These functions will set *z* to :math:`z \pm x \times y`.

   :param z: the return value.
   :param x: the first argument.
   :param y: the second argument.

   :return: a reference to *z*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::mul_2exp(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n, mp_bitcnt_t s)

   Ternary left shift.

   This function will set *rop* to :math:`n\times 2^s`.

   :param rop: the return value.
   :param n: the multiplicand.
   :param s: the bit shift value.

   :return: a reference to *rop*.

   :exception std\:\:overflow_error: if *s* is larger than an implementation-defined limit.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::sqr(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n)

   .. versionadded:: 0.18

   Binary squaring.

   This function will set *rop* to the square of *n*.

   :param rop: the return value.
   :param n: the argument.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::sqr(const mppp::integer<SSize> &n)

   .. versionadded:: 0.18

   Unary squaring.

   This function will return the square of *n*.

   :param n: the argument.

   :return: the square of *n*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::sqrm(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n, const mppp::integer<SSize> &mod)

   .. versionadded:: 0.18

   Ternary modular squaring.

   This function will set *rop* to the square of *n* modulo *mod*.

   :param rop: the return value.
   :param n: the argument.
   :param mod: the modulus.

   :return: a reference to *rop*.

   :exception mppp\:\:zero_division_error: if *mod* is zero.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::sqrm(const mppp::integer<SSize> &n, const mppp::integer<SSize> &mod)

   .. versionadded:: 0.18

   Binary modular squaring.

   This function will return the square of *n* modulo *mod*.

   :param n: the argument.
   :param mod: the modulus.

   :return: the square of *n* modulo *mod*.
   :exception mppp\:\:zero_division_error: if *mod* is zero.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::neg(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::abs(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n)

   Binary negation and absolute value.

   These functions will set *rop* to, respectively, :math:`-n` and :math:`\left| n \right|`.

   :param rop: the return value.
   :param n: the input argument.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::neg(const mppp::integer<SSize> &n)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::abs(const mppp::integer<SSize> &n)

   Unary negation and absolute value.

   :param n: the input argument.

   :return: :math:`-n` and :math:`\left| n \right|` respectively.

.. _integer_division:

Division
~~~~~~~~

.. cpp:function:: template <std::size_t SSize> void mppp::tdiv_qr(mppp::integer<SSize> &q, mppp::integer<SSize> &r, const mppp::integer<SSize> &n, const mppp::integer<SSize> &d)

   Truncated division with remainder.

   This function will set *q* to the truncated quotient, :math:`n \operatorname{div} d`, and *r* to
   the remainder, :math:`n \bmod d`. The remainder *r* has the same sign as *n*. *q* and *r* must be
   distinct objects.

   :param q: the quotient.
   :param r: the remainder.
   :param n: the dividend.
   :param d: the divisor.

   :exception std\:\:invalid_argument: if *q* and *r* are the same object.
   :exception mppp\:\:zero_division_error: if *d* is zero.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::tdiv_q(mppp::integer<SSize> &q, const mppp::integer<SSize> &n, const mppp::integer<SSize> &d)

   Truncated division without remainder.

   This function will set *q* to the truncated quotient, :math:`n \operatorname{div} d`.

   :param q: the quotient.
   :param n: the dividend.
   :param d: the divisor.

   :return: a reference to *q*.

   :exception mppp\:\:zero_division_error: if *d* is zero.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::divexact(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n, const mppp::integer<SSize> &d)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::divexact_gcd(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n, const mppp::integer<SSize> &d)

   Ternary exact divisions.

   These functions will set *rop* to the quotient :math:`\frac{n}{d}`.

   .. warning::

      Both functions require *d* to divide *n* **exactly**. ``divexact_gcd()``
      additionally requires *d* to be positive.

   :param rop: the return value.
   :param n: the dividend.
   :param d: the divisor.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::divexact(const mppp::integer<SSize> &n, const mppp::integer<SSize> &d)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::divexact_gcd(const mppp::integer<SSize> &n, const mppp::integer<SSize> &d)

   Binary exact divisions.

   These functions will return the quotient :math:`\frac{n}{d}`.

   .. warning::

      Both functions require *d* to divide *n* **exactly**. ``divexact_gcd()``
      additionally requires *d* to be positive.

   :param n: the dividend.
   :param d: the divisor.

   :return: :math:`\frac{n}{d}`.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::tdiv_q_2exp(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n, mp_bitcnt_t s)

   Ternary right shift.

   This function will set *rop* to the truncated quotient :math:`n \operatorname{div} 2^s`.

   :param rop: the return value.
   :param n: the dividend.
   :param s: the bit shift value.

   :return: a reference to *rop*.

.. _integer_comparison:

Comparison
~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> int mppp::cmp(const mppp::integer<SSize> &x, const mppp::integer<SSize> &y)

   Three-way comparison.

   :param x: the first operand.
   :param y: the second operand.

   :return: 0 if :math:`x = y`, a negative value if :math:`x<y`, a positive value if
     :math:`x>y`.

.. cpp:function:: template <std::size_t SSize> int mppp::sgn(const mppp::integer<SSize> &n)

   Sign function.

   :param n: the input argument.

   :return: 0 if *n* is zero, 1 if *n* is positive, -1 if *n* is negative.

.. cpp:function:: template <std::size_t SSize> bool mppp::odd_p(const mppp::integer<SSize> &n)
.. cpp:function:: template <std::size_t SSize> bool mppp::even_p(const mppp::integer<SSize> &n)

   Parity detection.

   :param n: the input argument.

   :return: ``true`` if *n* is odd (resp. even), ``false`` otherwise.

.. cpp:function:: template <std::size_t SSize> bool mppp::is_zero(const mppp::integer<SSize> &n)
.. cpp:function:: template <std::size_t SSize> bool mppp::is_one(const mppp::integer<SSize> &n)
.. cpp:function:: template <std::size_t SSize> bool mppp::is_negative_one(const mppp::integer<SSize> &n)

   Detect special values.

   :param n: the input argument.

   :return: ``true`` if *n* is equal to :math:`0`, :math:`1` or :math:`-1` respectively,
     ``false`` otherwise.

.. _integer_logic:

Logic and bit fiddling
~~~~~~~~~~~~~~~~~~~~~~

.. versionadded:: 0.6

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::bitwise_not(mppp::integer<SSize> &rop, const mppp::integer<SSize> &op)

   Bitwise NOT.

   This function will set *rop* to the bitwise NOT (i.e., the one's complement) of *op*. Negative operands
   are treated as-if they were represented using two's complement.

   :param rop: the return value.
   :param op: the operand.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::bitwise_ior(mppp::integer<SSize> &rop, const mppp::integer<SSize> &x, const mppp::integer<SSize> &y)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::bitwise_and(mppp::integer<SSize> &rop, const mppp::integer<SSize> &x, const mppp::integer<SSize> &y)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::bitwise_xor(mppp::integer<SSize> &rop, const mppp::integer<SSize> &x, const mppp::integer<SSize> &y)

   Binary bitwise operations.

   These functions will set *rop* to, respectively the bitwise OR, AND and XOR of :math:`x` and :math:`y`.
   Negative operands are treated as-if they were represented using two's complement.

   :param rop: the return value.
   :param x: the first operand.
   :param y: the second operand.

   :return: a reference to *rop*.

.. _integer_ntheory:

Number theoretic functions
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::gcd(mppp::integer<SSize> &rop, const mppp::integer<SSize> &op1, const mppp::integer<SSize> &op2)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::lcm(mppp::integer<SSize> &rop, const mppp::integer<SSize> &op1, const mppp::integer<SSize> &op2)

   Ternary GCD and LCM.

   These functions will set *rop* to, respectively, the GCD and LCM of *op1* and *op2*. The result is always nonnegative.
   If both operands are zero, *rop* is set to zero.

   .. versionadded:: 0.21

      The ``lcm()`` function.

   :param rop: the return value.
   :param op1: the first operand.
   :param op2: the second operand.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::gcd(const mppp::integer<SSize> &op1, const mppp::integer<SSize> &op2)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::lcm(const mppp::integer<SSize> &op1, const mppp::integer<SSize> &op2)

   Binary GCD and LCM.

   These functions will return, respectively, the GCD and LCM of *op1* and *op2*. The result is always nonnegative.
   If both operands are zero, zero is returned.

   .. versionadded:: 0.21

      The ``lcm()`` function.

   :param op1: the first operand.
   :param op2: the second operand.

   :return: the GCD or LCM of *op1* and *op2*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::fac_ui(mppp::integer<SSize> &rop, unsigned long n)

   Factorial.

   This function will set *rop* to :math:`n!`.

   :param rop: the return value.
   :param n: the operand.

   :return: a reference to *rop*.

   :exception std\:\:invalid_argument: if *n* is larger than an implementation-defined limit.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::bin_ui(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n, unsigned long k)

   Ternary binomial coefficient.

   This function will set *rop* to :math:`{n \choose k}`. Negative values of *n* are
   supported.

   :param rop: the return value.
   :param n: the top argument.
   :param k: the bottom argument.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::bin_ui(const mppp::integer<SSize> &n, unsigned long k)

   Binary binomial coefficient.

   :param n: the top argument.
   :param k: the bottom argument.

   :return: :math:`{n \choose k}`.

.. cpp:function:: template <typename T, mppp::integer_integral_op_types<T> U> auto mppp::binomial(const T &n, const U &k)

   Generic binomial coefficient.

   This function will compute the binomial coefficient :math:`{{n}\choose{k}}`, supporting integral input values.
   The implementation can handle positive and negative values for both the top and the bottom argument.

   The return type is always :cpp:class:`~mppp::integer`.

   .. seealso::

      https://arxiv.org/abs/1105.3689/

   :param n: the top argument.
   :param k: the bottom argument.

   :return: :math:`{n \choose k}`.

   :exception std\:\:overflow_error: if *k* is outside an implementation-defined range.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::nextprime(mppp::integer<SSize> &rop, const mppp::integer<SSize> &n)

   Compute next prime number (binary version).

   This function will set *rop* to the first prime number greater than *n*.
   Note that for negative values of *n* this function always sets *rop* to :math:`2`.

   :param rop: the return value.
   :param n: the input argument.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::nextprime(const mppp::integer<SSize> &n)

   Compute next prime number (unary version).

   For negative values of *n* this function always returns :math:`2`.

   :param n: the input argument.

   :return: the first prime number greater than *n*.

.. cpp:function:: template <std::size_t SSize> int mppp::probab_prime_p(const mppp::integer<SSize> &n, int reps = 25)

   Primality test.

   This is the free-function version of :cpp:func:`mppp::integer::probab_prime_p()`.

   :param n: the integer whose primality will be tested.
   :param reps: the number of tests to run.

   :return: an integer indicating if *n* is a prime.

   :exception unspecified: any exception thrown by :cpp:func:`mppp::integer::probab_prime_p()`.

.. _integer_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::pow_ui(mppp::integer<SSize> &rop, const mppp::integer<SSize> &base, unsigned long exp)

   Ternary integral exponentiation.

   This function will set *rop* to ``base**exp``.

   :param rop: the return value.
   :param base: the base.
   :param exp: the exponent.

   :return: a reference to *rop*.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::pow_ui(const mppp::integer<SSize> &base, unsigned long exp)

   Binary integral exponentiation.

   :param base: the base.
   :param exp: the exponent.

   :return: ``base**exp``.

.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> auto mppp::pow(const T &base, const U &exp)

   Generic binary exponentiation.

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

   Binary square root.

   This function will set *rop* to the truncated integer part of the square root of *n*.

   :param rop: the return value.
   :param n: the argument.

   :return: a reference to *rop*.

   :exception std\:\:domain_error: if *n* is negative.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::sqrt(const mppp::integer<SSize> &n)

   Unary square root.

   This function will return the truncated integer part of the square root of *n*.

   :param n: the argument.

   :return: the integer square root of *n*.

   :exception std\:\:domain_error: if *n* is negative.

.. cpp:function:: template <std::size_t SSize> void mppp::sqrtrem(mppp::integer<SSize> &rop, mppp::integer<SSize> &rem, const mppp::integer<SSize> &n)

   .. versionadded:: 0.12

   Square root with remainder.

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

.. cpp:function:: template <std::size_t SSize> std::size_t mppp::binary_size(const mppp::integer<SSize> &n)

   Binary size.

   This function is the free function equivalent of the
   :cpp:func:`mppp::integer::binary_size()` member function.

   :param n: the input argument.

   :return: the output of :cpp:func:`mppp::integer::binary_size()` called on *n*.

   :exception unspecified: any exception thrown by :cpp:func:`mppp::integer::binary_size()`.

.. cpp:function:: template <std::size_t SSize, typename T> std::size_t mppp::binary_save(const mppp::integer<SSize> &n, T &&dest)

   Binary serialisation.

   .. note::

      This function participates in overload resolution only if the expression

      .. code-block:: c++

         return n.binary_save(std::forward<T>(dest));

      is well-formed.

   This function is the free function equivalent of the
   :cpp:func:`mppp::integer::binary_save()` overloads.

   :param n: the input argument.
   :param dest: the object into which *n* will be serialised.

   :return: the output of the invoked :cpp:func:`mppp::integer::binary_save()`
     overload called on *n* with *dest* as argument.

   :exception unspecified: any exception thrown by the invoked :cpp:func:`mppp::integer::binary_save()` overload.

.. cpp:function:: template <std::size_t SSize, typename T> std::size_t mppp::binary_load(mppp::integer<SSize> &n, T &&src)

   Binary deserialisation.

   .. note::

      This function participates in overload resolution only if the expression

      .. code-block:: c++

         return n.binary_load(std::forward<T>(src));

      is well-formed.

   This function is the free function equivalent of the
   :cpp:func:`mppp::integer::binary_load()` overloads.

   :param n: the output argument.
   :param src: the object containing the serialised :cpp:class:`~mppp::integer`
     that will be loaded into *n*.

   :return: the output of the invoked :cpp:func:`mppp::integer::binary_load()`
     overload called on *n* with *src* as argument.

   :exception unspecified: any exception thrown by the invoked :cpp:func:`mppp::integer::binary_load()` overload.

.. _integer_other:

Other
~~~~~

.. cpp:function:: template <std::size_t SSize> std::size_t mppp::hash(const mppp::integer<SSize> &n)

   Hash value.

   This function will return a hash value for *n*. The hash value depends only on the value of *n*
   (and *not* on its storage type).

   A :ref:`specialisation <integer_std_specialisations>` of the standard ``std::hash`` functor is also provided, so that
   it is possible to use :cpp:class:`~mppp::integer` in standard unordered associative containers out of the box.

   :param n: the input value.

   :return: a hash value for *n*.

.. cpp:function:: void mppp::free_integer_caches()

   Free the :cpp:class:`~mppp::integer` caches.

   On some platforms, :cpp:class:`~mppp::integer` manages thread-local caches
   to speed-up the allocation/deallocation of small objects. These caches are automatically
   freed on program shutdown or when a thread exits. In certain situations, however,
   it may be desirable to manually free the memory in use by the caches before
   the program's end or a thread's exit. This function does exactly that.

   On platforms where thread local storage is not supported, this funcion will be a no-op.

   It is safe to call this function concurrently from different threads.

.. _integer_operators:

Mathematical operators
----------------------

Overloaded operators are provided for convenience.
Their interface is generic, and their implementation
is typically built on top of basic :ref:`functions <integer_functions>`.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::operator+(const mppp::integer<SSize> &n)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::operator-(const mppp::integer<SSize> &n)

   Identity and negation operators.

   :param n: the input argument.

   :return: :math:`n` and :math:`-n` respectively.

.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> auto mppp::operator+(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> auto mppp::operator-(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> auto mppp::operator*(const T &x, const U &y)
.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> auto mppp::operator/(const T &x, const U &y)

   Binary arithmetic operators.

   These operators will return, respectively:

   * :math:`x+y`,
   * :math:`x-y`,
   * :math:`x \times y`,
   * :math:`x \operatorname{div} y`.

   The return type is determined as follows:

   * if the non-:cpp:class:`~mppp::integer` argument is a floating-point or complex value, then the
     type of the result is floating-point or complex; otherwise,
   * the type of the result is :cpp:class:`~mppp::integer`.

   :param x: the first operand.
   :param y: the second operand.

   :return: the result of the arithmetic operation.

   :exception mppp\:\:zero_division_error: if, in a division, *y* is zero and only integral
     types are involved.

.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> T &mppp::operator+=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> T &mppp::operator-=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> T &mppp::operator*=(T &x, const U &y)
.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> T &mppp::operator/=(T &x, const U &y)

   In-place arithmetic operators.

   These operators will set *x* to, respectively:

   * :math:`x+y`,
   * :math:`x-y`,
   * :math:`x \times y`,
   * :math:`x \operatorname{div} y`.

   :param x: the first operand.
   :param y: the second operand.

   :return: a reference to *x*.

   :exception mppp\:\:zero_division_error: if, in a division, *y* is zero and only integral
     types are involved.
   :exception unspecified: any exception thrown by the assignment/conversion operators
     of :cpp:class:`~mppp::integer`.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::operator++(mppp::integer<SSize> &n)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> &mppp::operator--(mppp::integer<SSize> &n)

   Prefix increment/decrement.

   :param n: the input argument.

   :return: a reference to *n* after the increment/decrement.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::operator++(mppp::integer<SSize> &n, int)
.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::operator--(mppp::integer<SSize> &n, int)

   Suffix increment/decrement.

   :param n: the input argument.

   :return: a copy of *n* before the increment/decrement.

.. cpp:function:: template <typename T, mppp::integer_integral_op_types<T> U> auto mppp::operator%(const T &n, const U &d)

   Binary modulo operator.

   The return type is always :cpp:class:`~mppp::integer`.

   :param n: the dividend.
   :param d: the divisor.

   :return: :math:`n \bmod d`.

   :exception mppp\:\:zero_division_error: if *d* is zero.

.. cpp:function:: template <typename T, mppp::integer_integral_op_types<T> U> T &mppp::operator%=(T &rop, const U &op)

   In-place modulo operator.

   :param rop: the dividend.
   :param op: the divisor.

   :return: a reference to *rop*.

   :exception mppp\:\:zero_division_error: if *op* is zero.
   :exception unspecified: any exception thrown by the conversion operator of :cpp:class:`~mppp::integer`.

.. cpp:function:: template <mppp::cpp_integral T, std::size_t SSize> mppp::integer<SSize> mppp::operator<<(const mppp::integer<SSize> &n, T s)
.. cpp:function:: template <mppp::cpp_integral T, std::size_t SSize> mppp::integer<SSize> mppp::operator>>(const mppp::integer<SSize> &n, T s)

   Binary left/right shift operators.

   :param n: the multiplicand/dividend.
   :param s: the bit shift value.

   :return: :math:`n \times 2^s` and :math:`n \operatorname{div} 2^s` respectively.

   :exception std\:\:overflow_error: if *s* is negative or larger than an implementation-defined value.

.. cpp:function:: template <mppp::cpp_integral T, std::size_t SSize> mppp::integer<SSize> &mppp::operator<<=(mppp::integer<SSize> &rop, T s)
.. cpp:function:: template <mppp::cpp_integral T, std::size_t SSize> mppp::integer<SSize> &mppp::operator>>=(mppp::integer<SSize> &rop, T s)

   In-place left/right shift operators.

   :param rop: the multiplicand/dividend.
   :param s: the bit shift value.

   :return: a reference to *rop*.

   :exception std\:\:overflow_error: if *s* is negative or larger than an implementation-defined value.

.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> bool mppp::operator==(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> bool mppp::operator!=(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> bool mppp::operator<(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> bool mppp::operator<=(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> bool mppp::operator>(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::integer_op_types<T> U> bool mppp::operator>=(const T &op1, const U &op2)

   Binary comparison operators.

   :param op1: first argument.
   :param op2: second argument.

   :return: the result of the comparison.

.. cpp:function:: template <std::size_t SSize> mppp::integer<SSize> mppp::operator~(const mppp::integer<SSize> &op)

   Unary bitwise NOT.

   This operator returns the bitwise NOT (i.e., the one's complement) of *op*. Negative operands
   are treated as-if they were represented using two's complement.

   :param op: the operand.

   :return: the bitwise NOT of *op*.

.. cpp:function:: template <typename T, mppp::integer_integral_op_types<T> U> auto mppp::operator|(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::integer_integral_op_types<T> U> auto mppp::operator&(const T &op1, const U &op2)
.. cpp:function:: template <typename T, mppp::integer_integral_op_types<T> U> auto mppp::operator^(const T &op1, const U &op2)

   Binary bitwise operators.

   These operators will return, respectively:

   * the bitwise OR,
   * the bitwise AND,
   * the bitwise XOR,

   of *op1* and *op2*.

   Negative operands are treated as-if they were represented using two's complement.

   The return type is always :cpp:class:`~mppp::integer`.

   :param op1: the first operand.
   :param op2: the second operand.

   :return: the result of the bitwise operation.

.. cpp:function:: template <typename T, mppp::integer_integral_op_types<T> U> T &mppp::operator|=(T &rop, const U &op)
.. cpp:function:: template <typename T, mppp::integer_integral_op_types<T> U> T &mppp::operator&=(T &rop, const U &op)
.. cpp:function:: template <typename T, mppp::integer_integral_op_types<T> U> T &mppp::operator^=(T &rop, const U &op)

   In-place bitwise operators.

   These operators will set *rop* to, respectively:

   * the bitwise OR,
   * the bitwise AND,
   * the bitwise XOR,

   of *rop* and *op*.

   Negative operands are treated as-if they were represented using two's complement.

   :param rop: the first operand.
   :param op: the second operand.

   :return: a reference to *rop*.

   :exception unspecified: any exception thrown by the conversion operator of :cpp:class:`~mppp::integer`.

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
