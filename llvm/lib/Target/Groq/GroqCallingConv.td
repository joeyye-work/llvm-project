//===- GroqCallingConv.td - Groq Calling Conventions -*- tablegen ---*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This describes a provisional calling convention for backend bring-up only.
// TODO(groq-spec): Replace with the official Groq program interface once
// public ABI/runtime documentation is available.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Groq base calling convention
//===----------------------------------------------------------------------===//
// Groq return value
def RetCC_Groq : CallingConv<[
  // First two return values go in a2, a3, a4, a5
  CCIfType<[i32], CCAssignToReg<[A2, A3, A4, A5]>>,
  CCIfType<[f32], CCAssignToReg<[A2, A3, A4, A5]>>,
  CCIfType<[i64], CCAssignToRegWithShadow<[A2, A4], [A3, A5]>>
]>;

// Callee-saved register lists
def CSR_Groq : CalleeSavedRegs<(add A0, A12, A13, A14, A15)>;

//===----------------------------------------------------------------------===//
// Groq windowed calling convention. Currently by default implemented
// rotation window by 8 registers.
//===----------------------------------------------------------------------===//
// Groq return value for 8 registers window
def RetCCW8_Groq : CallingConv<[
  //First two return values go in a10, a11, a12, a13
  CCIfType<[i32], CCAssignToReg<[A10, A11, A12, A13]>>,
  CCIfType<[i64], CCAssignToRegWithShadow<[A10, A12], [A11, A13]>>
]>;

// Callee-saved register lists for rotation window by 8 registers
def CSRW8_Groq : CalleeSavedRegs<(add)> {
  let OtherPreserved = (add A0, SP, A2, A3, A4, A5, A6, A7);
}
