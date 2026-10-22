//===- GroqISelLowering.cpp - Groq DAG Lowering Implementation --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Groq uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "GroqISelLowering.h"
#include "GroqConstantPoolValue.h"
#include "GroqInstrInfo.h"
#include "GroqMachineFunctionInfo.h"
#include "GroqSelectionDAGInfo.h"
#include "GroqSubtarget.h"
#include "GroqTargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <deque>

using namespace llvm;

#define DEBUG_TYPE "groq-lower"

// Return true if we must use long (in fact, indirect) function call.
// It's simplified version, production implimentation must
// resolve a functions in ROM (usually glibc functions)
static bool isLongCall(const char *str) {
  // Currently always use long calls
  return true;
}

// The calling conventions in GroqCallingConv.td are described in terms of the
// callee's register window. This function translates registers to the
// corresponding caller window %o register.
static unsigned toCallerWindow(unsigned Reg) {
  if (Reg >= Groq::A2 && Reg <= Groq::A7)
    return Reg - Groq::A2 + Groq::A10;
  return Reg;
}

GroqTargetLowering::GroqTargetLowering(const TargetMachine &TM,
                                           const GroqSubtarget &STI)
    : TargetLowering(TM, STI), Subtarget(STI) {
  MVT PtrVT = MVT::i32;
  // Set up the register classes.
  addRegisterClass(MVT::i32, &Groq::ARRegClass);

  if (Subtarget.hasSingleFloat()) {
    addRegisterClass(MVT::f32, &Groq::FPRRegClass);
  }

  if (Subtarget.hasBoolean()) {
    addRegisterClass(MVT::v1i1, &Groq::BRRegClass);
  }

  // Set up special registers.
  setStackPointerRegisterToSaveRestore(Groq::SP);

  setSchedulingPreference(Sched::RegPressure);

  setMinFunctionAlignment(Align(4));

  setOperationAction(ISD::Constant, MVT::i32, Custom);
  setOperationAction(ISD::Constant, MVT::i64, Expand);
  setOperationAction(ISD::ConstantFP, MVT::f32, Expand);
  setOperationAction(ISD::ConstantFP, MVT::f64, Expand);

  setBooleanContents(ZeroOrOneBooleanContent);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, {MVT::i8, MVT::i16},
                     Subtarget.hasSEXT() ? Legal : Expand);

  setOperationAction(ISD::BITCAST, MVT::i32, Expand);
  setOperationAction(ISD::BITCAST, MVT::f32, Expand);
  setOperationAction(ISD::UINT_TO_FP, MVT::i32, Expand);
  setOperationAction(ISD::SINT_TO_FP, MVT::i32, Expand);
  setOperationAction(ISD::FP_TO_UINT, MVT::i32, Expand);
  setOperationAction(ISD::FP_TO_SINT, MVT::i32, Expand);

  // No sign extend instructions for i1 and sign extend load i8
  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i8, Expand);
  }

  setOperationAction(ISD::ConstantPool, PtrVT, Custom);
  setOperationAction(ISD::GlobalAddress, PtrVT, Custom);
  setOperationAction(ISD::GlobalTLSAddress, PtrVT, Custom);
  setOperationAction(ISD::BlockAddress, PtrVT, Custom);
  setOperationAction(ISD::JumpTable, PtrVT, Custom);

  // Expand jump table branches as address arithmetic followed by an
  // indirect jump.
  setOperationAction(ISD::BR_JT, MVT::Other, Custom);

  setOperationAction({ISD::TRAP, ISD::DEBUGTRAP}, MVT::Other, Legal);

  setOperationAction(ISD::BR_CC, MVT::i32, Legal);
  setOperationAction(ISD::BR_CC, MVT::i64, Expand);

  setOperationAction(ISD::SELECT, MVT::i32, Expand);
  setOperationAction(ISD::SELECT, MVT::f32, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);

  if (Subtarget.hasSingleFloat()) {
    setOperationAction(ISD::BR_CC, MVT::f32, Legal);
    setOperationAction(ISD::SELECT_CC, MVT::f32, Custom);
  } else {
    setOperationAction(ISD::BR_CC, MVT::f32, Expand);
    setOperationAction(ISD::SELECT_CC, MVT::f32, Expand);
  }

  setOperationAction(ISD::SETCC, MVT::i32, Expand);
  setOperationAction(ISD::SETCC, MVT::f32, Expand);

  setCondCodeAction(ISD::SETGT, MVT::i32, Expand);
  setCondCodeAction(ISD::SETLE, MVT::i32, Expand);
  setCondCodeAction(ISD::SETUGT, MVT::i32, Expand);
  setCondCodeAction(ISD::SETULE, MVT::i32, Expand);

  if (Subtarget.hasMul32())
    setOperationAction(ISD::MUL, MVT::i32, Legal);
  else
    setOperationAction(ISD::MUL, MVT::i32, Expand);

  if (Subtarget.hasMul32High()) {
    setOperationAction(ISD::MULHU, MVT::i32, Legal);
    setOperationAction(ISD::MULHS, MVT::i32, Legal);
  } else {
    setOperationAction(ISD::MULHU, MVT::i32, Expand);
    setOperationAction(ISD::MULHS, MVT::i32, Expand);
  }

  setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);

  if (Subtarget.hasDiv32()) {
    setOperationAction(ISD::SDIV, MVT::i32, Legal);
    setOperationAction(ISD::UDIV, MVT::i32, Legal);
    setOperationAction(ISD::SREM, MVT::i32, Legal);
    setOperationAction(ISD::UREM, MVT::i32, Legal);
  } else {
    setOperationAction(ISD::SDIV, MVT::i32, Expand);
    setOperationAction(ISD::UDIV, MVT::i32, Expand);
    setOperationAction(ISD::SREM, MVT::i32, Expand);
    setOperationAction(ISD::UREM, MVT::i32, Expand);
  }

  setOperationAction(ISD::SDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i32, Expand);

  setOperationAction(ISD::SHL_PARTS, MVT::i32, Custom);
  setOperationAction(ISD::SRA_PARTS, MVT::i32, Custom);
  setOperationAction(ISD::SRL_PARTS, MVT::i32, Custom);

  setOperationAction(ISD::BSWAP, MVT::i32, Expand);
  setOperationAction(ISD::ROTL, MVT::i32, Expand);
  setOperationAction(ISD::ROTR, MVT::i32, Expand);
  setOperationAction(ISD::CTPOP, MVT::i32, Custom);
  setOperationAction(ISD::CTTZ, MVT::i32, Expand);
  setOperationAction(ISD::CTLZ, MVT::i32, Expand);
  setOperationAction(ISD::CTTZ_ZERO_POISON, MVT::i32, Expand);
  setOperationAction(ISD::CTLZ_ZERO_POISON, MVT::i32, Expand);

  setOperationAction({ISD::SMIN, ISD::SMAX, ISD::UMIN, ISD::UMAX}, MVT::i32,
                     Subtarget.hasMINMAX() ? Legal : Expand);

  // Implement custom stack allocations
  setOperationAction(ISD::DYNAMIC_STACKALLOC, PtrVT, Custom);
  // Implement custom stack save and restore
  setOperationAction(ISD::STACKSAVE, MVT::Other, Custom);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Custom);

  // VASTART, VAARG and VACOPY need to deal with the Groq-specific varargs
  // structure, but VAEND is a no-op.
  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction(ISD::VAARG, MVT::Other, Custom);
  setOperationAction(ISD::VACOPY, MVT::Other, Custom);
  setOperationAction(ISD::VAEND, MVT::Other, Expand);

  // Handle floating-point types.
  for (unsigned I = MVT::FIRST_FP_VALUETYPE; I <= MVT::LAST_FP_VALUETYPE; ++I) {
    MVT VT = MVT::SimpleValueType(I);
    if (isTypeLegal(VT)) {
      if (VT.getSizeInBits() == 32 && Subtarget.hasSingleFloat()) {
        setOperationAction(ISD::FABS, VT, Legal);
        setOperationAction(ISD::FADD, VT, Legal);
        setOperationAction(ISD::FSUB, VT, Legal);
        setOperationAction(ISD::FMA, VT, Legal);
        setOperationAction(ISD::FMUL, VT, Legal);
        setOperationAction(ISD::FNEG, VT, Legal);
      } else {
        setOperationAction(ISD::FABS, VT, Expand);
        setOperationAction(ISD::FADD, VT, Expand);
        setOperationAction(ISD::FSUB, VT, Expand);
        setOperationAction(ISD::FMA, VT, Expand);
        setOperationAction(ISD::FMUL, VT, Expand);
        setOperationAction(ISD::FNEG, VT, Expand);
      }

      // TODO: once implemented in InstrInfo uncomment
      setOperationAction(ISD::FSQRT, VT, Expand);
      setOperationAction(ISD::FSIN, VT, Expand);
      setOperationAction(ISD::FCOS, VT, Expand);
      setOperationAction(ISD::FREM, VT, LibCall);
      setOperationAction(ISD::FDIV, VT, Expand);
      setOperationAction(ISD::FPOW, VT, Expand);
      setOperationAction(ISD::FSQRT, VT, Expand);
      setOperationAction(ISD::FCOPYSIGN, VT, Expand);
    }
  }

  // Handle floating-point types.
  if (Subtarget.hasSingleFloat()) {
    setOperationAction(ISD::BITCAST, MVT::i32, Legal);
    setOperationAction(ISD::BITCAST, MVT::f32, Legal);
    setOperationAction(ISD::UINT_TO_FP, MVT::i32, Legal);
    setOperationAction(ISD::SINT_TO_FP, MVT::i32, Legal);
    setOperationAction(ISD::FP_TO_UINT, MVT::i32, Legal);
    setOperationAction(ISD::FP_TO_SINT, MVT::i32, Legal);
  } else {
    setOperationAction(ISD::BITCAST, MVT::i32, Expand);
    setOperationAction(ISD::BITCAST, MVT::f32, Expand);
    setOperationAction(ISD::UINT_TO_FP, MVT::i32, Expand);
    setOperationAction(ISD::SINT_TO_FP, MVT::i32, Expand);
    setOperationAction(ISD::FP_TO_UINT, MVT::i32, Expand);
    setOperationAction(ISD::FP_TO_SINT, MVT::i32, Expand);
  }

  for (MVT VT : MVT::fp_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::f16, Expand);
  }

  setOperationAction(ISD::FP16_TO_FP, MVT::f64, Expand);
  setOperationAction(ISD::FP_TO_FP16, MVT::f64, Expand);
  setOperationAction(ISD::FP16_TO_FP, MVT::f32, Expand);
  setOperationAction(ISD::FP_TO_FP16, MVT::f32, Expand);

  // Floating-point truncation and stores need to be done separately.
  setTruncStoreAction(MVT::f64, MVT::f32, Expand);
  setTruncStoreAction(MVT::f64, MVT::f16, Expand);
  setTruncStoreAction(MVT::f32, MVT::f16, Expand);

  if (Subtarget.hasS32C1I()) {
    setMaxAtomicSizeInBitsSupported(32);
    setMinCmpXchgSizeInBits(32);
  } else if (Subtarget.hasForcedAtomics()) {
    setMaxAtomicSizeInBitsSupported(32);
  } else {
    setMaxAtomicSizeInBitsSupported(0);
  }

  // Compute derived properties from the register classes
  computeRegisterProperties(STI.getRegisterInfo());
}

