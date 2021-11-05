#include "mesh_subdiv_cpu.h"

Mesh_Subdiv_CPU::Mesh_Subdiv_CPU(const std::string &filename, uint max_depth):
	Mesh_Subdiv(filename,max_depth)
{}

void
Mesh_Subdiv_CPU::allocate_subdiv_buffers()
{
	halfedge_subdiv_buffers.resize(d_max + 1) ;
	crease_subdiv_buffers.resize(d_max + 1) ;
	vertex_subdiv_buffers.resize(d_max + 1) ;

	uint d = 0 ;
	halfedge_subdiv_buffers[d]	= halfedges ;
	crease_subdiv_buffers[d]	= creases	;
	vertex_subdiv_buffers[d]	= vertices	;

	for (d = 1 ; d <= d_max ; ++d)
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
	halfedges	= halfedge_subdiv_buffers[d_max] ;
	creases		= crease_subdiv_buffers[d_max] ;
	vertices	= vertex_subdiv_buffers[d_max] ;
}

void
Mesh_Subdiv_CPU::refine_creases()
{
	for (uint d = 0 ; d < d_max; ++d)
	{
		set_current_depth(d) ;
		const crease_buffer& C_old = crease_subdiv_buffers[d] ;
		crease_buffer& C_new = crease_subdiv_buffers[d + 1] ;
		const uint Cd = C(d) ;

		_PARALLEL_FOR
		for (int c_id = 0; c_id < Cd; ++c_id)
		{
			Crease& c0 = C_new[2*c_id + 0] ;
			Crease& c1 = C_new[2*c_id + 1] ;
			if (is_crease_edge(C_old,c_id))
			{
				const int c_next_id = C_old[c_id].Next ;
				const int c_prev_id = C_old[c_id].Prev ;
				const bool b1 = c_id == C_old[c_next_id].Prev && c_id != c_next_id ;
				const bool b2 = c_id == C_old[c_prev_id].Next && c_id != c_prev_id;
				const float thisS = 3.0f * C_old[c_id].Sharpness ;
				const float nextS = C_old[c_next_id].Sharpness ;
				const float prevS = C_old[c_prev_id].Sharpness ;

				c0.Next = 2*c_id + 1 ;
				c1.Next = 2 * c_next_id + (b1 ? 0 : 1) ;

				c0.Prev = 2 * c_prev_id + (b2 ? 1 : 0) ;
				c1.Prev = 2*c_id + 0 ;

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
Mesh_Subdiv_CPU::refine_halfedges_and_time(int n_repetitions)
{
	for (int i = 0 ; i < n_repetitions ; ++i)
	{
		auto start = timer::now() ;	// start timer
		refine_halfedges() ;
		auto stop = timer::now() ;			// stop timer
		duration elapsed = stop - start;	// make stats
		std::cout << elapsed.count() << std::endl ;
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
