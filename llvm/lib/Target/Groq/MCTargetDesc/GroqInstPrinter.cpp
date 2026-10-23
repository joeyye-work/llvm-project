//===- GroqInstPrinter.cpp - Convert Groq MCInst to asm syntax --------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints an Groq MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "GroqInstPrinter.h"
#include "MCTargetDesc/GroqMCAsmInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#include "GroqGenAsmWriter.inc"

static void printExpr(const MCExpr *Expr, raw_ostream &OS) {
  int Offset = 0;
  const MCSymbolRefExpr *SRE;

  if (!(SRE = cast<MCSymbolRefExpr>(Expr)))
    assert(false && "Unexpected MCExpr type.");

  assert(SRE->getSpecifier() == 0);

  OS << SRE->getSymbol();

  if (Offset) {
    if (Offset > 0)
      OS << '+';
    OS << Offset;
  }
}

void GroqInstPrinter::printOperand(const MCOperand &MC, raw_ostream &O) {
  if (MC.isReg())
    O << getRegisterName(MC.getReg());
  else if (MC.isImm())
    O << MC.getImm();
  else if (MC.isExpr())
    printExpr(MC.getExpr(), O);
  else
    report_fatal_error("Invalid operand");
}

void GroqInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                  StringRef Annot, const MCSubtargetInfo &STI,
                                  raw_ostream &O) {
  printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void GroqInstPrinter::printRegName(raw_ostream &O, MCRegister Reg) {
  O << getRegisterName(Reg);
}

void GroqInstPrinter::printOperand(const MCInst *MI, int OpNum,
                                     raw_ostream &O) {
  printOperand(MI->getOperand(OpNum), O);
}

void GroqInstPrinter::printMemOperand(const MCInst *MI, int OpNum,
                                        raw_ostream &OS) {
  OS << getRegisterName(MI->getOperand(OpNum).getReg());
  OS << ", ";
  printOperand(MI, OpNum + 1, OS);
}

void GroqInstPrinter::printBranchTarget(const MCInst *MI, uint64_t Address,
                                          int OpNum, raw_ostream &O) {
  const MCOperand &MC = MI->getOperand(OpNum);
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Val = MC.getImm() + 4;
    printPCRelImm(Address, Val, O);
  } else if (MC.isExpr())
    MAI.printExpr(O, *MC.getExpr());
  else
    llvm_unreachable("Invalid operand");
}

void GroqInstPrinter::printLoopTarget(const MCInst *MI, uint64_t Address,
                                        int OpNum, raw_ostream &O) {
  const MCOperand &MC = MI->getOperand(OpNum);
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Val = MC.getImm() + 4;
    printPCRelImm(Address, Val, O);
  } else if (MC.isExpr())
    MAI.printExpr(O, *MC.getExpr());
  else
    llvm_unreachable("Invalid operand");
}

void GroqInstPrinter::printJumpTarget(const MCInst *MI, uint64_t Address,
                                        int OpNum, raw_ostream &O) {
  const MCOperand &MC = MI->getOperand(OpNum);
  if (MC.isImm()) {
    int64_t Val = MC.getImm() + 4;
    printPCRelImm(Address, Val, O);
  } else if (MC.isExpr())
    MAI.printExpr(O, *MC.getExpr());
  else
    llvm_unreachable("Invalid operand");
  ;
}

void GroqInstPrinter::printCallOperand(const MCInst *MI, uint64_t Address,
                                         int OpNum, raw_ostream &O) {
  const MCOperand &MC = MI->getOperand(OpNum);
  if (MC.isImm()) {
    int64_t Val = MC.getImm() + 4;
    if (PrintBranchImmAsAddress) {
      uint64_t Target = Address;
      Target &= ~0x3;
      Target += Val & (~0x3);
      O << formatHex(Target);
    } else {
      O << ". ";
      if (Val > 0)
        O << '+';
      O << Val;
    }
  } else if (MC.isExpr())
    MAI.printExpr(O, *MC.getExpr());
  else
    llvm_unreachable("Invalid operand");
}

