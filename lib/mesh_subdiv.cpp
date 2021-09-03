#include "mesh_subdiv.h"

Mesh_Subdiv::Mesh_Subdiv(const std::string &filename, uint max_depth):
	Mesh(filename), d_max(max_depth), subdivided(false) {}

void
Mesh_Subdiv::subdivide()
{
	allocate_subdiv_buffers() ;

	refine_halfedges() ;
	refine_creases() ;
	refine_vertices() ;
	set_current_depth(d_max) ;

	readback_from_subdiv_buffers() ;

	finalize_subdivision() ;
}

void
Mesh_Subdiv::set_current_depth(int depth)
{
	d_cur = depth ;
	subdivided = d_cur > 0 ;
}

void
Mesh_Subdiv::finalize_subdivision()
{
	const int Hd = H() ;
	const int Ed = E() ;
	const int Fd = F() ;
	const int Vd = V() ;
	const int Cd = C() ;

	H_count = Hd ;
	E_count = Ed ;
	F_count = Fd ;
	V_count = Vd ;
	C_count = Cd ;

	d_cur = 0 ;

	assert(H() == halfedges.size()) ;
	assert(C() == creases.size()) ;
	assert(V() == vertices.size()) ;
}


int
Mesh_Subdiv::C(int depth) const
{
	const int& d = depth < 0 ? d_cur : depth ;
	return std::pow(2,d) * C_count ;
}
