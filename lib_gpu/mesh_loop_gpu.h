#ifndef __MESH_LOOP_GPU_H__
#define __MESH_LOOP_GPU_H__

#include "mesh.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

class Mesh_Loop_GPU: public Mesh_Loop
{
public:
	Mesh_Loop_GPU(int H, int V, int E, int F) ;
	Mesh_Loop_GPU(const std::string& filename) ;
	~Mesh_Loop_GPU() ;

	void refine_step_gpu() ;

private:
    GLuint halfedges_gpu ; //, creases_gpu, vertices_gpu ;
	enum {
		BUFFER_HALFEDGES_IN,
		BUFFER_HALFEDGES_OUT,

		BUFFER_COUNT
	};
	void rebind_buffers() const ;

	void init_buffers() ;
	void readback_buffers() ;
	void release_buffers() ;

	static GLuint create_buffer(GLuint buffer_bind_id, uint size, void* data, bool clear_buffer) ;
	static void release_buffer(GLuint buffer) ;

	static GLuint create_program_refine_halfedges(GLuint halfedges_gpu_in, GLuint halfedges_gpu_out) ;
};

#endif
