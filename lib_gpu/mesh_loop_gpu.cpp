#include "mesh_loop_gpu.h"

#define DJ_OPENGL_IMPLEMENTATION
#include "dj_opengl.h"

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
	halfedges_gpu	= create_buffer(BUFFER_HALFEDGES_IN	, Hd * sizeof(HalfEdge)	, halfedges.data()	) ;
	creases_gpu		= create_buffer(BUFFER_CREASES_IN	, Cd * sizeof(Crease)	, creases.data()	) ;
	vertices_gpu	= create_buffer(BUFFER_VERTICES_IN	, Vd * sizeof(vec3)		, vertices.data()	) ;
}

void
Mesh_Loop_GPU::release_buffers()
{
	release_buffer(halfedges_gpu) ;
	release_buffer(creases_gpu) ;
	release_buffer(vertices_gpu) ;
}

GLuint
Mesh_Loop_GPU::create_buffer(GLuint buffer_bind_id, uint size, void* data, bool clear_buffer)
{
	GLuint new_buffer ;

	glGenBuffers(1, &new_buffer) ;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_buffer) ;
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, data, GL_MAP_READ_BIT) ; // TODO remove READ_BIT ?
	if (clear_buffer)
		glClearNamedBufferData(new_buffer,GL_R32F,GL_RED,GL_FLOAT,nullptr) ;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer_bind_id, new_buffer) ; // allows to be read in shader

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
	// halfedges
	{
		HalfEdge* data = (HalfEdge*) glMapNamedBuffer(halfedges_gpu, GL_READ_ONLY) ;

		halfedges.resize(Hd) ;
		memcpy(&(halfedges[0]), data, Hd * sizeof(HalfEdge)) ;

		glUnmapNamedBuffer(halfedges_gpu) ;
	}

	// creases
	{
		Crease* data = (Crease*) glMapNamedBuffer(creases_gpu, GL_READ_ONLY) ;

		creases.resize(Cd) ;
		memcpy(&(creases[0]), data, Cd * sizeof(Crease)) ;

		glUnmapNamedBuffer(creases_gpu) ;
	}

	// vertices
	{
		vec3* data = (vec3*) glMapNamedBuffer(vertices_gpu, GL_READ_ONLY) ;

		vertices.resize(Vd) ;
		memcpy(&(vertices[0]), data, Vd * sizeof(vec3)) ;

		glUnmapNamedBuffer(vertices_gpu) ;
	}
}

GLuint
Mesh_Loop_GPU::create_program(const std::string& shader_file, GLuint in_buffer, GLuint out_buffer, bool is_vertex_program)
{
	GLuint program = glCreateProgram() ;

	djg_program* builder = djgp_create() ;
	djgp_push_string(builder,"#define BUFFER_IN %d\n", in_buffer) ;
	djgp_push_string(builder,"#define BUFFER_OUT %d\n", out_buffer) ;
	if (is_vertex_program)
	{
		djgp_push_string(builder, "#define HALFEDGE_BUFFER %d\n", BUFFER_HALFEDGES_IN) ;
		djgp_push_string(builder, "#define CREASE_BUFFER %d\n", BUFFER_CREASES_IN) ;
		djgp_push_string(builder, "#extension GL_NV_shader_atomic_float: require\n");
	}

	djgp_push_file(builder, shader_file.c_str()) ;
	djgp_to_gl(builder, 450, false, true, &program) ;

	djgp_release(builder) ;

	return program ;
}

void
Mesh_Loop_GPU::rebind_buffers() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, halfedges_gpu) ;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_HALFEDGES_IN, halfedges_gpu) ;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, creases_gpu) ;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_CREASES_IN, creases_gpu) ;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertices_gpu) ;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_VERTICES_IN, vertices_gpu) ;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0) ;
}

