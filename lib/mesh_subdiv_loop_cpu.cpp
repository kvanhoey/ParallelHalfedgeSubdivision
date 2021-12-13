#include "mesh_subdiv_loop_cpu.h"

Mesh_Subdiv_Loop_CPU::Mesh_Subdiv_Loop_CPU(const std::string &filename, uint depth):
	Mesh_Subdiv_Loop(filename, depth),
	Mesh_Subdiv_CPU(filename, depth),
	Mesh_Subdiv(filename, depth)
{}

// ----------- Member functions that do the actual subdivision -----------
void
Mesh_Subdiv_Loop_CPU::refine_halfedges()
{
	for (uint d = 0 ; d < d_max; ++d)
	{
		set_current_depth(d) ;

		halfedge_buffer& H_old = halfedge_subdiv_buffers[d] ;
		halfedge_buffer& H_new = halfedge_subdiv_buffers[d+1] ;
		const int Hd = H(d) ;
		const int Vd = V(d) ;
		const int Ed = E(d) ;

		const int _3Hd = 3 * Hd ;

		_PARALLEL_FOR
		for (int h_id = 0; h_id < Hd ; ++h_id)
		{
			const int twin_id = Twin(H_old,h_id) ;
			const int edge_id = Edge(H_old,h_id) ;
			const int next_id_safe = twin_id < 0 ? twin_id : Next(twin_id) ;

			const int prev_id = Prev(h_id) ;
			const int prev_twin_id = Twin(H_old,prev_id) ;
			const int prev_edge_id = Edge(H_old,prev_id) ;

			const int _3h = 3 * h_id ;
			const int _3h_p_1 = _3h + 1 ;

			HalfEdge& h0 = H_new[_3h + 0] ;
			HalfEdge& h1 = H_new[_3h_p_1] ;
			HalfEdge& h2 = H_new[_3h + 2] ;
			HalfEdge& h3 = H_new[_3Hd + h_id] ;

			h0.Twin = 3 * next_id_safe + 2 ;
			h1.Twin = _3Hd + h_id ;
			h2.Twin = 3 * prev_twin_id ;
			h3.Twin = _3h_p_1 ;

			h0.Vert = Vert(H_old,h_id) ;
			h1.Vert = Vd + edge_id ;
			h2.Vert = Vd + prev_edge_id ;
			h3.Vert = h2.Vert ;

			h0.Edge = 2 * edge_id + (int(h_id) > twin_id ? 0 : 1)  ;
			h1.Edge = 2 * Ed + h_id ;
			h2.Edge = 2 * prev_edge_id + (int(prev_id) > prev_twin_id ? 1 : 0) ;
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

	for (uint d = 0 ; d < d_max; ++d)
	{
		set_current_depth(d) ;

		const halfedge_buffer& H_old = halfedge_subdiv_buffers[d] ;
		const crease_buffer& C_old = crease_subdiv_buffers[d] ;
		const vertex_buffer& V_old = vertex_subdiv_buffers[d] ;
		vertex_buffer& V_new = vertex_subdiv_buffers[d+1] ;

		const int Vd = V(d) ;
		const int Hd = H(d) ;

		_PARALLEL_FOR
		for (int h_id = 0; h_id < Hd ; ++h_id)
		{
			const int v_id = Vert(H_old,h_id) ;
			const int new_odd_pt_id = Vd + Edge(H_old,h_id) ;
			vec3& v_new = V_new[new_odd_pt_id] ;
			vec3& new_even_pt_vx = V_new[v_id] ;

			// edgepoints
			const int v_prev_id = Vert(H_old,Prev(h_id)) ;
			const int v_next_id = Vert(H_old,Next(h_id)) ;

			const int c_id = Edge(H_old,h_id) ;

			const vec3& v_old_vx = V_old[v_id] ;
			const vec3& v_prev_old_vx = V_old[v_prev_id] ;
			const vec3& v_next_old_vx = V_old[v_next_id] ;

			const bool is_border = is_border_halfedge(H_old,h_id) ;
			const float sharpness = std::clamp(Sharpness(C_old,c_id),0.0f,1.0f) ;

			const vec3 increm_smooth_edge = 0.375f * v_old_vx + 0.125f * v_prev_old_vx ;
			const vec3 increm_sharp_edge = 0.5f * (is_border ? v_old_vx + v_next_old_vx : v_old_vx) ;
			vec3 increm = lerp(increm_smooth_edge,increm_sharp_edge,sharpness) ;
			apply_atomic_vec3_increment(v_new, increm) ;

			// vertex points
			const int n = vertex_edge_valence(H_old,h_id) ;
			const int n_creases = vertex_crease_valence(H_old, C_old, h_id) ;
			const int vertex_he_valence = vertex_halfedge_valence(H_old,h_id) ;

			const float edge_sharpness = Sharpness(C_old,c_id) ;
			const float vx_sharpness = n_creases < 2 ? 0.0f :  // n_creases < 0 ==> dart vertex ==> smooth
													   0.5f * vertex_sharpness_sum(H_old, C_old, h_id) ; // only used iff exactly 2 adjacent crease edges

			const float lerp_alpha = std::clamp(vx_sharpness,0.0f,1.0f) ;

			// utility notations
			const float n_ = 1./float(n) ;
			const float beta = compute_beta(n_) ;
			const float beta_ = n_ - beta ;


			float edge_sharpness_factr = edge_sharpness < 1e-6 ? 0.0 : 1.0 ;

			// border correction
			float increm_sharp_factr_v_old = 0.375f ;
			float increm_sharp_factr_v_border = 0.0f ;

			int v_border_id = v_id ;
			if (is_border)
			{
				increm_sharp_factr_v_old = 0.75f ;

				for (int h_id_it = Prev(h_id) ; ; h_id_it = Prev(h_id_it))
				{
					const int h_it_twin = Twin(H_old, h_id_it) ;
					if (h_it_twin < 0)
					{
						assert(is_crease_halfedge(H_old, C_old, h_id_it)) ;
						v_border_id = Vert(H_old, h_id_it) ;
						increm_sharp_factr_v_border = 0.125f ;
						break ;
					}

					h_id_it = h_it_twin ;
				}
			}

			const vec3 increm_corner_vx = v_old_vx / vertex_he_valence ;
			const vec3 increm_smooth_vx = beta_ * v_old_vx + beta * v_next_old_vx ;
			const vec3 increm_sharp_vx = edge_sharpness_factr * (0.125f * v_next_old_vx + increm_sharp_factr_v_old * v_old_vx + increm_sharp_factr_v_border * V_old[v_border_id]) ;

			if ((n==2) || n_creases > 2) // Corner vertex rule
			{
				apply_atomic_vec3_increment(new_even_pt_vx,increm_corner_vx) ;
			}
			else if (vx_sharpness < 1e-6) // smooth
			{
				apply_atomic_vec3_increment(new_even_pt_vx,increm_smooth_vx) ;
			}
			else // creased or blend
			{
				const vec3 incremV = lerp(increm_corner_vx,increm_sharp_vx,lerp_alpha) ;
				apply_atomic_vec3_increment(new_even_pt_vx, incremV) ;
			}
		}
	_BARRIER
	}

	set_current_depth(d_max) ;
}

// ----------- Utility functions -----------
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



