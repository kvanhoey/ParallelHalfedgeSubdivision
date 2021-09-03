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
#include "timings.h"
#include "utils.h"
#include <array>
#include <cmath>
#include <chrono>
#include <algorithm>

class Mesh
{
protected:
	typedef std::vector<HalfEdge_cage> halfedge_buffer_cage ;
	typedef std::vector<HalfEdge> halfedge_buffer ;
	typedef std::vector<vec3> vertex_buffer ;
	typedef std::vector<Crease> crease_buffer ;

	int H_count, V_count, E_count, F_count, C_count ;

	halfedge_buffer_cage halfedges_cage ;
	halfedge_buffer halfedges ;
	vertex_buffer vertices ;
	crease_buffer creases ;

public:
	// ----------- Constructor/destructor -----------
	Mesh(const std::string& filename) ;
	virtual ~Mesh() = default ;

	// ----------- Accessors -----------
	virtual int H(int depth = -1) const ;
	virtual int V(int depth = -1) const ;
	virtual int F(int depth = -1) const ;
	virtual int E(int depth = -1) const ;
	virtual int C(int depth = -1) const ;

	// ----------- Public member functions for mesh inspection -----------
	bool check() const ;
	int count_border_edges() const ;
	int count_sharp_creases() const ;
	bool is_tri_only() const ;
	bool is_quad_only() const ;
	void export_to_obj(const std::string& filename) const ;

	// ----------- Accessors for halfedge and crease values from specified buffers -----------
protected:
	virtual int Twin(const halfedge_buffer& buffer, int h) const final ;
	virtual int Vert(const halfedge_buffer& buffer, int h) const final ;
	virtual int Edge(const halfedge_buffer& buffer, int h) const final ;

	virtual int Prev(const halfedge_buffer_cage& buffer, int h) const ;
	virtual int Next(const halfedge_buffer_cage& buffer, int h) const ;
	virtual int Face(const halfedge_buffer_cage& buffer, int h) const ;

	virtual int Next_safe(const halfedge_buffer_cage& buffer, int h) const final ;
	virtual int Next_safe(int h) const final ;

	virtual float Sharpness(const crease_buffer& buffer, int c) const final ;
	virtual int NextC(const crease_buffer& buffer, int c) const final ;
	virtual int PrevC(const crease_buffer& buffer, int c) const final ;

	// ----------- utility functions for evaluating local configurations -----------
protected:
	virtual int n_vertex_of_polygon(int h) const ;

	bool is_border_halfedge(const halfedge_buffer& buffer, int h) const ;
	bool is_border_vertex(const halfedge_buffer& h_buffer, int h) const ;
	/**
	 * @brief vertex_halfedge_valence compute vertex valence in terms of adjacent halfedges (as opposed to edges, which is usually targeted).
	 * @note vertex_halfedge_valence is equivalent to valence if it is not a border vertex. Otherwise, it is equivalent to valence(h) - 1.
	 * @param h a halfedge index
	 * @return halfedge valence of the vertex Vert(h)
	 */
	bool is_crease_edge(const crease_buffer& buffer, int crease_id) const ;
	bool is_crease_halfedge(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;

	float vertex_sharpness(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;
	float vertex_sharpness_or_border(const halfedge_buffer& h_buffer, const crease_buffer&, int h) const ;

	/**
	 * @brief vertex_edge_valence_or_border returns the valence of Vert(h), or -1 if v is a border vertex.
	 * @note vertex_edge_valence_or_border is cheaper to compute than vertex_edge_valence
	 * @param h a halfedge index
	 * @return valence of the vertex Vert(h), or -1 if it is along a mesh border
	 */
	int vertex_edge_valence_or_border(const halfedge_buffer& h_buffer, int h) const ;
	/**
	 * @brief vertex_edge_valence returns the valence of Vert(h), robust wrt borders.
	 * @note see also: vertex_edge_valence_or_border
	 * @param h a halfedge index
	 * @return valence of the vertex Vert(h)
	 */
	int vertex_edge_valence(const halfedge_buffer& buffer, int h) const ;

	int vertex_halfedge_valence(const halfedge_buffer& h_buffer, int h) const ;
	int vertex_crease_valence_or_border(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;
	int vertex_crease_valence(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;

	// ----------- Accessors for halfedge and crease values from the base mesh buffers -----------
private:
	virtual int Twin(int h) const final ;
	virtual int Vert(int h) const final ;
	virtual int Edge(int h) const final ;

	virtual int Prev(int h) const ;
	virtual int Next(int h) const ;
	virtual int Face(int h) const ;

	virtual float Sharpness(int c) const final ;
	virtual int NextC(int c) const final ;
	virtual int PrevC(int c) const final ;

	// ----------- Functions for loading and exporting from/to OBJ files. -----------
private:
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
