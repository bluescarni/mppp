Utility functions
=================

.. cpp:function:: template <typename T> std::string mppp::demangle()

   This function will return a string representation
   for the name of the input type ``T``.
   
   If supported by the
   current platform/compiler combination, the returned string
   will be a human-readable demangled identifier. Otherwise,
   a string based on ``typeid(T).name()`` will be returned.

   Example:

   .. code-block:: c++

      std::cout << demangle<const std::string *>() << '\n'; // Prints "std::string const*" on Linux.

   :return: a string representation for the type ``T``.

   :exception unspecified: any exception raised by memory allocation failures.
