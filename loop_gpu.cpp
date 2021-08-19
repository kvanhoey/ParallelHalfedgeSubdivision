// basic file operations
#include <iostream>
#include <fstream>
#include <sstream>

#define MAX_VERTICES pow(2,28)

#include "mesh_subdiv_loop_gpu.h"

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " <filename>.obj <depth>" << std::endl ;
		return 0 ;
	}

	const std::string f_name(argv[1]) ;
	const uint D = atoi(argv[2]) ;

	std::stringstream fname_out_ss ;
	fname_out_ss << "S" << D << "_gpu.obj" ;
	std::string fname_out = fname_out_ss.str() ;


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

	{ // encapsulates what requires GL context
		std::cout << "Loading " << f_name << " ... " << std::flush ;
		Mesh_Subdiv_Loop_GPU M(f_name, D) ;
		std::cout << "[OK]" << std::endl ;

		if (M.V(D) > MAX_VERTICES)
		{
			std::cout << std::endl << "ERROR: Mesh may exceed memory limits at depth " << D << std::endl ;
			return 0 ;
		}

		const char* num_threads_str = std::getenv("OMP_NUM_THREADS") ;
		if (num_threads_str != NULL)
			std::cout << "Using " << atoi(num_threads_str) << " threads" << std::endl ;
		else
			std::cout << "Using default number of threads" << std::endl ;

		// Check & export input
		M.check() ;
		std::cout << "Exporting input S0_input.obj ... " << std::flush ;
		M.export_to_obj("S0_input.obj") ;
		std::cout << "[OK]" << std::endl ;

		// subdiv down to depth D
		std::cout << "Processing subdivision ... " << std::flush ;
		M.subdivide() ;
		std::cout << "[OK]" << std::endl ;

		// Check & export output
		M.check() ;
		std::cout << "Exporting output " << fname_out << " ... " << std::flush ;
		M.export_to_obj(fname_out) ;
		std::cout << "[OK]" << std::endl ;
	}

	return 0 ;
}
