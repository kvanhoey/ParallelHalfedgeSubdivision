#include "mesh.h"

int
Mesh_Loop::H(int depth) const
{
	const int& d = depth < 0 ? this->depth() : depth ;
	return std::pow(4,d) * H0 ;
}

int
Mesh_Loop::F(int depth) const
{
	const int& d = depth < 0 ? this->depth() : depth ;
	return std::pow(4,d) * F0 ;
}


int
Mesh_Loop::E(int depth) const
{
	const int& d = depth < 0 ? this->depth() : depth ;
	return pow(2,d)*E0 + 3*(pow(2,2*d-1) - pow(2,d-1))*F0 ;
}


int
Mesh_Loop::V(int depth) const
{
	const int& d = depth < 0 ? this->depth() : depth ;
	return V0 + (pow(2,d) - 1)*E0 + (pow(2,2*d-1) - 3*pow(2,d-1) + 1)*F0 ;
}


int
Mesh_Loop::Next(int h) const
{
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

int
Mesh_Loop::n_vertex_of_polygon(int h) const
{
	return 3 ;
}

void
Mesh_Loop::refine_halfedges(halfedge_buffer& new_he) const
{
	const int _3Hd = 3 * Hd ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int _3h = 3 * h ;
		const int _3h_p_1 = _3h + 1 ;
		const int h_twin = Twin(h) ;
		const int h_edge = Edge(h) ;

		const int h_prev = Prev(h) ;
		const int h_prev_twin = Twin(h_prev) ;
		const int h_prev_edge = Edge(h_prev) ;

		HalfEdge& h0 = new_he[_3h + 0] ;
		HalfEdge& h1 = new_he[_3h_p_1] ;
		HalfEdge& h2 = new_he[_3h + 2] ;
		HalfEdge& h3 = new_he[_3Hd + h] ;

		h0.Twin = 3 * Next_safe(h_twin) + 2 ;
		h1.Twin = _3Hd + h ;
		h2.Twin = 3 * h_prev_twin ;
		h3.Twin = _3h_p_1 ;

		h0.Vert = Vert(h) ;
		h1.Vert = Vd + h_edge ;
		h2.Vert = Vd + h_prev_edge ;
		h3.Vert = h2.Vert ;

		h0.Edge = 2 * h_edge + (int(h) > h_twin ? 0 : 1)  ;
		h1.Edge = 2 * Ed + h ;
		h2.Edge = 2 * h_prev_edge + (int(h_prev) > h_prev_twin ? 1 : 0) ;
		h3.Edge = h1.Edge ;
	}
CC_BARRIER
}

void
Mesh_Loop::refine_vertices(vertex_buffer& V_new) const
{
	init_vertex_buffer(V_new) ;
	allpoints(V_new) ;
}

void
Mesh_Loop::refine_vertices_with_creases(vertex_buffer& V_new) const
{
	init_vertex_buffer(V_new) ;
	allpoints_with_creases_branchless(V_new) ;
}

void
Mesh_Loop::refine_vertices_inplace()
{
	vertex_buffer& V_old = this->vertices ;
	init_vertex_buffer(V_old,Vd) ;
	edgepoints(V_old) ;
	vertexpoints_inplace() ;
}

void
Mesh_Loop::refine_vertices_twosteps(vertex_buffer& V_new) const
{
	edgepoints(V_new) ;
	vertexpoints(V_new) ;
}

void
Mesh_Loop::refine_vertices_with_creases_twosteps(vertex_buffer& V_new) const
{
	edgepoints_with_creases(V_new) ;
	vertexpoints_with_creases(V_new) ;
}

void
Mesh_Loop::refine_vertices_with_creases_twosteps_branchless(vertex_buffer& V_new) const
{
	edgepoints_with_creases_branchless(V_new) ;
	vertexpoints_with_creases_branchless(V_new) ;
}

