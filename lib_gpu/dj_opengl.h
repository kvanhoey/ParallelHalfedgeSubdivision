/* dj_opengl.h - v0.1 - public domain OpenGL3.3+ toolkit

   Do this:
      #define DJ_OPENGL_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.

   USAGE

   INTERFACING

   define DJG_ASSERT(x) to avoid using assert.h.
   define DJG_LOG(format, ...) to use your own logger (default prints in stdout)
   define DJG_MALLOC(x) to use your own memory allocator
   define DJG_FREE(x) to use your own memory deallocator

*/
#ifndef DJG_INCLUDE_DJ_OPENGL_H
#define DJG_INCLUDE_DJ_OPENGL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DJ_OPENGL_STATIC
#define DJGDEF static
#else
#define DJGDEF extern
#endif

//////////////////////////////////////////////////////////////////////////////
//
// Clock API - Get CPU and GPU ticks
//

typedef struct djg_clock djg_clock;

DJGDEF djg_clock *djgc_create(void);
DJGDEF void djgc_release(djg_clock *clock);

DJGDEF void djgc_start(djg_clock *clock);
DJGDEF void djgc_stop(djg_clock *clock);
DJGDEF void djgc_ticks(djg_clock *clock, double *cpu, double *gpu);

//////////////////////////////////////////////////////////////////////////////
//
// Program API - Load OpenGL programs quickly
//

typedef struct djg_program djg_program;

DJGDEF djg_program *djgp_create(void);
DJGDEF void djgp_release(djg_program *program);

DJGDEF bool djgp_push_file(djg_program *program, const char *filename);
DJGDEF bool djgp_push_string(djg_program *program, const char *format, ...);

DJGDEF bool djgp_to_gl(const djg_program *program,
                       int version,
                       bool compatible,
                       bool link,
                       GLuint *gl);

#ifdef __cplusplus
} // extern "C"
#endif

//
//
//// end header file /////////////////////////////////////////////////////
#endif // DJG_INCLUDE_DJ_OPENGL_H

#ifdef DJ_OPENGL_IMPLEMENTATION

#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef DJG_ASSERT
#    include <assert.h>
#    define DJG_ASSERT(x) assert(x)
#endif

#ifndef DJG_LOG
#    include <stdio.h>
#    define DJG_LOG(format, ...) do { fprintf(stdout, format, ##__VA_ARGS__); fflush(stdout); } while(0)
#endif

#ifndef DJG_MALLOC
#    include <stdlib.h>
#    define DJG_MALLOC(x) (malloc(x))
#    define DJG_FREE(x) (free(x))
#else
#    ifndef DJG_FREE
#        error DJG_MALLOC defined without DJG_FREE
#    endif
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

// Internal constants
#define DJG__EPSILON 1e-4
#define DJG__CHAR_BUFFER_SIZE 4096

// Internal macros
#define DJG__BUFSIZE(x) sizeof(x) / sizeof(x[0])

// *************************************************************************************************
// Timer API Implementation

enum {DJGC__QUERY_START, DJGC__QUERY_STOP, DJGC__QUERY_COUNT};

typedef struct djg_clock {
    GLdouble cpu_ticks, gpu_ticks;
    GLint64 cpu_start_ticks;
    GLuint queries[DJGC__QUERY_COUNT];
    GLint is_gpu_ticking,
          is_cpu_ticking,
          is_gpu_ready;
} djg_clock;

DJGDEF djg_clock *djgc_create(void)
{
    djg_clock *clock = (djg_clock *)DJG_MALLOC(sizeof(*clock));

    glGenQueries(DJGC__QUERY_COUNT, clock->queries);
    clock->cpu_ticks = 0.0;
    clock->gpu_ticks = 0.0;
    clock->cpu_start_ticks = 0.0;
    clock->is_cpu_ticking = GL_FALSE;
    clock->is_gpu_ticking = GL_FALSE;
    clock->is_gpu_ready = GL_TRUE;
    glQueryCounter(clock->queries[DJGC__QUERY_START], GL_TIMESTAMP);
    glQueryCounter(clock->queries[DJGC__QUERY_STOP], GL_TIMESTAMP);

    return clock;
}

