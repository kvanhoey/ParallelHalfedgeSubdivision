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

			const int h_twin_id = Twin(H_old,_4h_id) ;
			const int h_edge_id = Edge(H_old,_4h_id) ;

			const int h_prev_id = Prev(h_id) ;
			const int h_prev_twin_id = Twin(H_old,h_prev_id) ;
			const int h_prev_edge_id = Edge(H_old,h_prev_id) ;

			h0.Twin = 4 * Next_safe(h_twin_id) + 3 ;
			h1.Twin = 4 * Next(h_id) + 2 ;
			h2.Twin = 4 * h_prev_id + 1 ;
			h3.Twin = 4 * h_prev_twin_id + 0 ;

			h0.Vert = Vert(H_old,h_id) ;
			h1.Vert = Vd + Fd + h_edge_id ;
			h2.Vert = Vd + Face(h_id) ;
			h3.Vert = Vd + Fd + h_prev_edge_id ;

			h0.Edge = 2 * h_edge_id + (int(h_id) > h_twin_id ? 0 : 1) ;
			h1.Edge = _2Ed + h_id ;
			h2.Edge = _2Ed + h_prev_id ;
			h3.Edge = 2 * h_prev_edge_id + (int(h_prev_id) > h_prev_twin_id ? 1 : 0) ;
		}
		_BARRIER
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

		const int new_edge_pt_id = Vd + Fd + e_id ;

		const vec3& v_old = V_old[v_id] ;
		vec3& new_edge_pt = V_new[new_edge_pt_id] ;

		if (is_border_halfedge(H_old,h_id)) // Boundary rule: B.1
		{
			const int vn_id = Vert(H_old, Next(h_id)) ;
			const vec3& vn_old = V_old[vn_id] ;

			new_edge_pt = 0.5f * (v_old + vn_old) ; // at border, there is a single write, no need for atomic
		}
		else
		{
			const int new_face_pt_id = Vd + Face(h_id) ;
			const vec3& new_face_pt = V_new[new_face_pt_id] ;
			const vec3 increm_smooth = 0.25f * (v_old + new_face_pt) ; // Smooth rule B.2
			const vec3 increm_sharp = 0.5f * v_old ; // Crease rule: B.3

			const float sharpness = Sharpness(C_old, c_id) ;
			const float alpha = std::clamp(sharpness,0.0f,1.0f) ;
			const vec3 increm = lerp(increm_smooth,increm_sharp,alpha) ; // Blending crease rule: B.4

			apply_atomic_vec3_increment(new_edge_pt, increm) ;
		}
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
		const int n = vertex_edge_valence_or_border(H_old, h_id) ;
		const int v = Vert(H_old, h_id) ;
		vec3& new_vx_point = V_new[v] ;
		const vec3& old_vx_point = V_old[v] ;

		if (n < 0) // Boundary rule: C.1
		{
			const float vertex_he_valence = float(vertex_halfedge_valence(H_old, h_id)) ;
			const vec3 increm_border = old_vx_point / vertex_he_valence ;
			apply_atomic_vec3_increment(new_vx_point, increm_border) ;
		}
		else
		{
			const int n_creases = vertex_crease_valence_or_border(H_old, C_old, h_id) ;
			if ((n == 2) || n_creases > 2) // Corner vertex rule: C.3
			{
				const float vx_halfedge_valence = 1.0f / float(vertex_halfedge_valence(H_old, h_id)) ;
				const vec3 increm_corner = vx_halfedge_valence * old_vx_point ; // Corner vertex rule: C.3
				apply_atomic_vec3_increment(new_vx_point, increm_corner) ;
			}
			else
			{
				const float vx_sharpness = n_creases < 2 ? 0.0f : vertex_sharpness_or_border(H_old, C_old, h_id) ; // n_creases < 0 ==> dart vertex ==> smooth
				if (vx_sharpness < 1e-6) // Smooth rule: C.2
				{
					//const int i = Vd + Face(h) ;
					const int new_edge_pt_id = Vd + Fd + Edge(H_old, h_id) ;
					const int new_face_pt_id = Vd + Face(h_id) ;

					const float n_ = float(n) ;
					const float _n2 = 1.0f / float(n*n) ;

					const vec3& new_face_pt = V_new[new_face_pt_id] ;
					const vec3& new_edge_pt = V_new[new_edge_pt_id] ;

					const vec3 increm_smooth = (4.0f * new_edge_pt - new_face_pt + (n_ - 3.0f)*old_vx_point) * _n2 ; // Smooth rule: C.2
					apply_atomic_vec3_increment(new_vx_point, increm_smooth) ;
				}
				else // creased case
				{
					const int c_id = Edge(H_old, h_id) ;
					float edge_sharpness = Sharpness(C_old, c_id) ;
					if (edge_sharpness > 1e-6) // current edge is crease and contributes
					{
						const int new_edge_pt_id = Vd + Fd + Edge(H_old, h_id) ;
						const vec3& new_edge_pt = V_new[new_edge_pt_id] ;

						if (vx_sharpness > 1.0)
						{
							const vec3 increm_creased = 0.25f * (new_edge_pt + old_vx_point) ; // Creased vertex rule: C.5
							apply_atomic_vec3_increment(new_vx_point, increm_creased) ;
						}
						else // Blended vertex rule: C.4
						{
							const vec3 increm_creased = 0.25f * (new_edge_pt + old_vx_point) ; // Creased vertex rule: C.5
							const vec3 increm_corner = 0.5f * old_vx_point ;
							const vec3 increm = lerp(increm_corner,increm_creased,vx_sharpness) ;
							apply_atomic_vec3_increment(new_vx_point, increm) ;
						}
					}
				}
			}
		}
	}
	_BARRIER
}
