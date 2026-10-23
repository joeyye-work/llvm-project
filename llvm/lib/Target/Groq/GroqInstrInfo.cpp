//===- GroqInstrInfo.cpp - Groq Instruction Information ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Groq implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "GroqInstrInfo.h"
#include "GroqConstantPoolValue.h"
#include "GroqMachineFunctionInfo.h"
#include "GroqTargetMachine.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "GroqGenInstrInfo.inc"

using namespace llvm;

static const MachineInstrBuilder &
addFrameReference(const MachineInstrBuilder &MIB, int FI) {
  MachineInstr *MI = MIB;
  MachineFunction &MF = *MI->getParent()->getParent();
  MachineFrameInfo &MFFrame = MF.getFrameInfo();
  const MCInstrDesc &MCID = MI->getDesc();
  MachineMemOperand::Flags Flags = MachineMemOperand::MONone;
  if (MCID.mayLoad())
    Flags |= MachineMemOperand::MOLoad;
  if (MCID.mayStore())
    Flags |= MachineMemOperand::MOStore;
  int64_t Offset = 0;
  Align Alignment = MFFrame.getObjectAlign(FI);

  MachineMemOperand *MMO =
      MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(MF, FI, Offset),
                              Flags, MFFrame.getObjectSize(FI), Alignment);
  return MIB.addFrameIndex(FI).addImm(Offset).addMemOperand(MMO);
}

GroqInstrInfo::GroqInstrInfo(const GroqSubtarget &STI)
    : GroqGenInstrInfo(STI, RI, Groq::ADJCALLSTACKDOWN,
                         Groq::ADJCALLSTACKUP),
      RI(STI), STI(STI) {}

Register GroqInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                              int &FrameIndex) const {
  if (MI.getOpcode() == Groq::L32I) {
    if (MI.getOperand(1).isFI() && MI.getOperand(2).isImm() &&
        MI.getOperand(2).getImm() == 0) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
  }
  return Register();
}

Register GroqInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                             int &FrameIndex) const {
  if (MI.getOpcode() == Groq::S32I) {
    if (MI.getOperand(1).isFI() && MI.getOperand(2).isImm() &&
        MI.getOperand(2).getImm() == 0) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
  }
  return Register();
}

/// Adjust SP by Amount bytes.
void GroqInstrInfo::adjustStackPtr(MCRegister SP, int64_t Amount,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const {
  DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();

  if (Amount == 0)
    return;

  MachineRegisterInfo &RegInfo = MBB.getParent()->getRegInfo();
  const TargetRegisterClass *RC = &Groq::ARRegClass;

  // create virtual reg to store immediate
  MCRegister Reg = RegInfo.createVirtualRegister(RC);

  if (isInt<8>(Amount)) { // addi sp, sp, amount
    BuildMI(MBB, I, DL, get(Groq::ADDI), Reg).addReg(SP).addImm(Amount);
  } else { // Expand immediate that doesn't fit in 8-bit.
    MCRegister Reg1;
    loadImmediate(MBB, I, &Reg1, Amount);
    BuildMI(MBB, I, DL, get(Groq::ADD), Reg)
        .addReg(SP)
        .addReg(Reg1, RegState::Kill);
  }

  if (STI.isWindowedABI()) {
    BuildMI(MBB, I, DL, get(Groq::MOVSP), SP).addReg(Reg, RegState::Kill);
  } else {
    BuildMI(MBB, I, DL, get(Groq::OR), SP)
        .addReg(Reg, RegState::Kill)
        .addReg(Reg, RegState::Kill);
  }
}

void GroqInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MBBI,
                                  const DebugLoc &DL, Register DestReg,
                                  Register SrcReg, bool KillSrc,
                                  bool RenamableDest, bool RenamableSrc) const {
  unsigned Opcode;

  // The MOV instruction is not present in core ISA for AR registers,
  // so use OR instruction.
  if (Groq::ARRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MBBI, DL, get(Groq::OR), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc))
        .addReg(SrcReg, getKillRegState(KillSrc));
    return;
  }

  if (STI.hasSingleFloat() && Groq::FPRRegClass.contains(SrcReg) &&
      Groq::FPRRegClass.contains(DestReg))
    Opcode = Groq::MOV_S;
  else if (STI.hasSingleFloat() && Groq::FPRRegClass.contains(SrcReg) &&
           Groq::ARRegClass.contains(DestReg))
    Opcode = Groq::RFR;
  else if (STI.hasSingleFloat() && Groq::ARRegClass.contains(SrcReg) &&
           Groq::FPRRegClass.contains(DestReg))
    Opcode = Groq::WFR;
  else
    report_fatal_error("Impossible reg-to-reg copy");

  BuildMI(MBB, MBBI, DL, get(Opcode), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
}

void GroqInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register SrcReg,
    bool isKill, int FrameIdx, const TargetRegisterClass *RC,

    Register VReg, MachineInstr::MIFlag Flags) const {
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  unsigned LoadOpcode, StoreOpcode;
  getLoadStoreOpcodes(RC, LoadOpcode, StoreOpcode, FrameIdx);
  MachineInstrBuilder MIB = BuildMI(MBB, MBBI, DL, get(StoreOpcode))
                                .addReg(SrcReg, getKillRegState(isKill));
  addFrameReference(MIB, FrameIdx);
}

void GroqInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator MBBI,
                                           Register DestReg, int FrameIdx,
                                           const TargetRegisterClass *RC,
                                           Register VReg, unsigned SubReg,
                                           MachineInstr::MIFlag Flags) const {
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  unsigned LoadOpcode, StoreOpcode;
  getLoadStoreOpcodes(RC, LoadOpcode, StoreOpcode, FrameIdx);
  addFrameReference(BuildMI(MBB, MBBI, DL, get(LoadOpcode), DestReg), FrameIdx);
}

void GroqInstrInfo::getLoadStoreOpcodes(const TargetRegisterClass *RC,
                                          unsigned &LoadOpcode,
                                          unsigned &StoreOpcode,
                                          int64_t offset) const {
  if (RC == &Groq::ARRegClass) {
    LoadOpcode = Groq::L32I;
    StoreOpcode = Groq::S32I;
  } else if (RC == &Groq::FPRRegClass) {
    LoadOpcode = Groq::LSI;
    StoreOpcode = Groq::SSI;
  } else {
    llvm_unreachable("Unsupported regclass to load or store");
  }
}

void GroqInstrInfo::loadImmediate(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MBBI,
                                    MCRegister *Reg, int64_t Value) const {
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  MachineRegisterInfo &RegInfo = MBB.getParent()->getRegInfo();
  const TargetRegisterClass *RC = &Groq::ARRegClass;

  // create virtual reg to store immediate
  *Reg = RegInfo.createVirtualRegister(RC);
  if (Value >= -2048 && Value <= 2047) {
    BuildMI(MBB, MBBI, DL, get(Groq::MOVI), *Reg).addImm(Value);
  } else if (Value >= -32768 && Value <= 32767) {
    int Low = Value & 0xFF;
    int High = Value & ~0xFF;

    BuildMI(MBB, MBBI, DL, get(Groq::MOVI), *Reg).addImm(Low);
    BuildMI(MBB, MBBI, DL, get(Groq::ADDMI), *Reg).addReg(*Reg).addImm(High);
  } else if (Value >= -4294967296LL && Value <= 4294967295LL) {
    // 32 bit arbitrary constant
    MachineConstantPool *MCP = MBB.getParent()->getConstantPool();
    uint64_t UVal = ((uint64_t)Value) & 0xFFFFFFFFLL;
    const Constant *CVal = ConstantInt::get(
        Type::getInt32Ty(MBB.getParent()->getFunction().getContext()), UVal,
        false);
    unsigned Idx = MCP->getConstantPoolIndex(CVal, Align(2U));
    //	MCSymbol MSym
    BuildMI(MBB, MBBI, DL, get(Groq::L32R), *Reg).addConstantPoolIndex(Idx);
  } else {
    // use L32R to let assembler load immediate best
    // TODO replace to L32R
    report_fatal_error("Unsupported load immediate value");
  }
}

unsigned GroqInstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  case TargetOpcode::INLINEASM: { // Inline Asm: Variable size.
    const MachineFunction *MF = MI.getParent()->getParent();
    const char *AsmStr = MI.getOperand(0).getSymbolName();
    return getInlineAsmLength(AsmStr, MF->getTarget().getMCAsmInfo());
  }
  case TargetOpcode::BUNDLE:
    return getInstBundleSize(MI);
  default:
    return MI.getDesc().getSize();
  }
}

