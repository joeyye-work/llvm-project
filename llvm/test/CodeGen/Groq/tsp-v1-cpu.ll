; RUN: llc -mtriple=groq-unknown-unknown -mcpu=tsp-v1 -verify-machineinstrs < %s | FileCheck %s

define i32 @tsp_v1_add(i32 %a, i32 %b) {
; CHECK-LABEL: tsp_v1_add:
; CHECK: add
; CHECK: ret
  %sum = add i32 %a, %b
  ret i32 %sum
}
