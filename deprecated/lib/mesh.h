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
#include "helpers.h"
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

public:
	int H0, V0, E0, F0 ;

protected:
	int _depth ;

public:
	virtual int H(int depth = -1) const ;
	virtual int V(int depth = -1) const ;
	virtual int F(int depth = -1) const ;
	virtual int E(int depth = -1) const ;
	virtual int C(int depth = -1) const final ;

	halfedge_buffer_cage halfedges_cage ;
	halfedge_buffer halfedges ;
	vertex_buffer vertices ;
	crease_buffer creases ;

	Mesh(int H, int V, int E, int F) ;
	Mesh(const std::string& filename) ;
	virtual ~Mesh() = default ;

	bool check() const ;

	int count_border_edges() const ;
	int count_sharp_creases() const ;
	bool is_tri_only() const ;
	bool is_quad_only() const ;

	void export_to_obj(const std::string& filename) const ;

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

private:
	void alloc_halfedge_buffer(int H) ;
	void alloc_vertex_buffer(int V) ;
	void alloc_crease_buffer(int E) ;

private:
	virtual int Twin(int h) const final ;
	virtual int Prev(int h) const ;
	virtual int Next(int h) const ;
	virtual int Vert(int h) const final ;
	virtual int Edge(int h) const final ;
	virtual int Face(int h) const ;

	virtual float Sharpness(int c) const final ;
	virtual int NextC(int c) const final ;
	virtual int PrevC(int c) const final ;

