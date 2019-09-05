//===-- Obfuscation.h - Obfuscation Transformations -----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes for accessor functions that expose passes
// in the Obfuscation transformations library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_OBFUSCATION_H
#define LLVM_TRANSFORMS_OBFUSCATION_H

#include <functional>

namespace llvm {

class BasicBlockPass;
class Function;
class FunctionPass;
class ModulePass;
class Pass;
class GetElementPtrInst;
class PassInfo;
class TerminatorInst;
class TargetLowering;
class TargetMachine;


Pass *createCfgIndirectPass();
Pass *createBogusPass();
Pass *createFlatteningPass();
Pass *createSplitBasicBlockPass();
Pass *createStringObfuscationPass();
Pass *createSubstitutionPass();
} // End llvm namespace

#endif
