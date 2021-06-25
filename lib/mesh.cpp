#include "mesh.h"

Mesh::Mesh(int H, int V, int E, int F):
	H0(H), V0(V), E0(E), F0(F)
{
	set_depth(0) ;
	alloc_halfedge_buffer(H0) ;
	alloc_vertex_buffer(V0) ;
	alloc_crease_buffer(E0) ;
}

void
Mesh::alloc_halfedge_buffer(int H)
{
	halfedges_cage.resize(H) ;
	halfedges.resize(H) ;
}

void
Mesh::alloc_vertex_buffer(int V)
{
	vertices.resize(V,{0.,0.,0.}) ;
}

void
Mesh::alloc_crease_buffer(int E)
{
	creases.resize(E) ;
}

Mesh::Mesh(const std::string& filename)
{
	read_from_obj(filename) ;
}

const int&
Mesh::depth() const
{
	return _depth ;
}

void
Mesh::set_depth(int d)
{
	if (d == 0)
		init_depth() ;
	else
	{
		_depth = d ;

		Hd = H(d) ;
		Vd = V(d) ;
		Ed = E(d) ;
		Fd = F(d) ;
		Cd = C(d) ;
	}
}

void
Mesh::init_depth()
{
	_depth = 0 ;
	Hd = H0 ;
	Vd = V0 ;
	Ed = E0 ;
	Fd = F0 ;
	Cd = E0 ;
}

bool
Mesh::is_cage() const
{
	return depth() == 0 ;
}

int
Mesh::Twin(int idx) const
{
	return halfedges[idx].Twin ;
}

int
Mesh::Prev(int idx) const
{
	return halfedges_cage[idx].Prev ;
}

int
Mesh::Next(int idx) const
{
	return halfedges_cage[idx].Next ;
}

int
Mesh::Next_safe(int idx) const
{
	if (idx < 0)
		return idx ;

	return Next(idx) ;
}

int
Mesh::Vert(int idx) const
{
	return halfedges[idx].Vert ;
}

int 
Mesh::Edge(int idx) const
{
	return halfedges[idx].Edge ;
}

int
Mesh::Face(int idx) const
{
	return halfedges_cage[idx].Face ;
}

float
Mesh::Sigma(int idx) const
{
	return idx > Cd ? 0. : creases[idx].Sigma ;
}

int
Mesh::NextC(int idx) const
{
	return creases[idx].Next ;
}

int
Mesh::PrevC(int idx) const
{
	return creases[idx].Prev ;
}

int
Mesh::C(int depth) const
{
	const int& d = depth < 0 ? this->depth() : depth ;
	return std::pow(2,d) * E0 ;
}

int
Mesh::vertex_halfedge_valence(int h) const
{
	int n = 1 ;

	int h_back_valid = h ;
	int h_back = Twin(Prev(h)) ;

	// rewind to border (if any)
	while ((h_back != h) && (h_back >= 0))
	{
		h_back_valid = h_back ;
		h_back = Twin(Prev(h_back)) ;
	}

	// fw to border or back to start and count
	int h_fw = Next_safe(Twin(h_back_valid)) ;
	while ((h_fw != h_back_valid) && (h_fw >= 0))
	{
		n++ ;
		h_fw = Next_safe(Twin(h_fw)) ;
	}

	return n ;
}

int
Mesh::vertex_edge_valence(int h) const
{
	int n = 1 ;

	int h_back_valid = h ;
	int h_back = Twin(Prev(h)) ;
	
	// rewind to border (if any)
	while ((h_back != h) && (h_back >= 0))
	{
		h_back_valid = h_back ;
		h_back = Twin(Prev(h_back)) ;
	}

	// fw to border or back to start and count
	int h_fw = Next_safe(Twin(h_back_valid)) ;
	while ((h_fw != h_back_valid) && (h_fw >= 0))
	{
		n++ ;
		h_fw = Next_safe(Twin(h_fw)) ;
	}

	if (h_fw < 0)
		n++ ;
	
	return n ;
}

