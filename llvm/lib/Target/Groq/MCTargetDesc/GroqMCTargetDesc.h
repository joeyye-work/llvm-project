//===-- GroqMCTargetDesc.h - Groq Target Descriptions -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Groq specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_GROQ_MCTARGETDESC_GROQMCTARGETDESC_H
#define LLVM_LIB_TARGET_GROQ_MCTARGETDESC_GROQMCTARGETDESC_H
#include "llvm/Support/DataTypes.h"
#include <memory>

namespace llvm {

class FeatureBitset;
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCObjectWriter;
class MCRegister;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class MachineInstr;
class StringRef;
class Target;
class raw_ostream;

extern Target TheGroqTarget;

MCCodeEmitter *createGroqMCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx);

MCAsmBackend *createGroqAsmBackend(const Target &T,
                                     const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &MRI,
                                     const MCTargetOptions &Options);
std::unique_ptr<MCObjectTargetWriter>
createGroqObjectWriter(uint8_t OSABI, bool IsLittleEndian);

namespace Groq {
// Check address offset for load/store instructions.
// The offset should be multiple of scale.
bool isValidAddrOffset(int Scale, int64_t OffsetVal);

// Check address offset for load/store instructions.
bool isValidAddrOffsetForOpcode(unsigned Opcode, int64_t Offset);

enum RegisterAccessType {
  REGISTER_WRITE = 1,
  REGISTER_READ = 2,
  REGISTER_EXCHANGE = 3
};

// Verify if it's correct to use a special register.
bool checkRegister(MCRegister RegNo, const FeatureBitset &FeatureBits,
                   RegisterAccessType RA);

// Get Groq User Register by register encoding value.
MCRegister getUserRegister(unsigned Code, const MCRegisterInfo &MRI);
} // namespace Groq
} // end namespace llvm

// Defines symbolic names for Groq registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "GroqGenRegisterInfo.inc"

// Defines symbolic names for the Groq instructions.
#define GET_INSTRINFO_ENUM
#include "GroqGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "GroqGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_GROQ_MCTARGETDESC_GROQMCTARGETDESC_H
