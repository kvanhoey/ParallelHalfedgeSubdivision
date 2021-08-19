#ifndef __MESH_SUDBIV_CPU_H__
#define __MESH_SUDBIV_CPU_H__

#include "mesh_subdiv.h"

class Mesh_Subdiv_CPU: virtual public Mesh_Subdiv
{
public:
	Mesh_Subdiv_CPU(const std::string& filename, uint max_depth) ;

protected:
	std::vector<halfedge_buffer> halfedge_subdiv_buffers ;
	std::vector<crease_buffer> crease_subdiv_buffers ;
	std::vector<vertex_buffer> vertex_subdiv_buffers ;

	void allocate_subdiv_buffers() final ;
	void readback_from_subdiv_buffers() final ;
	void refine_creases() ;

	static void apply_atomic_vec3_increment(vec3& v, const vec3& v_increm) ;

};

#endif
