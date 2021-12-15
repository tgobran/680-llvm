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

Didn't make changes when only pruned are run:
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
* 93.73s (O2)
* 93.91s 1.00x (Active Passes)
* 93.53s 1.00x (SimplifyCFGPass Excluded)
* 93.66s 1.00x (SROA Excluded)
* 93.44s 1.00x (EarlyCSEPass Excluded)
* 93.59s 1.00x (GlobalOptPass Excluded)
* 93.80s 1.00x (InstCombinePass Excluded)
* 93.29s 1.00x (PostOrderFunctionAttrsPass Excluded)
* 93.47s 1.00x (ReassociatePass Excluded)
* 93.65s 1.00x (LoopSimplifyPass Excluded)
* 94.63s 0.99x (LoopRotatePass Excluded)
* 93.68s 1.00x (LICMPass Excluded)
* 94.21s 0.99x (IndVarSimplifyPass Excluded)
* 93.33s 1.00x (GVN Excluded)
* 93.47s 1.00x (JumpThreadingPass Excluded)
* 93.42s 1.00x (LCSSAPass Excluded)
* 93.50s 1.00x (LoopVectorizePass Excluded)
* 93.68s 1.00x (LoopLoadEliminationPass Excluded)
* 68.99s 1.36x (LoopUnrollPass Excluded)
* 93.80s 1.00x (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: 0.99x (LoopRotatePass Excluded)

Smallest Runtime Time Cost: 1.36x (LoopUnrollPass Excluded)
