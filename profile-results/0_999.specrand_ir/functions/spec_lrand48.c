/* Just a copy of spec_genrand_int31() */
CLINK long spec_lrand48(void) {
    return (long)(spec_genrand_int32()>>1);
}
