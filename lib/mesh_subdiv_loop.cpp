#include "mesh_subdiv_loop.h"

Mesh_Subdiv_Loop::Mesh_Subdiv_Loop(const std::string &filename, uint max_depth):
	Mesh_Subdiv(filename, max_depth)
{
	if (!is_tri_only())
	{
		std::cerr << "ERROR Mesh_Subdiv_Loop: The mesh is not valid or not fully triangular" << std::endl ;
		exit(0) ;
	}
	halfedges_cage.clear() ;
}

int
Mesh_Subdiv_Loop::H(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	return std::pow(4,d) * H0 ;
}

int
Mesh_Subdiv_Loop::F(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	return std::pow(4,d) * F0 ;
}

int
Mesh_Subdiv_Loop::E(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	return pow(2,d)*E0 + 3*(pow(2,2*d-1) - pow(2,d-1))*F0 ;
}

int
Mesh_Subdiv_Loop::V(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	return V0 + (pow(2,d) - 1)*E0 + (pow(2,2*d-1) - 3*pow(2,d-1) + 1)*F0 ;
}

int
Mesh_Subdiv_Loop::Next(int h) const
{
	return h % 3 == 2 ? h - 2 : h + 1 ;
}

int
Mesh_Subdiv_Loop::Prev(int h) const
{
	return h % 3 == 0 ? h + 2 : h - 1 ;
}

int
Mesh_Subdiv_Loop::Face(int h) const
{
	return h / 3 ;
}
