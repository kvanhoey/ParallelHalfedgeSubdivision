#ifdef COMPUTE_SHADER

#ifndef HALFEDGE_BUFFER_IN
#   Error: no binding of input halfedge buffer found
#endif
#ifndef HALFEDGE_BUFFER_OUT
#   Error: no binding of output halfedge buffer found
#endif

uniform int Hd ;

struct Halfedge
{
    int twinID ;
    int edgeID ;
    int vertexID ;
};

layout (local_size_x = 256,
        local_size_y = 1,
        local_size_z = 1) in;

layout (binding = HALFEDGE_BUFFER_IN, std430)
readonly buffer HalfedgeBufferIn
{
    Halfedge halfedges[] ;
} HalfedgeBuffer_in ;

layout (binding = HALFEDGE_BUFFER_OUT, std430)
buffer HalfedgeBufferOut
{
    Halfedge halfedges[] ;
} HalfedgeBuffer_out ;

void main()
{
    const uint threadID = gl_GlobalInvocationID.x;
    const uint h = threadID ;

    if (h < Hd)
    {
	HalfedgeBuffer_out.halfedges[h] = HalfedgeBuffer_in.halfedges[h] ;
    }
}

#endif
