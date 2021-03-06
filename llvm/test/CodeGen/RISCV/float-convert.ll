; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=riscv32 -mattr=+f -verify-machineinstrs < %s \
; RUN:   | FileCheck -check-prefix=RV32IF %s
; RUN: llc -mtriple=riscv64 -mattr=+f -verify-machineinstrs < %s \
; RUN:   | FileCheck -check-prefix=RV64IF %s

; For RV64F, fcvt.l.s is semantically equivalent to fcvt.w.s in this case
; because fptosi will produce poison if the result doesn't fit into an i32.
define i32 @fcvt_w_s(float %a) nounwind {
; RV32IF-LABEL: fcvt_w_s:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    fmv.w.x ft0, a0
; RV32IF-NEXT:    fcvt.w.s a0, ft0, rtz
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_w_s:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fmv.w.x ft0, a0
; RV64IF-NEXT:    fcvt.w.s a0, ft0, rtz
; RV64IF-NEXT:    ret
  %1 = fptosi float %a to i32
  ret i32 %1
}

define i32 @fcvt_w_s_sat(float %a) nounwind {
; RV32IF-LABEL: fcvt_w_s_sat:
; RV32IF:       # %bb.0: # %start
; RV32IF-NEXT:    fmv.w.x ft0, a0
; RV32IF-NEXT:    feq.s a0, ft0, ft0
; RV32IF-NEXT:    bnez a0, .LBB1_2
; RV32IF-NEXT:  # %bb.1: # %start
; RV32IF-NEXT:    mv a0, zero
; RV32IF-NEXT:    ret
; RV32IF-NEXT:  .LBB1_2:
; RV32IF-NEXT:    fcvt.w.s a0, ft0, rtz
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_w_s_sat:
; RV64IF:       # %bb.0: # %start
; RV64IF-NEXT:    fmv.w.x ft0, a0
; RV64IF-NEXT:    feq.s a0, ft0, ft0
; RV64IF-NEXT:    bnez a0, .LBB1_2
; RV64IF-NEXT:  # %bb.1: # %start
; RV64IF-NEXT:    mv a0, zero
; RV64IF-NEXT:    ret
; RV64IF-NEXT:  .LBB1_2:
; RV64IF-NEXT:    fcvt.w.s a0, ft0, rtz
; RV64IF-NEXT:    ret
start:
  %0 = tail call i32 @llvm.fptosi.sat.i32.f32(float %a)
  ret i32 %0
}
declare i32 @llvm.fptosi.sat.i32.f32(float)

; For RV64F, fcvt.lu.s is semantically equivalent to fcvt.wu.s in this case
; because fptoui will produce poison if the result doesn't fit into an i32.
define i32 @fcvt_wu_s(float %a) nounwind {
; RV32IF-LABEL: fcvt_wu_s:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    fmv.w.x ft0, a0
; RV32IF-NEXT:    fcvt.wu.s a0, ft0, rtz
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_wu_s:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fmv.w.x ft0, a0
; RV64IF-NEXT:    fcvt.wu.s a0, ft0, rtz
; RV64IF-NEXT:    ret
  %1 = fptoui float %a to i32
  ret i32 %1
}

; Test where the fptoui has multiple uses, one of which causes a sext to be
; inserted on RV64.
define i32 @fcvt_wu_s_multiple_use(float %x, i32* %y) {
; RV32IF-LABEL: fcvt_wu_s_multiple_use:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    fmv.w.x ft0, a0
; RV32IF-NEXT:    fcvt.wu.s a1, ft0, rtz
; RV32IF-NEXT:    addi a0, zero, 1
; RV32IF-NEXT:    beqz a1, .LBB3_2
; RV32IF-NEXT:  # %bb.1:
; RV32IF-NEXT:    mv a0, a1
; RV32IF-NEXT:  .LBB3_2:
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_wu_s_multiple_use:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fmv.w.x ft0, a0
; RV64IF-NEXT:    fcvt.wu.s a1, ft0, rtz
; RV64IF-NEXT:    addi a0, zero, 1
; RV64IF-NEXT:    beqz a1, .LBB3_2
; RV64IF-NEXT:  # %bb.1:
; RV64IF-NEXT:    mv a0, a1
; RV64IF-NEXT:  .LBB3_2:
; RV64IF-NEXT:    ret
  %a = fptoui float %x to i32
  %b = icmp eq i32 %a, 0
  %c = select i1 %b, i32 1, i32 %a
  ret i32 %c
}

