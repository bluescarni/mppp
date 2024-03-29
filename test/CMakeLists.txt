# We have some multithreaded tests, so let's activate threading support.
include(YACMAThreadingSetup)
include(YACMACompilerLinkerSettings)

list(APPEND MPPP_CXX_FLAGS_DEBUG ${YACMA_THREADING_CXX_FLAGS})
list(APPEND MPPP_CXX_FLAGS_RELEASE ${YACMA_THREADING_CXX_FLAGS})

add_library(mp++_test STATIC catch_main.cpp)
target_compile_options(mp++_test PRIVATE
  "$<$<CONFIG:Debug>:${MPPP_CXX_FLAGS_DEBUG}>"
  "$<$<CONFIG:Release>:${MPPP_CXX_FLAGS_RELEASE}>"
  "$<$<CONFIG:RelWithDebInfo>:${MPPP_CXX_FLAGS_RELEASE}>"
  "$<$<CONFIG:MinSizeRel>:${MPPP_CXX_FLAGS_RELEASE}>"
)

target_compile_features(mp++_test PRIVATE cxx_std_11)
set_property(TARGET mp++_test PROPERTY CXX_EXTENSIONS NO)

# Enable the "-Wno-psabi" flag on GCC to silence ABI-related
# warnings in the tests when using std::complex.
if(YACMA_COMPILER_IS_GNUCXX)
  message(STATUS "Activating the '-Wno-psabi' flag for the unit tests.")
  # NOTE: because we are within an add_subdirectory()
  # command, altering these variables has no influence
  # in the parent scope.
  list(APPEND MPPP_CXX_FLAGS_DEBUG "-Wno-psabi")
  list(APPEND MPPP_CXX_FLAGS_RELEASE "-Wno-psabi")
endif()

function(ADD_MPPP_TESTCASE arg1)
  if(MPPP_TEST_NSPLIT)
    # Increase by one the _MPPP_TEST_NUM variable in the parent scope.
    # NOTE: we do it here (rather than at the end of the function) because we may exit the
    # function earlier in the return() below.
    # NOTE: even though we updated the _MPPP_TEST_NUM variable in the parent scope,
    # the _MPPP_TEST_NUM in the *current* scope still contains the old value.
    math(EXPR __MPPP_TEST_NUM "(${_MPPP_TEST_NUM} + 1) % ${MPPP_TEST_NSPLIT}")
    set(_MPPP_TEST_NUM ${__MPPP_TEST_NUM} PARENT_SCOPE)
  endif()
  if(MPPP_TEST_NSPLIT AND NOT "${MPPP_TEST_SPLIT_NUM}" STREQUAL "${_MPPP_TEST_NUM}")
    return()
  endif()
  add_executable(${arg1} ${arg1}.cpp)
  target_link_libraries(${arg1} PRIVATE mp++ mp++_test Threads::Threads)
  if(MPPP_WITH_QUADMATH)
    # NOTE: in some real128 tests we access
    # functions from the quadmath API.
    target_link_libraries(${arg1} PRIVATE mp++::quadmath)
  endif()
  target_compile_options(${arg1} PRIVATE
    "$<$<CONFIG:Debug>:${MPPP_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${MPPP_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${MPPP_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${MPPP_CXX_FLAGS_RELEASE}>"
  )
  target_compile_features(${arg1} PRIVATE cxx_std_11)
  set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)
  add_test(${arg1} ${arg1})
endfunction()

ADD_MPPP_TESTCASE(concepts)
ADD_MPPP_TESTCASE(global_header)
# NOTE: the interop test requires all optional
# deps to be enabled.
if(MPPP_WITH_QUADMATH AND MPPP_WITH_MPFR AND MPPP_WITH_MPC)
  ADD_MPPP_TESTCASE(interop_test)
endif()

