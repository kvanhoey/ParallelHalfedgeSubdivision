#ifndef __MESH_SUDBIV_CATMULLCLARK_CPU_H__
#define __MESH_SUDBIV_CATMULLCLARK_CPU_H__

#include "mesh_subdiv_catmull-clark.h"
#include "mesh_subdiv_cpu.h"

class Mesh_Subdiv_CatmullClark_CPU: public Mesh_Subdiv_CatmullClark, Mesh_Subdiv_CPU
{
public:
	Mesh_Subdiv_CatmullClark_CPU(const std::string& filename, uint depth);

protected:
	void refine_halfedges() ;
	void refine_vertices() ;
};

#endif
