// basic file operations
#include <iostream>
#include <fstream>
#include <sstream>

#include "mesh.h"

//#define INPLACE
//#define EXPORT
#define MAX_VERTICES pow(2,28)

int main(int argc, char* argv[])
{
	if (argc < 3)
    {
		std::cout << "Usage: " << argv[0] << " <filename>.obj <depth> <nb_repetitions=16>" << std::endl ;
		return 0 ;
    }

	const std::string f_name(argv[1]) ;
	const uint D = atoi(argv[2]) ;
	const uint runCount = argc == 4 ? atoi(argv[3]) : 16 ;

	Mesh_Loop S0(f_name) ;
	if (!S0.is_tri_only())
	{
		std::cerr << "ERROR: The provided mesh should be triangle-only" << std::endl ;
		exit(0) ;
	}

    S0.check() ;
# ifdef EXPORT
    S0.export_to_obj("S0.obj") ;
# endif

	Mesh_Loop S = S0 ;
	double refine_he_time = 0 ;
	double refine_cr_time = 0 ;
	double refine_vx_time = 0 ;
	std::cout << "Benching depth " << D << std::endl ;
	for (int d = 1 ; d <= D ; d++)
    {
        if (S.V(d+1) > MAX_VERTICES)
            break ;

		refine_he_time += S.bench_refine_step(true, false, false, runCount) ;
		refine_cr_time += S.bench_refine_step(false, true, false, runCount) ;
		refine_vx_time += S.bench_refine_step(false, false, true, runCount) ;

		S.bench_refine_step(true, true, true, 1, true) ;

        S.check() ;

# ifdef EXPORT
        // Export
        std::stringstream ss ;
        ss << "S" << d ;
#   ifdef INPLACE
        ss << "_inplace" ;
#   endif
        ss << ".obj" ;

        S.export_to_obj(ss.str()) ;
# endif
    }

//	std::cout << "Halfedges:\t" << refine_he_time << "ms" << std::endl ;
//	std::cout << "Creases:\t" << refine_cr_time << "ms" << std::endl ;
//	std::cout << "Vertices:\t" << refine_vx_time << "ms" << std::endl ;
	std::cout << "He/Cr/Ve" << std::endl ;
	std::cout << refine_he_time << "\t" << refine_cr_time << "\t" << refine_vx_time << std::endl ;


	// print the number of threads
	#pragma omp parallel
	{
		std::cout << "num threads: " << omp_get_num_threads() << std::endl << std::flush ;
	}


    return 0 ;
}
