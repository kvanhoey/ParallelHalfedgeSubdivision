#ifndef __MESH_SUBDIV_LOOP_GPU_H__
#define __MESH_SUBDIV_LOOP_GPU_H__

#include "mesh_subdiv_loop.h"
#include "mesh_subdiv_gpu.h"

class Mesh_Subdiv_Loop_GPU: public Mesh_Subdiv_Loop, Mesh_Subdiv_GPU
{
public:
	Mesh_Subdiv_Loop_GPU(const std::string& filename, uint depth) ;
};

#endif
