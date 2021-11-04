void FastBoard::merge_strings(const int ip, const int aip) {            
    assert(ip != MAXSQ && aip != MAXSQ);

    /* merge stones */    
    m_stones[ip] += m_stones[aip];
    
    /* loop over stones, update parents */           
    int newpos = aip;    

    do {       
        // check if this stone has a liberty        
        for (int k = 0; k < 4; k++) {
            int ai = newpos + m_dirs[k];
            // for each liberty, check if it is not shared        
            if (m_square[ai] == EMPTY) {                
                // find liberty neighbors                
                bool found = false;                
                for (int kk = 0; kk < 4; kk++) {
                    int aai = ai + m_dirs[kk];
                    // friendly string shouldn't be ip
                    // ip can also be an aip that has been marked                    
                    if (m_parent[aai] == ip) {
                        found = true;
                        break;
                    }                    
                }                                
                
                if (!found) {
                    m_libs[ip]++;
                }
            }
        }
        
        m_parent[newpos] = ip;         
        newpos = m_next[newpos];
    } while (newpos != aip);                    
        
    /* merge stings */
    int tmp = m_next[aip];
    m_next[aip] = m_next[ip];
    m_next[ip] = tmp;                
}