void
Mesh_Loop::edgepoints(vertex_buffer& V_new) const
{
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int v = Vert(h) ;
		const int new_odd_pt_id = Vd + Edge(h) ;
		vec3& v_new = V_new[new_odd_pt_id] ;
        const vec3& v_old = V_old[v] ;
		if (is_border_halfedge(h)) // Boundary rule
		{
            const int vn = Vert(Next(h)) ;
            const vec3& vn_old = V_old[vn] ;
            vec3 incremV = 0.5f * (v_old + vn_old) ;
            apply_atomic_vec3_increment(v_new,incremV) ;
		}
		else
		{
			const int vp = Vert(Prev(h)) ;
            const vec3& vp_old = V_old[vp] ;
            const vec3 incremV = 0.375f * v_old + 0.125f * vp_old ;
            apply_atomic_vec3_increment(v_new,incremV) ;
		}
	}
CC_BARRIER
}

void
Mesh_Loop::edgepoints_with_creases(vertex_buffer& V_new) const
{
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int v = Vert(h) ;
		const int new_odd_pt_id = Vd + Edge(h) ;
		const vec3& v_old_vx = V_old[v] ;

		vec3& v_new_edge = V_new[new_odd_pt_id] ;
		{
			const int vp = Vert(Prev(h)) ;
			const int vn = Vert(Next(h)) ;
			const int c_id = Edge(h) ;

			const vec3& vp_old_vx = V_old[vp] ;
			const vec3& vn_old_vx = V_old[vn] ;

			const float edge_sharpness = Sigma(c_id) ;
			if (edge_sharpness < 1e-6)
			{
                const vec3 increm_v_edge = 0.375f * v_old_vx + 0.125f * vp_old_vx ;
                apply_atomic_vec3_increment(v_new_edge, increm_v_edge) ;
			}
			else
			{
				const bool is_border = is_border_halfedge(h) ;
				if (edge_sharpness > 1.0)
				{
                    const vec3 increm_v_edge = 0.5f * (is_border ? v_old_vx + vn_old_vx : v_old_vx) ;
                    apply_atomic_vec3_increment(v_new_edge,increm_v_edge) ;
				}
				else
				{
                    const vec3 increm_smooth_edge = 0.375f * v_old_vx + 0.125f * vp_old_vx ;
                    const vec3 increm_sharp_edge = 0.5f * (is_border ? v_old_vx + vn_old_vx : v_old_vx) ;
                    vec3 increm_v_edge = vec3::lerp(increm_smooth_edge,increm_sharp_edge,edge_sharpness) ;
                    apply_atomic_vec3_increment(v_new_edge,increm_v_edge) ;
				}
			}
		}
	}
CC_BARRIER
}


void
Mesh_Loop::edgepoints_with_creases_branchless(vertex_buffer& V_new) const
{
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int v = Vert(h) ;
		const int new_odd_pt_id = Vd + Edge(h) ;
		vec3& v_new = V_new[new_odd_pt_id] ;

		const int vp = Vert(Prev(h)) ;
		const int vn = Vert(Next(h)) ;
		const vec3& v_old_vx = V_old[v] ;
		const vec3& vp_old_vx = V_old[vp] ;
		const vec3& vn_old_vx = V_old[vn] ;

		const int c_id = Edge(h) ;
		const float sharpness = std::clamp(Sigma(c_id),0.0f,1.0f) ;
		const bool is_border = is_border_halfedge(h) ;

        const vec3 increm_smooth = 0.375f * v_old_vx + 0.125f * vp_old_vx ;
        const vec3 increm_sharp = 0.5f * (is_border ? v_old_vx + vn_old_vx : v_old_vx) ;
        vec3 incremV = vec3::lerp(increm_smooth,increm_sharp,sharpness) ;

        apply_atomic_vec3_increment(v_new,incremV) ;
	}
CC_BARRIER
}

