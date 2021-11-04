#if !defined(SPEC)
const
#endif
BITBOARD RankAttacks(const BITBOARD occ, const unsigned int sq) {
   unsigned int f = sq &  7;
   unsigned int r = sq & ~7; 
   unsigned int o = (unsigned int)(occ >> (r+1)) & 63;
   return (BITBOARD) firstRankAttacks[o][f] << r;
}
