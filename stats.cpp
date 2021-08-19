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

	const Mesh M(f_name) ;
	const bool is_tri = M.is_tri_only() ;
	const bool is_quad = M.is_quad_only() ;
	const int borders = M.count_border_edges() ;
	const int H = M.H() ;
	const int V = M.V() ;
	const int E = M.E() ;
	const int F = M.F() ;
	const int C = M.C() ;
	const int sharp_creases = M.count_sharp_creases() ;

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