void GroqInstPrinter::printL32RTarget(const MCInst *MI, uint64_t Address,
                                        int OpNum, raw_ostream &O) {
  const MCOperand &MC = MI->getOperand(OpNum);
  if (MC.isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    if (PrintBranchImmAsAddress) {
      uint64_t Target = (Address + 0x3) & (~0x3);
      Value &= ~0x3;
      Target += Value;
      O << formatHex(Target);
    } else {
      int64_t InstrOff = Value & 0x3;
      Value -= InstrOff;
      assert((Value >= -262144 && Value <= -4) &&
             "Invalid argument, value must be in ranges [-262144,-4]");
      Value += ((InstrOff + 0x3) & 0x4) - InstrOff;
      printPCRelImm(Address, Value, O);
    }
  } else if (MC.isExpr())
    MAI.printExpr(O, *MC.getExpr());
  else
    llvm_unreachable("Invalid operand");
}

void GroqInstPrinter::printPCRelImm(uint64_t Address, int64_t Offset,
                                      raw_ostream &O) {
  if (PrintBranchImmAsAddress) {
    uint64_t Target = Address + Offset;
    Target &= 0xffffffff;
    O << formatHex(Target);
  } else {
    O << ". ";
    if (Offset > 0)
      O << '+';
    O << Offset;
  }
}

void GroqInstPrinter::printImm8_AsmOperand(const MCInst *MI, int OpNum,
                                             raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert(isInt<8>(Value) &&
           "Invalid argument, value must be in ranges [-128,127]");
    O << Value;
  } else {
    printOperand(MI, OpNum, O);
  }
}

void GroqInstPrinter::printImm8_sh8_AsmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((isInt<16>(Value) && ((Value & 0xFF) == 0)) &&
           "Invalid argument, value must be multiples of 256 in range "
           "[-32768,32512]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printImm12_AsmOperand(const MCInst *MI, int OpNum,
                                              raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= -2048 && Value <= 2047) &&
           "Invalid argument, value must be in ranges [-2048,2047]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printImm12m_AsmOperand(const MCInst *MI, int OpNum,
                                               raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= -2048 && Value <= 2047) &&
           "Invalid argument, value must be in ranges [-2048,2047]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printUimm4_AsmOperand(const MCInst *MI, int OpNum,
                                              raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 15) && "Invalid argument");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printUimm5_AsmOperand(const MCInst *MI, int OpNum,
                                              raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 31) && "Invalid argument");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printShimm1_31_AsmOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 1 && Value <= 31) &&
           "Invalid argument, value must be in range [1,31]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printImm1_16_AsmOperand(const MCInst *MI, int OpNum,
                                                raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 1 && Value <= 16) &&
           "Invalid argument, value must be in range [1,16]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printImm1n_15_AsmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= -1 && (Value != 0) && Value <= 15) &&
           "Invalid argument, value must be in ranges <-1,-1> or <1,15>");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printImm32n_95_AsmOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= -32 && Value <= 95) &&
           "Invalid argument, value must be in ranges <-32,95>");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printImm8n_7_AsmOperand(const MCInst *MI, int OpNum,
                                                raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= -8 && Value <= 7) &&
           "Invalid argument, value must be in ranges <-8,7>");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printImm64n_4n_AsmOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= -64 && Value <= -4) & ((Value & 0x3) == 0) &&
           "Invalid argument, value must be in ranges <-64,-4>");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printOffset8m8_AsmOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert(isUInt<8>(Value) &&
           "Invalid argument, value must be in range [0,255]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printOffset8m16_AsmOperand(const MCInst *MI, int OpNum,
                                                   raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 510 && ((Value & 0x1) == 0)) &&
           "Invalid argument, value must be multiples of two in range [0,510]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printOffset8m32_AsmOperand(const MCInst *MI, int OpNum,
                                                   raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert(
        (Value >= 0 && Value <= 1020 && ((Value & 0x3) == 0)) &&
        "Invalid argument, value must be multiples of four in range [0,1020]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printOffset4m32_AsmOperand(const MCInst *MI, int OpNum,
                                                   raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 60 && ((Value & 0x3) == 0)) &&
           "Invalid argument, value must be multiples of four in range [0,60]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printEntry_Imm12_AsmOperand(const MCInst *MI, int OpNum,
                                                    raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 32760) &&
           "Invalid argument, value must be multiples of eight in range "
           "<0,32760>");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printB4const_AsmOperand(const MCInst *MI, int OpNum,
                                                raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();

    switch (Value) {
    case -1:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 10:
    case 12:
    case 16:
    case 32:
    case 64:
    case 128:
    case 256:
      break;
    default:
      assert((0) && "Invalid B4const argument");
    }
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printB4constu_AsmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();

    switch (Value) {
    case 32768:
    case 65536:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 10:
    case 12:
    case 16:
    case 32:
    case 64:
    case 128:
    case 256:
      break;
    default:
      assert((0) && "Invalid B4constu argument");
    }
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printImm7_22_AsmOperand(const MCInst *MI, int OpNum,
                                                raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 7 && Value <= 22) &&
           "Invalid argument, value must be in range <7,22>");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printSelect_2_AsmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 1) &&
           "Invalid argument, value must be in range [0,1]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printSelect_4_AsmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 3) &&
           "Invalid argument, value must be in range [0,3]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printSelect_8_AsmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 7) &&
           "Invalid argument, value must be in range [0,7]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printSelect_16_AsmOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 15) &&
           "Invalid argument, value must be in range [0,15]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printSelect_256_AsmOperand(const MCInst *MI, int OpNum,
                                                   raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 255) &&
           "Invalid argument, value must be in range [0,255]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printOffset_16_16_AsmOperand(const MCInst *MI,
                                                     int OpNum,
                                                     raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= -128 && Value <= 112 && (Value & 0xf) == 0) &&
           "Invalid argument, value must be in range [-128,112], first 4 bits "
           "should be zero");
    O << Value;
  } else {
    printOperand(MI, OpNum, O);
  }
}

