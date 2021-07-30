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

layout (binding = HALFEDGE_BUFFER_IN, std430)
readonly buffer HalfedgeBufferIn
{
    HalfEdge halfedges[] ;
} HalfedgeBuffer_in ;

layout (binding = HALFEDGE_BUFFER_OUT, std430)
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
	const int h0 = 3 * h + 0 ;
	const int h1 = 3 * h + 1 ;
	const int h2 = 3 * h + 2 ;
	const int h3 = 3 * Hd + h ;

	const int h_twin = Twin(h) ;
	const int h_edge = Edge(h) ;

	const int h_prev = Prev(h) ;
	const int h_prev_twin = Twin(h_prev) ;
	const int h_prev_edge = Edge(h_prev) ;

	HalfedgeBuffer_out.halfedges[h0].Twin = 3 * Next_safe(h_twin) + 2 ;
	HalfedgeBuffer_out.halfedges[h1].Twin = h3 ;
	HalfedgeBuffer_out.halfedges[h2].Twin = 3 * h_prev_twin ;
	HalfedgeBuffer_out.halfedges[h3].Twin = h1 ;

	HalfedgeBuffer_out.halfedges[h0].Vert = Vert(h) ;
	HalfedgeBuffer_out.halfedges[h1].Vert = Vd + h_edge ;
	HalfedgeBuffer_out.halfedges[h2].Vert = Vd + h_prev_edge ;
	HalfedgeBuffer_out.halfedges[h3].Vert = HalfedgeBuffer_out.halfedges[h2].Vert ;

	HalfedgeBuffer_out.halfedges[h0].Edge = 2 * h_edge + (h > h_twin ? 0 : 1) ;
	HalfedgeBuffer_out.halfedges[h1].Edge = 2 * Ed + h ;
	HalfedgeBuffer_out.halfedges[h2].Edge = 2 * h_prev_edge + (int(h_prev) > h_prev_twin ? 1 : 0) ;
	HalfedgeBuffer_out.halfedges[h3].Edge = HalfedgeBuffer_out.halfedges[h1].Edge ;
    }
}

#endif
