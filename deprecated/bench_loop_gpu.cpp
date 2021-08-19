#include <iostream>

#define MAX_VERTICES pow(2,28)
#define N_REPETITIONS 50

#include "mesh_loop_gpu.h"
#ifndef NDEBUG
	#include "gpu_debug_logger.h"
#endif

int main(int argc, char **argv)
{
	// Fetch arguments
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " <filename>.obj <depth> <export_mesh=0> <nb_repetitions=" << N_REPETITIONS << ">" << std::endl ;
		return 0 ;
	}

	const std::string f_name(argv[1]) ;
	const uint D = atoi(argv[2]) ;
	const bool enable_export = argc < 4 ? false : atoi(argv[3]) ;
	const uint runCount = argc == 5 ? atoi(argv[4]) : N_REPETITIONS ;

	// Init GL
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Hello OpenGL Window", nullptr, nullptr);
	if (window == nullptr)
	{
		printf("window creation failed!\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("window creation failed!\n");
		return -1;
	}

#ifndef NDEBUG
	gpu_log_debug_output() ;
#endif

    Timings refine_he_time, refine_cr_time, refine_cl_time, refine_vx_time ;

	// encapsulates what requires GL context
	{
		// Load data
		std::cout << "Loading " << f_name << " ... " << std::flush ;
		Mesh_Loop_GPU S0(f_name) ;
		std::cout << "[OK]" << std::endl ;

		if (!S0.is_tri_only())
		{
			std::cerr << "ERROR: The provided mesh should be triangle-only" << std::endl ;
			exit(0) ;
		}
		if (S0.V(D) > MAX_VERTICES)
		{
			std::cout << std::endl << "ERROR: Mesh may exceed memory limits at depth " << D << std::endl ;
			return 0 ;
		}

		assert(S0.check()) ;
		Mesh_Loop_GPU S = S0 ;

		for (int d = 1 ; d <= D ; d++)
		{
			const bool export_to_obj = enable_export && d==D ;
			std::cout << "Subdividing level " << d << " " << runCount << "x ... " << std::flush ;
            refine_he_time += S.bench_refine_step_gpu(true, false, false, false, runCount) ;
            refine_cr_time += S.bench_refine_step_gpu(false, true, false, false, runCount) ;
            refine_cl_time += S.bench_refine_step_gpu(false, false, true, false, runCount) ;
            refine_vx_time += S.bench_refine_step_gpu(false, false, false, true, runCount) ;
            S.bench_refine_step_gpu(true, true, true, true, 1, true, export_to_obj) ;
			std::cout << "[OK]" << std::endl ;
		}
		std::cout << std::endl ;

		std::cout << std::fixed << "Halfedges:\t" << refine_he_time.median << "ms" << std::endl ;
		std::cout << std::fixed << "Creases:\t" << refine_cr_time.median << "ms" << std::endl ;
		std::cout << std::fixed << "Clear V:\t" << refine_cl_time.median << "ms" << std::endl ;
		std::cout << std::fixed << "Vertices:\t" << refine_vx_time.median << "ms" << std::endl ;
	//	    std::cout << std::fixed << refine_he_time.median << "\t" << refine_cr_time.median << "\t" << (refine_cl_time.median + refine_vx_time.median) << std::endl ;

		if (enable_export)
		{
			assert(S.check()) ;

			std::stringstream ss ;
			ss << "S" << D << ".obj" ;

			std::cout << "Exporting " << ss.str() << " ... " << std::flush ;
			S.export_to_obj(ss.str()) ;
			std::cout << "[OK]" << std::endl ;
		}
	}

	glfwTerminate();

//	Timings rendering_time = refine_vx_time ;
//    rendering_time += refine_cl_time ;
//	Timings modelling_time = refine_he_time ;
//	modelling_time += refine_cr_time ;
//    modelling_time += refine_cl_time ;
//	modelling_time += refine_vx_time ;

//	// write into files
//	const std::string f_name_tmp = f_name.substr(f_name.find_last_of("\\/") + 1, 999) ;
//	const std::string f_name_clean = f_name_tmp.substr(0,f_name_tmp.find_last_of(".")) ;

//    std::stringstream fname_mod, fname_render, fname_he, fname_cr, fname_clear, fname_vx ;
//	fname_mod << f_name_clean << "_gpu_modelling.txt" ;
//	fname_render << f_name_clean << "_gpu_rendering.txt" ;
//    fname_he << f_name_clean << "_gpu_halfedge.txt" ;
//    fname_cr << f_name_clean << "_gpu_crease.txt" ;
//    fname_clear << f_name_clean << "_gpu_clear.txt" ;
//    fname_vx << f_name_clean << "_gpu_vertex.txt" ;

//	std::ofstream f_mod ;
//	f_mod.open(fname_mod.str(), std::ofstream::out | std::ofstream::app) ;
//	f_mod << std::fixed << "(" << D << ", " << modelling_time.median << ") -= (0.0, " << modelling_time.median - modelling_time.min << ") += (0.0, " << modelling_time.max - modelling_time.median << ")" << std::endl ;
//	f_mod.close() ;

//	std::ofstream f_render ;
//	f_render.open(fname_render.str(), std::ofstream::out | std::ofstream::app) ;
//    f_render << std::fixed << "(" << D << ", " << rendering_time.median << ") -= (0.0, " <<  rendering_time.median - rendering_time.min << ") += (0.0, " << rendering_time.max - rendering_time.median << ")" << std::endl ;
//	f_render.close() ;

//    std::ofstream f_halfedge ;
//    f_halfedge.open(fname_he.str(), std::ofstream::out | std::ofstream::app) ;
//    f_halfedge << std::fixed << "(" << D << ", " << refine_he_time.median << ") -= (0.0, " << refine_he_time.median - refine_he_time.min << ") += (0.0, " << refine_he_time.max - refine_he_time.median << ")" << std::endl ;
//    f_halfedge.close() ;

//    std::ofstream f_crease ;
//    f_crease.open(fname_cr.str(), std::ofstream::out | std::ofstream::app) ;
//    f_crease << std::fixed << "(" << D << ", " << refine_cr_time.median << ") -= (0.0, " << refine_cr_time.median - refine_cr_time.min << ") += (0.0, " << refine_cr_time.max - refine_cr_time.median << ")" << std::endl ;
//    f_crease.close() ;

//    std::ofstream f_clear ;
//    f_clear.open(fname_clear.str(), std::ofstream::out | std::ofstream::app) ;
//    f_clear << std::fixed << "(" << D << ", " << refine_cl_time.median << ") -= (0.0, " << refine_cl_time.median - refine_cl_time.min << ") += (0.0, " << refine_cl_time.max - refine_cl_time.median << ")" << std::endl ;
//    f_clear.close() ;

//    std::ofstream f_vertex ;
//    f_vertex.open(fname_vx.str(), std::ofstream::out | std::ofstream::app) ;
//    f_vertex << std::fixed << "(" << D << ", " << refine_vx_time.median << ") -= (0.0, " << refine_vx_time.median - refine_vx_time.min << ") += (0.0, " << refine_vx_time.max - refine_vx_time.median << ")" << std::endl ;
//    f_vertex.close() ;

	return 0;
}







