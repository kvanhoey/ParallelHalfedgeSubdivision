#include "mesh.h"

int
Mesh_CC::H(int depth) const
{
	const int& d = depth < 0 ? this->depth : depth ;
	return std::pow(4,d) * H0 ;
}

int
Mesh_CC::V(int depth) const
{	
	const int& d = depth < 0 ? this->depth : depth ;
	switch(depth)
	{
		case (0):
			return V0 ;
			break ;
		case (1):
			return V(0) + F(0) + E(0) ;
			break;
		default:
			return V(1) + pow(2,d - 1)*(E(1) + (pow(2,d) - 1)*F(1)) ;
	}
}

int
Mesh_CC::F(int depth) const
{
	const int& d = depth < 0 ? this->depth : depth ;
	return d == 0 ? F0 : std::pow(4,d - 1) * H0 ;
}

int
Mesh_CC::E(int depth) const
{
	const int& d = depth < 0 ? this->depth : depth ;
	return d == 0 ? E0 : std::pow(2,d-1) * (2*E0 + (std::pow(2,d) - 1)*H0) ;
}

//int
//Mesh_CC::Next(int h) const
//{
//	return h % 4 == 3 ? h - 3 : h + 1 ;
//}

//int
//Mesh_CC::Next_safe(int h) const
//{
//	if (h < 0)
//		return h ;
//	return Next(h) ;
//}

//int
//Mesh_CC::Prev(int h) const
//{
//	return h % 4 == 0 ? h + 3 : h - 1 ;
//}

//int
//Mesh_CC::Face(int h) const
//{
//	return h / 4 ;
//}


void
Mesh_CC::refine_halfedges(halfedge_buffer& new_he) const
{
CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int h_prime = Prev(h) ;

		int h0_Twin = 4 * Next_safe(Twin(h)) + 3 ; 
		int h1_Twin = 4 * Next(h) + 2 ;
		int h2_Twin = 4 * h_prime + 1 ;
		int h3_Twin = 4 * Twin(h_prime) + 0 ;

		int h0_Next = 4 * h + 1 ;
		int h1_Next = 4 * h + 2 ;
		int h2_Next = 4 * h + 3 ;
		int h3_Next = 4 * h + 0 ;

		int h0_Prev = 4 * h + 3 ;
		int h1_Prev = 4 * h + 0 ;
		int h2_Prev = 4 * h + 1 ;
		int h3_Prev = 4 * h + 2 ;

		int h0_Vert = Vert(h) ;
		int h1_Vert = V(depth) + F(depth) + Edge(h) ;
		int h2_Vert = V(depth) + Face(h) ;
		int h3_Vert = V(depth) + F(depth) + Edge(h_prime) ;

		int h0_Edge = 2*Edge(h) + (int(h) > Twin(h) ? 0 : 1)  ;
		int h1_Edge = 2*E(depth) + h ;
		int h2_Edge = 2*E(depth) + h_prime ;
		int h3_Edge = 2*Edge(h_prime) + (int(h_prime) > Twin(h_prime) ? 1 : 0) ;

		int h0_Face = h ;
		int h1_Face = h ;
		int h2_Face = h ;
		int h3_Face = h ;

		new_he[4*h + 0] = HalfEdge(h0_Twin,h0_Next,h0_Prev,h0_Vert,h0_Edge,h0_Face) ;
		new_he[4*h + 1] = HalfEdge(h1_Twin,h1_Next,h1_Prev,h1_Vert,h1_Edge,h1_Face) ;
		new_he[4*h + 2] = HalfEdge(h2_Twin,h2_Next,h2_Prev,h2_Vert,h2_Edge,h2_Face) ;
		new_he[4*h + 3] = HalfEdge(h3_Twin,h3_Next,h3_Prev,h3_Vert,h3_Edge,h3_Face) ;
	}
CC_BARRIER
}

void
Mesh_CC::refine_vertices_inplace()
{
	vertex_buffer& Vd = this->vertices ;
	facepoints(Vd) ;
	edgepoints(Vd) ;
	vertexpoints_inplace() ;
}

void
Mesh_CC::refine_vertices(vertex_buffer& V_new) const
{
	facepoints(V_new) ;
	edgepoints(V_new) ;
	vertexpoints(V_new) ;
}

void
Mesh_CC::refine_vertices_with_creases(vertex_buffer& V_new) const
{
	facepoints(V_new) ;
	edgepoints_with_creases(V_new) ;
	vertexpoints_with_creases(V_new) ;
}

