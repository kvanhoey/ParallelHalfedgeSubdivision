// basic file operations
#include <iostream>
#include <fstream>
#include <sstream>

#include <chrono>
typedef std::chrono::high_resolution_clock timer;
typedef std::chrono::duration<float, std::milli> duration;

//#define TIMING
//#define INPLACE
#define EXPORT
#define MAX_VERTICES pow(2,28)

#include "mesh.h"

int Vd(int V0, int E0, int F0, int D)
{
	int v_term = D ;
	int e_term = pow(2,(D+1)) - D - 2 ;
	int f_term = (pow(4,D+1) - 4) / 6 - 3*(pow(2,D)-1) + D ;
	return v_term * V0 + e_term * E0 + f_term * F0 ;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " filename.obj" << std::endl ;
		return 0 ;
	}

	std::string f_name(argv[1]) ;
	Mesh_Loop S0(f_name) ;

# ifdef TIMING
	std::cout << "Timings_" << f_name ;
#	ifdef	INPLACE
	        std::cout << "inplace" ;
#	endif
	std::cout << " = [" ;
# endif


	S0.check() ;
# ifdef EXPORT
	S0.export_to_obj("S0.obj") ;
# endif

	Mesh_Loop S = S0 ;
	for (int d = 1 ; d < 6 ; d++)
	{
		std::cout << d << std::endl ;
		if (S.V(d+1) > MAX_VERTICES)
			break ;
# ifdef TIMING
		auto start = timer::now() ;
# endif
# ifdef INPLACE
		S.refine_step_inplace() ;
# else
		S.refine_step() ;
# endif
# ifdef TIMING
		auto stop = timer::now() ;

		duration diff = stop - start;
		std::cout << diff.count() << "," << std::flush ;
# endif
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
# ifdef TIMING
	std::cout << "];" << std::endl ;
# endif

	return 0 ;
}
