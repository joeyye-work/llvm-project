// RUN: mlir-translate -mlir-to-llvmir -split-input-file -verify-diagnostics %s | FileCheck %s

// CHECK: define <4 x float> @round_sse41() {
// CHECK:  %1 = call reassoc <4 x float> @llvm.x86.sse41.round.ss(<4 x float> {{.*}}, <4 x float> {{.*}}, i32 1)
// CHECK:  ret <4 x float> %1
// CHECK: }
llvm.func @round_sse41() -> vector<4xf32> {
  %0 = llvm.mlir.constant(1 : i32) : i32
  %1 = llvm.mlir.constant(dense<0.2> : vector<4xf32>) : vector<4xf32>
  %res = llvm.call_intrinsic "llvm.x86.sse41.round.ss"(%1, %1, %0) {fastmathFlags = #llvm.fastmath<reassoc>} : (vector<4xf32>, vector<4xf32>, i32) -> vector<4xf32>
  llvm.return %res: vector<4xf32>
}

// -----

// CHECK: define float @round_overloaded() {
// CHECK:   %1 = call float @llvm.round.f32(float 1.000000e+00)
// CHECK:   ret float %1
// CHECK: }
llvm.func @round_overloaded() -> f32 {
  %0 = llvm.mlir.constant(1.0 : f32) : f32
  %res = llvm.call_intrinsic "llvm.round"(%0) {} : (f32) -> f32
  llvm.return %res: f32
}

// -----

// CHECK: define void @lifetime_start() {
// CHECK:   %1 = alloca float, i8 1, align 4
// CHECK:   call void @llvm.lifetime.start.p0(ptr %1)
// CHECK:   ret void
// CHECK: }
llvm.func @lifetime_start() {
  %0 = llvm.mlir.constant(1 : i8) : i8
  %1 = llvm.alloca %0 x f32 : (i8) -> !llvm.ptr
  llvm.call_intrinsic "llvm.lifetime.start"(%1) {} : (!llvm.ptr) -> ()
  llvm.return
}

// -----

// CHECK-LABEL: define void @variadic()
llvm.func @variadic() {
  %0 = llvm.mlir.constant(1 : i8) : i8
  %1 = llvm.alloca %0 x f32 : (i8) -> !llvm.ptr
  // CHECK: call void (...) @llvm.localescape(ptr %1, ptr %1)
  llvm.call_intrinsic "llvm.localescape"(%1, %1) : (!llvm.ptr, !llvm.ptr) -> ()
  llvm.return
}

// -----

llvm.func @no_intrinsic() {
  // expected-error@below {{could not find LLVM intrinsic: "llvm.does_not_exist"}}
  // expected-error@below {{LLVM Translation failed for operation: llvm.call_intrinsic}}
  llvm.call_intrinsic "llvm.does_not_exist"() : () -> ()
  llvm.return
}

// -----

llvm.func @bad_types() {
  %0 = llvm.mlir.constant(1 : i8) : i8
  // expected-error@below {{call intrinsic signature i8 (i8) to overloaded intrinsic "llvm.round" does not match any of the overloads: intrinsic return type (overload type 0) expected any fp or fp vector, but got i8}}
  // expected-error@below {{LLVM Translation failed for operation: llvm.call_intrinsic}}
  llvm.call_intrinsic "llvm.round"(%0) {} : (i8) -> i8
  llvm.return
}

// -----

llvm.func @bad_result() {
  // expected-error @below {{intrinsic call returns void but "llvm.x86.sse41.round.ss" actually returns <4 x float>}}
  // expected-error@below {{LLVM Translation failed for operation: llvm.call_intrinsic}}
  llvm.call_intrinsic "llvm.x86.sse41.round.ss"() : () -> ()
  llvm.return
}

// -----

llvm.func @bad_result() {
  // expected-error @below {{intrinsic call returns <8 x float> but "llvm.x86.sse41.round.ss" actually returns <4 x float>}}
  // expected-error@below {{LLVM Translation failed for operation: llvm.call_intrinsic}}
  llvm.call_intrinsic "llvm.x86.sse41.round.ss"() : () -> (vector<8xf32>)
  llvm.return
}

// -----

llvm.func @bad_args() {
  // expected-error @below {{intrinsic call has 0 operands but "llvm.x86.sse41.round.ss" expects 3}}
  // expected-error@below {{LLVM Translation failed for operation: llvm.call_intrinsic}}
  llvm.call_intrinsic "llvm.x86.sse41.round.ss"() : () -> (vector<4xf32>)
  llvm.return
}

// -----

llvm.func @bad_args() {
  %0 = llvm.mlir.constant(1 : i64) : i64
  %1 = llvm.mlir.constant(dense<0.2> : vector<4xf32>) : vector<4xf32>
  // expected-error @below {{intrinsic call operand #2 has type i64 but "llvm.x86.sse41.round.ss" expects i32}}
  // expected-error@below {{LLVM Translation failed for operation: llvm.call_intrinsic}}
  %res = llvm.call_intrinsic "llvm.x86.sse41.round.ss"(%1, %1, %0) {fastmathFlags = #llvm.fastmath<reassoc>} : (vector<4xf32>, vector<4xf32>, i64) -> vector<4xf32>
  llvm.return
}

// -----

// CHECK-LABEL: intrinsic_call_arg_attrs
llvm.func @intrinsic_call_arg_attrs(%arg0: i32) -> i32 {
  // CHECK: call i32 @llvm.riscv.sha256sig0(i32 signext %{{.*}})
  %0 = llvm.call_intrinsic "llvm.riscv.sha256sig0"(%arg0) : (i32 {llvm.signext}) -> (i32)
  llvm.return %0 : i32
}

// -----

// CHECK-LABEL: intrinsic_element_type
llvm.func @intrinsic_element_type(%arg0: !llvm.ptr) {
  // CHECK: call i64 @llvm.aarch64.ldxr.p0(ptr elementtype(i8) %{{.*}})
  %0 = llvm.call_intrinsic "llvm.aarch64.ldxr.p0"(%arg0) : (!llvm.ptr {llvm.elementtype = i8}) -> i64
  llvm.return
}

// -----

// CHECK-LABEL: define float @constrained_sqrt(float %0)
// CHECK: call float @llvm.experimental.constrained.sqrt.f32(float %{{.*}}, metadata !"round.tonearest", metadata !"fpexcept.strict")
llvm.func @constrained_sqrt(%a: f32) -> f32 {
  %rm = llvm.mlir.metadata_as_value #llvm.md_string<"round.tonearest">
  %eb = llvm.mlir.metadata_as_value #llvm.md_string<"fpexcept.strict">
  %r = llvm.call_intrinsic "llvm.experimental.constrained.sqrt.f32"(%a, %rm, %eb)
      : (f32, !llvm.metadata, !llvm.metadata) -> f32
  llvm.return %r : f32
}

// -----

// CHECK-LABEL: define i32 @read_named_register()
// CHECK: call i32 @llvm.read_register.i32(metadata !{{[0-9]+}})
// CHECK: !{{[0-9]+}} = !{!"sp"}
llvm.func @read_named_register() -> i32 {
  %md = llvm.mlir.metadata_as_value #llvm.md_node<#llvm.md_string<"sp">>
  %r = llvm.call_intrinsic "llvm.read_register.i32"(%md)
      : (!llvm.metadata) -> i32
  llvm.return %r : i32
}

// -----

// CHECK-LABEL: define <16 x i8> @groq_tsp_families
// CHECK: call void @llvm.experimental.groq.tsp.icu.nop(i32 0)
// CHECK: call void @llvm.experimental.groq.tsp.icu.ifetch.v16i8
// CHECK: call void @llvm.experimental.groq.tsp.icu.sync()
// CHECK: call void @llvm.experimental.groq.tsp.icu.notify()
// CHECK: call void @llvm.experimental.groq.tsp.icu.config(i32 0)
// CHECK: call void @llvm.experimental.groq.tsp.icu.repeat(i32 0, i32 1)
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.mem.read
// CHECK: call void @llvm.experimental.groq.tsp.mem.write
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.mem.gather
// CHECK: call void @llvm.experimental.groq.tsp.mem.scatter
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.vxm.unary
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.vxm.binary
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.vxm.type.convert
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.vxm.relu
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.vxm.tanh
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.vxm.exp
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.vxm.rsqrt
// CHECK: call void @llvm.experimental.groq.tsp.mxm.lw
// CHECK: call void @llvm.experimental.groq.tsp.mxm.iw
// CHECK: call void @llvm.experimental.groq.tsp.mxm.abc(i32 0)
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.mxm.acc
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.sxm.shift.updown
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.sxm.permute
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.sxm.distribute
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.sxm.rotate.stream
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.sxm.transpose
// CHECK: call void @llvm.experimental.groq.tsp.c2c.deskew(i32 0)
// CHECK: call void @llvm.experimental.groq.tsp.c2c.send
// CHECK: call <16 x i8> @llvm.experimental.groq.tsp.c2c.receive
llvm.func @groq_tsp_families(%ptr: !llvm.ptr, %v: vector<16xi8>) -> vector<16xi8> {
  %c0 = llvm.mlir.constant(0 : i32) : i32
  %c1 = llvm.mlir.constant(1 : i32) : i32
  llvm.call_intrinsic "llvm.experimental.groq.tsp.icu.nop"(%c0) : (i32) -> ()
  llvm.call_intrinsic "llvm.experimental.groq.tsp.icu.ifetch"(%v) : (vector<16xi8>) -> ()
  llvm.call_intrinsic "llvm.experimental.groq.tsp.icu.sync"() : () -> ()
  llvm.call_intrinsic "llvm.experimental.groq.tsp.icu.notify"() : () -> ()
  llvm.call_intrinsic "llvm.experimental.groq.tsp.icu.config"(%c0) : (i32) -> ()
  llvm.call_intrinsic "llvm.experimental.groq.tsp.icu.repeat"(%c0, %c1) : (i32, i32) -> ()

  %read = llvm.call_intrinsic "llvm.experimental.groq.tsp.mem.read"(%ptr, %c0)
      : (!llvm.ptr, i32) -> vector<16xi8>
  llvm.call_intrinsic "llvm.experimental.groq.tsp.mem.write"(%read, %ptr, %c0)
      : (vector<16xi8>, !llvm.ptr, i32) -> ()
  %gather = llvm.call_intrinsic "llvm.experimental.groq.tsp.mem.gather"(%ptr, %v, %c0)
      : (!llvm.ptr, vector<16xi8>, i32) -> vector<16xi8>
  llvm.call_intrinsic "llvm.experimental.groq.tsp.mem.scatter"(%gather, %ptr, %v, %c0)
      : (vector<16xi8>, !llvm.ptr, vector<16xi8>, i32) -> ()

  %unary = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.unary"(%v, %c0)
      : (vector<16xi8>, i32) -> vector<16xi8>
  %binary = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.binary"(%unary, %v, %c0)
      : (vector<16xi8>, vector<16xi8>, i32) -> vector<16xi8>
  %convert = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.type.convert"(%binary, %c0)
      : (vector<16xi8>, i32) -> vector<16xi8>
  %relu = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.relu"(%convert)
      : (vector<16xi8>) -> vector<16xi8>
  %tanh = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.tanh"(%relu)
      : (vector<16xi8>) -> vector<16xi8>
  %exp = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.exp"(%tanh)
      : (vector<16xi8>) -> vector<16xi8>
  %rsqrt = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.rsqrt"(%exp)
      : (vector<16xi8>) -> vector<16xi8>

  llvm.call_intrinsic "llvm.experimental.groq.tsp.mxm.lw"(%v, %c0)
      : (vector<16xi8>, i32) -> ()
  llvm.call_intrinsic "llvm.experimental.groq.tsp.mxm.iw"(%v, %c0)
      : (vector<16xi8>, i32) -> ()
  llvm.call_intrinsic "llvm.experimental.groq.tsp.mxm.abc"(%c0) : (i32) -> ()
  %acc = llvm.call_intrinsic "llvm.experimental.groq.tsp.mxm.acc"(%rsqrt)
      : (vector<16xi8>) -> vector<16xi8>

  %shift = llvm.call_intrinsic "llvm.experimental.groq.tsp.sxm.shift.updown"(%acc, %c0)
      : (vector<16xi8>, i32) -> vector<16xi8>
  %permute = llvm.call_intrinsic "llvm.experimental.groq.tsp.sxm.permute"(%shift, %v)
      : (vector<16xi8>, vector<16xi8>) -> vector<16xi8>
  %distribute = llvm.call_intrinsic "llvm.experimental.groq.tsp.sxm.distribute"(%permute, %v)
      : (vector<16xi8>, vector<16xi8>) -> vector<16xi8>
  %rotate = llvm.call_intrinsic "llvm.experimental.groq.tsp.sxm.rotate.stream"(%distribute, %c0)
      : (vector<16xi8>, i32) -> vector<16xi8>
  %transpose = llvm.call_intrinsic "llvm.experimental.groq.tsp.sxm.transpose"(%rotate)
      : (vector<16xi8>) -> vector<16xi8>

  llvm.call_intrinsic "llvm.experimental.groq.tsp.c2c.deskew"(%c0) : (i32) -> ()
  llvm.call_intrinsic "llvm.experimental.groq.tsp.c2c.send"(%transpose, %c0)
      : (vector<16xi8>, i32) -> ()
  %receive = llvm.call_intrinsic "llvm.experimental.groq.tsp.c2c.receive"(%c0)
      : (i32) -> vector<16xi8>
  llvm.return %receive : vector<16xi8>
}

// -----

// CHECK-LABEL: define <320 x i8> @groq_tsp_valid_width_320
// CHECK: call <320 x i8> @llvm.experimental.groq.tsp.vxm.binary
llvm.func @groq_tsp_valid_width_320(%lhs: vector<320xi8>, %rhs: vector<320xi8>) -> vector<320xi8> {
  %c0 = llvm.mlir.constant(0 : i32) : i32
  %0 = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.binary"(%lhs, %rhs, %c0)
      : (vector<320xi8>, vector<320xi8>, i32) -> vector<320xi8>
  llvm.return %0 : vector<320xi8>
}

// -----

llvm.func @groq_tsp_too_small(%arg0: vector<8xi8>) -> vector<8xi8> {
  // expected-error@below {{'llvm.call_intrinsic' op llvm.experimental.groq.tsp vector operands must have total byte length divisible by 16 and in the inclusive range [16, 320]}}
  %0 = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.relu"(%arg0) : (vector<8xi8>) -> vector<8xi8>
  llvm.return %0 : vector<8xi8>
}

// -----

llvm.func @groq_tsp_bad_multiple(%arg0: vector<24xi8>) -> vector<24xi8> {
  // expected-error@below {{'llvm.call_intrinsic' op llvm.experimental.groq.tsp vector operands must have total byte length divisible by 16 and in the inclusive range [16, 320]}}
  %0 = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.relu"(%arg0) : (vector<24xi8>) -> vector<24xi8>
  llvm.return %0 : vector<24xi8>
}

// -----

llvm.func @groq_tsp_too_large(%arg0: vector<336xi8>) -> vector<336xi8> {
  // expected-error@below {{'llvm.call_intrinsic' op llvm.experimental.groq.tsp vector operands must have total byte length divisible by 16 and in the inclusive range [16, 320]}}
  %0 = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.relu"(%arg0) : (vector<336xi8>) -> vector<336xi8>
  llvm.return %0 : vector<336xi8>
}

// -----

llvm.func @groq_tsp_scalable(%arg0: vector<[4]xi32>) -> vector<[4]xi32> {
  // expected-error@below {{'llvm.call_intrinsic' op llvm.experimental.groq.tsp vector operands must be fixed vectors}}
  %0 = llvm.call_intrinsic "llvm.experimental.groq.tsp.vxm.relu"(%arg0) : (vector<[4]xi32>) -> vector<[4]xi32>
  llvm.return %0 : vector<[4]xi32>
}
