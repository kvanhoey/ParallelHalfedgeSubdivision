#include "mesh_subdiv_catmull-clark_cpu.h"

Mesh_Subdiv_CatmullClark_CPU::Mesh_Subdiv_CatmullClark_CPU(const std::string &filename, uint depth):
	Mesh_Subdiv_CatmullClark(filename, depth),
	Mesh_Subdiv_CPU(filename, depth),
	Mesh_Subdiv(filename, depth)
{}

void
Mesh_Subdiv_CatmullClark_CPU::refine_halfedges()
{
	for (uint d = 0 ; d < D; ++d)
	{
		set_current_depth(d) ;
		halfedge_buffer& H_old = halfedge_subdiv_buffers[d] ;
		halfedge_buffer& H_new = halfedge_subdiv_buffers[d+1] ;
		const int Hd = H(d) ;
		const int Vd = V(d) ;
		const int Fd = F(d) ;
		const int _2Ed = 2 * E(d) ;

		_PARALLEL_FOR
		for (int h_id = 0; h_id < Hd ; ++h_id)
		{
			const int _4h_id = 4 * h_id ;

			HalfEdge& h0 = H_new[_4h_id + 0] ;
			HalfEdge& h1 = H_new[_4h_id + 1] ;
			HalfEdge& h2 = H_new[_4h_id + 2] ;
			HalfEdge& h3 = H_new[_4h_id + 3] ;

			const int twin_id = Twin(H_old,h_id) ;
			const int edge_id = Edge(H_old,h_id) ;

			const int prev_id = Prev(h_id) ;
			const int prev_twin_id = Twin(H_old,prev_id) ;
			const int prev_edge_id = Edge(H_old,prev_id) ;

			h0.Twin = 4 * Next_safe(twin_id) + 3 ;
			h1.Twin = 4 * Next(h_id) + 2 ;
			h2.Twin = 4 * prev_id + 1 ;
			h3.Twin = 4 * prev_twin_id + 0 ;

			h0.Vert = Vert(H_old,h_id) ;
			h1.Vert = Vd + Fd + edge_id ;
			h2.Vert = Vd + Face(h_id) ;
			h3.Vert = Vd + Fd + prev_edge_id ;

			h0.Edge = 2 * edge_id + (int(h_id) > twin_id ? 0 : 1) ;
			h1.Edge = _2Ed + h_id ;
			h2.Edge = _2Ed + prev_id ;
			h3.Edge = 2 * prev_edge_id + (int(prev_id) > prev_twin_id ? 1 : 0) ;
		}
		_BARRIER
	}
}

void
Mesh_Subdiv_CatmullClark_CPU::refine_vertices()
{
	for (uint d = 0 ; d < D; ++d)
	{
		set_current_depth(d) ;
		refine_vertices_facepoints(d) ;
		refine_vertices_edgepoints(d) ;
		refine_vertices_vertexpoints(d) ;
	}
}

void
Mesh_Subdiv_CatmullClark_CPU::refine_vertices_facepoints(uint d)
{
	const halfedge_buffer& H_old = halfedge_subdiv_buffers[d] ;
	const vertex_buffer& V_old = vertex_subdiv_buffers[d] ;
	vertex_buffer& V_new = vertex_subdiv_buffers[d+1] ;

	const int Vd = V(d) ;
	const int Hd = H(d) ;

_PARALLEL_FOR
	for (int h_id = 0; h_id < Hd ; ++h_id)
	{
		const int vert_id = Vert(H_old, h_id) ;
		const int new_face_pt_id = Vd + Face(h_id) ;
		vec3& new_face_pt = V_new[new_face_pt_id] ;

		const int m = n_vertex_of_polygon(h_id) ;
		const vec3 increm = V_old[vert_id] / m ;

		apply_atomic_vec3_increment(new_face_pt, increm) ;
	}
_BARRIER
}

void
Mesh_Subdiv_CatmullClark_CPU::refine_vertices_edgepoints(uint d)
{
	const halfedge_buffer& H_old = halfedge_subdiv_buffers[d] ;
	const crease_buffer& C_old = crease_subdiv_buffers[d] ;
	const vertex_buffer& V_old = vertex_subdiv_buffers[d] ;
	vertex_buffer& V_new = vertex_subdiv_buffers[d+1] ;

	const int Vd = V(d) ;
	const int Hd = H(d) ;
	const int Fd = F(d) ;

_PARALLEL_FOR
	for (int h_id = 0; h_id < Hd ; ++h_id)
	{
		const int v_id = Vert(H_old,h_id) ;
		const int e_id = Edge(H_old, h_id) ;
		const int& c_id = e_id ;
		const int v_next_id = Vert(H_old, Next(h_id)) ;

		const int new_edge_pt_id = Vd + Fd + e_id ;
		const int new_face_pt_id = Vd + Face(h_id) ;

		vec3& new_edge_pt = V_new[new_edge_pt_id] ;
		const vec3& v_old = V_old[v_id] ;
		const vec3& new_face_pt = V_new[new_face_pt_id] ;
		const vec3& v_next_old = V_old[v_next_id] ;

		const bool is_border = is_border_halfedge(H_old, h_id) ;
		const vec3 increm_smooth = 0.25f * (v_old + new_face_pt) ; // Smooth rule B.2
		const vec3 increm_sharp = (is_border ? 1.0f : 0.5f) * lerp(v_old, v_next_old, 0.5f) ; // Crease rule: B.3

		const float sharpness = Sharpness(C_old, c_id) ;
		const float lerp_alpha = std::clamp(sharpness,0.0f,1.0f) ;
		const vec3 increm = lerp(increm_smooth,increm_sharp,lerp_alpha) ; // Blending crease rule: B.4

		apply_atomic_vec3_increment(new_edge_pt, increm) ;
	}
_BARRIER
}