void 
Mesh_CC::facepoints(vertex_buffer& V_new) const
{
	const vertex_buffer& Vd = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int v = Vert(h) ;
		const int i = V(depth) + Face(h) ;

		const int m = n_vertex_of_polygon(h) ;
		for (int c=0; c < 3; ++c)
		{
			float increm = Vd[v][c] / m ;
CC_ATOMIC
			V_new[i][c] += increm ;
		}
	}
CC_BARRIER
}

void
Mesh_CC::edgepoints(vertex_buffer& V_new) const
{
	const vertex_buffer& Vd = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int v = Vert(h) ;
		const int j = V(depth) + F(depth) + Edge(h) ;
		if (Twin(h) < 0) // Boundary rule: B.1
		{
			const int vv = Vert(Next(h)) ;
			for (int c=0; c < 3; ++c)
			{
				V_new[j][c] = (Vd[v][c] + Vd[vv][c])  / 2. ;
			}
		}
		else // B.2
		{
			const int i = V(depth) + Face(h) ;
			for (int c=0; c < 3; ++c)
			{
				float increm = (Vd[v][c] + V_new[i][c]) / 4 ;
CC_ATOMIC
				V_new[j][c] += increm ;
			}
		}
	}
CC_BARRIER
}

void
Mesh_CC::edgepoints_with_creases(vertex_buffer& V_new) const
{
	const vertex_buffer& Vd = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int v = Vert(h) ;
		const float c = Edge(h) ;
		const int j = V(depth) + F(depth) + Edge(h) ;

		if (is_border_halfedge(h)) // Boundary rule: B.1
		{
			const int vn = Vert(Next(h)) ;
			for (int c=0; c < 3; ++c)
			{
				V_new[j][c] = (Vd[v][c] + Vd[vn][c])  / 2. ;
			}
		}
		else
		{
			float sharpness = Sigma(c) ;
			if (sharpness < 1e-6) // Smooth rule B.2
			{
				const int i = V(depth) + Face(h) ;
				for (int c=0; c < 3; ++c)
				{
					float increm = (Vd[v][c] + V_new[i][c]) / 4 ;
	CC_ATOMIC
					V_new[j][c] += increm ;
				}
			}
			else if (sharpness > 1.0) // Crease rule: B.3
			{
				for (int c=0; c < 3; ++c)
				{
					float increm = Vd[v][c]  / 2. ;
	CC_ATOMIC
					V_new[j][c] += increm ;
				}
			}
			else // Blending crease rule: B.4
			{
				const int vn = Vert(Next(h)) ;
				const int i = V(depth) + Face(h) ;
				for (int c=0; c < 3; ++c)
				{
					float increm_sharp = Vd[v][c] / 2. ;
					float increm_smooth = (Vd[v][c] + V_new[i][c]) / 4 ;
					float increm = std::lerp(increm_smooth,increm_sharp,sharpness) ;
	CC_ATOMIC
					V_new[j][c] += increm ;
				}
			}
		}
	}
CC_BARRIER
}


void
Mesh_CC::vertexpoints(vertex_buffer& V_new) const
{
	const vertex_buffer& Vd = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int n = vertex_edge_valence_or_border(h) ;
		const int v = Vert(h) ;

		if (n < 0) // Boundary rule: C.1
		{
			const int h_valence = vertex_halfedge_valence(h) ;
			for (int c=0; c < 3; ++c)
			{
				float increm = Vd[v][c] / h_valence ;
CC_ATOMIC
				V_new[v][c] += increm ;
			}
		}
		else // C.2
		{
			const int i = V(depth) + Face(h) ;
			const int j = V(depth) + F(depth) + Edge(h) ;
			const int n2 = n*n ;
			for (int c=0; c < 3; ++c)
			{
				const float increm = (4*V_new[j][c] - V_new[i][c] + (n-3)*Vd[v][c]) / n2 ;
CC_ATOMIC
				V_new[v][c] += increm ;
			}
		}
	}
CC_BARRIER
}


