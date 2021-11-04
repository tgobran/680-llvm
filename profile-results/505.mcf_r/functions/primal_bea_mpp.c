#ifdef _PROTO_
BASKET *primal_bea_mpp( LONG m,  arc_t *arcs, arc_t *stop_arcs,
                          LONG* basket_sizes, BASKET** perm, int thread, arc_t** end_arc, LONG step, LONG num_threads, LONG max_elems)
#else
arc_t *primal_bea_mpp( m, arcs, stop_arcs, basket_sizes, perm, thread, end_arc, step, num_threads, max_elems )
LONG m;
arc_t *arcs;
arc_t *stop_arcs;
LONG *basket_sizes;
BASKET** perm;
int thread;
arc_t** end_arc;
LONG step;
LONG num_threads;
LONG max_elems;
#endif
{
    LONG i, j, count,  global_basket_size, next;
    arc_t *arc, *old_end_arc;
    cost_t red_cost;

       for( i = 1, next = 0; i <= B && i <= basket_sizes[thread]; i++ )
       {
           arc = perm[i]->a;
           count = perm[i]->number;
           red_cost = arc->cost - arc->tail->potential + arc->head->potential;
           if( count > 0 && ((red_cost < 0 && arc->ident == AT_LOWER)
               || (red_cost > 0 && arc->ident == AT_UPPER)) )
           {
               next++;
               perm[next]->a = arc;
               perm[next]->cost = red_cost;
               perm[next]->abs_cost = ABS(red_cost);
               perm[next]->number = 0;
           }
        }

        basket_sizes[thread] = next;

        old_end_arc = *end_arc;

    NEXT:


    arc = *end_arc + step;

    if (*end_arc >= full_group_end_arc)
      *end_arc = *end_arc + max_elems - 1;
    else
      *end_arc = *end_arc + max_elems;


       for ( ; arc < *end_arc; arc += num_threads) {
      if( arc->ident > BASIC)
      {
        /* red_cost = bea_compute_red_cost( arc ); */
        red_cost = arc->cost - arc->tail->potential + arc->head->potential;
        if( bea_is_dual_infeasible( arc, red_cost ) )
        {
          basket_sizes[thread]++;
          perm[basket_sizes[thread]]->a = arc;
          perm[basket_sizes[thread]]->cost = red_cost;
          perm[basket_sizes[thread]]->abs_cost = ABS(red_cost);
          perm[basket_sizes[thread]]->number = 0;
        }
      }
       }

       if( *end_arc >= stop_arcs ) {
           *end_arc = arcs;
       }


    if (*end_arc != old_end_arc) {

#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
#pragma omp barrier
#endif
      global_basket_size = 0;
      for (j = 0; j < num_threads; j++) {
        global_basket_size+=basket_sizes[j];
      }
      if ( global_basket_size >= B) {
        goto READY;
      }
#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
#pragma omp barrier
#endif
        goto NEXT;
    }

   READY:

   perm[basket_sizes[thread] + 1]->number = -1;

    if (basket_sizes[thread] == 0) {
      return NULL;
    }

#if defined(SPEC)
    spec_qsort(perm + 1, basket_sizes[thread], sizeof(BASKET*),
            (int (*)(const void *, const void *))cost_compare);
#else
    qsort(perm + 1, basket_sizes[thread], sizeof(BASKET*),
            (int (*)(const void *, const void *))cost_compare);
#endif

    return perm[1];

}