#include "mesh.h"

Mesh_Subdiv_GPU::~Mesh_Subdiv_GPU()
{
	for (uint d = 0 ; d <= D ; ++d)
	{
		release_buffer(halfedge_subdiv_buffers[d]) ;
		release_buffer(crease_subdiv_buffers[d]) ;
		release_buffer(vertex_subdiv_buffers[d]) ;
	}
}

void
Mesh_Subdiv_GPU::allocate_subdiv_buffers()
{
	halfedge_subdiv_buffers.resize(D + 1) ;
	crease_subdiv_buffers.resize(D + 1) ;
	vertex_subdiv_buffers.resize(D + 1) ;


	uint d = 0 ;
	bool enable_readback = d == D ;
	halfedge_subdiv_buffers[d]	= create_buffer(BUFFER_HALFEDGES_IN	, H(d) * sizeof(HalfEdge)	, halfedges.data()	,	false,	enable_readback) ;
	crease_subdiv_buffers[d]	= create_buffer(BUFFER_CREASES_IN	, C(d) * sizeof(Crease)		, creases.data()	,	false,	enable_readback) ;
	vertex_subdiv_buffers[d]	= create_buffer(BUFFER_VERTICES_IN	, V(d) * sizeof(vec3)		, vertices.data()	,	false,	enable_readback) ;

	for (d = 1 ; d <= D ; ++d)
	{
		bool enable_readback = d == D ;

		halfedge_subdiv_buffers[d]	= create_buffer(BUFFER_HALFEDGES_IN	, H(d) * sizeof(HalfEdge)	, nullptr,	false,	enable_readback) ;
		crease_subdiv_buffers[d]	= create_buffer(BUFFER_CREASES_IN	, C(d) * sizeof(Crease)		, nullptr,	false,	enable_readback) ;
		vertex_subdiv_buffers[d]	= create_buffer(BUFFER_VERTICES_IN	, V(d) * sizeof(vec3)		, nullptr,	true,	enable_readback) ;
	}
}

void
Mesh_Subdiv_GPU::readback_from_subdiv_buffers()
{
	const uint Hd = H(D) ;
	const uint Cd = C(D) ;
	const uint Vd = V(D) ;

	// halfedges
	{
		HalfEdge* data = (HalfEdge*) glMapNamedBuffer(halfedge_subdiv_buffers[D], GL_READ_ONLY) ;

		halfedges.resize(Hd) ;
		memcpy(&(halfedges[0]), data, Hd * sizeof(HalfEdge)) ;

		glUnmapNamedBuffer(halfedge_subdiv_buffers[D]) ;
	}

	// creases
	{
		Crease* data = (Crease*) glMapNamedBuffer(crease_subdiv_buffers[D], GL_READ_ONLY) ;

		creases.resize(Cd) ;
		memcpy(&(creases[0]), data, Cd * sizeof(Crease)) ;

		glUnmapNamedBuffer(crease_subdiv_buffers[D]) ;
	}

	// vertices
	{
		vec3* data = (vec3*) glMapNamedBuffer(vertex_subdiv_buffers[D], GL_READ_ONLY) ;

		vertices.resize(Vd) ;
		memcpy(&(vertices[0]), data, Vd * sizeof(vec3)) ;

		glUnmapNamedBuffer(vertex_subdiv_buffers[D]) ;
	}
}

void
Mesh_Subdiv_GPU::refine_creases()
{
	for (uint d = 0 ; d < D; ++d)
	{
		assert(false) ;
	}
}

GLuint
Mesh_Subdiv_GPU::create_buffer(GLuint buffer_bind_id, uint size, void *data, bool clear_buffer, bool enable_readback)
{
	GLuint new_buffer ;

	glGenBuffers(1, &new_buffer) ;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_buffer) ;
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, data, enable_readback ? GL_MAP_READ_BIT : 0) ;
	if (clear_buffer)
		glClearNamedBufferData(new_buffer,GL_R32F,GL_RED,GL_FLOAT,nullptr) ;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer_bind_id, new_buffer) ;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0) ;

	return new_buffer ;
}


void
Mesh_Subdiv_GPU::release_buffer(GLuint buffer_id)
{
	glDeleteBuffers(1, &buffer_id) ;
}
