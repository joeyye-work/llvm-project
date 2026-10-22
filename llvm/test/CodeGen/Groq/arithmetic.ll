; RUN: llc -mtriple=groq-unknown-unknown -mcpu=generic -verify-machineinstrs < %s | FileCheck %s

define i32 @add_i32(i32 %a, i32 %b) {
; CHECK-LABEL: add_i32:
; CHECK: add
; CHECK: ret
  %sum = add i32 %a, %b
  ret i32 %sum
}

define i32 @add_imm(i32 %a) {
; CHECK-LABEL: add_imm:
; CHECK: addi
; CHECK: ret
  %sum = add i32 %a, 7
  ret i32 %sum
}
