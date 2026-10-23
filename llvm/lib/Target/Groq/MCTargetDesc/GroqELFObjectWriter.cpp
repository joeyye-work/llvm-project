//===-- GroqMCObjectWriter.cpp - Groq ELF writer ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// TODO(groq-spec): Groq-specific ELF machine/relocation numbers are not public.
// This provisional backend reuses Xtensa ELF constants so asm/object streaming
// paths can be exercised in bring-up tests.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/GroqMCAsmInfo.h"
#include "MCTargetDesc/GroqMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"

using namespace llvm;

namespace {
class GroqObjectWriter : public MCELFObjectTargetWriter {
public:
  explicit GroqObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(false, OSABI, ELF::EM_XTENSA,
                                /*HasRelocationAddend=*/true) {}

protected:
  unsigned getRelocType(const MCFixup &Fixup, const MCValue &Target,
                        bool IsPCRel) const override {
    (void)IsPCRel;
    uint8_t Specifier = Target.getSpecifier();
    switch ((unsigned)Fixup.getKind()) {
    case FK_Data_4:
      return Specifier == Groq::S_TPOFF ? ELF::R_XTENSA_TLS_TPOFF
                                        : ELF::R_XTENSA_32;
    default:
      return ELF::R_XTENSA_SLOT0_OP;
    }
  }

  bool needsRelocateWithSymbol(const MCValue &, unsigned) const override {
    return false;
  }
};
} // namespace

std::unique_ptr<MCObjectTargetWriter>
llvm::createGroqObjectWriter(uint8_t OSABI, bool IsLittleEndian) {
  (void)IsLittleEndian;
  return std::make_unique<GroqObjectWriter>(OSABI);
}
