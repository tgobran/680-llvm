#ifdef _PROTO_
LONG price_out_impl( network_t *net )
#else
LONG price_out_impl( net )
     network_t *net;
#endif
{
    LONG i;
    LONG trips;
    LONG new_arcs = 0;
    LONG resized = 0;
    LONG latest;
    LONG min_impl_duration = 15;
    LONG max_new_par_residual_new_arcs;
    int thread;
    LONG *new_arcs_array;
    LONG id, list_size, *num_del_arcs;
    arc_p **arcs_pointer_sorted, *deleted_arcs;
    LONG max_redcost;
    short first_replace = 1, local_first_replace;
    LONG count = 0;
    LONG num_switch_iterations;
  LONG size_del;

    register list_elem *first_list_elem;
    register list_elem *new_list_elem;
    register list_elem* iterator;

    register cost_t bigM = net->bigM;
    register cost_t head_potential;
    register cost_t arc_cost = 30;
    register cost_t red_cost;
    register cost_t bigM_minus_min_impl_duration;
        
    register arc_t *arcout, *arcin, *arcnew, *stop, *sorted_array, *arc;
    register node_t *tail, *head;

#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
    LONG num_threads = omp_get_max_threads();
#else
    LONG num_threads = 1;
#endif


    new_arcs_array = (LONG*) malloc(num_threads * sizeof(LONG));
    num_del_arcs = (LONG*) malloc(num_threads * sizeof(LONG));
    arcs_pointer_sorted = (arc_p**) malloc(num_threads * sizeof(arc_p*));
    deleted_arcs = (arc_p*) malloc(num_threads * sizeof(arc_p));

#if defined AT_HOME
    wall_time -= Get_Time();
#endif

    
    bigM_minus_min_impl_duration = (cost_t)bigM - min_impl_duration;
    

    
    if( net->n_trips <= MAX_NB_TRIPS_FOR_SMALL_NET )
    {
      if( net->m + net->max_new_m > net->max_m 
          &&
          (net->n_trips*net->n_trips)/2 + net->m > net->max_m
          )
      {
        resized = 1;
        if( resize_prob( net ) )
          return -1;
        
        refresh_neighbour_lists( net, &getOriginalArcPosition );
      }
    }
    else
    {
      if( net->m + net->max_new_m > net->max_m 
          &&
          (net->n_trips*net->n_trips)/2 + net->m > net->max_m
          )
      {
        resized = 1;
        if( resize_prob( net ) )
          return -1;
        
        refresh_neighbour_lists( net, &getOriginalArcPosition );
      }
    }

    if (net->n_trips <= MAX_NB_TRIPS_FOR_SMALL_NET)
        num_switch_iterations = ITERATIONS_FOR_SMALL_NET;
    else
        num_switch_iterations = ITERATIONS_FOR_BIG_NET;

    sorted_array = net->sorted_arcs;
    if (!sorted_array)
      return -1;
    max_new_par_residual_new_arcs = net->max_residual_new_m / num_threads;
  first_replace = 1;
    size_del = net->max_m/num_threads;
#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
#pragma omp parallel private(local_first_replace,count,arc,max_redcost,list_size,id, thread, stop, red_cost, arcin, head_potential, iterator, head, tail, latest, new_list_elem, first_list_elem, arcout, i, arcnew, trips )
#endif
    {
      //printf("del %d\n", size_del);
      local_first_replace = 1;
      max_redcost = 0;
      count =0;

#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
  thread = omp_get_thread_num();
#else
  thread = 0;
#endif

      deleted_arcs[thread] = &sorted_array[size_del * thread];
      num_del_arcs[thread] = 0;
        new_arcs_array[thread] = 0;
      arcnew = net->stop_arcs + thread * max_new_par_residual_new_arcs;
      trips = net->n_trips;
      id = 0;
      list_size = -1;
      arcs_pointer_sorted[thread] = (arc_p*) calloc (max_new_par_residual_new_arcs, sizeof(arc_p));


      for (i = 0; i < max_new_par_residual_new_arcs; i++) {
        arcs_pointer_sorted[thread][i] = &arcnew[i];
      }

      arcout = net->arcs;
      for( i = 0; i < trips && arcout[1].ident == FIXED; i++, arcout += 3);

      first_list_elem = (list_elem *)NULL;
      for( ; i < trips; i++, arcout += 3 )
      {
        if (!first_replace) {
            calculate_max_redcost(net, &max_redcost, arcs_pointer_sorted, num_threads);
        }

        if ( i % num_switch_iterations == 0) {
#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
#pragma omp barrier
#endif
            calculate_max_redcost(net, &max_redcost, arcs_pointer_sorted, num_threads);
            if (!first_replace) {
              //printf("thread %d count %ld del_size %ld\n", thread, count, size_del);
              num_del_arcs[thread] = count;
              switch_arcs(net, num_del_arcs, deleted_arcs, arcnew, thread, max_new_par_residual_new_arcs, size_del, num_threads);
              count = 0;
              num_del_arcs[thread] = 0;
          }
        }

        if( arcout[1].ident != FIXED )
        {
          new_list_elem = (list_elem*) calloc(1, sizeof(list_elem));
          new_list_elem->next = first_list_elem;
          new_list_elem->arc = arcout + 1;
          first_list_elem = new_list_elem;
          list_size++;
        }

        if( arcout->ident == FIXED || i % num_threads != thread)
        {
          id += list_size;
          continue;
        }

        head = arcout->head;
        latest = head->time - arcout->org_cost
            + (LONG)bigM_minus_min_impl_duration;

        head_potential = head->potential;

        iterator = first_list_elem->next;
        while( iterator )
        {

          arcin = iterator->arc;
          tail = arcin->tail;

          if( tail->time + arcin->org_cost > latest )
          {
            iterator = iterator->next;
            id++;
            continue;
          }

          red_cost = arc_cost - tail->potential + head->potential;

          if( red_cost < 0 )
          {

            if( new_arcs_array[thread] < max_new_par_residual_new_arcs)
            {
              insert_new_arc( arcnew, new_arcs_array[thread], tail, head,
                  arc_cost, red_cost, net->m, id);
              new_arcs_array[thread]++;
            }
            else if( (cost_t)arcnew[0].flow > red_cost ) {
              if (local_first_replace) {
                first_replace = 0;
                local_first_replace = 0;
              }

              deleted_arcs[thread][num_del_arcs[thread]] = arcnew[0];
              num_del_arcs[thread]++;
              count++;
              replace_weaker_arc( arcnew, tail, head, arc_cost, red_cost, max_new_par_residual_new_arcs, id);
            }
            else if (red_cost < max_redcost ) {
              arc = &deleted_arcs[thread][num_del_arcs[thread]++];
              arc->tail     = tail;
              arc->head     = head;
              arc->org_cost = arc_cost;
              arc->cost     = arc_cost;
              arc->flow     = (flow_t)red_cost;
              arc->id       = id;
              count++;
            }
            if (num_del_arcs[thread] == size_del)
            {
              num_del_arcs[thread] = 0;
            }
          }

          iterator = iterator->next;
          id++;
        }

      }

      num_del_arcs[thread] = count;
      while (!first_replace) {
#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
#pragma omp barrier
#endif
        first_replace = 1;
        //printf("Schleife vorher thread %d count %ld del_size %ld\n", thread, count, size_del);
          count = switch_arcs(net, num_del_arcs, deleted_arcs, arcnew, thread, max_new_par_residual_new_arcs, size_del, num_threads);
          //printf("Schleife nachher thread %d count %ld del_size %ld\n", thread, count, size_del);
          if (count)
            first_replace = 0;
#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
#pragma omp barrier
#endif
      }

      while (first_list_elem->next) {
        new_list_elem = first_list_elem;
        first_list_elem = first_list_elem->next;
        free(new_list_elem);
      }
      free(first_list_elem);

#if defined(SPEC)
        spec_qsort(arcs_pointer_sorted[thread], new_arcs_array[thread], sizeof(arc_p),
                (int (*)(const void *, const void *))arc_compare);
#else
        qsort(arcs_pointer_sorted[thread], new_arcs_array[thread], sizeof(arc_p),
                (int (*)(const void *, const void *))arc_compare);
#endif

#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
#pragma omp barrier
#pragma omp master
#endif
      {
          marc_arcs(net, &new_arcs, new_arcs_array, arcs_pointer_sorted);
      }

#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
#pragma omp barrier
#endif
  free(arcs_pointer_sorted[thread]);
      if( new_arcs_array[thread] )
      {
        arcnew = net->stop_arcs + thread * max_new_par_residual_new_arcs;
        stop = arcnew + new_arcs_array[thread];
        if( resized )
        {
          for( ; arcnew != stop; arcnew++ )
          {
            if (arcnew->flow == 1) {
                arcnew->flow = (flow_t)0;
                arcnew->ident = AT_LOWER;
#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
#pragma omp critical
#endif
	        sorted_array[getArcPosition(net, arcnew->id)] = *arcnew;
            }
          }
        }
        else
        {
          for( ; arcnew != stop; arcnew++ )
          {
            if (arcnew->flow == 1) {
              arcnew->flow = (flow_t)0;
              arcnew->ident = AT_LOWER;
              arcnew->nextout = arcnew->tail->firstout;
              arcnew->tail->firstout = arcnew;
              arcnew->nextin = arcnew->head->firstin;
              arcnew->head->firstin = arcnew;
#if (defined(_OPENMP) || defined(SPEC_OPENMP)) && !defined(SPEC_SUPPRESS_OPENMP) && !defined(SPEC_AUTO_SUPPRESS_OPENMP)
#pragma omp critical
#endif
              sorted_array[getArcPosition(net, arcnew->id)] = *arcnew;
            }
          }
        }
      }
    }

        net->m_impl += new_arcs;
        net->max_residual_new_m -= new_arcs;
        refreshPositions(net, &getArcPosition, net->m);
        net->m = net->m + new_arcs;
        net->stop_arcs = net->arcs + net->m;

#ifdef DEBUG
       arc_t* arc = net->arcs;
       for (i=0;arc < net->stop_arcs; arc++, i++)
         if (!arc->head) {
           printf("arc %ld is null\n", i);
         }
#endif
    

#if defined AT_HOME
    wall_time += Get_Time();
    printf( "total time price_out_impl(): %0.0f\n", wall_time );
#endif

    free(new_arcs_array);
    free(num_del_arcs);
    free(arcs_pointer_sorted);
    free(deleted_arcs);

    return new_arcs;
}   