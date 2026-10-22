//===- GroqOperators.td - Groq-specific operators ---------*- tblgen-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Type profiles
//===----------------------------------------------------------------------===//
def SDT_GroqCallSeqStart        : SDCallSeqStart<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_GroqCallSeqEnd          : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_GroqCall                : SDTypeProfile<0, -1, [SDTCisPtrTy<0>]>;

def SDT_GroqWrapPtr             : SDTypeProfile<1, 1,
                                                 [SDTCisSameAs<0, 1>,
                                                  SDTCisPtrTy<0>]>;

def SDT_GroqBrJT                : SDTypeProfile<0, 2,
                                                 [SDTCisPtrTy<0>, SDTCisVT<1, i32>]>;

def SDT_GroqSelectCC            : SDTypeProfile<1, 5,
                                                 [SDTCisSameAs<0, 3>, SDTCisSameAs<1, 2>,
                                                  SDTCisSameAs<3, 4>,
                                                  SDTCisVT<5, i32>]>;

def SDT_GroqCmp                 : SDTypeProfile<1, 2, [SDTCisVT<0, v1i1>, SDTCisVT<1, f32>, SDTCisVT<2, f32>]>;
def SDT_GroqMADD                : SDTypeProfile<1, 3, [SDTCisSameAs<0, 1>, SDTCisSameAs<0, 2>, SDTCisSameAs<0, 3>, SDTCisVT<0, f32>]>;
def SDT_GroqMOVS                : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisVT<0, f32>]>;
def SDT_GroqSelectCCFP          : SDTypeProfile<1, 6, [SDTCisSameAs<0, 3>, SDTCisSameAs<1, 2>, SDTCisSameAs<3, 4>,
                                                         SDTCisVT<5, i32>, SDTCisVT<6, i32>]>;

def SDT_GroqSRC                 : SDTypeProfile<1, 3, [SDTCisVT<0, i32>, SDTCisVT<1, i32>,
                                                         SDTCisVT<2, i32>, SDTCisVT<3, i32>]>;

def SDT_GroqEXTUI               : SDTypeProfile<1, 3, [SDTCisVT<0, i32>, SDTCisVT<1, i32>,
                                                         SDTCisVT<2, i32>, SDTCisVT<3, i32>]>;

def SDT_GroqMOVSP               : SDTypeProfile<0, 1, [SDTCisVT<0, i32>]>;

def SDT_GroqRUR                : SDTypeProfile<1, 1, [SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

//===----------------------------------------------------------------------===//
// Node definitions
//===----------------------------------------------------------------------===//
def Groq_call: SDNode<"GroqISD::CALL", SDT_GroqCall,
                       [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;

def Groq_callw8: SDNode<"GroqISD::CALLW8", SDT_GroqCall,
                        [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;

def Groq_ret: SDNode<"GroqISD::RET", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def Groq_retw: SDNode<"GroqISD::RETW", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def Groq_pcrel_wrapper: SDNode<"GroqISD::PCREL_WRAPPER", SDT_GroqWrapPtr, []>;

def Groq_callseq_start: SDNode<"ISD::CALLSEQ_START", SDT_GroqCallSeqStart,
                                [SDNPHasChain, SDNPSideEffect, SDNPOutGlue]>;

def Groq_callseq_end  : SDNode<"ISD::CALLSEQ_END",   SDT_GroqCallSeqEnd,
                                [SDNPHasChain, SDNPSideEffect, SDNPOptInGlue,
                                 SDNPOutGlue]>;

def Groq_brjt: SDNode<"GroqISD::BR_JT", SDT_GroqBrJT, [SDNPHasChain]>;

def Groq_select_cc: SDNode<"GroqISD::SELECT_CC", SDT_GroqSelectCC,
                            [SDNPInGlue]>;

def Groq_srcl: SDNode<"GroqISD::SRCL", SDT_GroqSRC>;

def Groq_srcr: SDNode<"GroqISD::SRCR", SDT_GroqSRC>;

def Groq_extui: SDNode<"GroqISD::EXTUI", SDT_GroqEXTUI>;

def Groq_movsp: SDNode<"GroqISD::MOVSP", SDT_GroqMOVSP,
                        [SDNPHasChain, SDNPSideEffect, SDNPInGlue]>;

def Groq_rur: SDNode<"GroqISD::RUR", SDT_GroqRUR,
                      [SDNPInGlue]>;

def Groq_cmpoeq     : SDNode<"GroqISD::CMPOEQ", SDT_GroqCmp, [SDNPOutGlue]>;
def Groq_cmpolt     : SDNode<"GroqISD::CMPOLT", SDT_GroqCmp, [SDNPOutGlue]>;
def Groq_cmpole     : SDNode<"GroqISD::CMPOLE", SDT_GroqCmp, [SDNPOutGlue]>;
def Groq_cmpueq     : SDNode<"GroqISD::CMPUEQ", SDT_GroqCmp, [SDNPOutGlue]>;
def Groq_cmpult     : SDNode<"GroqISD::CMPULT", SDT_GroqCmp, [SDNPOutGlue]>;
def Groq_cmpule     : SDNode<"GroqISD::CMPULE", SDT_GroqCmp, [SDNPOutGlue]>;
def Groq_cmpuo      : SDNode<"GroqISD::CMPUO", SDT_GroqCmp, [SDNPOutGlue]>;

def Groq_madd: SDNode<"GroqISD::MADD", SDT_GroqMADD, [SDNPInGlue]>;
def Groq_msub: SDNode<"GroqISD::MSUB", SDT_GroqMADD, [SDNPInGlue]>;
def Groq_movs: SDNode<"GroqISD::MOVS", SDT_GroqMOVS, [SDNPInGlue]>;

def Groq_select_cc_fp: SDNode<"GroqISD::SELECT_CC_FP", SDT_GroqSelectCCFP,
                               [SDNPInGlue]>;
