/* Worklist-based dataflow solver. It uses sbitmap as a worklist,
   with "n"-th bit representing the n-th block in the reverse-postorder order.
   The solver is a double-queue algorithm similar to the "double stack" solver
   from Cooper, Harvey and Kennedy, "Iterative data-flow analysis, Revisited".
   The only significant difference is that the worklist in this implementation
   is always sorted in RPO of the CFG visiting direction.  */

void
df_worklist_dataflow (struct dataflow *dataflow,
                      bitmap blocks_to_consider,
                      int *blocks_in_postorder,
                      int n_blocks)
{
  bitmap pending = BITMAP_ALLOC (&df_bitmap_obstack);
  sbitmap considered = sbitmap_alloc (last_basic_block);
  bitmap_iterator bi;
  unsigned int *bbindex_to_postorder;
  int i;
  unsigned int index;
  enum df_flow_dir dir = dataflow->problem->dir;

  gcc_assert (dir != DF_NONE);

  /* BBINDEX_TO_POSTORDER maps the bb->index to the reverse postorder.  */
  bbindex_to_postorder =
    (unsigned int *)xmalloc (last_basic_block * sizeof (unsigned int));

  /* Initialize the array to an out-of-bound value.  */
  for (i = 0; i < last_basic_block; i++)
    bbindex_to_postorder[i] = last_basic_block;

  /* Initialize the considered map.  */
  sbitmap_zero (considered);
  EXECUTE_IF_SET_IN_BITMAP (blocks_to_consider, 0, index, bi)
    {
      SET_BIT (considered, index);
    }

  /* Initialize the mapping of block index to postorder.  */
  for (i = 0; i < n_blocks; i++)
    {
      bbindex_to_postorder[blocks_in_postorder[i]] = i;
      /* Add all blocks to the worklist.  */
      bitmap_set_bit (pending, i);
    }

  /* Initialize the problem. */
  if (dataflow->problem->init_fun)
    dataflow->problem->init_fun (blocks_to_consider);

  /* Solve it.  */
  df_worklist_dataflow_doublequeue (dataflow, pending, considered,
				    blocks_in_postorder,
				    bbindex_to_postorder);

  sbitmap_free (considered);
  free (bbindex_to_postorder);
}
