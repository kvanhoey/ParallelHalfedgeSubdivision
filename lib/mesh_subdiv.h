#ifndef __MESH_SUBDIV_H__
#define __MESH_SUBDIV_H__

#include "mesh.h"

class Mesh_Subdiv: public Mesh
{
public:
	Mesh_Subdiv(const std::string& filename, uint max_depth) ;

	virtual void subdivide() final ;

protected:
	const uint D ;
	virtual void allocate_subdiv_buffers() = 0 ;
	virtual void readback_from_subdiv_buffers() = 0 ;

protected:
	virtual void refine_halfedges() = 0 ;
	virtual void refine_creases() = 0 ;
	virtual void refine_vertices() = 0 ;
};

#endif
