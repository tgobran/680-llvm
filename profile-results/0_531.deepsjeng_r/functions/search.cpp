/*
*    search the current node using alpha-beta with negamax search
*/
int search(state_t *s, int alpha, int beta, int depth, int is_null, int cutnode) {
    move_s moves[MOVE_BUFF];
    int move_ordering[MOVE_BUFF];
    int num_moves, i, j;
    int score;
    int no_moves, legal_move;
    int bound, threat, donull, best_score, old_ep;
    unsigned int best;
    int incheck, first;
    int extend;
    int originalalpha;
    int mateprune;
    int afterincheck;
    int legalmoves;
    int reduc;
    int remoneflag;
    int mn;
    int singular;
    int huber;
    int nosingular;
    int wpcs, bpcs;
    int fullext;
    move_s searched_moves[MOVE_BUFF];

    if (depth <= 0 || s->ply >= MAXDEPTH) {
        return qsearch(s, alpha, beta, 0, 0);
    }

    s->nodes++;

    if (search_time_check(s)) {
        return 0;
    }

    if (is_draw(&gamestate, s) || s->fifty > 99) {
        return (gamestate.comp_color == s->white_to_move ? contempt : -contempt);
    }

    /*
        mate pruning
        if being mated here would be good enough...
    */
    mateprune = -MATE + s->ply;
    if (mateprune > alpha) {
        alpha = mateprune;
        if (mateprune >= beta) {
            return mateprune;
        }
    }

    /*
        mate pruning
        if mating the opponent isn't good enough..
    */
    mateprune = MATE - s->ply - 1;
    if (mateprune < beta) {
        beta = mateprune;
        if (mateprune <= alpha) {
            return mateprune;
        }
    }

    switch (ProbeTT(s,&bound,alpha,beta,&best,&threat,&donull,&singular,&nosingular,depth)) {
        case EXACT:            
            return bound;
            break;
        case UPPER:
            if (bound <= alpha) {                
                return bound;
            }
            cutnode = FALSE;
            break;
        case LOWER:
            if (bound >= beta) {                
                return bound;
            }
            cutnode = TRUE;
            break;
        case DUMMY:
            if (bound >= beta) {
                cutnode = TRUE;
            }
            break;
        case HMISS:
            best = 0xFFFF;
            threat = FALSE;
            singular = FALSE;
            nosingular = FALSE;
            break;        
    }
    
    originalalpha = alpha;
    incheck = s->checks[s->ply];
    int escore = retrieve_eval(s);

     if (!incheck && beta == alpha + 1) {
        if (depth <= PLY) {
            if (escore - 75 >= beta) {
                StoreTT(s, escore - 75,originalalpha,beta,best,threat,singular,nosingular,depth);
                return escore;
            }
            if (escore < beta) {
                return qsearch(s, alpha, beta, 0, 0);
            }
        } else if (depth <= 2*PLY) {
            if (escore - 125 >= beta) {
                StoreTT(s, escore - 125,originalalpha,beta,best,threat,singular,nosingular,depth);
                return escore;
            }   
        }
    }
    
    best_score = -MATE;
    score     = -MATE;

    wpcs = s->npieces[wqueen] + s->npieces[wrook] + s->npieces[wbishop] + s->npieces[wknight];
    bpcs = s->npieces[bqueen] + s->npieces[brook] + s->npieces[bbishop] + s->npieces[bknight];

    threat = 0;

    if ((is_null == NONE)
        && (s->white_to_move ? wpcs : bpcs)
        && !incheck && donull
        && beta == alpha + 1
        && (threat == FALSE)
        && depth > PLY) {

        /*
        * zugzwang detector
        */
        if (gamestate.phase == ENDGAME) {
            int newdepth = depth - 6*PLY;
            if (newdepth <= 0) {
                score = qsearch(s, beta - 1, beta, 0, 0);
            } else {
                score = search(s, beta - 1, beta, newdepth, SINGLE, cutnode);
            }
            if (gamestate.time_exit) {
                return 0;
            }
        }

        if (gamestate.phase != ENDGAME || score >= beta) {
            old_ep = s->ep_square;
            s->ep_square = 0;
            s->white_to_move ^= 1;
            s->ply++;
            s->fifty++;

            s->path[s->ply - 1] = 0;
            s->checks[s->ply] = 0;
            s->plyeval[s->ply] = s->plyeval[s->ply - 1];

            /*
                R = 3
            */
            reduc = 4 * PLY;
            int newdepth = depth - reduc;

            if (newdepth <= 0) {
                score = -qsearch(s, -beta, -beta + 1, 0, 0);
            } else {
                score = -search(s, -beta, -beta + 1, newdepth, SINGLE, !cutnode);
            }

            s->fifty--;
            s->ply--;
            s->white_to_move ^= 1;
            s->ep_square = old_ep;

            if (gamestate.time_exit) {
                return 0;
            }

            if (score >= beta) {
                StoreTT(s, score, originalalpha, beta, best, threat, FALSE, nosingular, depth);
                return score;
            } else if (score < -MATE + 600) {
                threat = TRUE;
            }
        }
    } else if (beta == alpha + 1 && depth <= 3*PLY
        && escore < beta - 300) {
        int rscore = qsearch(s, alpha, beta, 0, 0);
        if (gamestate.time_exit) {
            return 0;
        }
        // can be < beta, store result score
        if (rscore <= alpha) {
            StoreTT(s,alpha,originalalpha,beta,best,threat,singular,nosingular,depth);
            return alpha;
        }
    }

    extend = 0;

    num_moves = 0;
    no_moves = TRUE;
    legalmoves = 0;

    if (incheck) {
        num_moves = gen_evasions(s, moves, incheck);
    } else {
        num_moves = gen(s, moves);
    }

    if (incheck) {
        if (num_moves) {
            for (i = 0; (i < num_moves) && (legalmoves < 2); i++) {
                make(s, moves[i]);

                if (check_legal(s, moves[i])) {
                    legalmoves++;
                }

                unmake(s, moves[i]);
            }
        }
    } else {
        legalmoves = num_moves;
    }

    order_moves(s, moves, move_ordering, num_moves, best);

    if (depth >= 5 * PLY && (beta != alpha + 1) && (best == 0xFFFF)) {
        int goodmove = FALSE;

        for (i = 0; i < num_moves; i++) {
            if (CAPTURED(moves[i]) != npiece
                && (abs(CAPTURED(moves[i])) > abs(material[s->sboard[FROM(moves[i])]]))) {
                goodmove = TRUE;
            }
        }

        if (!goodmove) {
            int tmp;
            unsigned int mv;

            score = search(s, alpha, beta, depth >> 1, NONE, cutnode);

            if (ProbeTT(s, &tmp, 0, 0, &mv ,&tmp, &tmp, &tmp, &tmp, 0) != HMISS) {
                order_moves(s, moves, move_ordering, num_moves, mv);
            } else {
                order_moves(s, moves, move_ordering, num_moves, best);
            }
        }
    }

    score = -MATE;

    /*
    *   check for multi-cut
    */
    if (!incheck
        && !threat
        && depth >= 4 * PLY
        && legalmoves > 8
        /*&& cutnode*/
        && !s->checks[s->ply - 1]
        && (s->ply < 3 || (!s->checks[s->ply - 2]))
        && (s->ply < 4 || (!s->checks[s->ply - 3]))) {

        int m_s, m_c;

        m_s = 0;
        m_c = 0;
        i = -1;

        remoneflag = remove_one_fast(&i, move_ordering, moves, num_moves);

        while (remoneflag && m_s < 3) {
            m_s++;

            make(s, moves[i]);

            legal_move = FALSE;

            if (check_legal(s, moves[i])) {
                s->hash_history[gamestate.move_number + s->ply - 1] = s->hash;
                s->path[s->ply - 1] = moves[i];

                legal_move = TRUE;

                afterincheck = in_check(s);
                s->checks[s->ply] = afterincheck;

                int newdepth = depth - 4*PLY;

                eval(s, -beta, -alpha+50, newdepth > 0 || afterincheck);

                if (newdepth <= 0) {
                    score = -qsearch(s, -beta,-beta+1, 0, 0);
                } else {
                    score = -search(s, -beta,-beta + 1, newdepth, NONE, !cutnode);
                }
            }

            unmake(s, moves[i]);

            if (!gamestate.time_exit) {
                if (score >= beta && legal_move) {
                    m_c++;

                    if (m_c >= 2) {
                        StoreTT(s, beta, originalalpha, beta, best, threat, FALSE, nosingular, depth);
                        return beta;
                    }
                }
            } else {
                break;
            }

            remoneflag = remove_one_fast(&i,move_ordering,moves,num_moves);
        }
    }

    first = TRUE;

    int s_c;

    /*
    *   check for singularity
    */
    if (!singular
        && !nosingular
        && !threat
        && depth >= 5 * PLY
        && legalmoves > 1
        && (gamestate.phase != ENDGAME)) {

        int prescore = search(s, alpha, beta, depth - 6*PLY, NONE, cutnode);

        if (prescore > alpha) {
            i = -1;

            remoneflag = remove_one_fast(&i,move_ordering,moves,num_moves);

            s_c = 0;

            while (remoneflag && singular <= 1 && s_c < 3) {
                make(s, moves[i]);

                legal_move = FALSE;

                if (check_legal(s, moves[i])) {
                    s->hash_history[gamestate.move_number + s->ply - 1] = s->hash;
                    s->path[s->ply - 1] = moves[i];

                    s_c++;

                    legal_move = TRUE;

                    afterincheck = in_check(s);
                    s->checks[s->ply] = afterincheck;
                    int newdepth = depth - 4 * PLY;
                    eval(s, -beta, -alpha+50, newdepth > 0 || afterincheck);

                    if (first) {
                        score = -search(s, -(alpha + 1),-(alpha), newdepth, NONE, !cutnode);
                        if (score > alpha) {
                            singular = TRUE;
                        } else {
                            singular = FALSE;
                            s_c += 10;
                        }
                    } else {
                        score = -search(s, -(alpha - 49),-(alpha - 50), newdepth, NONE, FALSE);
                        if (score > alpha - 50) {
                            singular = FALSE;
                            s_c += 10;
                        }
                    }

                    first = FALSE;
                }

                unmake(s, moves[i]);
                remoneflag = remove_one_fast(&i,move_ordering,moves,num_moves);
            }
        }
    }

    fullext = (beta != alpha + 1) && !(s->ply > 2*gamestate.i_depth);

    
    first = TRUE;

    mn = 1;
    i = -1;

    remoneflag = remove_one_fast(&i, move_ordering, moves, num_moves);

    /* loop through the moves at the current node: */
    while (remoneflag) {

        extend = 0;

        if (s->ply < MAXDEPTH) {
            if (incheck && legalmoves == 1) {                    
                extend += PLY;                    
            }

            if (PIECET(s, FROM(moves[i])) == pawn
                && (Rank(TARGET(moves[i])) == Rank7
                    || Rank(TARGET(moves[i])) == Rank2
                    || PROMOTED(moves[i]))) {
                if (fullext) {
                    extend += PLY/2;
                } else {
                    extend += 0;
                }
            }               

            if (CAPTURED(moves[i]) != npiece
                && CAPTURED(s->path[s->ply - 1]) != npiece
                && rc_index[CAPTURED(moves[i])] == rc_index[CAPTURED(s->path[s->ply - 1])]
                && TARGET(moves[i]) == TARGET(s->path[s->ply - 1])) {
                int capsee = see(s, TOMOVE(s), FROM(moves[i]), TARGET(moves[i]), PROMOTED(moves[i]));
                if (capsee > 0) {
                    if (fullext) {
                        extend += 3;
                    } else {
                        extend += 1;
                    }
                }
            }

            if (singular == 1 && extend && first) {
                /* wasted work as it gets already extended */
                nosingular = TRUE;
            } else if (!extend && (singular == 1) && (first)) {
                nosingular = FALSE;
                if (fullext) {
                    extend += 3;
                } else {
                    extend += 1;
                }
            }

            if (extend > PLY) {
                extend = PLY;
            }

            /* capture into pawn endgame */
            if (CAPTURED(moves[i]) != npiece
                && CAPTURED(moves[i]) != wpawn
                && CAPTURED(moves[i]) != bpawn) {
                if ((wpcs + bpcs) == 1) {
                    extend += PLY;
                }
            }
        }

        if (CAPTURED(moves[i]) == npiece
            && (mn > 1 + (depth/PLY))
            && history_pre_cut(s, moves[i], 1 + (depth/PLY))
            && depth <= 6 * PLY
            && !extend
            && (beta == alpha + 1)
            && !PROMOTED(moves[i])) {
            remoneflag = remove_one_fast(&i, move_ordering, moves, num_moves);
            no_moves = FALSE;
            continue;
        }
        
        int afutprun = FALSE;
        int pfutprun = FALSE;

        if (depth <= 2 * PLY) {
            if (escore + 75 < alpha) {
                afutprun = TRUE;
            }
            if (escore + 200 < alpha) {
                pfutprun = TRUE;
            }
        } else if (depth <= 3 *PLY) {
            if (escore + 100 < alpha) {
                afutprun = TRUE;
            }
            if (escore + 300 < alpha) {
                pfutprun = TRUE;
            }
        } 

        int capval;
        if (CAPTURED(moves[i]) != npiece) {
            capval = see(s, TOMOVE(s), FROM(moves[i]), TARGET(moves[i]), PROMOTED(moves[i]));
        } else {
            capval = -INF;
        }

        make(s, moves[i]);

        legal_move = FALSE;

        if (check_legal(s, moves[i])) {
            legal_move = TRUE;
            no_moves = FALSE;

            afterincheck = in_check(s);

            if (afterincheck) {
                if (fullext) {
                    extend += PLY;
                } else {
                    extend += PLY/2;
                }
            }

            if (!incheck && !afterincheck && beta == alpha+1) {
                if (pfutprun) {
                    if (capval <= MAT_PAWN && !PROMOTED(moves[i])) {
                        unmake(s, moves[i]);
                        remoneflag = remove_one_fast(&i,move_ordering,moves,num_moves);
                        best_score = alpha;
                        continue;
                    }
                }
                if (afutprun) {
                    if (capval < -50 && !PROMOTED(moves[i])) {
                        unmake(s, moves[i]);
                        remoneflag = remove_one_fast(&i,move_ordering,moves,num_moves);
                        best_score = alpha;
                        continue;
                    }
                }
            }

            int newdepth = depth + extend - PLY;
            eval(s, -beta, -alpha+130, newdepth > 0 || afterincheck);

            s->checks[s->ply] = afterincheck;
            s->hash_history[gamestate.move_number + s->ply - 1] = s->hash;
            s->path[s->ply - 1] = moves[i];

            huber = 0;

            /* reductions */
            if (depth > PLY
                && mn > 3
                && beta == alpha + 1
                && !extend && !afterincheck
                && capval < -50
                && history_score(s, moves[i]) < 80
                && !PROMOTED(moves[i])
                ) {
                extend -= PLY;
                huber += PLY;
                newdepth = depth + extend - PLY;
            }

            if (first == TRUE) {
                if (newdepth <= 0) {
                    score = -qsearch(s,-beta,-alpha,0,0);
                } else {
                    score = -search(s,-beta,-alpha,newdepth,NONE, !cutnode);
                }
            } else {
                if (newdepth <= 0) {
                    score = -qsearch(s,-alpha-1,-alpha,0,0);
                } else {
                    score = -search(s,-alpha-1,-alpha,newdepth,NONE, TRUE);
                }
                if (score > best_score
                    && !gamestate.time_exit) {
                    if (score > alpha) {
                        if (huber) {
                            extend += huber;
                        }
                        if ((score < beta) || huber) {
                            newdepth = depth + extend - PLY;
                            if (newdepth <= 0) {
                                score = -qsearch(s,-beta,-alpha,0,0);
                            } else {
                                score = -search(s,-beta,-alpha,newdepth,NONE, (huber ? TRUE : FALSE));
                            }
                        }
                    }
                }
            }
            
            if (score > best_score) {
                best_score = score;
            }                
        }

        unmake(s, moves[i]);

        if (gamestate.time_exit) {
            return 0;
        }

        if (legal_move) {
            if (score > alpha) {                    
                if (score >= beta) {                        
                    history_good(s, moves[i], depth);                        

                    for (j = 0; j < mn - 1; j++) {                    
                        history_bad(s, searched_moves[j], depth);                            
                    }
                    StoreTT(s,score,originalalpha,beta,compact_move(moves[i]),threat,singular,
                            nosingular,depth);
                    return score;
                }

                alpha = score;
                best = compact_move(moves[i]);
            }

            first = FALSE;
            searched_moves[mn - 1] = moves[i];
            mn++;
        }

        remoneflag = remove_one_fast(&i,move_ordering,moves,num_moves);
    }    

    int validresult = !gamestate.time_exit;

    /* check for mate / stalemate: */
    if (no_moves && validresult) {
        if (in_check(s)) {
            StoreTT(s,-MATE + s->ply,originalalpha,beta,0,threat,singular,nosingular,depth);
            return (-MATE + s->ply);
        } else {
            StoreTT(s,0,originalalpha,beta,0,threat,singular,nosingular,depth);
            return 0;
        }
    } else {
        if (s->fifty >= 99) {
            return 0;
        }
    }    

    if (validresult) {
        StoreTT(s,best_score,originalalpha,beta,best,threat,singular,nosingular,depth);
    }

    return best_score;
}