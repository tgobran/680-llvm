/*
* perform a quiescence search on the current node
* using alpha-beta with negamax search
*/
int qsearch(state_t *s, int alpha, int beta, int depth, int qply) {
    int num_moves, i;
    int score, standpat;
    int legal_move, no_moves;
    int delta;
    unsigned int best, sbest;
    int originalalpha;
    int bound, xdummy;
    int incheck, afterincheck;
    int pass, multipass;
    int standpatmargin;
    move_s moves[MOVE_BUFF];
    int move_ordering[MOVE_BUFF];

    s->nodes++;
    s->qnodes++;

    if (s->ply > s->maxply) {
        s->maxply = s->ply;
    }

    if (search_time_check(s)) {
        return 0;
    }

    if (is_draw(&gamestate, s) || s->fifty > 99) {
        return (gamestate.comp_color == s->white_to_move ? contempt : -contempt);
    }

    switch (ProbeTT(s, &bound, alpha, beta, &best, &xdummy, &xdummy, &xdummy, &xdummy, 0)) {
        case EXACT:
            return bound;
            break;
        case UPPER:
            if (bound <= alpha) {
                return bound;
            }
            break;
        case LOWER:
            if (bound >= beta) {
                return bound;
            }
            break;
        case DUMMY:
            break;
        case HMISS:
            best = 0xFFFF;
            break;
    }   

    if ((qply > 2*gamestate.i_depth) || s->ply > MAXDEPTH) {
        return eval(s, alpha, beta, FALSE);
    }

    incheck = s->checks[s->ply];

    originalalpha = alpha;

    standpat = retrieve_eval(s);
    standpatmargin = standpat + 50;

    if (!incheck) {
        if (standpat >= beta) {
            StoreTT(s, standpat, originalalpha, beta, best, 0, 0, 0, 0);
            return standpat;
        } else if (standpat > alpha) {
            alpha = standpat;
        } else if (standpatmargin + MAT_QUEEN <= alpha) {
            StoreTT(s, standpatmargin + MAT_QUEEN, originalalpha, beta, best, 0, 0, 0, 0);
            return standpatmargin + MAT_QUEEN;
        } else {
            int *npieces = s->npieces;
            if (s->white_to_move) {
                if (!npieces[bqueen]) {
                    if (!npieces[brook]) {
                        if (!npieces[bbishop] && !npieces[bknight]) {
                            if (standpatmargin + MAT_PAWN <= alpha) {
                                return standpatmargin + MAT_PAWN;
                            }
                        } else {
                            if (standpatmargin + MAT_BISHOP <= alpha) {
                                return standpatmargin + MAT_BISHOP;
                            }
                        }
                    } else {
                        if (standpatmargin + MAT_ROOK <= alpha) {
                            StoreTT(s, standpatmargin + MAT_ROOK, originalalpha, beta, best, 0, 0, 0, 0);
                            return standpatmargin + MAT_ROOK;
                        }
                    }
                }
            } else {
                if (!npieces[wqueen]) {
                    if (!npieces[wrook]) {
                        if (!npieces[wbishop] && !npieces[wknight]) {
                            if (standpatmargin + MAT_PAWN <= alpha) {
                                return standpatmargin + MAT_PAWN;
                            }
                        } else {
                            if (standpatmargin + MAT_BISHOP <= alpha) {
                                return standpatmargin + MAT_BISHOP;
                            }
                        }
                    } else {
                        if (standpatmargin + MAT_ROOK <= alpha) {
                            StoreTT(s, standpatmargin + MAT_ROOK, originalalpha, beta, best, 0, 0, 0, 0);
                            return standpatmargin + MAT_ROOK;
                        }
                    }
                }
            }
        }
    }

    num_moves = 0;    

    if (!incheck) {
        delta = alpha - standpatmargin;
    } else {
        delta = 0;
    }
    
    multipass = FALSE;
    pass = 1;
    score = -MATE;
    no_moves = TRUE;
    sbest = best;

    if (depth > -6) {
        if (incheck) {
            num_moves = gen_evasions(s, moves, incheck);
        } else {
            num_moves = gen_captures(s, moves);
            multipass = TRUE;
        }
    } else {
        if (!incheck) {
            num_moves = gen_captures(s, moves);
        } else {
            num_moves = gen_evasions(s, moves, incheck);
        }
    }

mpass:
    if (pass == 2) {
        num_moves = gen_good_checks(s, moves);
    } else if (pass == 3) {
        num_moves = gen_quiet(s, moves);
    }

    fast_order_moves(s, moves, move_ordering, num_moves, best);

    i = -1;

    /* loop through the moves at the current node: */
    while (remove_one_fast(&i, move_ordering, moves, num_moves)) {
        if (!incheck) {
            if (pass == 1) {
                if (abs(material[CAPTURED(moves[i])]) <= delta && !PROMOTED(moves[i])) {
                    goto endpass;
                }
            }
            if ((pass == 2 || pass == 3) && CAPTURED(moves[i]) != npiece && abs(material[CAPTURED(moves[i])]) > delta) {
                continue;
            }
            if (pass == 3) {
                if (history_pre_cut(s, moves[i], 1)) {
                    continue;
                }
            }
            if (pass == 2 || pass == 3 || abs(material[CAPTURED(moves[i])]) < abs(material[s->sboard[FROM(moves[i])]]) ) {
                if (see(s, TOMOVE(s), FROM(moves[i]), TARGET(moves[i]), FALSE) < -50) {
                    continue;
                }
            }
        }

        legal_move = FALSE;

        make(s, moves[i]);

        if (check_legal(s, moves[i])) {
            s->hash_history[gamestate.move_number + s->ply - 1] = s->hash;
            s->path[s->ply - 1] = moves[i];

            afterincheck = in_check(s);
            s->checks[s->ply] = afterincheck;
            int posteval = -eval(s, -beta, -alpha+60, !!afterincheck);

            if (pass != 3 || posteval > alpha) {
                int newdepth;

                if (pass == 3) {                    
                    newdepth = depth;
                } else if (afterincheck || incheck) {
                    newdepth = depth - 1;
                } else {
                    newdepth = depth - 8;
                }
                score = -qsearch(s, -beta, -alpha, newdepth, qply + 1);
            }

            legal_move = TRUE;
            no_moves = FALSE;
        }

        unmake(s, moves[i]);

        if (gamestate.time_exit) {
            return 0;
        }

        if (score > alpha && legal_move) {
            sbest = compact_move(moves[i]);

            if (score >= beta) {
                StoreTT(s, score, originalalpha, beta, sbest, 0, 0, 0, 0);
                return score;
            }

            alpha = score;
        }        
    }

endpass:
    if (multipass && pass == 1) {
        pass++;
        goto mpass;
    } else if (multipass && pass == 2) {
        if (depth >= 0) {
            if (standpat + 50 > alpha) {
                pass++;
                goto mpass;
            }
        }
    }

    /*
        we don't check for mate / stalemate here, because without generating all
        of the moves leading up to it, we don't know if the position could have
        been avoided by one side or not
    */
    if (no_moves && incheck) {
        alpha = -MATE + s->ply;
    }
    
    StoreTT(s, alpha, originalalpha, beta, sbest, 0, 0, 0, 0);

    return alpha;
}
