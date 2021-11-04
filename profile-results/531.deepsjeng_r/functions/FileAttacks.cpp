#if !defined(SPEC)
const
#endif
BITBOARD FileAttacks(BITBOARD occ, const unsigned int sq) {
   unsigned int o;
   unsigned int f = sq & 7;
   occ   =   0x0101010101010101ULL & (occ   >> f); 
   o     = ( 0x0080402010080400ULL *  occ ) >> 58;
   return  ( aFileAttacks[o][sq>>3]    ) << f;   
}