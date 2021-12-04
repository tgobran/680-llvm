# Compilation

## O2 Active Passes: 
1. SimplifyCFGPass (simplifycfg)
2. SROA (sroa)
3. EarlyCSEPass (early-cse)
4. GlobalOptPass (globalopt)
5. InstCombinePass (instcombine)
6. PostOrderFunctionAttrsPass (function-attrs)
7. ReassociatePass (reassociate)
8. LoopSimplifyPass (loop-simplify)
9. LoopRotatePass (loop-rotate)
10. LICMPass (licm)
11. IndVarSimplifyPass (indvars)
12. GVN (gvn)
13. JumpThreadingPass (jump-threading)
14. LCSSAPass (lcssa)
15. LoopVectorizePass (loop-vectorize)
16. LoopLoadEliminationPass (loop-load-elim)
17. LoopUnrollPass (loop-unroll)
18. InstSimplifyPass (instsimplify)

## Pruned (Only Active) Active Passes:
* SimplifyCFGPass
* SROA
* EarlyCSEPass
* GlobalOptPass
* InstCombinePass
* PostOrderFunctionAttrsPass
* ReassociatePass
* LoopRotatePass
* LICMPass
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
* .037752s (O2)
* .013804s 2.73x (Active Passes)
* .013912s 2.71x (SimplifyCFGPass Excluded)
* .017968s 2.10x (SROA Excluded)
* .013936s 2.71x (EarlyCSEPass Excluded)
* .013748s 2.75x (GlobalOptPass Excluded)
* .014048s 2.69x (InstCombinePass Excluded)
* .013660s 2.76x (PostOrderFunctionAttrsPass Excluded)
* .013716s 2.75x (ReassociatePass Excluded)
* .013784s 2.74x (LoopSimplifyPass Excluded)
* .010100s 3.74x (LoopRotatePass Excluded)
* .013592s 2.78x (LICMPass Excluded)
* .011144s 3.39x (IndVarSimplifyPass Excluded)
* .013844s 2.73x (GVN Excluded)
* .013640s 2.77x (JumpThreadingPass Excluded)
* .013820s 2.73x (LCSSAPass Excluded)
* .011216s 3.67x (LoopVectorizePass Excluded)
* .011724s 3.22x (LoopLoadEliminationPass Excluded)
* .011268s 3.35x (LoopUnrollPass Excluded)
* .013724s 2.75x (InstSimplifyPass Excluded)

Largest Compilation Time Improvment: 3.74x (LoopRotatePass Excluded)

Smallest Compilation Time Improvement: 2.10x (SROA Excluded)

# Runtime

## Runtime Times (5 Run Average, LARGE_DATASET):
* 6.49s (O2)
* 6.52s 1.00x (Active Passes)
* 6.50s 1.00x (SimplifyCFGPass Excluded)
* 6.49s 1.00x (SROA Excluded)
* 6.51s 1.00x (EarlyCSEPass Excluded)
* 6.51s 1.00x (GlobalOptPass Excluded)
* 6.48s 1.00x (InstCombinePass Excluded)
* 6.56s 0.99x (PostOrderFunctionAttrsPass Excluded)
* 6.51s 1.00x (ReassociatePass Excluded)
* 6.49s 1.00x (LoopSimplifyPass Excluded)
* 6.59s 0.98x (LoopRotatePass Excluded)
* 6.51s 1.00x (LICMPass Excluded)
* 6.55s 0.99x (IndVarSimplifyPass Excluded)
* 6.54s 0.99x (GVN Excluded)
* 6.49s 1.00x (JumpThreadingPass Excluded)
* 6.51s 1.00x (LCSSAPass Excluded)
* 6.52s 1.00x (LoopVectorizePass Excluded)
* 6.50s 1.00x (LoopLoadEliminationPass Excluded)
* 4.80s 1.35x (LoopUnrollPass Excluded)
* 6.58s 0.99x (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: 0.98x (LoopRotatePass Excluded)

Smallest Runtime Time Cost: 1.35x (LoopUnrollPass Excluded)

## Runtime Times (5 Run Average, EXTRALARGE_DATASET):
* TODO (O2)
* TODO (Active Passes)
* TODO (SimplifyCFGPass Excluded)
* TODO (SROA Excluded)
* TODO (EarlyCSEPass Excluded)
* TODO (GlobalOptPass Excluded)
* TODO (InstCombinePass Excluded)
* TODO (PostOrderFunctionAttrsPass Excluded)
* TODO (ReassociatePass Excluded)
* TODO (LoopSimplifyPass Excluded)
* TODO (LoopRotatePass Excluded)
* TODO (LICMPass Excluded)
* TODO (IndVarSimplifyPass Excluded)
* TODO (GVN Excluded)
* TODO (JumpThreadingPass Excluded)
* TODO (LCSSAPass Excluded)
* TODO (LoopVectorizePass Excluded)
* TODO (LoopLoadEliminationPass Excluded)
* TODO (LoopUnrollPass Excluded)
* TODO (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: TODO

Smallest Runtime Time Cost: TODO
