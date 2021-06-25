#ifndef __HALFEDGE__
#define __HALFEDGE__

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

#endif