define i32 @fcvt_wu_s_sat(float %a) nounwind {
; RV32IF-LABEL: fcvt_wu_s_sat:
; RV32IF:       # %bb.0: # %start
; RV32IF-NEXT:    fmv.w.x ft0, a0
; RV32IF-NEXT:    feq.s a0, ft0, ft0
; RV32IF-NEXT:    bnez a0, .LBB4_2
; RV32IF-NEXT:  # %bb.1: # %start
; RV32IF-NEXT:    mv a0, zero
; RV32IF-NEXT:    ret
; RV32IF-NEXT:  .LBB4_2:
; RV32IF-NEXT:    fcvt.wu.s a0, ft0, rtz
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_wu_s_sat:
; RV64IF:       # %bb.0: # %start
; RV64IF-NEXT:    fmv.w.x ft0, a0
; RV64IF-NEXT:    feq.s a0, ft0, ft0
; RV64IF-NEXT:    bnez a0, .LBB4_2
; RV64IF-NEXT:  # %bb.1: # %start
; RV64IF-NEXT:    mv a0, zero
; RV64IF-NEXT:    ret
; RV64IF-NEXT:  .LBB4_2:
; RV64IF-NEXT:    fcvt.wu.s a0, ft0, rtz
; RV64IF-NEXT:    ret
start:
  %0 = tail call i32 @llvm.fptoui.sat.i32.f32(float %a)
  ret i32 %0
}
declare i32 @llvm.fptoui.sat.i32.f32(float)

define i32 @fmv_x_w(float %a, float %b) nounwind {
; RV32IF-LABEL: fmv_x_w:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    fmv.w.x ft0, a1
; RV32IF-NEXT:    fmv.w.x ft1, a0
; RV32IF-NEXT:    fadd.s ft0, ft1, ft0
; RV32IF-NEXT:    fmv.x.w a0, ft0
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fmv_x_w:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fmv.w.x ft0, a1
; RV64IF-NEXT:    fmv.w.x ft1, a0
; RV64IF-NEXT:    fadd.s ft0, ft1, ft0
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
; Ensure fmv.x.w is generated even for a soft float calling convention
  %1 = fadd float %a, %b
  %2 = bitcast float %1 to i32
  ret i32 %2
}

define float @fcvt_s_w(i32 %a) nounwind {
; RV32IF-LABEL: fcvt_s_w:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    fcvt.s.w ft0, a0
; RV32IF-NEXT:    fmv.x.w a0, ft0
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_w:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fcvt.s.w ft0, a0
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
  %1 = sitofp i32 %a to float
  ret float %1
}

define float @fcvt_s_w_load(i32* %p) nounwind {
; RV32IF-LABEL: fcvt_s_w_load:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    lw a0, 0(a0)
; RV32IF-NEXT:    fcvt.s.w ft0, a0
; RV32IF-NEXT:    fmv.x.w a0, ft0
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_w_load:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    lw a0, 0(a0)
; RV64IF-NEXT:    fcvt.s.w ft0, a0
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
  %a = load i32, i32* %p
  %1 = sitofp i32 %a to float
  ret float %1
}

define float @fcvt_s_wu(i32 %a) nounwind {
; RV32IF-LABEL: fcvt_s_wu:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    fcvt.s.wu ft0, a0
; RV32IF-NEXT:    fmv.x.w a0, ft0
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_wu:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fcvt.s.wu ft0, a0
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
  %1 = uitofp i32 %a to float
  ret float %1
}