Register GroqTargetLowering::getExceptionPointerRegister(
    ExceptionHandling EH, const Constant *PersonalityFn) const {
  return Groq::A2;
}

Register GroqTargetLowering::getExceptionSelectorRegister(
    ExceptionHandling EH, const Constant *PersonalityFn) const {
  return Groq::A3;
}

bool GroqTargetLowering::isOffsetFoldingLegal(
    const GlobalAddressSDNode *GA) const {
  // The Groq target isn't yet aware of offsets.
  return false;
}

bool GroqTargetLowering::isFPImmLegal(const APFloat &Imm, EVT VT,
                                        bool ForCodeSize) const {
  return false;
}

//===----------------------------------------------------------------------===//
// Inline asm support
//===----------------------------------------------------------------------===//
TargetLowering::ConstraintType
GroqTargetLowering::getConstraintType(StringRef Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
    case 'f':
      return C_RegisterClass;
    default:
      break;
    }
  }
  return TargetLowering::getConstraintType(Constraint);
}

TargetLowering::ConstraintWeight
GroqTargetLowering::getSingleConstraintMatchWeight(
    AsmOperandInfo &Info, const char *Constraint) const {
  ConstraintWeight Weight = CW_Invalid;
  Value *CallOperandVal = Info.CallOperandVal;
  // If we don't have a value, we can't do a match,
  // but allow it at the lowest weight.
  if (!CallOperandVal)
    return CW_Default;

  Type *Ty = CallOperandVal->getType();

  // Look at the constraint type.
  switch (*Constraint) {
  default:
    Weight = TargetLowering::getSingleConstraintMatchWeight(Info, Constraint);
    break;
  case 'r':
    if (Ty->isIntegerTy())
      Weight = CW_Register;
    break;
  case 'f':
    if (Ty->isFloatingPointTy())
      Weight = CW_Register;
    break;
  }
  return Weight;
}

std::pair<unsigned, const TargetRegisterClass *>
GroqTargetLowering::getRegForInlineAsmConstraint(
    const TargetRegisterInfo *TRI, StringRef Constraint, MVT VT) const {
  if (Constraint.size() == 1) {
    // GCC Constraint Letters
    switch (Constraint[0]) {
    default:
      break;
    case 'r': // General-purpose register
      return std::make_pair(0U, &Groq::ARRegClass);
    case 'f': // Floating-point register
      if (Subtarget.hasSingleFloat())
        return std::make_pair(0U, &Groq::FPRRegClass);
    }
  }
  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}

void GroqTargetLowering::LowerAsmOperandForConstraint(
    SDValue Op, StringRef Constraint, std::vector<SDValue> &Ops,
    SelectionDAG &DAG) const {
  SDLoc DL(Op);

  // Only support length 1 constraints for now.
  if (Constraint.size() > 1)
    return;

  TargetLowering::LowerAsmOperandForConstraint(Op, Constraint, Ops, DAG);
}

//===----------------------------------------------------------------------===//
// Calling conventions
//===----------------------------------------------------------------------===//

#define GET_CALLING_CONV_IMPL
#include "GroqGenCallingConv.inc"

static const MCPhysReg IntRegs[] = {Groq::A2, Groq::A3, Groq::A4,
                                    Groq::A5, Groq::A6, Groq::A7};

static bool CC_Groq_Custom(unsigned ValNo, MVT ValVT, MVT LocVT,
                             CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                             CCState &State) {
  if (ArgFlags.isByVal()) {
    Align ByValAlign = ArgFlags.getNonZeroByValAlign();
    unsigned ByValSize = ArgFlags.getByValSize();
    if (ByValSize < 4) {
      ByValSize = 4;
    }
    if (ByValAlign < Align(4)) {
      ByValAlign = Align(4);
    }
    unsigned Offset = State.AllocateStack(ByValSize, ByValAlign);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
    // Mark all unused registers as allocated to avoid misuse
    // of such registers.
    while (State.AllocateReg(IntRegs))
      ;
    return false;
  }

  // Promote i8 and i16
  if (LocVT == MVT::i8 || LocVT == MVT::i16) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  unsigned Register;

  Align OrigAlign = ArgFlags.getNonZeroOrigAlign();
  bool needs64BitAlign = (ValVT == MVT::i32 && OrigAlign == Align(8));
  bool needs128BitAlign = (ValVT == MVT::i32 && OrigAlign == Align(16));

  if (ValVT == MVT::i32) {
    Register = State.AllocateReg(IntRegs);
    // If this is the first part of an i64 arg,
    // the allocated register must be either A2, A4 or A6.
    if (needs64BitAlign && (Register == Groq::A3 || Register == Groq::A5 ||
                            Register == Groq::A7))
      Register = State.AllocateReg(IntRegs);
    // arguments with 16byte alignment must be passed in the first register or
    // passed via stack
    if (needs128BitAlign && (Register != Groq::A2))
      while ((Register = State.AllocateReg(IntRegs)))
        ;
    LocVT = MVT::i32;
  } else if (ValVT == MVT::f64) {
    // Allocate int register and shadow next int register.
    Register = State.AllocateReg(IntRegs);
    if (Register == Groq::A3 || Register == Groq::A5 ||
        Register == Groq::A7)
      Register = State.AllocateReg(IntRegs);
    State.AllocateReg(IntRegs);
    LocVT = MVT::i32;
  } else {
    report_fatal_error("Cannot handle this ValVT.");
  }

  if (!Register) {
    unsigned Offset = State.AllocateStack(ValVT.getStoreSize(), OrigAlign);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
  } else {
    State.addLoc(CCValAssign::getReg(ValNo, ValVT, Register, LocVT, LocInfo));
  }

  return false;
}

/// Return the register type for a given MVT
MVT GroqTargetLowering::getRegisterTypeForCallingConv(LLVMContext &Context,
                                                        CallingConv::ID CC,
                                                        EVT VT) const {
  if (VT.isFloatingPoint())
    return MVT::i32;

  return TargetLowering::getRegisterTypeForCallingConv(Context, CC, VT);
}

CCAssignFn *GroqTargetLowering::CCAssignFnForCall(CallingConv::ID CC,
                                                    bool IsVarArg) const {
  return CC_Groq_Custom;
}

