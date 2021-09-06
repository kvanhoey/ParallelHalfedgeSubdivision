#ifndef __MESH_SUBDIV_LOOP_GPU_H__
#define __MESH_SUBDIV_LOOP_GPU_H__

#include "mesh_subdiv_loop.h"
#include "mesh_subdiv_gpu.h"

/**
 * @brief The Mesh_Subdiv_Loop_GPU class implements Catmull-Clark subdivision on the CPU
 */
class Mesh_Subdiv_Loop_GPU: public Mesh_Subdiv_Loop, Mesh_Subdiv_GPU
{
public:
	/**
	 * @brief Mesh_Subdiv_Loop_GPU constructor from OBJ file
	 * @param filename path to an OBJ file
	 * @param max_depth the depth at which to subdivide the mesh
	 */
	Mesh_Subdiv_Loop_GPU(const std::string& filename, uint max_depth) ;
};

#endif
