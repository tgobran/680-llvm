int FindFirstRemove(BITBOARD * const b) {    
    const BITBOARD lsb = (*b) & -(*b);
    *b ^= lsb;
    return magictable[(lsb * magic) >> 58];  
}