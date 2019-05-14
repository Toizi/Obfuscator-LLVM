#define DEBUG_TYPE "cfg-indirect"
#include "llvm/Support/Debug.h"

#include "llvm/ADT/Statistic.h"
STATISTIC(IndirectCount, "Number of direct jumps transformed to indirect ones");
STATISTIC(SkippedCount, "Number of skipped jumps");

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/RandomNumberGenerator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <memory>
#include "llvm/Transforms/Obfuscation/CfgIndirection.h"

using namespace llvm;
static cl::opt<double> CfgIndirectRatio{
    "cfg-indirect-ratio",
    cl::desc(
        "Only apply the cfg indirection pass on <ratio> of the candidates"),
    cl::value_desc("ratio"), llvm::cl::init(1.0), llvm::cl::Optional};

namespace {
class CfgIndirection : public BasicBlockPass {
public:
  static char ID;
  bool is_enabled;
  std::unique_ptr<RandomNumberGenerator> rng;
  std::unique_ptr<std::uniform_real_distribution<double>> dist;

  CfgIndirection() : BasicBlockPass(ID) {}
  CfgIndirection(bool is_enabled) : BasicBlockPass(ID) { this->is_enabled = is_enabled; CfgIndirection(); }

  bool doInitialization(Module &M) override {
    rng = M.createRNG(this);
    dist.reset(new std::uniform_real_distribution<double>(0.0, 1.0));

    return false;
  }

  bool runOnBasicBlock(BasicBlock &BB) override {
    if (!is_enabled)
        return false;
    bool modified = false;
    TerminatorInst *t = BB.getTerminator();
    SmallVector<std::pair<BasicBlock *, Value *>, 2> destinations;
    if (auto *branch = dyn_cast<BranchInst>(t)) {

      if ((*dist)(*rng) > CfgIndirectRatio.getValue()) {
        ++SkippedCount;
        return modified;
      }

      IndirectBrInst *inst = nullptr;
      bool is_conditional = branch->isConditional();
      IRBuilder<> builder{branch};

      for (int i = 0; i < (int)(branch->getNumSuccessors()); ++i) {
        auto *suc = branch->getSuccessor(i);

        // sanity check. input must not be in SSA form
        if (isa<PHINode>(suc->front())) {
          errs() << "error: CfgIndirection not possible since input contains "
                    "phi nodes.\n"
                    "Convert the input to non-SSA form using reg2mem\n";
          exit(1);
        }

        Value *destination = BlockAddress::get(suc);
        destinations.push_back(std::make_pair(suc, destination));
      }

      // final destination
      Value *dest_ptr = nullptr;
      // dest_ptr = cond ? dest0 : dest1
      if (is_conditional) {
        Value *condition = branch->getCondition();
        dest_ptr = builder.CreateSelect(condition, destinations[0].second,
                                        destinations[1].second);
      }
      // unconditional
      else {
        dest_ptr = destinations[0].second;
      }
      inst = IndirectBrInst::Create(dest_ptr, 1 + (is_conditional ? 1 : 0));
      for (auto &destination : destinations) {
        inst->addDestination(destination.first);
      }
      ReplaceInstWithInst(branch, inst);

      // bookkeeping
      ++IndirectCount;
      modified = true;
    }

    return modified;
  }
};
} // namespace


char CfgIndirection::ID = 0;
static RegisterPass<CfgIndirection> X("cfg-indirection", "control flow indirection");

Pass *llvm::createCfgIndirect() {
  return new CfgIndirection();
}

Pass *llvm::createCfgIndirect(bool is_enabled) {
  return new CfgIndirection(is_enabled);
}