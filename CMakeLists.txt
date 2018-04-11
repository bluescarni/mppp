cmake_minimum_required(VERSION 3.3.0)

project(mp++ VERSION 0.11 LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/yacma")

message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "mp++ version: ${mp++_VERSION}")

# Set default build type to "Release".
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	FORCE)
endif()

# The build options.
option(MPPP_BUILD_TESTS "Build unit tests." OFF)
option(MPPP_BUILD_BENCHMARKS "Build benchmarks." OFF)
option(MPPP_BENCHMARK_BOOST "Build benchmarks against Boost.Multiprecision (effective only if MPPP_BUILD_BENCHMARKS is TRUE, requires Boost)." OFF)
mark_as_advanced(MPPP_BENCHMARK_BOOST)
option(MPPP_BENCHMARK_FLINT "Build benchmarks against Flint (effective only if MPPP_BUILD_BENCHMARKS is TRUE, requires FLINT)." OFF)
mark_as_advanced(MPPP_BENCHMARK_FLINT)
option(MPPP_WITH_MPFR "Enable features relying on MPFR (e.g., interoperability with long double)." OFF)
option(MPPP_WITH_QUADMATH "Enable features relying on libquadmath (e.g., the real128 type)." OFF)
option(MPPP_TEST_PYBIND11 "Build tests for the pybind11 integration utilities (effective only if MPPP_BUILD_TESTS is TRUE, requires pybind11 and Python).")
mark_as_advanced(MPPP_TEST_PYBIND11)

include(YACMACompilerLinkerSettings)
include(CheckCXXCompilerFlag)

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
  if(YACMA_COMPILER_IS_CLANGXX)
    # clang-cl emits various warnings from GMP/MPFR, let's just silence them.
    # NOTE: at one point in the recent past, MSVC added an options similar to GCC's isystem:
    # https://blogs.msdn.microsoft.com/vcblog/2017/12/13/broken-warnings-theory/
    # We probably just need to wait for this to be picked up by CMake/clang-cl. Let's
    # revisit the issue in the future.
    list(APPEND _MPPP_CLANG_CL_DISABLED_WARNINGS
        "-Wno-deprecated-dynamic-exception-spec"
        "-Wno-old-style-cast"
        "-Wno-sign-conversion")
    list(APPEND MPPP_CXX_FLAGS_DEBUG ${_MPPP_CLANG_CL_DISABLED_WARNINGS})
    list(APPEND MPPP_CXX_FLAGS_RELEASE ${_MPPP_CLANG_CL_DISABLED_WARNINGS})
    unset(_MPPP_CLANG_CL_DISABLED_WARNINGS)
  endif()
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

# Concepts detection.
if(YACMA_COMPILER_IS_GNUCXX)
    # This is just a hackish way of detecting concepts, need to revisit once
    # more compilers support them.
    check_cxx_compiler_flag("-fconcepts" MPPP_COMPILER_SUPPORTS_CONCEPTS)
endif()

# Setup of the mp++ interface library.
add_library(mp++ INTERFACE)

# Find GMP first of all. We will make mp++ link to the imported
# target later, because of linker sensitivity to the linking order
# on some platforms (see the comment below).
find_package(GMP REQUIRED)

# Optional dependency on MPFR.
if(MPPP_WITH_MPFR)
    find_package(MPFR REQUIRED)
    target_link_libraries(mp++ INTERFACE MPFR::MPFR)
    set(MPPP_ENABLE_MPFR "#define MPPP_WITH_MPFR")
endif()

# Optional dependency on quadmath.
if(MPPP_WITH_QUADMATH)
    include(CheckTypeSize)
    check_type_size(__float128 _MPPP_FLOAT128_EXISTS BUILTIN_TYPES_ONLY LANGUAGE CXX)
    if(NOT _MPPP_FLOAT128_EXISTS)
        message(FATAL_ERROR "The 'MPPP_WITH_QUADMATH' option was enabled but the '__float128' type does not exist.")
    endif()
    unset(_MPPP_FLOAT128_EXISTS)
    find_package(Quadmath REQUIRED)
    target_link_libraries(mp++ INTERFACE Quadmath::quadmath)
    set(MPPP_ENABLE_QUADMATH "#define MPPP_WITH_QUADMATH")