bool GroqInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() <= 4 && "Invalid branch condition!");

  switch (Cond[0].getImm()) {
  case Groq::BEQ:
    Cond[0].setImm(Groq::BNE);
    return false;
  case Groq::BNE:
    Cond[0].setImm(Groq::BEQ);
    return false;
  case Groq::BLT:
    Cond[0].setImm(Groq::BGE);
    return false;
  case Groq::BGE:
    Cond[0].setImm(Groq::BLT);
    return false;
  case Groq::BLTU:
    Cond[0].setImm(Groq::BGEU);
    return false;
  case Groq::BGEU:
    Cond[0].setImm(Groq::BLTU);
    return false;
  case Groq::BEQI:
    Cond[0].setImm(Groq::BNEI);
    return false;
  case Groq::BNEI:
    Cond[0].setImm(Groq::BEQI);
    return false;
  case Groq::BGEI:
    Cond[0].setImm(Groq::BLTI);
    return false;
  case Groq::BLTI:
    Cond[0].setImm(Groq::BGEI);
    return false;
  case Groq::BGEUI:
    Cond[0].setImm(Groq::BLTUI);
    return false;
  case Groq::BLTUI:
    Cond[0].setImm(Groq::BGEUI);
    return false;
  case Groq::BEQZ:
    Cond[0].setImm(Groq::BNEZ);
    return false;
  case Groq::BNEZ:
    Cond[0].setImm(Groq::BEQZ);
    return false;
  case Groq::BLTZ:
    Cond[0].setImm(Groq::BGEZ);
    return false;
  case Groq::BGEZ:
    Cond[0].setImm(Groq::BLTZ);
    return false;
  case Groq::BF:
    Cond[0].setImm(Groq::BT);
    return false;
  case Groq::BT:
    Cond[0].setImm(Groq::BF);
    return false;
  default:
    report_fatal_error("Invalid branch condition!");
  }
}

MachineBasicBlock *
GroqInstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  unsigned OpCode = MI.getOpcode();
  switch (OpCode) {
  case Groq::BR_JT:
  case Groq::JX:
    return nullptr;
  case Groq::J:
    return MI.getOperand(0).getMBB();
  case Groq::BEQ:
  case Groq::BNE:
  case Groq::BLT:
  case Groq::BLTU:
  case Groq::BGE:
  case Groq::BGEU:
    return MI.getOperand(2).getMBB();
  case Groq::BEQI:
  case Groq::BNEI:
  case Groq::BLTI:
  case Groq::BLTUI:
  case Groq::BGEI:
  case Groq::BGEUI:
    return MI.getOperand(2).getMBB();
  case Groq::BEQZ:
  case Groq::BNEZ:
  case Groq::BLTZ:
  case Groq::BGEZ:
    return MI.getOperand(1).getMBB();
  case Groq::BT:
  case Groq::BF:
    return MI.getOperand(1).getMBB();
  default:
    llvm_unreachable("Unknown branch opcode");
  }
}

bool GroqInstrInfo::isBranchOffsetInRange(unsigned BranchOp,
                                            int64_t BrOffset) const {
  switch (BranchOp) {
  case Groq::J:
    BrOffset -= 4;
    return isIntN(18, BrOffset);
  case Groq::JX:
    return true;
  case Groq::BR_JT:
    return true;
  case Groq::BEQ:
  case Groq::BNE:
  case Groq::BLT:
  case Groq::BLTU:
  case Groq::BGE:
  case Groq::BGEU:
  case Groq::BEQI:
  case Groq::BNEI:
  case Groq::BLTI:
  case Groq::BLTUI:
  case Groq::BGEI:
  case Groq::BGEUI:
    BrOffset -= 4;
    return isIntN(8, BrOffset);
  case Groq::BEQZ:
  case Groq::BNEZ:
  case Groq::BLTZ:
  case Groq::BGEZ:
    BrOffset -= 4;
    return isIntN(12, BrOffset);
  case Groq::BT:
  case Groq::BF:
    BrOffset -= 4;
    return isIntN(8, BrOffset);
  default:
    llvm_unreachable("Unknown branch opcode");
  }
}

