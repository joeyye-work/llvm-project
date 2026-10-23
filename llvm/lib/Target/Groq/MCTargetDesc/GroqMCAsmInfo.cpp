//===-- GroqMCAsmInfo.cpp - Groq Asm Properties -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the GroqMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "GroqMCAsmInfo.h"
#include "llvm/ADT/Enum.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

constexpr EnumStringDef<MCAsmInfo::AtSpecifierKind> AtSpecifierDefs[] = {
    {{"TPOFF"}, Groq::S_TPOFF},
};
constexpr auto atSpecifiers = BUILD_ENUM_STRINGS(AtSpecifierDefs);

GroqMCAsmInfo::GroqMCAsmInfo(const Triple &TT,
                                 const MCTargetOptions &Options)
    : MCAsmInfoELF(Options) {
  CodePointerSize = 4;
  CalleeSaveStackSlotSize = 4;
  InternalSymbolPrefix = ".L";
  CommentString = "#";
  ZeroDirective = "\t.space\t";
  Data64bitsDirective = "\t.quad\t";
  GlobalDirective = "\t.global\t";
  UsesELFSectionDirectiveForBSS = true;
  SupportsDebugInformation = true;
  ExceptionsType = ExceptionHandling::DwarfCFI;
  AlignmentIsInBytes = false;

  initializeAtSpecifiers(atSpecifiers);
}

void GroqMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                         const MCSpecifierExpr &Expr) const {
  StringRef S = Groq::getSpecifierName(Expr.getSpecifier());
  if (!S.empty())
    OS << '%' << S << '(';
  printExpr(OS, *Expr.getSubExpr());
  if (!S.empty())
    OS << ')';
}

uint8_t Groq::parseSpecifier(StringRef name) { return 0; }

StringRef Groq::getSpecifierName(uint8_t S) {
  switch (S) {
  default:
    llvm_unreachable("Invalid ELF symbol kind");
  }
}
