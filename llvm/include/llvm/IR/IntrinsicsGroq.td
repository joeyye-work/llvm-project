//===- IntrinsicsGroq.td - Defines Groq/TSP intrinsics --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines LLVM IR-only Groq/TSP intrinsics scoped to the
// publicly-described operations in Table I of:
//   Think Fast: A Tensor Streaming Processor (TSP) for Accelerating Deep
//   Learning Workloads (ISCA 2020), Abts et al.
//
// These intrinsics intentionally model only an incremental IR milestone.
// Backend instruction definitions, lowering, and codegen are intentionally
// deferred to follow-up work.
//
// TODO(groq-tsp): The paper provides operation-level behavior but omits full
// encoding/ABI/lowering details for many operations. Keep semantics conservative
// at the IR layer until additional public specification is available.
//
//===----------------------------------------------------------------------===//

class GroqTSPIntrinsic<list<LLVMType> RetTypes, list<LLVMType> ArgTypes,
                       list<IntrinsicProperty> Properties = []>
    : DefaultAttrsIntrinsic<RetTypes, ArgTypes, Properties>;

//===----------------------------------------------------------------------===//
// Instruction control unit (ICU): NOP, Ifetch, Sync, Notify, Config, Repeat.
//===----------------------------------------------------------------------===//

def int_experimental_groq_tsp_icu_nop
    : GroqTSPIntrinsic<[], [llvm_i32_ty], [ImmArg<ArgIndex<0>>]>;
def int_experimental_groq_tsp_icu_ifetch
    : GroqTSPIntrinsic<[], [llvm_anyvector_ty]>;
def int_experimental_groq_tsp_icu_sync : GroqTSPIntrinsic<[], []>;
def int_experimental_groq_tsp_icu_notify : GroqTSPIntrinsic<[], []>;
def int_experimental_groq_tsp_icu_config
    : GroqTSPIntrinsic<[], [llvm_i32_ty], [ImmArg<ArgIndex<0>>]>;
def int_experimental_groq_tsp_icu_repeat
    : GroqTSPIntrinsic<[], [llvm_i32_ty, llvm_i32_ty],
                       [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;

//===----------------------------------------------------------------------===//
// Memory (MEM): Read, Write, Gather, Scatter.
//===----------------------------------------------------------------------===//

def int_experimental_groq_tsp_mem_read
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [llvm_anyptr_ty, llvm_i32_ty],
                       [IntrReadMem, ImmArg<ArgIndex<1>>]>;
def int_experimental_groq_tsp_mem_write
    : GroqTSPIntrinsic<[], [llvm_anyvector_ty, llvm_anyptr_ty, llvm_i32_ty],
                       [IntrWriteMem, ImmArg<ArgIndex<2>>]>;
def int_experimental_groq_tsp_mem_gather
    : GroqTSPIntrinsic<[llvm_anyvector_ty],
                       [llvm_anyptr_ty, llvm_anyvector_ty, llvm_i32_ty],
                       [IntrReadMem, ImmArg<ArgIndex<2>>]>;
def int_experimental_groq_tsp_mem_scatter
    : GroqTSPIntrinsic<[],
                       [llvm_anyvector_ty, llvm_anyptr_ty, llvm_anyvector_ty,
                        llvm_i32_ty],
                       [IntrWriteMem, ImmArg<ArgIndex<3>>]>;

//===----------------------------------------------------------------------===//
// Vector execution module (VXM):
// unary operation, binary operation, type conversions, ReLU, TanH, Exp, RSqrt.
//===----------------------------------------------------------------------===//

def int_experimental_groq_tsp_vxm_unary
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>, llvm_i32_ty],
                       [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_experimental_groq_tsp_vxm_binary
    : GroqTSPIntrinsic<[llvm_anyvector_ty],
                       [LLVMMatchType<0>, LLVMMatchType<0>, llvm_i32_ty],
                       [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_experimental_groq_tsp_vxm_type_convert
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [llvm_anyvector_ty, llvm_i32_ty],
                       [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_experimental_groq_tsp_vxm_relu
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>], [IntrNoMem]>;
def int_experimental_groq_tsp_vxm_tanh
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>], [IntrNoMem]>;
def int_experimental_groq_tsp_vxm_exp
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>], [IntrNoMem]>;
def int_experimental_groq_tsp_vxm_rsqrt
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>], [IntrNoMem]>;

//===----------------------------------------------------------------------===//
// Matrix execution module (MXM): LW, IW, ABC, ACC.
//===----------------------------------------------------------------------===//

def int_experimental_groq_tsp_mxm_lw
    : GroqTSPIntrinsic<[], [llvm_anyvector_ty, llvm_i32_ty],
                       [ImmArg<ArgIndex<1>>]>;
def int_experimental_groq_tsp_mxm_iw
    : GroqTSPIntrinsic<[], [llvm_anyvector_ty, llvm_i32_ty],
                       [ImmArg<ArgIndex<1>>]>;
def int_experimental_groq_tsp_mxm_abc
    : GroqTSPIntrinsic<[], [llvm_i32_ty], [ImmArg<ArgIndex<0>>]>;
def int_experimental_groq_tsp_mxm_acc
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>]>;

//===----------------------------------------------------------------------===//
// Switch execution module (SXM):
// Shift up/down, Permute, Distribute, Rotate stream, Transpose.
//===----------------------------------------------------------------------===//

def int_experimental_groq_tsp_sxm_shift_updown
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>, llvm_i32_ty],
                       [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_experimental_groq_tsp_sxm_permute
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>, llvm_anyvector_ty],
                       [IntrNoMem]>;
def int_experimental_groq_tsp_sxm_distribute
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>, llvm_anyvector_ty],
                       [IntrNoMem]>;
def int_experimental_groq_tsp_sxm_rotate_stream
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>, llvm_i32_ty],
                       [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_experimental_groq_tsp_sxm_transpose
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [LLVMMatchType<0>], [IntrNoMem]>;

//===----------------------------------------------------------------------===//
// Chip-to-chip (C2C): Deskew, Send, Receive.
//===----------------------------------------------------------------------===//

def int_experimental_groq_tsp_c2c_deskew
    : GroqTSPIntrinsic<[], [llvm_i32_ty], [ImmArg<ArgIndex<0>>]>;
def int_experimental_groq_tsp_c2c_send
    : GroqTSPIntrinsic<[], [llvm_anyvector_ty, llvm_i32_ty],
                       [ImmArg<ArgIndex<1>>]>;
def int_experimental_groq_tsp_c2c_receive
    : GroqTSPIntrinsic<[llvm_anyvector_ty], [llvm_i32_ty],
                       [ImmArg<ArgIndex<0>>]>;