void GroqInstPrinter::printOffset_256_8_AsmOperand(const MCInst *MI,
                                                     int OpNum,
                                                     raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= -1024 && Value <= 1016 && (Value & 0x7) == 0) &&
           "Invalid argument, value must be in range [-1024,1016], first 3 "
           "bits should be zero");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printOffset_256_16_AsmOperand(const MCInst *MI,
                                                      int OpNum,
                                                      raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= -2048 && Value <= 2032 && (Value & 0xf) == 0) &&
           "Invalid argument, value must be in range [-2048,2032], first 4 "
           "bits should be zero");
    O << Value;
  } else {
    printOperand(MI, OpNum, O);
  }
}

void GroqInstPrinter::printOffset_256_4_AsmOperand(const MCInst *MI,
                                                     int OpNum,
                                                     raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= -512 && Value <= 508 && (Value & 0x3) == 0) &&
           "Invalid argument, value must be in range [-512,508], first 2 bits "
           "should be zero");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printOffset_128_2_AsmOperand(const MCInst *MI,
                                                     int OpNum,
                                                     raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 254 && (Value & 0x1) == 0) &&
           "Invalid argument, value must be in range [0,254], first bit should "
           "be zero");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printOffset_128_1_AsmOperand(const MCInst *MI,
                                                     int OpNum,
                                                     raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= 0 && Value <= 127) &&
           "Invalid argument, value must be in range [0,127]");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}

void GroqInstPrinter::printOffset_64_16_AsmOperand(const MCInst *MI,
                                                     int OpNum,
                                                     raw_ostream &O) {
  if (MI->getOperand(OpNum).isImm()) {
    int64_t Value = MI->getOperand(OpNum).getImm();
    assert((Value >= -512 && Value <= 496 && (Value & 0xf) == 0) &&
           "Invalid argument, value must be in range [-512,496], first 4 bits "
           "should be zero");
    O << Value;
  } else
    printOperand(MI, OpNum, O);
}
