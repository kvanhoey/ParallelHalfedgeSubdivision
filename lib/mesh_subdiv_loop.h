#ifndef __MESH_SUDBIV_LOOP_H__
#define __MESH_SUDBIV_LOOP_H__

#include "mesh_subdiv.h"

/**
 * @brief The Mesh_Subdiv_Loop class specializes routines of Mesh_Subdiv for Loop subdivision
 */
class Mesh_Subdiv_Loop: virtual public Mesh_Subdiv
{
	// ----------- Constructor -----------
public:
	/**
	 * @brief Mesh_Subdiv_Loop constructor from OBJ file
	 * @pre the mesh should be triangle-only
	 * @param filename path to an OBJ file
	 * @param max_depth the depth at which to subdivide the mesh
	 */
	Mesh_Subdiv_Loop(const std::string& filename, uint max_depth) ;

	// ----------- Override of accessors -----------
	int H(int depth = -1) const ;
	int V(int depth = -1) const ;
	int F(int depth = -1) const ;
	int E(int depth = -1) const ;

protected:
	// override with analytic versions
	/**
	 * @brief Prev is the (faster) analytic override of the computation of the previous index of a halfedge, specialized for Loop subdivision
	 * @param h index of a halfedge
	 * @return the previous halfedge in a face, analytically computed.
	 */
	int Prev(int h) const ;
	/**
	 * @brief Next is the (faster) analytic override of the computation of the next index of a halfedge, specialized for Loop subdivision
	 * @param h index of a halfedge
	 * @return the next halfedge in a face, analytically computed.
	 */
	int Next(int h) const ;
	/**
	 * @brief Face is the (faster) analytic override of the computation of the face index of a halfedge, specialized for Loop subdivision
	 * @param h index of a halfedge to access
	 * @return the index of the face the halfedge lives in
	 */
	int Face(int h) const ;

	/**
	 * @brief n_vertex_of_polygon is the (faster) analytic override of the computation of #n_vertex_of_polygon, specialized for Loop subdivision.
	 * @param h the index of a halfedge of the polygon
	 * @return the number of vertices of the polygon
	 */
	virtual int n_vertex_of_polygon(int h) const final ;
};


#endif
