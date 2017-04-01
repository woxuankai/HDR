cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS "-std=c++11 -march=native ${CMAKE_CXX_FLAGS}")

project( project_hdr )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_library( hdr hdr.cxx )
add_library( thread_functions thread_functions.cxx)

add_executable( dohdr main.cpp )
set(LINK_LIBS thread_functions hdr ${OpenCV_LIBS} pthread)
target_link_libraries( dohdr  ${LINK_LIBS})
