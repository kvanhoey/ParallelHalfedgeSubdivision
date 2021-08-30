#include "mesh_subdiv_loop_cpu.h"

Mesh_Subdiv_Loop_CPU::Mesh_Subdiv_Loop_CPU(const std::string &filename, uint depth):
	Mesh_Subdiv_Loop(filename, depth),
	Mesh_Subdiv_CPU(filename, depth),
	Mesh_Subdiv(filename, depth)
{}

void
Mesh_Subdiv_Loop_CPU::refine_halfedges()
{
	for (uint d = 0 ; d < D; ++d)
	{
		set_current_depth(d) ;

		halfedge_buffer& H_old = halfedge_subdiv_buffers[d] ;
		halfedge_buffer& H_new = halfedge_subdiv_buffers[d+1] ;
		const int Hd = H(d) ;
		const int Vd = V(d) ;
		const int Ed = E(d) ;

		const int _3Hd = 3 * Hd ;

		_PARALLEL_FOR
		for (int h = 0; h < Hd ; ++h)
		{
			const int _3h = 3 * h ;
			const int _3h_p_1 = _3h + 1 ;
			const int h_twin = Twin(H_old,h) ;
			const int h_edge = Edge(H_old,h) ;

			const int h_prev = Prev(h) ;
			const int h_prev_twin = Twin(H_old,h_prev) ;
			const int h_prev_edge = Edge(H_old,h_prev) ;

			HalfEdge& h0 = H_new[_3h + 0] ;
			HalfEdge& h1 = H_new[_3h_p_1] ;
			HalfEdge& h2 = H_new[_3h + 2] ;
			HalfEdge& h3 = H_new[_3Hd + h] ;

			h0.Twin = 3 * Next_safe(h_twin) + 2 ;
			h1.Twin = _3Hd + h ;
			h2.Twin = 3 * h_prev_twin ;
			h3.Twin = _3h_p_1 ;

			h0.Vert = Vert(H_old,h) ;
			h1.Vert = Vd + h_edge ;
			h2.Vert = Vd + h_prev_edge ;
			h3.Vert = h2.Vert ;

			h0.Edge = 2 * h_edge + (int(h) > h_twin ? 0 : 1)  ;
			h1.Edge = 2 * Ed + h ;
			h2.Edge = 2 * h_prev_edge + (int(h_prev) > h_prev_twin ? 1 : 0) ;
			h3.Edge = h1.Edge ;
		}
		_BARRIER
	}
}

