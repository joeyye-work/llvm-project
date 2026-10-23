//===- GroqConstantPoolValue.h - Groq constantpool value ----*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_GROQ_GROQCONSTANTPOOLVALUE_H
#define LLVM_LIB_TARGET_GROQ_GROQCONSTANTPOOLVALUE_H

#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include <cstddef>
#include <string>
#include <vector>

namespace llvm {

class BlockAddress;
class Constant;
class GlobalValue;
class LLVMContext;
class MachineBasicBlock;

namespace GroqCP {
enum GroqCPKind {
  CPExtSymbol,
  CPBlockAddress,
  CPMachineBasicBlock,
  CPJumpTable
};

enum GroqCPModifier {
  no_modifier, // None
  TPOFF        // Thread Pointer Offset
};
} // namespace GroqCP

/// GroqConstantPoolValue - Groq specific constantpool value. This is used
/// to represent PC-relative displacement between the address of the load
/// instruction and the constant being loaded.
class GroqConstantPoolValue : public MachineConstantPoolValue {
  unsigned LabelId;                    // Label id of the load.
  GroqCP::GroqCPKind Kind;         // Kind of constant.
  GroqCP::GroqCPModifier Modifier; // Symbol name modifier
                                       //(for example Global Variable name)

protected:
  GroqConstantPoolValue(
      Type *Ty, unsigned ID, GroqCP::GroqCPKind Kind,
      GroqCP::GroqCPModifier Modifier = GroqCP::no_modifier);

  GroqConstantPoolValue(
      LLVMContext &C, unsigned id, GroqCP::GroqCPKind Kind,
      GroqCP::GroqCPModifier Modifier = GroqCP::no_modifier);

  template <typename Derived>
  int getExistingMachineCPValueImpl(MachineConstantPool *CP, Align Alignment) {
    const std::vector<MachineConstantPoolEntry> &Constants = CP->getConstants();
    for (unsigned i = 0, e = Constants.size(); i != e; ++i) {
      if (Constants[i].isMachineConstantPoolEntry() &&
          (Constants[i].getAlign() >= Alignment)) {
        auto *CPV = static_cast<GroqConstantPoolValue *>(
            Constants[i].Val.MachineCPVal);
        if (Derived *APC = dyn_cast<Derived>(CPV))
          if (cast<Derived>(this)->equals(APC))
            return i;
      }
    }

    return -1;
  }

public:
  ~GroqConstantPoolValue() override;

  GroqCP::GroqCPModifier getModifier() const { return Modifier; }
  bool hasModifier() const { return Modifier != GroqCP::no_modifier; }
  StringRef getModifierText() const;

  unsigned getLabelId() const { return LabelId; }
  void setLabelId(unsigned ID) { LabelId = ID; }

  bool isExtSymbol() const { return Kind == GroqCP::CPExtSymbol; }
  bool isBlockAddress() const { return Kind == GroqCP::CPBlockAddress; }
  bool isMachineBasicBlock() const {
    return Kind == GroqCP::CPMachineBasicBlock;
  }
  bool isJumpTable() const { return Kind == GroqCP::CPJumpTable; }

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;

  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;

  /// hasSameValue - Return true if this Groq constpool value can share the
  /// same constantpool entry as another Groq constpool value.
  virtual bool hasSameValue(GroqConstantPoolValue *ACPV);

  bool equals(const GroqConstantPoolValue *A) const {
    return this->LabelId == A->LabelId && this->Modifier == A->Modifier;
  }

  void print(raw_ostream &O) const override;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
#endif
};

inline raw_ostream &operator<<(raw_ostream &O,
                               const GroqConstantPoolValue &V) {
  V.print(O);
  return O;
}

/// GroqConstantPoolConstant - Groq-specific constant pool values for
/// Constants (for example BlockAddresses).
class GroqConstantPoolConstant : public GroqConstantPoolValue {
  const Constant *CVal; // Constant being loaded.

  GroqConstantPoolConstant(const Constant *C, unsigned ID,
                             GroqCP::GroqCPKind Kind);

public:
  static GroqConstantPoolConstant *Create(const Constant *C, unsigned ID,
                                            GroqCP::GroqCPKind Kind);