void
Mesh_Loop::vertexpoints(vertex_buffer& V_new) const
{
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int v = Vert(h) ;
		vec3& v_new = V_new[v] ;

		const int n = vertex_edge_valence_or_border(h) ;
		const int vn = Vert(Next(h)) ;
		vec3& vn_new = V_new[vn] ;

		if (n < 0) // Boundary rule
		{
			// Vd[v]*3/4 + (sum of 2 neighbors) / 8.
			if (is_border_halfedge(h))
			{
				for (int c=0; c < 3; ++c)
				{
                    const vec3 incremV  = 0.375f * V_old[v] + 0.125f * V_old[vn] ;
                    const vec3 incremVn = 0.125f * V_old[v] + 0.375f * V_old[vn] ;

                    apply_atomic_vec3_increment(v_new, incremV) ;
                    apply_atomic_vec3_increment(vn_new, incremVn) ;
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
				const float increm = beta_ * V_old[v][c] + beta * V_old[vn][c] ;
CC_ATOMIC
				v_new[c] += increm ;
			}
		}
	}
CC_BARRIER
}

void
Mesh_Loop::vertexpoints_with_creases(vertex_buffer& V_new) const
{
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int v = Vert(h) ;
		vec3& v_new_vx = V_new[v] ;
		const vec3& v_old_vx = V_old[v] ;
		const int n = vertex_edge_valence(h) ;

		const int n_creases = vertex_crease_valence(h) ;
		if ((n==2) || n_creases > 2) // Corner vertex rule
		{
			const int vertex_he_valence = vertex_halfedge_valence(h) ;
			for (int c=0; c < 3; ++c)
			{
				const float increm_corner = v_old_vx[c] / vertex_he_valence ;
CC_ATOMIC
				v_new_vx[c] += increm_corner ;
			}
		}
		else
		{
			const float vx_sharpness = n_creases < 2 ? 0.0f : vertex_sharpness(h) ; // n_creases < 0 ==> dart vertex ==> smooth
			if (vx_sharpness < 1e-6) // smooth
			{
				const int vn = Vert(Next(h)) ;
				const vec3& vn_old_vx = V_old[vn] ;
				const float n_ = 1./float(n) ;
				const float beta = compute_beta(n_) ;
				const float beta_ = n_ - beta ;
				for (int c=0; c < 3; ++c)
				{
					const float increm_smooth = beta_*v_old_vx[c] + beta*vn_old_vx[c] ;
CC_ATOMIC
					v_new_vx[c] += increm_smooth ;
				}
			}
			else // creased or blend
			{
				const int c_id = Edge(h) ;
				const float edge_sharpness = Sigma(c_id) ;
				if (edge_sharpness > 1e-6) // current edge is one of two creases and contributes
				{
					const int vn = Vert(Next(h)) ;
					const vec3& vn_old_vx = V_old[vn] ;
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
					const vec3& vb_old_vx = V_old[vb] ;

					if (vx_sharpness >= 1.0) // sharp
					{
						for (int c=0; c < 3; ++c)
						{
							const float increm_sharp = 0.125f * vn_old_vx[c] + increm_sharp_factr_vn * v_old_vx[c] + increm_sharp_factr_vb * vb_old_vx[c] ;
CC_ATOMIC
							v_new_vx[c] += increm_sharp ;
						}
					}
					else // blend
					{
						for (int c=0; c < 3; ++c)
						{
							const float increm_corner = 0.5*v_old_vx[c] ;
							const float increm_sharp = 0.125f * vn_old_vx[c] + increm_sharp_factr_vn * v_old_vx[c] + increm_sharp_factr_vb * vb_old_vx[c] ;
CC_ATOMIC
							v_new_vx[c] += std::lerp(increm_corner,increm_sharp,vx_sharpness) ;
						}
					}
				}
			}
		}
	}
CC_BARRIER
}

