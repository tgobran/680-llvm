#ifdef _PROTO_
int cost_compare( BASKET **b1, BASKET **b2 )
#else
int cost_compare( b1, b2 )
    BASKET **b1;
    BASKER **b2;
#endif
{
  if( (*b1)->abs_cost < (*b2)->abs_cost )
    return 1;
  if( (*b1)->abs_cost > (*b2)->abs_cost )
    return -1;
  if( (*b1)->a->id > (*b2)->a->id )
    return 1;
  else
    return -1;
}