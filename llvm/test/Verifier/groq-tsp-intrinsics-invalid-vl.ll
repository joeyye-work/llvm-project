; RUN: not opt -passes=verify -disable-output %s 2>&1 | FileCheck %s

declare <8 x i8> @llvm.experimental.groq.tsp.vxm.binary.v8i8(<8 x i8>, <8 x i8>, i32 immarg)
declare <24 x i8> @llvm.experimental.groq.tsp.vxm.binary.v24i8(<24 x i8>, <24 x i8>, i32 immarg)
declare <336 x i8> @llvm.experimental.groq.tsp.vxm.binary.v336i8(<336 x i8>, <336 x i8>, i32 immarg)

define void @invalid_lengths() {
entry:
  %r8 = call <8 x i8> @llvm.experimental.groq.tsp.vxm.binary.v8i8(<8 x i8> zeroinitializer, <8 x i8> zeroinitializer, i32 0)
  %r24 = call <24 x i8> @llvm.experimental.groq.tsp.vxm.binary.v24i8(<24 x i8> zeroinitializer, <24 x i8> zeroinitializer, i32 0)
  %r336 = call <336 x i8> @llvm.experimental.groq.tsp.vxm.binary.v336i8(<336 x i8> zeroinitializer, <336 x i8> zeroinitializer, i32 0)
  ret void
}

; CHECK: llvm.experimental.groq.tsp vector operands/results must be multiples of 16 bytes in the inclusive range [16, 320]
