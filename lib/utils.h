#ifndef __HELPERS__
#define __HELPERS__

#include <omp.h>

#if defined(_WIN32)
	typedef unsigned int uint;
#endif


#define ENABLE_PARALLEL

# ifdef ENABLE_PARALLEL
#	if defined(_WIN32)
#       ifndef _ATOMIC
#           define _ATOMIC          __pragma("omp atomic" )
#       endif
#       ifndef _PARALLEL_FOR
#           define _PARALLEL_FOR    __pragma("omp parallel for")
#       endif
#       ifndef _BARRIER
#           define _BARRIER         __pragma("omp barrier")
#       endif
#	else
#       ifndef _ATOMIC
#           define _ATOMIC          _Pragma("omp atomic" )
#       endif
#       ifndef _PARALLEL_FOR
#           define _PARALLEL_FOR    _Pragma("omp parallel for")
#       endif
#       ifndef _BARRIER
#           define _BARRIER         _Pragma("omp barrier")
#       endif
#	endif
# else
#		define _ATOMIC
#		define _PARALLEL_FOR
#		define _BARRIER
# endif


#define _5_o_8 0.625f
#define _8_o_5 1.6f
#define _3_o_8 0.375f
#define _2pi 6.28318530718f

template <typename T>
static T lerp(const T& a, const T& b, const float& alpha)
{
	return (1 - alpha) * a + alpha * b;
}

template <typename T>
int sgn(T val)
{
	return (T(1e-9) < val) - (val < T(-1e-9));
}


#endif
