#ifndef __MESH_SUDBIV_CATMULLCLARK_CPU_H__
#define __MESH_SUDBIV_CATMULLCLARK_CPU_H__

#include "mesh_subdiv_catmull-clark.h"
#include "mesh_subdiv_cpu.h"

/**
 * @brief The Mesh_Subdiv_CatmullClark_CPU class implements Catmull-Clark subdivision on the CPU
 */
class Mesh_Subdiv_CatmullClark_CPU: public Mesh_Subdiv_CatmullClark, Mesh_Subdiv_CPU
{
public:
	/**
	 * @brief Mesh_Subdiv_CatmullClark_CPU constructor from OBJ file
	 * @param filename path to an OBJ file
	 * @param max_depth the depth at which to subdivide the mesh
	 */
	Mesh_Subdiv_CatmullClark_CPU(const std::string& filename, uint max_depth);

protected:
	// ----------- Member functions that do the actual subdivision -----------
	/**
	 * @brief refine_halfedges operates Catmull-Clark halfedge refinement on the CPU
	 */
	void refine_halfedges() ;
	/**
	 * @brief refine_vertices operates Catmull-Clark vertex refinement on the CPU
	 */
	void refine_vertices() ;

	// ----------- Utility functions -----------
	/**
	 * @brief refine_vertices_facepoints operates face point refinement on the CPU
	 * @param d current depth
	 */
	void refine_vertices_facepoints(uint d) ;
	/**
	 * @brief refine_vertices_edgepoints operates edge point refinement on the CPU
	 * @param d current depth
	 */
	void refine_vertices_edgepoints(uint d) ;
	/**
	 * @brief refine_vertices_vertexpoints operates vertex point refinement on the CPU
	 * @param d current depth
	 */
	void refine_vertices_vertexpoints(uint d) ;
};

#endif
