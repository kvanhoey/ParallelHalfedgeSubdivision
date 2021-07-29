#ifdef COMPUTE_SHADER

layout (local_size_x = 256,
        local_size_y = 1,
        local_size_z = 1) in;

uniform int Hd ;

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

void main()
{
    const uint threadID = gl_GlobalInvocationID.x ;
    const int h = int(threadID) ;

    if (h < Hd)
    {
        HalfedgeBuffer_out.halfedges[h] = HalfedgeBuffer_in.halfedges[h] ;
    }
}

#endif
