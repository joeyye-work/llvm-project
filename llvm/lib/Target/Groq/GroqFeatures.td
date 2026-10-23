//===----------------------------------------------------------------------===//
// Groq subtarget features.
//===----------------------------------------------------------------------===//

// TSP architecture traits derived from public ISCA 2020 documentation.
// TODO(groq-spec): refine and split when official ISA/ABI descriptions are
// available for LLVM integration.
def FeatureTSPStreams : SubtargetFeature<"tsp-streams", "HasTSPStreams", "true",
                                         "Model stream-oriented TSP execution">;
def HasTSPStreams : Predicate<"Subtarget->hasTSPStreams()">,
                    AssemblerPredicate<(all_of FeatureTSPStreams)>;

def FeatureTSPSlices : SubtargetFeature<"tsp-slices", "HasTSPSlices", "true",
                                        "Model functional slice partitioning (ICU/MEM/VXM/MXM/SXM/C2C)">;
def HasTSPSlices : Predicate<"Subtarget->hasTSPSlices()">,
                   AssemblerPredicate<(all_of FeatureTSPSlices)>;

def FeatureTSPDeterministicTiming : SubtargetFeature<"tsp-deterministic-timing",
                                    "HasTSPDeterministicTiming", "true",
                                    "Model compiler-managed deterministic timing">;
def HasTSPDeterministicTiming
    : Predicate<"Subtarget->hasTSPDeterministicTiming()">,
      AssemblerPredicate<(all_of FeatureTSPDeterministicTiming)>;

// Groq ISA extensions (Groq Options).
def FeatureDensity : SubtargetFeature<"density", "HasDensity", "true",
                                      "Enable Density instructions">;
def HasDensity : Predicate<"Subtarget->hasDensity()">,
                 AssemblerPredicate<(all_of FeatureDensity)>;

def FeatureSingleFloat : SubtargetFeature<"fp", "HasSingleFloat", "true",
                                          "Enable Groq Single FP instructions">;
def HasSingleFloat : Predicate<"Subtarget->hasSingleFloat()">,
                     AssemblerPredicate<(all_of FeatureSingleFloat)>;

def FeatureWindowed : SubtargetFeature<"windowed", "HasWindowed", "true",
                                       "Enable Groq Windowed Register option">;
def HasWindowed : Predicate<"Subtarget->hasWindowed()">,
                  AssemblerPredicate<(all_of FeatureWindowed)>;

def FeatureMAC16 : SubtargetFeature<"mac16", "HasMAC16", "true",
                                    "Enable Groq MAC16 instructions">;
def HasMAC16 : Predicate<"Subtarget->hasMAC16()">,
               AssemblerPredicate<(all_of FeatureMAC16)>;

def FeatureBoolean          : SubtargetFeature<"bool", "HasBoolean", "true",
                                               "Enable Groq Boolean extension">;
def HasBoolean              : Predicate<"Subtarget->hasBoolean()">,
                                         AssemblerPredicate<(all_of FeatureBoolean)>;

def FeatureLoop : SubtargetFeature<"loop", "HasLoop", "true",
                                   "Enable Groq Loop extension">;
def HasLoop : Predicate<"Subtarget->hasLoop()">,
              AssemblerPredicate<(all_of FeatureLoop)>;

def FeatureSEXT : SubtargetFeature<"sext", "HasSEXT", "true",
                                   "Enable Groq Sign Extend option">;
def HasSEXT : Predicate<"Subtarget->hasSEXT()">,
              AssemblerPredicate<(all_of FeatureSEXT)>;

def FeatureCLAMPS : SubtargetFeature<"clamps", "HasCLAMPS", "true",
                                     "Enable Groq CLAMPS option">;
def HasCLAMPS : Predicate<"Subtarget->hasCLAMPS()">,
                AssemblerPredicate<(all_of FeatureCLAMPS)>;

def FeatureNSA : SubtargetFeature<"nsa", "HasNSA", "true",
                                  "Enable Groq NSA option">;
