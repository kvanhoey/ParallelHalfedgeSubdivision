#ifndef __CREASE__
#define __CREASE__

struct Crease
{
	float Sigma ;
	int Next ;
	int Prev ;

	Crease();

	Crease(float Sigma,int Next,int Prev) ;
};

#endif
