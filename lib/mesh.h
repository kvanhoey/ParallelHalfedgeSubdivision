#include <vector>
#include <map>
#include <cmath>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "halfedge.h"
#include "crease.h"
#include "timings.h"
#include <array>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <bits/stdc++.h>

#include <omp.h>

#define ENABLE_PARALLEL

# ifdef ENABLE_PARALLEL
#       ifndef CC_ATOMIC
#           define CC_ATOMIC          _Pragma("omp atomic" )
#       endif
#       ifndef CC_PARALLEL_FOR
#           define CC_PARALLEL_FOR    _Pragma("omp parallel for")
#       endif
#       ifndef CC_BARRIER
#           define CC_BARRIER         _Pragma("omp barrier")
#       endif
# else
#		define CC_ATOMIC
#		define CC_PARALLEL_FOR
#		define CC_BARRIER
# endif

#define _5_o_8 0.625f
#define _8_o_5 1.6f
#define _3_o_8 0.375f
#define _2pi 6.28318530718f


class Mesh
{
protected:
	typedef std::array<float,3> vec3 ;
	typedef std::vector<HalfEdge_cage> halfedge_buffer_cage ;
	typedef std::vector<HalfEdge> halfedge_buffer ;
	typedef std::vector<vec3> vertex_buffer ;
	typedef std::vector<Crease> crease_buffer ;

public:
	int H0, V0, E0, F0 ;
	int Hd, Vd, Ed, Fd, Cd ;
private:
	int _depth ;
	void init_depth() ;
protected:
	const int& depth() const ;

public:
	virtual int H(int depth = -1) const = 0 ;
	virtual int V(int depth = -1) const = 0 ;
	virtual int F(int depth = -1) const = 0 ;
	virtual int E(int depth = -1) const = 0 ;
	virtual int C(int depth = -1) const final ;

	halfedge_buffer_cage halfedges_cage ;
	halfedge_buffer halfedges ;
	vertex_buffer vertices ;
	crease_buffer creases ;

	Mesh(int H, int V, int E, int F) ;
	Mesh(const std::string& filename) ;
	virtual ~Mesh() = default ;

	bool check() const ;
	bool is_cage() const ;

	int count_border_edges() const ;
	int count_sharp_creases() const ;
	bool is_tri_only() const ;
	bool is_quad_only() const ;
	bool all_faces_are_ngons(int n) const ;

	void export_to_obj(const std::string& filename) const ;
	void export_to_obj_tri(std::ofstream& file) const ;
	void export_to_obj_contig_faces(std::ofstream& file) const ;

private:
	void read_from_obj(const std::string& filename) ;
	static void read_obj_mesh_size(std::ifstream& open_file, int& h_count, int& v_count, int& f_count) ;
	crease_buffer read_obj_data(std::ifstream& open_file) ;

	void compute_and_set_twins() ;
	void compute_and_set_twins_optim() ;
	int compute_and_set_edges() ;
	void set_creases(const crease_buffer&) ;
	void compute_and_set_crease_neighbors() ;
	void set_boundaries_sharp() ;

	/**
	 * @brief find_second_crease
	 * @pre h is the halfedge of a crease and a non-border vertex with exactly two adjacent creases.
	 * @param h a halfedge whose crease at Edge(h) has a sharpness value superior to 0.
	 * @return the crease id of a second crease
	 */
	int find_second_crease(int h) const ;

protected:
	void alloc_halfedge_buffer(int H) ;
	void alloc_vertex_buffer(int V) ;
	void alloc_crease_buffer(int E) ;

	virtual void set_depth(int d) ;

	virtual int Twin(int h) const final ;
	virtual int Prev(int h) const ;
	virtual int Next(int h) const ;
	virtual int Next_safe(int h) const final ;
	virtual int Vert(int h) const final ;
	virtual int Edge(int h) const final ;
	virtual int Face(int h) const ;

	virtual float Sigma(int c) const final ;
	virtual int NextC(int c) const final ;
	virtual int PrevC(int c) const final ;

	/**
	 * @brief vertex_edge_valence_or_border returns the valence of Vert(h), or -1 if v is a border vertex.
	 * @note vertex_edge_valence_or_border is cheaper to compute than vertex_edge_valence
	 * @param h a halfedge index
	 * @return valence of the vertex Vert(h), or -1 if it is along a mesh border
	 */
	int vertex_edge_valence_or_border(int h) const ;

	/**
	 * @brief vertex_edge_valence returns the valence of Vert(h), robust wrt borders.
	 * @note see also: vertex_edge_valence_or_border
	 * @param h a halfedge index
	 * @return valence of the vertex Vert(h)
	 */
	int vertex_edge_valence(int h) const ;

	/**
	 * @brief vertex_halfedge_valence compute vertex valence in terms of adjacent halfedges (as opposed to edges, which is usually targeted).
	 * @note vertex_halfedge_valence is equivalent to valence if it is not a border vertex. Otherwise, it is equivalent to valence(h) - 1.
	 * @param h a halfedge index
	 * @return halfedge valence of the vertex Vert(h)
	 */
	int vertex_halfedge_valence(int h) const ;


	int vertex_crease_valence_or_border(int h) const ;
	int vertex_crease_valence(int h) const ;

	float vertex_sharpness_or_border(int h) const ;
	float vertex_sharpness(int h) const ;

	bool is_border_halfedge(int h) const ;
	bool is_border_vertex(int h) const ;
	bool is_crease_halfedge(int h) const ;
	bool is_crease_edge(int e) const ;

	int n_vertex_of_polygon_check(int h) const ;
	virtual int n_vertex_of_polygon(int h) const ;

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

protected:
	typedef std::chrono::high_resolution_clock timer;
	typedef std::chrono::duration<float, std::milli> duration;
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

//	static Mesh_Loop tri() ;
//	static Mesh_Loop polyhedron() ;

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
