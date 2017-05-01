cmake_policy(SET CMP0048 NEW)

project(mp++ VERSION 0.1 LANGUAGES CXX C)

cmake_minimum_required(VERSION 3.2.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/yacma")

include(YACMACompilerLinkerSettings)

# Assemble the flags.
set(MPPP_CXX_FLAGS_DEBUG ${YACMA_CXX_FLAGS} ${YACMA_CXX_FLAGS_DEBUG})
set(MPPP_CXX_FLAGS_RELEASE ${YACMA_CXX_FLAGS})
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND YACMA_COMPILER_IS_CLANGXX)
  message(STATUS "Clang compiler on OSX detected, setting the standard library to 'libc++'.")
  list(APPEND MPPP_CXX_FLAGS_DEBUG "-stdlib=libc++")
  list(APPEND MPPP_CXX_FLAGS_RELEASE "-stdlib=libc++")
endif()
if(YACMA_COMPILER_IS_MSVC)
  # Disable the idiotic minmax macros on MSVC, some annoying warnings,
  # and enable the bigobj option.
  list(APPEND MPPP_CXX_FLAGS_DEBUG "-DNOMINMAX" "/wd4459" "/wd4127" "/bigobj")
  list(APPEND MPPP_CXX_FLAGS_RELEASE "-DNOMINMAX" "/wd4459" "/wd4127" "/bigobj")
endif()
if(YACMA_COMPILER_IS_INTELXX)
  # NOTE: on MSVC we use the push/pop pragmas, but they do not seem to work on Intel (the pragmas
  # in icc influence the behaviour at instantiation point, not at definition point).
  list(APPEND MPPP_CXX_FLAGS_DEBUG "-diag-disable" "3373,1682")
  list(APPEND MPPP_CXX_FLAGS_RELEASE "-diag-disable" "3373,1682")
endif()
if(MINGW)
  # In MinGW some tests generate big object files.
  list(APPEND MPPP_CXX_FLAGS_DEBUG "-Wa,-mbig-obj")
  list(APPEND MPPP_CXX_FLAGS_RELEASE "-Wa,-mbig-obj")
endif()

option(MPPP_BUILD_TESTS "Build unit tests." OFF)
option(MPPP_BUILD_BENCHMARKS "Build benchmarks." OFF)
option(MPPP_BENCHMARK_BOOST "Build benchmarks against Boost.Multiprecision (effective only if MPPP_BUILD_BENCHMARKS is TRUE, requires Boost)." OFF)
option(MPPP_BENCHMARK_FLINT "Build benchmarks against Flint (effective only if MPPP_BUILD_BENCHMARKS is TRUE, requires FLINT)." OFF)
option(MPPP_WITH_LONG_DOUBLE "Enable interoperability with long double (requires MPFR)." OFF)
if(YACMA_COMPILER_IS_MSVC)
    option(MPPP_MSVC_STATIC_RUNTIME "Use the MSVC static runtime instead of the shared runtime." OFF)
endif()

# This is just a hackish way of detecting concepts, need to revisit once
# more compilers support them.
check_cxx_compiler_flag("-fconcepts" MPPP_HAVE_CONCEPTS)

find_package(GMP REQUIRED)

# Setup of the mp++ interface library.
add_library(mp++ INTERFACE)
if(MPPP_WITH_LONG_DOUBLE)
    find_package(MPFR REQUIRED)
    target_compile_definitions(mp++ INTERFACE MPPP_WITH_LONG_DOUBLE)
    target_link_libraries(mp++ INTERFACE MPFR::MPFR)
endif()
# mp++ depends on GMP.
# NOTE: depend on GMP *after* optionally depending on MPFR, as the order
# of the libraries matters on some platforms.
target_link_libraries(mp++ INTERFACE GMP::GMP)
target_include_directories(mp++ INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
install(DIRECTORY include/ DESTINATION include)

if(MPPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(MPPP_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()