def HasNSA : Predicate<"Subtarget->hasNSA()">,
             AssemblerPredicate<(all_of FeatureNSA)>;

def FeatureMINMAX : SubtargetFeature<"minmax", "HasMINMAX", "true",
                                     "Enable Groq MINMAX option">;
def HasMINMAX : Predicate<"Subtarget->hasMINMAX()">,
                AssemblerPredicate<(all_of FeatureMINMAX)>;

def FeatureMul16 : SubtargetFeature<"mul16", "HasMul16", "true",
                                    "Enable Groq Mul16 option">;
def HasMul16 : Predicate<"Subtarget->hasMul16()">,
               AssemblerPredicate<(all_of FeatureMul16)>;

def FeatureMul32 : SubtargetFeature<"mul32", "HasMul32", "true",
                                    "Enable Groq Mul32 option">;
def HasMul32 : Predicate<"Subtarget->hasMul32()">,
               AssemblerPredicate<(all_of FeatureMul32)>;

def FeatureMul32High : SubtargetFeature<"mul32high", "HasMul32High", "true",
                                        "Enable Groq Mul32High option">;
def HasMul32High : Predicate<"Subtarget->hasMul32High()">,
                   AssemblerPredicate<(all_of FeatureMul32High)>;

def FeatureDiv32 : SubtargetFeature<"div32", "HasDiv32", "true",
                                    "Enable Groq Div32 option">;
def HasDiv32 : Predicate<"Subtarget->hasDiv32()">,
               AssemblerPredicate<(all_of FeatureDiv32)>;

def FeatureS32C1I : SubtargetFeature<"s32c1i", "HasS32C1I", "true",
                                     "Enable Groq S32C1I option">;
def HasS32C1I : Predicate<"Subtarget->hasS32C1I()">,
                AssemblerPredicate<(all_of FeatureS32C1I)>;

// Assume that lock-free native-width atomics are available, even if the target
// and operating system combination would not usually provide them. The user
// is responsible for providing any necessary __sync implementations. Code
// built with this feature is not ABI-compatible with code built without this
// feature, if atomic variables are exposed across the ABI boundary.
def FeatureForcedAtomics    : SubtargetFeature<"forced-atomics", "HasForcedAtomics", "true",
                                               "Assume that lock-free native-width atomics are available">;
def HasForcedAtomics        : Predicate<"Subtarget->hasForcedAtomics()">,
                                         AssemblerPredicate<(all_of FeatureForcedAtomics)>;
def HasAtomicLdSt           : Predicate<"Subtarget->hasS32C1I() || Subtarget->hasForcedAtomics()">;

def FeatureRegionProtection : SubtargetFeature<"regprotect", "HasRegionProtection", "true",
                                               "Enable Groq Region Protection option">;
def HasRegionProtection : Predicate<"Subtarget->hasRegionProtection()">,
                          AssemblerPredicate<(all_of FeatureRegionProtection)>;

def FeatureRelocatableVector : SubtargetFeature<"rvector", "HasRelocatableVector", "true",
                                                "Enable Groq Relocatable Vector option">;
def HasRelocatableVector : Predicate<"Subtarget->hasRelocatableVector()">,
                           AssemblerPredicate<(all_of FeatureRelocatableVector)>;

def FeatureMiscSR : SubtargetFeature<"miscsr", "HasMiscSR", "true",
                                     "Enable Groq Miscellaneous SR option">;
def HasMiscSR : Predicate<"Subtarget->hasMiscSR()">,
                AssemblerPredicate<(all_of FeatureMiscSR)>;

def FeatureExtendedL32R : SubtargetFeature<"extendedl32r", "HasExtendedL32R", "true",
                                           "Enable Groq Extended L32R option">;
def HasExtendedL32R : Predicate<"Subtarget->hasExtendedL32R()">,
                      AssemblerPredicate<(all_of FeatureExtendedL32R)>;

