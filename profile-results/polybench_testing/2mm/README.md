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
14. LoopUnrollPass (loop-unroll)
15. LCSSAPass (lcssa)
16. InstSimplifyPass (instsimplify)

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
* LoopUnrollPass
* InstSimplifyPass

Missing:
* LCSSAPass
* LoopSimplifyPass

## Compilation Times (25 Run Average):
* .028696s (O2)
* .008220s  3.49x (Active Passes)
* .008392s  3.42x (SimplifyCFGPass Excluded)
* .011704s  2.45x (SROA Excluded)
* .007760s  3.70x (EarlyCSEPass Excluded)
* .008160s  3.52x (GlobalOptPass Excluded)
* .008508s  3.37x (InstCombinePass Excluded)
* .008128s  3.53x (PostOrderFunctionAttrsPass Excluded)
* .008152s  3.52x (ReassociatePass Excluded)
* .008272s  3.47x (LoopSimplifyPass Excluded)
* .007820s  3.67x (LoopRotatePass Excluded)
* .008072s  3.56x (LICM Excluded) 
* .006112s  4.70x (IndVarSimplifyPass Excluded)
* .007748s  3.70x (GVN Excluded)
* .007924s  3.62x (JumpThreadingPass Excluded)
* .006508s  4.41x (LoopUnrollPass Excluded)
* .008268s  3.47x (LCSSA Excluded)
* .008176s  3.51x (InstSimplifyPass Excluded)

Largest Compilation Time Improvment: 4.70x (IndVarSimplifyPass Excluded)

Smallest Compilation Time Improvement: 2.45x (SROA Excluded)

# Runtime

## Runtime Times (5 Run Average, LARGE_DATASET):
* 2.08s (O2)
* 2.58s 0.81x (Active Passes)
* 2.50s 0.83x (SimplifyCFGPass Excluded)
* 2.63s 0.79x (SROA Excluded)
* 2.42s 0.86x (EarlyCSEPass Excluded)
* 2.43s 0.86x (GlobalOptPass Excluded)
* 2.51s 0.83x (InstCombinePass Excluded)
* 2.40s 0.86x (PostOrderFunctionAttrsPass Excluded)
* 2.58s 0.81x (ReassociatePass Excluded)
* 2.48s 0.84x (LoopSimplifyPass Excluded)
* 2.64s 0.79x (LoopRotatePass Excluded)
* 2.43s 0.86x (LICM Excluded) 
* 2.84s 0.73x (IndVarSimplifyPass Excluded)
* 4.63s 0.45x (GVN Excluded)
* 2.46s 0.85x (JumpThreadingPass Excluded)
* 2.68s 0.78x (LoopUnrollPass Excluded)
* 2.59s 0.80x (LCSSA Excluded)
* 2.48s 0.84x (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: 0.45x (GVN Excluded)

Smallest Runtime Time Cost: 0.86x (PostOrderFunctionAttrsPass Excluded)

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
* TODO (LICM Excluded) 
* TODO (IndVarSimplifyPass Excluded)
* TODO (GVN Excluded)
* TODO (JumpThreadingPass Excluded)
* TODO (LoopUnrollPass Excluded)
* TODO (LCSSA Excluded)
* TODO (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: TODO

Smallest Runtime Time Cost: TODO
