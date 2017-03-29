cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11 -march=native ${CMAKE_CXX_FLAGS}")

project( project_hdr )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_library( hdr hdr.cxx )
add_executable( dohdr main.cpp )
target_link_libraries( dohdr ${OpenCV_LIBS} hdr pthread tbb)
