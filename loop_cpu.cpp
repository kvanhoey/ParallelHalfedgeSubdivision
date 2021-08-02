// basic file operations
#include <iostream>
#include <fstream>
#include <sstream>

//#define INPLACE
#define MAX_VERTICES pow(2,28)

#include "mesh.h"

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " <filename>.obj <depth> <export=0>" << std::endl ;
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

	S0.check() ;

	if (enable_export)
	{
		std::cout << "Exporting S0.obj" << std::endl ;
		S0.export_to_obj("S0.obj") ;
	}

	Mesh_Loop S = S0 ;
	for (int d = 1 ; d <= D ; d++)
	{
		std::cout << "Subdividing level " << d << std::endl ;
		if (S.V(d+1) > MAX_VERTICES)
			break ;
		# ifdef INPLACE
			S.refine_step_inplace() ;
		# else
			S.refine_step() ;
		# endif
		S.check() ;

		if (enable_export)
		{
			std::stringstream ss ;
			ss << "S" << d ;
			# ifdef INPLACE
				ss << "_inplace" ;
			# endif
			ss << ".obj" ;

			std::cout << "Exporting " << ss.str() << std::endl ;
			S.export_to_obj(ss.str()) ;
		}
	}

	return 0 ;
}
