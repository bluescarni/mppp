include(YACMACompilerLinkerSettings)

# We have some multithreaded tests, so let's activate threading support.
set(THREADS_PREFER_PTHREAD_FLAG YES)
find_package(Threads REQUIRED)
unset(THREADS_PREFER_PTHREAD_FLAG)

macro(ADD_MPPP_TESTCASE arg1)
    add_executable(${arg1} ${arg1}.cpp)
    target_link_libraries(${arg1} mp++ Threads::Threads)
    target_compile_options(${arg1} PUBLIC "$<$<CONFIG:DEBUG>:${YACMA_CXX_FLAGS_DEBUG}>")
    target_compile_options(${arg1} PUBLIC "$<$<CONFIG:RELEASE>:${YACMA_CXX_FLAGS}>")
    if(YACMA_COMPILER_IS_MSVC)
        # Disable the idiotic minmax macros on MSVC, and some annoying warnings.
        target_compile_options(${arg1} PUBLIC "-DNOMINMAX" "/wd4459" "/wd4127")
    endif()
    set_property(TARGET ${arg1} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${arg1} PROPERTY CXX_STANDARD_REQUIRED YES)
    set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)
    add_test(${arg1} ${arg1})
endmacro()

ADD_MPPP_TESTCASE(test_arith)
ADD_MPPP_TESTCASE(test_basic)
ADD_MPPP_TESTCASE(test_rel)
