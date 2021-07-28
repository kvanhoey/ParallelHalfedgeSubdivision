#ifndef __MESH_LOOP_GPU_H__
#define __MESH_LOOP_GPU_H__

#include "mesh.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

class Mesh_Loop_GPU: public Mesh_Loop
{
public:
	Mesh_Loop_GPU(int H, int V, int E, int F) ;
	Mesh_Loop_GPU(const std::string& filename) ;



private:
	GLuint halfedges_gpu, creases_gpu, vertices_gpu ;

	void init_buffers() ;
	void readback_buffers() ;
};

#endif
