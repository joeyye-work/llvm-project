add_llvm_component_library(LLVMGroqDesc
  GroqAsmBackend.cpp
  GroqELFObjectWriter.cpp
  GroqInstPrinter.cpp
  GroqMCAsmInfo.cpp
  GroqMCCodeEmitter.cpp
  GroqMCTargetDesc.cpp
  GroqTargetStreamer.cpp

  LINK_COMPONENTS
  MC
  Support
  GroqInfo

  ADD_TO_COMPONENT
  Groq
 )
