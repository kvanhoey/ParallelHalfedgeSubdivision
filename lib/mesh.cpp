#include "mesh.h"

// ----------- Constructor/destructor -----------
Mesh::Mesh(const std::string& filename)
{
	read_from_obj(filename) ;
}

// ----------- Accessors -----------
int
Mesh::H(int depth) const
{
	assert(depth <= 0) ;
	return H_count ;
}

int
Mesh::E(int depth) const
{
	assert(depth <= 0) ;
	return E_count ;
}

int
Mesh::V(int depth) const
{
	assert(depth <= 0) ;
	return V_count ;
}

int
Mesh::F(int depth) const
{
	assert(depth <= 0) ;
	return F_count ;
}

int
Mesh::C(int depth) const
{
	assert(depth <= 0) ;
	return E_count ;
}

// ----------- Accessors for halfedge and crease values from specified buffers -----------
int
Mesh::Twin(const halfedge_buffer& buffer, int idx) const
{
	return buffer[idx].Twin ;
}

int
Mesh::Prev(const halfedge_buffer_cage& buffer, int idx) const
{
	return buffer[idx].Prev ;
}

int
Mesh::Next(const halfedge_buffer_cage& buffer, int idx) const
{
	return buffer[idx].Next ;
}

int
Mesh::Vert(const halfedge_buffer& buffer, int idx) const
{
	return buffer[idx].Vert ;
}

int
Mesh::Edge(const halfedge_buffer& buffer, int idx) const
{
	return buffer[idx].Edge ;
}

int
Mesh::Face(const halfedge_buffer_cage& buffer, int idx) const
{
	return buffer[idx].Face ;
}

float
Mesh::Sharpness(const crease_buffer& buffer, int idx) const
{
	return idx > buffer.size() ? 0. : buffer[idx].Sharpness ;
}

int
Mesh::NextC(const crease_buffer& buffer, int idx) const
{
	return buffer[idx].Next ;
}

int
Mesh::PrevC(const crease_buffer& buffer, int idx) const
{
	return buffer[idx].Prev ;
}

// ----------- Accessors for halfedge and crease values from the base mesh buffers -----------
int
Mesh::Twin(int idx) const
{
	return Twin(halfedges,idx) ;
}

int
Mesh::Prev(int idx) const
{
	return halfedges_cage[idx].Prev ;
}

int
Mesh::Next(int idx) const
{
	return Next(halfedges_cage,idx) ;
}

int
Mesh::Vert(int idx) const
{
	return Vert(halfedges,idx) ;
}

int
Mesh::Edge(int idx) const
{
	return Edge(halfedges,idx) ;
}

int
Mesh::Face(int idx) const
{
	return Face(halfedges_cage,idx) ;
}

float
Mesh::Sharpness(int idx) const
{
	return Sharpness(creases, idx) ;
}

int
Mesh::NextC(int idx) const
{
	return NextC(creases,idx) ;
}

int
Mesh::PrevC(int idx) const
{
	return PrevC(creases, idx) ;
}

int
Mesh::Next_safe(int idx) const
{
	return idx < 0 ? idx : Next(idx) ;
}


// ----------- utility functions for evaluating local configurations -----------
bool
Mesh::is_border_halfedge(const halfedge_buffer& buffer, int h) const
{
	return Twin(buffer,h) < 0 ;
}

bool
Mesh::is_crease_edge(const crease_buffer& buffer, int crease_id) const
{
	return Sharpness(buffer,crease_id) > _epsilon_ ;
}

int
Mesh::vertex_halfedge_valence(const halfedge_buffer& h_buffer, int h) const
{
	int n = 1 ;

	int h_back_valid = h ;
	int h_back = Twin(h_buffer,Prev(h)) ;

	// rewind to border (if any)
	while ((h_back != h) && (h_back >= 0))
	{
		h_back_valid = h_back ;
		h_back = Twin(h_buffer,Prev(h_back)) ;
	}

	// fw to border or back to start and count
	int h_fw = Next_safe(Twin(h_buffer,h_back_valid)) ;
	while ((h_fw != h_back_valid) && (h_fw >= 0))
	{
		n++ ;
		h_fw = Next_safe(Twin(h_buffer,h_fw)) ;
	}

	return n ;
}

