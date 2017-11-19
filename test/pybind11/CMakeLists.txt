# Disable automatic detection of the standard by pybind11.
set(PYBIND11_CPP_STANDARD "")

# Find pybind11.
find_package(pybind11 REQUIRED)

# NOTE: this is very similar to function for creating C++ tests.
function(ADD_MPPP_PYBIND11_TESTCASE arg1)
    pybind11_add_module(${arg1} ${arg1}.cpp)
    target_link_libraries(${arg1} PRIVATE mp++)
    target_compile_options(${arg1} PRIVATE "$<$<CONFIG:DEBUG>:${MPPP_CXX_FLAGS_DEBUG}>" "$<$<CONFIG:RELEASE>:${MPPP_CXX_FLAGS_RELEASE}>")
    # NOTE: for clang-cl, cmake tries to set -std=c++11 here, which makes
    # it error out. Disable it as MSVC 2015 is implicitly C++14 anyway.
    if(NOT (YACMA_COMPILER_IS_MSVC AND YACMA_COMPILER_IS_CLANGXX))
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
    endif()
endfunction()

ADD_MPPP_PYBIND11_TESTCASE(pybind11_test_01)
