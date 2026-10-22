; RUN: llc -mtriple=groq-unknown-unknown -mcpu=generic -verify-machineinstrs < %s | FileCheck %s

define i32 @br_cond(i1 %cond, i32 %x) {
; CHECK-LABEL: br_cond:
; Provisional scaffold currently reuses Xtensa-style branch mnemonics.
; Tighten this once Groq-specific mnemonics are introduced in TableGen.
; CHECK: b{{[a-z0-9.]+}}
; CHECK: ret
entry:
  br i1 %cond, label %t, label %f

t:
  ret i32 %x

f:
  %y = sub i32 0, %x
  ret i32 %y
}
