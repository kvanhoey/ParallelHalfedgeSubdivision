#ifndef __MESH_H__
#define __MESH_H__

#include <vector>
#include <map>
#include <cmath>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "vec3.h"
#include "halfedge.h"
#include "crease.h"
#include "utils.h"
#include <array>
#include <cmath>
#include <chrono>

/**
 * @brief The Mesh class represents a mesh.
 *
 * It stores:
 * - the mesh topology as a buffer of HalfEdge (and HalfEdge_cage)
 * - the mesh geometry as a buffer of vec3 coordinates
 * - the mesh creases as a buffer of Crease
 */
class Mesh
{
protected:
	typedef std::vector<HalfEdge_cage> halfedge_buffer_cage ;	/*!< defines type for a buffer of HalfEdge_cage */
	typedef std::vector<HalfEdge> halfedge_buffer ;				/*!< defines type for a buffer of HalfEdge */
	typedef std::vector<vec3> vertex_buffer ;					/*!< defines type for a buffer of vec3 */
	typedef std::vector<Crease> crease_buffer ;					/*!< defines type for a buffer of Crease */

	int H_count ; /*!< halfedge counter represents the number of halfedges of the Mesh */
	int V_count ; /*!< vertex counter represents the number of vertices of the Mesh */
	int E_count ; /*!< edge counter represents the number of edges of the Mesh */
	int F_count ; /*!< face counter represents the number of faces of the Mesh */
	int C_count ; /*!< crease counter represents the number of creases of the Mesh */

	halfedge_buffer_cage halfedges_cage ; /*!< HalfEdge_cage buffer */
	halfedge_buffer halfedges ; /*!< HalfEdge buffer */
	vertex_buffer vertices ; /*!< vec3 buffer */
	crease_buffer creases ; /*!< Crease buffer */

public:
	// ----------- Constructor/destructor -----------
	/**
	 * @brief Mesh constructor from OBJ file
	 * @param filename path to an OBJ file
	 */
	Mesh(const std::string& filename) ;
	virtual ~Mesh() = default ;

	// ----------- Accessors -----------
	/**
	 * @brief H counts the number of halfedges at a given subdivision depth.
	 * @param depth subdivision depth.
	 * @pre depth should be <= 0 when called on base Mesh class.
	 * @return the number of halfedges if the mesh gets subdivided to depth.
	 */
	virtual int H(int depth = -1) const ;

	/**
	 * @brief V counts the number of vertices at a given subdivision depth.
	 * @param depth subdivision depth.
	 * @pre depth should be <= 0 when called on base Mesh class.
	 * @return the number of vertices if the mesh gets subdivided to depth.
	 */
	virtual int V(int depth = -1) const ;

	/**
	 * @brief F counts the number of faces at a given subdivision depth.
	 * @param depth subdivision depth.
	 * @pre depth should be <= 0 when called on base Mesh class.
	 * @return the number of faces if the mesh gets subdivided to depth.
	 */
	virtual int F(int depth = -1) const ;

	/**
	 * @brief E counts the number of edges at a given subdivision depth.
	 * @param depth subdivision depth.
	 * @pre depth should be <= 0 when called on base Mesh class.
	 * @return the number of edges if the mesh gets subdivided to depth.
	 */
	virtual int E(int depth = -1) const ;

	/**
	 * @brief C counts the number of creases at a given subdivision depth.
	 * @param depth subdivision depth.
	 * @pre depth should be <= 0 when called on base Mesh class.
	 * @return the number of creases if the mesh gets subdivided to depth.
	 */
	virtual int C(int depth = -1) const ;

	// ----------- Public member functions for mesh inspection -----------
	/**
	 * @brief check does a consistency check on the mesh.
	 * @return true if consistent, false (with error notification) if not.
	 */
	bool check() const ;

	/**
	 * @brief count_border_edges counts the amount of border edges (thus halfedges) in the mesh.
	 * @return the border edge counter.
	 */
	int count_border_edges() const ;

	/**
	 * @brief count_sharp_creases counts the amount of edges that are sharp creases.
	 * An edge is a sharp crease if it is a crease (i.e., its index exists in the crease buffer)
	 * and is sharp (i.e., its sharpness value is >0).
	 * @return the sharp crease counter.
	 */
	int count_sharp_creases() const ;

	/**
	 * @brief is_tri_only verifies if all polygons are triangles
	 * @return boolean
	 */
	bool is_tri_only() const ;

	/**
	 * @brief is_quad_only verifies if all polygons are quadrilaterals
	 * @return boolean
	 */
	bool is_quad_only() const ;

	/**
	 * @brief export_to_obj writes the current mesh to an OBJ file
	 * @param filename path to a file (that will be overwritten).
	 * @post Only topology and geometry is written to file: creases are not!
	 */
	void export_to_obj(const std::string& filename) const ;

