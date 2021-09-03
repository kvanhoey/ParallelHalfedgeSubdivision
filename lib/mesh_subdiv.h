#ifndef __MESH_SUBDIV_H__
#define __MESH_SUBDIV_H__

#include "mesh.h"

class Mesh_Subdiv: public Mesh
{
public:
	Mesh_Subdiv(const std::string& filename, uint max_depth) ;

protected:
	const uint d_max ;
	uint d_cur ;
	bool subdivided ;

	virtual int C(int depth = -1) const final ;

public:
	virtual void subdivide() final ;

protected:
	/**
	 * @brief set_current_depth sets an internal state with the current depth.
	 * This allows to write low-level routines (e.g., Next, Prev, Face) that rely on this state.
	 * Whenever iterating over subdivision depth, remember to first call set_current_depth().
	 * @param depth
	 */
	void set_current_depth(int depth) ;

	virtual void allocate_subdiv_buffers() = 0 ;
	virtual void readback_from_subdiv_buffers() = 0 ;

	virtual void refine_halfedges() = 0 ;
	virtual void refine_creases() = 0 ;
	virtual void refine_vertices() = 0 ;

private:
	void finalize_subdivision() ;
};

#endif
