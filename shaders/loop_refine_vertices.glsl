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

bool is_crease_halfedge(int h)
{
	return Sharpness(Edge(h)) > 1e-6 ;
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

int vertex_edge_valence(int h)
{
	int n = 1 ;

	int h_back_valid = h ;
	int h_back = Twin(Prev(h)) ;

	// rewind to border (if any)
	while ((h_back != h) && (h_back >= 0))
	{
		h_back_valid = h_back ;
		h_back = Twin(Prev(h_back)) ;
	}

	// fw to border or back to start and count
	int h_fw = Next_safe(Twin(h_back_valid)) ;
	while ((h_fw != h_back_valid) && (h_fw >= 0))
	{
		n++ ;
		h_fw = Next_safe(Twin(h_fw)) ;
	}

	if (h_fw < 0)
		n++ ;

	return n ;
}

int vertex_crease_valence(int h)
{
	int n = is_crease_halfedge(h) ? 1 : 0 ;

	int h_it ;
	for (h_it = Twin(h) ; h_it >= 0 ; h_it = Twin(h_it))
	{
		h_it = Next(h_it) ;

		if (h_it == h)
			break ;

		if (is_crease_halfedge(h_it))
			n++ ;
	}

	if (h_it < 0)
	{	// do backward iteration too
		for (h_it = h ; h_it >= 0 ; h_it = Twin(h_it))
		{
			h_it = Prev(h_it) ;
			if (is_crease_halfedge(h_it))
				n++ ;
		}
	}

	return n ;
}

int vertex_halfedge_valence(int h)
{
	int n = 1 ;

	int h_back_valid = h ;
	int h_back = Twin(Prev(h)) ;

	// rewind to border (if any)
	while ((h_back != h) && (h_back >= 0))
	{
		h_back_valid = h_back ;
		h_back = Twin(Prev(h_back)) ;
	}

	// fw to border or back to start and count
	int h_fw = Next_safe(Twin(h_back_valid)) ;
	while ((h_fw != h_back_valid) && (h_fw >= 0))
	{
		n++ ;
		h_fw = Next_safe(Twin(h_fw)) ;
	}

	return n ;
}

float vertex_sharpness(int h)
{
	float edge_valence = 1.0 ;
	float sharpness = Sharpness(Edge(h)) ;

	int h_it ;
	for (h_it = Twin(h) ; h_it >= 0 ; h_it = Twin(h_it))
	{
		h_it = Next(h_it) ;
		if (h_it == h)
			break ;

		edge_valence++ ;
		sharpness += Sharpness(Edge(h_it)) ;
	}

	if (h_it < 0)
	{	// do backward iteration too
		for (h_it = h ; h_it >= 0 ; h_it = Twin(h_it))
		{
			h_it = Prev(h_it) ;
			edge_valence++ ;
			sharpness += Sharpness(Edge(h_it)) ;
		}
	}

	return sharpness / edge_valence ;
}

float compute_beta(float one_over_n)
{
	return (_5_o_8 - pow((_3_o_8 + cos(_2pi * one_over_n) * 0.250),2)) * one_over_n ;
}

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

void main()
{
	const uint threadID = gl_GlobalInvocationID.x ;
	const int h = int(threadID) ;

	if (h < Hd)
	{
		//		VertexBuffer_out.vertices[h] = VertexBuffer_in.vertices[h] + 1 ;
		//VertexBuffer_out.vertices[h] = vec3(3*h+1,3*h+2,3*h+3) ;
		//VertexBuffer_out.vertices[3+h] = vec3(3*h,3*h+1,3*h+2) ;

		// edgepoints
		const int v = Vert(h) ;
		const int new_edge_pt_id = Vd + Edge(h) ;

		const int vp = Vert(Prev(h)) ;
		const int vn = Vert(Next(h)) ;

		const int c_id = Edge(h) ;
		const float sharpness = clamp(Sharpness(c_id),0.0f,1.0f) ;
		const bool is_border = is_border_halfedge(h) ;

		const vec3 v_old_vx = V(v) ;
		const vec3 vn_old_vx = V(vn) ;
		const vec3 vp_old_vx = V(vp) ;

		const vec3 increm_smooth_edge = 0.375f * v_old_vx + 0.125f * vp_old_vx ;
		const vec3 increm_sharp_edge = 0.5f * (is_border ? v_old_vx + vn_old_vx : v_old_vx) ;
		const vec3 increm = mix(increm_smooth_edge,increm_sharp_edge,sharpness) ;

		apply_atomic_vec3_increment(new_edge_pt_id, increm) ;

		// vertex points
		const int new_vx_pt_id = v ;

		const int n = vertex_edge_valence(h) ;
		const int n_creases = vertex_crease_valence(h) ;
		const int vertex_he_valence = vertex_halfedge_valence(h) ;

		const float edge_sharpness = Sharpness(c_id) ;
		const float vx_sharpness = n_creases < 2 ? 0.0f : vertex_sharpness(h) ; // n_creases < 0 ==> dart vertex ==> smooth

		const float lerp_alpha = clamp(vx_sharpness,0.0,1.0) ;

		// utility notations
		const float n_ = 1./float(n) ;
		const float beta = compute_beta(n_) ;
		const float beta_ = n_ - beta ;


		float edge_sharpness_factr = edge_sharpness < 1e-6 ? 0.0 : 1.0 ;

		// border correction
		float increm_corner_factr = 0.5f ;
		float increm_sharp_factr_vn = 0.375f ;
		float increm_sharp_factr_vb = 0.0f ;

		int vb = v ;
		if (is_border)
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

		const vec3 vb_old_vx = V(vb) ;
		const vec3 increm_corner_vx = v_old_vx / vertex_he_valence ;
		const vec3 increm_smooth_vx = beta_*v_old_vx + beta*vn_old_vx ;
		const vec3 increm_sharp_vx = edge_sharpness_factr * (0.125f * vn_old_vx + increm_sharp_factr_vn * v_old_vx + increm_sharp_factr_vb * vb_old_vx) ;

		if ((n==2) || n_creases > 2) // Corner vertex rule
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
