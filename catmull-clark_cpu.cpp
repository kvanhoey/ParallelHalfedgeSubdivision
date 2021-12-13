// basic file operations
#include <iostream>
#include <fstream>
#include <sstream>

#define MAX_VERTICES pow(2,28)

#include "mesh_subdiv_catmull-clark_cpu.h"

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " <filename>.obj <depth> [timing=nb_repetitions (default 0)]" << std::endl ;
		return 0 ;
	}

	const std::string f_name(argv[1]) ;
	const uint D = atoi(argv[2]) ;
	const uint timing_reps = (argc < 4) ? 0 : atoi(argv[3]) ;

	std::stringstream fname_out_ss ;
	fname_out_ss << "S" << D << "_catmull-clark_cpu.obj" ;
	std::string fname_out = fname_out_ss.str() ;

	std::cout << "Loading " << f_name << std::endl ;
	Mesh_Subdiv_CatmullClark_CPU M(f_name, D) ;

	if (M.V(D) > MAX_VERTICES)
	{
		std::cout << std::endl << "ERROR: Mesh may exceed memory limits at depth " << D << std::endl ;
		return 0 ;
	}

	const char* num_threads_str = std::getenv("OMP_NUM_THREADS") ;
	if (num_threads_str != NULL)
		std::cout << "Using " << atoi(num_threads_str) << " threads" << std::endl ;
	else
		std::cout << "Using default number of threads" << std::endl ;

	// Check & export input
	M.check() ;
	std::cout << "Exporting input S0_input.obj ... " << std::flush ;
	M.export_to_obj("S0_input.obj") ;
	std::cout << "[OK]" << std::endl ;

	if (timing_reps)
	{
		Timing_stats stats_he, stats_cr, stats_cl, stats_vx ;
		M.subdivide_and_time(timing_reps, stats_he, stats_cr, stats_cl, stats_vx) ;
		std::cout << "- Halfedges:\t"	<< stats_he << std::endl ;
		std::cout << "- Creases:\t"		<< stats_cr << std::endl ;
		std::cout << "- Clear:\t"		<< stats_cl << std::endl ;
		std::cout << "- Vertices:\t"	<< stats_vx << std::endl ;
	}
	else // subdiv down to depth D
	{
		std::cout << "Processing subdivision ... " << std::flush ;
		M.subdivide() ;
		std::cout << "\t\t[OK]" << std::endl ;
	}

	// Check & export output
	M.check() ;
	std::cout << "Exporting output " << fname_out << " ... " << std::flush ;
	M.export_to_obj(fname_out) ;
	std::cout << "\t[OK]" << std::endl ;

	return 0 ;
}
