#include "mesh_subdiv_catmull-clark.h"

Mesh_Subdiv_CatmullClark::Mesh_Subdiv_CatmullClark(const std::string &filename, uint maxd_cur):
	Mesh_Subdiv(filename, maxd_cur)
{}

int
Mesh_Subdiv_CatmullClark::H(int depth) const
{
	const int& d = depth < 0 ? d_cur : depth ;
	return std::pow(4,d) * H_count ;
}

int
Mesh_Subdiv_CatmullClark::F(int depth) const
{
	const int& d = depth < 0 ? d_cur : depth ;
	return d == 0 ? F_count : std::pow(4,d - 1) * H_count ;
}

int
Mesh_Subdiv_CatmullClark::E(int depth) const
{
	const int& d = depth < 0 ? d_cur : depth ;
	return d == 0 ? E_count : std::pow(2,d-1) * (2*E_count + (std::pow(2,d) - 1)*H_count) ;
}

int
Mesh_Subdiv_CatmullClark::V(int depth) const
{
	const int& d = depth < 0 ? d_cur : depth ;
	switch(d)
	{
		case (0):
			return V_count ;
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
	if (!subdivided) // not quad-only
		return Mesh::Next(halfedges_cage,h) ;

	return h % 4 == 3 ? h - 3 : h + 1 ;
}

int
Mesh_Subdiv_CatmullClark::Prev(int h) const
{
	if (!subdivided) // not quad-only
		return Mesh::Prev(halfedges_cage,h) ;

	return h % 4 == 0 ? h + 3 : h - 1 ;
}

int
Mesh_Subdiv_CatmullClark::Face(int h) const
{
	if (!subdivided) // not quad-only
		return Mesh::Face(halfedges_cage, h) ;

	return h / 4 ;
}

int
Mesh_Subdiv_CatmullClark::n_vertex_of_polygon(int h) const
{
	if (!subdivided) // not quad-only
		return Mesh::n_vertex_of_polygon(h) ;

	return 4 ;
}
