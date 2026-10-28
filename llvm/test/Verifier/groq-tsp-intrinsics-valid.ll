; RUN: opt -passes=verify -disable-output %s

; Positive coverage for vector byte lengths 16, 32, 48, ..., 320.
; This is IR-level modeling only (no backend/codegen expectations).

declare <16 x i8> @llvm.experimental.groq.tsp.vxm.binary.v16i8(<16 x i8>, <16 x i8>, i32 immarg)
declare <32 x i8> @llvm.experimental.groq.tsp.vxm.binary.v32i8(<32 x i8>, <32 x i8>, i32 immarg)
declare <48 x i8> @llvm.experimental.groq.tsp.vxm.binary.v48i8(<48 x i8>, <48 x i8>, i32 immarg)
declare <64 x i8> @llvm.experimental.groq.tsp.vxm.binary.v64i8(<64 x i8>, <64 x i8>, i32 immarg)
declare <80 x i8> @llvm.experimental.groq.tsp.vxm.binary.v80i8(<80 x i8>, <80 x i8>, i32 immarg)
declare <96 x i8> @llvm.experimental.groq.tsp.vxm.binary.v96i8(<96 x i8>, <96 x i8>, i32 immarg)
declare <112 x i8> @llvm.experimental.groq.tsp.vxm.binary.v112i8(<112 x i8>, <112 x i8>, i32 immarg)
declare <128 x i8> @llvm.experimental.groq.tsp.vxm.binary.v128i8(<128 x i8>, <128 x i8>, i32 immarg)
declare <144 x i8> @llvm.experimental.groq.tsp.vxm.binary.v144i8(<144 x i8>, <144 x i8>, i32 immarg)
declare <160 x i8> @llvm.experimental.groq.tsp.vxm.binary.v160i8(<160 x i8>, <160 x i8>, i32 immarg)
declare <176 x i8> @llvm.experimental.groq.tsp.vxm.binary.v176i8(<176 x i8>, <176 x i8>, i32 immarg)
declare <192 x i8> @llvm.experimental.groq.tsp.vxm.binary.v192i8(<192 x i8>, <192 x i8>, i32 immarg)
declare <208 x i8> @llvm.experimental.groq.tsp.vxm.binary.v208i8(<208 x i8>, <208 x i8>, i32 immarg)
declare <224 x i8> @llvm.experimental.groq.tsp.vxm.binary.v224i8(<224 x i8>, <224 x i8>, i32 immarg)
declare <240 x i8> @llvm.experimental.groq.tsp.vxm.binary.v240i8(<240 x i8>, <240 x i8>, i32 immarg)
declare <256 x i8> @llvm.experimental.groq.tsp.vxm.binary.v256i8(<256 x i8>, <256 x i8>, i32 immarg)
declare <272 x i8> @llvm.experimental.groq.tsp.vxm.binary.v272i8(<272 x i8>, <272 x i8>, i32 immarg)
declare <288 x i8> @llvm.experimental.groq.tsp.vxm.binary.v288i8(<288 x i8>, <288 x i8>, i32 immarg)
declare <304 x i8> @llvm.experimental.groq.tsp.vxm.binary.v304i8(<304 x i8>, <304 x i8>, i32 immarg)
declare <320 x i8> @llvm.experimental.groq.tsp.vxm.binary.v320i8(<320 x i8>, <320 x i8>, i32 immarg)

define void @valid_lengths() {
entry:
  %r16 = call <16 x i8> @llvm.experimental.groq.tsp.vxm.binary.v16i8(<16 x i8> zeroinitializer, <16 x i8> zeroinitializer, i32 0)
  %r32 = call <32 x i8> @llvm.experimental.groq.tsp.vxm.binary.v32i8(<32 x i8> zeroinitializer, <32 x i8> zeroinitializer, i32 0)
  %r48 = call <48 x i8> @llvm.experimental.groq.tsp.vxm.binary.v48i8(<48 x i8> zeroinitializer, <48 x i8> zeroinitializer, i32 0)
  %r64 = call <64 x i8> @llvm.experimental.groq.tsp.vxm.binary.v64i8(<64 x i8> zeroinitializer, <64 x i8> zeroinitializer, i32 0)
  %r80 = call <80 x i8> @llvm.experimental.groq.tsp.vxm.binary.v80i8(<80 x i8> zeroinitializer, <80 x i8> zeroinitializer, i32 0)
  %r96 = call <96 x i8> @llvm.experimental.groq.tsp.vxm.binary.v96i8(<96 x i8> zeroinitializer, <96 x i8> zeroinitializer, i32 0)
  %r112 = call <112 x i8> @llvm.experimental.groq.tsp.vxm.binary.v112i8(<112 x i8> zeroinitializer, <112 x i8> zeroinitializer, i32 0)
  %r128 = call <128 x i8> @llvm.experimental.groq.tsp.vxm.binary.v128i8(<128 x i8> zeroinitializer, <128 x i8> zeroinitializer, i32 0)
  %r144 = call <144 x i8> @llvm.experimental.groq.tsp.vxm.binary.v144i8(<144 x i8> zeroinitializer, <144 x i8> zeroinitializer, i32 0)
  %r160 = call <160 x i8> @llvm.experimental.groq.tsp.vxm.binary.v160i8(<160 x i8> zeroinitializer, <160 x i8> zeroinitializer, i32 0)
  %r176 = call <176 x i8> @llvm.experimental.groq.tsp.vxm.binary.v176i8(<176 x i8> zeroinitializer, <176 x i8> zeroinitializer, i32 0)
  %r192 = call <192 x i8> @llvm.experimental.groq.tsp.vxm.binary.v192i8(<192 x i8> zeroinitializer, <192 x i8> zeroinitializer, i32 0)
  %r208 = call <208 x i8> @llvm.experimental.groq.tsp.vxm.binary.v208i8(<208 x i8> zeroinitializer, <208 x i8> zeroinitializer, i32 0)
  %r224 = call <224 x i8> @llvm.experimental.groq.tsp.vxm.binary.v224i8(<224 x i8> zeroinitializer, <224 x i8> zeroinitializer, i32 0)
  %r240 = call <240 x i8> @llvm.experimental.groq.tsp.vxm.binary.v240i8(<240 x i8> zeroinitializer, <240 x i8> zeroinitializer, i32 0)
  %r256 = call <256 x i8> @llvm.experimental.groq.tsp.vxm.binary.v256i8(<256 x i8> zeroinitializer, <256 x i8> zeroinitializer, i32 0)
  %r272 = call <272 x i8> @llvm.experimental.groq.tsp.vxm.binary.v272i8(<272 x i8> zeroinitializer, <272 x i8> zeroinitializer, i32 0)
  %r288 = call <288 x i8> @llvm.experimental.groq.tsp.vxm.binary.v288i8(<288 x i8> zeroinitializer, <288 x i8> zeroinitializer, i32 0)
  %r304 = call <304 x i8> @llvm.experimental.groq.tsp.vxm.binary.v304i8(<304 x i8> zeroinitializer, <304 x i8> zeroinitializer, i32 0)
  %r320 = call <320 x i8> @llvm.experimental.groq.tsp.vxm.binary.v320i8(<320 x i8> zeroinitializer, <320 x i8> zeroinitializer, i32 0)
  ret void
}
