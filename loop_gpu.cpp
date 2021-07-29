#include <iostream>

#include "mesh_loop_gpu.h"
#include "gpu_debug_logger.h"

int main(int argc, char **argv)
{
	// Fetch arguments
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " <filename>.obj <depth> <export=0>" << std::endl ;
		return 0 ;
	}

	const std::string f_name(argv[1]) ;
	const uint D = atoi(argv[2]) ;
	const bool enable_export = argc < 4 ? false : atoi(argv[3]) ;

	// Init GL
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

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

	// encapsulates what requires GL context
	{
		// Load data
		std::cout << "Loading " << f_name << std::endl ;
		Mesh_Loop_GPU S0(f_name) ;
		if (!S0.is_tri_only())
		{
			std::cerr << "ERROR: The provided mesh should be triangle-only" << std::endl ;
			exit(0) ;
		}

		S0.check() ;

		if (enable_export)
		{
			std::cout << "Exporting S0.obj" << std::endl ;
			S0.export_to_obj("S0.obj") ;
		}

		Mesh_Loop_GPU S = S0 ;

        log_debug_output() ;
		S.refine_step_gpu() ;

		if (enable_export)
		{
			std::cout << "Exporting S1.obj" << std::endl ;
			S.export_to_obj("S1.obj") ;
		}
	}

	glfwTerminate();

	return 0;
}







