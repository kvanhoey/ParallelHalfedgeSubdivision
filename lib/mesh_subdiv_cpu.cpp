#include "mesh.h"

void
Mesh_Subdiv_CPU::allocate_subdiv_buffers()
{
	halfedge_subdiv_buffers.resize(D + 1) ;
	crease_subdiv_buffers.resize(D + 1) ;
	vertex_subdiv_buffers.resize(D + 1) ;

	uint d = 0 ;
	halfedge_subdiv_buffers[d]	= halfedges ;
	crease_subdiv_buffers[d]	= creases	;
	vertex_subdiv_buffers[d]	= vertices	;

	for (d = 1 ; d <= D ; ++d)
	{
		const uint Hd = H(d) ;
		const uint Vd = V(d) ;
		const uint Cd = C(d) ;

		halfedge_subdiv_buffers[d].resize(Hd);
		crease_subdiv_buffers[d].resize(Cd);
		vertex_subdiv_buffers[d].resize(Vd,{0.0f,0.0f,0.0f});
	}
}

void
Mesh_Subdiv_CPU::readback_from_subdiv_buffers()
{
	halfedges	= halfedge_subdiv_buffers[D] ;
	creases		= crease_subdiv_buffers[D] ;
	vertices	= vertex_subdiv_buffers[D] ;
}

void
Mesh_Subdiv_CPU::refine_creases()
{
	for (uint d = 0 ; d < D; ++d)
	{
		const crease_buffer& Cr = crease_subdiv_buffers[d] ;
		crease_buffer& C_new = crease_subdiv_buffers[d + 1] ;
		const uint Cd = C(d) ;
		_PARALLEL_FOR
		for (int c = 0; c < Cd; ++c)
		{
			const int _2c = 2*c ;

			Crease& c0 = C_new[_2c + 0] ;
			Crease& c1 = C_new[_2c + 1] ;
			if (is_crease_edge(Cr,c))
			{
				const int c_next = Cr[c].Next ;
				const int c_prev = Cr[c].Prev ;
				const bool b1 = c == Cr[c_next].Prev && c != c_next ;
				const bool b2 = c == Cr[c_prev].Next && c != c_prev;
				const float thisS = 3.0f * Cr[c].Sharpness ;
				const float nextS = Cr[c_next].Sharpness ;
				const float prevS = Cr[c_prev].Sharpness ;

				c0.Next = _2c + 1 ;
				c1.Next = 2 * c_next + (b1 ? 0 : 1) ;

				c0.Prev = 2 * c_prev + (b2 ? 1 : 0) ;
				c1.Prev = _2c + 0 ;

				c0.Sharpness = std::max(0.0f, 0.250f * (prevS + thisS ) - 1.0f) ;
				c1.Sharpness = std::max(0.0f, 0.250f * (nextS + thisS ) - 1.0f) ;
			}
			else
			{
				c0.Sharpness = 0.0f ;
				c1.Sharpness = 0.0f ;
			}
		}
		_BARRIER
	}
}

void
Mesh_Subdiv_CPU::apply_atomic_vec3_increment(vec3& v, const vec3& v_increm)
{
	for (int c=0; c < 3; ++c)
	{
_ATOMIC
		v[c] += v_increm[c] ;
	}
}


