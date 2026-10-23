//===-- GroqMCTargetDesc.cpp - Groq target descriptions ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "GroqMCTargetDesc.h"
#include "TargetInfo/GroqTargetInfo.h"
#include "GroqInstPrinter.h"
#include "GroqMCAsmInfo.h"
#include "GroqTargetStreamer.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_INSTRINFO_MC_DESC
#include "GroqGenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "GroqGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "GroqGenSubtargetInfo.inc"

using namespace llvm;

bool Groq::isValidAddrOffset(int Scale, int64_t OffsetVal) {
  bool Valid = false;

  switch (Scale) {
  case 1:
    Valid = (OffsetVal >= 0 && OffsetVal <= 255);
    break;
  case 2:
    Valid = (OffsetVal >= 0 && OffsetVal <= 510) && ((OffsetVal & 0x1) == 0);
    break;
  case 4:
    Valid = (OffsetVal >= 0 && OffsetVal <= 1020) && ((OffsetVal & 0x3) == 0);
    break;
  default:
    break;
  }
  return Valid;
}

bool Groq::isValidAddrOffsetForOpcode(unsigned Opcode, int64_t Offset) {
  int Scale = 0;

  switch (Opcode) {
  case Groq::L8UI:
  case Groq::S8I:
    Scale = 1;
    break;
  case Groq::L16SI:
  case Groq::L16UI:
  case Groq::S16I:
    Scale = 2;
    break;
  case Groq::LEA_ADD:
    return (Offset >= -128 && Offset <= 127);
  default:
    // assume that MI is 32-bit load/store operation
    Scale = 4;
    break;
  }
  return isValidAddrOffset(Scale, Offset);
}

// Verify Special Register
bool Groq::checkRegister(MCRegister RegNo, const FeatureBitset &FeatureBits,
                           RegisterAccessType RAType) {
  switch (RegNo) {
  case Groq::BREG:
    return FeatureBits[Groq::FeatureBoolean];
  case Groq::CCOUNT:
  case Groq::CCOMPARE0:
    if (FeatureBits[Groq::FeatureTimers1])
      return true;
    [[fallthrough]];
  case Groq::CCOMPARE1:
    if (FeatureBits[Groq::FeatureTimers2])
      return true;
    [[fallthrough]];
  case Groq::CCOMPARE2:
    if (FeatureBits[Groq::FeatureTimers3])
      return true;
    return false;
  case Groq::CONFIGID0:
    return RAType != Groq::REGISTER_EXCHANGE;
  case Groq::CONFIGID1:
    return RAType == Groq::REGISTER_READ;
  case Groq::CPENABLE:
    return FeatureBits[Groq::FeatureCoprocessor];
  case Groq::DEBUGCAUSE:
    return RAType == Groq::REGISTER_READ && FeatureBits[Groq::FeatureDebug];
  case Groq::DEPC:
  case Groq::EPC1:
  case Groq::EXCCAUSE:
  case Groq::EXCSAVE1:
  case Groq::EXCVADDR:
    return FeatureBits[Groq::FeatureException];
    [[fallthrough]];
  case Groq::EPC2:
  case Groq::EPS2:
  case Groq::EXCSAVE2:
    if (FeatureBits[Groq::FeatureHighPriInterrupts])
      return true;
    [[fallthrough]];
  case Groq::EPC3:
  case Groq::EPS3:
  case Groq::EXCSAVE3:
    if (FeatureBits[Groq::FeatureHighPriInterruptsLevel3])
      return true;
    [[fallthrough]];
  case Groq::EPC4:
  case Groq::EPS4:
  case Groq::EXCSAVE4:
    if (FeatureBits[Groq::FeatureHighPriInterruptsLevel4])
      return true;
    [[fallthrough]];
  case Groq::EPC5:
  case Groq::EPS5:
  case Groq::EXCSAVE5:
    if (FeatureBits[Groq::FeatureHighPriInterruptsLevel5])
      return true;
    [[fallthrough]];
  case Groq::EPC6:
  case Groq::EPS6:
  case Groq::EXCSAVE6:
    if (FeatureBits[Groq::FeatureHighPriInterruptsLevel6])
      return true;
    [[fallthrough]];
  case Groq::EPC7:
  case Groq::EPS7:
  case Groq::EXCSAVE7:
    if (FeatureBits[Groq::FeatureHighPriInterruptsLevel7])
      return true;
    return false;
  case Groq::INTENABLE:
    return FeatureBits[Groq::FeatureInterrupt];
  case Groq::INTERRUPT:
    return RAType == Groq::REGISTER_READ &&
           FeatureBits[Groq::FeatureInterrupt];
  case Groq::INTSET:
  case Groq::INTCLEAR:
    return RAType == Groq::REGISTER_WRITE &&
           FeatureBits[Groq::FeatureInterrupt];
  case Groq::ICOUNT:
  case Groq::ICOUNTLEVEL:
  case Groq::IBREAKENABLE:
  case Groq::DDR:
  case Groq::IBREAKA0:
  case Groq::IBREAKA1:
  case Groq::DBREAKA0:
  case Groq::DBREAKA1:
  case Groq::DBREAKC0:
  case Groq::DBREAKC1:
    return FeatureBits[Groq::FeatureDebug];
  case Groq::LBEG:
  case Groq::LEND:
  case Groq::LCOUNT:
    return FeatureBits[Groq::FeatureLoop];
  case Groq::LITBASE:
    return FeatureBits[Groq::FeatureExtendedL32R];
  case Groq::MEMCTL:
    return FeatureBits[Groq::FeatureDataCache];
  case Groq::ACCLO:
  case Groq::ACCHI:
  case Groq::M0:
  case Groq::M1:
  case Groq::M2:
  case Groq::M3:
    return FeatureBits[Groq::FeatureMAC16];
  case Groq::MISC0:
  case Groq::MISC1:
  case Groq::MISC2:
  case Groq::MISC3:
    return FeatureBits[Groq::FeatureMiscSR];
  case Groq::PRID:
    return RAType == Groq::REGISTER_READ && FeatureBits[Groq::FeaturePRID];
  case Groq::THREADPTR:
    return FeatureBits[FeatureTHREADPTR];
  case Groq::VECBASE:
    return FeatureBits[Groq::FeatureRelocatableVector];
  case Groq::FCR:
  case Groq::FSR:
    return FeatureBits[FeatureSingleFloat];
  case Groq::F64R_LO:
  case Groq::F64R_HI:
  case Groq::F64S:
    return FeatureBits[FeatureDFPAccel];
  case Groq::WINDOWBASE:
  case Groq::WINDOWSTART:
    return FeatureBits[Groq::FeatureWindowed];
  case Groq::ATOMCTL:
  case Groq::SCOMPARE1:
    return FeatureBits[Groq::FeatureS32C1I];
    return FeatureBits[Groq::FeatureWindowed];
  case Groq::GPIO_OUT_S2:
    return FeatureBits[Groq::FeatureESP32S2Ops];
  case Groq::ACCX_0:
  case Groq::ACCX_1:
  case Groq::QACC_H_0:
  case Groq::QACC_H_1:
  case Groq::QACC_H_2:
  case Groq::QACC_H_3:
  case Groq::QACC_H_4:
  case Groq::QACC_L_0:
  case Groq::QACC_L_1:
  case Groq::QACC_L_2:
  case Groq::QACC_L_3:
  case Groq::QACC_L_4:
  case Groq::GPIO_OUT_S3:
  case Groq::FFT_BIT_WIDTH:
  case Groq::UA_STATE_0:
  case Groq::UA_STATE_1:
  case Groq::UA_STATE_2:
  case Groq::UA_STATE_3:
    return FeatureBits[Groq::FeatureESP32S3Ops];
  case Groq::NoRegister:
    return false;
  }

  return true;
}

