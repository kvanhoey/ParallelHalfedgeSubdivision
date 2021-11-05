#ifndef __MESH_SUDBIV_CPU_H__
#define __MESH_SUDBIV_CPU_H__

#include "mesh_subdiv.h"

/**
 * @brief The Mesh_Subdiv_CPU (pure virtual) class specializes memory operations for the CPU, and implements crease refinement.
 */
class Mesh_Subdiv_CPU: virtual public Mesh_Subdiv
{
public:
	/**
	 * @brief Mesh_Subdiv_CPU constructor from OBJ file
	 * @param filename path to an OBJ file
	 * @param max_depth the depth at which to subdivide the mesh
	 */
	Mesh_Subdiv_CPU(const std::string& filename, uint max_depth) ;

protected:
	typedef std::chrono::high_resolution_clock timer;
	typedef std::chrono::duration<double, std::milli> duration;

	// ----------- Subdivision buffers on the CPU -----------
	std::vector<halfedge_buffer> halfedge_subdiv_buffers ; /*!< @brief halfedge_subdiv_buffers CPU halfedge subdivision buffers */
	std::vector<crease_buffer> crease_subdiv_buffers ; /*!< @brief crease_subdiv_buffers CPU crease subdivision buffers */
	std::vector<vertex_buffer> vertex_subdiv_buffers ; /*!< @brief vertex_subdiv_buffers CPU vertex subdivision buffers */

	// ----------- Buffer management -----------
	/**
	 * @brief allocate_subdiv_buffers allocates and initializes the CPU buffers in which subdivision will be computed.
	 */
	void allocate_subdiv_buffers() final ;
	/**
	 * @brief readback_from_subdiv_buffers copies the result from the CPU subdivision buffers into the current buffer
	 */
	void readback_from_subdiv_buffers() final ;
	/**
	 * @brief refine_creases operates crease refinement in the CPU crease subdivision buffers.
	 */
	void refine_creases() final ;

	void refine_halfedges_and_time(int n_repetitions) final;

	// ----------- Utility function for OpenMP atomic adds -----------
	/**
	 * @brief apply_atomic_vec3_increment applies an atomic OpenMP increment on vertex coordinates
	 * @param v reference to a vertex coordinate to increment
	 * @param v_increm the coordinate incrementation value.
	 */
	static void apply_atomic_vec3_increment(vec3& v, const vec3& v_increm) ;

};

#endif