protected:
	virtual int Next_safe(int h) const final ;

	virtual int Twin(const halfedge_buffer& buffer, int h) const final ;
	virtual int Prev(const halfedge_buffer_cage& buffer, int h) const ;
	virtual int Next(const halfedge_buffer_cage& buffer, int h) const ;
	virtual int Next_safe(const halfedge_buffer_cage& buffer, int h) const final ;
	virtual int Vert(const halfedge_buffer& buffer, int h) const final ;
	virtual int Edge(const halfedge_buffer& buffer, int h) const final ;
	virtual int Face(const halfedge_buffer_cage& buffer, int h) const ;

	virtual float Sharpness(const crease_buffer& buffer, int c) const final ;
	virtual int NextC(const crease_buffer& buffer, int c) const final ;
	virtual int PrevC(const crease_buffer& buffer, int c) const final ;

	/**
	 * @brief vertex_edge_valence_or_border returns the valence of Vert(h), or -1 if v is a border vertex.
	 * @note vertex_edge_valence_or_border is cheaper to compute than vertex_edge_valence
	 * @param h a halfedge index
	 * @return valence of the vertex Vert(h), or -1 if it is along a mesh border
	 */
	int vertex_edge_valence_or_border(const halfedge_buffer& h_buffer, int h) const ;

	float vertex_sharpness_or_border(const halfedge_buffer& h_buffer, const crease_buffer&, int h) const ;

	bool is_border_vertex(const halfedge_buffer& h_buffer, int h) const ;
	/**
	 * @brief vertex_halfedge_valence compute vertex valence in terms of adjacent halfedges (as opposed to edges, which is usually targeted).
	 * @note vertex_halfedge_valence is equivalent to valence if it is not a border vertex. Otherwise, it is equivalent to valence(h) - 1.
	 * @param h a halfedge index
	 * @return halfedge valence of the vertex Vert(h)
	 */
	int vertex_halfedge_valence(const halfedge_buffer& h_buffer, int h) const ;

	int vertex_crease_valence_or_border(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;
	bool is_crease_halfedge(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;
	int vertex_crease_valence(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;
	int vertex_edge_valence(const halfedge_buffer& buffer, int h) const ;

	float vertex_sharpness(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const ;

	bool is_border_halfedge(const halfedge_buffer& buffer, int h) const ;
	bool is_crease_edge(const crease_buffer& buffer, int e) const ;

	int n_vertex_of_polygon_cage(int h) const ;
	virtual int n_vertex_of_polygon(int h) const ;

private:
	/**
	 * @brief vertex_edge_valence returns the valence of Vert(h), robust wrt borders.
	 * @note see also: vertex_edge_valence_or_border
	 * @param h a halfedge index
	 * @return valence of the vertex Vert(h)
	 */
	int vertex_edge_valence(int h) const ;



	int vertex_crease_valence(int h) const ;

//	bool is_crease_halfedge(int h) const ;
	bool is_border_halfedge(int h) const ;

};


class MeshSubdivision: public Mesh
{
public:
	MeshSubdivision(int H, int V, int E, int F): Mesh(H,V,E,F) {}
	MeshSubdivision(const std::string& filename): Mesh(filename) {}

	virtual void refine_step() final ;
   virtual void refine_step_inplace() final ;

protected:
	static void init_vertex_buffer(vertex_buffer& V, uint start_index = 0) ;

	virtual void refine_halfedges(halfedge_buffer&) const = 0 ;
	virtual void refine_vertices(vertex_buffer&) const = 0 ;
	virtual void refine_vertices_with_creases(vertex_buffer&) const = 0 ;
	virtual void refine_vertices_inplace() = 0 ;
	// virtual void refine_vertices_with_creases_inplace() = 0 ;

	virtual void refine_creases(crease_buffer&) const  ;
	virtual void refine_creases_branchless(crease_buffer&) const  ;

   static void apply_atomic_vec3_increment(vec3& v_old, const vec3& v_increm) ;

protected:
	typedef std::chrono::high_resolution_clock timer;
	typedef std::chrono::duration<double, std::milli> duration;
public:
	virtual Timings bench_refine_step(bool refine_he, bool refine_cr, bool refine_vx, uint repetitions, bool save_result=false) final ;
};

class Mesh_CC: public MeshSubdivision
{

public:
	Mesh_CC(int H, int V, int E, int F): MeshSubdivision(H,V,E,F) {}
	Mesh_CC(const std::string& filename): MeshSubdivision(filename) {}

	int H(int depth = -1) const ;
	int V(int depth = -1) const ;
	int F(int depth = -1) const ;
	int E(int depth = -1) const ;

	// override with analytic versions
	int Prev(int h) const ;
	int Next(int h) const ;
	int Face(int h) const ;

//	static Mesh_CC quad() ;
//	static Mesh_CC cube() ;
//	static Mesh_CC fig1_left() ;

private:
	void refine_halfedges(halfedge_buffer&) const ;
	void refine_vertices(vertex_buffer&) const ;
	void refine_vertices_with_creases(vertex_buffer&) const ;
	void refine_vertices_inplace() ;

	void facepoints(vertex_buffer&) const ;
	void edgepoints(vertex_buffer&) const ;
	void edgepoints_with_creases(vertex_buffer&) const ;
	void vertexpoints(vertex_buffer&) const ;
	void vertexpoints_with_creases(vertex_buffer&) const ;
	void vertexpoints_inplace() ;
	void vertexpoints_inplace_pass1() ;
	void vertexpoints_inplace_pass2() ;

	int n_vertex_of_polygon(int h) const ;
};

class Mesh_Loop: public MeshSubdivision
{
public:
	Mesh_Loop(int H, int V, int E, int F): MeshSubdivision(H,V,E,F) {}
	Mesh_Loop(const std::string& filename): MeshSubdivision(filename) {}

	int H(int depth = -1) const ;
	int V(int depth = -1) const ;
	int F(int depth = -1) const ;
	int E(int depth = -1) const ;

	// override with analytic versions
	int Prev(int h) const ;
	int Next(int h) const ;
	int Face(int h) const ;

private:
	void refine_halfedges(halfedge_buffer& new_he) const ;
	void refine_halfedges_old(halfedge_buffer& new_he) const ;
	void refine_vertices(vertex_buffer& V_new) const ;
	void refine_vertices_with_creases(vertex_buffer& V_new) const ;
	void refine_vertices_twosteps(vertex_buffer& V_new) const ;
	void refine_vertices_with_creases_twosteps(vertex_buffer& V_new) const ;
	void refine_vertices_with_creases_twosteps_branchless(vertex_buffer& V_new) const ;
	void refine_vertices_inplace() ;

	void edgepoints(vertex_buffer& V_new) const ;
	void edgepoints_with_creases(vertex_buffer& V_new) const ;
	void edgepoints_with_creases_branchless(vertex_buffer& V_new) const ;

	void vertexpoints(vertex_buffer& V_new) const ;
	void vertexpoints_with_creases(vertex_buffer& V_new) const ;
	void vertexpoints_with_creases_branchless(vertex_buffer& V_new) const ;

	void allpoints(vertex_buffer& V_new) const ;
	void allpoints_with_creases(vertex_buffer& V_new) const ;
	void allpoints_with_creases_branchless(vertex_buffer& V_new) const ;

	void vertexpoints_inplace() ;
	void vertexpoints_inplace_pass1() ;
	void vertexpoints_inplace_pass2() ;

   static float compute_beta(float one_over_n) ;
	static float compute_gamma(float one_over_n) ;
	static float compute_ngamma(float one_over_n) ;

	int n_vertex_of_polygon(int h) const ;
};


#endif