// Get Groq User Register by encoding value.
MCRegister Groq::getUserRegister(unsigned Code, const MCRegisterInfo &MRI) {
  MCRegister UserReg = Groq::NoRegister;

  if (MRI.getEncodingValue(Groq::FCR) == Code) {
    UserReg = Groq::FCR;
  } else if (MRI.getEncodingValue(Groq::FSR) == Code) {
    UserReg = Groq::FSR;
  } else if (MRI.getEncodingValue(Groq::F64R_LO) == Code) {
    UserReg = Groq::F64R_LO;
  } else if (MRI.getEncodingValue(Groq::F64R_HI) == Code) {
    UserReg = Groq::F64R_HI;
  } else if (MRI.getEncodingValue(Groq::F64S) == Code) {
    UserReg = Groq::F64S;
  } else if (MRI.getEncodingValue(Groq::THREADPTR) == Code) {
    UserReg = Groq::THREADPTR;
  } else if (MRI.getEncodingValue(Groq::GPIO_OUT_S2) == Code) {
    UserReg = Groq::GPIO_OUT_S2;
  } else if (MRI.getEncodingValue(Groq::GPIO_OUT_S3) == Code) {
    UserReg = Groq::GPIO_OUT_S3;
  } else if (MRI.getEncodingValue(Groq::ACCX_0) == Code) {
    UserReg = Groq::ACCX_0;
  } else if (MRI.getEncodingValue(Groq::ACCX_1) == Code) {
    UserReg = Groq::ACCX_1;
  } else if (MRI.getEncodingValue(Groq::QACC_H_0) == Code) {
    UserReg = Groq::QACC_H_0;
  } else if (MRI.getEncodingValue(Groq::QACC_H_1) == Code) {
    UserReg = Groq::QACC_H_1;
  } else if (MRI.getEncodingValue(Groq::QACC_H_2) == Code) {
    UserReg = Groq::QACC_H_2;
  } else if (MRI.getEncodingValue(Groq::QACC_H_3) == Code) {
    UserReg = Groq::QACC_H_3;
  } else if (MRI.getEncodingValue(Groq::QACC_H_4) == Code) {
    UserReg = Groq::QACC_H_4;
  } else if (MRI.getEncodingValue(Groq::QACC_L_0) == Code) {
    UserReg = Groq::QACC_L_0;
  } else if (MRI.getEncodingValue(Groq::QACC_L_1) == Code) {
    UserReg = Groq::QACC_L_1;
  } else if (MRI.getEncodingValue(Groq::QACC_L_2) == Code) {
    UserReg = Groq::QACC_L_2;
  } else if (MRI.getEncodingValue(Groq::QACC_L_3) == Code) {
    UserReg = Groq::QACC_L_3;
  } else if (MRI.getEncodingValue(Groq::QACC_L_4) == Code) {
    UserReg = Groq::QACC_L_4;
  } else if (MRI.getEncodingValue(Groq::FFT_BIT_WIDTH) == Code) {
    UserReg = Groq::FFT_BIT_WIDTH;
  } else if (MRI.getEncodingValue(Groq::SAR_BYTE) == Code) {
    UserReg = Groq::SAR_BYTE;
  } else if (MRI.getEncodingValue(Groq::UA_STATE_0) == Code) {
    UserReg = Groq::UA_STATE_0;
  } else if (MRI.getEncodingValue(Groq::UA_STATE_1) == Code) {
    UserReg = Groq::UA_STATE_1;
  } else if (MRI.getEncodingValue(Groq::UA_STATE_2) == Code) {
    UserReg = Groq::UA_STATE_2;
  } else if (MRI.getEncodingValue(Groq::UA_STATE_3) == Code) {
    UserReg = Groq::UA_STATE_3;
  }

  return UserReg;
}

