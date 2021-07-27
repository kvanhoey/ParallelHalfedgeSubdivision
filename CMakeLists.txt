cmake_minimum_required(VERSION 3.10)

project (ParallelHalfedgeSubdivision CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Os -march=native")
#set(CMAKE_CXX_FLAGS "-g") # Debug

option(BUILD_DOC "Build documentation" OFF)

# disable GLFW docs, examples and tests
# see http://www.glfw.org/docs/latest/build_guide.html
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# set path to dependencies
add_subdirectory(submodules/glfw)
include_directories(submodules/dj_opengl)

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

file(GLOB dependency_files lib/*.cpp)

include_directories(lib/)

add_executable(main_cc main_cc.cpp ${dependency_files})
add_executable(main_loop main_loop.cpp ${dependency_files})

add_executable(bench_cc bench_cc.cpp ${dependency_files})
add_executable(bench_loop bench_loop.cpp ${dependency_files})

add_executable(stats stats.cpp ${dependency_files})

add_executable(loop_gpu loop_gpu.cpp ${dependency_files} lib/glad/glad.c)
target_link_libraries(loop_gpu glfw)


# ------------------------------------------------------------------------------
##set(SRC_DIR demo-all)
#include_directories(${SRC_DIR})
#add_executable(demo-all ${SRC_DIR}/demo-all.cpp ${LIB_DIR}/glad/glad.c)
#target_link_libraries(demo-all glfw)

# ------------------------------------------------------------------------------

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
	if (BUILD_DOC)
	    # set input and output files
	    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
	    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

	    # request to configure the file
	    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	    message("Doxygen build started")

	    # note the option ALL which allows to build the docs together with the application
	    add_custom_target( doc_doxygen ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM )
        endif (BUILD_DOC)
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
