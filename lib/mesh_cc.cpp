#include "mesh.h"

int
Mesh_CC::H(int depth) const
{
	const int& d = depth < 0 ? this->depth() : depth ;
	return std::pow(4,d) * H0 ;
}

int
Mesh_CC::V(int depth) const
{	
	const int& d = depth < 0 ? this->depth() : depth ;
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
	const int& d = depth < 0 ? this->depth() : depth ;
	return d == 0 ? F0 : std::pow(4,d - 1) * H0 ;
}

int
Mesh_CC::E(int depth) const
{
	const int& d = depth < 0 ? this->depth() : depth ;
	return d == 0 ? E0 : std::pow(2,d-1) * (2*E0 + (std::pow(2,d) - 1)*H0) ;
}

int
Mesh_CC::Next(int h) const
{
	if (is_cage()) // TODO: optimize to avoid this branch
		return Mesh::Next(h) ;

	return h % 4 == 3 ? h - 3 : h + 1 ;
}

int
Mesh_CC::Prev(int h) const
{
	if (is_cage()) // TODO: optimize to avoid this branch
		return Mesh::Prev(h) ;

	return h % 4 == 0 ? h + 3 : h - 1 ;
}

int
Mesh_CC::Face(int h) const
{
	if (is_cage()) // TODO: optimize to avoid this branch
		return Mesh::Face(h) ;

	return h / 4 ;
}

int
Mesh_CC::n_vertex_of_polygon(int h) const
{
	if (is_cage()) // TODO: optimize to avoid this branch
		return Mesh::n_vertex_of_polygon(h) ;

	return 4 ;
}


void
Mesh_CC::refine_halfedges(halfedge_buffer& new_he) const
{
	const int _2Ed = 2 * Ed ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int _4h = 4 * h + 0 ;

		HalfEdge& h0 = new_he[_4h + 0] ;
		HalfEdge& h1 = new_he[_4h + 1] ;
		HalfEdge& h2 = new_he[_4h + 2] ;
		HalfEdge& h3 = new_he[_4h + 3] ;

		const int h_twin = Twin(h) ;
		const int h_edge = Edge(h) ;

		const int h_prev = Prev(h) ;
		const int h_prev_twin = Twin(h_prev) ;
		const int h_prev_edge = Edge(h_prev) ;


		h0.Twin = 4 * Next_safe(h_twin) + 3 ;
		h1.Twin = 4 * Next(h) + 2 ;
		h2.Twin = 4 * h_prev + 1 ;
		h3.Twin = 4 * h_prev_twin + 0 ;

		h0.Vert = Vert(h) ;
		h1.Vert = Vd + Fd + h_edge ;
		h2.Vert = Vd + Face(h) ;
		h3.Vert = Vd + Fd + h_prev_edge ;

		h0.Edge = 2 * h_edge + (int(h) > h_twin ? 0 : 1) ;
		h1.Edge = _2Ed + h ;
		h2.Edge = _2Ed + h_prev ;
		h3.Edge = 2 * h_prev_edge + (int(h_prev) > h_prev_twin ? 1 : 0) ;
	}
CC_BARRIER
}



void
Mesh_CC::refine_vertices_inplace()
{
	vertex_buffer& V_new = this->vertices ;
	init_vertex_buffer(V_new,Vd) ;
	facepoints(V_new) ;
	edgepoints(V_new) ;
	vertexpoints_inplace() ;
}

void
Mesh_CC::refine_vertices(vertex_buffer& V_new) const
{
	init_vertex_buffer(V_new) ;
	facepoints(V_new) ;
	edgepoints(V_new) ;
	vertexpoints(V_new) ;
}

void
Mesh_CC::refine_vertices_with_creases(vertex_buffer& V_new) const
{
	init_vertex_buffer(V_new) ;
	facepoints(V_new) ;
	edgepoints_with_creases(V_new) ;
	vertexpoints_with_creases(V_new) ;
}

void 
Mesh_CC::facepoints(vertex_buffer& V_new) const
{
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int v = Vert(h) ;
		const int i = Vd + Face(h) ;

		const int m = n_vertex_of_polygon(h) ;
		for (int c=0; c < 3; ++c)
		{
			float increm = V_old[v][c] / m ;
CC_ATOMIC
			V_new[i][c] += increm ;
		}
	}
