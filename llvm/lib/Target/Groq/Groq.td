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

// TODO(groq-spec): Public TSP documentation (e.g. ISCA 2020) describes a
// functionally sliced streaming architecture. The current backend is a
// provisional scalar stand-in so LLVM can build and run smoke tests while
// official ISA/ABI/encoding specifications are unavailable.
// Replace placeholder register-like compiler plumbing and encoding/scheduling
// assumptions with official stream-based definitions.

//===----------------------------------------------------------------------===//
// Subtarget Features.
//===----------------------------------------------------------------------===//

include "GroqFeatures.td"

//===----------------------------------------------------------------------===//
// Scheduling model (provisional, paper-grounded placeholders).
//===----------------------------------------------------------------------===//

include "GroqSchedule.td"

//===----------------------------------------------------------------------===//
// Groq supported processors.
//===----------------------------------------------------------------------===//

include "GroqProcessors.td"

//===----------------------------------------------------------------------===//
// Placeholder compiler operand-state description
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
