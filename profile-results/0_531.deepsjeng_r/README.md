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

### 1. ProbeTT (ttable.cpp)
Refrate Runtime: 18.36% \
Calls: 506568965

Features:
- TODO

Active Passes:
- TODO

### 2. search (search.cpp)
Refrate Runtime: 11.07% \
Calls: 9697

Features:
- TODO

### 3. qsearch (search.cpp)
Refrate Runtime: 7.77% \
Calls: 140537493

Features:
- TODO

## Reason for Error

This benchmark worked fine with the profiling and instrumentation but reported no passes to be running on the two search functions despite them being present in the compiled code, we consider this to be an error or quirk with how llvm is treating the functions.
