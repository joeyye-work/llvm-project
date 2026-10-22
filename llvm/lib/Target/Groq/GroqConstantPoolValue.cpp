//===- GroqConstantPoolValue.cpp - Groq constantpool value ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Groq specific constantpool value class.
//
//===----------------------------------------------------------------------===//

#include "GroqConstantPoolValue.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
using namespace llvm;

GroqConstantPoolValue::GroqConstantPoolValue(
    Type *Ty, unsigned ID, GroqCP::GroqCPKind Kind,
    GroqCP::GroqCPModifier modifier)
    : MachineConstantPoolValue(Ty), LabelId(ID), Kind(Kind),
      Modifier(modifier) {}

GroqConstantPoolValue::GroqConstantPoolValue(
    LLVMContext &C, unsigned ID, GroqCP::GroqCPKind Kind,
    GroqCP::GroqCPModifier Modifier)
    : MachineConstantPoolValue((Type *)Type::getInt32Ty(C)), LabelId(ID),
      Kind(Kind), Modifier(Modifier) {}

GroqConstantPoolValue::~GroqConstantPoolValue() {}

StringRef GroqConstantPoolValue::getModifierText() const {
  switch (Modifier) {
  case GroqCP::no_modifier:
    return "";
  case GroqCP::TPOFF:
    return "@TPOFF";
  }
  report_fatal_error("Unknown modifier!");
}

int GroqConstantPoolValue::getExistingMachineCPValue(MachineConstantPool *CP,
                                                       Align Alignment) {
  report_fatal_error("Shouldn't be calling this directly!");
}

void GroqConstantPoolValue::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  ID.AddInteger(LabelId);
}

bool GroqConstantPoolValue::hasSameValue(GroqConstantPoolValue *ACPV) {
  if (ACPV->Kind == Kind) {
    if (ACPV->LabelId == LabelId)
      return true;
  }
  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void GroqConstantPoolValue::dump() const { errs() << "  " << *this; }
#endif

void GroqConstantPoolValue::print(raw_ostream &O) const {}

//===----------------------------------------------------------------------===//
// GroqConstantPoolConstant
//===----------------------------------------------------------------------===//

GroqConstantPoolConstant::GroqConstantPoolConstant(
    const Constant *C, unsigned ID, GroqCP::GroqCPKind Kind)
    : GroqConstantPoolValue(C->getType(), ID, Kind), CVal(C) {}

GroqConstantPoolConstant *
GroqConstantPoolConstant::Create(const Constant *C, unsigned ID,
                                   GroqCP::GroqCPKind Kind) {
  return new GroqConstantPoolConstant(C, ID, Kind);
}

const BlockAddress *GroqConstantPoolConstant::getBlockAddress() const {
  return dyn_cast_or_null<BlockAddress>(CVal);
}

int GroqConstantPoolConstant::getExistingMachineCPValue(
    MachineConstantPool *CP, Align Alignment) {
  return getExistingMachineCPValueImpl<GroqConstantPoolConstant>(CP,
                                                                   Alignment);
}

bool GroqConstantPoolConstant::hasSameValue(GroqConstantPoolValue *ACPV) {
  const GroqConstantPoolConstant *ACPC =
      dyn_cast<GroqConstantPoolConstant>(ACPV);
  return ACPC && ACPC->CVal == CVal &&
         GroqConstantPoolValue::hasSameValue(ACPV);
}

void GroqConstantPoolConstant::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  ID.AddPointer(CVal);
  GroqConstantPoolValue::addSelectionDAGCSEId(ID);
}

void GroqConstantPoolConstant::print(raw_ostream &O) const {
  O << CVal->getName();
  GroqConstantPoolValue::print(O);
}

GroqConstantPoolSymbol::GroqConstantPoolSymbol(
    LLVMContext &C, const char *Str, unsigned ID, bool PrivLinkage,
    GroqCP::GroqCPModifier Modifier)
    : GroqConstantPoolValue(C, ID, GroqCP::CPExtSymbol, Modifier), S(Str),
      PrivateLinkage(PrivLinkage) {}

