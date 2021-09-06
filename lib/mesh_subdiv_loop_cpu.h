#ifndef __MESH_SUDBIV_LOOP_CPU_H__
#define __MESH_SUDBIV_LOOP_CPU_H__

#include "mesh_subdiv_loop.h"
#include "mesh_subdiv_cpu.h"

/**
 * @brief The Mesh_Subdiv_Loop_CPU class implements Loop subdivision on the CPU
 */
class Mesh_Subdiv_Loop_CPU: public Mesh_Subdiv_Loop, Mesh_Subdiv_CPU
{
public:
	/**
	 * @brief Mesh_Subdiv_Loop_CPU constructor from OBJ file
	 * @param filename path to an OBJ file
	 * @param max_depth the depth at which to subdivide the mesh
	 */
	Mesh_Subdiv_Loop_CPU(const std::string& filename, uint max_depth);

protected:
	// ----------- Member functions that do the actual subdivision -----------
	/**
	 * @brief refine_halfedges operates Loop halfedge refinement on the CPU
	 */
	void refine_halfedges() ;
	/**
	 * @brief refine_vertices operates Loop vertex refinement on the CPU
	 */
	void refine_vertices() ;

	// ----------- Utility functions -----------
	/**
	 * @brief compute_beta is a static mathematical utility function
	 * @param one_over_n is a scalar containing the value 1/valence.
	 * @return beta (see accompanying paper formulae)
	 */
	static float compute_beta(float one_over_n) ;
	/**
	 * @brief compute_gamma is a static mathematical utility function
	 * @param one_over_n is a scalar containing the value 1/valence.
	 * @return gamma (see accompanying paper formulae)
	 */
	static float compute_gamma(float one_over_n) ;
	/**
	 * @brief compute_ngamma is a static mathematical utility function
	 * @param one_over_n is a scalar containing the value 1/valence.
	 * @return n times gamma (see accompanying paper formulae)
	 */
	static float compute_ngamma(float one_over_n) ;
};

#endif
