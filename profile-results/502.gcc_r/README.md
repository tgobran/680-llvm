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

### 1. df_worklist_dataflow (df-core.c)
Refrate Runtime: 3.09% \
Calls: 1329

Features:
- TODO

Active Passes:
- TODO

### 2. bitmap_set_bit (bitmap.c)
Refrate Runtime: 2.70% \
Calls: 178091693

Features:
- TODO

Active Passes:
- TODO

### 3. df_note_compute (df-problems.c)
Refrate Runtime: 2.16% \
Calls: 395

Features:
- TODO

Active Passes:
- TODO
