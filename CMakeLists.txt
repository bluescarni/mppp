cmake_minimum_required(VERSION 3.8.0)
# NOTE: policy for using the CheckIPOSupported module:
# https://cmake.org/cmake/help/latest/policy/CMP0069.html
cmake_policy(SET CMP0069 NEW)

# Set default build type to "Release".
# NOTE: this should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	FORCE)
endif()

project(mp++ VERSION 0.26 LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/yacma")

message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "System processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "mp++ version: ${mp++_VERSION}")

include(YACMACompilerLinkerSettings)
include(CheckCXXCompilerFlag)

# The build options.
option(MPPP_BUILD_TESTS "Build unit tests." OFF)
option(MPPP_BUILD_BENCHMARKS "Build benchmarks." OFF)
option(MPPP_BENCHMARK_BOOST "Build benchmarks against Boost.Multiprecision (effective only if MPPP_BUILD_BENCHMARKS is TRUE, requires Boost)." OFF)
mark_as_advanced(MPPP_BENCHMARK_BOOST)
option(MPPP_BENCHMARK_FLINT "Build benchmarks against Flint (effective only if MPPP_BUILD_BENCHMARKS is TRUE, requires FLINT)." OFF)
mark_as_advanced(MPPP_BENCHMARK_FLINT)
option(MPPP_WITH_MPFR "Enable features relying on MPFR." OFF)
option(MPPP_WITH_ARB "Enable features relying on Arb." OFF)
option(MPPP_WITH_MPC "Enable features relying on MPC." OFF)
option(MPPP_WITH_QUADMATH "Enable features relying on libquadmath (e.g., the real128 type)." OFF)
option(MPPP_WITH_BOOST_S11N "Enable features relying on the Boost.Serialization library." OFF)
option(MPPP_TEST_PYBIND11 "Build tests for the pybind11 integration utilities (effective only if MPPP_BUILD_TESTS is TRUE, requires pybind11 and Python).")
mark_as_advanced(MPPP_TEST_PYBIND11)
option(MPPP_BUILD_STATIC_LIBRARY "Build mp++ as a static library, instead of dynamic." OFF)
if(YACMA_COMPILER_IS_MSVC)
    option(MPPP_MSVC_UNICODE "Enable Unicode solutions for MSVC." OFF)
endif()
# Build option: enable IPO.
option(MPPP_ENABLE_IPO "Enable IPO (requires CMake >= 3.9 and compiler support)." OFF)
mark_as_advanced(MPPP_ENABLE_IPO)

if(MPPP_WITH_ARB AND NOT MPPP_WITH_MPFR)
    message(FATAL_ERROR "Arb support requires MPFR, please enable the MPPP_WITH_MPFR build option.")
endif()

if(MPPP_WITH_MPC AND NOT MPPP_WITH_MPFR)
    message(FATAL_ERROR "MPC support requires MPFR, please enable the MPPP_WITH_MPFR build option.")
endif()

if(YACMA_COMPILER_IS_MSVC AND MPPP_BUILD_STATIC_LIBRARY)
    option(MPPP_BUILD_STATIC_LIBRARY_WITH_DYNAMIC_MSVC_RUNTIME "Link to the dynamic MSVC runtime when building mp++ as a static library." OFF)
    mark_as_advanced(MPPP_BUILD_STATIC_LIBRARY_WITH_DYNAMIC_MSVC_RUNTIME)
endif()

# NOTE: on Unix systems, the correct library installation path
# could be something other than just "lib", such as "lib64",
# "lib32", etc., depending on platform/configuration. Apparently,
# CMake provides this information via the GNUInstallDirs module.
# Let's enable this for now on all Unixes except OSX.
# NOTE: potentially, this could be applicable to Cygwin as well.
#
# https://cmake.org/cmake/help/v3.15/module/GNUInstallDirs.html
# https://cmake.org/pipermail/cmake/2013-July/055375.html
if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)
    set(_MPPP_INSTALL_LIBDIR_DEFAULT "${CMAKE_INSTALL_LIBDIR}")
else()
    set(_MPPP_INSTALL_LIBDIR_DEFAULT "lib")
endif()
if(NOT MPPP_INSTALL_LIBDIR)
    set(MPPP_INSTALL_LIBDIR "${_MPPP_INSTALL_LIBDIR_DEFAULT}" CACHE STRING
        "Library installation directory." FORCE)
endif()
mark_as_advanced(MPPP_INSTALL_LIBDIR)
message(STATUS "Library installation directory: ${MPPP_INSTALL_LIBDIR}")