SDValue GroqTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  GroqMachineFunctionInfo *GroqFI = MF.getInfo<GroqMachineFunctionInfo>();

  // Used with vargs to acumulate store chains.
  std::vector<SDValue> OutChains;

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CCAssignFnForCall(CallConv, IsVarArg));

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    // Arguments stored on registers
    if (VA.isRegLoc()) {
      EVT RegVT = VA.getLocVT();

      if (RegVT != MVT::i32)
        report_fatal_error("RegVT not supported by FormalArguments Lowering");

      // Transform the arguments stored on
      // physical registers into virtual ones
      Register Reg = 0;
      MCRegister FrameReg = Subtarget.getRegisterInfo()->getFrameRegister(MF);

      // Argument passed in FrameReg in Windowed ABI we save in A8 (in
      // emitPrologue), so load argument from A8
      if (Subtarget.isWindowedABI() && (VA.getLocReg() == FrameReg)) {
        Reg = MF.addLiveIn(Groq::A8, &Groq::ARRegClass);
        GroqFI->setSaveFrameRegister();
      } else {
        Reg = MF.addLiveIn(VA.getLocReg(), &Groq::ARRegClass);
      }

      SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegVT);

      // If this is an 8 or 16-bit value, it has been passed promoted
      // to 32 bits.  Insert an assert[sz]ext to capture this, then
      // truncate to the right size.
      if (VA.getLocInfo() != CCValAssign::Full) {
        unsigned Opcode = 0;
        if (VA.getLocInfo() == CCValAssign::SExt)
          Opcode = ISD::AssertSext;
        else if (VA.getLocInfo() == CCValAssign::ZExt)
          Opcode = ISD::AssertZext;
        if (Opcode)
          ArgValue = DAG.getNode(Opcode, DL, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));
        ArgValue = DAG.getNode((VA.getValVT() == MVT::f32) ? ISD::BITCAST
                                                           : ISD::TRUNCATE,
                               DL, VA.getValVT(), ArgValue);
      }

      InVals.push_back(ArgValue);

    } else {
      assert(VA.isMemLoc());

      EVT ValVT = VA.getValVT();

      // The stack pointer offset is relative to the caller stack frame.
      int FI = MFI.CreateFixedObject(ValVT.getStoreSize(), VA.getLocMemOffset(),
                                     true);

      if (Ins[VA.getValNo()].Flags.isByVal()) {
        // Assume that in this case load operation is created
        SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
        InVals.push_back(FIN);
      } else {
        // Create load nodes to retrieve arguments from the stack
        SDValue FIN =
            DAG.getFrameIndex(FI, getFrameIndexTy(DAG.getDataLayout()));
        InVals.push_back(DAG.getLoad(
            ValVT, DL, Chain, FIN,
            MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI)));
      }
    }
  }

  if (IsVarArg) {
    unsigned Idx = CCInfo.getFirstUnallocated(IntRegs);
    unsigned ArgRegsNum = std::size(IntRegs);
    const TargetRegisterClass *RC = &Groq::ARRegClass;
    MachineFrameInfo &MFI = MF.getFrameInfo();
    MachineRegisterInfo &RegInfo = MF.getRegInfo();
    unsigned RegSize = 4;
    MVT RegTy = MVT::i32;
    MVT FITy = getFrameIndexTy(DAG.getDataLayout());

    GroqFI->setVarArgsFirstGPR(Idx + 2); // 2 - number of a2 register

    GroqFI->setVarArgsOnStackFrameIndex(
        MFI.CreateFixedObject(4, CCInfo.getStackSize(), true));

    // Offset of the first variable argument from stack pointer, and size of
    // the vararg save area. For now, the varargs save area is either zero or
    // large enough to hold a0-a7.
    int VaArgOffset, VarArgsSaveSize;

    // If all registers are allocated, then all varargs must be passed on the
    // stack and we don't need to save any argregs.
    if (ArgRegsNum == Idx) {
      VaArgOffset = CCInfo.getStackSize();
      VarArgsSaveSize = 0;
    } else {
      VarArgsSaveSize = RegSize * (ArgRegsNum - Idx);
      VaArgOffset = -VarArgsSaveSize;

      // Record the frame index of the first variable argument
      // which is a value necessary to VASTART.
      int FI = MFI.CreateFixedObject(RegSize, VaArgOffset, true);
      GroqFI->setVarArgsInRegsFrameIndex(FI);

      // Copy the integer registers that may have been used for passing varargs
      // to the vararg save area.
      for (unsigned I = Idx; I < ArgRegsNum; ++I, VaArgOffset += RegSize) {
        const Register Reg = RegInfo.createVirtualRegister(RC);
        RegInfo.addLiveIn(IntRegs[I], Reg);

        SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegTy);
        FI = MFI.CreateFixedObject(RegSize, VaArgOffset, true);
        SDValue PtrOff = DAG.getFrameIndex(FI, FITy);
        SDValue Store = DAG.getStore(Chain, DL, ArgValue, PtrOff,
                                     MachinePointerInfo::getFixedStack(MF, FI));
        OutChains.push_back(Store);
      }
    }
  }

  // All stores are grouped in one node to allow the matching between
  // the size of Ins and InVals. This only happens when on varg functions
  if (!OutChains.empty()) {
    OutChains.push_back(Chain);
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, OutChains);
  }

  return Chain;
}

