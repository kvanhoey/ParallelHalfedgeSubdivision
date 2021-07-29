#include "mesh_loop_gpu.h"

#define DJ_OPENGL_IMPLEMENTATION
#include "dj_opengl.h"

#include "gpu_debug_logger.h"

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

Mesh_Loop_GPU::~Mesh_Loop_GPU()
{
	release_buffers() ;
}

void
Mesh_Loop_GPU::init_buffers()
{
	halfedges_gpu = create_buffer(Hd,halfedges.data(),false) ;
}

void
Mesh_Loop_GPU::release_buffers()
{
	release_buffer(halfedges_gpu) ;
}

GLuint
Mesh_Loop_GPU::create_buffer(uint size, void* data, bool clear_buffer)
{
	GLuint new_buffer ;

	glGenBuffers(1, &new_buffer) ;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_buffer) ;
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, data, GL_MAP_READ_BIT) ; // TODO remove READ_BIT ?
	if (clear_buffer)
		glClearNamedBufferData(new_buffer,GL_R32F,GL_RED,GL_FLOAT,nullptr) ;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0) ;

	return new_buffer ;
}

void
Mesh_Loop_GPU::release_buffer(GLuint buffer)
{
	glDeleteBuffers(1, &buffer) ;
}



void
Mesh_Loop_GPU::readback_buffers()
{
	HalfEdge* data = (HalfEdge*) glMapNamedBuffer(halfedges_gpu, GL_READ_ONLY) ;

	halfedges.resize(Hd) ;
	memcpy(&halfedges[0], data, Hd) ;

	glUnmapNamedBuffer(halfedges_gpu) ;
}

void
Mesh_Loop_GPU::create_programs()
{
	assert(false) ;
}

GLuint
Mesh_Loop_GPU::create_program_refine_halfedges(GLuint halfedges_gpu_in, GLuint halfedges_gpu_out)
{
	GLuint refine_halfedges_program = glCreateProgram() ;

	djg_program* builder = djgp_create() ;
	djgp_push_string(builder,"#define HALFEDGE_BUFFER_IN %d\n", halfedges_gpu_in) ;
	djgp_push_string(builder,"#define HALFEDGE_BUFFER_OUT %d\n", halfedges_gpu_out) ;
	djgp_push_string(builder, "#extension GL_NV_shader_atomic_float: require\n");

	djgp_push_file(builder, "../shaders/loop_refine_halfedges.glsl") ;
	djgp_to_gl(builder, 450, false, true, &refine_halfedges_program) ;

	djgp_release(builder) ;

	return refine_halfedges_program ;
}

void
Mesh_Loop_GPU::delete_programs()
{
	assert(false);
	//glDeleteProgram(refine_halfedges_gpu) ;
}

void
Mesh_Loop_GPU::refine_step_gpu()
{
	const uint new_depth = depth() ; // todo reset + 1 ;
	GLuint H_new_gpu = create_buffer(H(new_depth), nullptr, false) ;

	for (uint k = 0; k < halfedges.size() ; ++k)
	{
		halfedges[k] = {0,0,0} ;
	}

	GLuint refine_halfedges_gpu = create_program_refine_halfedges(halfedges_gpu, H_new_gpu) ;
	const int u_Hd = glGetUniformLocation(refine_halfedges_gpu, "Hd");
	glUseProgram(refine_halfedges_gpu) ;
	glUniform1i(u_Hd, Hd) ;
	uint n_dispatch_groups = std::ceil(Hd / 256.0f) ;
	std::cout << "Dispating " << n_dispatch_groups << " threads of 256" << std::endl ;
	glDispatchCompute(n_dispatch_groups,1,1) ;

	glMemoryBarrier(GL_ALL_BARRIER_BITS) ;
	glDeleteProgram(refine_halfedges_gpu) ;

	release_buffer(halfedges_gpu) ;
	halfedges_gpu = H_new_gpu ;

	set_depth(new_depth) ;

	readback_buffers() ;
}
