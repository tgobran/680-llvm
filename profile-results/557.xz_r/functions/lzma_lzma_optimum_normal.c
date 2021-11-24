extern void
lzma_lzma_optimum_normal(lzma_coder */*restrict*/ pcoder, lzma_mf *restrict mf,
		uint32_t *restrict back_res, uint32_t *restrict len_res,
		uint32_t position)
{
	struct lzma_coder_s *restrict coder = pcoder;
	// If we have symbols pending, return the next pending symbol.
	if (coder->opts_end_index != coder->opts_current_index) {
		assert(mf->read_ahead > 0);
		*len_res = coder->opts[coder->opts_current_index].pos_prev
				- coder->opts_current_index;
		*back_res = coder->opts[coder->opts_current_index].back_prev;
		coder->opts_current_index = coder->opts[
				coder->opts_current_index].pos_prev;
		return;
	}

	// Update the price tables. In LZMA SDK <= 4.60 (and possibly later)
	// this was done in both initialization function and in the main loop.
	// In liblzma they were moved into this single place.
	if (mf->read_ahead == 0) {
		if (coder->match_price_count >= (1 << 7))
			fill_distances_prices(coder);

		if (coder->align_price_count >= ALIGN_TABLE_SIZE)
			fill_align_prices(coder);
	}

	// TODO: This needs quite a bit of cleaning still. But splitting
	// the original function into two pieces makes it at least a little
	// more readable, since those two parts don't share many variables.

	uint32_t len_end = helper1(coder, mf, back_res, len_res, position);
	if (len_end == UINT32_MAX)
		return;

	uint32_t reps[REP_DISTANCES];
	memcpy(reps, coder->reps, sizeof(reps));

	uint32_t cur;
	for (cur = 1; cur < len_end; ++cur) {
		assert(cur < OPTS);

		coder->longest_match_length = mf_find(
				mf, &coder->matches_count, coder->matches);

		if (coder->longest_match_length >= mf->nice_len)
			break;

		len_end = helper2(coder, reps, mf_ptr(mf) - 1, len_end,
				position + cur, cur, mf->nice_len,
				my_min(mf_avail(mf) + 1, OPTS - 1 - cur));
	}

	backward(coder, len_res, back_res, cur);
	return;
}