SDValue
GroqTargetLowering::LowerCall(CallLoweringInfo &CLI,
                                SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &IsTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;

  MachineFunction &MF = DAG.getMachineFunction();
  EVT PtrVT = getPointerTy(DAG.getDataLayout());
  const TargetFrameLowering *TFL = Subtarget.getFrameLowering();

  // TODO: Support tail call optimization.
  IsTailCall = false;

  // Analyze the operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());

  CCAssignFn *CC = CCAssignFnForCall(CallConv, IsVarArg);

  CCInfo.AnalyzeCallOperands(Outs, CC);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getStackSize();

  Align StackAlignment = TFL->getStackAlign();
  unsigned NextStackOffset = alignTo(NumBytes, StackAlignment);

  Chain = DAG.getCALLSEQ_START(Chain, NextStackOffset, 0, DL);

  // Copy argument values to their designated locations.
  std::deque<std::pair<unsigned, SDValue>> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;
  SDValue StackPtr;
  for (unsigned I = 0, E = ArgLocs.size(); I != E; ++I) {
    CCValAssign &VA = ArgLocs[I];
    SDValue ArgValue = OutVals[I];
    ISD::ArgFlagsTy Flags = Outs[I].Flags;

    if (VA.isRegLoc())
      // Queue up the argument copies and emit them at the end.
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), ArgValue));
    else if (Flags.isByVal()) {
      assert(VA.isMemLoc());
      assert(Flags.getByValSize() &&
             "ByVal args of size 0 should have been ignored by front-end.");
      assert(!IsTailCall &&
             "Do not tail-call optimize if there is a byval argument.");

      if (!StackPtr.getNode())
        StackPtr = DAG.getCopyFromReg(Chain, DL, Groq::SP, PtrVT);
      unsigned Offset = VA.getLocMemOffset();
      SDValue Address = DAG.getNode(ISD::ADD, DL, PtrVT, StackPtr,
                                    DAG.getIntPtrConstant(Offset, DL));
      SDValue SizeNode = DAG.getConstant(Flags.getByValSize(), DL, MVT::i32);
      Align Alignment = Flags.getNonZeroByValAlign();
      SDValue Memcpy = DAG.getMemcpy(
          Chain, DL, Address, ArgValue, SizeNode, Alignment, Alignment,
          /*isVolatile=*/false, /*AlwaysInline=*/false,
          /*CI=*/nullptr, std::nullopt, MachinePointerInfo(),
          MachinePointerInfo());
      MemOpChains.push_back(Memcpy);
    } else {
      assert(VA.isMemLoc() && "Argument not register or memory");

      // Work out the address of the stack slot.  Unpromoted ints and
      // floats are passed as right-justified 8-byte values.
      if (!StackPtr.getNode())
        StackPtr = DAG.getCopyFromReg(Chain, DL, Groq::SP, PtrVT);
      unsigned Offset = VA.getLocMemOffset();
      SDValue Address = DAG.getNode(ISD::ADD, DL, PtrVT, StackPtr,
                                    DAG.getIntPtrConstant(Offset, DL));

      // Emit the store.
      MemOpChains.push_back(
          DAG.getStore(Chain, DL, ArgValue, Address, MachinePointerInfo()));
    }
  }

  // Join the stores, which are independent of one another.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  // Build a sequence of copy-to-reg nodes, chained and glued together.
  SDValue Glue;
  for (unsigned I = 0, E = RegsToPass.size(); I != E; ++I) {
    unsigned Reg = RegsToPass[I].first;
    if (Subtarget.isWindowedABI())
      Reg = toCallerWindow(Reg);
    Chain = DAG.getCopyToReg(Chain, DL, Reg, RegsToPass[I].second, Glue);
    Glue = Chain.getValue(1);
  }
  std::string name;
  unsigned char TF = 0;

  // Accept direct calls by converting symbolic call addresses to the
  // associated Target* opcodes.
  if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    name = E->getSymbol();
    TF = E->getTargetFlags();
    if (isPositionIndependent()) {
      report_fatal_error("PIC relocations is not supported");
    } else
      Callee = DAG.getTargetExternalSymbol(E->getSymbol(), PtrVT, TF);
  } else if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    const GlobalValue *GV = G->getGlobal();
    name = GV->getName().str();
  }

  if ((!name.empty()) && isLongCall(name.c_str())) {
    // Create a constant pool entry for the callee address
    GroqCP::GroqCPModifier Modifier = GroqCP::no_modifier;
    GroqMachineFunctionInfo *GroqFI =
        MF.getInfo<GroqMachineFunctionInfo>();
    unsigned LabelId = GroqFI->createCPLabelId();

    GroqConstantPoolValue *CPV = GroqConstantPoolSymbol::Create(
        *DAG.getContext(), name.c_str(), LabelId, false, Modifier);

    // Get the address of the callee into a register
    SDValue CPAddr = DAG.getTargetConstantPool(CPV, PtrVT, Align(4), 0, TF);
    SDValue CPWrap = getAddrPCRel(CPAddr, DAG);
    Callee = DAG.getLoad(
        PtrVT, DL, DAG.getEntryNode(), CPWrap,
        MachinePointerInfo::getConstantPool(DAG.getMachineFunction()));
  }

  // The first call operand is the chain and the second is the target address.
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add a register mask operand representing the call-preserved registers.
  const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(MF, CallConv);
  assert(Mask && "Missing call preserved mask for calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned I = 0, E = RegsToPass.size(); I != E; ++I) {
    unsigned Reg = RegsToPass[I].first;
    if (Subtarget.isWindowedABI())
      Reg = toCallerWindow(Reg);
    Ops.push_back(DAG.getRegister(Reg, RegsToPass[I].second.getValueType()));
  }

  // Glue the call to the argument copies, if any.
  if (Glue.getNode())
    Ops.push_back(Glue);

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(Subtarget.isWindowedABI() ? GroqISD::CALLW8
                                                : GroqISD::CALL,
                      DL, NodeTys, Ops);
  Glue = Chain.getValue(1);

  // Mark the end of the call, which is glued to the call itself.
  Chain = DAG.getCALLSEQ_END(Chain, DAG.getConstant(NumBytes, DL, PtrVT, true),
                             DAG.getConstant(0, DL, PtrVT, true), Glue, DL);
  Glue = Chain.getValue(1);

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RetLocs;
  CCState RetCCInfo(CallConv, IsVarArg, MF, RetLocs, *DAG.getContext());
  RetCCInfo.AnalyzeCallResult(Ins, Subtarget.isWindowedABI() ? RetCCW8_Groq
                                                             : RetCC_Groq);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned I = 0, E = RetLocs.size(); I != E; ++I) {
    CCValAssign &VA = RetLocs[I];

    // Copy the value out, gluing the copy to the end of the call sequence.
    unsigned Reg = VA.getLocReg();
    SDValue RetValue = DAG.getCopyFromReg(Chain, DL, Reg, VA.getLocVT(), Glue);
    Chain = RetValue.getValue(1);
    Glue = RetValue.getValue(2);

    InVals.push_back(RetValue);
  }
  return Chain;
}

bool GroqTargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context,
    const Type *RetTy) const {
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, Context);
  return CCInfo.CheckReturn(Outs, RetCC_Groq);
}

SDValue
GroqTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                  bool IsVarArg,
                                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                                  const SmallVectorImpl<SDValue> &OutVals,
                                  const SDLoc &DL, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();

  // Assign locations to each returned value.
  SmallVector<CCValAssign, 16> RetLocs;
  CCState RetCCInfo(CallConv, IsVarArg, MF, RetLocs, *DAG.getContext());
  RetCCInfo.AnalyzeReturn(Outs, RetCC_Groq);

  SDValue Glue;
  // Quick exit for void returns
  if (RetLocs.empty())
    return DAG.getNode(Subtarget.isWindowedABI() ? GroqISD::RETW
                                                 : GroqISD::RET,
                       DL, MVT::Other, Chain);

  // Copy the result values into the output registers.
  SmallVector<SDValue, 4> RetOps;
  RetOps.push_back(Chain);
  for (unsigned I = 0, E = RetLocs.size(); I != E; ++I) {
    CCValAssign &VA = RetLocs[I];
    SDValue RetValue = OutVals[I];

    // Make the return register live on exit.
    assert(VA.isRegLoc() && "Can only return in registers!");

    // Chain and glue the copies together.
    unsigned Register = VA.getLocReg();
    Chain = DAG.getCopyToReg(Chain, DL, Register, RetValue, Glue);
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(Register, VA.getLocVT()));
  }

  // Update chain and glue.
  RetOps[0] = Chain;
  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(Subtarget.isWindowedABI() ? GroqISD::RETW
                                               : GroqISD::RET,
                     DL, MVT::Other, RetOps);
}

static unsigned getBranchOpcode(ISD::CondCode Cond) {
  switch (Cond) {
  case ISD::SETEQ:
    return Groq::BEQ;
  case ISD::SETNE:
    return Groq::BNE;
  case ISD::SETLT:
    return Groq::BLT;
  case ISD::SETLE:
    return Groq::BGE;
  case ISD::SETGT:
    return Groq::BLT;
  case ISD::SETGE:
    return Groq::BGE;
  case ISD::SETULT:
    return Groq::BLTU;
  case ISD::SETULE:
    return Groq::BGEU;
  case ISD::SETUGT:
    return Groq::BLTU;
  case ISD::SETUGE:
    return Groq::BGEU;
  default:
    llvm_unreachable("Unknown branch kind");
  }
}

static std::pair<unsigned, unsigned> getFPBranchKind(ISD::CondCode Cond) {
  switch (Cond) {
  case ISD::SETUNE:
    return std::make_pair(Groq::BF, Groq::OEQ_S);
  case ISD::SETUO:
    return std::make_pair(Groq::BT, Groq::UN_S);
  case ISD::SETO:
    return std::make_pair(Groq::BF, Groq::UN_S);
  case ISD::SETUEQ:
    return std::make_pair(Groq::BT, Groq::UEQ_S);
  case ISD::SETULE:
    return std::make_pair(Groq::BT, Groq::ULE_S);
  case ISD::SETULT:
    return std::make_pair(Groq::BT, Groq::ULT_S);
  case ISD::SETEQ:
  case ISD::SETOEQ:
    return std::make_pair(Groq::BT, Groq::OEQ_S);
  case ISD::SETNE:
    return std::make_pair(Groq::BF, Groq::OEQ_S);
  case ISD::SETLE:
  case ISD::SETOLE:
    return std::make_pair(Groq::BT, Groq::OLE_S);
  case ISD::SETLT:
  case ISD::SETOLT:
    return std::make_pair(Groq::BT, Groq::OLT_S);
  case ISD::SETGE:
    return std::make_pair(Groq::BF, Groq::OLT_S);
  case ISD::SETGT:
    return std::make_pair(Groq::BF, Groq::OLE_S);
  case ISD::SETOGT:
    return std::make_pair(Groq::BF, Groq::ULE_S);
  case ISD::SETOGE:
    return std::make_pair(Groq::BF, Groq::ULT_S);
  case ISD::SETONE:
    return std::make_pair(Groq::BF, Groq::UEQ_S);
  case ISD::SETUGT:
    return std::make_pair(Groq::BF, Groq::OLE_S);
  case ISD::SETUGE:
    return std::make_pair(Groq::BF, Groq::OLT_S);
  default:
    llvm_unreachable("Invalid condition!");
  }
}

SDValue GroqTargetLowering::LowerSELECT_CC(SDValue Op,
                                             SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT Ty = Op.getValueType();
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueValue = Op.getOperand(2);
  SDValue FalseValue = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op->getOperand(4))->get();

  if (LHS.getValueType() == MVT::i32) {
    unsigned BrOpcode = getBranchOpcode(CC);
    SDValue TargetCC = DAG.getConstant(BrOpcode, DL, MVT::i32);

    SDValue Res = DAG.getNode(GroqISD::SELECT_CC, DL, Ty, LHS, RHS, TrueValue,
                              FalseValue, TargetCC, Op->getFlags());
    return Res;
  }
  assert(LHS.getValueType() == MVT::f32 &&
         "We expect MVT::f32 type of the LHS Operand in SELECT_CC");
  unsigned BrOpcode;
  unsigned CmpOpCode;
  std::tie(BrOpcode, CmpOpCode) = getFPBranchKind(CC);
  SDValue TargetCC = DAG.getConstant(CmpOpCode, DL, MVT::i32);
  SDValue TargetBC = DAG.getConstant(BrOpcode, DL, MVT::i32);
  return DAG.getNode(GroqISD::SELECT_CC_FP, DL, Ty,
                     {LHS, RHS, TrueValue, FalseValue, TargetCC, TargetBC},
                     Op->getFlags());
}