def FeatureDataCache : SubtargetFeature<"dcache", "HasDataCache", "true",
                                        "Enable Groq Data Cache option">;
def HasDataCache : Predicate<"Subtarget->hasDataCache()">,
                   AssemblerPredicate<(all_of FeatureDataCache)>;

def FeatureTHREADPTR : SubtargetFeature<"threadptr", "HasTHREADPTR", "true",
                                        "Enable Groq THREADPTR option">;
def HasTHREADPTR : Predicate<"Subtarget->hasTHREADPTR()">,
                   AssemblerPredicate<(all_of FeatureTHREADPTR)>;

// Groq Interrupts Options.
def FeatureHighPriInterrupts : SubtargetFeature<"highpriinterrupts",
                                                "HasHighPriInterrupts", "true",
                                                "Enable Groq HighPriInterrupts option">;
def HasHighPriInterrupts : Predicate<"Subtarget->hasHighPriInterrupts()">,
                                      AssemblerPredicate<(all_of FeatureHighPriInterrupts)>;

foreach i = {3-7} in
    def FeatureHighPriInterruptsLevel#i : SubtargetFeature<"highpriinterrupts-level"#i,
         "HasHighPriInterruptsLevel"#i#"", "true", "Enable Groq HighPriInterrupts Level"#i, [FeatureHighPriInterrupts]>;

def FeatureInterrupt : SubtargetFeature<"interrupt", "HasInterrupt", "true",
                                        "Enable Groq Interrupt option">;
def HasInterrupt : Predicate<"Subtarget->hasInterrupt()">,
                              AssemblerPredicate<(all_of FeatureInterrupt)>;

def FeatureException : SubtargetFeature<"exception", "HasException", "true",
                                        "Enable Groq Exception option">;
def HasException : Predicate<"Subtarget->hasException()">,
                              AssemblerPredicate<(all_of FeatureException)>;

def FeatureDebug : SubtargetFeature<"debug", "HasDebug", "true",
                                    "Enable Groq Debug option">;
def HasDebug : Predicate<"Subtarget->hasDebug()">,
                          AssemblerPredicate<(all_of FeatureDebug)>;

foreach i = {1-3} in
    def FeatureTimers#i : SubtargetFeature<"timers"#i,
         "HasTimers"#i#"", "true", "Enable Groq Timers "#i>;

def FeaturePRID : SubtargetFeature<"prid", "HasPRID", "true",
                                   "Enable Groq Processor ID option">;
def HasPRID : Predicate<"Subtarget->hasPRID()">,
                         AssemblerPredicate<(all_of FeaturePRID)>;

def FeatureCoprocessor : SubtargetFeature<"coprocessor", "HasCoprocessor", "true",
                                          "Enable Groq Coprocessor option">;
def HasCoprocessor : Predicate<"Subtarget->hasCoprocessor()">,
                                AssemblerPredicate<(all_of FeatureCoprocessor)>;

def FeatureDFPAccel : SubtargetFeature<"dfpaccel", "HasDFPAccel", "true",
                                       "Enable Groq Double Precision FP acceleration">;
def HasDFPAccel : Predicate<"Subtarget->hasDFPAccel()">,
                  AssemblerPredicate<(all_of FeatureDFPAccel)>;

def FeatureESP32S2Ops : SubtargetFeature<"esp32s2ops", "HasESP32S2Ops", "true",
                                         "Support Groq esp32-s2 ISA extension">;
def HasESP32S2Ops : Predicate<"Subtarget->hasESP32S2Ops()">,
                               AssemblerPredicate<(all_of FeatureESP32S2Ops)>;

def FeatureESP32S3Ops : SubtargetFeature<"esp32s3ops", "HasESP32S3Ops", "true",
                                         "Support Groq esp32-s3 ISA extension">;
def HasESP32S3Ops : Predicate<"Subtarget->hasESP32S3Ops()">,
                               AssemblerPredicate<(all_of FeatureESP32S3Ops)>;