CC_BARRIER
}

void
Mesh_CC::edgepoints(vertex_buffer& V_new) const
{
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int v = Vert(h) ;
		const int j = Vd + Fd + Edge(h) ;
		if (Twin(h) < 0) // Boundary rule: B.1
		{
			const int vv = Vert(Next(h)) ;
			for (int c=0; c < 3; ++c)
			{
				V_new[j][c] = (V_old[v][c] + V_old[vv][c])  / 2.0f ;
			}
		}
		else // B.2
		{
			const int i = Vd + Face(h) ;
			for (int c=0; c < 3; ++c)
			{
				float increm = (V_old[v][c] + V_new[i][c]) / 4.0f ;
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
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int v = Vert(h) ;
		const int c_id = Edge(h) ;
		const int j = Vd + Fd + Edge(h) ;

		const vec3& v_old = V_old[v] ;
		vec3& j_new = V_new[j] ;

		if (is_border_halfedge(h)) // Boundary rule: B.1
		{
			const int vn = Vert(Next(h)) ;
			const vec3& vn_old = V_old[vn] ;
			for (int c=0; c < 3; ++c)
			{
				j_new[c] = (v_old[c] + vn_old[c])  / 2.0f ;
			}
		}
		else
		{
			const float& sharpness = Sigma(c_id) ;
			if (sharpness < 1e-6) // Smooth rule B.2
			{
				const int i = Vd + Face(h) ;
				const vec3& i_new = V_new[i] ;
				for (int c=0; c < 3; ++c)
				{
					float increm = 0.25f * (v_old[c] + i_new[c]) ;
CC_ATOMIC
					j_new[c] += increm ;
				}
			}
			else if (sharpness > 1.0) // Crease rule: B.3
			{
				for (int c=0; c < 3; ++c)
				{
					float increm = 0.5f * v_old[c] ;
CC_ATOMIC
					j_new[c] += increm ;
				}
			}
			else // Blending crease rule: B.4
			{
				const int i = Vd + Face(h) ;
				const vec3& i_new = V_new[i] ;
				for (int c=0; c < 3; ++c)
				{
					const float& vc_old = v_old[c] ;
					float increm_sharp = 0.5f * vc_old ;
					float increm_smooth = 0.25f * (vc_old + i_new[c]) ;
					float increm = std::lerp(increm_smooth,increm_sharp,sharpness) ;
CC_ATOMIC
					j_new[c] += increm ;
				}
			}
		}
	}
CC_BARRIER
}


void
Mesh_CC::vertexpoints(vertex_buffer& V_new) const
{
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int n = vertex_edge_valence_or_border(h) ;
		const int v = Vert(h) ;

		if (n < 0) // Boundary rule: C.1
		{
			const int h_valence = vertex_halfedge_valence(h) ;
			for (int c=0; c < 3; ++c)
			{
				float increm = V_old[v][c] / h_valence ;
CC_ATOMIC
				V_new[v][c] += increm ;
			}
		}
		else // C.2
		{
			const int i = Vd + Face(h) ;
			const int j = Vd + Fd + Edge(h) ;
			const float n2_ = 1.0f / float(n*n) ;
			for (int c=0; c < 3; ++c)
			{
				const float increm = (4.0f * V_new[j][c] - V_new[i][c] + (n-3.0f)*V_old[v][c]) * n2_ ;
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
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int n = vertex_edge_valence_or_border(h) ;
		const int v = Vert(h) ;
		vec3& v_new = V_new[v] ;
		const vec3& v_old = V_old[v] ;

		if (n < 0) // Boundary rule: C.1
		{
			const float vertex_he_valence = float(vertex_halfedge_valence(h)) ;
			for (int c=0; c < 3; ++c)
			{
				float increm = v_old[c] / vertex_he_valence ;
				v_new[c] += increm ;
			}
		}
		else
		{
			const int n_creases = vertex_crease_valence_or_border(h) ;
			if ((n == 2) || n_creases > 2) // Corner vertex rule: C.3
			{
				const float vx_halfedge_valence = 1.0f / float(vertex_halfedge_valence(h)) ;
				for (int c=0; c < 3; ++c)
				{
					float increm = vx_halfedge_valence * v_old[c] ;
CC_ATOMIC
					v_new[c] += increm ;
				}
			}
			else
			{
				const float vx_sharpness = n_creases < 2 ? 0.0f : vertex_sharpness_or_border(h) ; // n_creases < 0 ==> dart vertex ==> smooth
				if (vx_sharpness < 1e-6) // Smooth rule: C.2
				{
					const int i = Vd + Face(h) ;
					const int j = Vd + Fd + Edge(h) ;

					const float n_ = float(n) ;
					const float _n2 = 1.0f / float(n*n) ;

					const vec3& i_new = V_new[i] ;
					const vec3& j_new = V_new[j] ;

					for (int c=0; c < 3; ++c)
					{
						const float increm = (4.0f * j_new[c] - i_new[c] + (n_ - 3.0f)*v_old[c]) * _n2 ;
CC_ATOMIC
						v_new[c] += increm ;
					}
				}
				else // creased case
				{
					const int c_id = Edge(h) ;
					float edge_sharpness = Sigma(c_id) ;
					if (edge_sharpness > 1e-6) // current edge is crease and contributes
					{
						const int j = Vd + Fd + Edge(h) ;
						const vec3& j_new = V_new[j] ;

						if (vx_sharpness > 1.0) // Creased vertex rule: C.5
						{
							for (int c=0; c < 3; ++c)
							{
								const float increm = 0.25f * (j_new[c] + v_old[c]) ;
CC_ATOMIC
								v_new[c] += increm ;
							}
						}
						else // Blended vertex rule: C.4
						{
							for (int c=0; c < 3; ++c)
							{
								const float increm_creased = 0.25f * (j_new[c] + v_old[c]) ;
								const float increm_corner = 0.5f * v_old[c] ;
								float increm = std::lerp(increm_corner,increm_creased,vx_sharpness) ;
CC_ATOMIC
								v_new[c] += increm ;
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
	vertex_buffer& V_new = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int n = vertex_edge_valence_or_border(h) ;
		if (n > 0)
		{
			const float n_ = float(n) ;
            const int v = Vert(h) ;

			const float prod = std::pow((n_ - 3.0f) / n_, 1.0f/n_) ; // sqrt_n((n-3) / n)
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
	vertex_buffer& V_new = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int n = vertex_edge_valence_or_border(h) ;
		const int v = Vert(h) ;

		if (n > 0) // non-boundary case
		{
			const int i = Vd + Face(h) ;
			const int j = Vd + Fd + Edge(h) ;
			const float n2 = float(n*n) ;
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


//Mesh_CC
//Mesh_CC::cube()
//{
//	int H = 24 ;
//	int V = 8 ;
//	int E = 12 ;
//	int F = 6 ;

//	Mesh_CC M(H,V,E,F) ;
//	M.halfedges_cage[0] = HalfEdge(14, 1,3,0,3,0) ;
//	M.halfedges_cage[1] = HalfEdge(19, 2,0,1,7,0) ;
//	M.halfedges_cage[2] = HalfEdge(4 , 3,1,3,0,0) ;
//	M.halfedges_cage[3] = HalfEdge(21, 0,2,2,9,0) ;
//	M.halfedges_cage[4] = HalfEdge(2 , 5,7,2,0,1) ;
//	M.halfedges_cage[5] = HalfEdge(18, 6,4,3,6,1) ;
//	M.halfedges_cage[6] = HalfEdge(8 , 7,5,5,1,1) ;
//	M.halfedges_cage[7] = HalfEdge(22, 4,6,4,10,1) ;
//	M.halfedges_cage[8] = HalfEdge(6 , 9,11,4,1,2) ;
//	M.halfedges_cage[9] = HalfEdge(17, 10,8,5,5,2) ;
//	M.halfedges_cage[10] = HalfEdge(12,11,9,7,2,2) ;
//	M.halfedges_cage[11] = HalfEdge(23,8,10,6,11,2) ;
//	M.halfedges_cage[12] = HalfEdge(10,13,15,6,2,3) ;
//	M.halfedges_cage[13] = HalfEdge(16,14,12,7,4,3) ;
//	M.halfedges_cage[14] = HalfEdge(0 ,15,13,1,3,3) ;
//	M.halfedges_cage[15] = HalfEdge(20,12,14,0,8,3) ;
//	M.halfedges_cage[16] = HalfEdge(13,17,19,1,4,4) ;
//	M.halfedges_cage[17] = HalfEdge(9, 18,16,7,5,4) ;
//	M.halfedges_cage[18] = HalfEdge(5, 19,17,5,6,4) ;
//	M.halfedges_cage[19] = HalfEdge(1, 16,18,3,7,4) ;
//	M.halfedges_cage[20] = HalfEdge(15,21,23,6,8,5) ;
//	M.halfedges_cage[21] = HalfEdge(3, 22,20,0,9,5) ;
//	M.halfedges_cage[22] = HalfEdge(7, 23,21,2,10,5) ;
//	M.halfedges_cage[23] = HalfEdge(11,20,22,04,11,5) ;

//	M.vertices[0] = {0.000000, 0.000000, 1.000000};
//	M.vertices[1] = {1.000000, 0.000000, 1.000000};
//	M.vertices[2] = {0.000000, 1.000000, 1.000000};
//	M.vertices[3] = {1.000000, 1.000000, 1.000000};
//	M.vertices[4] = {0.000000, 1.000000, 0.000000};
//	M.vertices[5] = {1.000000, 1.000000, 0.000000};
//	M.vertices[6] = {0.000000, 0.000000, 0.000000};
//	M.vertices[7] = {1.000000, 0.000000, 0.000000};

//	return M ;
//}


//Mesh_CC
//Mesh_CC::fig1_left()
//{
//	// Set mesh from Fig.1-left as cage
//	int H = 12 ;
//	int V = 7 ;
//	int E = 9 ;
//	int F = 3 ;

//	Mesh_CC S0(H,V,E,F) ;
//	S0.halfedges_cage[0]  = HalfEdge(6, 1, 3, 3, 3, 0) ;
//	S0.halfedges_cage[1] = HalfEdge(7, 2, 0, 2, 4, 0) ;
//	S0.halfedges_cage[2] = HalfEdge(-1, 3, 1, 1, 0, 0) ;
//	S0.halfedges_cage[3] = HalfEdge(-1, 0, 2, 0, 1, 0) ;
//	S0.halfedges_cage[4] = HalfEdge(-1, 5, 6, 3, 2, 1) ;
//	S0.halfedges_cage[5] = HalfEdge(8, 6, 4, 4, 5, 1) ;
//	S0.halfedges_cage[6] = HalfEdge(0, 4, 5, 2, 3, 1) ;
//	S0.halfedges_cage[7] = HalfEdge(1, 8, 11, 1, 4, 2) ;
//	S0.halfedges_cage[8] = HalfEdge(5, 9, 7, 2, 5, 2) ;
//	S0.halfedges_cage[9] = HalfEdge(-1, 10, 8, 4, 6, 2) ;
//	S0.halfedges_cage[10] = HalfEdge(-1, 11, 9, 5, 7, 2) ;
//	S0.halfedges_cage[11] = HalfEdge(-1, 7, 10, 6, 8, 2) ;

//	S0.vertices[0] = {-3, 3, 0} ;
//	S0.vertices[1] = {1, 6, 0} ;
//	S0.vertices[2] = {0, 0, 1} ;
//	S0.vertices[3] = {-4, -4, 0} ;
//	S0.vertices[4] = {3, -5, 0} ;
//	S0.vertices[5] = {6, -2, 0} ;
//	S0.vertices[6] = {5, 4, 0} ;

//	return S0 ;
//}

//Mesh_CC
//Mesh_CC::quad()
//{
//	int H = 4 ;
//	int V = 4 ;
//	int E = 4 ;
//	int F = 1 ;

//	Mesh_CC M(H,V,E,F) ;
//	M.halfedges_cage[0] = HalfEdge(-1,1,3,0,0,0) ;
//	M.halfedges_cage[1] = HalfEdge(-1,2,0,1,1,0) ;
//	M.halfedges_cage[2] = HalfEdge(-1,3,1,2,2,0) ;
//	M.halfedges_cage[3] = HalfEdge(-1,0,2,3,3,0) ;

//	M.vertices[0] = {-10,-10,0} ;
//	M.vertices[1] = {-10,10,0} ;
//	M.vertices[2] = {10,10,0} ;
//	M.vertices[3] = {10,-10,0} ;

//	return M ;
//}
