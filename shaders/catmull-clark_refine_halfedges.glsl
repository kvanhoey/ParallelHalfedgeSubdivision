#ifdef COMPUTE_SHADER

layout (local_size_x = 256,
        local_size_y = 1,
        local_size_z = 1) in;

uniform int d ;
uniform int Hd ;
uniform int Vd ;
uniform int Fd ;
uniform int Ed ;

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

void main()
{
	const uint threadID = gl_GlobalInvocationID.x ;
	const int h_id = int(threadID) ;

	const int _2Ed = 2 * Ed ;

	if (h_id < Hd)
	{
		const int _4h_id = 4 * h_id ;

		const int h0 = _4h_id + 0 ;
		const int h1 = _4h_id + 1 ;
		const int h2 = _4h_id + 2 ;
		const int h3 = _4h_id + 3 ;

		const int twin_id = Twin(h_id) ;
		const int edge_id = Edge(h_id) ;

		const int prev_id = Prev(h_id) ;
		const int prev_twin_id = Twin(prev_id) ;
		const int prev_edge_id = Edge(prev_id) ;

		HalfEdge halfedges[4] ;

		halfedges[0].Twin = 4 * Next_safe(twin_id) + 3 ;
		halfedges[1].Twin = 4 * Next(h_id) + 2 ;
		halfedges[2].Twin = 4 * prev_id + 1 ;
		halfedges[3].Twin = 4 * prev_twin_id + 0 ;

		halfedges[0].Vert = Vert(h_id) ;
		halfedges[1].Vert = Vd + Fd + edge_id ;
		halfedges[2].Vert = Vd + Face(h_id) ;
		halfedges[3].Vert = Vd + Fd + prev_edge_id ;

		halfedges[0].Edge = 2 * edge_id + (int(h_id) > twin_id ? 0 : 1) ;
		halfedges[1].Edge = _2Ed + h_id ;
		halfedges[2].Edge = _2Ed + prev_id ;
		halfedges[3].Edge = 2 * prev_edge_id + (int(prev_id) > prev_twin_id ? 1 : 0) ;

		HalfedgeBuffer_out.halfedges[h0] = halfedges[0] ;
		HalfedgeBuffer_out.halfedges[h1] = halfedges[1] ;
		HalfedgeBuffer_out.halfedges[h2] = halfedges[2] ;
		HalfedgeBuffer_out.halfedges[h3] = halfedges[3] ;
	}
}

#endif
