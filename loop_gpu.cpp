
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

#define DJ_OPENGL_IMPLEMENTATION
#include "dj_opengl.h"

#define LOG(fmt, ...)  fprintf(stdout, fmt, ##__VA_ARGS__); fflush(stdout);
static void APIENTRY
debug_output_logger(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
) {
    char srcstr[32], typestr[32];
    switch (source) {
    case GL_DEBUG_SOURCE_API: strcpy(srcstr, "OpenGL"); break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: strcpy(srcstr, "Windows"); break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: strcpy(srcstr, "Shader Compiler"); break;
    case GL_DEBUG_SOURCE_THIRD_PARTY: strcpy(srcstr, "Third Party"); break;
    case GL_DEBUG_SOURCE_APPLICATION: strcpy(srcstr, "Application"); break;
    case GL_DEBUG_SOURCE_OTHER: strcpy(srcstr, "Other"); break;
    default: strcpy(srcstr, "???"); break;
    };
    switch (type) {
    case GL_DEBUG_TYPE_ERROR: strcpy(typestr, "Error"); break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: strcpy(typestr, "Deprecated Behavior"); break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: strcpy(typestr, "Undefined Behavior"); break;
    case GL_DEBUG_TYPE_PORTABILITY: strcpy(typestr, "Portability"); break;
    case GL_DEBUG_TYPE_PERFORMANCE: strcpy(typestr, "Performance"); break;
    case GL_DEBUG_TYPE_OTHER: strcpy(typestr, "Message"); break;
    default: strcpy(typestr, "???"); break;
    }
    if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM) {
        LOG("djg_debug_output: %s %s\n"                \
            "-- Begin -- GL_debug_output\n" \
            "%s\n"                              \
            "-- End -- GL_debug_output\n",
            srcstr, typestr, message);
    }
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
        LOG("djg_debug_output: %s %s\n"                 \
            "-- Begin -- GL_debug_output\n" \
            "%s\n"                              \
            "-- End -- GL_debug_output\n",
            srcstr, typestr, message);
    }
}
void log_debug_output(void)
{
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(&debug_output_logger, nullptr);
}

// ----- Program -----
GLuint glsl_program ;

uint data_id = 0 ;

void create_program()
{
    glsl_program = glCreateProgram() ;

    djg_program* builder = djgp_create() ;
    djgp_push_string(builder,"#define MY_LOCATION %d\n", data_id) ;
    djgp_push_string(builder,"#define MY_RO_LOCATION %d\n", data_id + 1) ;
    djgp_push_string(builder, "#extension GL_NV_shader_atomic_float: require\n");

    djgp_push_file(builder, "../demo-all/first_shader.glsl") ;

    djgp_to_gl(builder, 450, false, true, &glsl_program) ;

    djgp_release(builder) ;
}

void delete_program()
{
    glDeleteProgram(glsl_program) ;
}

// ----- Buffer -----
GLuint my_buffer, my_buffer_ro ;
void init_buffer()
{
    float data[4] = {1.,2.,3.,4.} ;

    glGenBuffers(1, &my_buffer) ;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, my_buffer) ;
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(data), NULL, GL_MAP_READ_BIT) ;
    glClearNamedBufferData(my_buffer, //g_gl.buffers[BUFFER_SUBD_VERTEX_POINTS],
                                       GL_R32F,
                                       GL_RED,
                                       GL_FLOAT,
                                       nullptr);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(data), NULL, GL_MAP_READ_BIT) ;

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, data_id, my_buffer) ; // allows to be read in shader


    glGenBuffers(1, &my_buffer_ro) ;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, my_buffer_ro) ;
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(data), data, GL_MAP_READ_BIT) ;

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, data_id + 1, my_buffer_ro) ; // allows to be read in shader



    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0) ;
}

void readback_buffer()
{
    float* data = (float*) glMapNamedBuffer(my_buffer, GL_READ_ONLY) ;

    for (int i = 0 ; i < 4 ; ++i)
        std::cout << data[i] << std::endl ;

    glUnmapNamedBuffer(my_buffer) ;
}

void release_buffer()
{
    glDeleteBuffers(1, &my_buffer) ;
    glDeleteBuffers(1, &my_buffer_ro) ;
}



int main(int argc, char **argv)
{
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

    init_buffer() ;
    create_program();

    glUseProgram(glsl_program) ;
    glDispatchCompute(1,1,1) ;

    glMemoryBarrier(GL_ALL_BARRIER_BITS) ;

    readback_buffer() ;

    release_buffer() ;
    delete_program();

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

