#include "mesh.h"

void
MeshSubdivision::refine_step()
{
	halfedge_buffer H_new ;
	crease_buffer C_new ;
	vertex_buffer V_new ;

	H_new.resize(H(depth + 1));
	C_new.resize(C(depth + 1));
	V_new.resize(V(depth + 1),{0.,0.,0.});

	refine_halfedges(H_new) ;
	refine_creases(C_new) ;
	refine_vertices_with_creases(V_new) ;

	halfedges = H_new ;
	creases = C_new ;
	vertices = V_new ;

	depth++ ;
}

void
MeshSubdivision::refine_step_inplace()
{
	halfedge_buffer H_new ;
	crease_buffer C_new ;
	vertex_buffer& V_new = this->vertices ;

	H_new.resize(H(depth + 1));
	C_new.resize(C(depth + 1));
	V_new.resize(V(depth + 1),{0.,0.,0.});

	refine_halfedges(H_new) ;
	refine_creases(C_new) ;
	refine_vertices_inplace() ;

	halfedges = H_new ;
	creases = C_new ;

	depth++ ;
}

void
MeshSubdivision::refine_creases(crease_buffer& C_new) const
{
	const int Cd = C(depth) ;

CC_PARALLEL_FOR
	for (int c = 0; c < Cd; ++c)
	{
		if (is_crease_edge(c))
		{
			const int c_next = NextC(c) ;
			const int c_prev = PrevC(c) ;
			const bool b1 = c == PrevC(c_next) ;
			const bool b2 = c == NextC(c_prev) ;
			const float thisS = 3.0f * Sigma(c) ;
			const float nextS = Sigma(NextC(c)) ;
			const float prevS = Sigma(c_prev) ;
			const int c2 = 2*c ;

			Crease& c0 = C_new[c2 + 0] ;
			Crease& c1 = C_new[c2 + 1] ;

			c0.Next = c2 + 1 ;
			c1.Next = 2 * c_next + (b1 ? 0 : 1) ;

			c0.Prev = 2 * c_prev + (b2 ? 1 : 0) ;
			c1.Prev = c2 + 0 ;

			c0.Sigma = std::max(0.0f, (prevS + thisS ) / 4.0f - 1.0f) ;
			c1.Sigma = std::max(0.0f, (nextS + thisS ) / 4.0f - 1.0f) ;
		}
	}
CC_BARRIER
}


double
MeshSubdivision::bench_refine_step(bool refine_he, bool refine_cr, bool refine_vx, uint repetitions, bool save_result)
{
	duration total_time(0) ;
	duration min_time(0) ;

	halfedge_buffer H_new ;
	crease_buffer C_new ;
	vertex_buffer V_new ;

	if (refine_he)
	{
		duration min_time_he(1e9) ;

		H_new.resize(H(depth + 1));

		for (uint i = 0 ; i < repetitions; ++i)
		{
			auto start = timer::now() ;
			refine_halfedges(H_new) ;
			auto stop = timer::now() ;

			duration elapsed = stop - start;
			total_time += elapsed ;
			min_time_he = elapsed < min_time_he ? elapsed : min_time_he ;
		}
		min_time += min_time_he ;
	}

	if (refine_cr)
	{
		duration min_time_cr(1e9) ;

		C_new.resize(C(depth + 1));

		for (uint i = 0 ; i < repetitions; ++i)
		{
			auto start = timer::now() ;
			refine_creases(C_new) ;
			auto stop = timer::now() ;

			duration elapsed = stop - start;
			total_time += elapsed ;
			min_time_cr = elapsed < min_time_cr ? elapsed : min_time_cr ;
		}
		min_time += min_time_cr ;
	}

	if (refine_vx)
	{
		duration min_time_vx(1e9) ;

		V_new.resize(V(depth + 1));

		for (uint i = 0 ; i < repetitions; ++i)
		{
			auto start = timer::now() ;
			std::fill(V_new.begin(), V_new.end(), vec3({0.,0.,0.})); // important: include memset for realtime scenario
			refine_vertices(V_new) ;
			auto stop = timer::now() ;

			duration elapsed = stop - start;
			total_time += elapsed ;
			min_time_vx = elapsed < min_time_vx ? elapsed : min_time_vx ;
		}
		min_time += min_time_vx ;
	}

	if (save_result && refine_he)
		halfedges = H_new ;
	if (save_result && refine_cr)
		creases = C_new ;
	if (save_result)
		vertices = V_new ;

	if (save_result && refine_he && refine_cr && refine_vx)
		depth++ ;

	const duration mean_time = total_time / double(repetitions) ;

	return min_time.count() ;
}
