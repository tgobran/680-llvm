# Compilation

All Ratios are done in comparison with the O2 results.

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

Didn't make changes when only pruned are run:
* LCSSAPass
* LoopSimplifyPass

## Compilation Times (25 Run Average):
* .025252s (O2) Ratio Reference
* .000508s 49.70x (O0)
* .025256s  1.00x (O1)
* .049700s  0.51x (O3)
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
* 2.07s (O2) Ratio Reference
* 8.87s 0.23x (O0)
* 4.38s 0.47x (O1)
* 2.07s 1.00x (O3)
* 2.07s 1.00x (Active Passes)
* 2.08s 1.00x (SimplifyCFGPass Excluded)
* 2.29s 0.90x (SROA Excluded)
* 2.07s 1.00x (EarlyCSEPass Excluded)
* 2.07s 1.00x (GlobalOptPass Excluded)
* 2.08s 1.00x (InstCombinePass Excluded)
* 2.07s 1.00x (PostOrderFunctionAttrsPass Excluded)
* 2.08s 1.00x (ReassociatePass Excluded)
* 2.07s 1.00x (LoopSimplifyPass Excluded)
* 2.15s 0.96x (LoopRotatePass Excluded)
* 2.07s 1.00x (LICMPass Excluded) 
* 2.23s 0.93x (IndVarSimplifyPass Excluded)
* 4.38s 0.47x (GVN Excluded)
* 2.06s 1.00x (JumpThreadingPass Excluded)
* 2.15s 0.96x (LoopUnrollPass Excluded)
* 2.07s 1.00x (LCSSAPass Excluded)
* 2.07s 1.00x (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: 0.47x (GVN Excluded)

Smallest Runtime Time Cost: 1.00x (JumpThreadingPass Excluded)

## Runtime Times (5 Run Average, EXTRALARGE_DATASET):
* 67.66s (O2) Ratio Reference
* 101.69s 0.67x (O0)
* 66.59s  1.02x (O1)
* 67.29s  1.01x (O3)
* 67.30s  1.01x (Active Passes)
* 66.61s  1.02x (SimplifyCFGPass Excluded)
* 69.67s  0.97x (SROA Excluded)
* 66.69s  1.01x (EarlyCSEPass Excluded)
* 67.00s  1.01x (GlobalOptPass Excluded)
* 66.59s  1.02x (InstCombinePass Excluded)
* 66.74s  1.01x (PostOrderFunctionAttrsPass Excluded)
* 67.84s  1.00x (ReassociatePass Excluded)
* 66.78s  1.01x (LoopSimplifyPass Excluded)
* 67.48s  1.00x (LoopRotatePass Excluded)
* 66.83s  1.01x (LICM Excluded) 
* 70.01s  0.97x (IndVarSimplifyPass Excluded)
* 67.29s  1.01x (GVN Excluded)
* 66.64s  1.02x (JumpThreadingPass Excluded)
* 68.77s  0.98x (LoopUnrollPass Excluded)
* 68.06s  0.99x (LCSSA Excluded)
* 67.59s  1.00x (InstSimplifyPass Excluded)
 
Largest Runtime Time Cost: 0.97x (IndVarSimplifyPass Excluded)

Smallest Runtime Time Cost: 1.02x (SimplifyCFGPass Excluded)
