// 15-745 S14 Assignment 1: FunctionInfo.cpp
// Group: bovik, bovik2
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/User.h"

#include <ostream>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>

using namespace llvm;

namespace {

class BDI_split : public ModulePass {
private:
  std::map<Value*, std::vector<Value*> > struct_field_map;
  std::map<Value*, std::vector<Value*> > index_map;

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
		  AllocaInst* alloca_ins = dyn_cast<AllocaInst>(BI);
		  Type* alloca_type = alloca_ins->getAllocatedType();
		  if (alloca_type->isArrayTy() ) {
			Type* struct_type = alloca_type->getArrayElementType();
			if (struct_type->isStructTy() ) {
			  unsigned elementNum = struct_type->getStructNumElements();
			  std::cout << BI->getName().str() << std::endl;
			  for (int i = 0; i < elementNum; i++) {
				std::string field_name = BI->getName().str() + "_" + char(48+i);
				std::cout << field_name << std::endl;
				std::cout << alloca_type->getArrayNumElements() << std::endl;

				Type* array_type =
				  ArrayType::get(struct_type->getStructElementType(i),
					  alloca_type->getArrayNumElements() );

				Instruction* ins_ptr = BI;

				AllocaInst* ai = new AllocaInst(array_type, 0,
					field_name.c_str(), ins_ptr);
				struct_field_map[dyn_cast<Value>(BI)].push_back(dyn_cast<Value>(ai) );
			  }
			}
		  }
		}

		if (isa<llvm::GetElementPtrInst>(BI) ) {
		  std::cout << "find a getelementptrInst" << std::endl;
		  GetElementPtrInst* getele_inst = dyn_cast<GetElementPtrInst>(BI);
		  Value* ptr_op = getele_inst->getPointerOperand();

		  if (struct_field_map.find(ptr_op) != struct_field_map.end() ) {
			std::vector<Value*> index_vec;
			for (User::op_iterator idx_iter = getele_inst->idx_begin(); idx_iter != getele_inst->idx_end(); idx_iter++) {
			  Value* index_ptr = idx_iter->get();
/*			  if (isa<ConstantInt>(index_ptr) )
				std::cout << dyn_cast<ConstantInt>(index_ptr)->getZExtValue() << std::endl;
			  else
				std::cout << index_ptr->getName().str() << std::endl;
*/
			  index_vec.push_back(index_ptr);
			}

			ArrayRef<Value*> testArr(index_vec);
/* 
			 Type * index_type = GetElementPtrInst::getIndexedType(getele_inst->getPointerOperandType(),testArr);
			 if(index_type->isAggregateType()) {
			   if(index_map.find(getele_inst->getPointerOperand()) != index_map.end() ) {
				std::vector<Value*> temp = index_map[getele_inst->getPointerOperand()];
			   unsigned start_in = 0;
			   if(isa<ConstantInt>(index_vec[0]))
				if(dyn_cast<ConstantInt>(index_vec[0])->getZExtValue() == 0) 
				  start_in = 1;
			   for(unsigned in = start_in; in < index_vec.size(); in++) {
				temp.push_back(index_vec[in]);
			   }
			   index_map[dyn_cast<Value>(BI)] = temp;
				}
			 }
			 else {
			   //Get index vector from index_map[getele_ins->getPointerOperand()]
			   //concatenate indices and create new getelementptr instruction
			 }
*/

//  Just testing if indices are correctly assigned.
			for (ArrayRef<Value*>::iterator arr_iter = testArr.begin(); arr_iter != testArr.end(); arr_iter++) {
			  Value* index_ptr = *arr_iter;

			  if (isa<ConstantInt>(index_ptr) )
				std::cout << dyn_cast<ConstantInt>(index_ptr)->getZExtValue() << std::endl;
			  else
				std::cout << index_ptr->getName().str() << std::endl;

			  if (index_ptr->getType() == NULL)
				std::cout << "CRAPPPPP" << std::endl;
			}
//
			Instruction* ins_ptr = BI;

			//GetElementPtrInst* new_getele_inst = GetElementPtrInst::Create(ptr_op, testArr, BI->getName().str() + "_new", ins_ptr);
			
			std::cout << "Found the operand!!" << std::endl;
			std::cout << "Has indices: " << getele_inst->getNumIndices() << std::endl;
		  }

		}
	  }
	}
	for (std::map<Value*, std::vector<Value*> >::iterator
	  map_iter = struct_field_map.begin(); map_iter !=
	  struct_field_map.end(); map_iter++) {
	  std::cout << "elements of struct \"" << map_iter->first->getName().str()
		<< "\": " << map_iter->second.size() << std::endl;
    }

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