bool GroqInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *&TBB,
                                    MachineBasicBlock *&FBB,
                                    SmallVectorImpl<MachineOperand> &Cond,
                                    bool AllowModify = false) const {
  // Most of the code and comments here are boilerplate.

  // Start from the bottom of the block and work up, examining the
  // terminator instructions.
  MachineBasicBlock::iterator I = MBB.end();
  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;

    // Working from the bottom, when we see a non-terminator instruction, we're
    // done.
    if (!isUnpredicatedTerminator(*I))
      break;

    // A terminator that isn't a branch can't easily be handled by this
    // analysis.
    SmallVector<MachineOperand, 4> ThisCond;
    ThisCond.push_back(MachineOperand::CreateImm(0));
    const MachineOperand *ThisTarget;
    if (!isBranch(I, ThisCond, ThisTarget))
      return true;

    // Can't handle indirect branches.
    if (!ThisTarget->isMBB())
      return true;

    if (ThisCond[0].getImm() == Groq::J) {
      // Handle unconditional branches.
      if (!AllowModify) {
        TBB = ThisTarget->getMBB();
        continue;
      }

      // If the block has any instructions after a JMP, delete them.
      while (std::next(I) != MBB.end())
        std::next(I)->eraseFromParent();

      Cond.clear();
      FBB = 0;

      // TBB is used to indicate the unconditinal destination.
      TBB = ThisTarget->getMBB();
      continue;
    }

    // Working from the bottom, handle the first conditional branch.
    if (Cond.empty()) {
      // FIXME: add X86-style branch swap
      FBB = TBB;
      TBB = ThisTarget->getMBB();
      Cond.push_back(MachineOperand::CreateImm(ThisCond[0].getImm()));

      // push remaining operands
      for (unsigned int i = 0; i < (I->getNumExplicitOperands() - 1); i++)
        Cond.push_back(I->getOperand(i));

      continue;
    }

    // Handle subsequent conditional branches.
    assert(Cond.size() <= 4);
    assert(TBB);

    // Only handle the case where all conditional branches branch to the same
    // destination.
    if (TBB != ThisTarget->getMBB())
      return true;

    // If the conditions are the same, we can leave them alone.
    unsigned OldCond = Cond[0].getImm();
    if (OldCond == ThisCond[0].getImm())
      continue;
  }

  return false;
}

unsigned GroqInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                       int *BytesRemoved) const {
  // Most of the code and comments here are boilerplate.
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;
  if (BytesRemoved)
    *BytesRemoved = 0;

  while (I != MBB.begin()) {
    --I;
    SmallVector<MachineOperand, 4> Cond;
    Cond.push_back(MachineOperand::CreateImm(0));
    const MachineOperand *Target;
    if (!isBranch(I, Cond, Target))
      break;
    if (!Target->isMBB())
      break;
    // Remove the branch.
    if (BytesRemoved)
      *BytesRemoved += getInstSizeInBytes(*I);
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }
  return Count;
}

