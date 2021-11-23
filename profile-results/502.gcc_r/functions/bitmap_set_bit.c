/* Set a single bit in a bitmap.  Return true if the bit changed.  */

bool
bitmap_set_bit (bitmap head, int bit)
{
  bitmap_element *ptr = bitmap_find_bit (head, bit);
  unsigned word_num = bit / BITMAP_WORD_BITS % BITMAP_ELEMENT_WORDS;
  unsigned bit_num  = bit % BITMAP_WORD_BITS;
  BITMAP_WORD bit_val = ((BITMAP_WORD) 1) << bit_num;

  if (ptr == 0)
    {
      ptr = bitmap_element_allocate (head);
      ptr->indx = bit / BITMAP_ELEMENT_ALL_BITS;
      ptr->bits[word_num] = bit_val;
      bitmap_element_link (head, ptr);
      return true;
    }
  else
    {
      bool res = (ptr->bits[word_num] & bit_val) == 0;
      if (res)
	ptr->bits[word_num] |= bit_val;
      return res;
    }
}