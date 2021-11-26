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

### 1. update_board_fast (FastBoard.cpp)
Refrate Runtime: 8.22% \
Refrate Compile Time: 11ms 59us \
Calls: 791662684

Features:
- several nested if statements inside a for loop with a fixed number of iterations
- function calls inside for loop
- function call outside of the loop

Active Passes:
- SimplifyCFGPass
- PostOrderFunctionAttrsPass
- EarlyCSEPass
- InstSimplifyPass
- LoopUnrollPass
- TailCallElimPass
- LoopSimplifyPass
- LoopRotatePass
- IPSCCPPass
- IndVarSimplifyPass
- GlobalOptPass
- InferFunctionAttrsPass
- LICMPass
- InstCombinePass
- JumpThreadingPass
- SROA
- InlinerPass
- LCSSAPass
- GVN
- GlobalDCEPass

### 2. play_random_move (FastState.cpp)
Refrate Runtime: 8.19% \
Refrate Compile Time: 31ms 283us \
Calls: 818055991

Features:
- Nested if statements with function call inside.
- One level for loop within the if condition.
- Two-level nested for loops inside if
- function call inside the inner loop 
- do-while loop with if statements and function calls inside
- for loop with if statements and function calls inside (no loop nest)

Active Passes:
- InstCombinePass
- SROA
- TailCallElimPass
- GlobalOptPass

### 3. self_atari (FastBoard.cpp)
Refrate Runtime: 7.65% \
Refrate Compile Time: 7ms 458us \
Calls: 4213754285

Features:
- Two level for loop nest
- Nested if statements in the outer loop
- If statement inside the inner loop
- Function call in the condition of an if statement inside the outer loop 

Active Passes:
- LoopInstSimplifyPass
- SimplifyCFGPass
- EarlyCSEPass
- LoopUnrollPass
- TailCallElimPass
- LoopSimplifyPass
- LoopRotatePass
- IPSCCPPass
- IndVarSimplifyPass
- GlobalOptPass
- InferFunctionAttrsPass
- LICMPass
- InstCombinePass
- JumpThreadingPass
- EliminateAvailableExternallyPass
- SROA
- InlinerPass
- ReassociatePass
- LCSSAPass
- GVN
- GlobalDCEPass
