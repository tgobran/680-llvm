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

### 1. df_worklist_dataflow (df-core.c)
Refrate Runtime: 3.09% \
Refrate Compile Time: 30ms 930us \
Calls: 1329

Features:
- TODO

Active Passes:
- TODO

### 2. bitmap_set_bit (bitmap.c)
Refrate Runtime: 2.70% \
Refrate Compile Time: 10ms 554us \
Calls: 178091693

Features:
- TODO

Active Passes:
1.	InferFunctionAttrsPass
2.	SROA
3.	EarlyCSEPass
4.	GlobalOptPass
5.	InstCombinePass
6.	SimplifyCFGPass
7.	InlinerPass
8.	PostOrderFunctionAttrsPass
9.	JumpThreadingPass
10.	CorrelatedValuePropagationPass
11.	LCSSAPass
12.	TailCallElimPass
13.	LoopSimplifyPassg
14.	LICMPass
15.	GVN
16.	LoopUnrollPass
17.	IPSCCPPass
18.	CalledValuePropagationPass
19.	InferFunctionAttrsPass


### 3. df_note_compute (df-problems.c)
Refrate Runtime: 2.16% \
Refrate Compile Time: 51ms 12us \
Calls: 395

Features:
- TODO

Active Passes:
1.	InferFunctionAttrsPass
2.	SimplifyCFGPass
3.	SROA
4.	EarlyCSEPass
5.	GlobalOptPass
6.	InstCombinePass
7.	InlinerPass
8.	PostOrderFunctionAttrsPass
9.	JumpThreadingPass
10.	CorrelatedValuePropagationPass
11.	TailCallElimPass
12.	ReassociatePass
13.	LoopSimplifyPass
14.	LCSSAPass
15.	LICMPass
16.	LoopRotatePass
17.	LoopInstSimplifyPass
18.	GVN
19.	LoopUnrollPass