DJGDEF void djgc_release(djg_clock *clock)
{
    DJG_ASSERT(clock);
    glDeleteQueries(DJGC__QUERY_COUNT, clock->queries);
    DJG_FREE(clock);
}

DJGDEF void djgc_start(djg_clock *clock)
{
    DJG_ASSERT(clock);
    if (!clock->is_cpu_ticking) {
        clock->is_cpu_ticking = GL_TRUE;
        glGetInteger64v(GL_TIMESTAMP, &clock->cpu_start_ticks);
    }
    if (!clock->is_gpu_ticking && clock->is_gpu_ready) {
        glQueryCounter(clock->queries[DJGC__QUERY_START], GL_TIMESTAMP);
        clock->is_gpu_ticking = GL_TRUE;
    }
}

DJGDEF void djgc_stop(djg_clock *clock)
{
    DJG_ASSERT(clock);
    if (clock->is_cpu_ticking) {
        GLint64 now = 0;

        glGetInteger64v(GL_TIMESTAMP, &now);
        clock->cpu_ticks = (now - clock->cpu_start_ticks) / 1e9;
        clock->is_cpu_ticking = GL_FALSE;
    }
    if (clock->is_gpu_ticking && clock->is_gpu_ready) {
        glQueryCounter(clock->queries[DJGC__QUERY_STOP], GL_TIMESTAMP);
        clock->is_gpu_ticking = GL_FALSE;
    }
}

// lazy GPU evaluation
DJGDEF void djgc_ticks(djg_clock *clock, double *tcpu, double *tgpu)
{
    DJG_ASSERT(clock);
    if (!clock->is_gpu_ticking) {
        glGetQueryObjectiv(clock->queries[DJGC__QUERY_STOP],
                           GL_QUERY_RESULT_AVAILABLE,
                           &clock->is_gpu_ready);
        if (clock->is_gpu_ready) {
            GLuint64 start, stop;

            glGetQueryObjectui64v(clock->queries[DJGC__QUERY_STOP],
                                  GL_QUERY_RESULT,
                                  &stop);
            glGetQueryObjectui64v(clock->queries[DJGC__QUERY_START],
                                  GL_QUERY_RESULT,
                                  &start);
            clock->gpu_ticks = (stop - start) / 1e9;
        }
    }
    if (tcpu) *tcpu = clock->cpu_ticks;
    if (tgpu) *tgpu = clock->gpu_ticks;
}

// *************************************************************************************************
// Program API Implementation

enum {
    DJGP__STAGE_VERTEX_BIT          = 1,
    DJGP__STAGE_FRAGMENT_BIT        = 1 << 1,
    DJGP__STAGE_GEOMETRY_BIT        = 1 << 2,
    DJGP__STAGE_TESS_CONTROL_BIT    = 1 << 3,
    DJGP__STAGE_TESS_EVALUATION_BIT = 1 << 4,
    DJGP__STAGE_TASK_BIT            = 1 << 5,
    DJGP__STAGE_MESH_BIT            = 1 << 6,
    DJGP__STAGE_COMPUTE_BIT         = 1 << 7
};

typedef struct djg_program {
    char *src; // null terminated string holding the source code
    struct djg_program *next;
} djg_program;

static bool
djgp__attach_shader(
    GLuint program,
    GLenum shader_t,
    GLsizei count,
    const GLchar **source
) {
    GLint compiled;
    GLuint shader = glCreateShader(shader_t);

    // set source and compile
    glShaderSource(shader, count, source, NULL);
    glCompileShader(shader);

    // check compilation
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint logc = 0;
        GLchar *logv = NULL;

        // retrieve GLSL compiler information and log
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logc);
        logv = (GLchar *)DJG_MALLOC(logc);
        glGetShaderInfoLog(shader, logc, NULL, logv);
        DJG_LOG("djg_error: Shader compilation failed\n"\
                "-- Begin -- GLSL Compiler Info Log\n"          \
                "%s\n"                                          \
                "-- End -- GLSL Compiler Info Log\n", logv);
        DJG_FREE(logv);

        glDeleteShader(shader);
        return false;
    }

    // attach shader and flag for deletion
    glAttachShader(program, shader);
    glDeleteShader(shader);
    return true;
}