GroqConstantPoolSymbol *
GroqConstantPoolSymbol::Create(LLVMContext &C, const char *Str, unsigned ID,
                                 bool PrivLinkage,
                                 GroqCP::GroqCPModifier Modifier)

{
  return new GroqConstantPoolSymbol(C, Str, ID, PrivLinkage, Modifier);
}

int GroqConstantPoolSymbol::getExistingMachineCPValue(MachineConstantPool *CP,
                                                        Align Alignment) {
  return getExistingMachineCPValueImpl<GroqConstantPoolSymbol>(CP, Alignment);
}

bool GroqConstantPoolSymbol::hasSameValue(GroqConstantPoolValue *ACPV) {
  const GroqConstantPoolSymbol *ACPS =
      dyn_cast<GroqConstantPoolSymbol>(ACPV);
  return ACPS && ACPS->S == S && GroqConstantPoolValue::hasSameValue(ACPV);
}

void GroqConstantPoolSymbol::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  ID.AddString(S);
  GroqConstantPoolValue::addSelectionDAGCSEId(ID);
}

void GroqConstantPoolSymbol::print(raw_ostream &O) const {
  O << S;
  GroqConstantPoolValue::print(O);
}

GroqConstantPoolMBB::GroqConstantPoolMBB(LLVMContext &C,
                                             const MachineBasicBlock *M,
                                             unsigned Id)
    : GroqConstantPoolValue(C, 0, GroqCP::CPMachineBasicBlock), MBB(M) {}

GroqConstantPoolMBB *GroqConstantPoolMBB::Create(LLVMContext &C,
                                                     const MachineBasicBlock *M,
                                                     unsigned Idx) {
  return new GroqConstantPoolMBB(C, M, Idx);
}

int GroqConstantPoolMBB::getExistingMachineCPValue(MachineConstantPool *CP,
                                                     Align Alignment) {
  return getExistingMachineCPValueImpl<GroqConstantPoolMBB>(CP, Alignment);
}

bool GroqConstantPoolMBB::hasSameValue(GroqConstantPoolValue *ACPV) {
  const GroqConstantPoolMBB *ACPMBB = dyn_cast<GroqConstantPoolMBB>(ACPV);
  return ACPMBB && ACPMBB->MBB == MBB &&
         GroqConstantPoolValue::hasSameValue(ACPV);
}

void GroqConstantPoolMBB::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  ID.AddPointer(MBB);
  GroqConstantPoolValue::addSelectionDAGCSEId(ID);
}

void GroqConstantPoolMBB::print(raw_ostream &O) const {
  O << "BB#" << MBB->getNumber();
  GroqConstantPoolValue::print(O);
}

GroqConstantPoolJumpTable::GroqConstantPoolJumpTable(LLVMContext &C,
                                                         unsigned Index)
    : GroqConstantPoolValue(C, 0, GroqCP::CPJumpTable), Idx(Index) {}

GroqConstantPoolJumpTable *GroqConstantPoolJumpTable::Create(LLVMContext &C,
                                                                 unsigned Idx) {
  return new GroqConstantPoolJumpTable(C, Idx);
}

int GroqConstantPoolJumpTable::getExistingMachineCPValue(
    MachineConstantPool *CP, Align Alignment) {
  return getExistingMachineCPValueImpl<GroqConstantPoolJumpTable>(CP,
                                                                    Alignment);
}

bool GroqConstantPoolJumpTable::hasSameValue(GroqConstantPoolValue *ACPV) {
  const GroqConstantPoolJumpTable *ACPJT =
      dyn_cast<GroqConstantPoolJumpTable>(ACPV);
  return ACPJT && ACPJT->Idx == Idx &&
         GroqConstantPoolValue::hasSameValue(ACPV);
}

void GroqConstantPoolJumpTable::addSelectionDAGCSEId(FoldingSetNodeID &ID) {}

void GroqConstantPoolJumpTable::print(raw_ostream &O) const {
  O << "JT" << Idx;
  GroqConstantPoolValue::print(O);
}