# Assemble the flags.
set(MPPP_CXX_FLAGS_DEBUG ${YACMA_CXX_FLAGS} ${YACMA_CXX_FLAGS_DEBUG})
set(MPPP_CXX_FLAGS_RELEASE ${YACMA_CXX_FLAGS})
if(YACMA_COMPILER_IS_MSVC)
  # On both cl and clang-cl, disable the idiotic minmax macros and enable the bigobj option.
  # Also, enable the WIN32_LEAN_AND_MEAN definition:
  # https://stackoverflow.com/questions/11040133/what-does-defining-win32-lean-and-mean-exclude-exactly
  list(APPEND MPPP_CXX_FLAGS_DEBUG "-DNOMINMAX" "/bigobj" "-DWIN32_LEAN_AND_MEAN")
  list(APPEND MPPP_CXX_FLAGS_RELEASE "-DNOMINMAX" "/bigobj" "-DWIN32_LEAN_AND_MEAN")
  if(MPPP_MSVC_UNICODE)
    # NOTE: Unicode solutions for MSVC can be enabled through
    # these definitions.
    list(APPEND MPPP_CXX_FLAGS_DEBUG "-DUNICODE" "-D_UNICODE")
    list(APPEND MPPP_CXX_FLAGS_RELEASE "-DUNICODE" "-D_UNICODE")
  endif()
  if(YACMA_COMPILER_IS_CLANGXX)
    # clang-cl emits various warnings from GMP/MPFR, let's just silence them.
    # NOTE: at one point in the recent past, MSVC added an options similar to GCC's isystem:
    # https://blogs.msdn.microsoft.com/vcblog/2017/12/13/broken-warnings-theory/
    # We probably just need to wait for this to be picked up by CMake/clang-cl. Let's
    # revisit the issue in the future.
    list(APPEND _MPPP_CLANG_CL_DISABLED_WARNINGS
        "-Wno-unused-variable"
        "-Wno-inconsistent-dllimport"
        "-Wno-unknown-pragmas"
        "-Wno-unused-parameter"
        "-Wno-sign-compare"
        "-Wno-deprecated-declarations"
        "-Wno-deprecated-dynamic-exception-spec"
        "-Wno-old-style-cast"
        "-Wno-sign-conversion"
        "-Wno-non-virtual-dtor"
        "-Wno-deprecated"
        "-Wno-shadow"
        "-Wno-shorten-64-to-32"
        "-Wno-reserved-id-macro"
        "-Wno-undef"
        "-Wno-c++98-compat-pedantic"
        "-Wno-documentation-unknown-command"
        "-Wno-zero-as-null-pointer-constant"
        "-Wno-language-extension-token"
        "-Wno-gnu-anonymous-struct"
        "-Wno-nested-anon-types"
        "-Wno-documentation"
        "-Wno-comma"
        "-Wno-nonportable-system-include-path"
        "-Wno-global-constructors"
        "-Wno-redundant-parens"
        "-Wno-exit-time-destructors"
        "-Wno-missing-noreturn"
        "-Wno-switch-enum"
        "-Wno-covered-switch-default"
        "-Wno-float-equal"
        "-Wno-double-promotion"
        "-Wno-microsoft-enum-value"
        "-Wno-missing-prototypes"
        "-Wno-implicit-fallthrough"
        "-Wno-format-nonliteral"
        "-Wno-cast-qual"
        "-Wno-disabled-macro-expansion"
        "-Wno-unused-private-field"
        "-Wno-unused-template"
        "-Wno-unused-macros"
        "-Wno-extra-semi-stmt"
        "-Wno-c++98-compat")
    list(APPEND MPPP_CXX_FLAGS_DEBUG ${_MPPP_CLANG_CL_DISABLED_WARNINGS})
    list(APPEND MPPP_CXX_FLAGS_RELEASE ${_MPPP_CLANG_CL_DISABLED_WARNINGS})
    unset(_MPPP_CLANG_CL_DISABLED_WARNINGS)
  else()
    # Same as above, disable some cl warnings.
    list(APPEND MPPP_CXX_FLAGS_DEBUG "/wd4459" "/wd4127")
    list(APPEND MPPP_CXX_FLAGS_RELEASE "/wd4459" "/wd4127")
  endif()
  # Enable strict conformance mode, if supported.
  set(CMAKE_REQUIRED_QUIET TRUE)
  check_cxx_compiler_flag("/permissive-" _MPPP_MSVC_SUPPORTS_STRICT_CONFORMANCE)
  unset(CMAKE_REQUIRED_QUIET)
  if(_MPPP_MSVC_SUPPORTS_STRICT_CONFORMANCE)
    message(STATUS "The '/permissive-' flag is supported, enabling it.")
    list(APPEND MPPP_CXX_FLAGS_DEBUG "/permissive-")
    list(APPEND MPPP_CXX_FLAGS_RELEASE "/permissive-")
  endif()
  unset(_MPPP_MSVC_SUPPORTS_STRICT_CONFORMANCE)
