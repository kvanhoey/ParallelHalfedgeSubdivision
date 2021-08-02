#ifdef COMPUTE_SHADER

layout (local_size_x = 256,
        local_size_y = 1,
        local_size_z = 1) in;

uniform int Hd ;
uniform int Vd ;
uniform int Ed ;

struct HalfEdge
{
    int Twin ;
    int Vert ;
    int Edge ;
};

layout (binding = BUFFER_IN, std430)
readonly buffer HalfedgeBufferIn
{
    HalfEdge halfedges[] ;
} HalfedgeBuffer_in ;

layout (binding = BUFFER_OUT, std430)
buffer HalfedgeBufferOut
{
    HalfEdge halfedges[] ;
} HalfedgeBuffer_out ;

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


void main()
{
    const uint threadID = gl_GlobalInvocationID.x ;
    const int h = int(threadID) ;

    if (h < Hd)
    {
	const int _3h = 3 * h ;

	const int h0 = _3h + 0 ;
	const int h1 = _3h + 1 ;
	const int h2 = _3h + 2 ;
	const int h3 = 3 * Hd + h ;

	const int h_twin = Twin(h) ;
	const int h_edge = Edge(h) ;
	const int h_twin_next = Next_safe(h_twin) ;

	const int h_prev = Prev(h) ;
	const int h_prev_twin = Twin(h_prev) ;
	const int h_prev_edge = Edge(h_prev) ;

	const int v = Vert(h) ;

	HalfEdge halfedges[4] ;

	halfedges[0].Twin = 3 * h_twin_next + 2 ;
	halfedges[1].Twin = h3 ;
	halfedges[2].Twin = 3 * h_prev_twin ;
	halfedges[3].Twin = h1 ;

	halfedges[0].Vert = v ;
	halfedges[1].Vert = Vd + h_edge ;
	halfedges[2].Vert = Vd + h_prev_edge ;
	halfedges[3].Vert = halfedges[2].Vert ;

	halfedges[0].Edge = 2 * h_edge + (h > h_twin ? 0 : 1) ;
	halfedges[1].Edge = 2 * Ed + h ;
	halfedges[2].Edge = 2 * h_prev_edge + (h_prev > h_prev_twin ? 1 : 0) ;
	halfedges[3].Edge = halfedges[1].Edge ;

	HalfedgeBuffer_out.halfedges[h0] = halfedges[0] ;
	HalfedgeBuffer_out.halfedges[h1] = halfedges[1] ;
	HalfedgeBuffer_out.halfedges[h2] = halfedges[2] ;
	HalfedgeBuffer_out.halfedges[h3] = halfedges[3] ;
    }
}

#endif
