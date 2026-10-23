//===- GroqSubtarget.cpp - Groq Subtarget Information -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Groq specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "GroqSubtarget.h"
#include "GroqSelectionDAGInfo.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "groq-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "GroqGenSubtargetInfo.inc"

using namespace llvm;

GroqSubtarget &
GroqSubtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS) {
  StringRef CPUName = CPU;
  if (CPUName.empty()) {
    // set default cpu name
    CPUName = "generic";
  }

  // Parse features string.
  ParseSubtargetFeatures(CPUName, CPUName, FS);
  return *this;
}

GroqSubtarget::GroqSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                                 const TargetMachine &TM)
    : GroqGenSubtargetInfo(TT, CPU, /*TuneCPU=*/CPU, FS), TargetTriple(TT),
      InstrInfo(initializeSubtargetDependencies(CPU, FS)), TLInfo(TM, *this),
      FrameLowering(*this) {
  TSInfo = std::make_unique<SelectionDAGTargetInfo>();
}

GroqSubtarget::~GroqSubtarget() = default;

const SelectionDAGTargetInfo *GroqSubtarget::getSelectionDAGInfo() const {
  return TSInfo.get();
}
