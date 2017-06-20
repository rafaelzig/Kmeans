cmake_minimum_required(VERSION 3.5)
project(Kmeans)

SET(CMAKE_CXX_FLAGS "-O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#set(SOURCE_FILES src/Kmeans_MPI.cpp src/Point.cpp src/Point.h src/Mapper.cpp src/Mapper.h src/Centroid.cpp src/Centroid.h)
set(SOURCE_FILES src/Kmeans_Sequential.cpp src/Point.cpp src/Point.h src/Mapper.cpp src/Mapper.h src/Centroid.cpp src/Centroid.h)
add_executable(Kmeans ${SOURCE_FILES})
target_link_libraries(Kmeans libmsmpi.a)