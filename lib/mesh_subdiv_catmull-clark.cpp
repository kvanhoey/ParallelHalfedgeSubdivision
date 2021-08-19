#include "mesh_subdiv_catmull-clark.h"

Mesh_Subdiv_CatmullClark::Mesh_Subdiv_CatmullClark(const std::string &filename, uint max_depth):
	Mesh_Subdiv(filename, max_depth)
{
	assert(false) ;
	if (!is_tri_only())
	{
		std::cerr << "ERROR Mesh_Subdiv_CatmullClark: The mesh is not valid or not fully triangular" << std::endl ;
		exit(0) ;
	}
	halfedges_cage.clear() ;
}

int
Mesh_Subdiv_CatmullClark::H(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	assert(false) ;
	return std::pow(4,d) * H0 ;
}

int
Mesh_Subdiv_CatmullClark::F(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	assert(false) ;
	return std::pow(4,d) * F0 ;
}

int
Mesh_Subdiv_CatmullClark::E(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	assert(false) ;
	return pow(2,d)*E0 + 3*(pow(2,2*d-1) - pow(2,d-1))*F0 ;
}

int
Mesh_Subdiv_CatmullClark::V(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	assert(false) ;
	return V0 + (pow(2,d) - 1)*E0 + (pow(2,2*d-1) - 3*pow(2,d-1) + 1)*F0 ;
}

int
Mesh_Subdiv_CatmullClark::Next(int h) const
{
	assert(false) ;
	return h % 3 == 2 ? h - 2 : h + 1 ;
}

int
Mesh_Subdiv_CatmullClark::Prev(int h) const
{
	assert(false) ;
	return h % 3 == 0 ? h + 2 : h - 1 ;
}

int
Mesh_Subdiv_CatmullClark::Face(int h) const
{
	assert(false) ;
	return h / 3 ;
}
