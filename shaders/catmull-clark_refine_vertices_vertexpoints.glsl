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


void loop_around_vertex(int h, out int edge_valence, out int halfedge_valence, out int crease_valence, out float vx_sharpness)
{
	halfedge_valence = 1 ;
	vx_sharpness = Sharpness(Edge(h)) ;
	crease_valence = int(vx_sharpness > 0.0) ;

	int h_it ;
	for (h_it = Twin(h) ; h_it >= 0 ; h_it = Twin(h_it))
	{
		h_it = Next(h_it) ;

		if (h_it == h) // no border encountered
			break ;

		halfedge_valence++ ;

		const float edge_sharpness = Sharpness(Edge(h_it)) ;
		if (edge_sharpness > 0.0)
		{
			crease_valence++ ;
			vx_sharpness += edge_sharpness ;
		}
	}

	bool border_encountered = h_it < 0 ;
	if (border_encountered) // border encountered
	{	// do backward iteration too
		for (h_it = h ; h_it >= 0 ; )
		{
			h_it = Prev(h_it) ;
			const float edge_sharpness = Sharpness(Edge(h_it)) ;
			if (edge_sharpness > 0.0)
			{
				crease_valence++ ;
				vx_sharpness += edge_sharpness ;
			}

			h_it = Twin(h_it) ;
			if (h_it >= 0)
				halfedge_valence++ ;

		}
	}

	edge_valence = halfedge_valence + int(border_encountered) ;
	vx_sharpness /= float(edge_valence) ;
}

int sgn(float val)
{
	return int(1e-9 < val) - int(val < -1e-9);
}


void main()
{
	const uint threadID = gl_GlobalInvocationID.x ;
	const int h_id = int(threadID) ;

	if (h_id < Hd)
	{
		// --- halfedges
		// ids
		const int twin_id = Twin(h_id) ;
		const int prev_id = Prev(h_id) ;
		const int edge_id = Edge(h_id) ;
		const int next_id = Edge(h_id) ;
		const int face_id = Face(h_id) ;
		// values
		const int prev_edge_id = Edge(prev_id) ;

		// --- creases
		// ids
		const int crease_id = edge_id ;
		const int crease_prev_id = Edge(prev_id) ;
		// crease values
		const float c_sharpness = Sharpness(crease_id) ;
		const float prev_sharpness = Sharpness(crease_prev_id) ;
		const int c_sharpness_sgn = sgn(c_sharpness) ;

		// --- vertices
		// ids
		const int vert_id = Vert(h_id) ;
		const int vert_next_id = Vert(next_id) ;
		const int new_face_pt_id = Vd + face_id ;
		const int new_edge_pt_id = Vd + Fd + edge_id ;
		const int new_vertex_pt_id = vert_id ;
		const int new_prev_edge_pt_id = Vd + Fd + prev_edge_id ;
		// vertex values
		const vec3 v_old = V_old(vert_id) ;
		const vec3 v_next_old = V_old(vert_next_id) ;
		const vec3 new_face_pt = V_new(new_face_pt_id) ;
		const vec3 new_edge_pt = V_new(new_edge_pt_id) ;
		const vec3 new_prev_edge_pt = V_new(new_prev_edge_pt_id) ;

		// --- computation

		// determine local vertex configuration
		int vx_n_creases = c_sharpness_sgn ;
		int vx_edge_valence = 1 ;
		float vx_sharpness = c_sharpness ;

		// loop around vx
		int h_id_it ;
		for (h_id_it = twin_id ; h_id_it >= 0 ; h_id_it = Twin(h_id_it))
		{
			h_id_it = Next(h_id_it) ;
			if (h_id_it == h_id)
				break ;

			vx_edge_valence++ ;

			const float s = Sharpness(Edge(h_id_it)) ;
			vx_sharpness += s ;
			vx_n_creases += sgn(s) ;
		}
		// if border, loop backward
		if (h_id_it < 0)
		{
			for (h_id_it = h_id ; h_id_it >= 0 ; h_id_it = Twin(h_id_it))
			{
				h_id_it = Prev(h_id_it) ;

				vx_edge_valence++ ;

				const float s = Sharpness(Edge(h_id_it)) ;
				vx_sharpness += s ;
				vx_n_creases += sgn(s) ;
			}
		}
		vx_sharpness /= float(vx_edge_valence) ;

		bool vx_is_border = h_id_it < 0 ;
		const int vx_halfedge_valence = vx_edge_valence + (vx_is_border ? -1 : 0) ;
		const float lerp_alpha = clamp(vx_sharpness,0.0f,1.0f) ;

		// determine incrementations for all configurations
		const vec3 increm_corner = v_old / float(vx_halfedge_valence) ; // corner vertex rule: C.3
		const vec3 increm_smooth = (4.0f * new_edge_pt - new_face_pt + (float(vx_edge_valence) - 3.0f) * v_old) / float (vx_edge_valence*vx_edge_valence) ; // Smooth rule: C.2
		vec3 increm_creased = c_sharpness_sgn * 0.25f * (new_edge_pt + v_old) ; // Creased vertex rule: C.5
		if (vx_is_border)
		{
			increm_creased = increm_creased + 0.25f * sgn(prev_sharpness) * (new_prev_edge_pt + v_old) ;
		}

		// choose the right incrementation
		vec3 increm ;
		if ((vx_edge_valence == 2) || (vx_n_creases > 2))
			increm = increm_corner ;
		else if (vx_n_creases < 2)
			increm = increm_smooth ;
		else // vx_n_creases = 2
			increm = mix(increm_corner, increm_creased, lerp_alpha) ;

		// --- scatter value to resulting vertex
		apply_atomic_vec3_increment(new_vertex_pt_id, increm) ;
	}
}

#endif
