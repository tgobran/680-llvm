/* Split a (typically critical) edge.  Return the new block.
   The edge must not be abnormal.

   ??? The code generally expects to be called on critical edges.
   The case of a block ending in an unconditional jump to a
   block with multiple predecessors is not handled optimally.  */

static basic_block
rtl_split_edge (edge edge_in)
{
  basic_block bb;
  rtx before;

  /* Abnormal edges cannot be split.  */
  gcc_assert (!(edge_in->flags & EDGE_ABNORMAL));

  /* We are going to place the new block in front of edge destination.
     Avoid existence of fallthru predecessors.  */
  if ((edge_in->flags & EDGE_FALLTHRU) == 0)
    {
      edge e;
      edge_iterator ei;

      FOR_EACH_EDGE (e, ei, edge_in->dest->preds)
        if (e->flags & EDGE_FALLTHRU)
          break;

      if (e)
        force_nonfallthru (e);
    }

  /* Create the basic block note.  */
  if (edge_in->dest != EXIT_BLOCK_PTR)
    before = BB_HEAD (edge_in->dest);
  else
    before = NULL_RTX;
  
  /* If this is a fall through edge to the exit block, the blocks might be
     not adjacent, and the right place is the after the source.  */
  if (edge_in->flags & EDGE_FALLTHRU && edge_in->dest == EXIT_BLOCK_PTR)
    {
      before = NEXT_INSN (BB_END (edge_in->src));
      bb = create_basic_block (before, NULL, edge_in->src);
      BB_COPY_PARTITION (bb, edge_in->src);
    }
  else
    {
      bb = create_basic_block (before, NULL, edge_in->dest->prev_bb);
      /* ??? Why not edge_in->dest->prev_bb here?  */
      BB_COPY_PARTITION (bb, edge_in->dest);
    }

  make_single_succ_edge (bb, edge_in->dest, EDGE_FALLTHRU);

  /* For non-fallthru edges, we must adjust the predecessor's
     jump instruction to target our new block.  */
  if ((edge_in->flags & EDGE_FALLTHRU) == 0)
    {
      edge redirected = redirect_edge_and_branch (edge_in, bb);
      gcc_assert (redirected);
    }
  else
    redirect_edge_succ (edge_in, bb);

  return bb;
}
  
  
