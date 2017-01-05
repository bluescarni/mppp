cmake_policy(SET CMP0048 NEW)

project(mp++ VERSION 0.1 LANGUAGES CXX C)

cmake_minimum_required(VERSION 3.2.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/yacma")

include(YACMACompilerLinkerSettings)

option(MPPP_BUILD_TESTS "Build unit tests." OFF)
option(MPPP_BUILD_BENCHMARKS "Build benchmarks." OFF)
option(MPPP_BENCHMARK_BOOST "Build benchmarks against Boost.Multiprecision (effective only if MPPP_BUILD_BENCHMARKS is TRUE, requires Boost)." OFF)
option(MPPP_BENCHMARK_FLINT "Build benchmarks against Flint (effective only if MPPP_BUILD_BENCHMARKS is TRUE, requires FLINT)." OFF)
option(MPPP_WITH_LONG_DOUBLE "Enable interoperability with long double (requires MPFR)." OFF)

find_package(GMP REQUIRED)

add_library(mp++ INTERFACE)

if(YACMA_COMPILER_IS_MSVC)
    option(MPPP_MSVC_STATIC_RUNTIME "Use the MSVC static runtime instead of the shared runtime." OFF)
endif()

if(YACMA_COMPILER_IS_INTELXX)
    # NOTE: on MSVC we use the push/pop pragmas, but they do not seem to work on Intel (the pragmas
    # in icc influence the behaviour at instantiation point, not at definition point).
    target_compile_options(mp++ INTERFACE "-diag-disable" "3373,1682")
endif()

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