void
Mesh_Loop_GPU::refine_step_gpu()
{
	const uint new_depth = depth() + 1 ;

	// ensure inputs are bound as inputs
	if (depth() > 0)
	{
		rebind_buffers() ;
	}

	// create new buffers
	const GLuint H_new_gpu = create_buffer(BUFFER_HALFEDGES_OUT	, H(new_depth) * sizeof(HalfEdge)	, nullptr) ;
	const GLuint C_new_gpu = create_buffer(BUFFER_CREASES_OUT	, C(new_depth) * sizeof(Crease)		, nullptr) ;
	const GLuint V_new_gpu = create_buffer(BUFFER_VERTICES_OUT	, V(new_depth) * sizeof(vec3)		, nullptr, true) ;

	// create programs
	const GLuint refine_halfedges_gpu	= create_program("../shaders/loop_refine_halfedges.glsl",BUFFER_HALFEDGES_IN, BUFFER_HALFEDGES_OUT	) ;
	const GLuint refine_creases_gpu		= create_program("../shaders/refine_creases.glsl"		,BUFFER_CREASES_IN	, BUFFER_CREASES_OUT	) ;
	const GLuint refine_vertices_gpu	= create_program("../shaders/loop_refine_vertices.glsl"	,BUFFER_VERTICES_IN	, BUFFER_VERTICES_OUT,true) ;

	// execute halfedge refinement
	{
		glUseProgram(refine_halfedges_gpu) ;

		// set uniforms
		const GLint u_Hd = glGetUniformLocation(refine_halfedges_gpu, "Hd");
		const GLint u_Vd = glGetUniformLocation(refine_halfedges_gpu, "Vd");
		const GLint u_Ed = glGetUniformLocation(refine_halfedges_gpu, "Ed");
		glUniform1i(u_Hd, Hd) ;
		glUniform1i(u_Ed, Ed) ;
		glUniform1i(u_Vd, Vd) ;

		// execute program
		const uint n_dispatch_groups = std::ceil(Hd / 256.0f) ;
		glDispatchCompute(n_dispatch_groups,1,1) ;
		glMemoryBarrier(GL_ALL_BARRIER_BITS) ;
	}

	// execute crease refinement
	{
		glUseProgram(refine_creases_gpu) ;

		// set uniforms
		const GLint u_Cd = glGetUniformLocation(refine_creases_gpu, "Cd");
		glUniform1i(u_Cd, Cd) ;

		// execute program
		const uint n_dispatch_groups = std::ceil(Cd / 256.0f) ;
		glDispatchCompute(n_dispatch_groups,1,1) ;
		glMemoryBarrier(GL_ALL_BARRIER_BITS) ;
	}

	// execute vertex refinement
	{
		glUseProgram(refine_vertices_gpu) ;

		// set uniforms
		const GLint u_Hd = glGetUniformLocation(refine_vertices_gpu, "Hd");
		const GLint u_Vd = glGetUniformLocation(refine_vertices_gpu, "Vd");
		const GLint u_Ed = glGetUniformLocation(refine_vertices_gpu, "Ed");
		const GLint u_Cd = glGetUniformLocation(refine_vertices_gpu, "Cd");
		glUniform1i(u_Hd, Hd) ;
		glUniform1i(u_Ed, Ed) ;
		glUniform1i(u_Vd, Vd) ;
		glUniform1i(u_Cd, Cd) ;

		// execute program
		const uint n_dispatch_groups = std::ceil(Hd / 256.0f) ;
		glDispatchCompute(n_dispatch_groups,1,1) ;
		glMemoryBarrier(GL_ALL_BARRIER_BITS) ;
	}

    // cleanup
    glDeleteProgram(refine_halfedges_gpu) ;
	glDeleteProgram(refine_creases_gpu) ;
	glDeleteProgram(refine_vertices_gpu) ;
	release_buffer(halfedges_gpu) ;
	release_buffer(creases_gpu) ;
	release_buffer(vertices_gpu) ;

    // save new state
    set_depth(new_depth) ;
	halfedges_gpu = H_new_gpu ;
	creases_gpu = C_new_gpu ;
	vertices_gpu = V_new_gpu ;
    readback_buffers() ;
}
