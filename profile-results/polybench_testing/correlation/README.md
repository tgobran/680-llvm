# Compilation

## O2 Active Passes: 
1. InferFunctionAttrsPass (inferattrs)
2. SimplifyCFGPass (simplifycfg)
3. SROA (sroa)
4. EarlyCSEPass (early-cse)
5. GlobalOptPass (globalopt)
6. InstCombinePass (instcombine)
7. PostOrderFunctionAttrsPass (function-attrs)
8. TailCallElimPass (tailcallelim)
9. ReassociatePass (reassociate)
10. LoopSimplifyPass (loop-simplify)
11. LCSSAPass (lcssa)
12. LoopRotatePass (loop-rotate)
13. LICMPass (licm)
14. IndVarSimplifyPass (indvars)
15. GVN (gvn)
16. JumpThreadingPass (jump-threading)
17. CorrelatedValuePropagationPass (correlated-propagation)
18. LoopUnrollPass (loop-unroll)
19. InstSimplifyPass (instsimplify)

## Pruned (Only Active) Active Passes:
* InferFunctionAttrsPass
* SimplifyCFGPass
* SROA
* EarlyCSEPass
* GlobalOptPass
* InstCombinePass
* PostOrderFunctionAttrsPass
* TailCallElimPass
* ReassociatePass
* LCSSAPass
* LoopRotatePass
* LICMPass
* IndVarSimplifyPass
* GVN
* JumpThreadingPass
* LoopUnrollPass
* InstSimplifyPass

Missing:
* LoopSimplifyPass
* CorrelatedValuePropagationPass

## Compilation Times (25 Run Average):
* .044864s (O2)
* .012152s 3.69x (Active Passes)
* .012136s 3.70x (InferFunctionAttrsPass Excluded)
* .012304s 3.65x (SimplifyCFGPass Excluded)
* .018404s 2.44x (SROA Excluded)
* .011924s 3.76x (EarlyCSEPass Excluded)
* .012100s 3.71x (GlobalOptPass Excluded)
* .011944s 3.76x (InstCombinePass Excluded)
* .011960s 3.75x (PostOrderFunctionAttrsPass Excluded)
* .012104s 3.71x (TailCallElimPass Excluded)
* .012084s 3.71x (ReassociatePass Excluded)
* .012132s 3.70x (LoopSimplifyPass Excluded)
* .012124s 3.70x (LCSSAPass Excluded)
* .011668s 3.85x (LoopRotatePass Excluded)
* .011872s 3.78x (LICMPass Excluded)
* .008708s 5.15x (IndVarSimplifyPass Excluded)
* .011272s 3.98x (GVN Excluded)
* .011884s 3.76x (JumpThreadingPass Excluded)
* .012024s 3.73x (CorrelatedValuePropagationPass Excluded)
* .009628s 4.66x (LoopUnrollPass Excluded)
* .012108s 3.71x (InstSimplifyPass Excluded)

Largest Compilation Time Improvment: 5.15x (IndVarSimplifyPass Excluded)

Smallest Compilation Time Improvement: 2.44x (SROA Excluded)

# Runtime

## Runtime Times (5 Run Average, LARGE_DATASET):
* 6.53s (O2)
* 6.55s 1.00x (Active Passes)
* 6.51s 1.00x (InferFunctionAttrsPass Excluded)
* 6.49s 1.01x (SimplifyCFGPass Excluded)
* 6.53s 1.00x (SROA Excluded)
* 6.50s 1.00x (EarlyCSEPass Excluded)
* 6.58s 0.99x (GlobalOptPass Excluded)
* 6.52s 1.00x (InstCombinePass Excluded)
* 6.55s 1.00x (PostOrderFunctionAttrsPass Excluded)
* 6.49s 1.01x (TailCallElimPass Excluded)
* 6.54s 1.00x (ReassociatePass Excluded)
* 6.55s 1.00x (LoopSimplifyPass Excluded)
* 6.45s 1.01x (LCSSAPass Excluded)
* 6.51s 1.00x (LoopRotatePass Excluded)
* 6.57s 0.99x (LICMPass Excluded)
* 6.58s 0.99x (IndVarSimplifyPass Excluded)
* 6.52s 1.00x (GVN Excluded)
* 6.60s 0.99x (JumpThreadingPass Excluded)
* 6.50s 1.00x (CorrelatedValuePropagationPass Excluded)
* 6.56s 1.00x (LoopUnrollPass Excluded)
* 6.57s 0.99x (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: 0.99x (JumpThreadingPass Excluded)

Smallest Runtime Time Cost: 1.01x (LCSSAPass Excluded)

## Runtime Times (5 Run Average, EXTRALARGE_DATASET):
* TODO (O2)
* TODO (Active Passes)
* TODO (InferFunctionAttrsPass Excluded)
* TODO (SimplifyCFGPass Excluded)
* TODO (SROA Excluded)
* TODO (EarlyCSEPass Excluded)
* TODO (GlobalOptPass Excluded)
* TODO (InstCombinePass Excluded)
* TODO (PostOrderFunctionAttrsPass Excluded)
* TODO (TailCallElimPass Excluded)
* TODO (ReassociatePass Excluded)
* TODO (LoopSimplifyPass Excluded)
* TODO (LCSSAPass Excluded)
* TODO (LoopRotatePass Excluded)
* TODO (LICMPass Excluded)
* TODO (IndVarSimplifyPass Excluded)
* TODO (GVN Excluded)
* TODO (JumpThreadingPass Excluded)
* TODO (CorrelatedValuePropagationPass Excluded)
* TODO (LoopUnrollPass Excluded)
* TODO (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: TODO

Smallest Runtime Time Cost: TODO
