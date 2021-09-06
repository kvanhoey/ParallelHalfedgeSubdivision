#ifndef __MESH_SUBDIV_H__
#define __MESH_SUBDIV_H__

#include "mesh.h"

/**
 * @brief The Mesh_Subdiv (pure virtual) class defines useful fields and mandatory methods for any specialized class that implements subdivision.
 */
class Mesh_Subdiv: public Mesh
{
	// ----------- Constructor and main subdivision function -----------
public:
	/**
	 * @brief Mesh_Subdiv constructor from OBJ file
	 * @param filename path to an OBJ file
	 * @param max_depth the depth at which to subdivide the mesh
	 */
	Mesh_Subdiv(const std::string& filename, uint max_depth) ;

	/**
	 * @brief subdivide is the main public subdivision routine that can be called from outside.
	 */
	virtual void subdivide() final ;

	// ----------- Internal state of subdivision -----------
protected:
	const uint d_max ; /*!< the target (maximal) subdivision depth */
	uint d_cur ; /*!< the current subdivision depth */
	bool subdivided ; /*!< true if subdivision has started (i.e., at least achieved one level of subdivision) */
	bool finalized ; /*!< true if subdivision has finished (i.e., achieved subdivision level d_max) */

	/**
	 * @brief C counts the number of creases at a given subdivision depth.
	 * @param depth subdivision depth or -1
	 * @return the number of creases at subdivision level depth, or at level d_cur (if depth = -1).
	 */
	virtual int C(int depth = -1) const final ;

	/**
	 * @brief set_current_depth sets an internal state with the current depth.
	 * This allows to write low-level routines (e.g., Next, Prev, Face) that rely on this state.
	 * Whenever iterating over subdivision depth, remember to first call set_current_depth.
	 * @param depth the current depth.
	 */
	void set_current_depth(int depth) ;

	// ----------- Mandatory overrides for derived classes -----------
	/**
	 * @brief allocate_subdiv_buffers (pure virtual) should allocate and initialize the buffers in which subdivision will be computed. It is called at the start of call to subdivision.
	 */
	virtual void allocate_subdiv_buffers() = 0 ;

	/**
	 * @brief readback_from_subdiv_buffers (pure virtual) should copy the result from the subdivision buffers into the current buffer
	 */
	virtual void readback_from_subdiv_buffers() = 0 ;

	/**
	 * @brief refine_halfedges (pure virtual) should operate the halfedge refinement in the halfedge subdivision buffers.
	 */
	virtual void refine_halfedges() = 0 ;

	/**
	 * @brief refine_creases (pure virtual) should operate crease refinement in the crease subdivision buffers.
	 */
	virtual void refine_creases() = 0 ;

	/**
	 * @brief refine_vertices (pure virtual) should operate vertex refinement in the vertex subdivision buffers.
	 */
	virtual void refine_vertices() = 0 ;

	// ----------- Finalize and lock the class -----------
private:
	void finalize_subdivision() ;
};

#endif
