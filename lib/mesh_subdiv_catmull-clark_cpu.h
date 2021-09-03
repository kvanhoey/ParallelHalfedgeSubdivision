#ifndef __MESH_SUDBIV_CATMULLCLARK_CPU_H__
#define __MESH_SUDBIV_CATMULLCLARK_CPU_H__

#include "mesh_subdiv_catmull-clark.h"
#include "mesh_subdiv_cpu.h"

class Mesh_Subdiv_CatmullClark_CPU: public Mesh_Subdiv_CatmullClark, Mesh_Subdiv_CPU
{
public:
	Mesh_Subdiv_CatmullClark_CPU(const std::string& filename, uint depth);

protected:
	// ----------- Member functions that do the actual subdivision -----------
	void refine_halfedges() ;
	void refine_vertices() ;

	// ----------- Utility functions -----------
	void refine_vertices_facepoints(uint d) ;
	void refine_vertices_edgepoints(uint d) ;
	void refine_vertices_vertexpoints(uint d) ;
};

#endif
