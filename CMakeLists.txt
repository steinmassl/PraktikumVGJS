cmake_minimum_required(VERSION 3.19)

project(PraktikumVGJS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(external/benchmark)

# Find gcc's built-in atomic shared library (libatomic.so)
FIND_LIBRARY(GCCLIBATOMIC_LIBRARY NAMES atomic atomic.so.1 libatomic.so.1
  HINTS
  $ENV{HOME}/local/lib64
  $ENV{HOME}/local/lib
  /usr/local/lib64
  /usr/local/lib
  /opt/local/lib64
  /opt/local/lib
  /usr/lib64
  /usr/lib
  /lib64
  /lib
)

IF (GCCLIBATOMIC_LIBRARY)
    SET(GCCLIBATOMIC_FOUND TRUE)
    MESSAGE(STATUS "Found GCC's libatomic.so: lib=${GCCLIBATOMIC_LIBRARY}")
ELSE ()
    SET(GCCLIBATOMIC_FOUND FALSE)
    MESSAGE(STATUS "GCC's libatomic.so not found. This library is essential in AArch64 (if you are on x86, this doesn't matter)")
    MESSAGE(STATUS "Try: 'sudo yum install libatomic' (or sudo apt-get install libatomic1)")
ENDIF ()



add_compile_options(-fcoroutines
                    -fconcepts
                    )

add_executable( main
                main.cpp
                workFunc.cpp
                workCoro.cpp
                mandelbrotFunc.cpp
                mandelbrotCoro.cpp
                mctsFunc.cpp
                mctsCoro.cpp
                Lock-free_queue_test.cpp
                )

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(main PRIVATE Threads::Threads
                           PRIVATE ${GCCLIBATOMIC_LIBRARY}
                           PRIVATE benchmark::benchmark
                           PRIVATE benchmark::benchmark_main
                           )

target_include_directories(main PRIVATE external/benchmark/include/benchmark)