void
Mesh_CC::vertexpoints_with_creases(vertex_buffer& V_new) const
{
	const vertex_buffer& Vd = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int n = vertex_edge_valence_or_border(h) ;
		const int v = Vert(h) ;

		if (n < 0) // Boundary rule: C.1
		{
			const int vertex_he_valence = vertex_halfedge_valence(h) ;
			for (int c=0; c < 3; ++c)
			{
				float increm = Vd[v][c] / vertex_he_valence ;
				V_new[v][c] += increm ;
			}
		}
		else
		{
			const int n_creases = vertex_crease_valence_or_border(h) ;
			if ((n == 2) || n_creases > 2) // Corner vertex rule: C.3
			{
				const int vx_halfedge_valence = vertex_halfedge_valence(h) ;
				for (int c=0; c < 3; ++c)
				{
					float increm = Vd[v][c] / vx_halfedge_valence ;
CC_ATOMIC
					V_new[v][c] += increm ;
				}
			}
			else
			{
				const float vx_sharpness = n_creases < 2 ? 0.0f : vertex_sharpness_or_border(h) ; // n_creases < 0 ==> dart vertex ==> smooth
				if (vx_sharpness < 1e-6) // Smooth rule: C.2
				{
					const int i = V(depth) + Face(h) ;
					const int j = V(depth) + F(depth) + Edge(h) ;
					const int n2 = n*n ;
					for (int c=0; c < 3; ++c)
					{
						const float increm = (4*V_new[j][c] - V_new[i][c] + (n-3)*Vd[v][c]) / n2 ;
CC_ATOMIC
						V_new[v][c] += increm ;
					}
				}
				else // creased case
				{
					const int c_id = Edge(h) ;
					float edge_sharpness = Sigma(c_id) ;
					if (edge_sharpness > 1e-6) // current edge is crease and contributes
					{
						const int j = V(depth) + F(depth) + Edge(h) ;
						if (vx_sharpness > 1.0) // Creased vertex rule: C.5
						{
							for (int c=0; c < 3; ++c)
							{
								const float increm = 0.25*(V_new[j][c] + Vd[v][c]) ;
CC_ATOMIC
								V_new[v][c] += increm ;
							}
						}
						else // Blended vertex rule: C.4
						{
							for (int c=0; c < 3; ++c)
							{
								const float increm_creased = 0.25*(V_new[j][c] + Vd[v][c]) ;
								float increm_corner = 0.5*Vd[v][c] ;
								float increm = std::lerp(increm_corner,increm_creased,vx_sharpness) ;
CC_ATOMIC
								V_new[v][c] += increm ;
							}
						}

					}
				 }
			}
		}
	}
CC_BARRIER
}

void
Mesh_CC::vertexpoints_inplace()
{
	vertexpoints_inplace_pass1() ;
	vertexpoints_inplace_pass2() ;
}

void
Mesh_CC::vertexpoints_inplace_pass1()
{
	vertex_buffer& Vd = this->vertices ;
	vertex_buffer& V_new = Vd ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int n = vertex_edge_valence_or_border(h) ;
		if (n >= 0)
		{
            const int v = Vert(h) ;

            const float prod = std::pow((n-3.)/n, 1./n) ; // sqrt_n((n-3) / n)
			for (int c=0; c < 3; ++c)
			{
CC_ATOMIC
				V_new[v][c] *= prod ;
			}
		}
	}
CC_BARRIER
}

void
Mesh_CC::vertexpoints_inplace_pass2()
{
	vertex_buffer& Vd = this->vertices ;
	vertex_buffer& V_new = Vd ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int n = vertex_edge_valence_or_border(h) ;
		const int v = Vert(h) ;

		if (n >= 0) // non-boundary case
		{
			const int i = V(depth) + Face(h) ;
			const int j = V(depth) + F(depth) + Edge(h) ;
			const int n2 = n*n ;
			for (int c=0; c < 3; ++c)
			{
				const float increm = (4*V_new[j][c] - V_new[i][c]) / n2 ;
CC_ATOMIC
				V_new[v][c] += increm ;
			}
		}
	}
CC_BARRIER
}


