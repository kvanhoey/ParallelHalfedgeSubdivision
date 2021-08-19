#include "mesh_subdiv_catmull-clark_gpu.h"

Mesh_Subdiv_CatmullClark_GPU::Mesh_Subdiv_CatmullClark_GPU(const std::string& filename, uint depth):
	Mesh_Subdiv_CatmullClark(filename, depth),
	Mesh_Subdiv_GPU(filename, depth),
	Mesh_Subdiv(filename, depth)
{
	refine_halfedges_step_program	= create_program("../shaders/catmull-clark_refine_halfedges.glsl",BUFFER_HALFEDGES_IN, BUFFER_HALFEDGES_OUT	) ;
	refine_vertices_step_program	= create_program("../shaders/catmull-clark_refine_vertices.glsl"	,BUFFER_VERTICES_IN	, BUFFER_VERTICES_OUT,	true) ;
}
