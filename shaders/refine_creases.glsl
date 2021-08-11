#ifdef COMPUTE_SHADER

layout (local_size_x = 256,
        local_size_y = 1,
        local_size_z = 1) in;

uniform int Cd ;

struct Crease
{
	float Sharpness ;
	int Next ;
	int Prev ;
};

layout (binding = BUFFER_IN, std430)
readonly buffer CreaseBufferIn
{
	Crease creases[] ;
} CreaseBuffer_in ;

layout (binding = BUFFER_OUT, std430)
buffer CreaseBufferOut
{
	Crease creases[] ;
} CreaseBuffer_out ;

float Sharpness(int idx)
{
	return idx > Cd ? 0. : CreaseBuffer_in.creases[idx].Sharpness ;
}

int Next(int idx)
{
	return CreaseBuffer_in.creases[idx].Next ;
}

int Prev(int idx)
{
	return CreaseBuffer_in.creases[idx].Prev ;
}

bool is_crease_edge(int c)
{
	return Sharpness(c) > 1e-6 ;
}


void main()
{
	const uint threadID = gl_GlobalInvocationID.x ;
	const int c = int(threadID) ;

	if (c < Cd)
	{
		const int c0 = 2 * c + 0 ;
		const int c1 = 2 * c + 1 ;

                if (is_crease_edge(c))
		{
			const int c_next = Next(c) ;
			const int c_prev = Prev(c) ;
			const bool b1 = c == Prev(c_next) && c != c_next ;
			const bool b2 = c == Next(c_prev) && c != c_prev;
			const float thisS = 3.0 * Sharpness(c) ;
			const float nextS = Sharpness(Next(c)) ;
			const float prevS = Sharpness(c_prev) ;

			CreaseBuffer_out.creases[c0].Next = 2 * c + 1 ;
			CreaseBuffer_out.creases[c1].Next = 2 * c_next + (b1 ? 0 : 1) ;

			CreaseBuffer_out.creases[c0].Prev = 2 * c_prev + (b2 ? 1 : 0) ;
			CreaseBuffer_out.creases[c1].Prev = 2 * c + 0 ;

			CreaseBuffer_out.creases[c0].Sharpness = max(0.0f, 0.250f * (prevS + thisS ) - 1.0f) ;
			CreaseBuffer_out.creases[c1].Sharpness = max(0.0f, 0.250f * (nextS + thisS ) - 1.0f) ;
		}
		else
		{
			CreaseBuffer_out.creases[c0].Sharpness = 0.0f ;
			CreaseBuffer_out.creases[c1].Sharpness = 0.0f ;
		}
	}
}

#endif
