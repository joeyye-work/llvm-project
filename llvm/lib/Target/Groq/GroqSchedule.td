//===- GroqSchedule.td - Groq TSP Scheduling Model -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Provisional TSP scheduling model seeded from public ISCA 2020 descriptions.
// TODO(groq-spec): Replace placeholder latencies/resources with official values.
//
//===----------------------------------------------------------------------===//

def GroqTSPModel : SchedMachineModel {
  // Provisional queue-level parallelism placeholder:
  // model up to 144 independent dispatch slots.
  let IssueWidth = 144;
  let MicroOpBufferSize = 0;
  let LoadLatency = 4;
  let MispredictPenalty = 0;
  let CompleteModel = 0;
}

// Public TSP materials describe functional slices rather than a scalar core
// pipeline. These resources are intentionally coarse placeholders.
def GroqIQ : ProcResource<144>;
def GroqICU : ProcResource<1>;
def GroqMEM : ProcResource<1>;
def GroqVXM : ProcResource<1>;
def GroqMXM : ProcResource<1>;
def GroqSXM : ProcResource<1>;
def GroqC2C : ProcResource<1>;

// Default scalar stand-in scheduling write used by the current backend.
def WriteGroqIQ : SchedWriteRes<[GroqIQ]> {
  let SchedModel = GroqTSPModel;
  let Latency = 1;
}
