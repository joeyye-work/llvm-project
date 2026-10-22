//===- Groq.td - Describe the Groq Target Machine ------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

// TODO(groq-spec): This backend currently uses a provisional scalar model for
// bring-up while public Groq LPU ISA/ABI documentation remains incomplete.
// Replace register/encoding/scheduling assumptions with official definitions.

//===----------------------------------------------------------------------===//
// Subtarget Features.
//===----------------------------------------------------------------------===//

include "GroqFeatures.td"

//===----------------------------------------------------------------------===//
// Groq supported processors.
//===----------------------------------------------------------------------===//

include "GroqProcessors.td"

//===----------------------------------------------------------------------===//
// Register File Description
//===----------------------------------------------------------------------===//

include "GroqRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling Convention Description
//===----------------------------------------------------------------------===//

include "GroqCallingConv.td"

//===----------------------------------------------------------------------===//
// Instruction Descriptions
//===----------------------------------------------------------------------===//

include "GroqInstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<AR>;

def GroqInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Target Declaration
//===----------------------------------------------------------------------===//

def GroqAsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = 1;
  let AllowDuplicateRegisterNames = 1;
}

def GroqInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
}

def Groq : Target {
  let InstructionSet = GroqInstrInfo;
  let AssemblyWriters = [GroqInstPrinter];
  let AssemblyParsers = [GroqAsmParser];
}
