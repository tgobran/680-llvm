/*
*  Function to give pick the top move order, one at a time on each call.
*  Will return TRUE while there are still moves left, FALSE after all moves
*  have been used
*/
static int remove_one_fast(int * marker, int * move_ordering,
                           move_s *moves, const int num_moves) {
    move_s tmpmv;
    int i, best = -INF;
    int tmp, mark;

    (*marker)++;
    mark = *marker;

    if (mark < 10) {
        for (i = mark; i < num_moves; i++) {
            if (move_ordering[i] > best) {
                best = move_ordering[i];
                tmp = i;
            }
        }
    } else if (mark < num_moves) {
        best = move_ordering[mark];
        tmp = mark;

        return TRUE;
    }

    if (best > -INF) {
        /*
            swap best move to marker pos
        */
        move_ordering[tmp] = move_ordering[mark];
        move_ordering[mark] = best;

        tmpmv = moves[mark];
        moves[mark] = moves[tmp];
        moves[tmp] = tmpmv;

        return TRUE;
    } else {
        return FALSE;
    }
}