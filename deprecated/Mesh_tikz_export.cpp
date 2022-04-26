
std::string
Mesh::export_to_tikz(int depth, float alpha) const
{
    if (H_count < 2)
    {
        std::cerr << "The mesh is empty" << std::endl ;
        return "" ;
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
