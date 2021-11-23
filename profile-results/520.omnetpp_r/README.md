## Refrate O2 Hot Functions

For Runtime information the following options were used:
```
-g -O2 -pg -march=native -fno-unsafe-math-optimizations -fcommon
```

For Basic Block information the following options were used:
```
-g -O2 -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate
```

### 1. shiftup (simulator/cmessageheap.cc)
Basic Block Executions: TODO \
Refrate Runtime: 27.37% \
Calls: 468300994

Features:
- Relatively simple function
- Single nested loop.
- Iteration count derived from a function argument and a global variable.
- Minor control flow inside of the loop.

### 2. deliver (simulator/cgate.cc)
Basic Block Executions: TODO \
Refrate Runtime: 9.43% \
Calls: 309644908

Features:
- Control flow function
- Calls other functions within.

### 3. record (simulator/indexedfileoutvectormgr.cc)
Basic Block Executions: TODO \
Refrate Runtime: 9.07% \
Calls: 494191607

Features:
- Control flow function
- Nested if statements with considerable calculation depending on the route.