  const BlockAddress *getBlockAddress() const;

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;

  /// hasSameValue - Return true if this Groq constpool value can share the
  /// same constantpool entry as another Groq constpool value.
  bool hasSameValue(GroqConstantPoolValue *ACPV) override;

  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;

  void print(raw_ostream &O) const override;
  static bool classof(const GroqConstantPoolValue *APV) {
    return APV->isBlockAddress();
  }

  bool equals(const GroqConstantPoolConstant *A) const {
    return CVal == A->CVal && GroqConstantPoolValue::equals(A);
  }
};

/// GroqConstantPoolSymbol - Groq-specific constantpool values for external
/// symbols.
class GroqConstantPoolSymbol : public GroqConstantPoolValue {
  const std::string S; // ExtSymbol being loaded.
  bool PrivateLinkage;

  GroqConstantPoolSymbol(
      LLVMContext &C, const char *S, unsigned Id, bool PrivLinkage,
      GroqCP::GroqCPModifier Modifier = GroqCP::no_modifier);

public:
  static GroqConstantPoolSymbol *
  Create(LLVMContext &C, const char *S, unsigned ID, bool PrivLinkage,
         GroqCP::GroqCPModifier Modifier = GroqCP::no_modifier);

  const char *getSymbol() const { return S.c_str(); }

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;

  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;

  /// hasSameValue - Return true if this Groq constpool value can share the
  /// same constantpool entry as another Groq constpool value.
  bool hasSameValue(GroqConstantPoolValue *ACPV) override;

  bool isPrivateLinkage() { return PrivateLinkage; }

  void print(raw_ostream &O) const override;

  static bool classof(const GroqConstantPoolValue *ACPV) {
    return ACPV->isExtSymbol();
  }

  bool equals(const GroqConstantPoolSymbol *A) const {
    return S == A->S && GroqConstantPoolValue::equals(A);
  }
};

/// GroqConstantPoolMBB - Groq-specific constantpool value of a machine
/// basic block.
class GroqConstantPoolMBB : public GroqConstantPoolValue {
  const MachineBasicBlock *MBB; // Machine basic block.

  GroqConstantPoolMBB(LLVMContext &C, const MachineBasicBlock *M,
                        unsigned ID);

public:
  static GroqConstantPoolMBB *Create(LLVMContext &C,
                                       const MachineBasicBlock *M, unsigned ID);

  const MachineBasicBlock *getMBB() const { return MBB; }

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;

  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;

  /// hasSameValue - Return true if this Groq constpool value can share the
  /// same constantpool entry as another Groq constpool value.
  bool hasSameValue(GroqConstantPoolValue *ACPV) override;

  void print(raw_ostream &O) const override;

  static bool classof(const GroqConstantPoolValue *ACPV) {
    return ACPV->isMachineBasicBlock();
  }

  bool equals(const GroqConstantPoolMBB *A) const {
    return MBB == A->MBB && GroqConstantPoolValue::equals(A);
  }
};

/// GroqConstantPoolJumpTable - Groq-specific constantpool values for Jump
/// Table symbols.
class GroqConstantPoolJumpTable : public GroqConstantPoolValue {
  unsigned Idx; // Jump Table Index.

  GroqConstantPoolJumpTable(LLVMContext &C, unsigned Idx);

public:
  static GroqConstantPoolJumpTable *Create(LLVMContext &C, unsigned Idx);

  unsigned getIndex() const { return Idx; }

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;

  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;

  /// hasSameValue - Return true if this Groq constpool value can share the
  /// same constantpool entry as another Groq constpool value.
  bool hasSameValue(GroqConstantPoolValue *ACPV) override;

  void print(raw_ostream &O) const override;

  static bool classof(const GroqConstantPoolValue *ACPV) {
    return ACPV->isJumpTable();
  }

  bool equals(const GroqConstantPoolJumpTable *A) const {
    return Idx == A->Idx && GroqConstantPoolValue::equals(A);
  }
};

} // namespace llvm

#endif /* LLVM_LIB_TARGET_GROQ_GROQCONSTANTPOOLVALUE_H */
