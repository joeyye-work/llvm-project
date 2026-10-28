# Groq/TSP IR Modeling (Incremental, IR-Only)

This document describes the current incremental LLVM IR milestone for
Groq/TSP modeling.

## Scope boundary

The modeled operation set is intentionally limited to the publicly described
instruction categories in Table I of:

- D. Abts et al., *Think Fast: A Tensor Streaming Processor (TSP) for
  Accelerating Deep Learning Workloads*, ISCA 2020.
  <http://pkamath.com/publications/papers/tsp-isca20.pdf>

No additional speculative instruction categories are introduced at this step.

## IR-only milestone

This milestone adds IR-level intrinsic declarations and verifier constraints
only. It intentionally does **not** add backend instruction definitions or
codegen work (including ISel, MC, asm printer/parser, disassembler, object
writer, or target lowering).

## Vector-length policy

For Groq/TSP intrinsics that carry vector stream operands/results, LLVM IR
verification currently requires fixed vector sizes that are:

- multiples of 16 bytes, and
- in the inclusive range 16..320 bytes.

Follow-up PRs can refine intrinsic semantics and add backend lowering when more
public architectural detail is available.
