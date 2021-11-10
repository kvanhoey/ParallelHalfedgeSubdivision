#ifndef __MESH_SUBDIV_GPU_H__
#define __MESH_SUBDIV_GPU_H__

#include "mesh_subdiv.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <cstring>

/**
 * @brief The Mesh_Subdiv_GPU (pure virtual) class specializes memory operations for the CPU, and implements OpenGL calls to the refinement routines that are defined in external GLSL shaders.
 */
class Mesh_Subdiv_GPU: virtual public Mesh_Subdiv
{
public:
	/**
	 * @brief Mesh_Subdiv_GPU constructor from OBJ file, and allocates OpenGL programs
	 * @param filename path to an OBJ file
	 * @param max_depth the depth at which to subdivide the mesh
	 */
	Mesh_Subdiv_GPU(const std::string& filename, uint max_depth) ;

	/**
	 * @brief ~Mesh_Subdiv_GPU destructor: free's memory on GPU device
	 */
	virtual ~Mesh_Subdiv_GPU() ;

protected:
	// ----------- Buffer management -----------
	/**
	 * @brief allocate_subdiv_buffers allocates and initializes the GPU buffers in which subdivision will be computed.
	 */
	void allocate_subdiv_buffers() final ;
	/**
	 * @brief readback_from_subdiv_buffers copies the result from the GPU subdivision buffers into the current buffer
	 */
	void readback_from_subdiv_buffers() final ;

	// ----------- Member functions that do the actual subdivision -----------
	/**
	 * @brief refine_creases operates crease refinement in the GPU crease subdivision buffers.
	 */
	void refine_creases() final;
	/**
	 * @brief refine_halfedges operates halfedge refinement in the GPU halfedge subdivision buffers.
	 */
	void refine_halfedges() final;
	/**
	 * @brief refine_vertices operates vertex refinement in the GPU vertex subdivision buffers.
	 */
	void refine_vertices() final;

//	void refine_halfedges_and_time(int n_repetitions) final;
	// ----------- Utility function for timing -----------
	virtual std::vector<double> measure_time(void (Mesh_Subdiv::*fptr)(), Mesh_Subdiv& c, int n_repetitions) final ;

	// ----------- Utility functions -----------
	/**
	 * @brief create_program is a static utility function to create OpenGL programs
	 * @param shader_file the GLSL compute shader to apply to the program
	 * @param in_buffer input buffer the shader will read from
	 * @param out_buffer output buffer the shader will write to
	 * @param is_vertex_program indicates if the program is created for vertex refinement (as opposed to crease or halfedge refinement).
	 * @return the program identifier.
	 */
	static GLuint create_program(const std::string& shader_file, GLuint in_buffer, GLuint out_buffer, bool is_vertex_program = false) ;

	/**
	 * @brief create_buffer is a static utility function to create an OpenGL SSBO
	 * @param buffer_bind_id the buffer binding identifier to bind to it
	 * @param size the size of the buffer
	 * @param data the data to initialize the buffer or nullptr
	 * @param clear_buffer (default false) should be true if the buffer should be initialized to 0
	 * @param enable_readback true if and only if a readback to CPU should be available for this buffer
	 * @return the buffer identifier.
	 */
	static GLuint create_buffer(GLuint buffer_bind_id, uint size, void* data, bool clear_buffer = false, bool enable_readback = true) ;

	/**
	 * @brief release_buffer is a static utility function to release SSBO memory
	 * @param buffer the identifier of the buffer to release
	 */
	static void release_buffer(GLuint buffer) ;

	// ----------- Ids for OpenGL shader storage buffer objects and program -----------
	GLuint halfedgecage_subdiv_buffer ;				/*!< identifier for the HalfEdge_cage subdiv buffer on GPU */
	std::vector<GLuint> halfedge_subdiv_buffers	;	/*!< identifier for the HalfEdge subdiv buffers (one per subdivision depth) on GPU */
	std::vector<GLuint> crease_subdiv_buffers	;	/*!< identifier for the Crease subdiv buffers (one per subdivision depth) on GPU */
	std::vector<GLuint> vertex_subdiv_buffers	;	/*!< identifier for the vec3 subdiv buffers (one per subdivision depth) on GPU */

	GLuint refine_halfedges_step_program ;				/*!< identifier for the HalfEdge refinement program */
	GLuint refine_creases_step_program ;				/*!< identifier for the Crease refinement program */
	std::vector<GLuint> refine_vertices_step_program ;	/*!< identifier for the vec3 refinement program */

	enum {
		BUFFER_HALFEDGESCAGE_IN=0,
		BUFFER_HALFEDGES_IN,
		BUFFER_HALFEDGES_OUT,
		BUFFER_CREASES_IN,
		BUFFER_CREASES_OUT,
		BUFFER_VERTICES_IN,
		BUFFER_VERTICES_OUT,
		BUFFER_COUNT
	};
};



#endif