void
Mesh_Loop::vertexpoints_with_creases_branchless(vertex_buffer& V_new) const
{
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int v = Vert(h) ;
		const int vn = Vert(Next(h)) ;

		vec3& v_new_vx = V_new[v] ;
		const vec3& v_old_vx = V_old[v] ;
		const vec3& vn_old_vx = V_old[vn] ;

		const int n = vertex_edge_valence(h) ;
		const int n_creases = vertex_crease_valence(h) ;
		const int vertex_he_valence = vertex_halfedge_valence(h) ;

		const int c_id = Edge(h) ;
		const float edge_sharpness = Sigma(c_id) ;
		const float vx_sharpness = n_creases < 2 ? 0.0f : vertex_sharpness(h) ; // n_creases < 0 ==> dart vertex ==> smooth

		const float lerp_alpha = std::clamp(vx_sharpness,0.0f,1.0f) ;

		// utility notations
		const float n_ = 1./float(n) ;
		const float beta = compute_beta(n_) ;
		const float beta_ = n_ - beta ;


		float edge_sharpness_factr = edge_sharpness < 1e-6 ? 0.0 : 1.0 ;

		// border correction
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

		const vec3 increm_corner = v_old_vx / vertex_he_valence ;
		const vec3 increm_smooth = beta_*v_old_vx + beta*vn_old_vx ;
		const vec3 increm_sharp = edge_sharpness_factr * (0.125f * vn_old_vx + increm_sharp_factr_vn * v_old_vx + increm_sharp_factr_vb * V_old[vb]) ;

		if ((n==2) || n_creases > 2) // Corner vertex rule
		{
            apply_atomic_vec3_increment(v_new_vx,increm_corner) ;
		}
        else if (vx_sharpness < 1e-6) // smooth
        {
            apply_atomic_vec3_increment(v_new_vx,increm_smooth) ;
        }
        else // creased or blend
        {
            const vec3 incremV = vec3::lerp(increm_corner,increm_sharp,lerp_alpha) ;
            apply_atomic_vec3_increment(v_new_vx, incremV) ;
        }
    }
CC_BARRIER
}

void
Mesh_Loop::allpoints(vertex_buffer &V_new) const
{
	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		const int v = Vert(h) ;
        const int vn = Vert(Next(h)) ;

        const vec3& v_old = V_old[v] ;
        const vec3& vn_old = V_old[vn] ;
        vec3& v_new_vx = V_new[v] ;

		const int new_odd_pt_id = Vd + Edge(h) ;
        vec3& v_new_edge = V_new[new_odd_pt_id] ;

		if (is_border_halfedge(h)) // halfedge (thus edge and vertex) is at border
		{	// apply border rules

            // edge
            const vec3 increm_edge = 0.5f * (v_old + vn_old) ;
            apply_atomic_vec3_increment(v_new_edge, increm_edge) ;

            // vertex
            const vec3 incremV  = 0.375f * v_old + 0.125f * vn_old ;
            const vec3 incremVn = 0.125f * v_old + 0.325f * vn_old ;
            apply_atomic_vec3_increment(v_new_vx, incremV) ;
            apply_atomic_vec3_increment(v_new_vx, incremVn) ;
        }
		else // edge is not at border
		{
            // apply normal edge rule
            const int vp = Vert(Prev(h)) ;
            vec3 increm_edge = 0.375f * v_old + 0.125f * V_old[vp] ;
            apply_atomic_vec3_increment(v_new_edge, increm_edge) ;

			const int n = vertex_edge_valence_or_border(h) ;
			if (n > 1) // vertex is not at border
			{	// apply normal vertex rule
				const float n_ = 1./float(n) ;
				const float beta = compute_beta(n_) ;
				const float beta_ = n_ - beta ;
                const vec3 increm_vx = beta_ * v_old + beta * vn_old ;

                apply_atomic_vec3_increment(v_new_vx, increm_vx) ;
			}
		}
	}
CC_BARRIER
}

