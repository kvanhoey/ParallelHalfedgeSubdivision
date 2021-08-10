// basic file operations
#include <iostream>
#include <fstream>
#include <sstream>

#include "mesh.h"

//#define INPLACE
#define MAX_VERTICES pow(2,28)

int main(int argc, char* argv[])
{
	if (argc < 3)
    {
		std::cout << "Usage: " << argv[0] << " <filename>.obj <depth> <export=0> <nb_repetitions=16>" << std::endl ;
		return 0 ;
    }

	const std::string f_name(argv[1]) ;
	const uint D = atoi(argv[2]) ;
	const bool enable_export = argc < 4 ? false : atoi(argv[3]) ;
	const uint runCount = argc == 5 ? atoi(argv[4]) : 16 ;

	Mesh_Loop S0(f_name) ;
	if (!S0.is_tri_only())
	{
		std::cerr << "ERROR: The provided mesh should be triangle-only" << std::endl ;
		exit(0) ;
	}
	if (S0.V(D) > MAX_VERTICES)
	{
		std::cout << std::endl << "ERROR: Mesh may exceed memory limits at depth " << D << std::endl ;
		return 0 ;
	}


    S0.check() ;

	if (enable_export)
		S0.export_to_obj("S0.obj") ;

	Mesh_Loop S = S0 ;
	Timings refine_he_time, refine_cr_time, refine_vx_time ;

	for (int d = 1 ; d <= D ; d++)
    {
		refine_he_time += S.bench_refine_step(true, false, false, runCount) ;
		refine_cr_time += S.bench_refine_step(false, true, false, runCount) ;
		refine_vx_time += S.bench_refine_step(false, false, true, runCount) ;
		std::cout << std::fixed << refine_he_time.median << "\t" << refine_cr_time.median << "\t" << refine_vx_time.median << std::endl ;
		S.bench_refine_step(true, true, true, 1, true) ;

		assert(S.check()) ;

		if (enable_export && d >= 6)
		{
			std::stringstream ss ;
			ss << "S" << d ;
#   ifdef INPLACE
			ss << "_inplace" ;
#   endif
			ss << ".obj" ;

			S.export_to_obj(ss.str()) ;
		}



        // write into files
        const std::string f_name_tmp = f_name.substr(f_name.find_last_of("\\/") + 1, 999) ;
        const std::string f_name_clean = f_name_tmp.substr(0,f_name_tmp.find_last_of(".")) ;
        const char* num_threads_str = std::getenv("OMP_NUM_THREADS") ;
        int num_threads = 0 ;
        if (num_threads_str != NULL)
             num_threads = atoi(num_threads_str) ;
    //	std::cout << "Using " << num_threads << " threads" << std::endl ;

        std::stringstream fname_he, fname_cr, fname_vx ;
        fname_he << f_name_clean << "_halfedge_" << d << "_" << num_threads << ".txt" ;
        fname_cr << f_name_clean << "_crease_" << d << "_" << num_threads << ".txt" ;
        fname_vx << f_name_clean << "_points_" << d << "_" << num_threads << ".txt" ;

        std::ofstream f_he ;
        f_he.open(fname_he.str()) ;
        f_he << std::fixed << refine_he_time.median << "\t/\t" << refine_he_time.mean << "\t/\t" << refine_he_time.min << "\t/\t" << refine_he_time.max << std::endl ;
        f_he.close() ;

        std::ofstream f_cr ;
        f_cr.open(fname_cr.str()) ;
        f_cr << std::fixed << refine_cr_time.median << "\t/\t" << refine_cr_time.mean << "\t/\t" << refine_cr_time.min << "\t/\t" << refine_cr_time.max << std::endl ;
        f_cr.close() ;

        std::ofstream f_vx ;
        f_vx.open(fname_vx.str()) ;
        f_vx << std::fixed << refine_vx_time.median << "\t/\t" << refine_vx_time.mean << "\t/\t" << refine_vx_time.min << "\t/\t" << refine_vx_time.max << std::endl ;
        f_vx.close() ;

    }

/*//	std::cout << "Halfedges:\t" << refine_he_time << "ms" << std::endl ;
//	std::cout << "Creases:\t" << refine_cr_time << "ms" << std::endl ;
//	std::cout << "Vertices:\t" << refine_vx_time << "ms" << std::endl ;
	std::cout << std::fixed << refine_he_time.median << "\t" << refine_cr_time.median << "\t" << refine_vx_time.median << std::endl ;

	// write into files
	const std::string f_name_tmp = f_name.substr(f_name.find_last_of("\\/") + 1, 999) ;
	const std::string f_name_clean = f_name_tmp.substr(0,f_name_tmp.find_last_of(".")) ;
	const char* num_threads_str = std::getenv("OMP_NUM_THREADS") ;
	int num_threads = 0 ;
	if (num_threads_str != NULL)
		 num_threads = atoi(num_threads_str) ;
//	std::cout << "Using " << num_threads << " threads" << std::endl ;

	std::stringstream fname_he, fname_cr, fname_vx ;
	fname_he << f_name_clean << "_halfedge_" << D << "_" << num_threads << ".txt" ;
	fname_cr << f_name_clean << "_crease_" << D << "_" << num_threads << ".txt" ;
	fname_vx << f_name_clean << "_points_" << D << "_" << num_threads << ".txt" ;

	std::ofstream f_he ;
	f_he.open(fname_he.str()) ;
	f_he << std::fixed << refine_he_time.median << "\t/\t" << refine_he_time.mean << "\t/\t" << refine_he_time.min << "\t/\t" << refine_he_time.max << std::endl ;
	f_he.close() ;

	std::ofstream f_cr ;
	f_cr.open(fname_cr.str()) ;
	f_cr << std::fixed << refine_cr_time.median << "\t/\t" << refine_cr_time.mean << "\t/\t" << refine_cr_time.min << "\t/\t" << refine_cr_time.max << std::endl ;
	f_cr.close() ;

	std::ofstream f_vx ;
	f_vx.open(fname_vx.str()) ;
	f_vx << std::fixed << refine_vx_time.median << "\t/\t" << refine_vx_time.mean << "\t/\t" << refine_vx_time.min << "\t/\t" << refine_vx_time.max << std::endl ;
    f_vx.close() ;*/


	// print the number of threads
//	#pragma omp parallel
//	{
//		std::cout << "num threads: " << omp_get_num_threads() << std::endl << std::flush ;
//	}


    return 0 ;
}
