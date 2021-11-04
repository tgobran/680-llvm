#ifdef _PROTO_
void replace_weaker_arc( arc_t *newarc, node_t *tail, node_t *head,
                         cost_t cost, cost_t red_cost,LONG max_new_par_residual_new_arcs, LONG number)
#else
void replace_weaker_arc( net, newarc, tail, head, cost, red_cost, max_new_par_residual_new_arcs, number)
     network *net;
     arc_t *newarc;
     node_t *tail;
     node_t *head;
     cost_t cost;
     cost_t red_cost;
     LONG max_new_par_residual_new_arcs;
     LONG number;
#endif
{
    LONG pos;
    LONG cmp;

    newarc[0].tail     = tail;
    newarc[0].head     = head;
    newarc[0].org_cost = cost;
    newarc[0].cost     = cost;
    newarc[0].flow     = (flow_t)red_cost; 
    newarc[0].id       = number;
                    
    pos = 1;
    cmp = (newarc[1].flow > newarc[2].flow) ? 2 : 3;
    while( cmp <= max_new_par_residual_new_arcs && red_cost < newarc[cmp-1].flow )
    {
        newarc[pos-1].tail = newarc[cmp-1].tail;
        newarc[pos-1].head = newarc[cmp-1].head;
        newarc[pos-1].cost = newarc[cmp-1].cost;
        newarc[pos-1].org_cost = newarc[cmp-1].cost;
        newarc[pos-1].flow = newarc[cmp-1].flow;
        newarc[pos-1].id   = newarc[cmp-1].id;
        
        newarc[cmp-1].tail = tail;
        newarc[cmp-1].head = head;
        newarc[cmp-1].cost = cost;
        newarc[cmp-1].org_cost = cost;
        newarc[cmp-1].flow = (flow_t)red_cost; 
        newarc[cmp-1].id   = number;
        pos = cmp;
        cmp *= 2;
        if( cmp + 1 <= max_new_par_residual_new_arcs )
            if( newarc[cmp-1].flow < newarc[cmp].flow )
                cmp++;
    }

    return;
}   