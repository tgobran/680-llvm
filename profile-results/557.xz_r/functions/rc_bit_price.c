static inline uint32_t
rc_bit_price(const probability prob, const uint32_t bit)
{
	return lzma_rc_prices[(prob ^ ((UINT32_C(0) - bit)
			& (RC_BIT_MODEL_TOTAL - 1))) >> RC_MOVE_REDUCING_BITS];
}