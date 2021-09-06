#ifndef __HALFEDGE__
#define __HALFEDGE__

/**
 * @brief The HalfEdge struct stores the (non-analytic) attributes of a halfedge
 */
struct HalfEdge
{
	int Twin ; /*!< Index of the twin halfedge (or -1 if its a border) */
	int Vert ; /*!< Index of the vertex the halfedge points out from */
	int Edge ; /*!< Index of the edge that spans the halfedge */
};

/**
 * @brief The HalfEdge_cage struct stores the attributes of a halfedge
 * that are non-analytic for the cage only,
 * and will be overridden by analytic formulae after subdivision.
 */
struct HalfEdge_cage
{
	int Next ; /*!< Index of the next halfedge within a face */
	int Prev ; /*!< Index of the previous halfedge within a face */
	int Face ; /*!< Index of the face the halfedge lives in */
};

#endif
