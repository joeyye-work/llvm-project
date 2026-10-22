; RUN: llc -mtriple=groq-unknown-unknown -mcpu=generic -verify-machineinstrs < %s | FileCheck %s

define i32 @reg_pressure(i32 %a) {
; CHECK-LABEL: reg_pressure:
; CHECK: add
; CHECK: ret
  %b = add i32 %a, 1
  %c = add i32 %b, 2
  %d = add i32 %c, 3
  %e = add i32 %d, 4
  %f = add i32 %e, 5
  %g = add i32 %f, 6
  %h = add i32 %g, 7
  %x = add i32 %h, %a
  %y = add i32 %x, %d
  %z = add i32 %y, %f
  ret i32 %z
}
