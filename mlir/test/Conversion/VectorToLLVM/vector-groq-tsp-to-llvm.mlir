// RUN: mlir-opt %s --convert-vector-to-llvm='enable-groq-tsp=1' -split-input-file | FileCheck %s

// CHECK-LABEL: func.func @groq_tsp_mem_read
// CHECK: %[[C0:.*]] = llvm.mlir.constant(0 : i32) : i32
// CHECK: %[[READ:.*]] = llvm.call_intrinsic "llvm.experimental.groq.tsp.mem.read"(%{{.*}}, %[[C0]]) : (!llvm.ptr, i32) -> vector<16xi8>
func.func @groq_tsp_mem_read(%mem : memref<256xi8>, %i : index) -> vector<16xi8> {
  %0 = vector.load %mem[%i] : memref<256xi8>, vector<16xi8>
  return %0 : vector<16xi8>
}

// -----

// CHECK-LABEL: func.func @groq_tsp_mem_write
// CHECK: %[[C0:.*]] = llvm.mlir.constant(0 : i32) : i32
// CHECK: llvm.call_intrinsic "llvm.experimental.groq.tsp.mem.write"(%{{.*}}, %{{.*}}, %[[C0]]) : (vector<80xi32>, !llvm.ptr, i32) -> ()
func.func @groq_tsp_mem_write(%mem : memref<256xi32>, %i : index, %v : vector<80xi32>) {
  vector.store %v, %mem[%i] : memref<256xi32>, vector<80xi32>
  return
}
