#include "mesh_subdiv.h"

Mesh_Subdiv::Mesh_Subdiv(const std::string &filename, uint max_depth):
	Mesh(filename), D(max_depth) {}

void
Mesh_Subdiv::subdivide()
{
	allocate_subdiv_buffers() ;

	refine_halfedges() ;
	refine_creases() ;
	refine_vertices() ;

	readback_from_subdiv_buffers() ;

	set_current_depth(D) ;
}
