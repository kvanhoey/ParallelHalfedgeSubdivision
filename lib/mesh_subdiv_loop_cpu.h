#ifndef __MESH_SUDBIV_LOOP_CPU_H__
#define __MESH_SUDBIV_LOOP_CPU_H__

#include "mesh_subdiv_loop.h"
#include "mesh_subdiv_cpu.h"

class Mesh_Subdiv_Loop_CPU: public Mesh_Subdiv_Loop, Mesh_Subdiv_CPU
{
public:
	Mesh_Subdiv_Loop_CPU(const std::string& filename, uint depth);

protected:
	void refine_halfedges() ;
	void refine_vertices() ;

	static float compute_beta(float one_over_n) ;
	static float compute_gamma(float one_over_n) ;
	static float compute_ngamma(float one_over_n) ;
};

#endif
