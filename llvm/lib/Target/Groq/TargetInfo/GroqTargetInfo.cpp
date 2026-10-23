//===-- GroqTargetInfo.cpp - Groq Target Implementation ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/GroqTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheGroqTarget() {
  static Target TheGroqTarget;
  return TheGroqTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeGroqTargetInfo() {
  RegisterTarget<Triple::groq> X(getTheGroqTarget(), "groq", "Groq 32",
                                   "GROQ");
}