int
Mesh::vertex_edge_valence(const halfedge_buffer& buffer, int h) const
{
	int n = 1 ;

	int h_back_valid = h ;
	int h_back = Twin(buffer,Prev(h)) ;
	
	// rewind to border (if any)
	while ((h_back != h) && (h_back >= 0))
	{
		h_back_valid = h_back ;
		h_back = Twin(buffer,Prev(h_back)) ;
	}

	// fw to border or back to start and count
	int h_fw = Next_safe(Twin(buffer,h_back_valid)) ;
	while ((h_fw != h_back_valid) && (h_fw >= 0))
	{
		n++ ;
		h_fw = Next_safe(Twin(buffer,h_fw)) ;
	}

	if (h_fw < 0)
		n++ ;
	
	return n ;
}

int
Mesh::vertex_crease_valence(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const
{
	int n = int(is_crease_halfedge(h_buffer, c_buffer, h)) ;

	int h_it ;
	for (h_it = Twin(h_buffer,h) ; h_it >= 0 ; h_it = Twin(h_buffer, h_it))
	{
		h_it = Next(h_it) ;

		if (h_it == h)
			break ;

		if (is_crease_halfedge(h_buffer, c_buffer, h_it))
			n++ ;
	}

	if (h_it < 0)
	{	// do backward iteration too
		for (h_it = h ; h_it >= 0 ; h_it = Twin(h_buffer, h_it))
		{
			h_it = Prev(h_it) ;
			if (is_crease_halfedge(h_buffer, c_buffer, h_it))
				n++ ;
		}
	}

	return n ;
}

int
Mesh::vertex_edge_valence_or_border(const halfedge_buffer& h_buffer, int h) const
{
	int n = 0 ;

	int hh = h ;
	do
	{
		n++ ;
		const int hh_twin = Twin(h_buffer, hh) ;

		bool is_border = hh_twin < 0 ;
		if (is_border)
			return -1 ;

		hh = Next(hh_twin) ;
	}
	while(hh != h);

	return n ;
}

int
Mesh::vertex_crease_valence_or_border(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const
{
	int n = 0 ;

	int hh = h ;
	do
	{
		if (is_crease_halfedge(h_buffer, c_buffer, hh))
			n++ ;

		int hh_twin = Twin(h_buffer, hh) ;
		bool is_border = hh_twin < 0 ;
		if (is_border)
			return -1 ;

		hh = Next(hh_twin) ;
	}
	while(hh != h);

	return n ;
}

float
Mesh::vertex_sharpnesssum_or_border(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const
{
	float edge_valence = 0.0 ;
	float sharpness = 0 ;

	int hh = h ;
	do
	{
		sharpness += Sharpness(c_buffer, Edge(h_buffer, hh)) ;
		++edge_valence ;

		const int hh_twin = Twin(h_buffer, hh) ;
		const bool is_border = hh_twin < 0 ;
		if (is_border)
			return -1.0f ;

		hh = Next(hh_twin) ;
	}
	while(hh != h);

	return sharpness ;
}

float
Mesh::vertex_sharpness_sum(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const
{
	float edge_valence = 1.0 ;
	float sharpness = Sharpness(c_buffer, Edge(h_buffer, h)) ;

	int h_it ;
	for (h_it = Twin(h_buffer, h) ; h_it >= 0 ; h_it = Twin(h_buffer, h_it))
	{
		h_it = Next(h_it) ;
		if (h_it == h)
			break ;

		edge_valence++ ;
		sharpness += Sharpness(c_buffer, Edge(h_buffer, h_it)) ;
	}

	if (h_it < 0)
	{	// do backward iteration too
		for (h_it = h ; h_it >= 0 ; h_it = Twin(h_buffer, h_it))
		{
			h_it = Prev(h_it) ;
			edge_valence++ ;
			sharpness += Sharpness(c_buffer, Edge(h_buffer, h_it)) ;
		}
	}

	return sharpness ;
}

bool
Mesh::is_crease_halfedge(const halfedge_buffer& h_buffer, const crease_buffer& c_buffer, int h) const
{
	return Sharpness(c_buffer,Edge(h_buffer,h)) > _epsilon_ ;
}

int
Mesh::n_vertex_of_polygon(int h) const
{
	int n = 1 ;
	for (int h_fw = Mesh::Next(h) ; h_fw != h ; h_fw = Mesh::Next(h_fw))
	{
		++n ;
	}
	return n ;
}

// ----------- Public member functions for mesh inspection -----------
bool
Mesh::check() const
{
	bool valid = true ;

	if (H_count < 2)
	{
		std::cerr << "The mesh is empty" << std::endl ;
		return false ;
	}

	for (int h = 0; h < H_count ; ++h)
	{
		const int h_twin = Twin(h) ;
		const int h_edge = Edge(h) ;
		const int h_vert = Vert(h) ;

		bool check_twin = (is_border_halfedge(halfedges,h) || h == Twin(h_twin)) ;
		if (!check_twin)
			std::cout << h << " : Twin(h)=" << h_twin << " ; Twin(Twin(h))=" << Twin(h_twin) << std::endl ;
		assert(check_twin) ;
		valid &= check_twin ;

		bool check_prevnext = (Next(Prev(h)) == h && Prev(Next(h)) == h) ;
		if (!check_prevnext)
			std::cerr << "Assert: PrevNext" << std::endl ;
		assert(check_prevnext) ;
		valid &= check_prevnext ;

		// two twins have same edge
		bool check_twinedge = (is_border_halfedge(halfedges,h) || (Edge(h_twin) == h_edge)) ;
		if (!check_twinedge)
			std::cerr << "Assert: TwinEdge" << std::endl ;
		assert(check_twinedge) ;
		valid &= check_twinedge ;

		bool check_valid_ids = (h_twin < H_count && Next(h) < H_count && Prev(h) < H_count && h_vert < V_count && h_edge < E_count && Face(h) < F_count) ;
		if (!check_valid_ids)
			std::cerr << "Assert: Invalid Ids" << std::endl ;
		valid &= check_valid_ids ;

		assert(check_valid_ids) ;
	}

	return valid ;
}

std::string
Mesh::export_to_tikz(int depth, float alpha) const
{
    if (H_count < 2)
    {
        std::cerr << "The mesh is empty" << std::endl ;
        return false ;
    }


    std::stringstream ss ;

    ss << "%% FACES %%" << std::endl ;
    for (int h = 0; h < H_count ; ++h)
    {
        const int& v0_idx = Vert(h);
        const int& vn_idx = Vert(Next(h));
        const int& vp_idx = Vert(Prev(h));
        const vec3& v0 = vertices[v0_idx];
        const vec3& vn = vertices[vn_idx];
        const vec3& vp = vertices[vp_idx];

        ss << "\\fill[mygreylighter]" << std::endl ;
        ss << "\t(" << v0[0] << ", " << v0[1] << ") -- ";
        ss << "(" << vn[0] << ", " << vn[1] << ") -- ";
        ss << "(" << vp[0] << ", " << vp[1] << ") -- cycle;" << std::endl ;
    }

    ss << "%% EDGES %%" << std::endl ;
    for (int h = 0; h < H_count ; ++h)
    {
        const int& v0_idx = Vert(h);
        const int& v1_idx = Vert(Next(h));
        const vec3& v0 = vertices[v0_idx];
        const vec3& v1 = vertices[v1_idx];
        const vec3& d = v1 - v0;

        if ((h >= 0 && h < pow(3,depth+1)) || (Twin(h) >= 0 && Twin(h) < pow(3,depth+1)))
            ss << "\\draw[myorange, line width = 0.900000]" << std::endl ;
        else
            ss << "\\draw[mygreylight, line width = 0.900000]" << std::endl ;
        ss << "\t(" << v0[0] << ", " << v0[1] << ") -- (" << v0[0] + d[0] << ", " << v0[1] + d[1] << ");" << std::endl ;
    }

    ss << "%% Halfedges %%" << std::endl ;
    for (int h = 0; h < H_count ; ++h)
    {
        const int& v0_idx = Vert(h);
        const int& vn_idx = Vert(Next(h));
        const int& vp_idx = Vert(Prev(h));
        const vec3& v0 = vertices[v0_idx];
        const vec3& vn = vertices[vn_idx];
        const vec3& vp = vertices[vp_idx];

        const vec3 e1 = vn - v0;
        const vec3 e2 = vp - v0;

        float offset = 0.1 ;

        const vec3& va = v0 + offset*(e1 + e2);

        const vec3& d = 0.68*(vn - v0);

        int hh = h/std::pow(3,depth) ;

        if (hh == 0 || h == 117)
            ss << "\\draw[myred, line width = 0.150000, ->, > = stealth, -{Latex[length = 1.700000, width = 1.700000]}]" << std::endl ;
        else if (hh==1 || h == 118)
            ss << "\\draw[mygreen, line width = 0.150000, ->, > = stealth, -{Latex[length = 1.700000, width = 1.700000]}]" << std::endl ;
        else if (hh==2 || h == 119)
            ss << "\\draw[myblue, line width = 0.150000, ->, > = stealth, -{Latex[length = 1.700000, width = 1.700000]}]" << std::endl ;
        else
            ss << "\\draw[mygrey, line width = 0.150000, ->, > = stealth, -{Latex[length = 1.700000, width = 1.700000]}]" << std::endl ;
        ss << "\t(" << va[0] << ", " << va[1] << ") -- (" << va[0] + alpha*d[0] << ", " << va[1] + alpha*d[1] << ");" << std::endl ;
    }

    ss << "%% Vertices %%" << std::endl ;
    float circlesize = (depth ==0) ? 0.17 : 0.1 ;
    for (int h = 0; h < H_count ; ++h)
    {
        const int& v0_idx = Vert(h);
        const vec3& v0 = vertices[v0_idx];

        ss << "\\draw[mygreylighter, fill = mygreylight, line width = 0.150000]" << std::endl ;
        ss << "\t(" << v0[0] << ", " << v0[1] << ") circle ( " << circlesize << ");" << std::endl ;
    }


    return ss.str() ;
}

bool
Mesh::is_tri_only() const
{
	return all_faces_are_ngons(3);
}

bool
Mesh::is_quad_only() const
{
	return all_faces_are_ngons(4);
}

int
Mesh::count_sharp_creases() const
{
	int counter = 0 ;
	for (int c = 0; c < C_count ; ++c)
	{
		if (Sharpness(c) > _epsilon_)
			counter ++ ;
	}

	return counter ;
}

int
Mesh::count_border_edges() const
{
	int counter = 0 ;
	for (int h = 0; h < H_count ; ++h)
	{
		if (Twin(h) < 0)
			counter ++ ;
	}

	return counter ;
}

bool
Mesh::all_faces_are_ngons(int n) const
{
	for (int h = 0; h < H_count ; ++h)
	{
		bool is_ngon = n_vertex_of_polygon(h) == n ;
		if (!is_ngon)
			return false ;
	}

	return H_count > 0 ;
}

void Mesh::export_to_obj(const std::string& filename) const
{
	std::ofstream file(filename) ;

	file << "# Vertices" << std::endl ;
	for (int v = 0 ; v < vertices.size() ; ++v)
	{
		file << "v " << vertices[v][0] << " " << vertices[v][1] << " " << vertices[v][2] << std::endl ;
	}

	file << "# Topology" ;
	int f_id_prev = -1 ;
	for (int h = 0 ; h < halfedges.size() ; ++h)
	{
		const int f_id = Face(h) ;
		if (f_id != f_id_prev)
		{
			file << std::endl << "f " ;
			f_id_prev = f_id ;
		}
		file << 1+Vert(h) << " " ;
	}

	file.close() ;
}

// ----------- Functions for loading and exporting from/to OBJ files. -----------
void
Mesh::read_obj_mesh_size(std::ifstream& file, int& h_count, int& v_count, int& f_count)
{
	v_count = 0 ;
	f_count = 0 ;
	h_count = 0 ;

	// Read quantities
	std::string line ;
	while(std::getline(file,line))
	{
		std::istringstream iss(line) ;
		char c = '\0' ;
		iss >> c ;
		if (c == 'v' && iss.peek() == ' ')
			v_count ++ ;
		else if (c == 'f')
		{
			f_count ++ ;

			int v_id ;
			int count_n_vert_per_face = 0 ;
			while (iss >> v_id)
			{
				count_n_vert_per_face++ ;
				h_count ++ ;

				if (iss.peek() == '/')  // if next is '/'
					iss.ignore(999,' ') ;      // advance to next space
			}
		}
	}
}

Mesh::crease_buffer
Mesh::read_obj_data(std::ifstream& file)
{
	halfedge_buffer_cage& He_cage = this->halfedges_cage ;
	halfedge_buffer& He = this->halfedges ;
	vertex_buffer& Vx = this->vertices ;

	crease_buffer Cr  ;

	// Read actual data
	int v = 0 ;
	int f = 0 ;
	int h = 0 ;

	std::string line ;
	while(std::getline(file,line))
	{
		std::istringstream iss(line) ;
		char c ;
		iss >> c ;
		if (c == 'v' && iss.peek() == ' ') // 'v x y z'
		{
			for (int i = 0 ; i < 3 ; ++i)
			{
				float val ;
				iss >> val ;
				Vx[v][i] = val ;
			}
			++v ;
		}
		else if (c == 'f' && iss.peek() == ' ') // 'f v1 v2 ... vn'
		{
			int v_id ;
			std::vector<int> v_ids ;
			while (iss >> v_id)
			{
				v_ids.push_back(v_id) ;

				if (iss.peek() == '/')  // if next is '/'
					iss.ignore(999,' ') ;      // advance to next space
			}
			int n = v_ids.size() ;
			for (int i = 0 ; i < n; ++i)
			{

				int twin = -1 ; // undetermined at this time

				int next = h + 1 ;
				if (i == n-1)
					next -= n ;

				int prev = h - 1 ;
				if (i == 0)
					prev += n ;

				int vert = v_ids[i] - 1 ; // obj is 1-based.
				int edge = -1 ; // undetermined at this time
				int face = f ;

				He[h].Twin = twin ;
				He[h].Vert = vert ;
				He[h].Edge = edge ;
				He_cage[h].Next = next ;
				He_cage[h].Prev = prev ;
				He_cage[h].Face = face ;

				++h ;
			}
			++f ;
		}
		else if (c == 't' && iss.peek() == ' ') // 't '
		{
			std::string type ;
			iss >> type ;
			iss.ignore(999,' ') ;      // advance to next space
			iss.ignore(999,' ') ;      // advance to next space

			float sharpness ;
			if (type == "crease")
			{
				int v0, v1 ;
				iss >> v0 >> v1 >> sharpness ;

				// temporary storage
				Crease cr ;
				cr.Sharpness = sharpness ;
				cr.Next = v0 ;
				cr.Prev = v1 ;
				Cr.push_back(cr) ;
			}
//			else if (type == "corner")
//			{
//				int vert ;
//				iss >> vert >> sharpness ;
//			}
		}
	}

	return Cr ;
}

void
Mesh::read_from_obj(const std::string& filename)
{
	std::ifstream file ;
	file.open(filename) ;

	int h_count, v_count, f_count ;
	read_obj_mesh_size(file,h_count,v_count,f_count) ;

	// set constants and alloc
	this->H_count = h_count ;
	this->V_count = v_count ;
	this->F_count = f_count ;
	halfedges.resize(H_count) ;
	halfedges_cage.resize(H_count) ;
	vertices.resize(V_count) ;

	 // rewind
	file.clear() ;
	file.seekg(0) ;

	// set he and vx buffers
	crease_buffer tmp_creases = read_obj_data(file) ;

	file.close() ;

	// Twins
	compute_and_set_twins() ;

	// Edge
	this->E_count = compute_and_set_edges() ;

	// Creases
	this->C_count = this->E_count ;
	creases.resize(C_count) ;
	set_creases(tmp_creases) ;
	set_boundaries_sharp() ;
	compute_and_set_crease_neighbors() ;
}

void
Mesh::compute_and_set_twins()
{
	halfedge_buffer& He = this->halfedges ;

	typedef std::pair<int,int> pi ;
	typedef std::pair<int,pi> map_item ;
	typedef std::multimap<int,pi >mmap ;
	typedef std::pair <mmap::iterator, mmap::iterator> range ;

	mmap table ;

	for (int h=0 ; h < H_count ; ++h)
	{
		const int nextID = Next(h) ;
		const int vh = Vert(h) ;
		const int vn = Vert(nextID) ;

		table.insert(map_item(vh,pi(vn,h))) ;
	}

	for (int vh=0 ; vh < V_count ; ++vh)
	{
		// for all elements <vh,vn> with key vh
		range range_vh = table.equal_range(vh) ;
		for (mmap::iterator it=range_vh.first; it != range_vh.second; ++it)
		{
			assert (it->first == vh) ;
			const int vn = it->second.first ;
			const int h = it->second.second ;

			// if twin <vn,vh> exist
			range range_vn = table.equal_range(vn) ;
			for (mmap::iterator it_vn=range_vn.first; it_vn != range_vn.second; ++it_vn)
			{
				assert (it_vn->first == vn) ;
				const int vnn = it_vn->second.first;
				const int h_twin = it_vn->second.second ;
				if (vnn == vh)
				{			// assign twin ids
					He[h].Twin = h_twin ;
					He[h_twin].Twin = h ;
				}
			}
		}
	}
}

int
Mesh::compute_and_set_edges()
{
	halfedge_buffer& He = this->halfedges;

	int edge_count = 0 ;
	for (int h_id=0 ; h_id < H_count ; ++h_id)
	{
		HalfEdge& h = He[h_id] ;
		if (h.Edge < 0) // if not yet set
		{
			// if border (no twin)
			if (Twin(h_id) < 0)
			{
				h.Edge = edge_count ;
				++edge_count ;
			}
			else // twin exists
			{
				const int twin_edge = Edge(Twin(h_id)) ;
				if (twin_edge < 0)
				{
					h.Edge = edge_count++ ;
				}
				else
				{
					h.Edge = twin_edge ;
				}
			}
		}
	}

	return edge_count ;
}

void
Mesh::set_creases(const Mesh::crease_buffer& list_of_creases)
{
	crease_buffer& Cr = this->creases ;
	for (const Crease& C: list_of_creases)
	{
		const int v0 = C.Prev ;
		const int v1 = C.Next ;
		const float sharpness = C.Sharpness ;

		for (int h_id=0 ; h_id < H_count ; ++h_id)
		{
			if (Vert(h_id) == v0)
			{
				// forward
				int h ;
				for (h = Next_safe(Twin(h_id)) ;
					 (h >= 0 && Vert(Next(h)) != v1 && h != h_id);
					 h = Next_safe(Twin(h)))
				{}

				if (h < 0)
				{
					// backward
					for (h = Twin(Prev(h_id)) ;
						 (h >= 0 && Vert(Next(h)) != v1 && h != h_id);
						 h = Twin(Prev(h)))
					{}
				}

				if (h >= 0 && Vert(Next(h)) == v1)
				{
					const int e_id = Edge(h) ;

					Crease& cr = Cr[e_id] ;
					cr.Sharpness = sharpness ;
					cr.Next = e_id ;
					cr.Prev = e_id ;
				}
			}
		}
	}
}

int
Mesh::find_second_crease(int h) const
{
	int c_second = Edge(h) ;
	for (int h_vx_it = Twin(Next(h)) ;
		 h_vx_it != h ;
		 h_vx_it = Twin(Next(h_vx_it)))
	{
		c_second = Edge(h_vx_it) ;
		const int sharpness_second = creases[c_second].Sharpness ;
		if (sharpness_second > _epsilon_)
			return c_second ;
	}

	assert(false || !"Mesh::find_second_crease: preconditions not satisfied.") ;
	return -1 ;
}

void
Mesh::compute_and_set_crease_neighbors()
{
	crease_buffer& Cr = creases ;
	for (int h = 0 ; h < H_count ; ++h)
	{
		const int c = Edge(h) ;
		const float sharpness = Cr[c].Sharpness ;
		int h_vx_it ;

		// only treat creases through their biggest halfedge_id
		if (h < Twin(h) || sharpness < _epsilon_)
			continue ;

		// Compute prev
		int prev_creases = 0 ;
		int c_prev = c ;
		for (h_vx_it = Next_safe(Twin(h));
		     h_vx_it >= 0 && h_vx_it != h;
		     h_vx_it = Next_safe(Twin(h_vx_it)))
		{
			float c_prev_sharpness = Cr[Edge(h_vx_it)].Sharpness ;
			if (c_prev_sharpness > _epsilon_)
			{
				++prev_creases ;
				c_prev = Edge(h_vx_it) ;
			}
		}
		if (h_vx_it < 0) // go backwards
		{
			h_vx_it = Prev(h) ;
			while(true)
			{
				int h_vx_it_twin = Twin(h_vx_it) ;
				if (h_vx_it_twin == h)
					break ;

				int c_id = Edge(h_vx_it) ;
				float c_prev_sharpness = Cr[c_id].Sharpness ;
				if (c_prev_sharpness > _epsilon_)
				{
					++prev_creases ;
					c_prev = c_id ;
				}

				if (h_vx_it_twin < 0)
					break ;

				h_vx_it = Prev(h_vx_it_twin) ;
			}
		}
		if (prev_creases == 1)
		{
			Cr[c].Prev = c_prev ;
		}

		// Compute next
		int h_next = Next(h) ;
		int next_creases = 0 ;
		int c_next = c ;
		for (h_vx_it = h_next;
			 h_vx_it >= 0 && h_vx_it != Twin(h);
		     h_vx_it = Next_safe(Twin(h_vx_it)))
		{
			int c_id = Edge(h_vx_it) ;
			float c_next_sharpness = Cr[c_id].Sharpness ;
			if (c_next_sharpness > _epsilon_)
			{
				++next_creases ;
				c_next = c_id ;
			}
		}
		if (h_vx_it < 0) // go backwards
		{
			int h_vx_it_twin = Twin(h) ;
			if (h_vx_it_twin >= 0)
			{
				h_vx_it = Prev(h_vx_it_twin) ;
				while(true)
				{
					if (h_vx_it == h)
						break ;

					int c_id = Edge(h_vx_it) ;
					float c_next_sharpness = Cr[c_id].Sharpness ;
					if (c_next_sharpness > _epsilon_)
					{
						++next_creases ;
						c_next = c_id ;
					}

					h_vx_it_twin = Twin(h_vx_it) ;
					if (h_vx_it_twin < 0)
						break ;
					h_vx_it = Prev(h_vx_it_twin) ;
				}
			}
		}
		if (next_creases == 1)
		{
			Cr[c].Next = c_next ;
		}
	}
}

void
Mesh::set_boundaries_sharp()
{
	for (int h = 0 ; h < H_count ; ++h)
	{
		if (Twin(h) < 0)
		{
			const int e = Edge(h) ;
			Crease& c = creases[e] ;
			c.Sharpness = 16.0 ;
			c.Next = e ;
			c.Prev = e ;
		}
	}
}

