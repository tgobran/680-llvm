int ProbeTT(state_t *s,
            int *score, int alpha, int beta, 
            unsigned int *best, int *threat, int *donull, int *singular,
            int *nosingular, const int depth) {
    int type;
    int i;    
    BITBOARD nhash;
    unsigned int index;
    ttentry_t *temp;
    ttbucket_t *entry;

    *donull = TRUE;

    s->TTProbes++;

    if (!s->white_to_move) {
        nhash = s->hash + 1;
    } else {
        nhash = s->hash;
    }

    index = (unsigned int)nhash;
    temp = &(TTable[index % (TTSize)]);

    nhash >>= 32;

    for (i = 0; i < BUCKETS; i++) {
        if (temp->buckets[i].hash == nhash) {
            s->TTHits++;

            entry = &(temp->buckets[i]);                        
            
            if (entry->age != TTAge) {
                entry->age = TTAge;
            }

            if (entry->type == UPPER 
                && depth - 4 * PLY <= entry->depth 
                && entry->bound < beta) {
                *donull = FALSE;
            }                        

            if (entry->depth >= depth) {
                *score = entry->bound;

                if (*score > (+MATE - 500)) {
                    *score -= (s->ply - 1);
                } else if (*score < (-MATE + 500)) {
                    *score += (s->ply - 1);
                }

                *best = entry->bestmove;
                *threat = entry->threat;
                *singular = entry->singular;
                *nosingular = entry->nosingular;
                type = entry->type;
                                   
                return type;
            } else {
                *best = entry->bestmove;
                *threat = entry->threat;
                *singular = entry->singular;
                *nosingular = entry->nosingular;

                if (entry->type == UPPER) {
                    *score = -INF;
                } else if (entry->type == LOWER) {
                    *score = +INF;
                } else {
                    *score = entry->bound;
                }
                
                entry->depth = depth;
                entry->type = DUMMY;                

                return DUMMY;
            }
        } 
    }      
         
    return HMISS;
}