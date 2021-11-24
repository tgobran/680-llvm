extern void
lzma_mf_bt4_skip(lzma_mf *mf, uint32_t amount)
{
	do {
		header_skip(true, 4);

		hash_4_calc();

		const uint32_t cur_match
				= mf->hash[FIX_4_HASH_SIZE + hash_value];

		mf->hash[hash_2_value] = pos;
		mf->hash[FIX_3_HASH_SIZE + hash_3_value] = pos;
		mf->hash[FIX_4_HASH_SIZE + hash_value] = pos;

		bt_skip();

	} while (--amount != 0);
}
#endif