	// ----------- Accessors for halfedge and crease values from specified buffers -----------
protected:
	/**
	 * @brief Twin accessor for the twin attribute
	 * @param buffer a vertex buffer
	 * @param h index to access
	 * @return index to the twin halfedge (which can be negative in case there is no Twin)
	 */
	virtual int Twin(const halfedge_buffer& buffer, int h) const final ;

	/**
	 * @brief Vert accessor for the vertex attribute of the vertex buffer
	 * @param buffer a vertex buffer
	 * @param h index to access
	 * @return index to the vertex the halfedge departs from
	 */
	virtual int Vert(const halfedge_buffer& buffer, int h) const final ;

	/**
	 * @brief Edge accessor for the edge attribute
	 * @param buffer a vertex buffer
	 * @param h index to access
	 * @return the index of the edge spanning the halfedge
	 */
	virtual int Edge(const halfedge_buffer& buffer, int h) const final ;

	/**
	 * @brief Prev accessor for the previous halfedge attribute
	 * @param buffer a vertex buffer
	 * @param h index to access
	 * @return the index of the previous halfedge within the face
	 */
	virtual int Prev(const halfedge_buffer_cage& buffer, int h) const ;

	/**
	 * @brief Next accessor for the next halfedge attribute
	 * @param buffer a vertex buffer
	 * @param h index to access
	 * @return the index of the next halfedge within the face
	 */
	virtual int Next(const halfedge_buffer_cage& buffer, int h) const ;

	/**
	 * @brief Face accessor for the face index of the halfedge
	 * @param buffer a vertex buffer
	 * @param h index of a halfedge to access
	 * @return the index of the face the halfedge lives in
	 */
	virtual int Face(const halfedge_buffer_cage& buffer, int h) const ;

	/**
	 * @brief Sharpness accessor for the sharpness value of a crease
	 * @param buffer a crease buffer
	 * @param c index into the crease buffer (or higher)
	 * @return the sharpness value, or 0 if index c is not a crease.
	 */
	virtual float Sharpness(const crease_buffer& buffer, int c) const final ;

	/**
	 * @brief NextC accessor for a crease's next crease.
	 * @param buffer a crease buffer
	 * @param c index into the crease buffer (or higher)
	 * @return the next crease index
	 */
	virtual int NextC(const crease_buffer& buffer, int c) const final ;

	/**
	 * @brief PrevC accessor for a crease's previous crease.
	 * @param buffer a crease buffer
	 * @param c index into the crease buffer (or higher)
	 * @return the previous crease index
	 */
	virtual int PrevC(const crease_buffer& buffer, int c) const final ;

	// ----------- utility functions for evaluating local configurations -----------
protected:
	/**
	 * @brief n_vertex_of_polygon computes how many vertices compose a polygon of the mesh (i.e.: it computes the 'n' in n-gon)
	 * @param h the index of a halfedge of the polygon
	 * @return the number of vertices of the polygon
	 */
	virtual int n_vertex_of_polygon(int h) const ;

	/**
	 * @brief is_border_halfedge determines if a halfedge lies at a border.
	 * Computed by checking if its twin halfedge exists.
	 * @param h_buffer a halfedge buffer
	 * @param h a halfedge index
	 * @return true if and only if h has no twin in buffer.
	 */
	bool is_border_halfedge(const halfedge_buffer& h_buffer, int h) const ;

	/**
	 * @brief is_border_vertex determines if a vertex lies at a border of the mesh
	 * @param h_buffer a halfedge buffer
	 * @param h index of a halfedge that points outward of the target vertex
	 * @return true if and only if the vertex from which h departs lies at a border.
	 */
	bool is_border_vertex(const halfedge_buffer& h_buffer, int h) const ;

	/**
	 * @brief is_crease_edge determines if an edge is a sharp crease
	 * @param c_buffer a crease buffer
	 * @param crease_id an id in the c_buffer, or beyond its limits
	 * @return false if the index does not point to a valid crease, or if the crease is not sharp (sharpness = 0).
	 */
	bool is_crease_edge(const crease_buffer& c_buffer, int crease_id) const ;

