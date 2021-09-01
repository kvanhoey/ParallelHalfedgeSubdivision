#ifndef __MESH_SUDBIV_CATMULLCLARK_H__
#define __MESH_SUDBIV_CATMULLCLARK_H__

#include "mesh_subdiv.h"

class Mesh_Subdiv_CatmullClark: virtual public Mesh_Subdiv
{
public:
	Mesh_Subdiv_CatmullClark(const std::string& filename, uint max_depth) ;

	int H(int depth = -1) const ;
	int V(int depth = -1) const ;
	int F(int depth = -1) const ;
	int E(int depth = -1) const ;

protected:
	// override with analytic versions
	int Prev(int h) const ;
	int Next(int h) const ;
	int Face(int h) const ;

	virtual int n_vertex_of_polygon(int h) const final ;
};


#endif
