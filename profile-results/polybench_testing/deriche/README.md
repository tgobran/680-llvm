# Compilation

## O2 Active Passes: 
1. InferFunctionAttrsPass (inferattrs)
2. SimplifyCFGPass (simplifycfg)
3. SROA (sroa)
4. EarlyCSEPass (early-cse)
5. GlobalOptPass (globalopt)
6. InstCombinePass (instcombine)
7. PostOrderFunctionAttrsPass (function-attrs)
8. CorrelatedValuePropagationPass (correlated-propagation)
9. TailCallElimPass (tailcallelim)
10. ReassociatePass (reassociate)
11. LoopSimplifyPass (loop-simplify)
12. LICMPass (licm)
13. LoopRotatePass (loop-rotate)
14. IndVarSimplifyPass (indvars)
15. GVN (gvn)
16. JumpThreadingPass (jump-threading)
17. LoopVectorizePass (loop-vectorize)
18. LoopLoadEliminationPass (loop-load-elim)
19. LoopUnrollPass (loop-unroll)
20. LCSSAPass (lcssa)
21. InstSimplifyPass (instsimplify)

## Pruned (Only Active) Active Passes:
* InferFunctionAttrsPass
* SimplifyCFGPass
* SROA
* EarlyCSEPass
* GlobalOptPass
* InstCombinePass
* PostOrderFunctionAttrsPass
* CorrelatedValuePropagationPass
* TailCallElimPass
* ReassociatePass
* LICMPass
* LoopRotatePass
* IndVarSimplifyPass
* GVN
* JumpThreadingPass
* LoopVectorizePass
* LoopLoadEliminationPass
* LoopUnrollPass
* InstSimplifyPass

Missing:
* LoopSimplifyPass
* LCSSAPass

## Compilation Times (25 Run Average):
* .056300s (O2)
* .022304s (Active Passes)
* .022244s (InferFunctionAttrsPass Excluded)
* .022624s (SimplifyCFGPass Excluded)
* .024392s (SROA Excluded)
* .022504s (EarlyCSEPass Excluded)
* .022140s (GlobalOptPass Excluded)
* .022272s (InstCombinePass Excluded)
* .022144s (PostOrderFunctionAttrsPass Excluded)
* .022464s (CorrelatedValuePropagationPass Excluded)
* .022268s (TailCallElimPass Excluded)
* .022188s (ReassociatePass Excluded)
* .022324s (LoopSimplifyPass Excluded)
* .021048s (LICMPass Excluded)
* .018508s (LoopRotatePass Excluded)
* .016740s (IndVarSimplifyPass Excluded)
* .021260s (GVN Excluded)
* .021516s (JumpThreadingPass Excluded)
* .017704s (LoopVectorizePass Excluded)
* .018748s (LoopLoadEliminationPass Excluded)
* .018576s (LoopUnrollPass Excluded)
* .022300s (LCSSAPass Excluded)
* .022164s (InstSimplifyPass Excluded)

Largest Compilation Time Improvment: TODO

Smallest Compilation Time Improvement: TODO

# Runtime

## Runtime Times (5 Run Average, LARGE_DATASET):
* 0.56s (O0)
* 0.30s (O1)
* 0.30s (O2)
* 0.29s (O3)
* 0.31s (Active Passes)
* 0.34s (InferFunctionAttrsPass Excluded)
* 0.30s (SimplifyCFGPass Excluded)
* 0.32s (SROA Excluded)
* 0.30s (EarlyCSEPass Excluded)
* 0.30s (GlobalOptPass Excluded)
* 0.29s (InstCombinePass Excluded)
* 0.36s (PostOrderFunctionAttrsPass Excluded)
* 0.31s (CorrelatedValuePropagationPass Excluded)
* 0.32s (TailCallElimPass Excluded)
* 0.30s (ReassociatePass Excluded)
* 0.33s (LoopSimplifyPass Excluded)
* 0.29s (LICMPass Excluded)
* 0.29s (LoopRotatePass Excluded)
* 0.30s (IndVarSimplifyPass Excluded)
* 0.32s (GVN Excluded)
* 0.28s (JumpThreadingPass Excluded)
* 0.30s (LoopVectorizePass Excluded)
* 0.27s (LoopLoadEliminationPass Excluded)
* 0.32s (LoopUnrollPass Excluded)
* 0.30s (LCSSAPass Excluded)
* 0.28s (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: TODO

Smallest Runtime Time Cost: TODO

## Runtime Times (5 Run Average, EXTRALARGE_DATASET):
* 2.09s (O0)
* 1.23s (O1)
* 1.11s (O2)
* 1.23s (O3)
* 1.11s (Active Passes)
* 1.11s (InferFunctionAttrsPass Excluded)
* 1.11s (SimplifyCFGPass Excluded)
* 1.10s (SROA Excluded)
* 1.11s (EarlyCSEPass Excluded)
* 1.11s (GlobalOptPass Excluded)
* 1.11s (InstCombinePass Excluded)
* 1.11s (PostOrderFunctionAttrsPass Excluded)
* 1.11s (CorrelatedValuePropagationPass Excluded)
* 1.14s (TailCallElimPass Excluded)
* 1.11s (ReassociatePass Excluded)
* 1.11s (LoopSimplifyPass Excluded)
* 1.11s (LICMPass Excluded)
* 1.11s (LoopRotatePass Excluded)
* 1.11s (IndVarSimplifyPass Excluded)
* 1.11s (GVN Excluded)
* 1.11s (JumpThreadingPass Excluded)
* 1.11s (LoopVectorizePass Excluded)
* 1.11s (LoopLoadEliminationPass Excluded)
* 1.11s (LoopUnrollPass Excluded)
* 1.11s (LCSSAPass Excluded)
* 1.11s (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: TODO

Smallest Runtime Time Cost: TODO
