// 15-745 S14 Assignment 1: FunctionInfo.cpp
// Group: bovik, bovik2
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

#include <ostream>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

using namespace llvm;

namespace {

class BDI_split : public ModulePass {
private:
   std::vector<Instruction*> inserted_ins;

public:

  static char ID;

  BDI_split() : ModulePass(ID) { }

  ~BDI_split() { }

  // We don't modify the program, so we preserve all analyses
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
  }

  virtual bool runOnFunction(Function *F) {
    // TODO: implement this.
    for (Function::iterator FI = F->begin(), FE = F->end(); FI != FE; ++FI) {
      for (BasicBlock::iterator BI = FI->begin(), BE = FI->end(); BI != BE; ++BI) {
		if (isa<llvm::AllocaInst>(BI) ) {
		  AllocaInst* alloca_Ins = dyn_cast<AllocaInst>(BI);
		  Type* alloca_type = alloca_Ins->getAllocatedType();
		  if (alloca_type->isArrayTy() ) {
			Type* struct_type = alloca_type->getArrayElementType();
			if (struct_type->isStructTy() ) {
			  unsigned elementNum = struct_type->getStructNumElements();
			  std::cout << BI->getName().str() << std::endl;
			  for (int i = 0; i < elementNum; i++) {
				std::string field_name = BI->getName().str() + "_" + char(48+i);
				std::cout << field_name << std::endl;
				std::cout << alloca_type->getArrayNumElements() << std::endl;

				Type* array_type = ArrayType::get(struct_type->getStructElementType(i), alloca_type->getArrayNumElements() );

				AllocaInst* ai = new AllocaInst(array_type, 0, field_name.c_str(), BI);
				inserted_ins.push_back(ai);
				//BI->insertBefore(ai);
			  }
			  std::cout << "elements of struct: " << elementNum << std::endl;
			}
		  }
		}

	  }
	}

	for (int i = 0; i < inserted_ins.size(); i++)
	  delete inserted_ins[i];
    return false;
  }
  
  virtual bool runOnModule(Module& M) {
    std::cerr << "15745 Function Information Pass\n"; // TODO: remove this.
    for (Module::iterator MI = M.begin(), ME = M.end(); MI != ME; ++MI) {
      runOnFunction(MI);
    }
    // TODO: uncomment this.
    // printFunctionInfo(M);
    return false;
  }

};

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char BDI_split::ID = 0;
RegisterPass<BDI_split> X("split-stack", "15745: Function Information");

}
