; RUN: llc -mtriple=groq-unknown-unknown -mcpu=generic -verify-machineinstrs -misched=ilpmax < %s | FileCheck %s

define i32 @sched_smoke(i32 %a, i32 %b, i32 %c) {
; CHECK-LABEL: sched_smoke:
; CHECK: add
; CHECK: sub
; CHECK: ret
  %x = add i32 %a, %b
  %y = sub i32 %x, %c
  ret i32 %y
}
