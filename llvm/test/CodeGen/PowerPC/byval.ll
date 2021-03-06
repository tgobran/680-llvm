; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -verify-machineinstrs < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-n32:64-S128-v256:256:256-v512:512:512"
target triple = "powerpc64le-unknown-linux-gnu"

%struct = type { [4 x i32], [20 x i8] }

declare dso_local i32 @foo1(%struct* byval(%struct) %var)
declare dso_local void @foo(%struct* %var)

; FIXME: for the byval parameter %x, now the memory for local variable and
; for parameter save area are overlap.
; For the below case,
; the local variable space is r1 + 40 ~ r1 + 76
; the parameter save area is r1 + 32 ~ r1 + 68

define dso_local i32 @bar() {
; CHECK-LABEL: bar:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    mflr 0
; CHECK-NEXT:    .cfi_def_cfa_offset 96
; CHECK-NEXT:    .cfi_offset lr, 16
; CHECK-NEXT:    .cfi_offset r30, -16
; CHECK-NEXT:    std 30, -16(1) # 8-byte Folded Spill
; CHECK-NEXT:    std 0, 16(1)
; CHECK-NEXT:    stdu 1, -96(1)
; CHECK-NEXT:    addi 30, 1, 40
; CHECK-NEXT:    mr 3, 30
; CHECK-NEXT:    bl foo
; CHECK-NEXT:    nop
; CHECK-NEXT:    li 3, 16
; CHECK-NEXT:    lxvd2x 0, 30, 3
; CHECK-NEXT:    li 3, 48
; CHECK-NEXT:    stxvd2x 0, 1, 3
; CHECK-NEXT:    li 3, 32
; CHECK-NEXT:    lxvd2x 0, 0, 30
; CHECK-NEXT:    stxvd2x 0, 1, 3
; CHECK-NEXT:    lwz 3, 72(1)
; CHECK-NEXT:    ld 7, 72(1)
; CHECK-NEXT:    ld 5, 56(1)
; CHECK-NEXT:    ld 4, 48(1)
; CHECK-NEXT:    stw 3, 64(1)
; CHECK-NEXT:    ld 3, 40(1)
; CHECK-NEXT:    ld 6, 64(1)
; CHECK-NEXT:    bl foo1
; CHECK-NEXT:    nop
; CHECK-NEXT:    addi 1, 1, 96
; CHECK-NEXT:    ld 0, 16(1)
; CHECK-NEXT:    ld 30, -16(1) # 8-byte Folded Reload
; CHECK-NEXT:    mtlr 0
; CHECK-NEXT:    blr
entry:
  %x = alloca %struct, align 4
  call void @foo(%struct* %x)
  %r = call i32 @foo1(%struct* byval(%struct) %x)
  ret i32 %r
}
