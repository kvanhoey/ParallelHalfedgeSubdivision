#include "mesh.h"

void
Mesh_Subdiv::subdivide()
{
	allocate_subdiv_buffers() ;

	refine_halfedges() ;
	refine_creases() ;
	refine_vertices() ;

	readback_from_subdiv_buffers() ;

	_depth = D ;
}
