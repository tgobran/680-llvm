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

### Total Program Runtime: 440.29s

### 1. spec_qsort (spec_qsort/spec_qsort.c)
Refrate Runtime: 109.97 (24.99%) \
Refrate Compile Time: 179ms 256us \
Calls: 1977279

Features:
- Nested two level loop with a function call inside (three present in the code).
- Function calls inside if statements.
- Function calls in conditions of the loops.
- Function calls inside of if statements.
- Goto present, jumps back to the start of the function allowing more loop executions.

Active Passes:
1.	GlobalOptPass
2.	InferFunctionAttrsPass
3.	IPSCCPPass

### 2. primal_bea_mpp (pbeampp.c)
Refrate Runtime: 82.73 (18.80%) \
Refrate Compile Time: 14ms 748us \
Calls: 1977279

Features:
- Done with a conditional definition for two different prototypes.
- Several single level loops with if statements based on complicated conditions inside.
- If statements with function calls as their conditions.

Active Passes:
1.	GlobalOptPass
2.	SimplifyCFGPass
3.	SROA
4.	EarlyCSEPass
5.	InstCombinePass
6.	InlinerPass
7.	PostOrderFunctionAttrsPass
8.	JumpThreadingPass
9.	TailCallElimPass
10.	ReassociatePass
11.	LoopSimplifyPass
12.	LCSSAPass
13.	LoopRotatePass
14.	LoopInstSimplifyPass
15.	LICMPass
16.	LoopVectorizePass
17.	LoopLoadEliminationPass
18.	LoopUnrollPass 
19.	InstSimplifyPass
20.	IndVarSimplifyPass
21.	GVN


### 3. cost_compare (pbeampp.c)
Refrate Runtime: 60.16 (13.67%) \
Refrate Compile Time: 1ms 561us \
Calls: 8609691404

Features:
- Very simple function.
- Done with a conditional definition for two different prototypes.
- Series of comparion if statements that decide from various return values.

Active Passes:
1.	SROA
2.	EarlyCSEPass
3.	InstCombinePass
4.	SimplifyCFGPass
5.	PostOrderFunctionAttrsPass
6.	GlobalOptPass

