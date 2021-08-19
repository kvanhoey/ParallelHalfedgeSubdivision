#include "mesh.h"

Mesh_Subdiv_Loop_GPU::Mesh_Subdiv_Loop_GPU(const std::string& filename, uint depth):
	Mesh_Subdiv_Loop(filename, depth),
	Mesh_Subdiv_GPU(filename, depth),
	Mesh_Subdiv(filename, depth)
{
	refine_halfedges_step_program	= create_program("../shaders/loop_refine_halfedges.glsl",BUFFER_HALFEDGES_IN, BUFFER_HALFEDGES_OUT	) ;
	refine_vertices_step_program	= create_program("../shaders/loop_refine_vertices.glsl"	,BUFFER_VERTICES_IN	, BUFFER_VERTICES_OUT,	true) ;
}
