#ifdef COMPUTE_SHADER

layout (local_size_x = 256,
        local_size_y = 1,
        local_size_z = 1) in;

layout (binding = MY_LOCATION, std430)
buffer my_buffer //readonly buffer my_buffer
{
    float data[] ;
};

layout (binding = MY_RO_LOCATION, std430)
readonly buffer my_read_buffer
{
    float data_ro[] ;
};


void main()
{
    const uint threadID = gl_GlobalInvocationID.x;

    if (threadID < 4)
    {

        //data[threadID] = data_ro[threadID] + 0 ;
        atomicAdd(data[threadID], data_ro[threadID]) ;
    }
}

#endif
