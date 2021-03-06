; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=m68k-linux -verify-machineinstrs | FileCheck %s

declare i32 @printf(i8*, ...) nounwind
declare {i32, i1} @llvm.smul.with.overflow.i32(i32, i32)
declare { i63, i1 } @llvm.smul.with.overflow.i63(i63, i63)

@ok = internal constant [4 x i8] c"%d\0A\00"
@no = internal constant [4 x i8] c"no\0A\00"

define fastcc i1 @test1(i32 %v1, i32 %v2) nounwind {
; CHECK-LABEL: test1:
; CHECK:       ; %bb.0: ; %entry
; CHECK-NEXT:    suba.l #28, %sp
; CHECK-NEXT:    movem.l %d2-%d3, (20,%sp) ; 12-byte Folded Spill
; CHECK-NEXT:    move.l %d1, (12,%sp)
; CHECK-NEXT:    move.l #31, %d2
; CHECK-NEXT:    asr.l %d2, %d1
; CHECK-NEXT:    move.l %d1, (8,%sp)
; CHECK-NEXT:    move.l %d0, (4,%sp)
; CHECK-NEXT:    asr.l %d2, %d0
; CHECK-NEXT:    move.l %d0, (%sp)
; CHECK-NEXT:    jsr __muldi3@PLT
; CHECK-NEXT:    move.l %d1, %d3
; CHECK-NEXT:    asr.l %d2, %d3
; CHECK-NEXT:    sub.l %d3, %d0
; CHECK-NEXT:    sne %d0
; CHECK-NEXT:    cmpi.b #0, %d0
; CHECK-NEXT:    beq .LBB0_1
; CHECK-NEXT:  ; %bb.2: ; %overflow
; CHECK-NEXT:    lea (no,%pc), %a0
; CHECK-NEXT:    move.l %a0, (%sp)
; CHECK-NEXT:    jsr printf@PLT
; CHECK-NEXT:    move.b #0, %d0
; CHECK-NEXT:    bra .LBB0_3
; CHECK-NEXT:  .LBB0_1: ; %normal
; CHECK-NEXT:    move.l %d1, (4,%sp)
; CHECK-NEXT:    lea (ok,%pc), %a0
; CHECK-NEXT:    move.l %a0, (%sp)
; CHECK-NEXT:    jsr printf@PLT
; CHECK-NEXT:    move.b #1, %d0
; CHECK-NEXT:  .LBB0_3: ; %overflow
; CHECK-NEXT:    movem.l (20,%sp), %d2-%d3 ; 12-byte Folded Reload
; CHECK-NEXT:    adda.l #28, %sp
; CHECK-NEXT:    rts
entry:
  %t = call {i32, i1} @llvm.smul.with.overflow.i32(i32 %v1, i32 %v2)
  %sum = extractvalue {i32, i1} %t, 0
  %obit = extractvalue {i32, i1} %t, 1
  br i1 %obit, label %overflow, label %normal

normal:
  %t1 = tail call i32 (i8*, ...) @printf( i8* getelementptr ([4 x i8], [4 x i8]* @ok, i32 0, i32 0), i32 %sum ) nounwind
  ret i1 true

overflow:
  %t2 = tail call i32 (i8*, ...) @printf( i8* getelementptr ([4 x i8], [4 x i8]* @no, i32 0, i32 0) ) nounwind
  ret i1 false
}

define fastcc i1 @test2(i32 %v1, i32 %v2) nounwind {
; CHECK-LABEL: test2:
; CHECK:       ; %bb.0: ; %entry
; CHECK-NEXT:    suba.l #28, %sp
; CHECK-NEXT:    movem.l %d2-%d3, (20,%sp) ; 12-byte Folded Spill
; CHECK-NEXT:    move.l %d1, (12,%sp)
; CHECK-NEXT:    move.l #31, %d2
; CHECK-NEXT:    asr.l %d2, %d1
; CHECK-NEXT:    move.l %d1, (8,%sp)
; CHECK-NEXT:    move.l %d0, (4,%sp)
; CHECK-NEXT:    asr.l %d2, %d0
; CHECK-NEXT:    move.l %d0, (%sp)
; CHECK-NEXT:    jsr __muldi3@PLT
; CHECK-NEXT:    move.l %d1, %d3
; CHECK-NEXT:    asr.l %d2, %d3
; CHECK-NEXT:    sub.l %d3, %d0
; CHECK-NEXT:    sne %d0
; CHECK-NEXT:    sub.b #1, %d0
; CHECK-NEXT:    bne .LBB1_3
; CHECK-NEXT:  ; %bb.1: ; %overflow
; CHECK-NEXT:    lea (no,%pc), %a0
; CHECK-NEXT:    move.l %a0, (%sp)
; CHECK-NEXT:    jsr printf@PLT
; CHECK-NEXT:    move.b #0, %d0
; CHECK-NEXT:    bra .LBB1_2
; CHECK-NEXT:  .LBB1_3: ; %normal
; CHECK-NEXT:    move.l %d1, (4,%sp)
; CHECK-NEXT:    lea (ok,%pc), %a0
; CHECK-NEXT:    move.l %a0, (%sp)
; CHECK-NEXT:    jsr printf@PLT
; CHECK-NEXT:    move.b #1, %d0
; CHECK-NEXT:  .LBB1_2: ; %overflow
; CHECK-NEXT:    movem.l (20,%sp), %d2-%d3 ; 12-byte Folded Reload
; CHECK-NEXT:    adda.l #28, %sp
; CHECK-NEXT:    rts
entry:
  %t = call {i32, i1} @llvm.smul.with.overflow.i32(i32 %v1, i32 %v2)
  %sum = extractvalue {i32, i1} %t, 0
  %obit = extractvalue {i32, i1} %t, 1
  br i1 %obit, label %overflow, label %normal

overflow:
  %t2 = tail call i32 (i8*, ...) @printf( i8* getelementptr ([4 x i8], [4 x i8]* @no, i32 0, i32 0) ) nounwind
  ret i1 false

normal:
  %t1 = tail call i32 (i8*, ...) @printf( i8* getelementptr ([4 x i8], [4 x i8]* @ok, i32 0, i32 0), i32 %sum ) nounwind
  ret i1 true
}

define i32 @test3(i32 %a, i32 %b) nounwind readnone {
; CHECK-LABEL: test3:
; CHECK:       ; %bb.0: ; %entry
; CHECK-NEXT:    move.l (8,%sp), %d0
; CHECK-NEXT:    add.l (4,%sp), %d0
; CHECK-NEXT:    add.l %d0, %d0
; CHECK-NEXT:    rts
entry:
	%tmp0 = add i32 %b, %a
	%tmp1 = call { i32, i1 } @llvm.smul.with.overflow.i32(i32 %tmp0, i32 2)
	%tmp2 = extractvalue { i32, i1 } %tmp1, 0
	ret i32 %tmp2
}

; Same as umul-with-overflow, we shouldn't fallback to
; builtin here
define i32 @test4(i32 %a, i32 %b) nounwind readnone {
; CHECK-LABEL: test4:
; CHECK:       ; %bb.0: ; %entry
; CHECK-NEXT:    move.l (8,%sp), %d0
; CHECK-NEXT:    add.l (4,%sp), %d0
; CHECK-NEXT:    lsl.l #2, %d0
; CHECK-NEXT:    rts
entry:
	%tmp0 = add i32 %b, %a
	%tmp1 = call { i32, i1 } @llvm.smul.with.overflow.i32(i32 %tmp0, i32 4)
	%tmp2 = extractvalue { i32, i1 } %tmp1, 0
	ret i32 %tmp2
}
