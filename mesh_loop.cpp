#include "mesh.h"

int
Mesh_Loop::H(int depth) const
{
	const int& d = depth < 0 ? this->depth : depth ;
	return std::pow(4,d) * H0 ;
}

int
Mesh_Loop::F(int depth) const
{
	const int& d = depth < 0 ? this->depth : depth ;
	return std::pow(4,d) * F0 ;
}


int
Mesh_Loop::E(int depth) const
{
	const int& d = depth < 0 ? this->depth : depth ;
	return pow(2,d)*E0 + 3*(pow(2,2*d-1) - pow(2,d-1))*F0 ;
}


int
Mesh_Loop::V(int depth) const
{
	const int& d = depth < 0 ? this->depth : depth ;
	return V0 + (pow(2,d) - 1)*E0 + (pow(2,2*d-1) - 3*pow(2,d-1) + 1)*F0 ;
}


int
Mesh_Loop::Next(int h) const
{
	return h % 3 == 2 ? h - 2 : h + 1 ;
}

int
Mesh_Loop::Next_safe(int h) const
{
	if (h < 0)
		return h ;
	return h % 3 == 2 ? h - 2 : h + 1 ;
}

int
Mesh_Loop::Prev(int h) const
{
	return h % 3 == 0 ? h + 2 : h - 1 ;
}

int
Mesh_Loop::Face(int h) const
{
	return h / 3 ;
}

void
Mesh_Loop::refine_halfedges(halfedge_buffer& new_he) const
{
	const int Hd = H(depth) ;
	const int Vd = V(depth) ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int _3h = 3 * h ;
		const int _3Hd = 3 * Hd ;
		const int h_prime = Prev(h) ;

		int h0_Twin = 3 * Next_safe(Twin(h)) + 2 ;
		int h1_Twin = _3Hd + h ;
		int h2_Twin = 3 * Twin(h_prime);
		int h3_Twin = _3h + 1 ;

		int h0_Next = _3h + 1 ;
		int h1_Next = _3h + 2 ;
		int h2_Next = _3h + 0 ;
		int h3_Next = _3Hd + Next(h) ;

		int h0_Prev = _3h + 2 ;
		int h1_Prev = _3h + 0 ;
		int h2_Prev = _3h + 1 ;
		int h3_Prev = _3Hd + Prev(h) ;

		int h0_Vert = Vert(h) ;
		int h1_Vert = Vd + Edge(h) ;
		int h2_Vert = Vd + Edge(Prev(h)) ;
		int h3_Vert = Vd + Edge(Prev(h)) ;

		int h0_Edge = 2*Edge(h) + (int(h) > Twin(h) ? 0 : 1)  ;
		int h1_Edge = 2*E(depth) + h ;
		int h2_Edge = 2*Edge(h_prime) + (int(h_prime) > Twin(h_prime) ? 1 : 0) ;
		int h3_Edge = h1_Edge ;

		int h0_Face = h ;
		int h1_Face = h ;
		int h2_Face = h ;
		int h3_Face = Hd + Face(h) ;

		new_he[_3h + 0] = HalfEdge(h0_Twin,h0_Next,h0_Prev,h0_Vert,h0_Edge,h0_Face) ;
		new_he[_3h + 1] = HalfEdge(h1_Twin,h1_Next,h1_Prev,h1_Vert,h1_Edge,h1_Face) ;
		new_he[_3h + 2] = HalfEdge(h2_Twin,h2_Next,h2_Prev,h2_Vert,h2_Edge,h2_Face) ;
		new_he[_3Hd + h] = HalfEdge(h3_Twin,h3_Next,h3_Prev,h3_Vert,h3_Edge,h3_Face) ;
	}
CC_BARRIER
}

