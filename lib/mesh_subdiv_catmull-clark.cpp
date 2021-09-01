#include "mesh_subdiv_catmull-clark.h"

Mesh_Subdiv_CatmullClark::Mesh_Subdiv_CatmullClark(const std::string &filename, uint max_depth):
	Mesh_Subdiv(filename, max_depth)
{}

int
Mesh_Subdiv_CatmullClark::H(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	return std::pow(4,d) * H0 ;
}

int
Mesh_Subdiv_CatmullClark::F(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	return d == 0 ? F0 : std::pow(4,d - 1) * H0 ;
}

int
Mesh_Subdiv_CatmullClark::E(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	return d == 0 ? E0 : std::pow(2,d-1) * (2*E0 + (std::pow(2,d) - 1)*H0) ;
}

int
Mesh_Subdiv_CatmullClark::V(int depth) const
{
	const int& d = depth < 0 ? _depth : depth ;
	switch(d)
	{
		case (0):
			return V0 ;
			break ;
		case (1):
			return V(0) + F(0) + E(0) ;
			break;
		default:
			const float tmp_pow = pow(2,d - 1) - 1 ;
			return V(1) + tmp_pow * (E(1) + tmp_pow * F(1)) ;
	}
}

int
Mesh_Subdiv_CatmullClark::Next(int h) const
{
	if (_depth < 1) // not quad-only
		return Mesh::Next(halfedges_cage,h) ;

	return h % 4 == 3 ? h - 3 : h + 1 ;
}

int
Mesh_Subdiv_CatmullClark::Prev(int h) const
{
	if (_depth < 1) // not quad-only
		return Mesh::Prev(halfedges_cage,h) ;

	return h % 4 == 0 ? h + 3 : h - 1 ;
}

int
Mesh_Subdiv_CatmullClark::Face(int h) const
{
	if (_depth < 1) // not quad-only
		return Mesh::Face(halfedges_cage, h) ;

	return h / 4 ;
}

int
Mesh_Subdiv_CatmullClark::n_vertex_of_polygon(int h) const
{
	if (_depth < 1) // not quad-only
		return Mesh::n_vertex_of_polygon(h) ;

	return 4 ;
}
