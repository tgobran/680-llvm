## Refrate O0 Hot Functions

For Runtime information the following options were used:
```
-g -O2 -pg -march=native -fno-unsafe-math-optimizations -fcommon
```

The benchmarks timing results for some unknown reason weren't able to produce useable results.

### 1. spec_genrand_int32 (specrand-common/specrand.c)
Basic Block Executions: 1879384

Features:
Two level nested for loop within nested if condition.

### 2. spec_rand (specrand-common/specrand.c)
Basic Block Executions: 939692

Features:
A function call within another function which only perform basic mathematical operation.

### 3. spec_lrand48 (specrand-common/specrand.c)
Basic Block Executions: 939692

Features:
A function call within another function which only perform basic mathematical operation.

### 4. run_sequence (main.c)
Basic Block Executions: 469846

Features:
1.	Multiple for loop with if else condition. Function call within the if else statement.
2.	Multiple one level for loop with a single if condition. Within the if condition there is function call.


### 5. spec_init_genrand (specrand-common/specrand.c)
Basic Block Executions: 4984

Features:
A simple for loop within a function.

## Refrate O2 Hot Functions

The program was compiled with the following options:

```-g -O2 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

TODO