ADD_MPPP_TESTCASE(integer_abs)
ADD_MPPP_TESTCASE(integer_addsub_ui_si)
ADD_MPPP_TESTCASE(integer_arith)
ADD_MPPP_TESTCASE(integer_arith_ops_01)
ADD_MPPP_TESTCASE(integer_arith_ops_02)
ADD_MPPP_TESTCASE(integer_arith_ops_03)
ADD_MPPP_TESTCASE(integer_basic_01)
ADD_MPPP_TESTCASE(integer_basic_02)
ADD_MPPP_TESTCASE(integer_basic_03)
ADD_MPPP_TESTCASE(integer_basic_04)
ADD_MPPP_TESTCASE(integer_bin)
ADD_MPPP_TESTCASE(integer_bitwise)
ADD_MPPP_TESTCASE(integer_caches)
ADD_MPPP_TESTCASE(integer_divexact)
ADD_MPPP_TESTCASE(integer_divexact_gcd)
ADD_MPPP_TESTCASE(integer_even_odd)
ADD_MPPP_TESTCASE(integer_fac)
ADD_MPPP_TESTCASE(integer_gcd_lcm)
ADD_MPPP_TESTCASE(integer_get_mpz_t)
ADD_MPPP_TESTCASE(integer_hash)
ADD_MPPP_TESTCASE(integer_is_zero_one)
ADD_MPPP_TESTCASE(integer_limb_size_nbits)
ADD_MPPP_TESTCASE(integer_literals)
ADD_MPPP_TESTCASE(integer_neg)
ADD_MPPP_TESTCASE(integer_nextprime)
ADD_MPPP_TESTCASE(integer_pow)
ADD_MPPP_TESTCASE(integer_probab_prime_p)
ADD_MPPP_TESTCASE(integer_rel)
ADD_MPPP_TESTCASE(integer_roots)
ADD_MPPP_TESTCASE(integer_set_zero_one)
ADD_MPPP_TESTCASE(integer_sqr)
ADD_MPPP_TESTCASE(integer_sqrm)
ADD_MPPP_TESTCASE(integer_stream_format)
ADD_MPPP_TESTCASE(integer_swap)
ADD_MPPP_TESTCASE(integer_tdiv_q)
ADD_MPPP_TESTCASE(integer_view)

ADD_MPPP_TESTCASE(rational_abs)
ADD_MPPP_TESTCASE(rational_arith)
ADD_MPPP_TESTCASE(rational_arith_ops_01)
ADD_MPPP_TESTCASE(rational_arith_ops_02)
ADD_MPPP_TESTCASE(rational_arith_ops_03)
ADD_MPPP_TESTCASE(rational_basic_01)
ADD_MPPP_TESTCASE(rational_basic_02)
ADD_MPPP_TESTCASE(rational_binomial)
ADD_MPPP_TESTCASE(rational_hash)
ADD_MPPP_TESTCASE(rational_inv)
ADD_MPPP_TESTCASE(rational_is_zero_one)
ADD_MPPP_TESTCASE(rational_neg)
ADD_MPPP_TESTCASE(rational_pow)
ADD_MPPP_TESTCASE(rational_rel)
ADD_MPPP_TESTCASE(rational_stream_format)
ADD_MPPP_TESTCASE(rational_literals)