int
Mesh::vertex_edge_valence_or_border(int h) const
{
	int n = 0 ;

	int hh = h ;
	do
	{
		n++ ;
		const int hh_twin = Twin(hh) ;

		bool is_border = hh_twin < 0 ;
		if (is_border)
			return -1 ;

		hh = Next(hh_twin) ;
	}
	while(hh != h);

	return n ;
}

int
Mesh::vertex_crease_valence_or_border(int h) const
{
	int n = 0 ;

	int hh = h ;
	do
	{
		if (is_crease_halfedge(hh))
			n++ ;

		int hh_twin = Twin(hh) ;
		bool is_border = hh_twin < 0 ;
		if (is_border)
			return -1 ;

		hh = Next(hh_twin) ;
	}
	while(hh != h);

	return n ;
}

int
Mesh::vertex_crease_valence(int h) const
{
	int n = is_crease_halfedge(h) ;

	int h_it ;
	for (h_it = Twin(h) ; h_it >= 0 ; h_it = Twin(h_it))
	{
		h_it = Next(h_it) ;

		if (h_it == h)
			break ;

		if (is_crease_halfedge(h_it))
			n++ ;
	}

	if (h_it < 0)
	{	// do backward iteration too
		for (h_it = h ; h_it >= 0 ; h_it = Twin(h_it))
		{
			h_it = Prev(h_it) ;
			if (is_crease_halfedge(h_it))
				n++ ;
		}
	}

	return n ;
}

float
Mesh::vertex_sharpness_or_border(int h) const
{
	float edge_valence = 0.0 ;
	float sharpness = 0 ;

	int hh = h ;
	do
	{
		sharpness += Sigma(Edge(hh)) ;
		++edge_valence ;

		const int hh_twin = Twin(hh) ;
		const bool is_border = hh_twin < 0 ;
		if (is_border)
			return -1. ;

		hh = Next(hh_twin) ;
	}
	while(hh != h);

	return sharpness / edge_valence ;
}

float
Mesh::vertex_sharpness(int h) const
{
	float edge_valence = 1.0 ;
	float sharpness = Sigma(Edge(h)) ;

	int h_it ;
	for (h_it = Twin(h) ; h_it >= 0 ; h_it = Twin(h_it))
	{
		h_it = Next(h_it) ;
		if (h_it == h)
			break ;

		edge_valence++ ;
		sharpness += Sigma(Edge(h_it)) ;
	}

	if (h_it < 0)
	{	// do backward iteration too
		for (h_it = h ; h_it >= 0 ; h_it = Twin(h_it))
		{
			h_it = Prev(h_it) ;
			edge_valence++ ;
			sharpness += Sigma(Edge(h_it)) ;
		}
	}

	return sharpness / edge_valence ;
}

bool
Mesh::is_border_halfedge(int h) const
{
	return Twin(h) < 0 ;
}

bool
Mesh::is_border_vertex(int h) const
{
	return vertex_edge_valence_or_border(Vert(h)) < 0 ;
}

bool
Mesh::is_crease_halfedge(int h) const
{
	return Sigma(Edge(h)) > 1e-6 ;
}

bool
Mesh::is_crease_edge(int h) const
{
	return Sigma(h) > 1e-6 ;
}


int
Mesh::n_vertex_of_polygon_check(int h) const
{
	int n = 1 ;
	for (int h_fw = Next(h) ; h_fw != h ; h_fw = Next(h_fw))
	{
		++n ;
	}
	return n ;
}


int
Mesh::n_vertex_of_polygon(int h) const
{
	return n_vertex_of_polygon_check(h) ;
}

