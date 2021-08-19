#include "mesh_subdiv_catmull-clark_cpu.h"

Mesh_Subdiv_CatmullClark_CPU::Mesh_Subdiv_CatmullClark_CPU(const std::string &filename, uint depth):
	Mesh_Subdiv_CatmullClark(filename, depth),
	Mesh_Subdiv_CPU(filename, depth),
	Mesh_Subdiv(filename, depth)
{
	assert(false) ;
}

void
Mesh_Subdiv_CatmullClark_CPU::refine_halfedges()
{
	assert(false) ;
	for (uint d = 0 ; d < D; ++d)
	{
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
Mesh_Subdiv_CatmullClark_CPU::refine_vertices()
{
	assert(false) ;
	// NOTE. This currently is only a copy-pasting of edgepoints_with_creases_branchless and vertexpoints_with_creases_branchless
	// within the same loop on halfedges.
	// TODO: one can probably exploit the single-loop case more optimally !

	for (uint d = 0 ; d < D; ++d)
	{
		const halfedge_buffer& H_old = halfedge_subdiv_buffers[d] ;
		const crease_buffer& C_old = crease_subdiv_buffers[d] ;
		const vertex_buffer& V_old = vertex_subdiv_buffers[d] ;
		vertex_buffer& V_new = vertex_subdiv_buffers[d+1] ;

		const int Vd = V(d) ;
		const int Hd = H(d) ;

		_PARALLEL_FOR
		for (int h = 0; h < Hd ; ++h)
		{

		}
	_BARRIER
	}
}
