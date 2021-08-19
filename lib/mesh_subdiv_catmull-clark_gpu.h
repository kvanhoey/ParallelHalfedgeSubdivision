#ifndef __MESH_SUBDIV_CATMULLCLARK_GPU_H__
#define __MESH_SUBDIV_CATMULLCLARK_GPU_H__

#include "mesh_subdiv_catmull-clark.h"
#include "mesh_subdiv_gpu.h"

class Mesh_Subdiv_CatmullClark_GPU: public Mesh_Subdiv_CatmullClark, Mesh_Subdiv_GPU
{
public:
	Mesh_Subdiv_CatmullClark_GPU(const std::string& filename, uint depth) ;
};

#endif
