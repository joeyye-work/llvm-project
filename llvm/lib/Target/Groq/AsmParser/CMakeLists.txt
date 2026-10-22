add_llvm_component_library(LLVMGroqAsmParser
  GroqAsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  Support
  GroqDesc
  GroqInfo

  ADD_TO_COMPONENT
  Groq
  )
