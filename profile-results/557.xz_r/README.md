## Refrate O2 Hot Functions

For Runtime information the following options were used:
```
-g -O2 -pg -march=native -fno-unsafe-math-optimizations -fcommon
```

Additional instrumentation information was gathered with the following options:
```
-g -O2 -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate
```

Active pass information was gathered with the following options:
```
-g -O2 -mllvm --print-changed=quiet -mllvm --filter-print-funcs=FUNCNAME -march=native -fno-unsafe-math-optimizations -fcommon
```

Function compile time information was gathered with the following options:
```
-g -O2 -march=native -fno-unsafe-math-optimizations -fcommon -ftime-trace
```

### 1. lzma_mf_bt4_find (liblzma/lz/lz_encoder_mf.c)
Refrate Runtime: 50.66% \
Refrate Compile Time: TODO \
Calls: 195785210

Features:
- TODO

Active Passes:
- TODO

### 2. lzma_lzma_optimum_normal (liblzma/lzma/lzma_encoder_optimum_normal.c)
Refrate Runtime: 30.43% \
Refrate Compile Time: TODO \
Calls: 27196108

Features:
- TODO

Active Passes:
- TODO

### 3. lzma_mf_bt4_skip (liblzma/lz/lz_encoder_mf.c)
Refrate Runtime: 10.03% \
Refrate Compile Time: TODO \
Calls: 463536

Features:
- TODO

Active Passes:
- TODO
