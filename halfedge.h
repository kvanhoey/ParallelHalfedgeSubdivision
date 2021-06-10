#ifndef __HALFEDGE__
#define __HALFEDGE__

struct HalfEdge
{
	int Twin ;
	int Next ;
	int Prev ;
	int Vert ;
	int Edge ;
	int Face ;

	HalfEdge();

	HalfEdge(int Twin,int Next,int Prev,int Vert,int Edge,int Face) ;
};

#endif