Mesh_CC
Mesh_CC::cube()
{
	int H = 24 ;
	int V = 8 ;
	int E = 12 ;
	int F = 6 ;

	Mesh_CC M(H,V,E,F) ;
	M.halfedges[0] = HalfEdge(14, 1,3,0,3,0) ;
	M.halfedges[1] = HalfEdge(19, 2,0,1,7,0) ;
	M.halfedges[2] = HalfEdge(4 , 3,1,3,0,0) ;
	M.halfedges[3] = HalfEdge(21, 0,2,2,9,0) ;
	M.halfedges[4] = HalfEdge(2 , 5,7,2,0,1) ;
	M.halfedges[5] = HalfEdge(18, 6,4,3,6,1) ;
	M.halfedges[6] = HalfEdge(8 , 7,5,5,1,1) ;
	M.halfedges[7] = HalfEdge(22, 4,6,4,10,1) ;
	M.halfedges[8] = HalfEdge(6 , 9,11,4,1,2) ;
	M.halfedges[9] = HalfEdge(17, 10,8,5,5,2) ;
	M.halfedges[10] = HalfEdge(12,11,9,7,2,2) ;
	M.halfedges[11] = HalfEdge(23,8,10,6,11,2) ;
	M.halfedges[12] = HalfEdge(10,13,15,6,2,3) ;
	M.halfedges[13] = HalfEdge(16,14,12,7,4,3) ;
	M.halfedges[14] = HalfEdge(0 ,15,13,1,3,3) ;
	M.halfedges[15] = HalfEdge(20,12,14,0,8,3) ;
	M.halfedges[16] = HalfEdge(13,17,19,1,4,4) ;
	M.halfedges[17] = HalfEdge(9, 18,16,7,5,4) ;
	M.halfedges[18] = HalfEdge(5, 19,17,5,6,4) ;
	M.halfedges[19] = HalfEdge(1, 16,18,3,7,4) ;
	M.halfedges[20] = HalfEdge(15,21,23,6,8,5) ;
	M.halfedges[21] = HalfEdge(3, 22,20,0,9,5) ;
	M.halfedges[22] = HalfEdge(7, 23,21,2,10,5) ;
	M.halfedges[23] = HalfEdge(11,20,22,04,11,5) ;

	M.vertices[0] = {0.000000, 0.000000, 1.000000};
	M.vertices[1] = {1.000000, 0.000000, 1.000000};
	M.vertices[2] = {0.000000, 1.000000, 1.000000};
	M.vertices[3] = {1.000000, 1.000000, 1.000000};
	M.vertices[4] = {0.000000, 1.000000, 0.000000};
	M.vertices[5] = {1.000000, 1.000000, 0.000000};
	M.vertices[6] = {0.000000, 0.000000, 0.000000};
	M.vertices[7] = {1.000000, 0.000000, 0.000000};

	return M ;
}


Mesh_CC
Mesh_CC::fig1_left()
{
	// Set mesh from Fig.1-left as cage
	int H = 12 ;
	int V = 7 ;
	int E = 9 ;
	int F = 3 ;

	Mesh_CC S0(H,V,E,F) ;
	S0.halfedges[0]  = HalfEdge(6, 1, 3, 3, 3, 0) ;
	S0.halfedges[1] = HalfEdge(7, 2, 0, 2, 4, 0) ;
	S0.halfedges[2] = HalfEdge(-1, 3, 1, 1, 0, 0) ;
	S0.halfedges[3] = HalfEdge(-1, 0, 2, 0, 1, 0) ;
	S0.halfedges[4] = HalfEdge(-1, 5, 6, 3, 2, 1) ;
	S0.halfedges[5] = HalfEdge(8, 6, 4, 4, 5, 1) ;
	S0.halfedges[6] = HalfEdge(0, 4, 5, 2, 3, 1) ;
	S0.halfedges[7] = HalfEdge(1, 8, 11, 1, 4, 2) ;
	S0.halfedges[8] = HalfEdge(5, 9, 7, 2, 5, 2) ;
	S0.halfedges[9] = HalfEdge(-1, 10, 8, 4, 6, 2) ;
	S0.halfedges[10] = HalfEdge(-1, 11, 9, 5, 7, 2) ;
	S0.halfedges[11] = HalfEdge(-1, 7, 10, 6, 8, 2) ;

	S0.vertices[0] = {-3, 3, 0} ;
	S0.vertices[1] = {1, 6, 0} ;
	S0.vertices[2] = {0, 0, 1} ;
	S0.vertices[3] = {-4, -4, 0} ;
	S0.vertices[4] = {3, -5, 0} ;
	S0.vertices[5] = {6, -2, 0} ;
	S0.vertices[6] = {5, 4, 0} ;

	return S0 ;
}

Mesh_CC
Mesh_CC::quad()
{
	int H = 4 ;
	int V = 4 ;
	int E = 4 ;
	int F = 1 ;

	Mesh_CC M(H,V,E,F) ;
	M.halfedges[0] = HalfEdge(-1,1,3,0,0,0) ;
	M.halfedges[1] = HalfEdge(-1,2,0,1,1,0) ;
	M.halfedges[2] = HalfEdge(-1,3,1,2,2,0) ;
	M.halfedges[3] = HalfEdge(-1,0,2,3,3,0) ;

	M.vertices[0] = {-10,-10,0} ;
	M.vertices[1] = {-10,10,0} ;
	M.vertices[2] = {10,10,0} ;
	M.vertices[3] = {10,-10,0} ;

	return M ;
}
