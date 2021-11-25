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

### 1. shiftup (simulator/cmessageheap.cc)
Refrate Runtime: 27.37% \
Refrate Compile Time:	5ms 93us \
Calls: 468300994

Features:
- Relatively simple function
- Single nested loop.
- Iteration count derived from a function argument and a global variable.
- Minor control flow inside of the loop.

Active Passes:
- TODO

### 2. deliver (simulator/cgate.cc)
Refrate Runtime: 9.43% \
Refrate Compile Time: 4ms 47us \
Calls: 309644908

Features:
- Control flow function
- Calls other functions within.

Active Passes:
- TODO

### 3. record (simulator/indexedfileoutvectormgr.cc)
Refrate Runtime: 9.07% \
Refrate Compile Time:	3ms 97us \
Calls: 494191607

Features:
- Control flow function
- Nested if statements with considerable calculation depending on the route.

Active Passes:
- TODO
