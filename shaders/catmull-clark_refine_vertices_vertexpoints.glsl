#ifdef COMPUTE_SHADER

#define _5_o_8 0.625f
#define _8_o_5 1.6f
#define _3_o_8 0.375f
#define _2pi 6.28318530718f

layout (local_size_x = 256,
        local_size_y = 1,
        local_size_z = 1) in;

uniform int d ;
uniform int Hd ;
uniform int Vd ;
uniform int Ed ;
uniform int Cd ;

// ------------ halfedges ------------
struct HalfEdge_cage
{
	int Next ;
	int Prev ;
	int Face ;
};

struct HalfEdge
{
	int Twin ;
	int Vert ;
	int Edge ;
};


layout (binding = CAGE_BUFFER, std430)
readonly buffer HalfedgeCageBufferIn
{
	HalfEdge_cage halfedges_cage[] ;
} HalfEdgeCageBuffer ;

layout (binding = HALFEDGE_BUFFER, std430)
readonly buffer HalfedgeBufferIn
{
	HalfEdge halfedges[] ;
} HalfedgeBuffer_in ;

int Twin(int idx)
{
	return HalfedgeBuffer_in.halfedges[idx].Twin ;
}

int Vert(int idx)
{
	return HalfedgeBuffer_in.halfedges[idx].Vert ;
}

int Edge(int idx)
{
	return HalfedgeBuffer_in.halfedges[idx].Edge ;
}

int Next(int h)
{
	if (d < 1) // not quad-only
		return HalfEdgeCageBuffer.halfedges_cage[h].Next ;

	return h % 4 == 3 ? h - 3 : h + 1 ;
}

int Next_safe(int idx)
{
	if (idx < 0)
		return idx ;

	return Next(idx) ;
}

int Prev(int h)
{
	if (d < 1) // not quad-only
		return HalfEdgeCageBuffer.halfedges_cage[h].Prev ;

	return h % 4 == 0 ? h + 3 : h - 1 ;
}

int Face(int h)
{
	if (d < 1) // not quad-only
		return HalfEdgeCageBuffer.halfedges_cage[h].Face ;

	return h / 4 ;
}

// ------------ vertices ------------
layout (binding = BUFFER_IN, std430)
readonly buffer VertexBufferIn
{
	float vertices[] ;
} VertexBuffer_in ;

layout (binding = BUFFER_OUT, std430)
buffer VertexBufferOut
{
	float vertices[] ;
} VertexBuffer_out ;

vec3 V(const int vertexID)
{
	return vec3(VertexBuffer_in.vertices[3*vertexID+0],
	                VertexBuffer_in.vertices[3*vertexID+1],
	                VertexBuffer_in.vertices[3*vertexID+2]) ;
}

void apply_atomic_vec3_increment(const int vertexID, const vec3 increm_value)
{
	atomicAdd(VertexBuffer_out.vertices[3*vertexID], increm_value.x) ;
	atomicAdd(VertexBuffer_out.vertices[3*vertexID+1], increm_value.y) ;
	atomicAdd(VertexBuffer_out.vertices[3*vertexID+2], increm_value.z) ;
}

int n_vertex_of_polygon_cage(int h)
{
	int n = 1 ;
	for (int h_fw = Next(h) ; h_fw != h ; h_fw = Next(h_fw))
	{
		++n ;
	}
	return n ;
}

int n_vertex_of_polygon(int h)
{
	if (d < 1) // not quad-only
		return n_vertex_of_polygon_cage(h) ;

	return 4 ;
}

void main()
{
	const uint threadID = gl_GlobalInvocationID.x ;
	const int h_id = int(threadID) ;

	if (h_id < Hd)
	{
		// --- halfedges
		// ids
		const int vert_id = Vert(h_id) ;

		// --- vertices
		// ids
		const int v_id = Vert(h_id) ;
		// vertex values
		const vec3 v_old_vx = V(vert_id) ;

		const int new_face_pt_id = Vd + Face(h_id) ;

		const int m = n_vertex_of_polygon(h_id) ;
		const vec3 increm = v_old_vx / m ;

		apply_atomic_vec3_increment(new_face_pt_id, increm) ;
	}
}

#endif
