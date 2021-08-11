// basic file operations
#include <iostream>
#include <fstream>
#include <sstream>

#define MAX_VERTICES pow(2,28)

#include "mesh.h"

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " <filename>.obj <depth> <export_all_levels=0>" << std::endl ;
		return 0 ;
	}

	const std::string f_name(argv[1]) ;
	const uint D = atoi(argv[2]) ;
	const bool enable_export = argc < 4 ? false : atoi(argv[3]) ;

	std::cout << "Loading " << f_name << std::endl ;
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

	const char* num_threads_str = std::getenv("OMP_NUM_THREADS") ;
	if (num_threads_str != NULL)
		 std::cout << "Using " << atoi(num_threads_str) << " threads" << std::endl ;
	else
		std::cout << "Using default number of threads" << std::endl ;

	S0.check() ;
	if (enable_export)
	{
		std::cout << "Exporting S0.obj ... " << std::flush ;
		S0.export_to_obj("S0.obj") ; // <-- subdivision happens here
		std::cout << "[OK]" << std::endl ;
	}

	Mesh_Loop S = S0 ;
	for (int d = 1 ; d <= D ; d++)
	{
		std::cout << "Subdividing level " << d << " ... " << std::flush;
		S.refine_step() ;
		std::cout << "[OK]" << std::endl ;

		assert(S.check()) ;

		if (enable_export || d == D)
		{
			std::stringstream ss ;
			ss << "S" << d << ".obj" ;

			std::cout << "Exporting " << ss.str() << " ... " << std::flush ;
			S.export_to_obj(ss.str()) ;
			std::cout << "[OK]" << std::endl ;
		}
	}

	return 0 ;
}