bool
Mesh::check() const
{
	bool valid = true ;
	if (Hd < 2)
	{
		std::cerr << "The mesh is empty" << std::endl ;
	}

	for (int h = 0; h < Hd ; ++h)
	{
		bool check_twin = (is_border_halfedge(h) || Twin(Twin(h)) == h) ;
		if (!check_twin)
			std::cout << h << " : Twin(h)=" << Twin(h) << " ; Twin(Twin(h))=" << Twin(Twin(h)) << std::endl ;
		assert(check_twin) ;
		valid &= check_twin ;

		bool check_prevnext = (Next(Prev(h)) == h && Prev(Next(h)) == h) ;
		if (!check_prevnext)
			std::cerr << "Assert: PrevNext" << std::endl ;
		assert(check_prevnext) ;
		valid &= check_prevnext ;

		// two twins have same edge
		bool check_twinedge = (is_border_halfedge(h) || (Edge(Twin(h)) == Edge(h))) ;
		if (!check_twinedge)
			std::cerr << "Assert: TwinEdge" << std::endl ;
		assert(check_twinedge) ;
		valid &= check_twinedge ;

		bool check_valid_ids = (Twin(h) < H() && Next(h) < H() && Prev(h) < H() && Vert(h) < V() && Edge(h) < E() && Face(h) < F()) ;
		if (!check_valid_ids)
			std::cerr << "Assert: Invalid Ids" << std::endl ;
		valid &= check_valid_ids ;

		assert(check_valid_ids) ;
	}

	return valid ;
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


bool
Mesh::all_faces_are_ngons(int n) const
{
	for (int h = 0; h < H0 ; ++h)
	{
		bool is_ngon = n_vertex_of_polygon_check(h) == n ;
		if (!is_ngon)
			return false ;
	}

	return H0 > 0 ? true : false ;
}

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
				iss >> Vx[v][i] ;
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
				cr.Sigma = sharpness ;
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
	this->H0 = h_count ;
	this->V0 = v_count ;
	this->F0 = f_count ;
	alloc_halfedge_buffer(this->H0) ;
	alloc_vertex_buffer(this->V0) ;

	 // rewind
	file.clear() ;
	file.seekg(0) ;

	// set he and vx buffers
	crease_buffer tmp_creases = read_obj_data(file) ;

	file.close() ;

	// Twins
	compute_and_set_twins_optim() ;

	// Edge
	this->E0 = compute_and_set_edges() ;

	// Creases
	alloc_crease_buffer(this->E0) ;
	set_creases(tmp_creases) ;
	set_boundaries_sharp() ;
	compute_and_set_crease_neighbors() ;

	// set depth
	set_depth(0) ;
}

void
Mesh::compute_and_set_twins_optim()
{
	halfedge_buffer& He = this->halfedges ;

	typedef std::pair<int,int> pi ;
	typedef std::pair<int,pi> map_item ;
	typedef std::multimap<int,pi >mmap ;
	typedef std::pair <mmap::iterator, mmap::iterator> range ;

	mmap table ;

	for (int h=0 ; h < H0 ; ++h)
	{
		const int nextID = Next(h) ;
		const int vh = Vert(h) ;
		const int vn = Vert(nextID) ;

		table.insert(map_item(vh,pi(vn,h))) ;
	}

	for (int vh=0 ; vh < V0 ; ++vh)
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

void
Mesh::compute_and_set_twins()
{
	halfedge_buffer& He = this->halfedges ;

	uint16_t table[V0][V0] ;

	// init table
	for (int i = 0 ; i < V0 ; ++i)
	{
		for (int j = 0 ; j < V0 ; ++j)
		{
			table[i][j] = -1 ;
		}
	}

	for (int h=0 ; h < H0 ; ++h)
	{
		const int nextID = Next(h) ;
		const int i = Vert(h) ;
		const int j = Vert(nextID) ;

		const int twinID = table[i][j] ;
		if (twinID < 0)
		{
			table[j][i] = h ; // twin of j->i known
			He[h].Twin = -1 ;
		}
		else // known
		{
			He[h].Twin = twinID ;
			He[twinID].Twin = h ;
		}

	}
}

int
Mesh::compute_and_set_edges()
{
	halfedge_buffer& He = this->halfedges;

	int edge_count = 0 ;
	for (int h_id=0 ; h_id < H0 ; ++h_id)
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
		const float sharpness = C.Sigma ;

		for (int h_id=0 ; h_id < H0 ; ++h_id)
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
					cr.Sigma = sharpness ;
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
		const int sharpness_second = creases[c_second].Sigma ;
		if (sharpness_second > 1e-6)
			return c_second ;
	}

	assert(false || !"Mesh::find_second_crease: preconditions not satisfied.") ;
	return -1 ;
}

