===============================
Groq Target (Provisional MVP)
===============================

.. contents::
   :local:

Overview
========

This document describes the initial LLVM ``Groq`` backend scaffold.
Public information about the Groq Lattice Processing Unit (LPU) / Tensor
Streaming Processor (TSP) is limited; complete ISA encodings
and a C-style ABI are not publicly available. The current backend is therefore
a provisional buildable scaffold intended to unblock bring-up, testing, and
integration.

Architecture facts and assumptions
==================================

Verified public facts (high level)
----------------------------------

* Groq's first-generation Tensor Streaming Processor (TSP) is a spatial,
  compiler-scheduled architecture with no OoO scheduling or branch prediction.
* Public material describes slice-level execution units (ICU/MEM/VXM/MXM/SXM)
  and stream-oriented execution rather than a conventional scalar CPU model.
* Public docs do not provide a complete, open binary ISA encoding and ABI spec.

Provisional backend assumptions (MVP)
-------------------------------------

To make an initial LLVM backend buildable, this target currently models a
minimal scalar pseudo-ISA in the style of a conventional LLVM backend:

* Register model assumptions

  * ``GPR`` integer register class with fixed-width scalar registers.
  * Conventional reserved stack/frame/link registers are modeled.
  * TODO: replace with stream/SRAM-centric architectural model once official
    Groq ISA/ABI details are available.

* Instruction class and encoding assumptions

  * Integer ALU ops, moves, loads/stores, branches, and return are modeled.
  * Fixed-width instruction encoding is assumed for MC bring-up.
  * TODO: map to official Groq encoding fields (including timing/skew metadata)
    when public or partner documentation permits.

* Calling convention assumptions

  * Initial ABI uses a conventional register-return/register-argument style to
    enable simple IR lowering and smoke tests.
  * TODO: replace with Groq-accurate ABI/memory interface model.

* Pipeline/scheduling assumptions

  * Scheduler model is intentionally conservative and simple for early bring-up.
  * TODO: represent Groq's cycle-exact scheduling and stream-distance hazards
    with target-specific scheduling and hazard logic.

Known gaps requiring non-public / proprietary specs
===================================================

* Official opcode encodings and field layouts.
* Exact instruction timing model and stream skew semantics.
* Production ABI/calling convention and binary format details.
* Complete assembler/disassembler behavior for production toolchain parity.
* Target-specific vector/tensor/matrix operations and lowering paths.

Developer notes and limitations
===============================

The current target should be treated as a bring-up scaffold only:

* Unsupported/incomplete:

  * Full Groq tensor/vector ISA coverage.
  * Accurate Groq ABI and relocation/object semantics.
  * Complete asm parser/disassembler fidelity.
  * End-to-end performance scheduling parity with Groq toolchain.

* Implemented for MVP:

  * LLVM target registration and build integration.
  * Basic SelectionDAG pipeline and asm emission for simple integer codegen
    smoke tests.
