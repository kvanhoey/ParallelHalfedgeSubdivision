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
	halfedges_gpu	= create_buffer(BUFFER_HALFEDGES_IN	, Hd * sizeof(HalfEdge)	, halfedges.data()	,	false,	false) ;
	creases_gpu		= create_buffer(BUFFER_CREASES_IN	, Cd * sizeof(Crease)	, creases.data()	,	false,	false) ;
	vertices_gpu	= create_buffer(BUFFER_VERTICES_IN	, Vd * sizeof(vec3)		, vertices.data()	,	false,	false) ;
}

void
Mesh_Loop_GPU::release_buffers()
{
	release_buffer(halfedges_gpu) ;
	release_buffer(creases_gpu) ;
	release_buffer(vertices_gpu) ;
}

GLuint
Mesh_Loop_GPU::create_buffer(GLuint buffer_bind_id, uint size, void* data, bool clear_buffer, bool enable_readback)
{
	GLuint new_buffer ;

	glGenBuffers(1, &new_buffer) ;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_buffer) ;
	if (enable_readback)
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, data, GL_MAP_READ_BIT) ;
	else
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, data, 0) ;
	if (clear_buffer)
		glClearNamedBufferData(new_buffer,GL_R32F,GL_RED,GL_FLOAT,nullptr) ;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer_bind_id, new_buffer) ; // allows to be read in shader

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0) ;

	return new_buffer ;
}

void
Mesh_Loop_GPU::clear_buffer(GLuint buffer)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer) ;
    glClearNamedBufferData(buffer,GL_R32F,GL_RED,GL_FLOAT,nullptr) ;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0) ;
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_HALFEDGES_IN, halfedges_gpu) ;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_CREASES_IN, creases_gpu) ;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_VERTICES_IN, vertices_gpu) ;
}

void
Mesh_Loop_GPU::refine_step_gpu(bool readback_to_cpu)
{
	// ensure inputs are bound as inputs
	if (depth() > 0)
	{
		rebind_buffers() ;
	}
	const uint new_depth = depth() + 1 ;

	// create new buffers
	const GLuint H_new_gpu = create_buffer(BUFFER_HALFEDGES_OUT	, H(new_depth) * sizeof(HalfEdge)	, nullptr,	false,	readback_to_cpu) ;
	const GLuint C_new_gpu = create_buffer(BUFFER_CREASES_OUT	, C(new_depth) * sizeof(Crease)		, nullptr,	false,	readback_to_cpu) ;
	const GLuint V_new_gpu = create_buffer(BUFFER_VERTICES_OUT	, V(new_depth) * sizeof(vec3)		, nullptr,	true,	readback_to_cpu) ;

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

	glUseProgram(0) ;

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

	if (readback_to_cpu)
		readback_buffers() ;
}