static bool djgp__push_src(djg_program *program, char *src)
{
    djg_program *tail = program;

    while (tail->next) tail = tail->next;
    tail->next = djgp_create();
    tail->next->src = src;

    return true;
}

static int djgp__count(const djg_program *program)
{
    const djg_program *it = program;
    int i = 0;

    while (it->next) {
        it = it->next;
        ++i;
    }

    return i;
}

DJGDEF djg_program *djgp_create(void)
{
    djg_program *head = (djg_program*)DJG_MALLOC(sizeof(*head));

    head->next = NULL;
    head->src = NULL;

    return head;
}

DJGDEF void djgp_release(djg_program *program)
{
    djg_program *tmp;

    DJG_ASSERT(program);
    while (program) {
        tmp = program;
        program = program->next;
        DJG_FREE(tmp->src);
        DJG_FREE(tmp);
    }
}

DJGDEF bool djgp_push_file(djg_program *program, const char *file)
{
    FILE *pf;
    char *src, letter;
    int n = 0;

    DJG_ASSERT(program);
    pf = fopen(file, "r");
    if (!pf) {
        DJG_LOG("djg_error: Function fopen() failed\n");
        return false;
    }

    // get file length
#if 0 // nonportable (does not work with vs2010)
    fseek(pf, 0, SEEK_END);
    n = (int)ftell(pf) + /* '\0' */1;
#else
    do {++n;} while (fread(&letter, 1, 1, pf));
    ++n; // add an extra char for an endline
#endif
    rewind(pf);

    // read and push to source
    src = (char *)DJG_MALLOC(sizeof(char) * n);
	size_t _s = fread(src, 1, n-1, pf);
    fclose(pf);
    src[n-2] = '\n'; // add endline
    src[n-1] = '\0'; // add EOF

    // add a line parameter (easier for debugging)
    djgp_push_string(program, "#line 1\n");

    return djgp__push_src(program, src);
}

DJGDEF bool djgp_push_string(djg_program *program, const char *str, ...)
{
    char *buf;
    va_list vl;
    int n;

    DJG_ASSERT(program);
    DJG_ASSERT(strlen(str) < DJG__CHAR_BUFFER_SIZE);
    buf = (char *)DJG_MALLOC(sizeof(char) * DJG__CHAR_BUFFER_SIZE);
    va_start(vl, str);
    n = vsnprintf(buf, DJG__CHAR_BUFFER_SIZE, str, vl);
    va_end(vl);
    if (n < 0 || n > DJG__CHAR_BUFFER_SIZE) {
        DJG_LOG("djg_error: string too long\n");
        return false;
    }

    return djgp__push_src(program, buf);
}

