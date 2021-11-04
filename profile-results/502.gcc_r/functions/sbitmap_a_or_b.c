void
bitmap_a_or_b (sbitmap dst, const_sbitmap a, const_sbitmap b)
{
  unsigned int i, n = dst->size;
  sbitmap_ptr dstp = dst->elms;
  const_sbitmap_ptr ap = a->elms;
  const_sbitmap_ptr bp = b->elms;
  bool has_popcount = dst->popcount != NULL;
  unsigned char *popcountp = dst->popcount;

  for (i = 0; i < n; i++)
    {
      const SBITMAP_ELT_TYPE tmp = *ap++ | *bp++;
      if (has_popcount)
        {
          bool wordchanged = (*dstp ^ tmp) != 0;
          if (wordchanged)
            *popcountp = do_popcount (tmp);
          popcountp++;
        }
      *dstp++ = tmp;
    }
#ifdef BITMAP_DEBUGGING
  if (has_popcount)
    sbitmap_verify_popcount (dst);
#endif
}
