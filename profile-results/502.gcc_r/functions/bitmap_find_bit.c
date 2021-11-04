static inline bitmap_element *
bitmap_find_bit (bitmap head, unsigned int bit)
{
  bitmap_element *element;
  unsigned int indx = bit / BITMAP_ELEMENT_ALL_BITS;

#ifdef GATHER_STATISTICS
  head->desc->nsearches++;
#endif
  if (head->current == 0
      || head->indx == indx)
    return head->current;

  if (head->indx < indx)
    /* INDX is beyond head->indx.  Search from head->current
       forward.  */
    for (element = head->current;
         element->next != 0 && element->indx < indx;
         element = element->next)
      ;

  else if (head->indx / 2 < indx)
    /* INDX is less than head->indx and closer to head->indx than to
       0.  Search from head->current backward.  */
    for (element = head->current;
         element->prev != 0 && element->indx > indx;
         element = element->prev)
      ;

  else
    /* INDX is less than head->indx and closer to 0 than to
       head->indx.  Search from head->first forward.  */
    for (element = head->first;
         element->next != 0 && element->indx < indx;
         element = element->next)
      ;

  /* `element' is the nearest to the one we want.  If it's not the one we
     want, the one we want doesn't exist.  */
  head->current = element;
  head->indx = element->indx;
  if (element != 0 && element->indx != indx)
    element = 0;

  return element;
}