define float @fcvt_s_wu_load(i32* %p) nounwind {
; RV32IF-LABEL: fcvt_s_wu_load:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    lw a0, 0(a0)
; RV32IF-NEXT:    fcvt.s.wu ft0, a0
; RV32IF-NEXT:    fmv.x.w a0, ft0
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_wu_load:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    lwu a0, 0(a0)
; RV64IF-NEXT:    fcvt.s.wu ft0, a0
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
  %a = load i32, i32* %p
  %1 = uitofp i32 %a to float
  ret float %1
}

define float @fmv_w_x(i32 %a, i32 %b) nounwind {
; RV32IF-LABEL: fmv_w_x:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    fmv.w.x ft0, a0
; RV32IF-NEXT:    fmv.w.x ft1, a1
; RV32IF-NEXT:    fadd.s ft0, ft0, ft1
; RV32IF-NEXT:    fmv.x.w a0, ft0
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fmv_w_x:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fmv.w.x ft0, a0
; RV64IF-NEXT:    fmv.w.x ft1, a1
; RV64IF-NEXT:    fadd.s ft0, ft0, ft1
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
; Ensure fmv.w.x is generated even for a soft float calling convention
  %1 = bitcast i32 %a to float
  %2 = bitcast i32 %b to float
  %3 = fadd float %1, %2
  ret float %3
}

define i64 @fcvt_l_s(float %a) nounwind {
; RV32IF-LABEL: fcvt_l_s:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    addi sp, sp, -16
; RV32IF-NEXT:    sw ra, 12(sp) # 4-byte Folded Spill
; RV32IF-NEXT:    call __fixsfdi@plt
; RV32IF-NEXT:    lw ra, 12(sp) # 4-byte Folded Reload
; RV32IF-NEXT:    addi sp, sp, 16
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_l_s:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fmv.w.x ft0, a0
; RV64IF-NEXT:    fcvt.l.s a0, ft0, rtz
; RV64IF-NEXT:    ret
  %1 = fptosi float %a to i64
  ret i64 %1
}

