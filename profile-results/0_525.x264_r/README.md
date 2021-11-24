## Refrate O2 Hot Functions

For Runtime information the following options were used:
```
-g -O2 -pg -march=native -fno-unsafe-math-optimizations -fcommon
```

Additional instrumentation information was gathered with the following options:
```
-g -O2 -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate
```

## Reason for Error

This benchmark produced inconsistent profiling results between the instrumentation and the runtime profiling. In particular the results from the runtime profiling make no logical sense as they assign above 50% of the runtime to two functions.
