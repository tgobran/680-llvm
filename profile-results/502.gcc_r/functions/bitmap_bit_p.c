/* Return whether a bit is set within a bitmap.  */

int
bitmap_bit_p (bitmap head, int bit)
{
  bitmap_element *ptr;
  unsigned bit_num;
  unsigned word_num;

  ptr = bitmap_find_bit (head, bit);
  if (ptr == 0)
    return 0;

  bit_num = bit % BITMAP_WORD_BITS;
  word_num = bit / BITMAP_WORD_BITS % BITMAP_ELEMENT_WORDS;

  return (ptr->bits[word_num] >> bit_num) & 1;
}
