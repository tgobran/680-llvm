int FastState::play_random_move(int color) {                            
    board.m_tomove = color;

    int movecnt = 0;    
    int newcnt = 0;
    int scoredcnt = 0;
    
    if (lastmove > 0 && lastmove < board.m_maxsq) {
        if (board.get_square(lastmove) == !color) {            
            board.add_global_captures(color, moves, movecnt);
            board.save_critical_neighbours(color, lastmove, moves, movecnt);                        
            board.add_pattern_moves(color, lastmove, moves, movecnt);
            
            // remove ko captures from count              
            newcnt = movecnt;
            for (int i = 0; i < movecnt; i++) {
                if (moves[i] == komove) {
                    newcnt--;
                }
            }
        }        
    }   
    
    Matcher * matcher = Matcher::get_Matcher();   
                
    if (newcnt > 0) {                                                                     
        int cumul = 0; 
        
        for (int i = 0; i < movecnt; i++) {
            int sq = moves[i];

            // skip ko
            if (sq == komove) continue;
            
            int pattern = board.get_pattern_fast_augment(sq);
            int score = matcher->matches(color, pattern);                                    
            std::pair<int, int> nbr_crit = board.nbr_criticality(color, sq);            
            
            static const std::tr1::array<int, 9> crit_mine = {{
                1, 4, 1, 1, 1, 1, 1, 1, 1                        
            }};
            
            static const std::tr1::array<int, 9> crit_enemy = {{
                1, 15, 12, 1, 1, 1, 1, 1, 1
            }};
                        
            score *= crit_mine[nbr_crit.first];
            score *= crit_enemy[nbr_crit.second];                        

            bool nearby = false;
            for (int i = 0; i < 8; i++) {
                int ai = lastmove + board.get_extra_dir(i);
                if (ai == sq) {
                    nearby = true;
                    break;
                }
            }
            
            if (!nearby) {
                score *= 20;
            }                       
        
            if (score >= 40) {                
                cumul += score;
                scoredmoves[scoredcnt++] = std::make_pair(sq, cumul);
            }
        }

        int index = Random::get_Rng()->randint(cumul);

        for (int i = 0; i < scoredcnt; i++) {
            int point = scoredmoves[i].second;
            if (index < point) {
                return play_move_fast(scoredmoves[i].first);                    
            }
        }                                
    }
           
    // fall back global moves      
    //SPEC removed per GCP 4/4/2016 MCOwnerTable * mctab = MCOwnerTable::get_MCO(); 
    
    int loops = 4;    
    int cumul = 0;
    
    do {
        int vidx = Random::get_Rng()->randint(board.m_empty_cnt); 
        int vtx = walk_empty_list(board.m_tomove, vidx, true);

        if (vtx == FastBoard::PASS) {
            return play_move_fast(FastBoard::PASS);
        }

        int pattern = board.get_pattern_fast_augment(vtx);
        int score = matcher->matches(color, pattern);                                                

        if (!board.self_atari(color, vtx)) {            
            score *= 64;
        }                       

        cumul += score;
        scoredmoves[scoredcnt++] = std::make_pair(vtx, cumul);              

    } while (--loops > 0);        

    int index = Random::get_Rng()->randint(cumul);

    for (int i = 0; i < scoredcnt; i++) {
        int point = scoredmoves[i].second;
        if (index < point) {
            return play_move_fast(scoredmoves[i].first);                    
        }
    }  
    
    return play_move_fast(FastBoard::PASS);      
}