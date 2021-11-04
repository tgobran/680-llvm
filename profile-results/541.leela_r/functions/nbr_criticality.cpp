// returns our lowest liberties, enemies lowest liberties
// 8 is the maximum
std::pair<int, int> FastBoard::nbr_criticality(int color, int vertex) {    
    std::tr1::array<int, 4> color_libs;

    color_libs[0] = 8;
    color_libs[1] = 8;
    color_libs[2] = 8;
    color_libs[3] = 8;
    
    for (int k = 0; k < 4; k++) {
        int ai = vertex + m_dirs[k];        
        int lc = m_libs[m_parent[ai]];     
        if (lc < color_libs[m_square[ai]]) {
            color_libs[m_square[ai]] = lc;
        }
    } 
        
    return std::make_pair(color_libs[color], color_libs[!color]);    
}