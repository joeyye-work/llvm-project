add_llvm_component_library(LLVMGroqInfo
  GroqTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  Groq
  )