DJGDEF bool
djgp_to_gl(
    const djg_program *program,
    int version,
    bool compatible,
    bool link,
    GLuint *gl
) {
    const GLchar **srcv;
    djg_program *it;
    int i, srcc, stages = 0;
    GLuint glprogram;

    DJG_ASSERT(program && gl);

    // prepare sources
    srcc = djgp__count(program) + /* head */1;
    srcv = (const GLchar **)DJG_MALLOC(sizeof(GLchar *) * srcc);

    // set source pointers
    it = program->next;
    i = 1;
    while (it) {
		//printf("%s\n",it->src);
        srcv[i] = it->src;
        it = it->next;
        ++i;
    }

    // look for stages in the last source file
    if (strstr(srcv[srcc-1], "VERTEX_SHADER"))
        stages|= DJGP__STAGE_VERTEX_BIT;
    if (strstr(srcv[srcc-1], "FRAGMENT_SHADER"))
        stages|= DJGP__STAGE_FRAGMENT_BIT;
    if (strstr(srcv[srcc-1], "GEOMETRY_SHADER"))
        stages|= DJGP__STAGE_GEOMETRY_BIT;
    if (strstr(srcv[srcc-1], "TESS_CONTROL_SHADER"))
        stages|= DJGP__STAGE_TESS_CONTROL_BIT;
    if (strstr(srcv[srcc-1], "TESS_EVALUATION_SHADER"))
        stages|= DJGP__STAGE_TESS_EVALUATION_BIT;
    if (strstr(srcv[srcc-1], "COMPUTE_SHADER"))
        stages|= DJGP__STAGE_COMPUTE_BIT;
    if (strstr(srcv[srcc-1], "TASK_SHADER"))
        stages|= DJGP__STAGE_TASK_BIT;
    if (strstr(srcv[srcc-1], "MESH_SHADER"))
        stages|= DJGP__STAGE_MESH_BIT;

    if (!stages) {
        DJG_LOG("djg_error: no shader stage found in source\n");
        DJG_FREE(srcv);
        return false;
    }
    // load program
    glprogram = glCreateProgram();
    if (!glIsProgram(glprogram)) {
        DJG_LOG("djg_error: glCreateProgram failed\n");
        DJG_FREE(srcv);
        return false;
    }

#define DJGP__ATTACH_SHADER(glstage, str, bit)                        \
    if (stages & bit) {                                               \
        char head[DJG__CHAR_BUFFER_SIZE];                             \
        head[0] = '\0';                                               \
                                                                      \
        sprintf(head,                                                 \
                "#version %d %s\n",                                   \
                version,                                              \
                compatible ? "compatibility" : "");                   \
        strcat(head, "#define " str " 1\n");                          \
        srcv[0] = head;                                               \
        if (!djgp__attach_shader(glprogram, glstage, srcc, srcv)) {   \
            glDeleteProgram(glprogram);                               \
            DJG_FREE(srcv);                                           \
            return false;                                             \
        }                                                             \
    }
    DJGP__ATTACH_SHADER(GL_VERTEX_SHADER, "VERTEX_SHADER", DJGP__STAGE_VERTEX_BIT);
    DJGP__ATTACH_SHADER(GL_FRAGMENT_SHADER, "FRAGMENT_SHADER", DJGP__STAGE_FRAGMENT_BIT);
#if defined(GL_GEOMETRY_SHADER)
    DJGP__ATTACH_SHADER(GL_GEOMETRY_SHADER, "GEOMETRY_SHADER", DJGP__STAGE_GEOMETRY_BIT);
#endif
#if defined(GL_TESS_CONTROL_SHADER)
    DJGP__ATTACH_SHADER(GL_TESS_CONTROL_SHADER, "TESS_CONTROL_SHADER", DJGP__STAGE_TESS_CONTROL_BIT);
#endif
#if defined(GL_TESS_EVALUATION_SHADER)
    DJGP__ATTACH_SHADER(GL_TESS_EVALUATION_SHADER, "TESS_EVALUATION_SHADER", DJGP__STAGE_TESS_EVALUATION_BIT);
#endif
#if defined(GL_COMPUTE_SHADER)
    DJGP__ATTACH_SHADER(GL_COMPUTE_SHADER, "COMPUTE_SHADER", DJGP__STAGE_COMPUTE_BIT);
#endif
#if defined(GL_TASK_SHADER_NV)
    DJGP__ATTACH_SHADER(GL_TASK_SHADER_NV, "TASK_SHADER", DJGP__STAGE_TASK_BIT);
#endif
#if defined(GL_MESH_SHADER_NV)
    DJGP__ATTACH_SHADER(GL_MESH_SHADER_NV, "MESH_SHADER", DJGP__STAGE_MESH_BIT);
#endif

#undef DJGP__ATTACH_SHADER

    // link if requested
    if (link) {
        GLint link_status = 0;

        glLinkProgram(glprogram);
        glGetProgramiv(glprogram, GL_LINK_STATUS, &link_status);
        if (!link_status) {
            GLint logc = 0;
            GLchar *logv = NULL;

            glGetProgramiv(glprogram, GL_INFO_LOG_LENGTH, &logc);
            logv = (GLchar *)DJG_MALLOC(logc);
            glGetProgramInfoLog(glprogram, logc, NULL, logv);
            DJG_LOG("djg_error: GLSL linker failure\n\
                     -- Begin -- GLSL Linker Info Log\n\
                     %s\n\
                     -- End -- GLSL Linker Info Log\n", logv);
            glDeleteProgram(glprogram);
            DJG_FREE(logv);
            DJG_FREE(srcv);
            return false;
        }
    }

    // cleanup
    DJG_FREE(srcv);
    // affect program
    if (glIsProgram (*gl)) glDeleteProgram(*gl);
    *gl = glprogram;

    return true;
}

#endif // DJ_OPENGL_IMPLEMENTATION

