add_llvm_component_group(Groq)

set(LLVM_TARGET_DEFINITIONS Groq.td)

tablegen(LLVM GroqGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM GroqGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM GroqGenCallingConv.inc -gen-callingconv)
tablegen(LLVM GroqGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM GroqGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM GroqGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM GroqGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM GroqGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM GroqGenSDNodeInfo.inc -gen-sd-node-info)
tablegen(LLVM GroqGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(GroqCommonTableGen)

add_llvm_target(GroqCodeGen
  GroqAsmPrinter.cpp
  GroqConstantPoolValue.cpp
  GroqFrameLowering.cpp
  GroqInstrInfo.cpp
  GroqISelDAGToDAG.cpp
  GroqISelLowering.cpp
  GroqRegisterInfo.cpp
  GroqSelectionDAGInfo.cpp
  GroqSubtarget.cpp
  GroqTargetMachine.cpp

  LINK_COMPONENTS
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  MC
  SelectionDAG
  Support
  Target
  TargetParser
  GroqDesc
  GroqInfo

  ADD_TO_COMPONENT
  Groq
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)

