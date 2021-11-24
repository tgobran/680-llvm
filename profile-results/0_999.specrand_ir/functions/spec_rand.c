/* Just a copy of spec_genrand_real2() */
CLINK double spec_rand(void) {
    return spec_genrand_int32()*(1.0/4294967296.0); 
}