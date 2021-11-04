bool FastBoard::kill_or_connect(int color, int vertex) {                        
    for (int k = 0; k < 4; k++) {
        int ai = vertex + m_dirs[k];        
        int sq = get_square(ai);                              
        int libs = m_libs[m_parent[ai]];
        
        if ((libs <= 1 && sq == !color) || (libs >= 3 && sq == color)) {
            return true;
        }              
    }          
    
    return false;       
}
