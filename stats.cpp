// basic file operations
#include <iostream>
#include <fstream>
#include <sstream>

#include "mesh.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " <filename>.obj" << std::endl ;
		return 0 ;
	}

	const std::string f_name(argv[1]) ;

	std::cout << "Loading " << f_name << std::endl ;

	const Mesh_Loop S0(f_name) ;
	const bool is_tri = S0.is_tri_only() ;
	const bool is_quad = S0.is_quad_only() ;
	const int borders = S0.count_border_edges() ;
	const int H = S0.H() ;
	const int V = S0.V() ;
	const int E = S0.E() ;
	const int F = S0.F() ;
	const int C = S0.C() ;
	const int sharp_creases = S0.count_sharp_creases() ;

	if (is_tri)
		std::cout << "Triangle-only mesh" << std::endl ;
	else if (is_quad)
		std::cout << "Quad-only mesh" << std::endl ;

	std::cout << "Statistics: " << std::endl ;
	std::cout << "H:\t" << H << std::endl ;
	std::cout << "V:\t" << V << std::endl ;
	std::cout << "E:\t" << E << "\t(" << borders << " border)" << std::endl ;
	std::cout << "F:\t" << F << std::endl ;
	std::cout << "C:\t" << C << "\t(" << sharp_creases << " sharp)" << std::endl ;

	return 0 ;
}
