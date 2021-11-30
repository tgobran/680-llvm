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

### 1. \<XStringCached\>destroyObject (xalanc/PlatformSupport/ReusableArenaAllocator.hpp)
Refrate Runtime:  45.85% \
Refrate Compile Time: 21ms 57us \
Calls: 67113825

Features:
- Two level for loop nest (not perfect nesting)
- few if statements inside loops
- several function calls

Active Passes:
- LoopInstSimplifyPass
- SimplifyCFGPass
- EarlyCSEPass
- InstSimplifyPass
- LoopUnrollPass
- TailCallElimPass
- BDCEPass
- LoopSimplifyPass
- SCCPPass
- LoopRotatePass
- ADCEPass
- GlobalOptPass
- LICMPass
- InstCombinePass
- JumpThreadingPass
- SROA
- InlinerPass
- LCSSAPass
- DSEPass
- GVN

### 2. contains (ValueStore.cpp)
Refrate Runtime:  19.47% \
Refrate Compile Time:	12ms 841us \
Calls: 317186

Features:
- Two while loops (not nested)
- Several if statements inside loops
- Several function calls
- Function calls inside if condition
- Nested if statements.

Active Passes:
- LoopInstSimplifyPass
- SimplifyCFGPass
- PostOrderFunctionAttrsPass
- EarlyCSEPass
- InstSimplifyPass
- LoopUnrollPass
- TailCallElimPass
- BDCEPass
- LoopSimplifyPass
- LoopRotatePass
- IPSCCPPass
- IndVarSimplifyPass
- GlobalOptPass
- LICMPass
- InstCombinePass
- JumpThreadingPass
- SROA
- InlinerPass
- LCSSAPass
- GVN
- GlobalDCEPass

### 3. release (XalanDOMStringCache.cpp)
Refrate Runtime:  7.34% \
Refrate Compile Time:	9ms 314us \
Calls: 67596232

Features:
- Several function calls
- Two level nested if statement
- calling a function as a parameter to another function call

Active Passes:
- SimplifyCFGPass
- CorrelatedValuePropagationPass
- EarlyCSEPass
- InstSimplifyPass
- LoopUnrollPass
- TailCallElimPass
- LoopSimplifyPass
- GlobalOptPass
- LICMPass
- InstCombinePass
- JumpThreadingPass
- SROA
- InlinerPass
- LCSSAPass
- GVN