void
Mesh::compute_and_set_crease_neighbors()
{
	crease_buffer& Cr = creases ;
	for (int h = 0 ; h < H0 ; ++h)
	{
		const int c = Edge(h) ;
		const float sharpness = Cr[c].Sigma ;
		int h_vx_it ;

		// only treat creases through their biggest halfedge_id
		if (h < Twin(h) || sharpness < 1e-6)
			continue ;

		// Compute prev
		int prev_creases = 0 ;
		int c_prev = c ;
		for (h_vx_it = Next_safe(Twin(h));
		     h_vx_it >= 0 && h_vx_it != h;
		     h_vx_it = Next_safe(Twin(h_vx_it)))
		{
			float c_prev_sharpness = Cr[Edge(h_vx_it)].Sigma ;
			if (c_prev_sharpness > 1e-06)
			{
				++prev_creases ;
				c_prev = Edge(h_vx_it) ;
			}
		}
		if (prev_creases == 1 && h == h_vx_it)
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
			float c_next_sharpness = Cr[c_id].Sigma ;
			if (c_next_sharpness > 1e-06)
			{
				++next_creases ;
				c_next = Edge(h_vx_it) ;
			}
		}
		if (next_creases == 1)
		{
			Cr[c].Next = c_next ;
		}

//		vx_crease_valence = vertex_crease_valence_or_border(h) ;
//		if (vx_crease_valence == 2)
//		{
//			int c_prev = find_second_crease(h) ;
//			assert(c_prev != c) ;
//			Cr[c].Prev = c_prev ;
//		}

//		int h_next = Next(h) ;
//		vx_crease_valence = vertex_crease_valence_or_border(h_next) ;
//		if (vx_crease_valence == 2)
//		{
//			int c_next = find_second_crease(h_next) ;
//			assert(c_next != c) ;
//			Cr[c].Next = c_next;
//		}
	}
}

void
Mesh::set_boundaries_sharp()
{
	for (int h = 0 ; h < H0 ; ++h)
	{
		if (Twin(h) < 0)
		{
			const int e = Edge(h) ;
			Crease& c = creases[e] ;
			c.Sigma = 16.0 ;
			c.Next = e ;
			c.Prev = e ;
		}
	}
}

void Mesh::export_to_obj(const std::string& filename) const
{
	std::ofstream file(filename) ;
	export_to_obj_contig_faces(file);
	file.close() ;
}

void
Mesh::export_to_obj_tri(std::ofstream& file) const
{
	//	allocate F x 3
	if (Fd >= 1024*1024)
	{
		std::cerr << "ERROR Mesh::export_to_obj_tri: mesh size too large" << std::endl ;
		return ;
	}
	int face_buffer[Fd][3] ;
	for (int h = 0 ; h < Hd ; ++h)
	{
		face_buffer[Face(h)][0] = 1 + Vert(h) ;
		face_buffer[Face(h)][1] = 1 + Vert(Next(h)) ;
		face_buffer[Face(h)][2] = 1 + Vert(Prev(h)) ;
	}

	file << "# Vertices" << std::endl ;
	for (int v = 0 ; v < Vd ; ++v)
	{
		file << "v " << vertices[v][0] << " " << vertices[v][1] << " " << vertices[v][2] << std::endl ;
	}

	file << "# Topology" << std::endl ;
	for (int f = 0 ; f < Fd ; ++f)
	{
		file << "f " << face_buffer[f][0] << " " << face_buffer[f][1] << " " << face_buffer[f][2] << std::endl ;
	}
}

void
Mesh::export_to_obj_contig_faces(std::ofstream& file) const
{
	file << "# Vertices" << std::endl ;
	for (int v = 0 ; v < Vd ; ++v)
	{
		file << "v " << vertices[v][0] << " " << vertices[v][1] << " " << vertices[v][2] << std::endl ;
	}

	file << "# Topology" ;
	int f_id_prev = -1 ;
	for (int h = 0 ; h < Hd ; ++h)
	{
		const int f_id = Face(h) ;
		if (f_id != f_id_prev)
		{
			file << std::endl << "f " ;
			f_id_prev = f_id ;
		}
		file << 1+Vert(h) << " " ;
	}
}

