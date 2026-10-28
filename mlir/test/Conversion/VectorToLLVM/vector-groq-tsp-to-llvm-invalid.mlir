// RUN: mlir-opt %s --convert-vector-to-llvm='enable-groq-tsp=1' -split-input-file --verify-diagnostics

func.func @groq_tsp_scalable_load(%mem : memref<64xi32>, %i : index) -> vector<[4]xi32> {
  // expected-error@+1 {{direct Groq/TSP intrinsic lowering for vector.load requires fixed vectors}}
  %0 = vector.load %mem[%i] : memref<64xi32>, vector<[4]xi32>
  return %0 : vector<[4]xi32>
}

// -----

func.func @groq_tsp_bad_width_load(%mem : memref<64xi32>, %i : index) -> vector<6xi32> {
  // expected-error@+1 {{direct Groq/TSP intrinsic lowering for vector.load requires total vector byte size divisible by 16 and in the inclusive range [16, 320]}}
  %0 = vector.load %mem[%i] : memref<64xi32>, vector<6xi32>
  return %0 : vector<6xi32>
}

// -----

func.func @groq_tsp_unsupported_masked_load(%mem : memref<64xi8>, %i : index, %mask : vector<16xi1>, %pass_thru : vector<16xi8>) -> vector<16xi8> {
  // expected-error@+1 {{direct Groq/TSP intrinsic lowering currently supports only vector.load/vector.store}}
  %0 = vector.maskedload %mem[%i], %mask, %pass_thru : memref<64xi8>, vector<16xi1>, vector<16xi8> into vector<16xi8>
  return %0 : vector<16xi8>
}

// -----

func.func @groq_tsp_unsupported_gather(%base : memref<64xi8>, %ofs : index, %indices : vector<16xi32>, %mask : vector<16xi1>, %pass_thru : vector<16xi8>) -> vector<16xi8> {
  // expected-error@+1 {{direct Groq/TSP intrinsic lowering currently supports only vector.load/vector.store}}
  %0 = vector.gather %base[%ofs][%indices], %mask, %pass_thru : memref<64xi8>, vector<16xi32>, vector<16xi1>, vector<16xi8> into vector<16xi8>
  return %0 : vector<16xi8>
}