SDValue GroqTargetLowering::LowerRETURNADDR(SDValue Op,
                                              SelectionDAG &DAG) const {
  // This nodes represent llvm.returnaddress on the DAG.
  // It takes one operand, the index of the return address to return.
  // An index of zero corresponds to the current function's return address.
  // An index of one to the parent's return address, and so on.
  // Depths > 0 not supported yet!
  if (Op.getConstantOperandVal(0) != 0)
    return SDValue();

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  EVT VT = Op.getValueType();
  MFI.setReturnAddressIsTaken(true);

  // Return RA, which contains the return address. Mark it an implicit
  // live-in.
  Register RA = MF.addLiveIn(Groq::A0, getRegClassFor(MVT::i32));
  return DAG.getCopyFromReg(DAG.getEntryNode(), SDLoc(Op), RA, VT);
}

SDValue GroqTargetLowering::LowerImmediate(SDValue Op,
                                             SelectionDAG &DAG) const {
  const ConstantSDNode *CN = cast<ConstantSDNode>(Op);
  SDLoc DL(CN);
  APInt APVal = CN->getAPIntValue();
  int64_t Value = APVal.getSExtValue();
  if (Op.getValueType() == MVT::i32) {
    // Check if use node maybe lowered to the MOVI instruction
    if (Value > -2048 && Value <= 2047)
      return Op;
    // Check if use node maybe lowered to the ADDMI instruction
    SDNode &OpNode = *Op.getNode();
    if ((OpNode.hasOneUse() && OpNode.user_begin()->getOpcode() == ISD::ADD) &&
        isShiftedInt<8, 8>(Value))
      return Op;
    Type *Ty = Type::getInt32Ty(*DAG.getContext());
    Constant *CV = ConstantInt::getSigned(Ty, Value);
    SDValue CP = DAG.getConstantPool(CV, MVT::i32);
    SDValue Res =
        DAG.getLoad(MVT::i32, DL, DAG.getEntryNode(), CP, MachinePointerInfo());
    return Res;
  }
  return Op;
}

SDValue GroqTargetLowering::LowerGlobalAddress(SDValue Op,
                                                 SelectionDAG &DAG) const {
  const GlobalAddressSDNode *G = cast<GlobalAddressSDNode>(Op);
  SDLoc DL(Op);
  auto PtrVT = Op.getValueType();
  const GlobalValue *GV = G->getGlobal();

  SDValue CPAddr = DAG.getTargetConstantPool(GV, PtrVT, Align(4));
  SDValue CPWrap = getAddrPCRel(CPAddr, DAG);
  SDValue Res = DAG.getLoad(
      PtrVT, DL, DAG.getEntryNode(), CPWrap,
      MachinePointerInfo::getConstantPool(DAG.getMachineFunction()));
  return Res;
}

SDValue GroqTargetLowering::LowerGlobalTLSAddress(SDValue Op,
                                                    SelectionDAG &DAG) const {
  const GlobalAddressSDNode *G = cast<GlobalAddressSDNode>(Op);
  SDLoc DL(Op);
  EVT PtrVT = Op.getValueType();
  const GlobalValue *GV = G->getGlobal();

  if (DAG.getTarget().useEmulatedTLS())
    return LowerToTLSEmulatedModel(G, DAG);

  TLSModel::Model model = getTargetMachine().getTLSModel(GV);

  if (!Subtarget.hasTHREADPTR()) {
    DAG.getContext()->diagnose(DiagnosticInfoUnsupported(
        DAG.getMachineFunction().getFunction(), "only emulated TLS supported",
        DL.getDebugLoc()));
    return DAG.getPOISON(Op->getValueType(0));
  }

  if (model == TLSModel::LocalExec || model == TLSModel::InitialExec) {
    bool Priv = GV->isPrivateLinkage(GV->getLinkage());
    MachineFunction &MF = DAG.getMachineFunction();
    GroqMachineFunctionInfo *GroqFI =
        MF.getInfo<GroqMachineFunctionInfo>();
    unsigned LabelId = GroqFI->createCPLabelId();

    // Create a constant pool entry for the callee address
    GroqConstantPoolValue *CPV = GroqConstantPoolSymbol::Create(
        *DAG.getContext(), GV->getName().str().c_str(), LabelId, Priv,
        GroqCP::TPOFF);

    // Get the address of the callee into a register
    SDValue CPAddr = DAG.getTargetConstantPool(CPV, PtrVT, Align(4));
    SDValue CPWrap = getAddrPCRel(CPAddr, DAG);
    SDValue Addr = DAG.getLoad(
        PtrVT, DL, DAG.getEntryNode(), CPWrap,
        MachinePointerInfo::getConstantPool(DAG.getMachineFunction()));

    SDValue TPRegister = DAG.getRegister(Groq::THREADPTR, MVT::i32);
    SDValue ThreadPointer =
        DAG.getNode(GroqISD::RUR, DL, MVT::i32, TPRegister);

    return DAG.getNode(ISD::ADD, DL, PtrVT, ThreadPointer, Addr);
  }

  DAG.getContext()->diagnose(DiagnosticInfoUnsupported(
      DAG.getMachineFunction().getFunction(),
      "only local-exec and initial-exec TLS mode supported", DL.getDebugLoc()));

  return DAG.getPOISON(Op->getValueType(0));
}

SDValue GroqTargetLowering::LowerBlockAddress(SDValue Op,
                                                SelectionDAG &DAG) const {
  BlockAddressSDNode *Node = cast<BlockAddressSDNode>(Op);
  SDLoc DL(Op);
  const BlockAddress *BA = Node->getBlockAddress();
  EVT PtrVT = Op.getValueType();
  MachineFunction &MF = DAG.getMachineFunction();
  GroqMachineFunctionInfo *GroqFI = MF.getInfo<GroqMachineFunctionInfo>();
  unsigned LabelId = GroqFI->createCPLabelId();

  GroqConstantPoolValue *CPV =
      GroqConstantPoolConstant::Create(BA, LabelId, GroqCP::CPBlockAddress);
  SDValue CPAddr = DAG.getTargetConstantPool(CPV, PtrVT, Align(4));
  SDValue CPWrap = getAddrPCRel(CPAddr, DAG);
  SDValue Res = DAG.getLoad(
      PtrVT, DL, DAG.getEntryNode(), CPWrap,
      MachinePointerInfo::getConstantPool(DAG.getMachineFunction()));
  return Res;
}

SDValue GroqTargetLowering::LowerBR_JT(SDValue Op, SelectionDAG &DAG) const {
  SDValue Chain = Op.getOperand(0);
  SDValue Table = Op.getOperand(1);
  SDValue Index = Op.getOperand(2);
  SDLoc DL(Op);
  JumpTableSDNode *JT = cast<JumpTableSDNode>(Table);
  MachineFunction &MF = DAG.getMachineFunction();
  const MachineJumpTableInfo *MJTI = MF.getJumpTableInfo();
  SDValue TargetJT = DAG.getTargetJumpTable(JT->getIndex(), MVT::i32);
  const DataLayout &TD = DAG.getDataLayout();
  EVT PtrVT = Table.getValueType();
  unsigned EntrySize = MJTI->getEntrySize(TD);

  assert((MJTI->getEntrySize(TD) == 4) && "Unsupported jump-table entry size");

  Index = DAG.getNode(
      ISD::SHL, DL, Index.getValueType(), Index,
      DAG.getConstant(Log2_32(EntrySize), DL, Index.getValueType()));

  SDValue Addr = DAG.getNode(ISD::ADD, DL, Index.getValueType(), Index, Table);
  SDValue LD =
      DAG.getLoad(PtrVT, DL, Chain, Addr,
                  MachinePointerInfo::getJumpTable(DAG.getMachineFunction()));

  return DAG.getNode(GroqISD::BR_JT, DL, MVT::Other, LD.getValue(1), LD,
                     TargetJT);
}