void
Mesh_Loop::allpoints_with_creases_branchless(vertex_buffer &V_new) const
{
	// NOTE. This currently is only a copy-pasting of edgepoints_with_creases_branchless and vertexpoints_with_creases_branchless
	// within the same loop on halfedges.
	// TODO: one can probably exploit the single-loop case more optimally !

	const vertex_buffer& V_old = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
	{
		// edgepoints
		const int v = Vert(h) ;
		const int new_odd_pt_id = Vd + Edge(h) ;
		vec3& v_new = V_new[new_odd_pt_id] ;

		const int vp = Vert(Prev(h)) ;
		const int vn = Vert(Next(h)) ;
		const vec3& v_old_vx = V_old[v] ;
		const vec3& vp_old_vx = V_old[vp] ;
		const vec3& vn_old_vx = V_old[vn] ;

		const int c_id = Edge(h) ;
		const float sharpness = std::clamp(Sigma(c_id),0.0f,1.0f) ;
		const bool is_border = is_border_halfedge(h) ;

        const vec3 increm_smooth_edge = 0.375f * v_old_vx + 0.125f * vp_old_vx ;
        const vec3 increm_sharp_edge = 0.5f * (is_border ? v_old_vx + vn_old_vx : v_old_vx) ;
        vec3 increm = vec3::lerp(increm_smooth_edge,increm_sharp_edge,sharpness) ;
        apply_atomic_vec3_increment(v_new, increm) ;

		// vertex points
        vec3& v_new_vx = V_new[v] ;

        const int n = vertex_edge_valence(h) ;
        const int n_creases = vertex_crease_valence(h) ;
        const int vertex_he_valence = vertex_halfedge_valence(h) ;

        const float edge_sharpness = Sigma(c_id) ;
        const float vx_sharpness = n_creases < 2 ? 0.0f : vertex_sharpness(h) ; // n_creases < 0 ==> dart vertex ==> smooth

        const float lerp_alpha = std::clamp(vx_sharpness,0.0f,1.0f) ;

        // utility notations
        const float n_ = 1./float(n) ;
        const float beta = compute_beta(n_) ;
        const float beta_ = n_ - beta ;


        float edge_sharpness_factr = edge_sharpness < 1e-6 ? 0.0 : 1.0 ;

        // border correction
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

        const vec3 increm_corner_vx = v_old_vx / vertex_he_valence ;
        const vec3 increm_smooth_vx = beta_*v_old_vx + beta*vn_old_vx ;
        const vec3 increm_sharp_vx = edge_sharpness_factr * (0.125f * vn_old_vx + increm_sharp_factr_vn * v_old_vx + increm_sharp_factr_vb * V_old[vb]) ;

        if ((n==2) || n_creases > 2) // Corner vertex rule
        {
            apply_atomic_vec3_increment(v_new_vx,increm_corner_vx) ;
        }
        else if (vx_sharpness < 1e-6) // smooth
        {
            apply_atomic_vec3_increment(v_new_vx,increm_smooth_vx) ;
        }
        else // creased or blend
        {
            const vec3 incremV = vec3::lerp(increm_corner_vx,increm_sharp_vx,lerp_alpha) ;
            apply_atomic_vec3_increment(v_new_vx, incremV) ;
        }
	}
CC_BARRIER
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
	vertex_buffer& V_new = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
    {
		const int v = Vert(h) ;
		vec3& v_new = V_new[v] ;

		int n = vertex_edge_valence_or_border(h) ;
		const bool v_at_border = n < 0 ;
		if (v_at_border) // boundary case
        {
			const int vx_halfedge_valence = vertex_halfedge_valence(h) ;
			const float prod = std::pow( 0.5f , 1./float(vx_halfedge_valence)) ;
			for (int c=0; c < 3; ++c)
			{
CC_ATOMIC
				v_new[c] *= prod ;
			}
        }
        else
        {
			const float n_ = 1./float(n) ;
			const float prod = std::pow(1. - compute_ngamma(n_), n_) ; // sqrt_n( 1 - gamma )
			for (int c = 0 ; c < 3 ; ++c)
            {
CC_ATOMIC
				v_new[c] *= prod ;
            }
        }
    }
CC_BARRIER
}

