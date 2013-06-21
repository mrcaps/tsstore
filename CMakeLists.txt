cmake_minimum_required(VERSION 2.6)

file(GLOB Sources_CPP "src/*.cpp")
file(GLOB Sources_HPP "src/*.hpp")
file(GLOB Sources_Tests "test/*.cpp")
add_executable(bin/tss ${Sources_Tests} ${Sources_CPP} ${Sources_HPP})

include(CheckIncludeFile)
include(CheckIncludeFiles)

find_package(Threads REQUIRED)
if(THREADS_FOUND)
  target_link_libraries(bin/tss ${CMAKE_THREAD_LIBS_INIT})
endif(THREADS_FOUND)

#on Windows, place in C:\Program Files\zlib
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories( ${ZLIB_INCLUDE_DIRS} )
    target_link_libraries( bin/tss ${ZLIB_LIBRARIES} )
endif(ZLIB_FOUND)

set(BOOST_ROOT /cygdrive/c/SDK/boost_1_53_0)
find_package(Boost COMPONENTS unit_test_framework system filesystem REQUIRED)
include_directories(bin/tss ${Boost_INCLUDE_DIR} )
target_link_libraries(bin/tss ${Boost_LIBRARIES})
set(liblist boost_unit_test_framework)
