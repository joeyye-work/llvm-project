===============================
Groq Target (Provisional MVP)
===============================

.. contents::
   :local:

Overview
========

This document describes the initial LLVM ``Groq`` backend scaffold.
The first-generation Groq Tensor Streaming Processor (TSP) is a highly
specialized streaming architecture whose public ISA/ABI details are incomplete.
The current backend is therefore intentionally provisional.

Sources digested for this revision
==================================

* ISCA 2020 paper: ``Think Fast: A Tensor Streaming Processor (TSP) for
  Accelerating Deep Learning Workloads`` by Abts et al.
* Reference text mirror of the paper used in this environment:
  ``https://raw.githubusercontent.com/bojieli/ai-infra-book/main/references/text/groq-tsp.txt``

Paper-grounded architecture constraints (verified)
==================================================

The publicly available paper describes the following high-level behavior:

* Functionally sliced architecture (ICU/MEM/VXM/MXM/SXM/C2C) instead of
  conventional per-core pipelines.
* Streaming producer-consumer execution model; operands/results move on streams
  across slices.
* 64 logical streams per lane (32 each direction), with 16-byte min vector
  granularity and 320-byte max vector length via 20 tiled stages.
* 220 MiB on-chip SRAM with very high bandwidth; explicit compiler-managed
  movement and scheduling.
* Compiler-scheduled timing model using explicit delay/synchronization
  instructions (e.g., NOP, Sync/Notify, Ifetch) and spatial/temporal stream
  placement.
* Public ISA semantics are described at slice level (Table I in the paper), but
  complete binary encoding details and full ABI/object format specifications are
  not published.

Backend implications
====================

Compared to a conventional LLVM CPU backend, a production-accurate Groq
backend must eventually model:

* stream-centric architectural state rather than scalar GPR-centric state,
* explicit timing/skew-aware scheduling constraints,
* explicit SRAM placement and movement primitives,
* accelerator-style program model rather than a standard C ABI stack model.

Provisional assumptions in this MVP backend
===========================================

To keep this target buildable and testable in-tree, the current backend uses a
scalar stand-in model with explicit TODO markers:

* Register model assumptions

  * A conventional integer register class is used for bring-up.
  * TODO(groq-spec): replace with stream/SRAM-centric architectural modeling.

* Instruction model assumptions

  * Integer ALU, branch, load/store, and return instructions are modeled for
    minimal SelectionDAG codegen tests.
  * TODO(groq-spec): map to official Groq slice-level ISA encoding details when
    available.

* Calling convention assumptions

  * Conventional argument/return handling is used for LLVM smoke tests.
  * TODO(groq-spec): replace with Groq-appropriate program/memory interface.

* Scheduling assumptions

  * Current scheduling is conservative and generic.
  * ``tsp-v1`` now uses a provisional 144-queue scheduler placeholder:

    * ``IssueWidth = 144`` in ``GroqTSPModel``.
    * all currently modeled instructions use a default ``WriteGroqIQ`` write
      resource backed by ``ProcResource<144>``.

  * This enables machine-scheduler experimentation with wide independent
    instruction issue in bring-up tests, but does **not** yet model:

    * per-queue addressing/selection,
    * queue-to-slice affinity/constraints,
    * queue-local timing/skew semantics.

  * TODO(groq-spec): represent timing/skew constraints implied by stream
    placement and slice transit latency.
  * The backend exposes provisional ``tsp-v1`` feature hooks
    (``tsp-streams``, ``tsp-slices``, ``tsp-deterministic-timing``) and a
    coarse slice-oriented scheduling model for machine-scheduler plumbing.

Known gaps requiring additional public or partner documentation
===============================================================

* Official opcode encodings and field layouts.
* Exact instruction timing metadata and operand skew semantics.
* Production ABI/calling convention and relocation/object-file specification.
* Full assembler/disassembler parity with Groq production tools.
* Complete vector/tensor/matrix operation lowering.

Developer notes and current limitations
=======================================

The current target should be treated as an MVP bring-up scaffold only.

* Unsupported/incomplete:

  * production-accurate stream register model,
  * production-accurate Groq ELF/relocation definitions,
  * complete architectural scheduling model,
  * complete tensor/matrix lowering pipeline.

* Implemented for bring-up:

  * target registration and build integration,
  * triple parsing and data-layout plumbing,
  * minimal SelectionDAG/asm emission flow,
  * focused ``llc -mtriple=groq-unknown-unknown`` smoke tests.