unsigned GroqInstrInfo::insertBranch(
    MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB,
    ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {
  unsigned Count = 0;
  if (BytesAdded)
    *BytesAdded = 0;
  if (FBB) {
    // Need to build two branches then
    // one to branch to TBB on Cond
    // and a second one immediately after to unconditionally jump to FBB
    Count = insertBranchAtInst(MBB, MBB.end(), TBB, Cond, DL, BytesAdded);
    auto &MI = *BuildMI(&MBB, DL, get(Groq::J)).addMBB(FBB);
    Count++;
    if (BytesAdded)
      *BytesAdded += getInstSizeInBytes(MI);
    return Count;
  }
  // This function inserts the branch at the end of the MBB
  Count += insertBranchAtInst(MBB, MBB.end(), TBB, Cond, DL, BytesAdded);
  return Count;
}

void GroqInstrInfo::insertIndirectBranch(MachineBasicBlock &MBB,
                                           MachineBasicBlock &DestBB,
                                           MachineBasicBlock &RestoreBB,
                                           const DebugLoc &DL, int64_t BrOffset,
                                           RegScavenger *RS) const {
  assert(RS && "RegScavenger required for long branching");
  assert(MBB.empty() &&
         "new block should be inserted for expanding unconditional branch");
  assert(MBB.pred_size() == 1);

  MachineFunction *MF = MBB.getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  MachineConstantPool *ConstantPool = MF->getConstantPool();
  auto *GroqFI = MF->getInfo<GroqMachineFunctionInfo>();
  MachineBasicBlock *JumpToMBB = &DestBB;

  if (!isInt<32>(BrOffset))
    report_fatal_error(
        "Branch offsets outside of the signed 32-bit range not supported");

  Register ScratchReg = MRI.createVirtualRegister(&Groq::ARRegClass);
  auto II = MBB.end();

  // Create l32r without last operand. We will add this operand later when
  // JumpToMMB will be calculated and placed to the ConstantPool.
  MachineInstr &L32R = *BuildMI(MBB, II, DL, get(Groq::L32R), ScratchReg);
  BuildMI(MBB, II, DL, get(Groq::JX)).addReg(ScratchReg, RegState::Kill);

  RS->enterBasicBlockEnd(MBB);
  Register ScavRegister =
      RS->scavengeRegisterBackwards(Groq::ARRegClass, L32R.getIterator(),
                                    /*RestoreAfter=*/false, /*SpAdj=*/0,
                                    /*AllowSpill=*/false);
  if (ScavRegister != Groq::NoRegister)
    RS->setRegUsed(ScavRegister);
  else {
    // The case when there is no scavenged register needs special handling.
    // Pick A8 because it doesn't make a difference
    ScavRegister = Groq::A12;

    int FrameIndex = GroqFI->getBranchRelaxationScratchFrameIndex();
    if (FrameIndex == -1)
      report_fatal_error(
          "Unable to properly handle scavenged register for indirect jump, "
          "function code size is significantly larger than estimated");

    storeRegToStackSlot(MBB, L32R, ScavRegister, /*IsKill=*/true, FrameIndex,
                        &Groq::ARRegClass, Register());
    RI.eliminateFrameIndex(std::prev(L32R.getIterator()),
                           /*SpAdj=*/0, /*FIOperandNum=*/1);

    loadRegFromStackSlot(RestoreBB, RestoreBB.end(), ScavRegister, FrameIndex,
                         &Groq::ARRegClass, Register());
    RI.eliminateFrameIndex(RestoreBB.back(),
                           /*SpAdj=*/0, /*FIOperandNum=*/1);
    JumpToMBB = &RestoreBB;
  }

  unsigned LabelId = GroqFI->createCPLabelId();

  GroqConstantPoolValue *C = GroqConstantPoolMBB::Create(
      MF->getFunction().getContext(), JumpToMBB, LabelId);
  unsigned Idx = ConstantPool->getConstantPoolIndex(C, Align(4));
  L32R.addOperand(MachineOperand::CreateCPI(Idx, 0));

  MRI.replaceRegWith(ScratchReg, ScavRegister);
  MRI.clearVirtRegs();
}

unsigned GroqInstrInfo::insertConstBranchAtInst(
    MachineBasicBlock &MBB, MachineInstr *I, int64_t offset,
    ArrayRef<MachineOperand> Cond, DebugLoc DL, int *BytesAdded) const {
  assert(Cond.size() <= 4 &&
         "Groq branch conditions have less than four components!");

  if (Cond.empty() || (Cond[0].getImm() == Groq::J)) {
    // Unconditional branch
    MachineInstr *MI = BuildMI(MBB, I, DL, get(Groq::J)).addImm(offset);
    if (BytesAdded && MI)
      *BytesAdded += getInstSizeInBytes(*MI);
    return 1;
  }

  unsigned Count = 0;
  unsigned BR_C = Cond[0].getImm();
  MachineInstr *MI = nullptr;
  switch (BR_C) {
  case Groq::BEQ:
  case Groq::BNE:
  case Groq::BLT:
  case Groq::BLTU:
  case Groq::BGE:
  case Groq::BGEU:
    MI = BuildMI(MBB, I, DL, get(BR_C))
             .addImm(offset)
             .addReg(Cond[1].getReg())
             .addReg(Cond[2].getReg());
    break;
  case Groq::BEQI:
  case Groq::BNEI:
  case Groq::BLTI:
  case Groq::BLTUI:
  case Groq::BGEI:
  case Groq::BGEUI:
    MI = BuildMI(MBB, I, DL, get(BR_C))
             .addImm(offset)
             .addReg(Cond[1].getReg())
             .addImm(Cond[2].getImm());
    break;
  case Groq::BEQZ:
  case Groq::BNEZ:
  case Groq::BLTZ:
  case Groq::BGEZ:
    MI = BuildMI(MBB, I, DL, get(BR_C)).addImm(offset).addReg(Cond[1].getReg());
    break;
  case Groq::BT:
  case Groq::BF:
    MI = BuildMI(MBB, I, DL, get(BR_C)).addImm(offset).addReg(Cond[1].getReg());
    break;
  default:
    llvm_unreachable("Invalid branch type!");
  }
  if (BytesAdded && MI)
    *BytesAdded += getInstSizeInBytes(*MI);
  ++Count;
  return Count;
}

unsigned GroqInstrInfo::insertBranchAtInst(MachineBasicBlock &MBB,
                                             MachineBasicBlock::iterator I,
                                             MachineBasicBlock *TBB,
                                             ArrayRef<MachineOperand> Cond,
                                             const DebugLoc &DL,
                                             int *BytesAdded) const {
  // Shouldn't be a fall through.
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");
  assert(Cond.size() <= 4 &&
         "Groq branch conditions have less than four components!");

  if (Cond.empty() || (Cond[0].getImm() == Groq::J)) {
    // Unconditional branch
    MachineInstr *MI = BuildMI(MBB, I, DL, get(Groq::J)).addMBB(TBB);
    if (BytesAdded && MI)
      *BytesAdded += getInstSizeInBytes(*MI);
    return 1;
  }

  unsigned Count = 0;
  unsigned BR_C = Cond[0].getImm();
  MachineInstr *MI = nullptr;
  switch (BR_C) {
  case Groq::BEQ:
  case Groq::BNE:
  case Groq::BLT:
  case Groq::BLTU:
  case Groq::BGE:
  case Groq::BGEU:
    MI = BuildMI(MBB, I, DL, get(BR_C))
             .addReg(Cond[1].getReg())
             .addReg(Cond[2].getReg())
             .addMBB(TBB);
    break;
  case Groq::BEQI:
  case Groq::BNEI:
  case Groq::BLTI:
  case Groq::BLTUI:
  case Groq::BGEI:
  case Groq::BGEUI:
    MI = BuildMI(MBB, I, DL, get(BR_C))
             .addReg(Cond[1].getReg())
             .addImm(Cond[2].getImm())
             .addMBB(TBB);
    break;
  case Groq::BEQZ:
  case Groq::BNEZ:
  case Groq::BLTZ:
  case Groq::BGEZ:
    MI = BuildMI(MBB, I, DL, get(BR_C)).addReg(Cond[1].getReg()).addMBB(TBB);
    break;
  case Groq::BT:
  case Groq::BF:
    MI = BuildMI(MBB, I, DL, get(BR_C)).addReg(Cond[1].getReg()).addMBB(TBB);
    break;
  default:
    report_fatal_error("Invalid branch type!");
  }
  if (BytesAdded && MI)
    *BytesAdded += getInstSizeInBytes(*MI);
  ++Count;
  return Count;
}

bool GroqInstrInfo::isBranch(const MachineBasicBlock::iterator &MI,
                               SmallVectorImpl<MachineOperand> &Cond,
                               const MachineOperand *&Target) const {
  unsigned OpCode = MI->getOpcode();
  switch (OpCode) {
  case Groq::J:
  case Groq::JX:
  case Groq::BR_JT:
    Cond[0].setImm(OpCode);
    Target = &MI->getOperand(0);
    return true;
  case Groq::BEQ:
  case Groq::BNE:
  case Groq::BLT:
  case Groq::BLTU:
  case Groq::BGE:
  case Groq::BGEU:
    Cond[0].setImm(OpCode);
    Target = &MI->getOperand(2);
    return true;

  case Groq::BEQI:
  case Groq::BNEI:
  case Groq::BLTI:
  case Groq::BLTUI:
  case Groq::BGEI:
  case Groq::BGEUI:
    Cond[0].setImm(OpCode);
    Target = &MI->getOperand(2);
    return true;

  case Groq::BEQZ:
  case Groq::BNEZ:
  case Groq::BLTZ:
  case Groq::BGEZ:
    Cond[0].setImm(OpCode);
    Target = &MI->getOperand(1);
    return true;

  case Groq::BT:
  case Groq::BF:
    Cond[0].setImm(OpCode);
    Target = &MI->getOperand(1);
    return true;

  default:
    assert(!MI->getDesc().isBranch() && "Unknown branch opcode");
    return false;
  }
}
