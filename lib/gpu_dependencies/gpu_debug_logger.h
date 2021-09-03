#ifndef __GPU_DEBUG_LOGGER__
#define __GPU_DEBUG_LOGGER__

#include <cstdio>
#include <cstring>
#include "glad/glad.h"

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
    if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM || true) {
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
void gpu_log_debug_output(void)
{
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(&debug_output_logger, nullptr);
}

#endif
