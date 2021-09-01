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
		for (int h = 0; h < Hd ; ++h)
		{
			const int _4h = 4 * h ;

			HalfEdge& h0 = H_new[_4h + 0] ;
			HalfEdge& h1 = H_new[_4h + 1] ;
			HalfEdge& h2 = H_new[_4h + 2] ;
			HalfEdge& h3 = H_new[_4h + 3] ;

			const int h_twin = Twin(H_old,h) ;
			const int h_edge = Edge(H_old,h) ;

			const int h_prev = Prev(h) ;
			const int h_prev_twin = Twin(H_old,h_prev) ;
			const int h_prev_edge = Edge(H_old,h_prev) ;

			h0.Twin = 4 * Next_safe(h_twin) + 3 ;
			h1.Twin = 4 * Next(h) + 2 ;
			h2.Twin = 4 * h_prev + 1 ;
			h3.Twin = 4 * h_prev_twin + 0 ;

			h0.Vert = Vert(H_old,h) ;
			h1.Vert = Vd + Fd + h_edge ;
			h2.Vert = Vd + Face(h) ;
			h3.Vert = Vd + Fd + h_prev_edge ;

			h0.Edge = 2 * h_edge + (int(h) > h_twin ? 0 : 1) ;
			h1.Edge = _2Ed + h ;
			h2.Edge = _2Ed + h_prev ;
			h3.Edge = 2 * h_prev_edge + (int(h_prev) > h_prev_twin ? 1 : 0) ;
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
	for (int h = 0; h < Hd ; ++h)
	{
		const int vert_id = Vert(H_old, h) ;
		const int new_face_pt_id = Vd + Face(h) ;
		vec3& new_face_pt = V_new[new_face_pt_id] ;

		const int m = n_vertex_of_polygon(h) ;
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
	for (int h = 0; h < Hd ; ++h)
	{
		const int v_id = Vert(H_old,h) ;
		const int e_id = Edge(H_old, h) ;
		const int& c_id = e_id ;

		const int new_edge_pt_id = Vd + Fd + e_id ;

		const vec3& v_old = V_old[v_id] ;
		vec3& new_edge_pt = V_new[new_edge_pt_id] ;

		if (is_border_halfedge(H_old,h)) // Boundary rule: B.1
		{
			const int vn_id = Vert(H_old, Next(h)) ;
			const vec3& vn_old = V_old[vn_id] ;

			new_edge_pt = 0.5f * (v_old + vn_old) ; // at border, there is a single write, no need for atomic
		}
		else
		{
			const int new_face_pt_id = Vd + Face(h) ;
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
	for (int h = 0; h < Hd ; ++h)
	{
		const int n = vertex_edge_valence_or_border(H_old, h) ;
		const int v = Vert(H_old, h) ;
		vec3& new_vx_point = V_new[v] ;
		const vec3& old_vx_point = V_old[v] ;

		if (n < 0) // Boundary rule: C.1
		{
			const float vertex_he_valence = float(vertex_halfedge_valence(H_old, h)) ;
			const vec3 increm_border = old_vx_point / vertex_he_valence ;
			apply_atomic_vec3_increment(new_vx_point, increm_border) ;
		}
		else
		{
			const int n_creases = vertex_crease_valence_or_border(H_old, C_old, h) ;
			if ((n == 2) || n_creases > 2) // Corner vertex rule: C.3
			{
				const float vx_halfedge_valence = 1.0f / float(vertex_halfedge_valence(H_old, h)) ;
				const vec3 increm_corner = vx_halfedge_valence * old_vx_point ; // Corner vertex rule: C.3
				apply_atomic_vec3_increment(new_vx_point, increm_corner) ;
			}
			else
			{
				const float vx_sharpness = n_creases < 2 ? 0.0f : vertex_sharpness_or_border(H_old, C_old, h) ; // n_creases < 0 ==> dart vertex ==> smooth
				if (vx_sharpness < 1e-6) // Smooth rule: C.2
				{
					//const int i = Vd + Face(h) ;
					const int new_edge_pt_id = Vd + Fd + Edge(H_old, h) ;
					const int new_face_pt_id = Vd + Face(h) ;

					const float n_ = float(n) ;
					const float _n2 = 1.0f / float(n*n) ;

					const vec3& new_face_pt = V_new[new_face_pt_id] ;
					const vec3& new_edge_pt = V_new[new_edge_pt_id] ;

					const vec3 increm_smooth = (4.0f * new_edge_pt - new_face_pt + (n_ - 3.0f)*old_vx_point) * _n2 ; // Smooth rule: C.2
					apply_atomic_vec3_increment(new_vx_point, increm_smooth) ;
				}
				else // creased case
				{
					const int c_id = Edge(H_old, h) ;
					float edge_sharpness = Sharpness(C_old, c_id) ;
					if (edge_sharpness > 1e-6) // current edge is crease and contributes
					{
						const int new_edge_pt_id = Vd + Fd + Edge(H_old, h) ;
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
