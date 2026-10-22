//===- GroqTargetMachine.cpp - Define TargetMachine for Groq ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the info about Groq target spec.
//
//===----------------------------------------------------------------------===//

#include "GroqTargetMachine.h"
#include "TargetInfo/GroqTargetInfo.h"
#include "GroqMachineFunctionInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/PassRegistry.h"
#include "llvm/Transforms/Scalar.h"
#include <optional>

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeGroqTarget() {
  // Register the target.
  RegisterTargetMachine<GroqTargetMachine> A(getTheGroqTarget());
  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeGroqAsmPrinterPass(PR);
}

static Reloc::Model getEffectiveRelocModel(bool JIT,
                                           std::optional<Reloc::Model> RM) {
  if (!RM || JIT)
     return Reloc::Static;
  return *RM;
}

GroqTargetMachine::GroqTargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT,
                                         bool IsLittle)
    : CodeGenTargetMachineImpl(T, TT, CPU, FS, Options,
                               getEffectiveRelocModel(JIT, RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()) {
  initAsmInfo();
}

GroqTargetMachine::GroqTargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT)
    : GroqTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT, true) {}

const GroqSubtarget *
GroqTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  auto CPU = CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;
  auto FS = FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;

  auto &I = SubtargetMap[CPU + FS];
  if (!I)
    I = std::make_unique<GroqSubtarget>(TargetTriple, CPU, FS, *this);
  return I.get();
}

MachineFunctionInfo *GroqTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return GroqMachineFunctionInfo::create<GroqMachineFunctionInfo>(Allocator,
                                                                      F, STI);
}

namespace {
/// Groq Code Generator Pass Configuration Options.
class GroqPassConfig : public TargetPassConfig {
public:
  GroqPassConfig(GroqTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  GroqTargetMachine &getGroqTargetMachine() const {
    return getTM<GroqTargetMachine>();
  }

  bool addInstSelector() override;
  void addIRPasses() override;
  void addPreEmitPass() override;
};
} // end anonymous namespace

bool GroqPassConfig::addInstSelector() {
  addPass(createGroqISelDag(getGroqTargetMachine(), getOptLevel()));
  return false;
}

void GroqPassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());
  TargetPassConfig::addIRPasses();
}

void GroqPassConfig::addPreEmitPass() { addPass(&BranchRelaxationPassID); }

TargetPassConfig *GroqTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new GroqPassConfig(*this, PM);
}
