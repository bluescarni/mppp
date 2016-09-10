cmake_policy(SET CMP0048 NEW)

project(mp++ VERSION 0.1 LANGUAGES CXX)

cmake_minimum_required(VERSION 3.2.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/yacma")

include(YACMACompilerLinkerSettings)

option(MPPP_BUILD_TESTS "Build unit tests." OFF)
option(MPPP_BUILD_BENCHMARKS "Build benchmarks." OFF)
option(MPPP_WITH_LONG_DOUBLE "Enable interoperability with long double (requires MPFR)." OFF)

find_package(GMP REQUIRED)

add_library(mp++ INTERFACE)

target_link_libraries(mp++ INTERFACE GMP::GMP)

if(MPPP_WITH_LONG_DOUBLE)
    find_package(MPFR REQUIRED)
    target_compile_definitions(mp++ INTERFACE MPPP_WITH_LONG_DOUBLE)
    target_link_libraries(mp++ INTERFACE MPFR::MPFR)
endif()

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