endif()
if(YACMA_COMPILER_IS_INTELXX)
  # NOTE: on MSVC we use the push/pop pragmas, but they do not seem to work on Intel (the pragmas
  # in icc influence the behaviour at instantiation point, not at definition point).
  list(APPEND MPPP_CXX_FLAGS_DEBUG "-diag-disable" "3373,1682,2259")
  list(APPEND MPPP_CXX_FLAGS_RELEASE "-diag-disable" "3373,1682,2259")
endif()
if(MINGW)
  # In MinGW some tests generate big object files.
  message(STATUS "Enabling the '-Wa,-mbig-obj' flag for MinGW.")
  list(APPEND MPPP_CXX_FLAGS_DEBUG "-Wa,-mbig-obj")
  list(APPEND MPPP_CXX_FLAGS_RELEASE "-Wa,-mbig-obj")
endif()

# Explanation: on MSVC, when building static libraries, it is good practice to link
# to the static runtime. CMake, however, is hard-coded to link to the dynamic runtime.
# Hence we hackishly replace the /MD flag with /MT. This is the approach suggested
# in the CMake FAQ:
#
# https://gitlab.kitware.com/cmake/community/wikis/FAQ#how-can-i-build-my-msvc-application-with-a-static-runtime
#
# Note that at one point CMake added the possiblity to set this as a target property,
# so in the future we should definitely migrate to that approach:
#
# https://cmake.org/cmake/help/git-master/prop_tgt/MSVC_RUNTIME_LIBRARY.html
#
# NOTE: the MPPP_BUILD_STATIC_LIBRARY_WITH_DYNAMIC_MSVC_RUNTIME option overrides this choice
# and keeps the dynamic runtime. This can be needed in specific rare situations.
if(YACMA_COMPILER_IS_MSVC AND MPPP_BUILD_STATIC_LIBRARY AND NOT MPPP_BUILD_STATIC_LIBRARY_WITH_DYNAMIC_MSVC_RUNTIME)
    foreach(flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
endif()

# List of source files.
set(MPPP_SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/integer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rational.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/type_name.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/parse_complex.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/utils.cpp"
)

if(MPPP_WITH_QUADMATH)
    set(MPPP_SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/real128.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/complex128.cpp"
        "${MPPP_SRC_FILES}"
    )
endif()

if(MPPP_WITH_MPFR)
    set(MPPP_SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/real.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/mpfr_arb_cleanup.cpp"
        "${MPPP_SRC_FILES}")
endif()

if(MPPP_WITH_ARB)
    set(MPPP_SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/arb.cpp"
        "${MPPP_SRC_FILES}")
endif()

if(MPPP_WITH_MPC)
    set(MPPP_SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/complex.cpp"
        "${MPPP_SRC_FILES}")
endif()

# Make mp++ header files accessible in Visual Studio IDE.
if(YACMA_COMPILER_IS_MSVC)
  set(MPPP_HEADER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/concepts.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/exceptions.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/integer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/mp++.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/rational.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/real.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/complex.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/real128.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/complex128.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/type_name.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/fwd.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/detail/gmp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/detail/integer_literals.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/detail/rational_literals.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/detail/real_literals.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/detail/real128_literal.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/detail/mpfr.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/detail/mpc.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/detail/type_traits.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/detail/utils.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/detail/visibility.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/detail/parse_complex.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++/extra/pybind11.hpp"
  )
  source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++" PREFIX "Header Files" FILES ${MPPP_HEADER_FILES})
  set(MPPP_SRC_FILES ${MPPP_SRC_FILES} ${MPPP_HEADER_FILES})
endif()

if(MPPP_BUILD_STATIC_LIBRARY)
    # Setup of the mp++ static library.
    message(STATUS "mp++ will be built as a static library.")
    set(MPPP_STATIC_BUILD "#define MPPP_STATIC_BUILD")
    add_library(mp++ STATIC "${MPPP_SRC_FILES}")
else()
    # Setup of the mp++ shared library.
    add_library(mp++ SHARED "${MPPP_SRC_FILES}")
    set_property(TARGET mp++ PROPERTY VERSION "12.0")
    set_property(TARGET mp++ PROPERTY SOVERSION 12)
    set_property(TARGET mp++ PROPERTY DEFINE_SYMBOL "mppp_EXPORTS")
    set_target_properties(mp++ PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(mp++ PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
endif()

# Setup common to both the shared and static variants.
target_compile_options(mp++ PRIVATE
    "$<$<CONFIG:Debug>:${MPPP_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${MPPP_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${MPPP_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${MPPP_CXX_FLAGS_RELEASE}>"
)

# Ensure that C++11 is employed when both compiling and consuming mp++.
target_compile_features(mp++ PUBLIC cxx_std_11)
# Enforce vanilla C++ when compiling mp++.
set_property(TARGET mp++ PROPERTY CXX_EXTENSIONS NO)

target_include_directories(mp++ PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>)

# IPO setup.
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.9.0")
    if(MPPP_ENABLE_IPO)
        include(CheckIPOSupported)
        check_ipo_supported(RESULT _MPPP_IPO_RESULT OUTPUT _MPPP_IPO_OUTPUT)
        if (_MPPP_IPO_RESULT)
            message(STATUS "IPO requested and supported, enabling.")
            set_property(TARGET mp++ PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        else()
            message(STATUS "IPO requested, but it is not supported by the compiler:\n${_MPPP_IPO_OUTPUT}")
        endif()
        unset(_MPPP_IPO_RESULT)
        unset(_MPPP_IPO_OUTPUT)
    endif()
endif()

# Find GMP first of all. We will make mp++ link to the imported
# target later, because of linker sensitivity to the linking order
# on some platforms (see the comment below).
find_package(mp++_GMP REQUIRED)

# Check support for certain GMP functions.
include(CheckSymbolExists)
set(CMAKE_REQUIRED_INCLUDES "${MPPP_GMP_INCLUDE_DIR}")
set(CMAKE_REQUIRED_LIBRARIES "${MPPP_GMP_LIBRARY}")
check_symbol_exists(mpn_divexact_1 "gmp.h" MPPP_GMP_HAVE_MPN_DIVEXACT_1)
unset(CMAKE_REQUIRED_INCLUDES)
unset(CMAKE_REQUIRED_LIBRARIES)

# Optional dependency on MPC.
if(MPPP_WITH_MPC)
    find_package(mp++_MPC REQUIRED)
    target_link_libraries(mp++ PUBLIC mp++::MPC)
    set(MPPP_ENABLE_MPC "#define MPPP_WITH_MPC")
endif()

# Optional dependency on MPFR.
if(MPPP_WITH_MPFR)
    find_package(mp++_MPFR REQUIRED)

    # Check support for certain MPFR functions.
    set(CMAKE_REQUIRED_INCLUDES "${MPPP_MPFR_INCLUDE_DIR}")
    set(CMAKE_REQUIRED_LIBRARIES "${MPPP_MPFR_LIBRARY}")
    check_symbol_exists(mpfr_get_q "mpfr.h" MPPP_MPFR_HAVE_MPFR_GET_Q)
    check_symbol_exists(mpfr_rootn_ui "mpfr.h" MPPP_MPFR_HAVE_MPFR_ROOTN_UI)
    check_symbol_exists(mpfr_gamma_inc "mpfr.h" MPPP_MPFR_HAVE_MPFR_GAMMA_INC)
    check_symbol_exists(mpfr_beta "mpfr.h" MPPP_MPFR_HAVE_MPFR_BETA)
    check_symbol_exists(mpfr_roundeven "mpfr.h" MPPP_MPFR_HAVE_MPFR_ROUNDEVEN)
    check_symbol_exists(mpfr_fmodquo "mpfr.h" MPPP_MPFR_HAVE_MPFR_FMODQUO)
    check_symbol_exists(mpfr_get_str_ndigits "mpfr.h" MPPP_MPFR_HAVE_MPFR_GET_STR_NDIGITS)
    unset(CMAKE_REQUIRED_INCLUDES)
    unset(CMAKE_REQUIRED_LIBRARIES)

    target_link_libraries(mp++ PUBLIC mp++::MPFR)
    set(MPPP_ENABLE_MPFR "#define MPPP_WITH_MPFR")
endif()

# Optional dependency on Arb.
if(MPPP_WITH_ARB)
    find_package(mp++_FLINT REQUIRED)
    find_package(mp++_Arb REQUIRED)

    # Check support for certain Arb functions.
    set(CMAKE_REQUIRED_INCLUDES "${MPPP_ARB_INCLUDE_DIR}")
    set(CMAKE_REQUIRED_LIBRARIES "${MPPP_ARB_LIBRARY}")
    check_symbol_exists(acb_agm "acb.h" MPPP_ARB_HAVE_ACB_AGM)
    unset(CMAKE_REQUIRED_INCLUDES)
    unset(CMAKE_REQUIRED_LIBRARIES)

    target_link_libraries(mp++ PRIVATE mp++::Arb mp++::FLINT)
    set(MPPP_ENABLE_ARB "#define MPPP_WITH_ARB")
endif()

# Optional dependency on quadmath.
if(MPPP_WITH_QUADMATH)
    include(CheckTypeSize)
    check_type_size(__float128 _MPPP_FLOAT128_EXISTS BUILTIN_TYPES_ONLY LANGUAGE CXX)
    if(NOT _MPPP_FLOAT128_EXISTS)
        message(FATAL_ERROR "The 'MPPP_WITH_QUADMATH' option was enabled but the '__float128' type does not exist.")
    endif()
    unset(_MPPP_FLOAT128_EXISTS)

    find_package(mp++_quadmath REQUIRED)

    # Check support for certain quadmath functions.
    # NOTE: the CMAKE_REQUIRED_INCLUDES variable
    # needs to be defined only if we are not using
    # libquadmath directly.
    if(NOT MPPP_QUADMATH_USE_DIRECTLY)
        set(CMAKE_REQUIRED_INCLUDES "${MPPP_QUADMATH_INCLUDE_DIR}")
    endif()
    set(CMAKE_REQUIRED_LIBRARIES "${MPPP_QUADMATH_LIBRARY}")
    check_symbol_exists(exp2q "quadmath.h" MPPP_QUADMATH_HAVE_EXP2Q)
    check_symbol_exists(logbq "quadmath.h" MPPP_QUADMATH_HAVE_LOGBQ)
    if(NOT MPPP_QUADMATH_USE_DIRECTLY)
        unset(CMAKE_REQUIRED_INCLUDES)
    endif()
    unset(CMAKE_REQUIRED_LIBRARIES)

    target_link_libraries(mp++ PRIVATE mp++::quadmath)
    set(MPPP_ENABLE_QUADMATH "#define MPPP_WITH_QUADMATH")
endif()

# Optional dependency on Boost s11n.
set(_MPPP_MIN_BOOST_VERSION "1.60")
if(MPPP_WITH_BOOST_S11N)
    find_package(Boost ${_MPPP_MIN_BOOST_VERSION} REQUIRED COMPONENTS serialization)
    target_link_libraries(mp++ PUBLIC Boost::serialization Boost::disable_autolinking)
endif()

# Mandatory dependency on GMP.
# NOTE: depend on GMP *after* optionally depending on MPFR, as the order
# of the libraries matters on some platforms.
target_link_libraries(mp++ PUBLIC mp++::GMP)

# Configure config.hpp.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/include/mp++/config.hpp" @ONLY)

# Configure the doc files.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/conf.py.in" "${CMAKE_CURRENT_SOURCE_DIR}/doc/conf.py" @ONLY)

# Installation of the header files.
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/mp++" DESTINATION include)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/mp++/config.hpp" DESTINATION include/mp++)

# Installation of the library.
install(TARGETS mp++
    EXPORT mp++_export
    LIBRARY DESTINATION "${MPPP_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${MPPP_INSTALL_LIBDIR}"
    RUNTIME DESTINATION bin
)

# Configure mp++-config.cmake.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mp++-config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/mp++-config.cmake" @ONLY)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/mp++-config.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Findmp++_GMP.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Findmp++_MPFR.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Findmp++_MPC.cmake"
    DESTINATION "${MPPP_INSTALL_LIBDIR}/cmake/mp++")
install(EXPORT mp++_export NAMESPACE mp++:: DESTINATION "${MPPP_INSTALL_LIBDIR}/cmake/mp++")
# Take care of versioning.
include(CMakePackageConfigHelpers)
# NOTE: SameMinorVersion available only
# since CMake 3.11.
if(${CMAKE_VERSION} VERSION_LESS "3.11.0")
    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/mp++-config-version.cmake" COMPATIBILITY SameMajorVersion)
else()
    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/mp++-config-version.cmake" COMPATIBILITY SameMinorVersion)
endif()
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mp++-config-version.cmake" DESTINATION "${MPPP_INSTALL_LIBDIR}/cmake/mp++")

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

unset(_MPPP_MIN_BOOST_VERSION)
