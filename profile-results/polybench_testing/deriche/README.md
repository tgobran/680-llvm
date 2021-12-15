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

Didn't make changes when only pruned are run:
* LoopSimplifyPass
* LCSSAPass

## Compilation Times (25 Run Average):
* .054964s (O2)
* .022304s 2.46x (Active Passes)
* .022244s 2.70x (InferFunctionAttrsPass Excluded)
* .022624s 2.43x (SimplifyCFGPass Excluded)
* .024392s 2.25x (SROA Excluded)
* .022504s 2.44x (EarlyCSEPass Excluded)
* .022140s 2.48x (GlobalOptPass Excluded)
* .022272s 2.47x (InstCombinePass Excluded)
* .022144s 2.48x (PostOrderFunctionAttrsPass Excluded)
* .022464s 2.45x (CorrelatedValuePropagationPass Excluded)
* .022268s 2.47x (TailCallElimPass Excluded)
* .022188s 2.48x (ReassociatePass Excluded)
* .022324s 2.46x (LoopSimplifyPass Excluded)
* .021048s 2.61x (LICMPass Excluded)
* .018508s 2.97x (LoopRotatePass Excluded)
* .016740s 3.28x (IndVarSimplifyPass Excluded)
* .021260s 2.59x (GVN Excluded)
* .021516s 2.55x (JumpThreadingPass Excluded)
* .017704s 3.10x (LoopVectorizePass Excluded)
* .018748s 2.93x (LoopLoadEliminationPass Excluded)
* .018576s 2.96x (LoopUnrollPass Excluded)
* .022300s 2.46x (LCSSAPass Excluded)
* .022164s 2.48x (InstSimplifyPass Excluded)

Largest Compilation Time Improvment: 3.28x (IndVarSimplifyPass Excluded)

Smallest Compilation Time Improvement: 2.25x (SROA Excluded)

# Runtime

## Runtime Times (5 Run Average, LARGE_DATASET):
* 0.30s (O2)
* 0.31s 0.97x (Active Passes)
* 0.34s 0.88x (InferFunctionAttrsPass Excluded)
* 0.30s 1.00x (SimplifyCFGPass Excluded)
* 0.32s 0.94x (SROA Excluded)
* 0.30s 1.00x (EarlyCSEPass Excluded)
* 0.30s 1.00x (GlobalOptPass Excluded)
* 0.29s 1.03x (InstCombinePass Excluded)
* 0.36s 0.83x (PostOrderFunctionAttrsPass Excluded)
* 0.31s 0.97x (CorrelatedValuePropagationPass Excluded)
* 0.32s 0.94x (TailCallElimPass Excluded)
* 0.30s 1.00x (ReassociatePass Excluded)
* 0.33s 0.91x (LoopSimplifyPass Excluded)
* 0.29s 1.03x (LICMPass Excluded)
* 0.29s 1.03x (LoopRotatePass Excluded)
* 0.30s 1.00x (IndVarSimplifyPass Excluded)
* 0.32s 0.94x (GVN Excluded)
* 0.28s 1.07x (JumpThreadingPass Excluded)
* 0.30s 1.00x (LoopVectorizePass Excluded)
* 0.27s 1.11x (LoopLoadEliminationPass Excluded)
* 0.32s 0.94x (LoopUnrollPass Excluded)
* 0.30s 1.00x (LCSSAPass Excluded)
* 0.28s 1.07x (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: 0.83x (PostOrderFunctionAttrsPass Excluded)

Smallest Runtime Time Cost: 1.11x (LoopLoadEliminationPass Excluded)

## Runtime Times (5 Run Average, EXTRALARGE_DATASET):
* 1.11s (O2)
* 1.11s 1.00x (Active Passes)
* 1.11s 1.00x (InferFunctionAttrsPass Excluded)
* 1.11s 1.00x (SimplifyCFGPass Excluded)
* 1.10s 1.01x (SROA Excluded)
* 1.11s 1.00x (EarlyCSEPass Excluded)
* 1.11s 1.00x (GlobalOptPass Excluded)
* 1.11s 1.00x (InstCombinePass Excluded)
* 1.11s 1.00x (PostOrderFunctionAttrsPass Excluded)
* 1.11s 1.00x (CorrelatedValuePropagationPass Excluded)
* 1.14s 0.97x (TailCallElimPass Excluded)
* 1.11s 1.00x (ReassociatePass Excluded)
* 1.11s 1.00x (LoopSimplifyPass Excluded)
* 1.11s 1.00x (LICMPass Excluded)
* 1.11s 1.00x (LoopRotatePass Excluded)
* 1.11s 1.00x (IndVarSimplifyPass Excluded)
* 1.11s 1.00x (GVN Excluded)
* 1.11s 1.00x (JumpThreadingPass Excluded)
* 1.11s 1.00x (LoopVectorizePass Excluded)
* 1.11s 1.00x (LoopLoadEliminationPass Excluded)
* 1.11s 1.00x (LoopUnrollPass Excluded)
* 1.11s 1.00x (LCSSAPass Excluded)
* 1.11s 1.00x (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: 0.97x (TailCallElimPass Excluded)

Smallest Runtime Time Cost: 1.01x (SROA Excluded)
