#ifdef COMPUTE_SHADER

#define _5_o_8 0.625f
#define _8_o_5 1.6f
#define _3_o_8 0.375f
#define _2pi 6.28318530718f

layout (local_size_x = 256,
        local_size_y = 1,
        local_size_z = 1) in;

uniform int Hd ;
uniform int Vd ;
uniform int Ed ;
uniform int Cd ;

// ------------ halfedges ------------
struct HalfEdge
{
	int Twin ;
	int Vert ;
	int Edge ;
};

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
	return h % 3 == 2 ? h - 2 : h + 1 ;
}

int Next_safe(int idx)
{
	if (idx < 0)
		return idx ;

	return Next(idx) ;
}

int Prev(int h)
{
	return h % 3 == 0 ? h + 2 : h - 1 ;
}

int Face(int h)
{
	return h / 3 ;
}

bool is_border_halfedge(int h)
{
	return Twin(h) < 0 ;
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
	vx_sharpness *= 0.5f ;
}

float compute_beta(float one_over_n)
{
	return (_5_o_8 - pow((_3_o_8 + cos(_2pi * one_over_n) * 0.250),2)) * one_over_n ;
}



void main()
{
	const uint threadID = gl_GlobalInvocationID.x ;
	const int h = int(threadID) ;

	if (h < Hd)
	{
		// --- halfedges
		// ids
		const int hp = Prev(h) ;
		const int hn = Next(h) ;
		// halfedge values
		const bool h_is_border = is_border_halfedge(h) ;

		// --- creases
		// ids
		const int c_id = Edge(h) ;
		// crease values
		const float edge_sharpness = clamp(Sharpness(c_id),0.0f,1.0f) ;

		// --- vertices
		// ids
		const int v = Vert(h) ;
		const int vp = Vert(hp) ;
		const int vn = Vert(hn) ;
		// vertex values
		const vec3 v_old_vx = V(v) ;
		const vec3 vn_old_vx = V(vn) ;
		const vec3 vp_old_vx = V(vp) ;



		// edgepoints
		const int new_edge_pt_id = Vd + Edge(h) ;

		const vec3 increm_smooth_edge = 0.375f * v_old_vx + 0.125f * vp_old_vx ;
		const vec3 increm_sharp_edge = 0.5f * (h_is_border ? v_old_vx + vn_old_vx : v_old_vx) ;
		const vec3 increm = mix(increm_smooth_edge,increm_sharp_edge,edge_sharpness) ;

		apply_atomic_vec3_increment(new_edge_pt_id, increm) ;

		// vertex points
		const int new_vx_pt_id = v ;

		int vx_edge_valence ; //= vertex_edge_valence(h) ;
		int vx_halfedge_valence ; // = vertex_halfedge_valence(h) ;
		int vx_crease_valence ; // = vertex_crease_valence(h) ;
		float vx_sharpness ;
		loop_around_vertex(h, vx_edge_valence, vx_halfedge_valence, vx_crease_valence, vx_sharpness) ;
		if (vx_crease_valence < 2) // vx_crease_valence < 2 ==> dart vertex ==> smooth
			vx_sharpness = 0.0f ;

		const float lerp_alpha = clamp(vx_sharpness,0.0f,1.0f) ;

		// border correction
		float increm_corner_factr = 0.5f ;
		float increm_sharp_factr_vn = 0.375f ;
		float increm_sharp_factr_vb = 0.0f ;

		int vb = v ;
		if (h_is_border)
		{
			increm_corner_factr = 1.0f ;
			increm_sharp_factr_vn = 0.75f ;

			for (int h_it = Prev(h) ; ; h_it = Prev(h_it))
			{
				const int h_it_twin = Twin(h_it) ;
				if (h_it_twin < 0)
				{
					vb = Vert(h_it) ;
					increm_sharp_factr_vb = 0.125f ;
					break ;
				}

				h_it = h_it_twin ;
			}
		}

		// utility notations
		const float n_ = 1./float(vx_edge_valence) ;
		const float beta = compute_beta(n_) ;
		const float beta_ = n_ - beta ;
		const float edge_sharpness_factr = edge_sharpness > 0.0 ? 1.0 : 0.0 ;
		const vec3 vb_old_vx = V(vb) ;

		// calculate possible increm values
		const vec3 increm_corner_vx = v_old_vx / vx_halfedge_valence ;
		const vec3 increm_smooth_vx = beta_*v_old_vx + beta*vn_old_vx ;
		const vec3 increm_sharp_vx = edge_sharpness_factr * (0.125f * vn_old_vx + increm_sharp_factr_vn * v_old_vx + increm_sharp_factr_vb * vb_old_vx) ;

		if ((vx_edge_valence == 2) || vx_crease_valence > 2) // Corner vertex rule
		{
			apply_atomic_vec3_increment(new_vx_pt_id,increm_corner_vx) ;
		}
		else if (vx_sharpness < 1e-6) // smooth
		{
			apply_atomic_vec3_increment(new_vx_pt_id,increm_smooth_vx) ;
		}
		else // creased or blend
		{
			const vec3 incremV = mix(increm_corner_vx,increm_sharp_vx,lerp_alpha) ;
			apply_atomic_vec3_increment(new_vx_pt_id, incremV) ;
		}

	}
}

#endif