SDValue GroqTargetLowering::LowerJumpTable(SDValue Op,
                                             SelectionDAG &DAG) const {
  JumpTableSDNode *JT = cast<JumpTableSDNode>(Op);
  EVT PtrVT = Op.getValueType();
  SDLoc DL(Op);

  // Create a constant pool entry for the jumptable address
  GroqConstantPoolValue *CPV =
      GroqConstantPoolJumpTable::Create(*DAG.getContext(), JT->getIndex());

  // Get the address of the jumptable into a register
  SDValue CPAddr = DAG.getTargetConstantPool(CPV, PtrVT, Align(4));

  SDValue Res = DAG.getLoad(
      PtrVT, DL, DAG.getEntryNode(), getAddrPCRel(CPAddr, DAG),
      MachinePointerInfo::getConstantPool(DAG.getMachineFunction()));
  return Res;
}

SDValue GroqTargetLowering::getAddrPCRel(SDValue Op,
                                           SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT Ty = Op.getValueType();
  return DAG.getNode(GroqISD::PCREL_WRAPPER, DL, Ty, Op);
}

SDValue GroqTargetLowering::LowerConstantPool(SDValue Op,
                                                SelectionDAG &DAG) const {
  EVT PtrVT = Op.getValueType();
  ConstantPoolSDNode *CP = cast<ConstantPoolSDNode>(Op);
  SDValue Result;

  if (!CP->isMachineConstantPoolEntry()) {
    Result = DAG.getTargetConstantPool(CP->getConstVal(), PtrVT, CP->getAlign(),
                                       CP->getOffset());
  } else {
    report_fatal_error("This constantpool type is not supported yet");
  }

  return getAddrPCRel(Result, DAG);
}

SDValue GroqTargetLowering::LowerSTACKSAVE(SDValue Op,
                                             SelectionDAG &DAG) const {
  return DAG.getCopyFromReg(Op.getOperand(0), SDLoc(Op), Groq::SP,
                            Op.getValueType());
}

SDValue GroqTargetLowering::LowerSTACKRESTORE(SDValue Op,
                                                SelectionDAG &DAG) const {
  SDValue Chain = Op.getOperand(0);
  SDValue NewSP = Op.getOperand(1);

  if (Subtarget.isWindowedABI()) {
    return DAG.getNode(GroqISD::MOVSP, SDLoc(Op), MVT::Other, Chain, NewSP);
  }

  return DAG.getCopyToReg(Chain, SDLoc(Op), Groq::SP, NewSP);
}

SDValue GroqTargetLowering::LowerFRAMEADDR(SDValue Op,
                                             SelectionDAG &DAG) const {
  // This nodes represent llvm.frameaddress on the DAG.
  // It takes one operand, the index of the frame address to return.
  // An index of zero corresponds to the current function's frame address.
  // An index of one to the parent's frame address, and so on.
  // Depths > 0 not supported yet!
  if (Op.getConstantOperandVal(0) != 0)
    return SDValue();

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MFI.setFrameAddressIsTaken(true);
  EVT VT = Op.getValueType();
  SDLoc DL(Op);

  MCRegister FrameRegister = Subtarget.getRegisterInfo()->getFrameRegister(MF);
  SDValue FrameAddr =
      DAG.getCopyFromReg(DAG.getEntryNode(), DL, FrameRegister, VT);
  return FrameAddr;
}

SDValue GroqTargetLowering::LowerDYNAMIC_STACKALLOC(SDValue Op,
                                                      SelectionDAG &DAG) const {
  SDValue Chain = Op.getOperand(0); // Legalize the chain.
  SDValue Size = Op.getOperand(1);  // Legalize the size.
  EVT VT = Size->getValueType(0);
  SDLoc DL(Op);

  // Round up Size to 32
  SDValue SizeTmp =
      DAG.getNode(ISD::ADD, DL, VT, Size, DAG.getConstant(31, DL, MVT::i32));
  SDValue SizeRoundUp = DAG.getNode(ISD::AND, DL, VT, SizeTmp,
                                    DAG.getSignedConstant(~31, DL, MVT::i32));

  MCRegister SPReg = Groq::SP;
  SDValue SP = DAG.getCopyFromReg(Chain, DL, SPReg, VT);
  SDValue NewSP = DAG.getNode(ISD::SUB, DL, VT, SP, SizeRoundUp); // Value
  if (Subtarget.isWindowedABI()) {
    Chain = DAG.getNode(GroqISD::MOVSP, SDLoc(Op), MVT::Other, SP.getValue(1),
                        NewSP);
  } else {
    Chain = DAG.getCopyToReg(SP.getValue(1), DL, SPReg, NewSP); // Output chain
  }

  SDValue NewVal = DAG.getCopyFromReg(Chain, DL, SPReg, MVT::i32);
  Chain = NewVal.getValue(1);

  SDValue Ops[2] = {NewVal, Chain};
  return DAG.getMergeValues(Ops, DL);
}

SDValue GroqTargetLowering::LowerVASTART(SDValue Op,
                                           SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  GroqMachineFunctionInfo *GroqFI = MF.getInfo<GroqMachineFunctionInfo>();
  SDValue Chain = Op.getOperand(0);
  SDValue Addr = Op.getOperand(1);
  EVT PtrVT = Addr.getValueType();
  SDLoc DL(Op);

  // Struct va_list_tag
  // int32 *va_stk - points to the arguments passed in memory
  // int32 *va_reg - points to the registers with arguments saved in memory
  // int32 va_ndx  - offset from va_stk or va_reg pointers which points to  the
  // next variable argument

  SDValue VAIndex;
  SDValue StackOffsetFI =
      DAG.getFrameIndex(GroqFI->getVarArgsOnStackFrameIndex(), PtrVT);
  unsigned ArgWords = GroqFI->getVarArgsFirstGPR() - 2;

  // If first variable argument passed in registers (maximum words in registers
  // is 6) then set va_ndx to the position of this argument in registers area
  // stored in memory (va_reg pointer). Otherwise va_ndx should point to the
  // position of the first variable argument on stack (va_stk pointer).
  if (ArgWords < 6) {
    VAIndex = DAG.getConstant(ArgWords * 4, DL, MVT::i32);
  } else {
    VAIndex = DAG.getConstant(32, DL, MVT::i32);
  }

  SDValue FrameIndex =
      DAG.getFrameIndex(GroqFI->getVarArgsInRegsFrameIndex(), PtrVT);
  uint64_t FrameOffset = PtrVT.getStoreSize();
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();

  // Store pointer to arguments given on stack (va_stk)
  SDValue StackPtr = DAG.getNode(ISD::SUB, DL, PtrVT, StackOffsetFI,
                                 DAG.getConstant(32, DL, PtrVT));

  SDValue StoreStackPtr =
      DAG.getStore(Chain, DL, StackPtr, Addr, MachinePointerInfo(SV));

  uint64_t NextOffset = FrameOffset;
  SDValue NextPtr =
      DAG.getObjectPtrOffset(DL, Addr, TypeSize::getFixed(NextOffset));

  // Store pointer to arguments given on registers (va_reg)
  SDValue StoreRegPtr = DAG.getStore(StoreStackPtr, DL, FrameIndex, NextPtr,
                                     MachinePointerInfo(SV, NextOffset));
  NextOffset += FrameOffset;
  NextPtr = DAG.getObjectPtrOffset(DL, Addr, TypeSize::getFixed(NextOffset));

  // Store third word : position in bytes of the first VA argument (va_ndx)
  return DAG.getStore(StoreRegPtr, DL, VAIndex, NextPtr,
                      MachinePointerInfo(SV, NextOffset));
}

SDValue GroqTargetLowering::LowerVACOPY(SDValue Op, SelectionDAG &DAG) const {
  // Size of the va_list_tag structure
  constexpr unsigned VAListSize = 3 * 4;
  SDValue Chain = Op.getOperand(0);
  SDValue DstPtr = Op.getOperand(1);
  SDValue SrcPtr = Op.getOperand(2);
  const Value *DstSV = cast<SrcValueSDNode>(Op.getOperand(3))->getValue();
  const Value *SrcSV = cast<SrcValueSDNode>(Op.getOperand(4))->getValue();
  SDLoc DL(Op);

  return DAG.getMemcpy(Chain, DL, DstPtr, SrcPtr,
                       DAG.getConstant(VAListSize, SDLoc(Op), MVT::i32),
                       Align(4), Align(4), /*isVolatile*/ false,
                       /*AlwaysInline*/ true,
                       /*CI=*/nullptr, std::nullopt, MachinePointerInfo(DstSV),
                       MachinePointerInfo(SrcSV));
}