Timings
Mesh_Loop_GPU::bench_refine_step_gpu(bool bench_halfedges, bool bench_creases, bool bench_clear, bool bench_vertices, uint repetitions, bool save_result, bool readback_to_cpu)
{
	duration min_he(0),max_he(0),sum_he(0),median_he(0) ;
	duration min_cr(0),max_cr(0),sum_cr(0),median_cr(0) ;
    duration min_clear(0),max_clear(0),sum_clear(0),median_clear(0) ;
    duration min_vx(0),max_vx(0),sum_vx(0),median_vx(0) ;
	djg_clock *clock = djgc_create();

	GLuint H_new_gpu, C_new_gpu, V_new_gpu ;
	const uint new_depth = depth() + 1 ;

	// ensure inputs are bound as inputs
	if (depth() > 0)
	{
		rebind_buffers() ;
	}

	// execute halfedge refinement
    if (bench_halfedges)
	{
		// create new buffer
		H_new_gpu = create_buffer(BUFFER_HALFEDGES_OUT	, H(new_depth) * sizeof(HalfEdge)	, nullptr, false, save_result) ;

		// create program
		const GLuint refine_halfedges_gpu	= create_program("../shaders/loop_refine_halfedges.glsl",BUFFER_HALFEDGES_IN, BUFFER_HALFEDGES_OUT	) ;

		std::vector<duration> timings_he ;

		for (uint i = 0 ; i < repetitions; ++i)
		{
			double cpuTime = 0.0, gpuTime = 0.0;

			glFinish();
			djgc_start(clock);
			{
                // set uniforms
                const GLint u_Hd = glGetUniformLocation(refine_halfedges_gpu, "Hd");
                const GLint u_Vd = glGetUniformLocation(refine_halfedges_gpu, "Vd");
                const GLint u_Ed = glGetUniformLocation(refine_halfedges_gpu, "Ed");

                glUseProgram(refine_halfedges_gpu) ;
                glUniform1i(u_Hd, Hd) ;
                glUniform1i(u_Ed, Ed) ;
                glUniform1i(u_Vd, Vd) ;

				// execute program
				const uint n_dispatch_groups = std::ceil(Hd / 256.0f) ;
				glDispatchCompute(n_dispatch_groups,1,1) ;
				glMemoryBarrier(GL_ALL_BARRIER_BITS) ;
			}
			djgc_stop(clock);
			glFinish();
			djgc_ticks(clock, &cpuTime, &gpuTime);

			duration elapsed(gpuTime * 1e3) ;
			timings_he.push_back(elapsed) ;
		}
		// cleanup
		glDeleteProgram(refine_halfedges_gpu) ;

		assert(timings_he.size() == repetitions) ;

		std::sort(timings_he.begin(), timings_he.end()) ;
		median_he = timings_he[repetitions / 2] ;
		min_he = timings_he.front() ;
		max_he = timings_he.back() ;
		for (int i = 0 ; i < timings_he.size() ; ++i)
		{
			sum_he += timings_he[i] ;
		}
		//sum_he = std::accumulate(timings_he.begin(), timings_he.end(), duration(0)) ;
	}

	// execute crease refinement
    if (bench_creases)
	{
		// create new buffer
		C_new_gpu = create_buffer(BUFFER_CREASES_OUT	, C(new_depth) * sizeof(Crease)		, nullptr, false, save_result) ;

		// create program
		const GLuint refine_creases_gpu		= create_program("../shaders/refine_creases.glsl"		,BUFFER_CREASES_IN	, BUFFER_CREASES_OUT	) ;

		std::vector<duration> timings_cr ;

		for (uint i = 0 ; i < repetitions; ++i)
		{
			double cpuTime = 0.0, gpuTime = 0.0;

			glFinish();
			djgc_start(clock);
			{
				// set uniforms
				const GLint u_Cd = glGetUniformLocation(refine_creases_gpu, "Cd");

				glUseProgram(refine_creases_gpu) ;
				glUniform1i(u_Cd, Cd) ;

				// execute program
				const uint n_dispatch_groups = std::ceil(Cd / 256.0f) ;
				glDispatchCompute(n_dispatch_groups,1,1) ;
				glMemoryBarrier(GL_ALL_BARRIER_BITS) ;
			}
			djgc_stop(clock);
			glFinish();
			djgc_ticks(clock, &cpuTime, &gpuTime);

			duration elapsed(gpuTime * 1e3) ;
			timings_cr.push_back(elapsed) ;
		}
		// cleanup
		glDeleteProgram(refine_creases_gpu) ;

		assert(timings_cr.size() == repetitions) ;

		std::sort(timings_cr.begin(), timings_cr.end()) ;
		median_cr = timings_cr[repetitions / 2] ;
		min_cr = timings_cr.front() ;
		max_cr = timings_cr.back() ;
//		sum_cr = std::accumulate(timings_cr.begin(), timings_cr.end(), duration(0)) ;
		for (int i = 0 ; i < timings_cr.size() ; ++i)
		{
			sum_cr += timings_cr[i] ;
		}
	}

	// execute vertex refinement
    if (bench_clear || bench_vertices)
	{
		// create new buffer
		V_new_gpu = create_buffer(BUFFER_VERTICES_OUT	, V(new_depth) * sizeof(vec3)		, nullptr, false, save_result) ;

		// create program
		const GLuint refine_vertices_gpu	= create_program("../shaders/loop_refine_vertices.glsl"	,BUFFER_VERTICES_IN	, BUFFER_VERTICES_OUT,true) ;

        std::vector<duration> timings_clear, timings_vx ;

		for (uint i = 0 ; i < repetitions; ++i)
		{
			double cpuTime = 0.0, gpuTime = 0.0;

			glFinish();
			djgc_start(clock);
			{
				// reset buffer to 0
				clear_buffer(V_new_gpu) ;
			}
			djgc_stop(clock);
			glFinish();
			djgc_ticks(clock, &cpuTime, &gpuTime);

			duration elapsed(1e3 * gpuTime) ;
            timings_clear.push_back(elapsed) ;

            glFinish();
            djgc_start(clock);
            {
                const GLint u_Hd = glGetUniformLocation(refine_vertices_gpu, "Hd");
                const GLint u_Vd = glGetUniformLocation(refine_vertices_gpu, "Vd");
                const GLint u_Ed = glGetUniformLocation(refine_vertices_gpu, "Ed");
                const GLint u_Cd = glGetUniformLocation(refine_vertices_gpu, "Cd");

                glUseProgram(refine_vertices_gpu) ;

                // set uniforms
                glUniform1i(u_Hd, Hd) ;
                glUniform1i(u_Ed, Ed) ;
                glUniform1i(u_Vd, Vd) ;
                glUniform1i(u_Cd, Cd) ;

                // execute program
                const uint n_dispatch_groups = std::ceil(Hd / 256.0f) ;
                glDispatchCompute(n_dispatch_groups,1,1) ;
                glMemoryBarrier(GL_ALL_BARRIER_BITS) ;
            }
            djgc_stop(clock);
            glFinish();
            djgc_ticks(clock, &cpuTime, &gpuTime);

            elapsed = duration(1e3 * gpuTime) ;
            timings_vx.push_back(elapsed) ;
		}
		// cleanup
		glDeleteProgram(refine_vertices_gpu) ;

        if (bench_clear)
        {
            assert(timings_clear.size() == repetitions) ;
            std::sort(timings_clear.begin(), timings_clear.end()) ;
            median_clear = timings_clear[repetitions / 2] ;
            min_clear = timings_clear.front() ;
            max_clear = timings_clear.back() ;
//            sum_clear = std::accumulate(timings_clear.begin(), timings_clear.end(), duration(0)) ;
			for (int i = 0 ; i < timings_clear.size() ; ++i)
			{
				sum_clear += timings_clear[i] ;
			}
        }

        if (bench_vertices)
        {
            assert(timings_vx.size() == repetitions) ;
            std::sort(timings_vx.begin(), timings_vx.end()) ;
            median_vx = timings_vx[repetitions / 2] ;
            min_vx = timings_vx.front() ;
            max_vx = timings_vx.back() ;
//            sum_vx = std::accumulate(timings_vx.begin(), timings_vx.end(), duration(0)) ;
			for (int i = 0 ; i < timings_vx.size() ; ++i)
			{
				sum_vx += timings_vx[i] ;
			}
        }
	}

	glUseProgram(0) ;

	// save new state
    if (bench_halfedges)
	{
		if (save_result)
		{
			release_buffer(halfedges_gpu) ;
			halfedges_gpu = H_new_gpu ;
		}
		else
			release_buffer(H_new_gpu) ;
	}

    if (bench_creases)
	{
		if(save_result)
		{
			release_buffer(creases_gpu) ;
			creases_gpu = C_new_gpu ;
		}
		else
		{
			release_buffer(C_new_gpu) ;
		}
	}

    if (bench_clear || bench_vertices)
	{
		if (save_result)
		{
			release_buffer(vertices_gpu) ;
			vertices_gpu = V_new_gpu ;
		}
		else
		{
			release_buffer(V_new_gpu) ;
		}
	}

    if (save_result && bench_halfedges && bench_creases && bench_clear && bench_vertices)
	{
		set_depth(new_depth) ;
	}

	if (readback_to_cpu)
		readback_buffers() ;

    duration min_time = min_he + min_cr + min_clear + min_vx ;
    duration max_time = max_he + max_cr + max_clear + max_vx ;
    duration sum_time = sum_he + sum_cr + sum_clear + sum_vx ;
    duration median_time = median_he + median_cr + median_clear + median_vx ;

	Timings t ;
	t.min = min_time.count() ;
	t.max = max_time.count() ;
	t.mean = sum_time.count() / repetitions ;
	t.median = median_time.count() ;

	return t ;

}
