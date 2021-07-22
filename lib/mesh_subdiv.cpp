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



Timings
MeshSubdivision::bench_refine_step(bool refine_he, bool refine_cr, bool refine_vx, uint repetitions, bool save_result)
{
	halfedge_buffer H_new ;
	crease_buffer C_new ;
	vertex_buffer V_new ;

	duration min_he(0),max_he(0),sum_he(0),median_he(0) ;
	duration min_cr(0),max_cr(0),sum_cr(0),median_cr(0) ;
	duration min_vx(0),max_vx(0),sum_vx(0),median_vx(0) ;

	if (refine_he)
	{
//		std::cout << "Benching refinement of " << Hd << " halfedges" << std::endl ;
		std::vector<duration> timings_he ;

		H_new.resize(H(depth() + 1));

		for (uint i = 0 ; i < repetitions; ++i)
		{
			auto start = timer::now() ;
			refine_halfedges(H_new) ;
			auto stop = timer::now() ;

			duration elapsed = stop - start;
			timings_he.push_back(elapsed) ;
		}
		assert(timings_he.size() == repetitions) ;

		std::sort(timings_he.begin(), timings_he.end()) ;
		median_he = timings_he[repetitions / 2] ;
		min_he = timings_he.front() ;
		max_he = timings_he.back() ;
		sum_he = std::accumulate(timings_he.begin(), timings_he.end(), duration(0)) ;
	}

	if (refine_cr)
	{
		std::vector<duration> timings_cr ;
//		std::cout << "Benching refinement of " << Cd << " creases" << std::endl ;

		C_new.resize(C(depth() + 1));

		for (uint i = 0 ; i < repetitions; ++i)
		{
			auto start = timer::now() ;
			refine_creases(C_new) ;
			auto stop = timer::now() ;

			duration elapsed = stop - start;
			timings_cr.push_back(elapsed) ;
		}
		assert(timings_cr.size() == repetitions) ;

		std::sort(timings_cr.begin(), timings_cr.end()) ;
		median_cr = timings_cr[repetitions / 2] ;
		min_cr = timings_cr.front() ;
		max_cr = timings_cr.back() ;
		sum_cr = std::accumulate(timings_cr.begin(), timings_cr.end(), duration(0)) ;
	}

	if (refine_vx)
	{
//		std::cout << "Benching refinement of " << Vd << " vertices" << std::endl ;
		std::vector<duration> timings_vx ;
		V_new.resize(V(depth() + 1));

		for (uint i = 0 ; i < repetitions; ++i)
		{
			auto start = timer::now() ;
			//refine_vertices_with_creases(V_new) ;
			refine_vertices(V_new) ;
			auto stop = timer::now() ;

			duration elapsed = stop - start;
			timings_vx.push_back(elapsed) ;
		}
		assert(timings_vx.size() == repetitions) ;

		std::sort(timings_vx.begin(), timings_vx.end()) ;
		median_vx = timings_vx[repetitions / 2] ;
		min_vx = timings_vx.front() ;
		max_vx = timings_vx.back() ;
		sum_vx = std::accumulate(timings_vx.begin(), timings_vx.end(), duration(0)) ;
	}

	if (save_result && refine_he)
		halfedges = H_new ;
	if (save_result && refine_cr)
		creases = C_new ;
	if (save_result)
		vertices = V_new ;

	if (save_result && refine_he && refine_cr && refine_vx)
		set_depth(depth() + 1) ;

	duration min_time = min_he + min_cr + min_vx ;
	duration max_time = max_he + max_cr + max_vx ;
	duration sum_time = sum_he + sum_cr + sum_vx ;
	duration median_time = median_he + median_cr + median_vx ;

	Timings t ;
	t.min = min_time.count() ;
	t.max = max_time.count() ;
	t.mean = sum_time.count() / repetitions ;
	t.median = median_time.count() ;

	return t ;
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
