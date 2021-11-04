static inline uint32_t
helper2(lzma_coder *pcoder, uint32_t *reps, const uint8_t *buf,
		uint32_t len_end, uint32_t position, const uint32_t cur,
		const uint32_t nice_len, const uint32_t buf_avail_full)
{
	struct lzma_coder_s * coder = pcoder;
	uint32_t matches_count = coder->matches_count;
	uint32_t new_len = coder->longest_match_length;
	uint32_t pos_prev = coder->opts[cur].pos_prev;
	lzma_lzma_state state;

	if (coder->opts[cur].prev_1_is_literal) {
		--pos_prev;

		if (coder->opts[cur].prev_2) {
			state = coder->opts[coder->opts[cur].pos_prev_2].state;

			if (coder->opts[cur].back_prev_2 < REP_DISTANCES)
				update_long_rep(state);
			else
				update_match(state);

		} else {
			state = coder->opts[pos_prev].state;
		}

		update_literal(state);

	} else {
		state = coder->opts[pos_prev].state;
	}

	if (pos_prev == cur - 1) {
		if (is_short_rep(coder->opts[cur]))
			update_short_rep(state);
		else
			update_literal(state);
	} else {
		uint32_t pos;
		if (coder->opts[cur].prev_1_is_literal
				&& coder->opts[cur].prev_2) {
			pos_prev = coder->opts[cur].pos_prev_2;
			pos = coder->opts[cur].back_prev_2;
			update_long_rep(state);
		} else {
			pos = coder->opts[cur].back_prev;
			if (pos < REP_DISTANCES)
				update_long_rep(state);
			else
				update_match(state);
		}

		if (pos < REP_DISTANCES) {
			reps[0] = coder->opts[pos_prev].backs[pos];

			uint32_t i;
			for (i = 1; i <= pos; ++i)
				reps[i] = coder->opts[pos_prev].backs[i - 1];

			for (; i < REP_DISTANCES; ++i)
				reps[i] = coder->opts[pos_prev].backs[i];

		} else {
			reps[0] = pos - REP_DISTANCES;

			for (uint32_t i = 1; i < REP_DISTANCES; ++i)
				reps[i] = coder->opts[pos_prev].backs[i - 1];
		}
	}

	coder->opts[cur].state = state;

	for (uint32_t i = 0; i < REP_DISTANCES; ++i)
		coder->opts[cur].backs[i] = reps[i];

	const uint32_t cur_price = coder->opts[cur].price;

	const uint8_t current_byte = *buf;
	const uint8_t match_byte = *(buf - reps[0] - 1);

	const uint32_t pos_state = position & coder->pos_mask;

	const uint32_t cur_and_1_price = cur_price
			+ rc_bit_0_price(coder->is_match[state][pos_state])
			+ get_literal_price(coder, position, buf[-1],
			!is_literal_state(state), match_byte, current_byte);

	bool next_is_literal = false;

	if (cur_and_1_price < coder->opts[cur + 1].price) {
		coder->opts[cur + 1].price = cur_and_1_price;
		coder->opts[cur + 1].pos_prev = cur;
		make_literal(&coder->opts[cur + 1]);
		next_is_literal = true;
	}

	const uint32_t match_price = cur_price
			+ rc_bit_1_price(coder->is_match[state][pos_state]);
	const uint32_t rep_match_price = match_price
			+ rc_bit_1_price(coder->is_rep[state]);

	if (match_byte == current_byte
			&& !(coder->opts[cur + 1].pos_prev < cur
				&& coder->opts[cur + 1].back_prev == 0)) {

		const uint32_t short_rep_price = rep_match_price
				+ get_short_rep_price(coder, state, pos_state);

		if (short_rep_price <= coder->opts[cur + 1].price) {
			coder->opts[cur + 1].price = short_rep_price;
			coder->opts[cur + 1].pos_prev = cur;
			make_short_rep(&coder->opts[cur + 1]);
			next_is_literal = true;
		}
	}

	if (buf_avail_full < 2)
		return len_end;

	const uint32_t buf_avail = my_min(buf_avail_full, nice_len);

	if (!next_is_literal && match_byte != current_byte) { // speed optimization
		// try literal + rep0
		const uint8_t *const buf_back = buf - reps[0] - 1;
		const uint32_t limit = my_min(buf_avail_full, nice_len + 1);

		uint32_t len_test = 1;
		while (len_test < limit && buf[len_test] == buf_back[len_test])
			++len_test;

		--len_test;

		if (len_test >= 2) {
			lzma_lzma_state state_2 = state;
			update_literal(state_2);

			const uint32_t pos_state_next = (position + 1) & coder->pos_mask;
			const uint32_t next_rep_match_price = cur_and_1_price
					+ rc_bit_1_price(coder->is_match[state_2][pos_state_next])
					+ rc_bit_1_price(coder->is_rep[state_2]);

			//for (; len_test >= 2; --len_test) {
			const uint32_t offset = cur + 1 + len_test;

			while (len_end < offset)
				coder->opts[++len_end].price = RC_INFINITY_PRICE;

			const uint32_t cur_and_len_price = next_rep_match_price
					+ get_rep_price(coder, 0, len_test,
						state_2, pos_state_next);

			if (cur_and_len_price < coder->opts[offset].price) {
				coder->opts[offset].price = cur_and_len_price;
				coder->opts[offset].pos_prev = cur + 1;
				coder->opts[offset].back_prev = 0;
				coder->opts[offset].prev_1_is_literal = true;
				coder->opts[offset].prev_2 = false;
			}
			//}
		}
	}


	uint32_t start_len = 2; // speed optimization

	for (uint32_t rep_index = 0; rep_index < REP_DISTANCES; ++rep_index) {
		const uint8_t *const buf_back = buf - reps[rep_index] - 1;
		if (not_equal_16(buf, buf_back))
			continue;

		uint32_t len_test;
		for (len_test = 2; len_test < buf_avail
				&& buf[len_test] == buf_back[len_test];
				++len_test) ;

		while (len_end < cur + len_test)
			coder->opts[++len_end].price = RC_INFINITY_PRICE;

		const uint32_t len_test_temp = len_test;
		const uint32_t price = rep_match_price + get_pure_rep_price(
				coder, rep_index, state, pos_state);

		do {
			const uint32_t cur_and_len_price = price
					+ get_len_price(&coder->rep_len_encoder,
							len_test, pos_state);

			if (cur_and_len_price < coder->opts[cur + len_test].price) {
				coder->opts[cur + len_test].price = cur_and_len_price;
				coder->opts[cur + len_test].pos_prev = cur;
				coder->opts[cur + len_test].back_prev = rep_index;
				coder->opts[cur + len_test].prev_1_is_literal = false;
			}
		} while (--len_test >= 2);

		len_test = len_test_temp;

		if (rep_index == 0)
			start_len = len_test + 1;


		uint32_t len_test_2 = len_test + 1;
		const uint32_t limit = my_min(buf_avail_full,
				len_test_2 + nice_len);
		for (; len_test_2 < limit
				&& buf[len_test_2] == buf_back[len_test_2];
				++len_test_2) ;

		len_test_2 -= len_test + 1;

		if (len_test_2 >= 2) {
			lzma_lzma_state state_2 = state;
			update_long_rep(state_2);

			uint32_t pos_state_next = (position + len_test) & coder->pos_mask;

			const uint32_t cur_and_len_literal_price = price
					+ get_len_price(&coder->rep_len_encoder,
						len_test, pos_state)
					+ rc_bit_0_price(coder->is_match[state_2][pos_state_next])
					+ get_literal_price(coder, position + len_test,
						buf[len_test - 1], true,
						buf_back[len_test], buf[len_test]);

			update_literal(state_2);

			pos_state_next = (position + len_test + 1) & coder->pos_mask;

			const uint32_t next_rep_match_price = cur_and_len_literal_price
					+ rc_bit_1_price(coder->is_match[state_2][pos_state_next])
					+ rc_bit_1_price(coder->is_rep[state_2]);

			//for(; len_test_2 >= 2; len_test_2--) {
			const uint32_t offset = cur + len_test + 1 + len_test_2;

			while (len_end < offset)
				coder->opts[++len_end].price = RC_INFINITY_PRICE;

			const uint32_t cur_and_len_price = next_rep_match_price
					+ get_rep_price(coder, 0, len_test_2,
						state_2, pos_state_next);

			if (cur_and_len_price < coder->opts[offset].price) {
				coder->opts[offset].price = cur_and_len_price;
				coder->opts[offset].pos_prev = cur + len_test + 1;
				coder->opts[offset].back_prev = 0;
				coder->opts[offset].prev_1_is_literal = true;
				coder->opts[offset].prev_2 = true;
				coder->opts[offset].pos_prev_2 = cur;
				coder->opts[offset].back_prev_2 = rep_index;
			}
			//}
		}
	}


	//for (uint32_t len_test = 2; len_test <= new_len; ++len_test)
	if (new_len > buf_avail) {
		new_len = buf_avail;

		matches_count = 0;
		while (new_len > coder->matches[matches_count].len)
			++matches_count;

		coder->matches[matches_count++].len = new_len;
	}


	if (new_len >= start_len) {
		const uint32_t normal_match_price = match_price
				+ rc_bit_0_price(coder->is_rep[state]);

		while (len_end < cur + new_len)
			coder->opts[++len_end].price = RC_INFINITY_PRICE;

		uint32_t i = 0;
		while (start_len > coder->matches[i].len)
			++i;

		for (uint32_t len_test = start_len; ; ++len_test) {
			const uint32_t cur_back = coder->matches[i].dist;
			uint32_t cur_and_len_price = normal_match_price
					+ get_pos_len_price(coder,
						cur_back, len_test, pos_state);

			if (cur_and_len_price < coder->opts[cur + len_test].price) {
				coder->opts[cur + len_test].price = cur_and_len_price;
				coder->opts[cur + len_test].pos_prev = cur;
				coder->opts[cur + len_test].back_prev
						= cur_back + REP_DISTANCES;
				coder->opts[cur + len_test].prev_1_is_literal = false;
			}

			if (len_test == coder->matches[i].len) {
				// Try Match + Literal + Rep0
				const uint8_t *const buf_back = buf - cur_back - 1;
				uint32_t len_test_2 = len_test + 1;
				const uint32_t limit = my_min(buf_avail_full,
						len_test_2 + nice_len);

				for (; len_test_2 < limit &&
						buf[len_test_2] == buf_back[len_test_2];
						++len_test_2) ;

				len_test_2 -= len_test + 1;

				if (len_test_2 >= 2) {
					lzma_lzma_state state_2 = state;
					update_match(state_2);
					uint32_t pos_state_next
							= (position + len_test) & coder->pos_mask;

					const uint32_t cur_and_len_literal_price = cur_and_len_price
							+ rc_bit_0_price(
								coder->is_match[state_2][pos_state_next])
							+ get_literal_price(coder,
								position + len_test,
								buf[len_test - 1],
								true,
								buf_back[len_test],
								buf[len_test]);

					update_literal(state_2);
					pos_state_next = (pos_state_next + 1) & coder->pos_mask;

					const uint32_t next_rep_match_price
							= cur_and_len_literal_price
							+ rc_bit_1_price(
								coder->is_match[state_2][pos_state_next])
							+ rc_bit_1_price(coder->is_rep[state_2]);

					// for(; len_test_2 >= 2; --len_test_2) {
					const uint32_t offset = cur + len_test + 1 + len_test_2;

					while (len_end < offset)
						coder->opts[++len_end].price = RC_INFINITY_PRICE;

					cur_and_len_price = next_rep_match_price
							+ get_rep_price(coder, 0, len_test_2,
								state_2, pos_state_next);

					if (cur_and_len_price < coder->opts[offset].price) {
						coder->opts[offset].price = cur_and_len_price;
						coder->opts[offset].pos_prev = cur + len_test + 1;
						coder->opts[offset].back_prev = 0;
						coder->opts[offset].prev_1_is_literal = true;
						coder->opts[offset].prev_2 = true;
						coder->opts[offset].pos_prev_2 = cur;
						coder->opts[offset].back_prev_2
								= cur_back + REP_DISTANCES;
					}
					//}
				}

				if (++i == matches_count)
					break;
			}
		}
	}

	return len_end;
}