#ifndef __MESH_SUDBIV_CATMULLCLARK_H__
#define __MESH_SUDBIV_CATMULLCLARK_H__

#include "mesh_subdiv.h"

/**
 * @brief The Mesh_Subdiv_CatmullClark (pure virtual) class specializes routines of Mesh_Subdiv for Catmull-Clark subdivision
 */
class Mesh_Subdiv_CatmullClark: virtual public Mesh_Subdiv
{
	// ----------- Constructor -----------
public:
	/**
	 * @brief Mesh_Subdiv_CatmullClark constructor from OBJ file
	 * @param filename path to an OBJ file
	 * @param max_depth the depth at which to subdivide the mesh
	 */
	Mesh_Subdiv_CatmullClark(const std::string& filename, uint max_depth) ;

	// ----------- Override of accessors -----------
	virtual int H(int depth = -1) const final ;
	virtual int V(int depth = -1) const final ;
	virtual int F(int depth = -1) const final ;
	virtual int E(int depth = -1) const final ;

protected:
	/**
	 * @brief Prev is the (faster) analytic override of the computation of the previous index of a halfedge, specialized for Catmull-Clark subdivision
	 * @param h index of a halfedge
	 * @return the previous halfedge id, analytically computed.
	 */
	int Prev(int h) const ;
	/**
	 * @brief Next is the (faster) analytic override of the computation of the next index of a halfedge, specialized for Catmull-Clark subdivision
	 * @param h index of a halfedge
	 * @return the next halfedge in a face, analytically computed.
	 */
	int Next(int h) const ;
	/**
	 * @brief Face is the (faster) analytic override of the computation of the face index of a halfedge, specialized for Catmull-Clark subdivision
	 * @param h index of a halfedge to access
	 * @return the index of the face the halfedge lives in
	 */
	int Face(int h) const ;

	/**
	 * @brief n_vertex_of_polygon is the (faster) analytic override of the computation of #n_vertex_of_polygon, specialized for Catmull-Clark subdivision.
	 * @param h the index of a halfedge of the polygon
	 * @return the number of vertices of the polygon
	 */
	virtual int n_vertex_of_polygon(int h) const final ;
};

#endif