endif()

# Mandatory dependency on GMP.
# NOTE: depend on GMP *after* optionally depending on MPFR, as the order
# of the libraries matters on some platforms.
target_link_libraries(mp++ INTERFACE GMP::GMP)

# Depend on DbgHelp on MSVC, for the demangling support.
if(YACMA_COMPILER_IS_MSVC)
    find_package(DbgHelp REQUIRED)
    target_link_libraries(mp++ INTERFACE DbgHelp::DbgHelp)
endif()

# Finish setting up the mp++ interface target.
target_include_directories(mp++ INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>)

# Configure config.hpp.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/include/mp++/config.hpp" @ONLY)

# Configure the doc files.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/Doxyfile.in" "${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/Doxyfile" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/sphinx/conf.py.in" "${CMAKE_CURRENT_SOURCE_DIR}/doc/sphinx/conf.py" @ONLY)

# Installation.
# Setup of the export.
install(TARGETS mp++ EXPORT mp++_export)
# Setup of the optional deps.
set(_MPPP_CONFIG_OPTIONAL_DEPS)
if(MPPP_WITH_MPFR)
    set(_MPPP_CONFIG_OPTIONAL_DEPS "${_MPPP_CONFIG_OPTIONAL_DEPS}find_package(MPFR REQUIRED)\n")
endif()
if(MPPP_WITH_QUADMATH)
    set(_MPPP_CONFIG_OPTIONAL_DEPS "${_MPPP_CONFIG_OPTIONAL_DEPS}find_package(Quadmath REQUIRED)\n")
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mp++-config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/mp++-config.cmake" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mp++-config.cmake" DESTINATION "lib/cmake/mp++")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindGMP.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindMPFR.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindQuadmath.cmake" DESTINATION "lib/cmake/mp++")
if(YACMA_COMPILER_IS_MSVC)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindDbgHelp.cmake" DESTINATION "lib/cmake/mp++")
endif()
install(EXPORT mp++_export NAMESPACE mp++:: DESTINATION lib/cmake/mp++)
# Take care of versioning.
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/mp++-config-version.cmake" VERSION ${mp++_VERSION}
	COMPATIBILITY SameMajorVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mp++-config-version.cmake" DESTINATION "lib/cmake/mp++")

# Installation of the header files.
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++" DESTINATION include)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/mp++/config.hpp" DESTINATION include/mp++)

# This is just a simple counter variable, internal use only.
set(_MPPP_TEST_NUM "0")
# Check splitting options. These need to be set from the command line.
# - MPPP_TEST_NSPLIT: number of chunks into which the unit tests will be divided (must be > 1).
# - MPPP_TEST_SPLIT_NUM: 0-based index of the chunk to run.
if(MPPP_TEST_NSPLIT AND "${MPPP_TEST_SPLIT_NUM}" STREQUAL "")
	message(FATAL_ERROR "Test splitting was requested, but the MPPP_TEST_SPLIT_NUM variable was not set.")
elseif(NOT MPPP_TEST_NSPLIT AND NOT "${MPPP_TEST_SPLIT_NUM}" STREQUAL "")
	message(FATAL_ERROR "The MPPP_TEST_SPLIT_NUM variable was set, but test splitting was not requested.")
endif()
if(MPPP_TEST_NSPLIT)
	message(STATUS "Tests will be split into ${MPPP_TEST_NSPLIT} chunks. The chunk with index ${MPPP_TEST_SPLIT_NUM} will be processed.")
endif()

if(MPPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(MPPP_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()
