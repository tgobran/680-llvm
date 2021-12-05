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
* TODO (O2)
* TODO (Active Passes)
* TODO (InferFunctionAttrsPass Excluded)
* TODO (SimplifyCFGPass Excluded)
* TODO (SROA Excluded)
* TODO (EarlyCSEPass Excluded)
* TODO (GlobalOptPass Excluded)
* TODO (InstCombinePass Excluded)
* TODO (PostOrderFunctionAttrsPass Excluded)
* TODO (CorrelatedValuePropagationPass Excluded)
* TODO (TailCallElimPass Excluded)
* TODO (ReassociatePass Excluded)
* TODO (LoopSimplifyPass Excluded)
* TODO (LICMPass Excluded)
* TODO (LoopRotatePass Excluded)
* TODO (IndVarSimplifyPass Excluded)
* TODO (GVN Excluded)
* TODO (JumpThreadingPass Excluded)
* TODO (LoopVectorizePass Excluded)
* TODO (LoopLoadEliminationPass Excluded)
* TODO (LoopUnrollPass Excluded)
* TODO (LCSSAPass Excluded)
* TODO (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: TODO

Smallest Runtime Time Cost: TODO

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
* TODO (CorrelatedValuePropagationPass Excluded)
* TODO (TailCallElimPass Excluded)
* TODO (ReassociatePass Excluded)
* TODO (LoopSimplifyPass Excluded)
* TODO (LICMPass Excluded)
* TODO (LoopRotatePass Excluded)
* TODO (IndVarSimplifyPass Excluded)
* TODO (GVN Excluded)
* TODO (JumpThreadingPass Excluded)
* TODO (LoopVectorizePass Excluded)
* TODO (LoopLoadEliminationPass Excluded)
* TODO (LoopUnrollPass Excluded)
* TODO (LCSSAPass Excluded)
* TODO (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: TODO

Smallest Runtime Time Cost: TODO
