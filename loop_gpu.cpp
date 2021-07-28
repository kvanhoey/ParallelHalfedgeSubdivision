#include <iostream>

#include "mesh_loop_gpu.h"


//// ----- Program -----
//GLuint glsl_program ;

//uint data_id = 0 ;

//void create_program()
//{
//    glsl_program = glCreateProgram() ;

//    djg_program* builder = djgp_create() ;
//    djgp_push_string(builder,"#define MY_LOCATION %d\n", data_id) ;
//    djgp_push_string(builder,"#define MY_RO_LOCATION %d\n", data_id + 1) ;
//    djgp_push_string(builder, "#extension GL_NV_shader_atomic_float: require\n");

//    djgp_push_file(builder, "../demo-all/first_shader.glsl") ;

//    djgp_to_gl(builder, 450, false, true, &glsl_program) ;

//    djgp_release(builder) ;
//}

//void delete_program()
//{
//    glDeleteProgram(glsl_program) ;
//}

//// ----- Buffer -----
//GLuint my_buffer, my_buffer_ro ;
//void init_buffer()
//{
//    float data[4] = {1.,2.,3.,4.} ;

//    glGenBuffers(1, &my_buffer) ;
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, my_buffer) ;
//    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(data), NULL, GL_MAP_READ_BIT) ;
//    glClearNamedBufferData(my_buffer, //g_gl.buffers[BUFFER_SUBD_VERTEX_POINTS],
//                                       GL_R32F,
//                                       GL_RED,
//                                       GL_FLOAT,
//                                       nullptr);
//    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(data), NULL, GL_MAP_READ_BIT) ;

//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, data_id, my_buffer) ; // allows to be read in shader


//    glGenBuffers(1, &my_buffer_ro) ;
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, my_buffer_ro) ;
//    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(data), data, GL_MAP_READ_BIT) ;

//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, data_id + 1, my_buffer_ro) ; // allows to be read in shader



//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0) ;
//}

//void readback_buffer()
//{
//    float* data = (float*) glMapNamedBuffer(my_buffer, GL_READ_ONLY) ;

//    for (int i = 0 ; i < 4 ; ++i)
//        std::cout << data[i] << std::endl ;

//    glUnmapNamedBuffer(my_buffer) ;
//}

//void release_buffer()
//{
//    glDeleteBuffers(1, &my_buffer) ;
//    glDeleteBuffers(1, &my_buffer_ro) ;
//}



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


/*	init_buffer() ;
	create_program();

	glUseProgram(glsl_program) ;
	glDispatchCompute(1,1,1) ;

	glMemoryBarrier(GL_ALL_BARRIER_BITS) ;

	readback_buffer() ;

	release_buffer() ;
	delete_program();
*/
//	while (!glfwWindowShouldClose(window))
//	{
//		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT);
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//	}

	glfwTerminate();

	return 0;
}