void
Mesh_Loop::refine_halfedges_old(halfedge_buffer& new_he) const
{
CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int h_prime = Prev(h) ;

		int h0_Twin = 4 * Next_safe(Twin(h)) + 2 ;
		int h1_Twin = 4 * h + 3 ;
		int h2_Twin = 4 * Twin(h_prime) + 0 ;
		int h3_Twin = 4 * h + 1 ;

		int h0_Next = 4 * h + 1 ;
		int h1_Next = 4 * h + 2 ;
		int h2_Next = 4 * h + 0 ;
		int h3_Next = 4 * Next(h) + 3 ;

		int h0_Prev = 4 * h + 2 ;
		int h1_Prev = 4 * h + 0 ;
		int h2_Prev = 4 * h + 1 ;
		int h3_Prev = 4 * Prev(h) + 3 ;

		int h0_Vert = Vert(h) ;
		int h1_Vert = V(depth) + Edge(h) ;
		int h2_Vert = V(depth) + Edge(Prev(h)) ;
		int h3_Vert = h2_Vert ;

		int h0_Edge = 2*Edge(h) + (int(h) > Twin(h) ? 0 : 1)  ;
		int h1_Edge = 2*E(depth) + h ;
		int h2_Edge = 2*Edge(h_prime) + (int(h_prime) > Twin(h_prime) ? 1 : 0) ;
		int h3_Edge = h1_Edge ;

		int h0_Face = h ;
		int h1_Face = h ;
		int h2_Face = h ;
		int h3_Face = H(depth) + Face(h) ;

		new_he[4*h + 0] = HalfEdge(h0_Twin,h0_Next,h0_Prev,h0_Vert,h0_Edge,h0_Face) ;
		new_he[4*h + 1] = HalfEdge(h1_Twin,h1_Next,h1_Prev,h1_Vert,h1_Edge,h1_Face) ;
		new_he[4*h + 2] = HalfEdge(h2_Twin,h2_Next,h2_Prev,h2_Vert,h2_Edge,h2_Face) ;
		new_he[4*h + 3] = HalfEdge(h3_Twin,h3_Next,h3_Prev,h3_Vert,h3_Edge,h3_Face) ;
	}
CC_BARRIER
}


void
Mesh_Loop::refine_vertices_twosteps(vertex_buffer& V_new) const
{
	edgepoints(V_new) ;
	vertexpoints(V_new) ;
}

void
Mesh_Loop::refine_vertices(vertex_buffer& V_new) const
{
	allpoints(V_new) ;
}

void
Mesh_Loop::refine_vertices_with_creases_twosteps(vertex_buffer& V_new) const
{
	edgepoints_with_creases(V_new) ;
	vertexpoints_with_creases(V_new) ;
}

void
Mesh_Loop::refine_vertices_with_creases(vertex_buffer& V_new) const
{
//	allpoints_with_creases(V_new) ;
	refine_vertices_with_creases_twosteps(V_new) ;
}

void
Mesh_Loop::refine_vertices_inplace()
{
	vertex_buffer& Vd = this->vertices ;
	edgepoints(Vd) ;
	vertexpoints_inplace() ;
}

void
Mesh_Loop::edgepoints(vertex_buffer& V_new) const
{
	const vertex_buffer& Vd = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int v = Vert(h) ;
		const int new_odd_pt_id = V(depth) + Edge(h) ;
		if (is_border_halfedge(h)) // Boundary rule
		{
			const int vn = Vert(Next(h)) ;
			for (int c = 0; c < 3; ++c)
			{
				V_new[new_odd_pt_id][c] = (Vd[v][c] + Vd[vn][c])  / 2. ;
			}
		}
		else
		{
			const int vp = Vert(Prev(h)) ;
			for (int c = 0; c < 3; ++c)
			{
				const float increm = (3*Vd[v][c] + Vd[vp][c]) / 8 ;
CC_ATOMIC
				V_new[new_odd_pt_id][c] += increm ;
			}
		}
	}
CC_BARRIER
}

void
Mesh_Loop::edgepoints_with_creases(vertex_buffer& V_new) const
{
	const vertex_buffer& Vd = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int v = Vert(h) ;
		const int new_odd_pt_id = V(depth) + Edge(h) ;
//		if (is_border_halfedge(h)) // Boundary rule A.1
//		{
//			const int vn = Vert(Next(h)) ;
//			for (int c = 0; c < 3; ++c)
//			{
//				V_new[new_odd_pt_id][c] = (Vd[v][c] + Vd[vn][c])  / 2. ;
//			}
//		}
//		else
		{
			const int vp = Vert(Prev(h)) ;
			const int vn = Vert(Next(h)) ;
			const int c_id = Edge(h) ;
			const float sharpness = std::clamp(Sigma(c_id),0.0f,1.0f) ;
			const bool is_border = is_border_halfedge(h) ;

			for (int c = 0; c < 3; ++c)
			{
				const float increm_smooth = 0.375*Vd[v][c] + 0.125*Vd[vp][c] ;
				const float increm_sharp = 0.5 * (is_border ? Vd[v][c] + Vd[vn][c] : Vd[v][c]) ;
				float increm = std::lerp(increm_smooth,increm_sharp,sharpness) ;
CC_ATOMIC
				V_new[new_odd_pt_id][c] += increm ;
			}
		}
	}
