#include "mesh_subdiv_gpu.h"

#define DJ_OPENGL_IMPLEMENTATION
#include "dj_opengl.h"

Mesh_Subdiv_GPU::Mesh_Subdiv_GPU(const std::string &filename, uint max_depth):
	Mesh_Subdiv(filename,max_depth)
{
	refine_creases_step_program = create_program("../shaders/refine_creases.glsl", BUFFER_CREASES_IN, BUFFER_CREASES_OUT) ;
}

Mesh_Subdiv_GPU::~Mesh_Subdiv_GPU()
{
	glDeleteProgram(refine_halfedges_step_program) ;
	glDeleteProgram(refine_creases_step_program) ;
	for (uint i = 0 ; i < refine_vertices_step_program.size() ; ++i)
		glDeleteProgram(refine_vertices_step_program[i]) ;

	release_buffer(halfedgecage_subdiv_buffer) ;
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
	halfedgecage_subdiv_buffer = create_buffer(BUFFER_HALFEDGESCAGE_IN, H(d) * sizeof(HalfEdge_cage), halfedges_cage.data(), false, false) ;
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

GLuint
Mesh_Subdiv_GPU::create_program(const std::string &shader_file, GLuint in_buffer, GLuint out_buffer, bool is_vertex_program)
{
	GLuint program = glCreateProgram() ;

	djg_program* builder = djgp_create() ;
	djgp_push_string(builder,"#define CAGE_BUFFER %d\n", BUFFER_HALFEDGESCAGE_IN) ;
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
Mesh_Subdiv_GPU::refine_halfedges()
{
	// bind cage buffer once and for all
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_HALFEDGESCAGE_IN,	halfedgecage_subdiv_buffer) ;

	for (uint d = 0 ; d < D; ++d)
	{
		const uint Hd = H(d) ;
		const uint Vd = V(d) ;
		const uint Ed = E(d) ;
		const uint Fd = F(d) ;

		// bind input and output buffers
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_HALFEDGES_IN,	halfedge_subdiv_buffers[d]) ;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_HALFEDGES_OUT,halfedge_subdiv_buffers[d+1]) ;

		glUseProgram(refine_halfedges_step_program) ;

		// set uniforms
		const GLint u_d = glGetUniformLocation(refine_halfedges_step_program, "d");
		const GLint u_Hd = glGetUniformLocation(refine_halfedges_step_program, "Hd");
		const GLint u_Vd = glGetUniformLocation(refine_halfedges_step_program, "Vd");
		const GLint u_Ed = glGetUniformLocation(refine_halfedges_step_program, "Ed");
		const GLint u_Fd = glGetUniformLocation(refine_halfedges_step_program, "Fd");
		glUniform1i(u_d, d) ;
		glUniform1i(u_Hd, Hd) ;
		glUniform1i(u_Ed, Ed) ;
		glUniform1i(u_Vd, Vd) ;
		glUniform1i(u_Fd, Fd) ;

		// execute program
		const uint n_dispatch_groups = std::ceil(Hd / 256.0f) ;
		glDispatchCompute(n_dispatch_groups,1,1) ;
		glMemoryBarrier(GL_ALL_BARRIER_BITS) ;
	}
}

void
Mesh_Subdiv_GPU::refine_creases()
{
	for (uint d = 0 ; d < D; ++d)
	{
		const uint Cd = C(d) ;
		// bind input and output buffers
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_CREASES_IN,	crease_subdiv_buffers[d]) ;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_CREASES_OUT,	crease_subdiv_buffers[d+1]) ;

		glUseProgram(refine_creases_step_program) ;

		// set uniforms
		const GLint u_Cd = glGetUniformLocation(refine_creases_step_program, "Cd");
		glUniform1i(u_Cd, Cd) ;

		// execute program
		const uint n_dispatch_groups = std::ceil(Cd / 256.0f) ;
		glDispatchCompute(n_dispatch_groups,1,1) ;
		glMemoryBarrier(GL_ALL_BARRIER_BITS) ;
	}
}

void
Mesh_Subdiv_GPU::refine_vertices()
{
	// bind cage buffer once and for all
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_HALFEDGESCAGE_IN,	halfedgecage_subdiv_buffer) ;

	for (uint d = 0 ; d < D; ++d)
	{
		const uint Hd = H(d) ;
		const uint Ed = E(d) ;
		const uint Vd = V(d) ;
		const uint Cd = C(d) ;
		const uint Fd = F(d) ;

		// bind input and output buffers
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_HALFEDGES_IN,	halfedge_subdiv_buffers[d]) ;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_CREASES_IN,	crease_subdiv_buffers[d]) ;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_VERTICES_IN,	vertex_subdiv_buffers[d]) ;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_VERTICES_OUT,	vertex_subdiv_buffers[d+1]) ;

		for(const auto& refine_vertices_program_stage: refine_vertices_step_program)
		{
			glUseProgram(refine_vertices_program_stage) ;

			// set uniforms
			const GLint u_d = glGetUniformLocation(refine_vertices_program_stage, "d");
			const GLint u_Hd = glGetUniformLocation(refine_vertices_program_stage, "Hd");
			const GLint u_Vd = glGetUniformLocation(refine_vertices_program_stage, "Vd");
			const GLint u_Ed = glGetUniformLocation(refine_vertices_program_stage, "Ed");
			const GLint u_Fd = glGetUniformLocation(refine_vertices_program_stage, "Fd");
			const GLint u_Cd = glGetUniformLocation(refine_vertices_program_stage, "Cd");
			glUniform1i(u_d, d) ;
			glUniform1i(u_Hd, Hd) ;
			glUniform1i(u_Ed, Ed) ;
			glUniform1i(u_Vd, Vd) ;
			glUniform1i(u_Cd, Cd) ;
			glUniform1i(u_Fd, Fd) ;

			// execute program
			const uint n_dispatch_groups = std::ceil(Hd / 256.0f) ;
			glDispatchCompute(n_dispatch_groups,1,1) ;
			glMemoryBarrier(GL_ALL_BARRIER_BITS) ;
		}
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