SDValue GroqTargetLowering::LowerVAARG(SDValue Op, SelectionDAG &DAG) const {
  SDNode *Node = Op.getNode();
  EVT VT = Node->getValueType(0);
  Type *Ty = VT.getTypeForEVT(*DAG.getContext());
  EVT PtrVT = Op.getValueType();
  SDValue InChain = Node->getOperand(0);
  SDValue VAListPtr = Node->getOperand(1);
  const Value *SV = cast<SrcValueSDNode>(Node->getOperand(2))->getValue();
  SDLoc DL(Node);
  auto &TD = DAG.getDataLayout();
  Align ArgAlignment = TD.getABITypeAlign(Ty);
  unsigned ArgAlignInBytes = ArgAlignment.value();
  unsigned ArgSizeInBytes = TD.getTypeAllocSize(Ty);
  unsigned VASizeInBytes = llvm::alignTo(ArgSizeInBytes, 4);

  // va_stk
  SDValue VAStack =
      DAG.getLoad(MVT::i32, DL, InChain, VAListPtr, MachinePointerInfo());
  InChain = VAStack.getValue(1);

  // va_reg
  SDValue VARegPtr =
      DAG.getObjectPtrOffset(DL, VAListPtr, TypeSize::getFixed(4));
  SDValue VAReg =
      DAG.getLoad(MVT::i32, DL, InChain, VARegPtr, MachinePointerInfo());
  InChain = VAReg.getValue(1);

  // va_ndx
  SDValue VarArgIndexPtr =
      DAG.getObjectPtrOffset(DL, VARegPtr, TypeSize::getFixed(4));
  SDValue VAIndex =
      DAG.getLoad(MVT::i32, DL, InChain, VarArgIndexPtr, MachinePointerInfo());
  InChain = VAIndex.getValue(1);

  SDValue OrigIndex = VAIndex;

  if (ArgAlignInBytes > 4) {
    OrigIndex = DAG.getNode(ISD::ADD, DL, PtrVT, OrigIndex,
                            DAG.getConstant(ArgAlignInBytes - 1, DL, MVT::i32));
    OrigIndex =
        DAG.getNode(ISD::AND, DL, PtrVT, OrigIndex,
                    DAG.getSignedConstant(-ArgAlignInBytes, DL, MVT::i32));
  }

  VAIndex = DAG.getNode(ISD::ADD, DL, PtrVT, OrigIndex,
                        DAG.getConstant(VASizeInBytes, DL, MVT::i32));

  SDValue CC = DAG.getSetCC(DL, MVT::i32, OrigIndex,
                            DAG.getConstant(6 * 4, DL, MVT::i32), ISD::SETLE);

  SDValue StkIndex =
      DAG.getNode(ISD::ADD, DL, PtrVT, VAIndex,
                  DAG.getConstant(32 + VASizeInBytes, DL, MVT::i32));

  CC = DAG.getSetCC(DL, MVT::i32, VAIndex, DAG.getConstant(6 * 4, DL, MVT::i32),
                    ISD::SETLE);

  SDValue Array = DAG.getNode(ISD::SELECT, DL, MVT::i32, CC, VAReg, VAStack);

  VAIndex = DAG.getNode(ISD::SELECT, DL, MVT::i32, CC, VAIndex, StkIndex);

  CC = DAG.getSetCC(DL, MVT::i32, VAIndex, DAG.getConstant(6 * 4, DL, MVT::i32),
                    ISD::SETLE);

  SDValue VAIndexStore = DAG.getStore(InChain, DL, VAIndex, VarArgIndexPtr,
                                      MachinePointerInfo(SV));
  InChain = VAIndexStore;

  SDValue Addr = DAG.getNode(ISD::SUB, DL, PtrVT, VAIndex,
                             DAG.getConstant(VASizeInBytes, DL, MVT::i32));

  Addr = DAG.getNode(ISD::ADD, DL, PtrVT, Array, Addr);

  return DAG.getLoad(VT, DL, InChain, Addr, MachinePointerInfo());
}

SDValue GroqTargetLowering::LowerShiftLeftParts(SDValue Op,
                                                  SelectionDAG &DAG) const {
  SDLoc DL(Op);
  MVT VT = MVT::i32;
  SDValue Lo = Op.getOperand(0), Hi = Op.getOperand(1);
  SDValue Shamt = Op.getOperand(2);

  // if Shamt - register size < 0: // Shamt < register size
  //   Lo = Lo << Shamt
  //   Hi = (Hi << Shamt) | (Lo >>u (register size - Shamt))
  // else:
  //   Lo = 0
  //   Hi = Lo << (Shamt - register size)

  SDValue MinusRegisterSize = DAG.getSignedConstant(-32, DL, VT);
  SDValue ShamtMinusRegisterSize =
      DAG.getNode(ISD::ADD, DL, VT, Shamt, MinusRegisterSize);

  SDValue LoTrue = DAG.getNode(ISD::SHL, DL, VT, Lo, Shamt);
  SDValue HiTrue = DAG.getNode(GroqISD::SRCL, DL, VT, Hi, Lo, Shamt);
  SDValue Zero = DAG.getConstant(0, DL, VT);
  SDValue HiFalse = DAG.getNode(ISD::SHL, DL, VT, Lo, ShamtMinusRegisterSize);

  SDValue Cond = DAG.getSetCC(DL, VT, ShamtMinusRegisterSize, Zero, ISD::SETLT);
  Lo = DAG.getNode(ISD::SELECT, DL, VT, Cond, LoTrue, Zero);
  Hi = DAG.getNode(ISD::SELECT, DL, VT, Cond, HiTrue, HiFalse);

  return DAG.getMergeValues({Lo, Hi}, DL);
}

SDValue GroqTargetLowering::LowerShiftRightParts(SDValue Op,
                                                   SelectionDAG &DAG,
                                                   bool IsSRA) const {
  SDLoc DL(Op);
  SDValue Lo = Op.getOperand(0), Hi = Op.getOperand(1);
  SDValue Shamt = Op.getOperand(2);
  MVT VT = MVT::i32;

  // SRA expansion:
  //   if Shamt - register size < 0: // Shamt < register size
  //     Lo = (Lo >>u Shamt) | (Hi << u (register size - Shamt))
  //     Hi = Hi >>s Shamt
  //   else:
  //     Lo = Hi >>s (Shamt - register size);
  //     Hi = Hi >>s (register size - 1)
  //
  // SRL expansion:
  //   if Shamt - register size < 0: // Shamt < register size
  //     Lo = (Lo >>u Shamt) | (Hi << u (register size - Shamt))
  //     Hi = Hi >>u Shamt
  //   else:
  //     Lo = Hi >>u (Shamt - register size);
  //     Hi = 0;

  unsigned ShiftRightOp = IsSRA ? ISD::SRA : ISD::SRL;
  SDValue MinusRegisterSize = DAG.getSignedConstant(-32, DL, VT);
  SDValue RegisterSizeMinus1 = DAG.getConstant(32 - 1, DL, VT);
  SDValue ShamtMinusRegisterSize =
      DAG.getNode(ISD::ADD, DL, VT, Shamt, MinusRegisterSize);

  SDValue LoTrue = DAG.getNode(GroqISD::SRCR, DL, VT, Hi, Lo, Shamt);
  SDValue HiTrue = DAG.getNode(ShiftRightOp, DL, VT, Hi, Shamt);
  SDValue Zero = DAG.getConstant(0, DL, VT);
  SDValue LoFalse =
      DAG.getNode(ShiftRightOp, DL, VT, Hi, ShamtMinusRegisterSize);
  SDValue HiFalse;

  if (IsSRA) {
    HiFalse = DAG.getNode(ShiftRightOp, DL, VT, Hi, RegisterSizeMinus1);
  } else {
    HiFalse = Zero;
  }

  SDValue Cond = DAG.getSetCC(DL, VT, ShamtMinusRegisterSize, Zero, ISD::SETLT);
  Lo = DAG.getNode(ISD::SELECT, DL, VT, Cond, LoTrue, LoFalse);
  Hi = DAG.getNode(ISD::SELECT, DL, VT, Cond, HiTrue, HiFalse);

  return DAG.getMergeValues({Lo, Hi}, DL);
}

SDValue GroqTargetLowering::LowerCTPOP(SDValue Op, SelectionDAG &DAG) const {
  auto &TLI = DAG.getTargetLoweringInfo();
  return TLI.expandCTPOP(Op.getNode(), DAG);
}

bool GroqTargetLowering::decomposeMulByConstant(LLVMContext &Context, EVT VT,
                                                  SDValue C) const {
  APInt Imm;
  unsigned EltSizeInBits;

  if (ISD::isConstantSplatVector(C.getNode(), Imm)) {
    EltSizeInBits = VT.getScalarSizeInBits();
  } else if (VT.isScalarInteger()) {
    EltSizeInBits = VT.getSizeInBits();
    if (auto *ConstNode = dyn_cast<ConstantSDNode>(C.getNode()))
      Imm = ConstNode->getAPIntValue();
    else
      return false;
  } else {
    return false;
  }

  // Omit if data size exceeds.
  if (EltSizeInBits > 32)
    return false;

  // Convert MULT to LSL.
  if (Imm.isPowerOf2() && Imm.isIntN(5))
    return true;

  return false;
}

