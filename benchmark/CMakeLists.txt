# Nonius needs support for threading.
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
unset(THREADS_PREFER_PTHREAD_FLAG)

# Nonius also depends on Boost.
find_package(Boost REQUIRED)

macro(ADD_MPPP_BENCHMARK arg1)
    add_executable(${arg1} ${arg1}.cpp)
    target_link_libraries(${arg1} mp++ Threads::Threads Boost::boost)
    target_include_directories(${arg1} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
    set_property(TARGET ${arg1} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${arg1} PROPERTY CXX_STANDARD_REQUIRED YES)
    set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)
endmacro()

ADD_MPPP_BENCHMARK(bench_constructors)
ADD_MPPP_BENCHMARK(bench_conversions)
ADD_MPPP_BENCHMARK(bench_nbits)
