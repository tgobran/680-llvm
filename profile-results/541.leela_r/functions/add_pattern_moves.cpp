void FastBoard::add_pattern_moves(int color, int vertex, movelist_t & moves, int & movecnt) {                                         
    for (int i = 0; i < 8; i++) {        
        int sq = vertex + m_extradirs[i];
        
        if (m_square[sq] == EMPTY) {                  
            if (!self_atari(color, sq)) {                    
                moves[movecnt++] = sq;        
            }            
        }                                        
    }                               
    
    return;
}    