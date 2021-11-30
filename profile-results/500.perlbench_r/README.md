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

### 1. regex.c:S_regmatch (ext/re/re_exec.c)
Refrate Runtime: 31.77% \
Refrate Compile Time: 668ms 157us \
Calls: 457545339 

Features:
- Single level while loop with complex function call.
- Multiple nested if conditions with function calls inside.
- Several if-else conditions within while loops.
- While loops inside if statements.
- Multiple nexted if-else statements.
- Case statements with multiple if-else statements.
- Several nested if conditions within a single level for loop.
- While condition with function inside a nested loop.
- Nested loops within while loops, sometimes based on conditions.

Active Passes:
- SimplifyCFGPass
- SROA
- EarlyCSEPass
- IPSCCPPass
- CalledValuePropagationPass
- GlobalOptPass
- DeadArgumentEliminationPass
- InstCombinePass
- InlinerPass
- JumpThreadingPass
- CorrelatedValuePropagationPass
- TailCallElimPass
- ReassociatePass
- LoopSimplifyPass
- LCSSAPass
- LICMPass
- LoopRotatePass
- LoopInstSimplifyPass
- IndVarSimplifyPass
- MergedLoadStoreMotionPass
- GVN
- BDCEPass
- DSEPass
- LoopUnrollPass

### 2. Perl_regexec_flags (regexec.c)
Refrate Runtime: 7.75% \
Refrate Compile Time: 107ms 910us \
Calls: 429181430

Features:
- Several nested if-else statements with complex function calls inside.
- Several nested if conditions with goto statements.
- Several nested if statements with function calls inside.
- A while loop with several nested if statements inside.
- A nested if condition with a while loop inside.

Active Passes:
- SimplifyCFGPass
- SROA
- EarlyCSEPass
- IPSCCPPass
- GlobalOptPass
- DeadArgumentEliminationPass
- InstCombinePass
- InlinerPass
- PostOrderFunctionAttrsPass
- JumpThreadingPass
- CorrelatedValuePropagationPass
- TailCallElimPass
- ReassociatePass
- LoopSimplifyPass
- LCSSAPass
- LICMPass
- LoopRotatePass
- IndVarSimplifyPass
- GVN
- BDCEPass
- MemCpyOptPass
- LoopUnrollPass

### 3. Perl_hv_common (hv.c)
Refrate Runtime: 6.96% \
Refrate Compile Time: 96ms 78us \
Calls: 506576246

Features:
- Several nested if-else statements with complex function calls within.
- Multiple single level loops with if statements inside, some have calls to complex functions.

Active Passes:
- IPSCCPPass
- GlobalOptPass
- InferFunctionAttrsPass
- SimplifyCFGPass
- SROA
- EarlyCSEPass
- DeadArgumentEliminationPass
- InstCombinePass
- JumpThreadingPass
- CorrelatedValuePropagationPass
- TailCallElimPass
- ReassociatePass
- LoopSimplifyPass
- LCSSAPass
- LoopRotatePass
- LICMPass
- GVN
- BDCEPass
- DSEPass
- InferFunctionAttrsPass

## Ignored Functions

### Perl_pp_match: 8.78%
- Couldn't be found in the source files for proper function analysis.

### Perl_pp_multideref: 7.96%
- Couldn't be found in the source files for proper function analysis.