define i64 @fcvt_l_s_sat(float %a) nounwind {
; RV32IF-LABEL: fcvt_l_s_sat:
; RV32IF:       # %bb.0: # %start
; RV32IF-NEXT:    addi sp, sp, -16
; RV32IF-NEXT:    sw ra, 12(sp) # 4-byte Folded Spill
; RV32IF-NEXT:    sw s0, 8(sp) # 4-byte Folded Spill
; RV32IF-NEXT:    lui a1, %hi(.LCPI12_0)
; RV32IF-NEXT:    flw ft0, %lo(.LCPI12_0)(a1)
; RV32IF-NEXT:    fmv.w.x ft1, a0
; RV32IF-NEXT:    fsw ft1, 4(sp) # 4-byte Folded Spill
; RV32IF-NEXT:    fle.s s0, ft0, ft1
; RV32IF-NEXT:    call __fixsfdi@plt
; RV32IF-NEXT:    mv a2, a0
; RV32IF-NEXT:    bnez s0, .LBB12_2
; RV32IF-NEXT:  # %bb.1: # %start
; RV32IF-NEXT:    mv a2, zero
; RV32IF-NEXT:  .LBB12_2: # %start
; RV32IF-NEXT:    lui a0, %hi(.LCPI12_1)
; RV32IF-NEXT:    flw ft0, %lo(.LCPI12_1)(a0)
; RV32IF-NEXT:    flw ft1, 4(sp) # 4-byte Folded Reload
; RV32IF-NEXT:    flt.s a3, ft0, ft1
; RV32IF-NEXT:    fmv.s ft0, ft1
; RV32IF-NEXT:    addi a0, zero, -1
; RV32IF-NEXT:    beqz a3, .LBB12_9
; RV32IF-NEXT:  # %bb.3: # %start
; RV32IF-NEXT:    feq.s a2, ft0, ft0
; RV32IF-NEXT:    beqz a2, .LBB12_10
; RV32IF-NEXT:  .LBB12_4: # %start
; RV32IF-NEXT:    lui a4, 524288
; RV32IF-NEXT:    beqz s0, .LBB12_11
; RV32IF-NEXT:  .LBB12_5: # %start
; RV32IF-NEXT:    bnez a3, .LBB12_12
; RV32IF-NEXT:  .LBB12_6: # %start
; RV32IF-NEXT:    bnez a2, .LBB12_8
; RV32IF-NEXT:  .LBB12_7: # %start
; RV32IF-NEXT:    mv a1, zero
; RV32IF-NEXT:  .LBB12_8: # %start
; RV32IF-NEXT:    lw s0, 8(sp) # 4-byte Folded Reload
; RV32IF-NEXT:    lw ra, 12(sp) # 4-byte Folded Reload
; RV32IF-NEXT:    addi sp, sp, 16
; RV32IF-NEXT:    ret
; RV32IF-NEXT:  .LBB12_9: # %start
; RV32IF-NEXT:    mv a0, a2
; RV32IF-NEXT:    feq.s a2, ft0, ft0
; RV32IF-NEXT:    bnez a2, .LBB12_4
; RV32IF-NEXT:  .LBB12_10: # %start
; RV32IF-NEXT:    mv a0, zero
; RV32IF-NEXT:    lui a4, 524288
; RV32IF-NEXT:    bnez s0, .LBB12_5
; RV32IF-NEXT:  .LBB12_11: # %start
; RV32IF-NEXT:    lui a1, 524288
; RV32IF-NEXT:    beqz a3, .LBB12_6
; RV32IF-NEXT:  .LBB12_12:
; RV32IF-NEXT:    addi a1, a4, -1
; RV32IF-NEXT:    beqz a2, .LBB12_7
; RV32IF-NEXT:    j .LBB12_8
;
; RV64IF-LABEL: fcvt_l_s_sat:
; RV64IF:       # %bb.0: # %start
; RV64IF-NEXT:    fmv.w.x ft0, a0
; RV64IF-NEXT:    feq.s a0, ft0, ft0
; RV64IF-NEXT:    bnez a0, .LBB12_2
; RV64IF-NEXT:  # %bb.1: # %start
; RV64IF-NEXT:    mv a0, zero
; RV64IF-NEXT:    ret
; RV64IF-NEXT:  .LBB12_2:
; RV64IF-NEXT:    fcvt.l.s a0, ft0, rtz
; RV64IF-NEXT:    ret
start:
  %0 = tail call i64 @llvm.fptosi.sat.i64.f32(float %a)
  ret i64 %0
}
declare i64 @llvm.fptosi.sat.i64.f32(float)

define i64 @fcvt_lu_s(float %a) nounwind {
; RV32IF-LABEL: fcvt_lu_s:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    addi sp, sp, -16
; RV32IF-NEXT:    sw ra, 12(sp) # 4-byte Folded Spill
; RV32IF-NEXT:    call __fixunssfdi@plt
; RV32IF-NEXT:    lw ra, 12(sp) # 4-byte Folded Reload
; RV32IF-NEXT:    addi sp, sp, 16
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_lu_s:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fmv.w.x ft0, a0
; RV64IF-NEXT:    fcvt.lu.s a0, ft0, rtz
; RV64IF-NEXT:    ret
  %1 = fptoui float %a to i64
  ret i64 %1
}

