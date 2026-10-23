//===- GroqRegisterInfo.cpp - Groq Register Information ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Groq implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "GroqRegisterInfo.h"
#include "MCTargetDesc/GroqMCTargetDesc.h"
#include "GroqInstrInfo.h"
#include "GroqSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "groq-reg-info"

#define GET_REGINFO_TARGET_DESC
#include "GroqGenRegisterInfo.inc"

using namespace llvm;

GroqRegisterInfo::GroqRegisterInfo(const GroqSubtarget &STI)
    : GroqGenRegisterInfo(Groq::A0), Subtarget(STI) {}

const uint16_t *
GroqRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return Subtarget.isWindowedABI() ? CSRW8_Groq_SaveList
                                   : CSR_Groq_SaveList;
}

const uint32_t *
GroqRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                         CallingConv::ID) const {
  return Subtarget.isWindowedABI() ? CSRW8_Groq_RegMask : CSR_Groq_RegMask;
}

BitVector GroqRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  Reserved.set(Groq::A0);
  if (TFI->hasFP(MF)) {
    // Reserve frame pointer.
    Reserved.set(getFrameRegister(MF));
  }
  if (Subtarget.hasTHREADPTR()) {
    // Reserve frame pointer.
    Reserved.set(Groq::THREADPTR);
  }
  // Reserve stack pointer.
  Reserved.set(Groq::SP);
  // Reserve QR regs
  Reserved.set(Groq::Q0);
  Reserved.set(Groq::Q1);
  Reserved.set(Groq::Q2);
  Reserved.set(Groq::Q3);
  Reserved.set(Groq::Q4);
  Reserved.set(Groq::Q5);
  Reserved.set(Groq::Q6);
  Reserved.set(Groq::Q7);
  return Reserved;
}

bool GroqRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                             int SPAdj, unsigned FIOperandNum,
                                             RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  uint64_t StackSize = MF.getFrameInfo().getStackSize();
  int64_t SPOffset = MF.getFrameInfo().getObjectOffset(FrameIndex);
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();
  int MinCSFI = 0;
  int MaxCSFI = -1;

  if (CSI.size()) {
    MinCSFI = CSI[0].getFrameIdx();
    MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
  }
  // The following stack frame objects are always referenced relative to $sp:
  //  1. Outgoing arguments.
  //  2. Pointer to dynamically allocated stack space.
  //  3. Locations for callee-saved registers.
  //  4. Locations for eh data registers.
  // Everything else is referenced relative to whatever register
  // getFrameRegister() returns.
  MCRegister FrameReg;
  if ((FrameIndex >= MinCSFI && FrameIndex <= MaxCSFI))
    FrameReg = Groq::SP;
  else
    FrameReg = getFrameRegister(MF);

  // Calculate final offset.
  // - There is no need to change the offset if the frame object is one of the
  //   following: an outgoing argument, pointer to a dynamically allocated
  //   stack space or a $gp restore location,
  // - If the frame object is any of the following, its offset must be adjusted
  //   by adding the size of the stack:
  //   incoming argument, callee-saved register location or local variable.
  bool IsKill = false;
  int64_t Offset =
      SPOffset + (int64_t)StackSize + MI.getOperand(FIOperandNum + 1).getImm();

  bool Valid = Groq::isValidAddrOffsetForOpcode(MI.getOpcode(), Offset);

  // If MI is not a debug value, make sure Offset fits in the 16-bit immediate
  // field.
  if (!MI.isDebugValue() && !Valid) {
    MachineBasicBlock &MBB = *MI.getParent();
    DebugLoc DL = II->getDebugLoc();
    unsigned ADD = Groq::ADD;
    MCRegister Reg;
    const GroqInstrInfo &TII = *static_cast<const GroqInstrInfo *>(
        MBB.getParent()->getSubtarget().getInstrInfo());

    TII.loadImmediate(MBB, II, &Reg, Offset);
    BuildMI(MBB, II, DL, TII.get(ADD), Reg)
        .addReg(FrameReg)
        .addReg(Reg, RegState::Kill);

    FrameReg = Reg;
    Offset = 0;
    IsKill = true;
  }

  MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, false, false, IsKill);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);

  return false;
}

Register GroqRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
  return TFI->hasFP(MF) ? (Subtarget.isWindowedABI() ? Groq::A7 : Groq::A15)
                        : Groq::SP;
}
