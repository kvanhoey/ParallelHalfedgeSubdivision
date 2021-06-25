// basic file operations
#include <iostream>
#include <fstream>
#include <sstream>

#include <chrono>
typedef std::chrono::high_resolution_clock timer;
typedef std::chrono::duration<float, std::milli> duration;

#include "mesh.h"

#define TIMING
#define INPLACE
#define EXPORT
#define MAX_VERTICES pow(2,28)

int main(int argc, char* argv[])
{
	if (argc < 3)
    {
		std::cout << "Usage: " << argv[0] << " <filename>.obj" << " <depth>" << std::endl ;
		return 0 ;
	}

    std::string f_name(argv[1]) ;
	Mesh_CC S0(f_name) ;

	uint D = atoi(argv[2]) ;

# ifdef TIMING
    std::cout << "Timings_" << f_name << " = [" ;
# endif


    S0.check() ;
# ifdef EXPORT
    S0.export_to_obj("S0.obj") ;
# endif

    Mesh_CC S = S0 ;
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
        std::cout << diff.count() << "," ;
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
