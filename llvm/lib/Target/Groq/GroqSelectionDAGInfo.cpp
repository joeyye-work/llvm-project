//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "GroqSelectionDAGInfo.h"

#define GET_SDNODE_DESC
#include "GroqGenSDNodeInfo.inc"

using namespace llvm;

GroqSelectionDAGInfo::GroqSelectionDAGInfo()
    : SelectionDAGGenTargetInfo(GroqGenSDNodeInfo) {}

GroqSelectionDAGInfo::~GroqSelectionDAGInfo() = default;