define i64 @fcvt_lu_s_sat(float %a) nounwind {
; RV32IF-LABEL: fcvt_lu_s_sat:
; RV32IF:       # %bb.0: # %start
; RV32IF-NEXT:    addi sp, sp, -16
; RV32IF-NEXT:    sw ra, 12(sp) # 4-byte Folded Spill
; RV32IF-NEXT:    sw s0, 8(sp) # 4-byte Folded Spill
; RV32IF-NEXT:    fmv.w.x ft1, a0
; RV32IF-NEXT:    fmv.w.x ft0, zero
; RV32IF-NEXT:    fsw ft1, 4(sp) # 4-byte Folded Spill
; RV32IF-NEXT:    fle.s s0, ft0, ft1
; RV32IF-NEXT:    call __fixunssfdi@plt
; RV32IF-NEXT:    mv a3, a0
; RV32IF-NEXT:    bnez s0, .LBB14_2
; RV32IF-NEXT:  # %bb.1: # %start
; RV32IF-NEXT:    mv a3, zero
; RV32IF-NEXT:  .LBB14_2: # %start
; RV32IF-NEXT:    lui a0, %hi(.LCPI14_0)
; RV32IF-NEXT:    flw ft0, %lo(.LCPI14_0)(a0)
; RV32IF-NEXT:    flw ft1, 4(sp) # 4-byte Folded Reload
; RV32IF-NEXT:    flt.s a4, ft0, ft1
; RV32IF-NEXT:    addi a2, zero, -1
; RV32IF-NEXT:    addi a0, zero, -1
; RV32IF-NEXT:    beqz a4, .LBB14_7
; RV32IF-NEXT:  # %bb.3: # %start
; RV32IF-NEXT:    beqz s0, .LBB14_8
; RV32IF-NEXT:  .LBB14_4: # %start
; RV32IF-NEXT:    bnez a4, .LBB14_6
; RV32IF-NEXT:  .LBB14_5: # %start
; RV32IF-NEXT:    mv a2, a1
; RV32IF-NEXT:  .LBB14_6: # %start
; RV32IF-NEXT:    mv a1, a2
; RV32IF-NEXT:    lw s0, 8(sp) # 4-byte Folded Reload
; RV32IF-NEXT:    lw ra, 12(sp) # 4-byte Folded Reload
; RV32IF-NEXT:    addi sp, sp, 16
; RV32IF-NEXT:    ret
; RV32IF-NEXT:  .LBB14_7: # %start
; RV32IF-NEXT:    mv a0, a3
; RV32IF-NEXT:    bnez s0, .LBB14_4
; RV32IF-NEXT:  .LBB14_8: # %start
; RV32IF-NEXT:    mv a1, zero
; RV32IF-NEXT:    beqz a4, .LBB14_5
; RV32IF-NEXT:    j .LBB14_6
;
; RV64IF-LABEL: fcvt_lu_s_sat:
; RV64IF:       # %bb.0: # %start
; RV64IF-NEXT:    fmv.w.x ft0, a0
; RV64IF-NEXT:    feq.s a0, ft0, ft0
; RV64IF-NEXT:    bnez a0, .LBB14_2
; RV64IF-NEXT:  # %bb.1: # %start
; RV64IF-NEXT:    mv a0, zero
; RV64IF-NEXT:    ret
; RV64IF-NEXT:  .LBB14_2:
; RV64IF-NEXT:    fcvt.lu.s a0, ft0, rtz
; RV64IF-NEXT:    ret
start:
  %0 = tail call i64 @llvm.fptoui.sat.i64.f32(float %a)
  ret i64 %0
}
declare i64 @llvm.fptoui.sat.i64.f32(float)

define float @fcvt_s_l(i64 %a) nounwind {
; RV32IF-LABEL: fcvt_s_l:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    addi sp, sp, -16
; RV32IF-NEXT:    sw ra, 12(sp) # 4-byte Folded Spill
; RV32IF-NEXT:    call __floatdisf@plt
; RV32IF-NEXT:    lw ra, 12(sp) # 4-byte Folded Reload
; RV32IF-NEXT:    addi sp, sp, 16
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_l:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fcvt.s.l ft0, a0
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
  %1 = sitofp i64 %a to float
  ret float %1
}

