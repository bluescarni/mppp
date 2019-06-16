# Disable selection of the standard by pybind11.
set(PYBIND11_CPP_STANDARD "")

# Find pybind11.
find_package(pybind11 REQUIRED)

# NOTE: this is very similar to function for creating C++ tests.
function(ADD_MPPP_PYBIND11_TESTCASE arg1)
    if(MPPP_TEST_NSPLIT)
        math(EXPR __MPPP_TEST_NUM "(${_MPPP_TEST_NUM} + 1) % ${MPPP_TEST_NSPLIT}")
        set(_MPPP_TEST_NUM ${__MPPP_TEST_NUM} PARENT_SCOPE)
    endif()
    if(MPPP_TEST_NSPLIT AND "${MPPP_TEST_SPLIT_NUM}" STREQUAL "${_MPPP_TEST_NUM}")
        return()
    endif()
    pybind11_add_module(${arg1} ${arg1}.cpp)
    target_link_libraries(${arg1} PRIVATE mp++)
    target_compile_options(${arg1} PRIVATE "$<$<CONFIG:DEBUG>:${MPPP_CXX_FLAGS_DEBUG}>" "$<$<CONFIG:RELEASE>:${MPPP_CXX_FLAGS_RELEASE}>"
        "$<$<CONFIG:RELWITHDEBINFO>:${MPPP_CXX_FLAGS_RELEASE}>")
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
    # Copy over the Python test runner.
    # NOTE: need to use file(GENERATE) because in multi-config build systems (e.g., MSVC) we need to copy
    # the runner in a directory depending on the config type, and we need to do it at generation time.
    # We can fetch the correct directory by reading the TARGET_FILE_DIR property of the python module.
    file(GENERATE OUTPUT "$<TARGET_FILE_DIR:${arg1}>/run_${arg1}.py" INPUT "${CMAKE_CURRENT_SOURCE_DIR}/run_${arg1}.py")
    # Add the actual test.
    add_test(NAME ${arg1} COMMAND "${PYTHON_EXECUTABLE}" run_${arg1}.py WORKING_DIRECTORY "$<TARGET_FILE_DIR:${arg1}>")
endfunction()

ADD_MPPP_PYBIND11_TESTCASE(pybind11_test_01)

# Change the split test number in the parent scope.
if(MPPP_TEST_NSPLIT)
    set(_MPPP_TEST_NUM ${_MPPP_TEST_NUM} PARENT_SCOPE)
endif()