CC_BARRIER
}

void
Mesh_Loop::vertexpoints(vertex_buffer& V_new) const
{
	const vertex_buffer& Vd = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int n = vertex_edge_valence_or_border(h) ;
		const int v = Vert(h) ;
		const int vn = Vert(Next(h)) ;

		if (n < 0) // Boundary rule
		{
			// Vd[v]*3/4 + (sum of 2 neighbors) / 8.
			if (is_border_halfedge(h))
			{
				for (int c=0; c < 3; ++c)
				{
					const float incremV = Vd[v][c]*3./8. + Vd[vn][c]*1./8. ;
					const float incremVn = Vd[v][c]*1./8. + Vd[vn][c]*3./8. ;
CC_ATOMIC
					V_new[v][c] += incremV ;
CC_ATOMIC
					V_new[vn][c] += incremVn ;
				}
			}
		}
		else
		{
			const float n_ = 1./float(n) ;
            const float beta = compute_beta(n_) ;
			const float beta_ = n_ - beta ;
			for (int c=0; c < 3; ++c)
			{
				const float increm = beta_*Vd[v][c] + beta*Vd[vn][c] ;
CC_ATOMIC
                V_new[v][c] += increm ;
			}
		}
	}
CC_BARRIER
}

void
Mesh_Loop::vertexpoints_with_creases(vertex_buffer& V_new) const
{
	const vertex_buffer& Vd = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int n = vertex_edge_valence(h) ;
		const int v = Vert(h) ;
		const int vn = Vert(Next(h)) ;

//		if (n < 0) // Boundary rule
//		{
//			// Vd[v]*3/4 + (sum of 2 neighbors) / 8.
//			if (is_border_halfedge(h))
//			{
//				for (int c=0; c < 3; ++c)
//				{
//					const float incremV = Vd[v][c]*3./8. + Vd[vn][c]*1./8. ;
//					const float incremVn = Vd[v][c]*1./8. + Vd[vn][c]*3./8. ;
//CC_ATOMIC
//					V_new[v][c] += incremV ;
//CC_ATOMIC
//					V_new[vn][c] += incremVn ;
//				}
//			}
//		}
//		else
		{
			const int n_creases = vertex_crease_valence(h) ;
			if ((n==2) || n_creases > 2) // Corner vertex rule
			{
				const int vertex_he_valence = vertex_halfedge_valence(h) ;
				for (int c=0; c < 3; ++c)
				{
					const float increm = Vd[v][c] / vertex_he_valence ;
CC_ATOMIC
					V_new[v][c] += increm ;
				}
			}
			else
			{
				const float vx_sharpness = n_creases < 2 ? 0.0f : vertex_sharpness(h) ; // n_creases < 0 ==> dart vertex ==> smooth
				if (vx_sharpness < 1e-6) // smooth
				{
					const float n_ = 1./float(n) ;
					const float beta = compute_beta(n_) ;
					const float beta_ = n_ - beta ;
					for (int c=0; c < 3; ++c)
					{
						const float increm = beta_*Vd[v][c] + beta*Vd[vn][c] ;
CC_ATOMIC
						V_new[v][c] += increm ;
					}
				}
				else // creased or blend
				{
					const int c_id = Edge(h) ;
					const float edge_sharpness = Sigma(c_id) ;
					if (edge_sharpness > 1e-6) // current edge is one of two creases and contributes
					{
						const float interp = std::clamp(vx_sharpness,0.0f,1.0f) ;
						const bool is_border = is_border_halfedge(h) ;

						float increm_corner_factr = 0.5f ;
						float increm_sharp_factr_vn = 0.375f ;
						float increm_sharp_factr_vb = 0.0f ;
						int vb = v ;

						if (is_border)
						{
							increm_corner_factr = 1.0f ;
							increm_sharp_factr_vn = 0.75f ;

							for (int h_it = Prev(h) ; ; h_it = Prev(h_it))
							{
								const int h_it_twin = Twin(h_it) ;
								if (h_it_twin < 0)
								{
									assert(is_crease_halfedge(h_it)) ;
									vb = Vert(h_it) ;
									increm_sharp_factr_vb = 0.125f ;
									break ;
								}

								h_it = h_it_twin ;
							}
						}

						for (int c=0; c < 3; ++c)
						{
							const float increm_corner = increm_corner_factr * Vd[v][c] ;
							const float increm_sharp = 0.125f*Vd[vn][c] + increm_sharp_factr_vn*Vd[v][c] + increm_sharp_factr_vb*Vd[vb][c] ;
							const float incremV = std::lerp(increm_corner,increm_sharp,interp) ;
CC_ATOMIC
							V_new[v][c] += incremV ;
						}

//						if (vx_sharpness >= 1.0) // sharp
//						{
//							for (int c=0; c < 3; ++c)
//							{
//								const float increm = 0.125*Vd[vn][c] + 0.375*Vd[v][c];
//CC_ATOMIC
//								V_new[v][c] += increm ;
//							}
//						}
//						else // blend
//						{
//							for (int c=0; c < 3; ++c)
//							{
//								const float increm_corner = 0.5*Vd[v][c] ;
//								const float increm_sharp = 0.125*Vd[vn][c] + 0.375*Vd[v][c];
//CC_ATOMIC
//								V_new[v][c] += std::lerp(increm_corner,increm_sharp,vx_sharpness) ;
//							}
//						}
					}
				}
			}
		}
	}
