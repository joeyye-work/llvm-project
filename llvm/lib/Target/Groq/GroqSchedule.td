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
  let IssueWidth = 1;
  let MicroOpBufferSize = 0;
  let LoadLatency = 4;
  let MispredictPenalty = 0;
  let CompleteModel = 0;
}

// Public TSP materials describe functional slices rather than a scalar core
// pipeline. These resources are intentionally coarse placeholders.
def GroqICU : ProcResource<1>;
def GroqMEM : ProcResource<1>;
def GroqVXM : ProcResource<1>;
def GroqMXM : ProcResource<1>;
def GroqSXM : ProcResource<1>;
def GroqC2C : ProcResource<1>;
