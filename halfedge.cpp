#include "halfedge.h"

HalfEdge::HalfEdge() {}

HalfEdge::HalfEdge(int Twin,int Next,int Prev,int Vert,int Edge,int Face):
		Twin(Twin),
		Next(Next),
		Prev(Prev),
		Vert(Vert),
		Edge(Edge),
		Face(Face)
	{}