CC_BARRIER
}

void
Mesh_Loop::allpoints(vertex_buffer &V_new) const
{
	const vertex_buffer& Vd = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < H(depth) ; ++h)
	{
		const int v = Vert(h) ;
		const int vn = Vert(Next(h)) ;
		const int new_odd_pt_id = V(depth) + Edge(h) ;

		if (is_border_halfedge(h)) // halfedge (thus edge and vertex) is at border
		{	// apply border rules
			for (int c = 0; c < 3; ++c)
			{
				// edge
				V_new[new_odd_pt_id][c] = (Vd[v][c] + Vd[vn][c])  / 2. ;

				// vertex
				const float incremV = Vd[v][c]*3./8. + Vd[vn][c]*1./8. ;
				const float incremVn = Vd[v][c]*1./8. + Vd[vn][c]*3./8. ;
CC_ATOMIC
				V_new[v][c] += incremV ;
CC_ATOMIC
				V_new[vn][c] += incremVn ;
			}
		}
		else // edge is not at border
		{
			{	// apply normal edge rule
				const int vp = Vert(Prev(h)) ;
				for (int c = 0; c < 3; ++c)
				{
					const float increm = (3*Vd[v][c] + Vd[vp][c]) / 8 ;
CC_ATOMIC
					V_new[new_odd_pt_id][c] += increm ;
				}
			}

			const int n = vertex_edge_valence_or_border(h) ;
			if (n > 1) // vertex is not at border
			{	// apply normal vertex rule
				const float n_ = 1./float(n) ;
				const float beta = compute_beta(n_) ;
				const float beta_ = n_ - beta ;
				for (int c=0; c < 3; ++c)
				{
					const float increm = beta_*Vd[v][c] + beta*Vd[vn][c] ;
CC_ATOMIC
					V_new[v][c] += increm ;
				}
			}
		}
	}
CC_BARRIER
}

void
Mesh_Loop::allpoints_with_creases(vertex_buffer &V_new) const
{
	assert(false) ;
}


void
Mesh_Loop::vertexpoints_inplace()
{
	vertexpoints_inplace_pass1() ;
	vertexpoints_inplace_pass2() ;
}