void
Mesh_Subdiv_CatmullClark_CPU::refine_vertices_vertexpoints(uint d)
{
	const halfedge_buffer& H_old = halfedge_subdiv_buffers[d] ;
	const crease_buffer& C_old = crease_subdiv_buffers[d] ;
	const vertex_buffer& V_old = vertex_subdiv_buffers[d] ;
	vertex_buffer& V_new = vertex_subdiv_buffers[d+1] ;

	const int Vd = V(d) ;
	const int Hd = H(d) ;
	const int Fd = F(d) ;

	_PARALLEL_FOR
	for (int h_id = 0; h_id < Hd ; ++h_id)
	{
		const int vert_id = Vert(H_old, h_id) ;
		const int prev_id = Prev(h_id) ;
		const int new_face_pt_id = Vd + Face(h_id) ;
		const int new_edge_pt_id = Vd + Fd + Edge(H_old, h_id) ;
		const int new_prev_edge_pt_id = Vd + Fd + Edge(H_old, prev_id) ;
		const int c_id = Edge(H_old, h_id) ;
		const int c_prev_id = Edge(H_old, prev_id) ;

		vec3& new_vx_pt = V_new[vert_id] ;
		const vec3& old_vx_pt = V_old[vert_id] ;
		const vec3& new_face_pt = V_new[new_face_pt_id] ;
		const vec3& new_edge_pt = V_new[new_edge_pt_id] ;
		const vec3& new_prev_edge_pt = V_new[new_prev_edge_pt_id] ;

		const float& c_sharpness = Sharpness(C_old, c_id) ;
		const float& prev_sharpness = Sharpness(C_old, c_prev_id) ;
		const float c_sharpness_sgn = sgn(c_sharpness) ;

		const int h_id_twin = Twin(H_old, h_id) ;

		// determine local vertex configuration
		int vx_n_creases = int(c_sharpness_sgn) ;
		int vx_edge_valence = 1.0 ;
		float vx_sharpness = c_sharpness ;

		// loop around vx
		int h_id_it ;
		for (h_id_it = h_id_twin ; h_id_it >= 0 ; h_id_it = Twin(H_old, h_id_it))
		{
			h_id_it = Next(h_id_it) ;
			if (h_id_it == h_id)
				break ;

			vx_edge_valence++ ;

			const float s = Sharpness(C_old, Edge(H_old, h_id_it)) ;
			vx_sharpness += s ;
			vx_n_creases += sgn(s) ;
		}
		// if border, loop backward
		if (h_id_it < 0)
		{
			for (h_id_it = h_id ; h_id_it >= 0 ; h_id_it = Twin(H_old, h_id_it))
			{
				h_id_it = Prev(h_id_it) ;

				vx_edge_valence++ ;

				const float s = Sharpness(C_old, Edge(H_old, h_id_it)) ;
				vx_sharpness += s ;
				vx_n_creases += sgn(s) ;
			}
		}
		vx_sharpness /= float(vx_edge_valence) ;

		bool vx_is_border = h_id_it < 0 ;
		const int vx_halfedge_valence = vx_edge_valence + (vx_is_border ? -1 : 0) ;

		const vec3 increm_corner = old_vx_pt / float(vx_halfedge_valence) ; // corner vertex rule: C.3
		const vec3 increm_smooth = (4.0f * new_edge_pt - new_face_pt + (float(vx_edge_valence) - 3.0f) * old_vx_pt) / float (vx_edge_valence*vx_edge_valence) ; // Smooth rule: C.2
		vec3 increm_creased = c_sharpness_sgn * 0.25f * (new_edge_pt + old_vx_pt) ; // Creased vertex rule: C.5
		if (vx_is_border)
		{
			increm_creased = increm_creased + 0.25f * sgn(prev_sharpness) * (new_prev_edge_pt + old_vx_pt) ;
		}

		// apply the right incrementation
		vec3 increm ;
		if ((vx_edge_valence == 2) || (vx_n_creases > 2))
			increm = increm_corner ;
		else if (vx_n_creases < 2)
			increm = increm_smooth ;
		else // vx_n_creases = 2
			increm = lerp(increm_corner, increm_creased, vx_sharpness) ;

		apply_atomic_vec3_increment(new_vx_pt, increm) ;
	}
	_BARRIER
}
