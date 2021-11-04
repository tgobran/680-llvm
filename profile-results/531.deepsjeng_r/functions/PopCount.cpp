#if !defined(SPEC)
const
#endif
int PopCount (BITBOARD b) {
    int c = 0;

    while (b) {
        b &= b - 1;
        c++;
    }

    return c;
}