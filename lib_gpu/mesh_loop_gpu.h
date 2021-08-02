#ifndef __MESH_LOOP_GPU_H__
#define __MESH_LOOP_GPU_H__

#include "mesh.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

enum {
	BUFFER_HALFEDGES_IN,
	BUFFER_HALFEDGES_OUT,
	BUFFER_CREASES_IN,
	BUFFER_CREASES_OUT,
	BUFFER_VERTICES_IN,
	BUFFER_VERTICES_OUT,

	BUFFER_COUNT
};

class Mesh_Loop_GPU: public Mesh_Loop
{
public:
	Mesh_Loop_GPU(int H, int V, int E, int F) ;
	Mesh_Loop_GPU(const std::string& filename) ;
	~Mesh_Loop_GPU() ;

	void refine_step_gpu() ;

private:
	GLuint halfedges_gpu, creases_gpu, vertices_gpu ;

	void rebind_buffers() const ;

	void init_buffers() ;
	void readback_buffers() ;
	void release_buffers() ;

	static GLuint create_buffer(GLuint buffer_bind_id, uint size, void* data, bool clear_buffer = false, bool enable_readback = true) ;
	static void release_buffer(GLuint buffer) ;
	static void clear_buffer(GLuint buffer) ;

	static GLuint create_program(const std::string& shader_file, GLuint in_buffer, GLuint out_buffer, bool is_vertex_program = false) ;

public:
	virtual Timings bench_refine_step_gpu(bool refine_he, bool refine_cr, bool refine_vx, uint repetitions, bool save_result=false) final ;
};

#endif
