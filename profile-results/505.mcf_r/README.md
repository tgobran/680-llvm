## Refrate O2 Hot Functions

For Runtime information the following options were used:
```
-g -O2 -pg -march=native -fno-unsafe-math-optimizations -fcommon
```

For Basic Block information the following options were used:
```
-g -O2 -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate
```

### 1. spec_qsort (spec_qsort/spec_qsort.c)
Basic Block Executions: TODO \
Refrate Runtime: 24.99% \
Calls: 1977279

Features:
- Nested two level loop with a function call inside (three present in the code).
- Function calls inside if statements.
- Function calls in conditions of the loops.
- Function calls inside of if statements.
- Goto present, jumps back to the start of the function allowing more loop executions.

### 2. primal_bea_mpp (pbeampp.c)
Basic Block Executions: TODO \
Refrate Runtime: 18.80% \
Calls: 1977279

Features:
- Done with a conditional definition for two different prototypes.
- Several single level loops with if statements based on complicated conditions inside.
- If statements with function calls as their conditions.

### 3. cost_compare (pbeampp.c)
Basic Block Executions: TODO \
Refrate Runtime: 13.67% \
Calls: 8609691404

Features:
- Very simple function.
- Done with a conditional definition for two different prototypes.
- Series of comparion if statements that decide from various return values.
