#ifndef __MESH_SUBDIV_GPU_H__
#define __MESH_SUBDIV_GPU_H__

#include "mesh_subdiv.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <cstring>

enum {
	BUFFER_HALFEDGESCAGE_IN,
	BUFFER_HALFEDGES_IN,
	BUFFER_HALFEDGES_OUT,
	BUFFER_CREASES_IN,
	BUFFER_CREASES_OUT,
	BUFFER_VERTICES_IN,
	BUFFER_VERTICES_OUT,

	BUFFER_COUNT
};

class Mesh_Subdiv_GPU: virtual public Mesh_Subdiv
{
public:
	Mesh_Subdiv_GPU(const std::string& filename, uint max_depth) ;

	virtual ~Mesh_Subdiv_GPU() ;

protected:
	void allocate_subdiv_buffers() final ;
	void readback_from_subdiv_buffers() final ;
	void refine_creases() ;
	void refine_halfedges() ;
	void refine_vertices() ;

	static GLuint create_buffer(GLuint buffer_bind_id, uint size, void* data, bool clear_buffer = false, bool enable_readback = true) ;
	static void release_buffer(GLuint buffer) ;
	void release_buffers(GLuint buffer) ;

	static GLuint create_program(const std::string& shader_file, GLuint in_buffer, GLuint out_buffer, bool is_vertex_program = false) ;

	GLuint halfedgecage_subdiv_buffer ;
	std::vector<GLuint> halfedge_subdiv_buffers	;
	std::vector<GLuint> crease_subdiv_buffers	;
	std::vector<GLuint> vertex_subdiv_buffers	;

	GLuint refine_halfedges_step_program ;
	GLuint refine_creases_step_program ;
	GLuint refine_vertices_step_program ;

private:
	void create_crease_program() ;
};



#endif
