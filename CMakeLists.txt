cmake_minimum_required(VERSION 3.10)

project (HE_parallelCC CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_CXX_FLAGS "-Os -march=native")
set(CMAKE_CXX_FLAGS "-g") # Debug

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(main_cc main_cc.cpp mesh_cc.cpp mesh_subdiv.cpp mesh.cpp halfedge.cpp crease.cpp)
add_executable(main_loop main_loop.cpp mesh_loop.cpp mesh_subdiv.cpp mesh.cpp halfedge.cpp crease.cpp)