void
Mesh_Subdiv_Loop_CPU::refine_vertices()
{
	// NOTE. This currently is only a copy-pasting of edgepoints_with_creases_branchless and vertexpoints_with_creases_branchless
	// within the same loop on halfedges.
	// TODO: one can probably exploit the single-loop case more optimally !

	for (uint d = 0 ; d < D; ++d)
	{
		set_current_depth(d) ;

		const halfedge_buffer& H_old = halfedge_subdiv_buffers[d] ;
		const crease_buffer& C_old = crease_subdiv_buffers[d] ;
		const vertex_buffer& V_old = vertex_subdiv_buffers[d] ;
		vertex_buffer& V_new = vertex_subdiv_buffers[d+1] ;

		const int Vd = V(d) ;
		const int Hd = H(d) ;

		_PARALLEL_FOR
		for (int h = 0; h < Hd ; ++h)
		{
			// edgepoints
			const int v = Vert(H_old,h) ;
			const int new_odd_pt_id = Vd + Edge(H_old,h) ;
			vec3& v_new = V_new[new_odd_pt_id] ;

			const int vp = Vert(H_old,Prev(h)) ;
			const int vn = Vert(H_old,Next(h)) ;
			const vec3& v_old_vx = V_old[v] ;
			const vec3& vp_old_vx = V_old[vp] ;
			const vec3& vn_old_vx = V_old[vn] ;

			const int c_id = Edge(H_old,h) ;
			const float sharpness = std::clamp(Sharpness(C_old,c_id),0.0f,1.0f) ;
			const bool is_border = is_border_halfedge(H_old,h) ;

			const vec3 increm_smooth_edge = 0.375f * v_old_vx + 0.125f * vp_old_vx ;
			const vec3 increm_sharp_edge = 0.5f * (is_border ? v_old_vx + vn_old_vx : v_old_vx) ;
			vec3 increm = lerp(increm_smooth_edge,increm_sharp_edge,sharpness) ;
			apply_atomic_vec3_increment(v_new, increm) ;

			// vertex points
			vec3& v_new_vx = V_new[v] ;

			const int n = vertex_edge_valence(H_old,h) ;
			const int n_creases = vertex_crease_valence(H_old, C_old, h) ;
			const int vertex_he_valence = vertex_halfedge_valence(H_old,h) ;

			const float edge_sharpness = Sharpness(C_old,c_id) ;
			const float vx_sharpness = n_creases < 2 ? 0.0f : vertex_sharpness(H_old, C_old, h) ; // n_creases < 0 ==> dart vertex ==> smooth

			const float lerp_alpha = std::clamp(vx_sharpness,0.0f,1.0f) ;

			// utility notations
			const float n_ = 1./float(n) ;
			const float beta = compute_beta(n_) ;
			const float beta_ = n_ - beta ;


			float edge_sharpness_factr = edge_sharpness < 1e-6 ? 0.0 : 1.0 ;

			// border correction
			float increm_corner_factr = 0.5f ;
			float increm_sharp_factr_vn = 0.375f ;
			float increm_sharp_factr_vb = 0.0f ;

			int vb = v ;
			if (is_border)
			{
				increm_corner_factr = 1.0f ;
				increm_sharp_factr_vn = 0.75f ;

				for (int h_it = Prev(h) ; ; h_it = Prev(h_it))
				{
					const int h_it_twin = Twin(H_old, h_it) ;
					if (h_it_twin < 0)
					{
						assert(is_crease_halfedge(H_old, C_old, h_it)) ;
						vb = Vert(H_old, h_it) ;
						increm_sharp_factr_vb = 0.125f ;
						break ;
					}

					h_it = h_it_twin ;
				}
			}

			const vec3 increm_corner_vx = v_old_vx / vertex_he_valence ;
			const vec3 increm_smooth_vx = beta_*v_old_vx + beta*vn_old_vx ;
			const vec3 increm_sharp_vx = edge_sharpness_factr * (0.125f * vn_old_vx + increm_sharp_factr_vn * v_old_vx + increm_sharp_factr_vb * V_old[vb]) ;

			if ((n==2) || n_creases > 2) // Corner vertex rule
			{
				apply_atomic_vec3_increment(v_new_vx,increm_corner_vx) ;
			}
			else if (vx_sharpness < 1e-6) // smooth
			{
				apply_atomic_vec3_increment(v_new_vx,increm_smooth_vx) ;
			}
			else // creased or blend
			{
				const vec3 incremV = lerp<vec3>(increm_corner_vx,increm_sharp_vx,lerp_alpha) ;
				apply_atomic_vec3_increment(v_new_vx, incremV) ;
			}
		}
	_BARRIER

	_depth = D ;
	}
}

float
Mesh_Subdiv_Loop_CPU::compute_beta(float one_over_n)
{
	return (_5_o_8 - std::pow((_3_o_8 + std::cos(_2pi * one_over_n) * 0.250),2)) * one_over_n ;
}

float
Mesh_Subdiv_Loop_CPU::compute_gamma(float one_over_n)
{
	return one_over_n * compute_ngamma(one_over_n);
}

float
Mesh_Subdiv_Loop_CPU::compute_ngamma(float one_over_n)
{
	return 1 - _8_o_5 * std::pow((_3_o_8 + std::cos(_2pi * one_over_n)* 0.250f),2);
}



