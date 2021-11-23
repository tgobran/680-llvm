## Refrate O2 Hot Functions

The program was compiled with the following options:

```
-g -O2 -pg -march=native -fno-unsafe-math-optimizations -fcommon
```

### 1. spec_qsort (spec_qsort/spec_qsort.c)
Basic Block Executions: 8160331559

Refrate Runtime: 24.99%

Features:
- Nested two level loop with a function call inside (three present in the code).
- Function calls inside if statements.
- Function calls in conditions of the loops.
- Function calls inside of if statements.
- Goto present, jumps back to the start of the function allowing more loop executions.

### 2. primal_bea_mpp (pbeampp.c)
Basic Block Executions: 9092114390

Refrate Runtime: 18.80%

Features:
- Done with a conditional definition for two different prototypes.
- Several single level loops with if statements based on complicated conditions inside.
- If statements with function calls as their conditions.

### 3. cost_compare (pbeampp.c)
Basic Block Executions: 17199625996

Refrate Runtime: 13.67%

Features:
- Very simple function.
- Done with a conditional definition for two different prototypes.
- Series of comparion if statements that decide from various return values.
