#include "mesh_loop_gpu.h"

Mesh_Loop_GPU::Mesh_Loop_GPU(int H, int V, int E, int F):
	Mesh_Loop(H,V,E,F)
{
	init_buffers() ;
}


Mesh_Loop_GPU::Mesh_Loop_GPU(const std::string& filename):
	Mesh_Loop(filename)
{
	init_buffers() ;
}

void
Mesh_Loop_GPU::init_buffers()
{
	const uint sizeof_halfedges = halfedges.size() * sizeof(HalfEdge) ;
	glGenBuffers(1, &halfedges_gpu) ;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, halfedges_gpu) ;
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof_halfedges, halfedges.data(), GL_MAP_READ_BIT) ; // TODO remove READ_BIT

//	glClearNamedBufferData(halfedges_gpu, //g_gl.buffers[BUFFER_SUBD_VERTEX_POINTS],
//						   GL_R32F,
//						   GL_RED,
//						   GL_FLOAT,
//						   nullptr);
//	    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(data), NULL, GL_MAP_READ_BIT) ;

//	    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, data_id, my_buffer) ; // allows to be read in shader


//	    glGenBuffers(1, &my_buffer_ro) ;
//	    glBindBuffer(GL_SHADER_STORAGE_BUFFER, my_buffer_ro) ;
//	    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(data), data, GL_MAP_READ_BIT) ;

//	    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, data_id + 1, my_buffer_ro) ; // allows to be read in shader



	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0) ;
}

void
Mesh_Loop_GPU::readback_buffer()
{
	HalfEdge* data = (HalfEdge*) glMapNamedBuffer(halfedges_gpu, GL_READ_ONLY) ;
	memcpy(&halfedges[0], data, halfedges.size()) ;

	glUnmapNamedBuffer(halfedges_gpu) ;
}
