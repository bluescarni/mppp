# NOTE: Boost is always required by the benchmarking code.
find_package(Boost 1.55.0 REQUIRED filesystem)
if(NOT TARGET Boost::boost)
  # Depending on the CMake version, the Boost targets might not have been created.
  message(STATUS "The 'Boost::boost' target is missing, creating it.")
  add_library(Boost::boost INTERFACE IMPORTED)
  set_target_properties(Boost::boost PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")

  message(STATUS "The 'Boost::filesystem' target is missing, creating it.")
  add_library(Boost::filesystem UNKNOWN IMPORTED)
  set_target_properties(Boost::filesystem PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
  set_target_properties(Boost::filesystem PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    IMPORTED_LOCATION "${Boost_FILESYSTEM_LIBRARY}")
endif()

# fmt is always required.
# NOTE: need at least version 6.2
# to print 128-bit integers.
find_package(fmt 6.2 REQUIRED)

if(MPPP_BENCHMARK_FLINT)
  find_package(mp++_FLINT REQUIRED)
endif()

# The benchmarking helper library.
add_library(mp++_benchmark STATIC utils.cpp)
target_compile_options(mp++_benchmark PRIVATE
  "$<$<CONFIG:Debug>:${MPPP_CXX_FLAGS_DEBUG}>"
  "$<$<CONFIG:Release>:${MPPP_CXX_FLAGS_RELEASE}>"
  "$<$<CONFIG:RelWithDebInfo>:${MPPP_CXX_FLAGS_RELEASE}>"
  "$<$<CONFIG:MinSizeRel>:${MPPP_CXX_FLAGS_RELEASE}>"
)
target_link_libraries(mp++_benchmark PRIVATE Boost::boost Boost::filesystem)
target_compile_features(mp++_benchmark PRIVATE cxx_std_11)
set_property(TARGET mp++_benchmark PROPERTY CXX_EXTENSIONS NO)

# Custom target to run all benchmarks.
add_custom_target(benchmark)

# Helper to add a benchmark.
function(ADD_MPPP_BENCHMARK arg1)
  add_executable(${arg1} ${arg1}.cpp)
  target_link_libraries(${arg1} PRIVATE mp++_benchmark mp++ fmt::fmt)
  if(MPPP_BENCHMARK_BOOST)
    # NOTE: when benchmarking the Boost classes,
    # we make direct use of functions from the GMP API.
    target_link_libraries(${arg1} PRIVATE Boost::boost mp++::GMP)
    target_compile_options(${arg1} PRIVATE "-DMPPP_BENCHMARK_BOOST")
  endif()
  if(MPPP_BENCHMARK_FLINT)
    target_link_libraries(${arg1} PRIVATE mp++::FLINT)
    target_compile_options(${arg1} PRIVATE "-DMPPP_BENCHMARK_FLINT")
  endif()
  target_include_directories(${arg1} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
  target_compile_options(${arg1} PRIVATE
    "$<$<CONFIG:Debug>:${MPPP_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${MPPP_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${MPPP_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${MPPP_CXX_FLAGS_RELEASE}>"
  )
  target_compile_features(${arg1} PRIVATE cxx_std_11)
  set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)

  # Visibility settings.
  set_target_properties(${arg1} PROPERTIES CXX_VISIBILITY_PRESET hidden)
  set_target_properties(${arg1} PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
  # Add the current benchmark to the "benchmark" custom target.
  add_dependencies(benchmark ${arg1})
  add_custom_command(TARGET benchmark POST_BUILD COMMAND "${arg1}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()

# NOTE: the malloc tracking feature requires glibc.
# Heuristically, let's enable it only Unix systems
# which are not OSX. Even on systems which are Unix
# but don't use glibc, the library should do no harm
# as long as the platform has sane rpath handling.
# NOTE: see also the explanation here:
# https://stackoverflow.com/questions/17803456/an-alternative-for-the-deprecated-malloc-hook-functionality-of-glibc
if(NOT WIN32 AND UNIX AND NOT APPLE)
  add_library(track_malloc SHARED track_malloc.cpp)
  set_target_properties(track_malloc PROPERTIES CXX_VISIBILITY_PRESET hidden)
  set_target_properties(track_malloc PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
  target_compile_options(track_malloc PRIVATE
      "$<$<CONFIG:Debug>:${MPPP_CXX_FLAGS_DEBUG}>"
      "$<$<CONFIG:Release>:${MPPP_CXX_FLAGS_RELEASE}>"
      "$<$<CONFIG:RelWithDebInfo>:${MPPP_CXX_FLAGS_RELEASE}>"
      "$<$<CONFIG:MinSizeRel>:${MPPP_CXX_FLAGS_RELEASE}>"
  )
  target_compile_features(track_malloc PRIVATE cxx_std_11)
  set_property(TARGET track_malloc PROPERTY CXX_EXTENSIONS NO)
endif()

ADD_MPPP_BENCHMARK(integer1_dot_product_unsigned)
ADD_MPPP_BENCHMARK(integer1_dot_product_signed)
ADD_MPPP_BENCHMARK(integer2_dot_product_unsigned)
ADD_MPPP_BENCHMARK(integer2_dot_product_signed)
ADD_MPPP_BENCHMARK(integer1_vec_lshift_unsigned)
ADD_MPPP_BENCHMARK(integer1_vec_lshift_signed)
ADD_MPPP_BENCHMARK(integer2_vec_lshift_unsigned)
ADD_MPPP_BENCHMARK(integer2_vec_lshift_signed)
ADD_MPPP_BENCHMARK(integer1_vec_mul_unsigned)
ADD_MPPP_BENCHMARK(integer1_vec_mul_signed)
ADD_MPPP_BENCHMARK(integer2_vec_mul_unsigned)
ADD_MPPP_BENCHMARK(integer2_vec_mul_signed)
ADD_MPPP_BENCHMARK(integer1_vec_div_unsigned)
ADD_MPPP_BENCHMARK(integer1_vec_div_signed)
ADD_MPPP_BENCHMARK(integer2_vec_div_unsigned)
ADD_MPPP_BENCHMARK(integer2_vec_div_signed)
ADD_MPPP_BENCHMARK(integer1_vec_gcd_signed)
ADD_MPPP_BENCHMARK(integer1_vec_lcm_signed)
ADD_MPPP_BENCHMARK(integer1_sort_unsigned)
ADD_MPPP_BENCHMARK(integer1_sort_signed)
ADD_MPPP_BENCHMARK(integer2_sort_unsigned)
ADD_MPPP_BENCHMARK(integer2_sort_signed)
ADD_MPPP_BENCHMARK(integer1_uint_conversion)
ADD_MPPP_BENCHMARK(integer1_int_conversion)
ADD_MPPP_BENCHMARK(integer2_uint_conversion)
ADD_MPPP_BENCHMARK(integer2_int_conversion)

if(MPPP_WITH_MPFR)
  ADD_MPPP_BENCHMARK(real_alloc)
  if(NOT WIN32 AND UNIX AND NOT APPLE)
    target_link_libraries(real_alloc PRIVATE track_malloc)
  endif()
endif()
