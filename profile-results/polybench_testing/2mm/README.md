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
