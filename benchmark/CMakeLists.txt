if(MPPP_BENCHMARK_BOOST)
  find_package(Boost 1.55.0 REQUIRED)
  if(NOT TARGET Boost::boost)
    # Depending on the CMake version, Boost::boost might not have been created.
    message(STATUS "The 'Boost::boost' target is missing, creating it.")
    add_library(Boost::boost INTERFACE IMPORTED)
    set_target_properties(Boost::boost PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
  endif()
endif()

if(MPPP_BENCHMARK_FLINT)
  find_package(FLINT REQUIRED)
endif()

# Custom target to run all benchmarks.
add_custom_target(benchmark)

# Helper to add a benchmark.
function(ADD_MPPP_BENCHMARK arg1)
  add_executable(${arg1} ${arg1}.cpp)
  target_link_libraries(${arg1} mp++)
  if(MPPP_BENCHMARK_BOOST)
    target_link_libraries(${arg1} Boost::boost)
    target_compile_options(${arg1} PRIVATE "-DMPPP_BENCHMARK_BOOST")
  endif()
  if(MPPP_BENCHMARK_FLINT)
    target_link_libraries(${arg1} FLINT::FLINT)
    target_compile_options(${arg1} PRIVATE "-DMPPP_BENCHMARK_FLINT")
  endif()
  target_include_directories(${arg1} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
  target_compile_options(${arg1} PRIVATE
    "$<$<CONFIG:Debug>:${MPPP_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${MPPP_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${MPPP_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${MPPP_CXX_FLAGS_RELEASE}>"
  )
  # Let's setup the target C++ standard, but only if the user did not provide it manually.
  if(NOT CMAKE_CXX_STANDARD)
    if(MPPP_COMPILER_SUPPORTS_CONCEPTS)
      # NOTE: the idea here is that we want to use C++14 if the compilers supports
      # concepts (as variable concepts are essentially template variables).
      set_property(TARGET ${arg1} PROPERTY CXX_STANDARD 14)
    else()
      set_property(TARGET ${arg1} PROPERTY CXX_STANDARD 11)
    endif()
  endif()
  set_property(TARGET ${arg1} PROPERTY CXX_STANDARD_REQUIRED YES)
  set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)
  # Visibility settings.
  set_target_properties(${arg1} PROPERTIES CXX_VISIBILITY_PRESET hidden)
  set_target_properties(${arg1} PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
  # Add the current benchmark to the "benchmark" custom target.
  add_dependencies(benchmark ${arg1})
  add_custom_command(TARGET benchmark POST_BUILD COMMAND "${arg1}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()

ADD_MPPP_BENCHMARK(integer1_dot_product_unsigned)
ADD_MPPP_BENCHMARK(integer2_dot_product_unsigned)
ADD_MPPP_BENCHMARK(integer1_dot_product_signed)
ADD_MPPP_BENCHMARK(integer2_dot_product_signed)
ADD_MPPP_BENCHMARK(integer1_vec_lshift_unsigned)
ADD_MPPP_BENCHMARK(integer2_vec_lshift_unsigned)
ADD_MPPP_BENCHMARK(integer1_vec_lshift_signed)
ADD_MPPP_BENCHMARK(integer2_vec_lshift_signed)
ADD_MPPP_BENCHMARK(integer1_vec_mul_unsigned)
ADD_MPPP_BENCHMARK(integer2_vec_mul_unsigned)
ADD_MPPP_BENCHMARK(integer1_vec_mul_signed)
ADD_MPPP_BENCHMARK(integer2_vec_mul_signed)
ADD_MPPP_BENCHMARK(integer1_vec_div_unsigned)
ADD_MPPP_BENCHMARK(integer2_vec_div_unsigned)
ADD_MPPP_BENCHMARK(integer1_vec_div_signed)
ADD_MPPP_BENCHMARK(integer2_vec_div_signed)
ADD_MPPP_BENCHMARK(integer1_vec_gcd_signed)
ADD_MPPP_BENCHMARK(integer1_sort_signed)
ADD_MPPP_BENCHMARK(integer2_sort_signed)
ADD_MPPP_BENCHMARK(integer1_sort_unsigned)
ADD_MPPP_BENCHMARK(integer2_sort_unsigned)
ADD_MPPP_BENCHMARK(integer1_int_conversion)
ADD_MPPP_BENCHMARK(integer2_int_conversion)
ADD_MPPP_BENCHMARK(integer1_uint_conversion)
ADD_MPPP_BENCHMARK(integer2_uint_conversion)
