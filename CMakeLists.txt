cmake_minimum_required(VERSION 3.8)

project (ParallelHalfedgeSubdivision CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Debugging off
set(CMAKE_CXX_FLAGS "-Os -march=native")
add_definitions(-DNDEBUG)
# Debugging on
#set(CMAKE_CXX_FLAGS "-g") # Debug

option(BUILD_DOC "Build documentation" OFF)

# disable GLFW docs, examples and tests
# see http://www.glfw.org/docs/latest/build_guide.html
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# set path to dependencies
add_subdirectory(submodules/glfw)

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

include_directories(lib/)
add_executable(loop_cpu loop_cpu.cpp lib/mesh.cpp lib/mesh_subdiv_cpu.cpp lib/mesh_subdiv.cpp lib/mesh_subdiv_loop.cpp lib/mesh_subdiv_loop_cpu.cpp)
add_executable(catmull-clark_cpu catmull-clark_cpu.cpp lib/mesh.cpp lib/mesh_subdiv_cpu.cpp lib/mesh_subdiv.cpp lib/mesh_subdiv_catmull-clark.cpp lib/mesh_subdiv_catmull-clark_cpu.cpp)
add_executable(stats stats.cpp lib/mesh.cpp)
add_executable(make_tikz_figure tikz.cpp lib/mesh.cpp lib/mesh_subdiv_cpu.cpp lib/mesh_subdiv.cpp lib/mesh_subdiv_loop.cpp lib/mesh_subdiv_loop_cpu.cpp)

file(GLOB lib_gpu lib/gpu_dependencies/*.cpp lib/gpu_dependencies/glad/glad.c)
file(GLOB loop_shaders shaders/*loop*.glsl shaders/*crease*.glsl)
file(GLOB catmull-clark_shaders shaders/*catmull*.glsl shaders/*crease*.glsl)
include_directories(lib/gpu_dependencies/)

add_executable(loop_gpu loop_gpu.cpp lib/mesh.cpp lib/mesh_subdiv.cpp lib/mesh_subdiv_loop.cpp lib/mesh_subdiv_gpu.cpp lib/mesh_subdiv_loop_gpu.cpp ${lib_gpu} ${loop_shaders})
target_link_libraries(loop_gpu glfw)

add_executable(catmull-clark_gpu catmull-clark_gpu.cpp lib/mesh.cpp lib/mesh_subdiv.cpp lib/mesh_subdiv_gpu.cpp lib/mesh_subdiv_catmull-clark.cpp lib/mesh_subdiv_catmull-clark_gpu.cpp ${lib_gpu} ${catmull-clark_shaders})
target_link_libraries(catmull-clark_gpu glfw)

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
		add_custom_target( doc ALL
			COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Generating API documentation with Doxygen"
			VERBATIM )
	endif (BUILD_DOC)
else (DOXYGEN_FOUND)
	message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
