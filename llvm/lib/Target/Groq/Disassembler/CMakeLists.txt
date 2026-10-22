add_llvm_component_library(LLVMGroqDisassembler
  GroqDisassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  Support
  GroqDesc
  GroqInfo

  ADD_TO_COMPONENT
  Groq
  )
