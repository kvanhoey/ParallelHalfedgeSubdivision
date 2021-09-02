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
uniform int Fd ;
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

// ------------ creases ------------
struct Crease
{
	float Sharpness ;
	int Next ;
	int Prev ;
};

layout (binding = CREASE_BUFFER, std430)
readonly buffer CreaseBufferIn
{
	Crease creases[] ;
} CreaseBuffer_in ;

float Sharpness(int idx)
{
	return idx > Cd ? 0. : CreaseBuffer_in.creases[idx].Sharpness ;
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

vec3 V_old(const int vertexID)
{
	return vec3(VertexBuffer_in.vertices[3*vertexID+0],
	                VertexBuffer_in.vertices[3*vertexID+1],
	                VertexBuffer_in.vertices[3*vertexID+2]) ;
}

vec3 V_new(const int vertexID)
{
	return vec3(VertexBuffer_out.vertices[3*vertexID+0],
	                VertexBuffer_out.vertices[3*vertexID+1],
	                VertexBuffer_out.vertices[3*vertexID+2]) ;
}

void apply_atomic_vec3_increment(const int vertexID, const vec3 increm_value)
{
	atomicAdd(VertexBuffer_out.vertices[3*vertexID],   increm_value.x) ;
	atomicAdd(VertexBuffer_out.vertices[3*vertexID+1], increm_value.y) ;
	atomicAdd(VertexBuffer_out.vertices[3*vertexID+2], increm_value.z) ;
}

bool is_border_halfedge(int h)
{
	return Twin(h) < 0 ;
}

void main()
{
	const uint threadID = gl_GlobalInvocationID.x ;
	const int h_id = int(threadID) ;

	if (h_id < Hd)
	{
		// --- creases
		// ids
		const int edge_id = Edge(h_id) ;
		const int crease_id = edge_id ;

		// --- vertices
		// ids
		const int vert_id = Vert(h_id) ;
		const int vert_next_id = Vert(Next(h_id)) ;
		const int new_face_pt_id = Vd + Face(h_id) ;
		const int new_edge_pt_id = Vd + Fd + edge_id ;
		// vertex values
		const vec3 v_old = V_old(vert_id) ;
		const vec3 v_next_old = V_old(vert_next_id) ;
		const vec3 new_face_pt = V_new(new_face_pt_id) ;

		// --- computation
		const bool is_border = is_border_halfedge(h_id) ;
		const float sharpness = Sharpness(crease_id) ;
		const float lerp_alpha = clamp(sharpness,0.0f,1.0f) ;

		const vec3 increm_smooth = 0.25f * (v_old + new_face_pt) ; // Smooth rule B.2
		const vec3 increm_sharp = (is_border ? 1.0f : 0.5f) * mix(v_old, v_next_old, 0.5f) ; // Crease rule: B.3
		vec3 increm = mix(increm_smooth,increm_sharp,lerp_alpha) ; // Blending crease rule: B.4

		// --- scatter value to resulting vertex
		apply_atomic_vec3_increment(new_edge_pt_id, increm) ;
	}
}

#endif
