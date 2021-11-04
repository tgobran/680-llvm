static uint32_t
get_literal_price(const lzma_coder *const pcoder, const uint32_t pos,
		const uint32_t prev_byte, const bool match_mode,
		uint32_t match_byte, uint32_t symbol)
{
	const struct lzma_coder_s *const coder = pcoder;
	const probability *const subcoder = literal_subcoder(coder->literal,
			coder->literal_context_bits, coder->literal_pos_mask,
			pos, prev_byte);

	uint32_t price = 0;

	if (!match_mode) {
		price = rc_bittree_price(subcoder, 8, symbol);
	} else {
		uint32_t offset = 0x100;
		symbol += UINT32_C(1) << 8;

		do {
			match_byte <<= 1;

			const uint32_t match_bit = match_byte & offset;
			const uint32_t subcoder_index
					= offset + match_bit + (symbol >> 8);
			const uint32_t bit = (symbol >> 7) & 1;
			price += rc_bit_price(subcoder[subcoder_index], bit);

			symbol <<= 1;
			offset &= ~(match_byte ^ symbol);

		} while (symbol < (UINT32_C(1) << 16));
	}

	return price;
}