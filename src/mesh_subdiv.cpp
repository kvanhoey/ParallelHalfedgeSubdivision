#include "mesh.h"

void
MeshSubdivision::refine_step()
{
	halfedge_buffer H_new ;
	crease_buffer C_new ;
	vertex_buffer V_new ;

	H_new.resize(H(depth() + 1));
	C_new.resize(C(depth() + 1));
	V_new.resize(V(depth() + 1),{0.0f,0.0f,0.0f});

	refine_halfedges(H_new) ;
	refine_creases(C_new) ;
	refine_vertices_with_creases(V_new) ;

	halfedges = H_new ;
	creases = C_new ;
	vertices = V_new ;

	set_depth(depth() + 1);
}

void
MeshSubdivision::refine_step_inplace()
{
	halfedge_buffer H_new ;
	crease_buffer C_new ;
	vertex_buffer& V_new = this->vertices ;

	H_new.resize(H(depth() + 1));
	C_new.resize(C(depth() + 1));
	V_new.resize(V(depth() + 1));

	refine_halfedges(H_new) ;
	refine_creases(C_new) ;
	refine_vertices_inplace() ;

	halfedges = H_new ;
	creases = C_new ;

	set_depth(depth() + 1);
}

void
MeshSubdivision::refine_creases(crease_buffer& C_new) const
{
CC_PARALLEL_FOR
	for (int c = 0; c < Cd; ++c)
	{
		const int _2c = 2*c ;

		Crease& c0 = C_new[_2c + 0] ;
		Crease& c1 = C_new[_2c + 1] ;
		if (is_crease_edge(c))
		{
			const int c_next = NextC(c) ;
			const int c_prev = PrevC(c) ;
			const bool b1 = c == PrevC(c_next) ;
			const bool b2 = c == NextC(c_prev) ;
			const float thisS = 3.0f * Sigma(c) ;
			const float nextS = Sigma(NextC(c)) ;
			const float prevS = Sigma(c_prev) ;

			c0.Next = _2c + 1 ;
			c1.Next = 2 * c_next + (b1 ? 0 : 1) ;

			c0.Prev = 2 * c_prev + (b2 ? 1 : 0) ;
			c1.Prev = _2c + 0 ;

			c0.Sigma = std::max(0.0f, 0.250f * (prevS + thisS ) - 1.0f) ;
			c1.Sigma = std::max(0.0f, 0.250f * (nextS + thisS ) - 1.0f) ;
		}
		else
		{
			c0.Sigma = 0.0f ;
			c1.Sigma = 0.0f ;
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
//		std::cout << "Benching refinement of " << Hd << " halfedges" << std::endl ;
		duration min_time_he(1e9) ;

		H_new.resize(H(depth() + 1));

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
//		std::cout << "Benching refinement of " << Cd << " creases" << std::endl ;

		duration min_time_cr(1e9) ;

		C_new.resize(C(depth() + 1));

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
//		std::cout << "Benching refinement of " << Vd << " vertices" << std::endl ;
		duration min_time_vx(1e9) ;

		V_new.resize(V(depth() + 1));

		for (uint i = 0 ; i < repetitions; ++i)
		{
			auto start = timer::now() ;
			refine_vertices_with_creases(V_new) ;
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
		set_depth(depth() + 1) ;

	const duration mean_time = total_time / double(repetitions) ;

	return min_time.count() ;
}

void
MeshSubdivision::init_vertex_buffer(vertex_buffer& V, uint start_index)
{
CC_PARALLEL_FOR
	for (uint k = start_index ; k < V.size() ; ++k)  // important: include memset for realtime scenario
	{
		V[k] = vec3({0.0f,0.0f,0.0f}) ;
	}
CC_BARRIER
}
