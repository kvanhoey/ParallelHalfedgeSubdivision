// basic file operations
#include <iostream>
#include <fstream>
#include <sstream>

#include "mesh.h"

#define MAX_VERTICES pow(2,28)
#define N_REPETITIONS 50

int main(int argc, char* argv[])
{
	if (argc < 3)
    {
		std::cout << "Usage: " << argv[0] << " <filename>.obj <depth> <export_mesh=0> <nb_repetitions=" << N_REPETITIONS << ">" << std::endl ;
		return 0 ;
    }

	const std::string f_name(argv[1]) ;
	const uint D = atoi(argv[2]) ;
	const bool enable_export = argc < 4 ? false : atoi(argv[3]) ;
	const uint runCount = argc == 5 ? atoi(argv[4]) : 50 ;

	std::cout << "Loading " << f_name << " ... " << std::flush ;
	Mesh_Loop S0(f_name) ;
	std::cout << "[OK]" << std::endl ;

	if (!S0.is_tri_only())
	{
		std::cerr << "ERROR: The provided mesh should be triangle-only" << std::endl ;
		exit(0) ;
	}
	S0.check() ;

	if (S0.V(D) > MAX_VERTICES)
	{
		std::cout << std::endl << "ERROR: Mesh may exceed memory limits at depth " << D << std::endl ;
		return 0 ;
	}

	const char* num_threads_str = std::getenv("OMP_NUM_THREADS") ;
	int num_threads = 0 ;
	if (num_threads_str != NULL)
	{
		 num_threads = atoi(num_threads_str) ;
		 std::cout << "Using " << num_threads << " threads" << std::endl ;
	}
	else
		std::cout << "Using default number of threads" << std::endl ;

	Timings refine_he_time, refine_cr_time, refine_vx_time ;

	Mesh_Loop S = S0 ;
	for (int d = 1 ; d <= D ; d++)
    {
		std::cout << "Subdividing level " << d << " " << runCount << "x ... " << std::flush ;
		refine_he_time += S.bench_refine_step(true, false, false, runCount) ;
		refine_cr_time += S.bench_refine_step(false, true, false, runCount) ;
		refine_vx_time += S.bench_refine_step(false, false, true, runCount) ;
		S.bench_refine_step(true, true, true, 1, true) ;
		std::cout << "[OK]" << std::endl ;

		assert(S.check()) ;
    }

	std::cout << std::fixed << "Halfedges:\t" << refine_he_time.median << "ms" << std::endl ;
	std::cout << std::fixed << "Creases:\t" << refine_cr_time.median << "ms" << std::endl ;
	std::cout << std::fixed << "Vertices:\t" << refine_vx_time.median << "ms" << std::endl ;
//	std::cout << std::fixed << refine_he_time.median << "\t" << refine_cr_time.median << "\t" << refine_vx_time.median << std::endl ;

	if (enable_export)
	{
		std::stringstream ss ;
		ss << "S" << D << ".obj" ;

		std::cout << "Exporting " << ss.str() << " ... " << std::flush ;
		S.export_to_obj(ss.str()) ;
		std::cout << "[OK]" << std::endl ;
	}

//	// write into files
//	const std::string f_name_tmp = f_name.substr(f_name.find_last_of("\\/") + 1, 999) ;
//	const std::string f_name_clean = f_name_tmp.substr(0,f_name_tmp.find_last_of(".")) ;

//	std::stringstream fname_he, fname_cr, fname_vx ;
//	fname_he << f_name_clean << "_halfedge_" << D << "_" << num_threads << ".txt" ;
//	fname_cr << f_name_clean << "_crease_" << D << "_" << num_threads << ".txt" ;
//	fname_vx << f_name_clean << "_points_" << D << "_" << num_threads << ".txt" ;

//	std::ofstream f_he ;
//	f_he.open(fname_he.str()) ;
//	f_he << std::fixed << refine_he_time.median << "\t/\t" << refine_he_time.mean << "\t/\t" << refine_he_time.min << "\t/\t" << refine_he_time.max << std::endl ;
//	f_he.close() ;

//	std::ofstream f_cr ;
//	f_cr.open(fname_cr.str()) ;
//	f_cr << std::fixed << refine_cr_time.median << "\t/\t" << refine_cr_time.mean << "\t/\t" << refine_cr_time.min << "\t/\t" << refine_cr_time.max << std::endl ;
//	f_cr.close() ;

//	std::ofstream f_vx ;
//	f_vx.open(fname_vx.str()) ;
//	f_vx << std::fixed << refine_vx_time.median << "\t/\t" << refine_vx_time.mean << "\t/\t" << refine_vx_time.min << "\t/\t" << refine_vx_time.max << std::endl ;
//	f_vx.close() ;

    return 0 ;
}