SDValue GroqTargetLowering::LowerOperation(SDValue Op,
                                             SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::BR_JT:
    return LowerBR_JT(Op, DAG);
  case ISD::Constant:
    return LowerImmediate(Op, DAG);
  case ISD::RETURNADDR:
    return LowerRETURNADDR(Op, DAG);
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::GlobalTLSAddress:
    return LowerGlobalTLSAddress(Op, DAG);
  case ISD::BlockAddress:
    return LowerBlockAddress(Op, DAG);
  case ISD::JumpTable:
    return LowerJumpTable(Op, DAG);
  case ISD::CTPOP:
    return LowerCTPOP(Op, DAG);
  case ISD::ConstantPool:
    return LowerConstantPool(Op, DAG);
  case ISD::SELECT_CC:
    return LowerSELECT_CC(Op, DAG);
  case ISD::STACKSAVE:
    return LowerSTACKSAVE(Op, DAG);
  case ISD::STACKRESTORE:
    return LowerSTACKRESTORE(Op, DAG);
  case ISD::FRAMEADDR:
    return LowerFRAMEADDR(Op, DAG);
  case ISD::DYNAMIC_STACKALLOC:
    return LowerDYNAMIC_STACKALLOC(Op, DAG);
  case ISD::VASTART:
    return LowerVASTART(Op, DAG);
  case ISD::VAARG:
    return LowerVAARG(Op, DAG);
  case ISD::VACOPY:
    return LowerVACOPY(Op, DAG);
  case ISD::SHL_PARTS:
    return LowerShiftLeftParts(Op, DAG);
  case ISD::SRA_PARTS:
    return LowerShiftRightParts(Op, DAG, true);
  case ISD::SRL_PARTS:
    return LowerShiftRightParts(Op, DAG, false);
  default:
    report_fatal_error("Unexpected node to lower");
  }
}

TargetLowering::AtomicExpansionKind
GroqTargetLowering::shouldExpandAtomicRMWInIR(const AtomicRMWInst *AI) const {
  return AtomicExpansionKind::CmpXChg;
}

//===----------------------------------------------------------------------===//
// Custom insertion
//===----------------------------------------------------------------------===//

MachineBasicBlock *
GroqTargetLowering::emitSelectCC(MachineInstr &MI,
                                   MachineBasicBlock *MBB) const {
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  MachineOperand &LHS = MI.getOperand(1);
  MachineOperand &RHS = MI.getOperand(2);
  MachineOperand &TrueValue = MI.getOperand(3);
  MachineOperand &FalseValue = MI.getOperand(4);

  // To "insert" a SELECT_CC instruction, we actually have to insert
  // CopyMBB and SinkMBB  blocks and add branch to MBB. We build phi
  // operation in SinkMBB like phi (TrueVakue,FalseValue), where TrueValue
  // is passed from MMB and FalseValue is passed from CopyMBB.
  //   MBB
  //   |   \
  //   |   CopyMBB
  //   |   /
  //   SinkMBB
  // The incoming instruction knows the
  // destination vreg to set, the condition code register to branch on, the
  // true/false values to select between, and a branch opcode to use.
  const BasicBlock *LLVM_BB = MBB->getBasicBlock();
  MachineFunction::iterator It = ++MBB->getIterator();

  MachineFunction *F = MBB->getParent();
  MachineBasicBlock *CopyMBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *SinkMBB = F->CreateMachineBasicBlock(LLVM_BB);

  F->insert(It, CopyMBB);
  F->insert(It, SinkMBB);

  // Transfer the remainder of MBB and its successor edges to SinkMBB.
  SinkMBB->splice(SinkMBB->begin(), MBB,
                  std::next(MachineBasicBlock::iterator(MI)), MBB->end());
  SinkMBB->transferSuccessorsAndUpdatePHIs(MBB);

  MBB->addSuccessor(CopyMBB);
  MBB->addSuccessor(SinkMBB);

  if (MI.getOpcode() == Groq::SELECT_CC_FP_FP ||
      MI.getOpcode() == Groq::SELECT_CC_FP_INT) {
    unsigned CmpKind = MI.getOperand(5).getImm();
    unsigned BrKind = MI.getOperand(6).getImm();
    MCPhysReg BReg = Groq::B0;

    BuildMI(MBB, DL, TII.get(CmpKind), BReg)
        .addReg(LHS.getReg())
        .addReg(RHS.getReg());
    BuildMI(MBB, DL, TII.get(BrKind))
        .addReg(BReg, RegState::Kill)
        .addMBB(SinkMBB);
  } else {
    unsigned BrKind = MI.getOperand(5).getImm();
    BuildMI(MBB, DL, TII.get(BrKind))
        .addReg(LHS.getReg())
        .addReg(RHS.getReg())
        .addMBB(SinkMBB);
  }

  CopyMBB->addSuccessor(SinkMBB);

  //  SinkMBB:
  //   %Result = phi [ %FalseValue, CopyMBB ], [ %TrueValue, MBB ]
  //  ...

  BuildMI(*SinkMBB, SinkMBB->begin(), DL, TII.get(Groq::PHI),
          MI.getOperand(0).getReg())
      .addReg(FalseValue.getReg())
      .addMBB(CopyMBB)
      .addReg(TrueValue.getReg())
      .addMBB(MBB);

  MI.eraseFromParent(); // The pseudo instruction is gone now.
  return SinkMBB;
}

MachineBasicBlock *GroqTargetLowering::EmitInstrWithCustomInserter(
    MachineInstr &MI, MachineBasicBlock *MBB) const {
  DebugLoc DL = MI.getDebugLoc();
  const GroqInstrInfo &TII = *Subtarget.getInstrInfo();

  switch (MI.getOpcode()) {
  case Groq::BRCC_FP: {
    MachineOperand &Cond = MI.getOperand(0);
    MachineOperand &LHS = MI.getOperand(1);
    MachineOperand &RHS = MI.getOperand(2);
    MachineBasicBlock *TargetBB = MI.getOperand(3).getMBB();
    unsigned BrKind = 0;
    unsigned CmpKind = 0;
    ISD::CondCode CondCode = (ISD::CondCode)Cond.getImm();
    MCPhysReg BReg = Groq::B0;

    std::tie(BrKind, CmpKind) = getFPBranchKind(CondCode);
    BuildMI(*MBB, MI, DL, TII.get(CmpKind), BReg)
        .addReg(LHS.getReg())
        .addReg(RHS.getReg());
    BuildMI(*MBB, MI, DL, TII.get(BrKind))
        .addReg(BReg, RegState::Kill)
        .addMBB(TargetBB);

    MI.eraseFromParent();
    return MBB;
  }
  case Groq::SELECT_CC_FP_FP:
  case Groq::SELECT_CC_FP_INT:
  case Groq::SELECT_CC_INT_FP:
  case Groq::SELECT:
    return emitSelectCC(MI, MBB);
  case Groq::S8I:
  case Groq::S16I:
  case Groq::S32I:
  case Groq::S32I_N:
  case Groq::SSI:
  case Groq::SSIP:
  case Groq::SSX:
  case Groq::SSXP:
  case Groq::L8UI:
  case Groq::L16SI:
  case Groq::L16UI:
  case Groq::L32I:
  case Groq::L32I_N:
  case Groq::LSI:
  case Groq::LSIP:
  case Groq::LSX:
  case Groq::LSXP: {
    // Insert memory wait instruction "memw" before volatile load/store as it is
    // implemented in gcc. If memoperands is empty then assume that it aslo
    // maybe volatile load/store and insert "memw".
    if (MI.memoperands_empty() || (*MI.memoperands_begin())->isVolatile()) {
      BuildMI(*MBB, MI, DL, TII.get(Groq::MEMW));
    }
    return MBB;
  }
  case Groq::MOVSP_P: {
    MachineOperand &NewSP = MI.getOperand(0);

    BuildMI(*MBB, MI, DL, TII.get(Groq::MOVSP), Groq::SP)
        .addReg(NewSP.getReg());
    MI.eraseFromParent();

    return MBB;
  }
  case Groq::ATOMIC_CMP_SWAP_32_P: {
    MachineOperand &R = MI.getOperand(0);
    MachineOperand &Addr = MI.getOperand(1);
    MachineOperand &Cmp = MI.getOperand(2);
    MachineOperand &Swap = MI.getOperand(3);

    BuildMI(*MBB, MI, DL, TII.get(Groq::WSR), Groq::SCOMPARE1)
        .addReg(Cmp.getReg());

    BuildMI(*MBB, MI, DL, TII.get(Groq::S32C1I), R.getReg())
        .addReg(Swap.getReg())
        .addReg(Addr.getReg())
        .addImm(0);

    MI.eraseFromParent();
    return MBB;
  }
  default:
    llvm_unreachable("Unexpected instr type to insert");
  }
}