void
Mesh_Loop::vertexpoints_inplace_pass1()
{
	vertex_buffer& Vd = this->vertices ;
	vertex_buffer& V_new = Vd ;

CC_PARALLEL_FOR
    for (int h = 0; h < H(depth) ; ++h)
    {
		int n = vertex_edge_valence_or_border(h) ;
		const int v = Vert(h) ;
		const bool v_at_border = n < 0 ;
		if (v_at_border) // boundary case
        {
			const int vx_halfedge_valence = vertex_halfedge_valence(h) ;
			const float prod = std::pow( 0.5 , 1./float(vx_halfedge_valence)) ;
			for (int c=0; c < 3; ++c)
			{
CC_ATOMIC
				V_new[v][c] *= prod ;
			}
        }
        else
        {
			const float n_ = 1./float(n) ;
			const float prod = std::pow(1.- compute_ngamma(n_), n_) ; // sqrt_n( 1 - gamma )
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
Mesh_Loop::vertexpoints_inplace_pass2()
{
	vertex_buffer& Vd = this->vertices ;
	vertex_buffer& V_new = Vd ;

CC_PARALLEL_FOR
    for (int h = 0; h < H(depth) ; ++h)
    {
		if (is_border_halfedge(h)) // halfedge (and vertex) are at border
		{
			// apply border pass2
			const int v = Vert(h) ;
			const int vn = Vert(Next(h)) ;
			const int i = V(depth) + Edge(h) ; // new odd (edge) vertex id
			for (int c=0; c < 3; ++c)
			{
				const float increm = V_new[i][c] / 4 ;
CC_ATOMIC
				V_new[v][c] += increm ;
CC_ATOMIC
				V_new[vn][c] += increm ;
			}

		}
		else
		{
			const int n = vertex_edge_valence_or_border(h) ;
			if (n >= 0) // vertex is not at border
			{
				const int v = Vert(h) ;
				const float n_ = 1./float(n) ;
				const float gamma = compute_gamma(n_) ;

				const int i = V(depth) + Edge(h) ; // new odd (edge) vertex id
				for (int c=0; c < 3; ++c)
				{
					const float increm = gamma * V_new[i][c] ;
CC_ATOMIC
					V_new[v][c] += increm ;
				}
			}

			// no ELSE: if vertex is at border (but halfedge is not): do nothing
		}
    }
CC_BARRIER
}

float
Mesh_Loop::compute_beta(float one_over_n)
{
    return (5./8. - std::pow((3./8. + std::cos(2 * M_PI * one_over_n)/4.0),2)) * one_over_n ;
}

float
Mesh_Loop::compute_gamma(float one_over_n)
{
	return one_over_n * compute_ngamma(one_over_n);
}

float
Mesh_Loop::compute_ngamma(float one_over_n)
{
	return 1 - 8./5. * std::pow((3./8. + std::cos(2*M_PI * one_over_n)/4.0),2);
}



Mesh_Loop
Mesh_Loop::tri()
{
	int H = 3 ;
	int V = 3 ;
	int E = 3 ;
	int F = 1 ;

	Mesh_Loop M(H,V,E,F) ;
	M.halfedges[0] = HalfEdge(-1,1,2,0,0,0) ;
	M.halfedges[1] = HalfEdge(-1,2,0,1,1,0) ;
	M.halfedges[2] = HalfEdge(-1,0,1,2,2,0) ;

	M.vertices[0] = {1,1,0} ;
	M.vertices[1] = {3,1,0} ;
	M.vertices[2] = {2,2,0} ;

	return M ;
}

Mesh_Loop
Mesh_Loop::polyhedron()
{
	int H = 12 ;
	int V = 4 ;
	int E = 6 ;
	int F = 4 ;

	Mesh_Loop M(H,V,E,F) ;
	M.halfedges[0] = HalfEdge(3,1,2,0,0,0) ;
	M.halfedges[1] = HalfEdge(6,2,0,1,1,0) ;
	M.halfedges[2] = HalfEdge(9,0,1,2,2,0) ;
	M.halfedges[3] = HalfEdge(0,4,5,1,0,1) ;
	M.halfedges[4] = HalfEdge(11,5,3,0,4,1) ;
	M.halfedges[5] = HalfEdge(7,3,4,3,3,1) ;
	M.halfedges[6] = HalfEdge(1,7,8,2,1,2) ;
	M.halfedges[7] = HalfEdge(5,8,6,1,3,2) ;
	M.halfedges[8] = HalfEdge(10,6,7,3,5,2) ;
	M.halfedges[9] = HalfEdge(2,10,11,0,2,3) ;
	M.halfedges[10] = HalfEdge(8,11,9,2,5,3) ;
	M.halfedges[11] = HalfEdge(4,9,10,3,4,3) ;

	M.vertices[0] = {8,8,8} ;
	M.vertices[1] = {24,8,8} ;
	M.vertices[2] = {16,24,8} ;
	M.vertices[3] = {16,8,24} ;

	return M ;
}

