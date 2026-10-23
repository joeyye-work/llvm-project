//===-- GroqMCAsmInfo.h - Groq Asm Info --------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the GroqMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_GROQ_MCTARGETDESC_GROQTARGETASMINFO_H
#define LLVM_LIB_TARGET_GROQ_MCTARGETDESC_GROQTARGETASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/MC/MCExpr.h"

namespace llvm {
class Triple;
class StringRef;

class GroqMCAsmInfo : public MCAsmInfoELF {
public:
  explicit GroqMCAsmInfo(const Triple &TT, const MCTargetOptions &Options);

  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
};

namespace Groq {
enum Specifier { S_None, S_TPOFF };

uint8_t parseSpecifier(StringRef name);
StringRef getSpecifierName(uint8_t S);
} // namespace Groq

} // namespace llvm

#endif // LLVM_LIB_TARGET_GROQ_MCTARGETDESC_GROQTARGETASMINFO_H