static MCAsmInfo *createGroqMCAsmInfo(const MCRegisterInfo &MRI,
                                        const Triple &TT,
                                        const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new GroqMCAsmInfo(TT, Options);
  return MAI;
}

static MCInstrInfo *createGroqMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitGroqMCInstrInfo(X);
  return X;
}

static MCInstPrinter *createGroqMCInstPrinter(const Triple &TT,
                                                unsigned SyntaxVariant,
                                                const MCAsmInfo &MAI,
                                                const MCInstrInfo &MII,
                                                const MCRegisterInfo &MRI) {
  return new GroqInstPrinter(MAI, MII, MRI);
}

static MCRegisterInfo *createGroqMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitGroqMCRegisterInfo(X, Groq::SP);
  return X;
}

static MCSubtargetInfo *
createGroqMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createGroqMCSubtargetInfoImpl(TT, CPU, CPU, FS);
}

static MCTargetStreamer *
createGroqAsmTargetStreamer(MCStreamer &S, formatted_raw_ostream &OS,
                              MCInstPrinter *InstPrint) {
  return new GroqTargetAsmStreamer(S, OS);
}

static MCTargetStreamer *
createGroqObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new GroqTargetELFStreamer(S);
}

static MCTargetStreamer *createGroqNullTargetStreamer(MCStreamer &S) {
  return new GroqTargetStreamer(S);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeGroqTargetMC() {
  // Register the MCAsmInfo.
  TargetRegistry::RegisterMCAsmInfo(getTheGroqTarget(),
                                    createGroqMCAsmInfo);

  // Register the MCCodeEmitter.
  TargetRegistry::RegisterMCCodeEmitter(getTheGroqTarget(),
                                        createGroqMCCodeEmitter);

  // Register the MCInstrInfo.
  TargetRegistry::RegisterMCInstrInfo(getTheGroqTarget(),
                                      createGroqMCInstrInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheGroqTarget(),
                                        createGroqMCInstPrinter);

  // Register the MCRegisterInfo.
  TargetRegistry::RegisterMCRegInfo(getTheGroqTarget(),
                                    createGroqMCRegisterInfo);

  // Register the MCSubtargetInfo.
  TargetRegistry::RegisterMCSubtargetInfo(getTheGroqTarget(),
                                          createGroqMCSubtargetInfo);

  // Register the MCAsmBackend.
  TargetRegistry::RegisterMCAsmBackend(getTheGroqTarget(),
                                       createGroqAsmBackend);

  // Register the asm target streamer.
  TargetRegistry::RegisterAsmTargetStreamer(getTheGroqTarget(),
                                            createGroqAsmTargetStreamer);

  // Register the ELF target streamer.
  TargetRegistry::RegisterObjectTargetStreamer(
      getTheGroqTarget(), createGroqObjectTargetStreamer);

  // Register the null target streamer.
  TargetRegistry::RegisterNullTargetStreamer(getTheGroqTarget(),
                                             createGroqNullTargetStreamer);
}
