# Compilation

## O2 Active Passes: 
1. SimplifyCFGPass (simplifycfg)
2. SROA (sroa)
3. EarlyCSEPass (early-cse)
4. GlobalOptPass (globalopt)
5. InstCombinePass (instcombine)
6. PostOrderFunctionAttrsPass (function-attrs)
7. CorrelatedValuePropagationPass (correlated-propagation)
8. ReassociatePass (reassociate)
9. LoopSimplifyPass (loop-simplify)
10. LCSSAPass (lcssa)
11. LICMPass (licm)
12. LoopRotatePass (loop-rotate)
13. LoopInstSimplifyPass (loop-instsimplify)
14. IndVarSimplifyPass (indvars)
15. GVN (gvn)
16. JumpThreadingPass (jump-threading)
17. LoopLoadEliminationPass (loop-load-elim)
18. SLPVectorizerPass (slp-vectorizer)
19. LoopUnrollPass (loop-unroll)

## Pruned (Only Active) Active Passes:
* SimplifyCFGPass
* SROA
* EarlyCSEPass
* GlobalOptPass
* InstCombinePass
* PostOrderFunctionAttrsPass
* CorrelatedValuePropagationPass
* ReassociatePass
* LCSSAPass
* LICMPass
* LoopRotatePass
* LoopInstSimplifyPass
* IndVarSimplifyPass
* GVN
* JumpThreadingPass
* LoopLoadEliminationPass
* SLPVectorizerPass

Missing:
* LoopSimplifyPass
* LoopUnrollPass

## Compilation Times (25 Run Average):
* .046772s (O2)
* .016776s 2.79x (Active Passes)
* .017020s 2.75x (SimplifyCFGPass Excluded)
* .020124s 2.32x (SROA Excluded)
* .017024s 2.75x (EarlyCSEPass Excluded)
* .016752s 2.79x (GlobalOptPass Excluded)
* .019080s 2.45x (InstCombinePass Excluded)
* .016628s 2.81x (PostOrderFunctionAttrsPass Excluded)
* .019248s 2.43x (CorrelatedValuePropagationPass Excluded)
* .016696s 2.80x (ReassociatePass Excluded)
* .016820s 2.78x (LoopSimplifyPass Excluded)
* .016804s 2.78x (LCSSAPass Excluded)
* .016532s 2.83x (LICMPass Excluded)
* .012524s 3.73x (LoopRotatePass Excluded)
* .016792s 2.76x (LoopInstSimplifyPass Excluded)
* .015236s 3.07x (IndVarSimplifyPass Excluded)
* .016168s 2.89x (GVN Excluded)
* .016464s 2.84x (JumpThreadingPass Excluded)
* .012032s 3.89x (LoopLoadEliminationPass Excluded)
* .015876s 2.95x (SLPVectorizerPass Excluded)
* .016044s 2.92x (LoopUnrollPass Excluded)

Largest Compilation Time Improvment: 3.89x (LoopLoadEliminationPass Excluded)

Smallest Compilation Time Improvement: 2.32x (SROA Excluded)

# Runtime

## Runtime Times (5 Run Average, LARGE_DATASET):
* 16.22s (O2)
* 16.16s 1.00x (Active Passes)
* 16.18s 1.00x (SimplifyCFGPass Excluded)
* 16.16s 1.00x (SROA Excluded)
* 16.17s 1.00x (EarlyCSEPass Excluded)
* 16.15s 1.00x (GlobalOptPass Excluded)
* 14.91s 1.09x (InstCombinePass Excluded)
* 16.15s 1.00x (PostOrderFunctionAttrsPass Excluded)
* 14.91s 1.09x (CorrelatedValuePropagationPass Excluded)
* 16.17s 1.00x (ReassociatePass Excluded)
* 16.16s 1.00x (LoopSimplifyPass Excluded)
* 16.18s 1.00x (LCSSAPass Excluded)
* 16.32s 0.99x (LICMPass Excluded)
* 20.35s 0.80x (LoopRotatePass Excluded)
* 16.19s 1.00x (LoopInstSimplifyPass Excluded)
* 14.15s 1.15x (IndVarSimplifyPass Excluded)
* 16.18s 1.00x (GVN Excluded)
* 16.18s 1.00x (JumpThreadingPass Excluded)
* 20.36s 0.80x (LoopLoadEliminationPass Excluded)
* 13.58s 1.19x (SLPVectorizerPass Excluded)
* 16.16s 1.00x (LoopUnrollPass Excluded)
 
Largest Runtime Time Cost: 0.80x (LoopLoadEliminationPass Excluded)

Smallest Runtime Time Cost: 1.19x (SLPVectorizerPass Excluded)

## Runtime Times (5 Run Average, EXTRALARGE_DATASET):
* TODO (O2)
* TODO (Active Passes)
* TODO (SimplifyCFGPass Excluded)
* TODO (SROA Excluded)
* TODO (EarlyCSEPass Excluded)
* TODO (GlobalOptPass Excluded)
* TODO (InstCombinePass Excluded)
* TODO (PostOrderFunctionAttrsPass Excluded)
* TODO (CorrelatedValuePropagationPass Excluded)
* TODO (ReassociatePass Excluded)
* TODO (LoopSimplifyPass Excluded)
* TODO (LCSSAPass Excluded)
* TODO (LICMPass Excluded)
* TODO (LoopRotatePass Excluded)
* TODO (LoopInstSimplifyPass Excluded)
* TODO (IndVarSimplifyPass Excluded)
* TODO (GVN Excluded)
* TODO (JumpThreadingPass Excluded)
* TODO (LoopLoadEliminationPass Excluded)
* TODO (SLPVectorizerPass Excluded)
* TODO (LoopUnrollPass Excluded)
 
Largest Runtime Time Cost: TODO

Smallest Runtime Time Cost: TODO