define float @fcvt_s_lu(i64 %a) nounwind {
; RV32IF-LABEL: fcvt_s_lu:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    addi sp, sp, -16
; RV32IF-NEXT:    sw ra, 12(sp) # 4-byte Folded Spill
; RV32IF-NEXT:    call __floatundisf@plt
; RV32IF-NEXT:    lw ra, 12(sp) # 4-byte Folded Reload
; RV32IF-NEXT:    addi sp, sp, 16
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_lu:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fcvt.s.lu ft0, a0
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
  %1 = uitofp i64 %a to float
  ret float %1
}

define float @fcvt_s_w_i8(i8 signext %a) nounwind {
; RV32IF-LABEL: fcvt_s_w_i8:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    fcvt.s.w ft0, a0
; RV32IF-NEXT:    fmv.x.w a0, ft0
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_w_i8:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fcvt.s.w ft0, a0
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
  %1 = sitofp i8 %a to float
  ret float %1
}

define float @fcvt_s_wu_i8(i8 zeroext %a) nounwind {
; RV32IF-LABEL: fcvt_s_wu_i8:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    fcvt.s.wu ft0, a0
; RV32IF-NEXT:    fmv.x.w a0, ft0
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_wu_i8:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fcvt.s.wu ft0, a0
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
  %1 = uitofp i8 %a to float
  ret float %1
}

define float @fcvt_s_w_i16(i16 signext %a) nounwind {
; RV32IF-LABEL: fcvt_s_w_i16:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    fcvt.s.w ft0, a0
; RV32IF-NEXT:    fmv.x.w a0, ft0
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_w_i16:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fcvt.s.w ft0, a0
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
  %1 = sitofp i16 %a to float
  ret float %1
}

define float @fcvt_s_wu_i16(i16 zeroext %a) nounwind {
; RV32IF-LABEL: fcvt_s_wu_i16:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    fcvt.s.wu ft0, a0
; RV32IF-NEXT:    fmv.x.w a0, ft0
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_wu_i16:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    fcvt.s.wu ft0, a0
; RV64IF-NEXT:    fmv.x.w a0, ft0
; RV64IF-NEXT:    ret
  %1 = uitofp i16 %a to float
  ret float %1
}

; Make sure we select W version of addi on RV64.
define signext i32 @fcvt_s_w_demanded_bits(i32 signext %0, float* %1) {
; RV32IF-LABEL: fcvt_s_w_demanded_bits:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    addi a0, a0, 1
; RV32IF-NEXT:    fcvt.s.w ft0, a0
; RV32IF-NEXT:    fsw ft0, 0(a1)
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_w_demanded_bits:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    addiw a0, a0, 1
; RV64IF-NEXT:    fcvt.s.w ft0, a0
; RV64IF-NEXT:    fsw ft0, 0(a1)
; RV64IF-NEXT:    ret
  %3 = add i32 %0, 1
  %4 = sitofp i32 %3 to float
  store float %4, float* %1, align 4
  ret i32 %3
}

; Make sure we select W version of addi on RV64.
define signext i32 @fcvt_s_wu_demanded_bits(i32 signext %0, float* %1) {
; RV32IF-LABEL: fcvt_s_wu_demanded_bits:
; RV32IF:       # %bb.0:
; RV32IF-NEXT:    addi a0, a0, 1
; RV32IF-NEXT:    fcvt.s.wu ft0, a0
; RV32IF-NEXT:    fsw ft0, 0(a1)
; RV32IF-NEXT:    ret
;
; RV64IF-LABEL: fcvt_s_wu_demanded_bits:
; RV64IF:       # %bb.0:
; RV64IF-NEXT:    addiw a0, a0, 1
; RV64IF-NEXT:    fcvt.s.wu ft0, a0
; RV64IF-NEXT:    fsw ft0, 0(a1)
; RV64IF-NEXT:    ret
  %3 = add i32 %0, 1
  %4 = uitofp i32 %3 to float
  store float %4, float* %1, align 4
  ret i32 %3
}
