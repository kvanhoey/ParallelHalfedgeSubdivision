#ifndef __MESH_SUBDIV_GPU_H__
#define __MESH_SUBDIV_GPU_H__

#include "mesh_subdiv.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <cstring>

class Mesh_Subdiv_GPU: virtual public Mesh_Subdiv
{
public:
	Mesh_Subdiv_GPU(const std::string& filename, uint max_depth) ;
	virtual ~Mesh_Subdiv_GPU() ;

protected:
	// ----------- Buffer management -----------
	void allocate_subdiv_buffers() final ;
	void release_buffers(GLuint buffer) ;
	void readback_from_subdiv_buffers() final ;

	// ----------- Member functions that do the actual subdivision -----------
	void refine_creases() ;
	void refine_halfedges() ;
	void refine_vertices() ;

	// ----------- Utility functions -----------
	static GLuint create_program(const std::string& shader_file, GLuint in_buffer, GLuint out_buffer, bool is_vertex_program = false) ;
	static GLuint create_buffer(GLuint buffer_bind_id, uint size, void* data, bool clear_buffer = false, bool enable_readback = true) ;
	static void release_buffer(GLuint buffer) ;

	// ----------- Ids for OpenGL shader storage buffer objects and program -----------
	GLuint halfedgecage_subdiv_buffer ;
	std::vector<GLuint> halfedge_subdiv_buffers	;
	std::vector<GLuint> crease_subdiv_buffers	;
	std::vector<GLuint> vertex_subdiv_buffers	;

	GLuint refine_halfedges_step_program ;
	GLuint refine_creases_step_program ;
	std::vector<GLuint> refine_vertices_step_program ;

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