void
Mesh_Loop::vertexpoints_inplace_pass2()
{
	vertex_buffer& V_new = this->vertices ;

CC_PARALLEL_FOR
	for (int h = 0; h < Hd ; ++h)
    {
		if (is_border_halfedge(h)) // halfedge (and vertex) are at border
		{
			// apply border pass2
			const int v = Vert(h) ;
			vec3& v_new = V_new[v] ;

			const int vn = Vert(Next(h)) ;
            vec3& vn_new = V_new[vn] ;

			const int i = Vd + Edge(h) ; // new odd (edge) vertex id

            const vec3 increm = 0.250f * V_new[i] ;
            apply_atomic_vec3_increment(v_new,increm) ;
            apply_atomic_vec3_increment(vn_new,increm) ;
		}
		else
		{
			const int n = vertex_edge_valence_or_border(h) ;
			if (n >= 0) // vertex is not at border
			{
				const int v = Vert(h) ;
				vec3& v_new = V_new[v] ;

				const float n_ = 1./float(n) ;
				const float gamma = compute_gamma(n_) ;

				const int i = Vd + Edge(h) ; // new odd (edge) vertex id
                const vec3& v_new_edge = V_new[i] ;
                const vec3 increm = gamma * v_new_edge ;
                apply_atomic_vec3_increment(v_new, increm) ;
			}

			// no ELSE: if vertex is at border (but halfedge is not): do nothing
		}
    }
CC_BARRIER
}

float
Mesh_Loop::compute_beta(float one_over_n)
{
	return (_5_o_8 - std::pow((_3_o_8 + std::cos(_2pi * one_over_n) * 0.250),2)) * one_over_n ;
}

float
Mesh_Loop::compute_gamma(float one_over_n)
{
	return one_over_n * compute_ngamma(one_over_n);
}

float
Mesh_Loop::compute_ngamma(float one_over_n)
{
	return 1 - _8_o_5 * std::pow((_3_o_8 + std::cos(_2pi * one_over_n)* 0.250f),2);
}

//Mesh_Loop
//Mesh_Loop::tri()
//{
//	int H = 3 ;
//	int V = 3 ;
//	int E = 3 ;
//	int F = 1 ;

//	Mesh_Loop M(H,V,E,F) ;
//	M.halfedges[0] = HalfEdge(-1,1,2,0,0,0) ;
//	M.halfedges[1] = HalfEdge(-1,2,0,1,1,0) ;
//	M.halfedges[2] = HalfEdge(-1,0,1,2,2,0) ;

//	M.vertices[0] = {1,1,0} ;
//	M.vertices[1] = {3,1,0} ;
//	M.vertices[2] = {2,2,0} ;

//	return M ;
//}

//Mesh_Loop
//Mesh_Loop::polyhedron()
//{
//	int H = 12 ;
//	int V = 4 ;
//	int E = 6 ;
//	int F = 4 ;

//	Mesh_Loop M(H,V,E,F) ;
//	M.halfedges[0] = HalfEdge(3,1,2,0,0,0) ;
//	M.halfedges[1] = HalfEdge(6,2,0,1,1,0) ;
//	M.halfedges[2] = HalfEdge(9,0,1,2,2,0) ;
//	M.halfedges[3] = HalfEdge(0,4,5,1,0,1) ;
//	M.halfedges[4] = HalfEdge(11,5,3,0,4,1) ;
//	M.halfedges[5] = HalfEdge(7,3,4,3,3,1) ;
//	M.halfedges[6] = HalfEdge(1,7,8,2,1,2) ;
//	M.halfedges[7] = HalfEdge(5,8,6,1,3,2) ;
//	M.halfedges[8] = HalfEdge(10,6,7,3,5,2) ;
//	M.halfedges[9] = HalfEdge(2,10,11,0,2,3) ;
//	M.halfedges[10] = HalfEdge(8,11,9,2,5,3) ;
//	M.halfedges[11] = HalfEdge(4,9,10,3,4,3) ;

//	M.vertices[0] = {8,8,8} ;
//	M.vertices[1] = {24,8,8} ;
//	M.vertices[2] = {16,24,8} ;
//	M.vertices[3] = {16,8,24} ;

//	return M ;
//}

