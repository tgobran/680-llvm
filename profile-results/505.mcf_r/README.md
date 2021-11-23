## Refrate O2 Hot Functions

The program was compiled with the following options:

```
-g -O2 -pg -march=native -fno-unsafe-math-optimizations -fcommon
```

### 1. spec_qsort (spec_qsort/spec_qsort.c)
Basic Block Executions: 8160331559

Refrate Runtime: 24.99%

Features:
- two level perfect nested loops with a function call inside the second loop.  --> three of this
- function calls inside if statements
- function calls in condition part of the loop
- function calls inside of if statement
- goto !!!!!!!  ----> considerning this goto it's three level nested loop


### 2. primal_bea_mpp (pbeampp.c)
Basic Block Executions: 9092114390

Refrate Runtime: 18.80%

Features:
   - function declared with two different prototypes by the help of #IFDEF
   - several one level loops with if statements with complicated condition inside them
   - if statements with function calls as their condition

### 3. cost_compare (pbeampp.c)
Basic Block Executions: 17199625996

Refrate Runtime: 13.67%

Features:
- several if then else statements
- function declared with two different prototypes by the help of #IFDEF
