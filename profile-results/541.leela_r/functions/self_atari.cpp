// check whether this move is a self-atari
bool FastBoard::self_atari(int color, int vertex) {    
    assert(get_square(vertex) == FastBoard::EMPTY);
    
    // 1) count new liberties, if we add 2 or more we're safe                
    if (count_pliberties(vertex) >= 2) {        
        return false;                
    }
    
    // 2) if we kill an enemy, or connect to safety, we're good 
    // as well    
    if (kill_or_connect(color, vertex)) {        
        return false;
    }
    
    // any neighbor by itself has at most 2 liberties now,
    // and we can have at most one empty neighbor
    // 3) if we don't connect at all, we're dead
    if (count_neighbours(color, vertex) == 0) {
        return true;
    }
    
    // 4) we only add at most 1 liberty, and we removed 1, so check if 
    // the sum of friendly neighbors had 2 or less that might have 
    // become one (or less, in which case this is multi stone suicide)
    
    // list of all liberties, this never gets big             
    std::tr1::array<int, 3> nbr_libs;
    int nbr_libs_cnt = 0;
    
    // add the vertex we play in to the liberties list
    nbr_libs[nbr_libs_cnt++] = vertex;           
    
    for (int k = 0; k < 4; k++) {
        int ai = vertex + m_dirs[k];
        
        if (get_square(ai) == FastBoard::EMPTY) {
            bool found = false;
                    
            for (int i = 0; i < nbr_libs_cnt; i++) {
                if (nbr_libs[i] == ai) {
                    found = true;
                    break;
                }
            }
                                        
            // not in list yet, so add
            if (!found) {
                if (nbr_libs_cnt > 1) return false;
                nbr_libs[nbr_libs_cnt++] = ai;
            }                
        } else if (get_square(ai) == color) {        
            int par = m_parent[ai];
            int lib = m_libs[par];
            
            // we already know this neighbor does not have a large 
            // number of liberties, and to contribute, he must have
            // more liberties than just the one that is "vertex"
            if (lib > 1) {
                add_string_liberties<3>(ai, nbr_libs, nbr_libs_cnt);
                if (nbr_libs_cnt > 2) {            
                    return false;
                }
            }            
        }                
    }

    // if we get here, there are no more than 2 liberties,
    // and we just removed 1 of those (since we added the play square
    // to the list), so it must be an auto-atari    
    return true;
}