/*    
    returns ko square or suicide tag
*/    
int FastBoard::update_board_fast(const int color, const int i) {                        
    assert(m_square[i] == EMPTY);    
    assert(color == WHITE || color == BLACK);
    
    /* did we play into an opponent eye? */    
    int eyeplay = (m_neighbours[i] & s_eyemask[!color]);  
       
    // because we check for single stone suicide, we know
    // its a capture, and it might be a ko capture         
    if (eyeplay) {          
        return update_board_eye(color, i);        
    }        
    
    m_square[i]  = (square_t)color;    
    m_next[i]    = i;     
    m_parent[i]  = i;
    m_libs[i]    = count_pliberties(i);       
    m_stones[i]  = 1;
    m_totalstones[color]++;                                    
        
    add_neighbour(i, color); 
        
    for (int k = 0; k < 4; k++) {
        int ai = i + m_dirs[k];
        
        if (m_square[ai] > WHITE) continue; 
        
        assert(ai >= 0 && ai <= m_maxsq);
                                       
        if (m_square[ai] == !color) {
            if (m_libs[m_parent[ai]] <= 0) {                
                m_prisoners[color] += remove_string_fast(ai);
            }
        } else if (m_square[ai] == color) {                                                    
            int ip  = m_parent[i];
            int aip = m_parent[ai];                    
            
            if (ip != aip) {
                if (m_stones[ip] >= m_stones[aip]) {                
                    merge_strings(ip, aip);                                                        
                } else {
                    merge_strings(aip, ip);                                                        
                }
            }
        }        
    }                   
    
    /* move last vertex in list to our position */    
    int lastvertex               = m_empty[--m_empty_cnt];
    m_empty_idx[lastvertex]      = m_empty_idx[i];
    m_empty[m_empty_idx[i]]      = lastvertex;                      
        
    assert(m_libs[m_parent[i]] >= 0);        

    /* check whether we still live (i.e. detect suicide) */    
    if (m_libs[m_parent[i]] == 0) {                                               
        remove_string_fast(i);                
    } 
        
    return -1;
}