	/**
	 * @brief is_crease_halfedge determines if a halfedge spans a sharp crease edge
	 * @param h_buffer a halfedge buffer
	 * @param c_buffer a crease buffer
	 * @param h an index into the h_buffer
	 * @return false if the edge along h is not a crease.
	 */
	bool is_crease_halfedge(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;

	/**
	 * @brief vertex_sharpness computes the average sharpness of the edges around vertex Vert(h)
	 * @param h_buffer a halfedge buffer
	 * @param c_buffer a crease buffer
	 * @param h index into h_buffer of a halfedge outgoing from the target vertex
	 * @return a scalar that represents the vertex sharpness
	 */
	float vertex_sharpness(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;

	/**
	 * @brief vertex_sharpness_or_border is similar to #vertex_sharpness but returns a negative number instead of the vertex sharpness if the vertex is at the mesh border.
	 * @note vertex_sharpness_or_border is cheaper to compute than vertex_sharpness
	 * @param h_buffer a halfedge buffer
	 * @param c_buffer a crease buffer
	 * @param h index into h_buffer of a halfedge outgoing from the target vertex
	 * @return a scalar that represents the vertex sharpness, or -1 if the vertex lies at a mesh border.
	 */
	float vertex_sharpness_or_border(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;

	/**
	 * @brief vertex_edge_valence_or_border returns the valence of Vert(h), or -1 if Vert(h) is a border vertex.
	 * @note vertex_edge_valence_or_border is cheaper to compute than vertex_edge_valence
	 * @param h_buffer a halfedge buffer
	 * @param h index into h_buffer of a halfedge outgoing from the target vertex
	 * @return valence of the vertex Vert(h), or -1 if it is along a mesh border
	 */
	int vertex_edge_valence_or_border(const halfedge_buffer& h_buffer, int h) const ;

	/**
	 * @brief vertex_edge_valence returns the valence of Vert(h), robust wrt borders.
	 * @note see also: vertex_edge_valence_or_border
	 * @param h_buffer a halfedge buffer
	 * @param h index into h_buffer of a halfedge outgoing from the target vertex
	 * @return valence of the vertex Vert(h)
	 */
	int vertex_edge_valence(const halfedge_buffer& h_buffer, int h) const ;

	/**
	 * @brief vertex_halfedge_valence computes vertex valence in terms of adjacent halfedges (as opposed to edges, which is usually targeted).
	 * @note vertex_halfedge_valence is equivalent to edge_valence if it is not a border vertex. Otherwise, it is equivalent to edge_valence(h) - 1.
	 * @param h_buffer a halfedge buffer
	 * @param h index into h_buffer of a halfedge outgoing from the target vertex
	 * @return the vertex valence in terms of halfedges
	 */
	int vertex_halfedge_valence(const halfedge_buffer& h_buffer, int h) const ;

	/**
	 * @brief vertex_crease_valence_or_border returns similar to #vertex_crease_valence, or -1 if Vert(h) is a border vertex.
	 * @param h_buffer a halfedge buffer
	 * @param c_buffer a crease buffer
	 * @param h index into h_buffer of a halfedge outgoing from the target vertex
	 * @return
	 */
	int vertex_crease_valence_or_border(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;

	/**
	 * @brief vertex_crease_valence returns the number of sharp creases among the edges around Vert(h).
	 * @param h_buffer a halfedge buffer
	 * @param c_buffer a crease buffer
	 * @param h index into h_buffer of a halfedge outgoing from the target vertex
	 * @return
	 */
	int vertex_crease_valence(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;

	// ----------- Accessors for halfedge and crease values from the base mesh buffers -----------
private:
	virtual int Twin(int h) const final ;
	virtual int Vert(int h) const final ;
	virtual int Edge(int h) const final ;

	virtual int Prev(int h) const ;
	virtual int Next(int h) const ;
	/**
	 * @brief Next_safe accessor for the next halfedge attribute supporting invalid indices.
	 * Note: this accesses the current Mesh halfedge_buffer by default.
	 * @param h index to access
	 * @return the next halfedge index within the face, or a negative number.
	 */
	virtual int Next_safe(int h) const final ;

	virtual int Face(int h) const ;

	virtual float Sharpness(int c) const final ;
	virtual int NextC(int c) const final ;
	virtual int PrevC(int c) const final ;

	// ----------- Functions for loading and exporting from/to OBJ files. -----------
	void read_from_obj(const std::string& filename) ;
	static void read_obj_mesh_size(std::ifstream& open_file, int& h_count, int& v_count, int& f_count) ;
	crease_buffer read_obj_data(std::ifstream& open_file) ;

	void compute_and_set_twins() ;
	int compute_and_set_edges() ;
	void set_creases(const crease_buffer&) ;
	void compute_and_set_crease_neighbors() ;
	void set_boundaries_sharp() ;

	bool all_faces_are_ngons(int n) const ;

	/**
	 * @brief find_second_crease
	 * @pre h is the halfedge of a crease and a non-border vertex with exactly two adjacent creases.
	 * @param h a halfedge whose crease at Edge(h) has a sharpness value superior to 0.
	 * @return the crease id of a second crease
	 */
	int find_second_crease(int h) const ;
};


#endif