if(MPPP_WITH_QUADMATH)
  ADD_MPPP_TESTCASE(real128_arith)
  ADD_MPPP_TESTCASE(real128_basic)
  ADD_MPPP_TESTCASE(real128_comparisons)
  ADD_MPPP_TESTCASE(real128_constants)
  ADD_MPPP_TESTCASE(real128_fpmanip)
  ADD_MPPP_TESTCASE(real128_hash)
  ADD_MPPP_TESTCASE(real128_ieee)
  ADD_MPPP_TESTCASE(real128_io)
  ADD_MPPP_TESTCASE(real128_logexp)
  ADD_MPPP_TESTCASE(real128_naninffinite)
  ADD_MPPP_TESTCASE(real128_operators)
  ADD_MPPP_TESTCASE(real128_pow)
  ADD_MPPP_TESTCASE(real128_roots)
  ADD_MPPP_TESTCASE(real128_signbit)
  ADD_MPPP_TESTCASE(real128_trig)
  ADD_MPPP_TESTCASE(real128_hyperbolic)
  ADD_MPPP_TESTCASE(real128_miscfunctions)
  ADD_MPPP_TESTCASE(real128_literal)
  ADD_MPPP_TESTCASE(real128_intrem)
  ADD_MPPP_TESTCASE(real128_fdim)
  ADD_MPPP_TESTCASE(real128_fmax_fmin)
  ADD_MPPP_TESTCASE(real128_bessel)

  ADD_MPPP_TESTCASE(complex128_basic)
  ADD_MPPP_TESTCASE(complex128_arith)
  ADD_MPPP_TESTCASE(complex128_roots)
  ADD_MPPP_TESTCASE(complex128_trig)
  ADD_MPPP_TESTCASE(complex128_hyper)
  ADD_MPPP_TESTCASE(complex128_logexp)
  ADD_MPPP_TESTCASE(complex128_pow)
  ADD_MPPP_TESTCASE(complex128_operators)
  ADD_MPPP_TESTCASE(complex128_literal)
  ADD_MPPP_TESTCASE(complex128_io)
endif()

if(MPPP_WITH_MPFR)
  ADD_MPPP_TESTCASE(real_arith)
  ADD_MPPP_TESTCASE(real_bessel)
  ADD_MPPP_TESTCASE(real_basic)
  ADD_MPPP_TESTCASE(real_cmp)
  ADD_MPPP_TESTCASE(real_constants)
  ADD_MPPP_TESTCASE(real_gamma)
  ADD_MPPP_TESTCASE(real_hyper)
  ADD_MPPP_TESTCASE(real_io)
  ADD_MPPP_TESTCASE(real_logexp)
  ADD_MPPP_TESTCASE(real_neg_abs)
  ADD_MPPP_TESTCASE(real_operators)
  ADD_MPPP_TESTCASE(real_pow)
  ADD_MPPP_TESTCASE(real_roots)
  ADD_MPPP_TESTCASE(real_get_set_z_2exp)
  ADD_MPPP_TESTCASE(real_trig)
  ADD_MPPP_TESTCASE(real_intrem)
  ADD_MPPP_TESTCASE(real_other_specfunc)
  ADD_MPPP_TESTCASE(real_literals)
  ADD_MPPP_TESTCASE(real_mul_div_2)
  ADD_MPPP_TESTCASE(real_polylogs)
  ADD_MPPP_TESTCASE(real_s11n)
  ADD_MPPP_TESTCASE(real_hash)
  ADD_MPPP_TESTCASE(real_nextafter)
endif()

if(MPPP_WITH_MPC)
  ADD_MPPP_TESTCASE(complex_basic)
  ADD_MPPP_TESTCASE(complex_arith)
  ADD_MPPP_TESTCASE(complex_operators)
  ADD_MPPP_TESTCASE(complex_cmp)
  ADD_MPPP_TESTCASE(complex_mul_div_2)
  ADD_MPPP_TESTCASE(complex_roots)
  ADD_MPPP_TESTCASE(complex_pow)
  ADD_MPPP_TESTCASE(complex_logexp)
  ADD_MPPP_TESTCASE(complex_rootofunity)
  ADD_MPPP_TESTCASE(complex_trig)
  ADD_MPPP_TESTCASE(complex_hyper)
  ADD_MPPP_TESTCASE(complex_literals)
  ADD_MPPP_TESTCASE(complex_io)

  if(MPPP_WITH_ARB)
    ADD_MPPP_TESTCASE(complex_agm)
  endif()
endif()

if(MPPP_TEST_PYBIND11)
  add_subdirectory(pybind11)
endif()

ADD_MPPP_TESTCASE(type_name)
ADD_MPPP_TESTCASE(utils)
ADD_MPPP_TESTCASE(type_traits)
