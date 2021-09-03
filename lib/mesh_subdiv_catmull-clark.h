#ifndef __MESH_SUDBIV_CATMULLCLARK_H__
#define __MESH_SUDBIV_CATMULLCLARK_H__

#include "mesh_subdiv.h"

class Mesh_Subdiv_CatmullClark: virtual public Mesh_Subdiv
{
	// ----------- Constructor -----------
public:
	Mesh_Subdiv_CatmullClark(const std::string& filename, uint max_depth) ;

	// ----------- Override of accessors -----------
	virtual int H(int depth = -1) const final ;
	virtual int V(int depth = -1) const final ;
	virtual int F(int depth = -1) const final ;
	virtual int E(int depth = -1) const final ;

protected:
	int Prev(int h) const ;
	int Next(int h) const ;
	int Face(int h) const ;

	virtual int n_vertex_of_polygon(int h) const final ;
};

#endif
