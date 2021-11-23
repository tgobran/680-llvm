/* Compute register info: lifetime, bb, and number of defs and uses.  */
static void
df_note_compute (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;
  bitmap live = BITMAP_ALLOC (&df_bitmap_obstack);
  bitmap do_not_gen = BITMAP_ALLOC (&df_bitmap_obstack);
  bitmap artificial_uses = BITMAP_ALLOC (&df_bitmap_obstack);

#ifdef REG_DEAD_DEBUGGING
  if (dump_file)
    print_rtl_with_bb (dump_file, get_insns());
#endif

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
  {
    df_note_bb_compute (bb_index, live, do_not_gen, artificial_uses);
  }

  BITMAP_FREE (live);
  BITMAP_FREE (do_not_gen);
  BITMAP_FREE (artificial_uses);
}