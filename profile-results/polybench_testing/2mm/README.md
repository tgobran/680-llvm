# Compilation

## O2 Active Passes: 

* EarlyCSEPass (early-cse)
* GVN (gvn)
* GlobalOptPass (globalopt)
* IndVarSimplifyPass (indvars)
* InstCombinePass (instcombine)
* InstSimplifyPass (instsimplify)
* JumpThreadingPass (jump-threading)
* LCSSAPass (lcssa)
* LICMPass (licm)
* LoopRotatePass (loop-rotate)
* LoopSimplifyPass (loop-simplify)
* LoopUnrollPass (loop-unroll)
* PostOrderFunctionAttrsPass (function-attrs)
* ReassociatePass (reassociate)
* SROA (sroa)
* SimplifyCFGPass (simplifycfg)

## Pruned (Only Active) Active Passes:
* EarlyCSEPass
* GVN
* GlobalOptPass
* IndVarSimplifyPass
* InstCombinePass
* InstSimplifyPass
* JumpThreadingPass
* LICMPass
* LoopRotatePass
* LoopUnrollPass
* PostOrderFunctionAttrsPass
* ReassociatePass
* SROA
* SimplifyCFGPass

Missing:
* LCSSAPass
* LoopSimplifyPass

## Compilation Times:
* O2 Time: .028520s
* O2 Active Time: .008280s
* 
