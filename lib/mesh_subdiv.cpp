#include "mesh_subdiv.h"

Mesh_Subdiv::Mesh_Subdiv(const std::string &filename, uint max_depth):
	Mesh(filename), d_max(max_depth), subdivided(false), finalized(false) {}

int
Mesh_Subdiv::C(int depth) const
{
	const int& d = depth < 0 ? d_cur : depth ;
	return std::pow(2,d) * C_count ;
}

void
Mesh_Subdiv::subdivide()
{
	if (finalized)
		return ;

	allocate_subdiv_buffers() ;

	refine_halfedges() ;
	refine_creases() ;
	refine_vertices() ;
	set_current_depth(d_max) ;

	readback_from_subdiv_buffers() ;

	finalize_subdivision() ;
}

void
Mesh_Subdiv::subdivide_and_time(int n_repetitions, Timing_stats& stats_he, Timing_stats& stats_cr, Timing_stats& stats_cl, Timing_stats& stats_vx)
{
	if (finalized)
		return ;

	allocate_subdiv_buffers() ;

	std::vector<double> t_he = measure_time(&Mesh_Subdiv::refine_halfedges, *this, n_repetitions) ;
	std::vector<double> t_cr = measure_time(&Mesh_Subdiv::refine_creases, *this, n_repetitions) ;
	std::vector<double> t_vx = measure_time(&Mesh_Subdiv::refine_vertices, *this, n_repetitions) ;
	set_current_depth(d_max) ;

	readback_from_subdiv_buffers() ;

	finalize_subdivision() ;

	Timing_stats::compute_stats(t_he, stats_he) ;
	Timing_stats::compute_stats(t_cr, stats_cr) ;
	Timing_stats::compute_stats(t_vx, stats_vx) ;
}

void
Mesh_Subdiv::set_current_depth(int depth)
{
	d_cur = depth ;
	subdivided = d_cur > 0 ;
}

void
Mesh_Subdiv::finalize_subdivision()
{
	const int Hd = H() ;
	const int Ed = E() ;
	const int Fd = F() ;
	const int Vd = V() ;
	const int Cd = C() ;

	H_count = Hd ;
	E_count = Ed ;
	F_count = Fd ;
	V_count = Vd ;
	C_count = Cd ;

	d_cur = 0 ;

	assert(H() == halfedges.size()) ;
	assert(C() == creases.size()) ;
	assert(V() == vertices.size()) ;

	finalized = true ;
}
