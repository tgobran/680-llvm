## Refrate O2 Hot Functions

The program was compiled with the following options:

```
-g -O2 -pg -march=native -fno-unsafe-math-optimizations -fcommon
```

Smaller functions such as simple Constructors were ignored for hot functions.

### 1. cost_compare (pbeampp.c)
Basic Block Executions: 17199625996

Features:
- several if then else statements
- function declared with two different prototypes by the help of #IFDEF

### 2. primal_bea_mpp (pbeampp.c)
Basic Block Executions: 9092114390

Features:
   - function declared with two different prototypes by the help of #IFDEF
   - several one level loops with if statements with complicated condition inside them
   - if statements with function calls as their condition

### 3. spec_qsort (spec_qsort/spec_qsort.c)
Basic Block Executions: 8160331559

Features:
- two level perfect nested loops with a function call inside the second loop.  --> three of this
- function calls inside if statements
- function calls in condition part of the loop
- function calls inside of if statement
- goto !!!!!!!  ----> considerning this goto it's three level nested loop

### 4. replace_weaker_arc (implicit.c)
Basic Block Executions: 3948246921

Features:
 - function declared with two different prototypes by the help of #IFDEF
 - one level loop
 - double nested if statement inside loop

### 5. price_out_impl (implicit.c)
Basic Block Executions: 3712252500

Features:
  - function declared with two different prototypes by the help of #IFDEF
  - malloc --> heap access
  - nested if statements with function calls inside
  - one level loops with if statements inside, some of thos if statments contain function calls inside
  - several if then elses inside loop
  - call another hot function --> qsort
  - if --> loop ----> function call

## Refrate O2 Hot Functions

The program was compiled with the following options:

```-g -O2 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

TODO
