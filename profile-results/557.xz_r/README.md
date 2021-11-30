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

### Total Program Runtime: TODO

### 1. lzma_mf_bt4_find (liblzma/lz/lz_encoder_mf.c)
Refrate Runtime: 50.66% \
Refrate Compile Time: 31ms 573us \
Calls: 195785210

Features:
- For loop inside if statement
- If statement inside the for loop
- Function calls inside if statements (not inside the loop)
- Function calls outside of if statements and fop loop

Active Passes:
- SimplifyCFGPass
- PostOrderFunctionAttrsPass
- CorrelatedValuePropagationPass
- EarlyCSEPass
- InstSimplifyPass
- LoopUnrollPass
- TailCallElimPass
- LoopSimplifyPass
- LoopRotatePass
- IndVarSimplifyPass
- GlobalOptPass
- LICMPass
- InstCombinePass
- JumpThreadingPass
- LoopVectorizePass
- SROA
- InlinerPass
- ReassociatePass
- LCSSAPass
- LoopLoadEliminationPass
- GVN

### 2. lzma_lzma_optimum_normal (liblzma/lzma/lzma_encoder_optimum_normal.c)
Refrate Runtime: 30.43% \
Refrate Compile Time: 162ms 170us \
Calls: 27196108

Features:
- For loop with if statement inside
- Function calls inside the loop and the if statement
- Nested if statements outside of the loop
- Function call outside of the loop

Active Passes:
- GlobalOptPass 

### 3. lzma_mf_bt4_skip (liblzma/lz/lz_encoder_mf.c)
Refrate Runtime: 10.03% \
Refrate Compile Time: 15ms 505us \
Calls: 463536

Features:
- A single do-while loop with 3 function calls inside

Active Passes:
- SimplifyCFGPass
- PostOrderFunctionAttrsPass
- EarlyCSEPass
- InstSimplifyPass
- LoopUnrollPass
- TailCallElimPass
- LoopSimplifyPass
- GlobalOptPass
- LICMPass
- InstCombinePass
- JumpThreadingPass
- LoopVectorizePass
- SROA
- InlinerPass
- ReassociatePass
- LCSSAPass
- LoopLoadEliminationPass
- GVN
