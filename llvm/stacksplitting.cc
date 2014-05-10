// 15-745 S14 Assignment 1: FunctionInfo.cpp
// Group: bovik, bovik2
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/User.h"

#include <ostream>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>

using namespace llvm;

namespace {

  class BDI_split : public ModulePass {
	private:
	  std::map<Value*, std::vector<Value*> > struct_field_map; //tracks old and new allocas
	  std::map<Value*, Value*> index_map;
	  std::map<Value*, Value*> root_map;
	  std::map<Value*, Value*> leaf_map;

	public:

	  static char ID;

	  BDI_split() : ModulePass(ID) { }

	  ~BDI_split() { }

	  // We don't modify the program, so we preserve all analyses
	  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
		AU.setPreservesAll();
	  }

	  virtual bool runOnFunction(Function *F) {
		for (Function::iterator FI = F->begin(), FE = F->end(); FI != FE; ++FI) {
		  for (BasicBlock::iterator BI = FI->begin(), BE = FI->end(); BI != BE; ++BI) {
			if (isa<llvm::AllocaInst>(BI) ) {
			  AllocaInst* alloca_ins = dyn_cast<AllocaInst>(BI);
			  Type* alloca_type = alloca_ins->getAllocatedType();
			  if (alloca_type->isArrayTy() ) { //Has to be of array type. TODO - modularize
				Type* struct_type = alloca_type->getArrayElementType();
				if (struct_type->isStructTy() ) {
				  unsigned elementNum = struct_type->getStructNumElements();
				  std::cout << BI->getName().str() << std::endl;
				  for (int i = 0; i < elementNum; i++) {
					std::string field_name = BI->getName().str() + "_" + char(48+i); //convert number to ascii
					std::cout << field_name << std::endl;
					std::cout << alloca_type->getArrayNumElements() << std::endl;

					Type* array_type =
					  ArrayType::get(struct_type->getStructElementType(i),
						  alloca_type->getArrayNumElements() );

					Instruction* ins_ptr = BI;

					AllocaInst* ai = new AllocaInst(array_type, 0, alloca_ins->getAlignment(),
						field_name.c_str(), ins_ptr); //Insert before the old alloca
					struct_field_map[dyn_cast<Value>(BI)].push_back(dyn_cast<Value>(ai) ); //add instruction and new allocas into the map.
					root_map[BI] = BI; //tracks all the root nodes.
				  }
				}
			  }
			}

			if (isa<llvm::GetElementPtrInst>(BI) ) {
			  std::cout << "found  a getelementptrInst" << std::endl;
			  GetElementPtrInst* getele_inst = dyn_cast<GetElementPtrInst>(BI);
			  Value* ptr_op = getele_inst->getPointerOperand(); //get the root operand 

			  if (root_map.find(ptr_op) != root_map.end() ) { //check if it references the root node.
				std::cout << "Dest: " << BI->getName().str() << std::endl;

				root_map[BI] = root_map[ptr_op];

				std::vector<Value*> index_vec;
				for (User::op_iterator idx_iter = getele_inst->idx_begin(); idx_iter != getele_inst->idx_end(); idx_iter++) { //get push all the operands in the getelementptr inst.
				  Value* index_ptr = idx_iter->get();
				  index_vec.push_back(index_ptr);
				}

				ArrayRef<Value*> indexArr(index_vec);

				/* Assuming we have at most 1 level of struct which has only primitive types and this is the leaf node
				 * then, we should use the last index to find new ptr and use the first index as the new index
				 */
				if(!GetElementPtrInst::getIndexedType(getele_inst->getPointerOperandType(), indexArr)->isAggregateType() ) {
				  // If I understand it correctly, it can only be a leaf when it has 2 indices.
				  assert(index_vec.size() == 2);
				  int field_idx = dyn_cast<ConstantInt>(index_vec.back() )->getZExtValue();
				  Value* field_ptr = struct_field_map[root_map[ptr_op]][field_idx];
				  GetElementPtrInst* new_getele_inst;
				  // If ptr_op is not an intermediate pointer, we can just use the first index as the new index
				  if (index_map.find(ptr_op)==index_map.end() ) {
					std::cout << "***REPLACING0: " << BI->getName().str() << std::endl;
					std::vector<Value*> temp;
					temp.push_back(ConstantInt::get(Type::getInt32Ty(index_vec[0]->getContext() ), 0));
					temp.push_back(index_vec[0]);
					new_getele_inst = GetElementPtrInst::Create(field_ptr, ArrayRef<Value*>(temp), BI->getName().str() + "_new");
				  }
				  // Otherwise, we gotta see if we need to inject another addition inst
				  else {
					Value* new_index = index_map[ptr_op];
					if (isa<ConstantInt>(index_vec[0]) && dyn_cast<ConstantInt>(index_vec[0])->isZero() ) {
					  std::cout << "***REPLACING1: " << BI->getName().str() << std::endl;
					  std::vector<Value*> temp;
					  temp.push_back(ConstantInt::get(Type::getInt32Ty(index_vec[0]->getContext() ), 0));
					  temp.push_back(new_index);
					  new_getele_inst = GetElementPtrInst::Create(field_ptr, ArrayRef<Value*>(temp), BI->getName().str() + "_new");
					  std::cout << "***WITH " << new_getele_inst->getName().str() << std::endl;
					}
					// Now, we got to inject the new add instruction as we did before!
					else {
					  std::cout << "***REPLACING2: " << BI->getName().str() << std::endl;
					  new_index = BinaryOperator::Create(Instruction::Add, new_index, index_vec[0], NULL, BI);
					  std::vector<Value*> temp;
					  temp.push_back(ConstantInt::get(Type::getInt32Ty(index_vec[0]->getContext() ), 0));
					  temp.push_back(new_index);
					  new_getele_inst = GetElementPtrInst::Create(field_ptr, ArrayRef<Value*>(temp), BI->getName().str() + "_new");
					}
					leaf_map[BI] = new_getele_inst;
				  }
				}
				/* Otherwise, it has only one index and it's just getting an intermediate pointer.
				 * In such case, we should just replace the getelementptr inst with new arithmatic inst
				 */
				else {
				  Value* new_index;
				  // Use the second index if it is an array type
				  // Add, I have no idea what happens with multi-dimension arrays. I don't care.
				  // If anyone wanna use this again, you gotta have a tree storing at least at each level what type the node is.
				  if (GetElementPtrInst::getIndexedType(getele_inst->getPointerOperandType(), ArrayRef<Value*>(index_vec[0]) )->isArrayTy() ) {
					assert(index_vec.size() == 2);
					new_index = BinaryOperator::Create(Instruction::Add, ConstantInt::get(Type::getInt32Ty(index_vec[0]->getContext() ), 0), index_vec[1], BI->getName().str() + "_new", BI);
				  }
				  else {
					assert(index_vec.size() == 1);
					new_index = BinaryOperator::Create(Instruction::Add, index_map[ptr_op], index_vec[0], BI->getName().str() + "_new", BI);
				  }
				  index_map[BI] = new_index;
				}

				/*
				   Type * index_type = GetElementPtrInst::getIndexedType(getele_inst->getPointerOperandType(),indexArr);

				   std::vector<Value*> temp;

				   if(index_map.find(getele_inst->getPointerOperand()) != index_map.end() )
				   temp = index_map[getele_inst->getPointerOperand()];

				   unsigned start_in = 0;
				   if(GetElementPtrInst::getIndexedType(getele_inst->getPointerOperandType(), ArrayRef<Value*>(index_vec[0]) )->isAggregateType() ) {
				   start_in = 1;
				   std::cout << "Yeah!!!" << std::endl;
				   }
				   for(unsigned in = start_in; in < index_vec.size(); in++)
				   temp.push_back(index_vec[in]);

				   if(index_type->isAggregateType()) // Not a leaf
				   index_map[dyn_cast<Value>(BI)] = temp;
				   else { // Is a leaf node
				   assert(isa<ConstantInt>( temp.back() ) );
				   int field_idx = dyn_cast<ConstantInt>(temp.back() )->getZExtValue();

				   std::cout << "root_ptr:" << root_map[ptr_op]->getName().str() << "\tfield_idx: " << field_idx << std::endl;
				   std::cout << "number of indices: " << temp.size() << std::endl;
				   for (int i = 0; i < temp.size(); i++) {
				   if (isa<ConstantInt>(temp[i]) )
				   std::cout << dyn_cast<ConstantInt>(temp[i])->getZExtValue() << " ";
				   else
				   std::cout << temp[i]->getName().str() << " ";
				   }

				   std::cout << std::endl;

				   Value* new_ptr = struct_field_map[root_map[ptr_op]][field_idx];
				   temp.pop_back();
				   ArrayRef<Value*> indexArr_new(temp);

				   GetElementPtrInst* new_getele_inst = GetElementPtrInst::Create(new_ptr, indexArr_new, BI->getName().str() + "_new", BI);
				   }

				 */

				//  Just testing if indices are correctly assigned.
				for (ArrayRef<Value*>::iterator arr_iter = indexArr.begin(); arr_iter != indexArr.end(); arr_iter++) {
				  Value* index_ptr = *arr_iter;

				  if (isa<ConstantInt>(index_ptr) )
					std::cout << dyn_cast<ConstantInt>(index_ptr)->getZExtValue() << std::endl;
				  else
					std::cout << index_ptr->getName().str() << std::endl;

				  if (index_ptr->getType() == NULL)
					std::cout << "CRAPPPPP" << std::endl;
				}

				//GetElementPtrInst* new_getele_inst = GetElementPtrInst::Create(ptr_op, testArr, BI->getName().str() + "_new", ins_ptr);

				std::cout << "Found the operand!!" << std::endl;
				std::cout << "Has indices: " << getele_inst->getNumIndices() << std::endl;
			  }

			}
		  }
		}

		std::cout << "index_map size: \"" << index_map.size() << std::endl;

		for (std::map<Value*, Value*>::iterator map_iter = leaf_map.begin(); map_iter != leaf_map.end(); map_iter++) {
		  std::cout << "deleting instruction: \"" << map_iter->first->getName().str() << std::endl;
		  BasicBlock::iterator BI (dyn_cast<Instruction>(map_iter->first) );
		  ReplaceInstWithInst(BI->getParent()->getInstList(), BI, dyn_cast<Instruction>(map_iter->second) );
		}
		leaf_map.clear();

		while (index_map.size() != 0) {
		  for (std::map<Value*, Value*>::iterator map_iter = index_map.begin(); map_iter != index_map.end(); map_iter++) {
			assert(isa<GetElementPtrInst>(map_iter->first) );
			bool no_use = true;

			std::cout << "index_map.size(): " << index_map.size() << std::endl;
			if (index_map.size() > 1) {
			  for (std::map<Value*, Value*>::iterator inner_iter = index_map.begin(); inner_iter != index_map.end(); inner_iter++) {
				assert(isa<GetElementPtrInst>(inner_iter->first) );
				if(inner_iter->first == map_iter->first)
				  continue;

				if (dyn_cast<GetElementPtrInst>(inner_iter->first)->getPointerOperand() == map_iter->first) {
				  no_use = false;
				  break;
				}
			  }
			}

			if (no_use) {
			  std::cout << "deleting instruction: " << map_iter->first->getName().str() << std::endl;
			  dyn_cast<Instruction>(map_iter->first)->eraseFromParent();
			  index_map.erase(map_iter);
			  map_iter = index_map.begin();
			}
			if (index_map.empty() )
			  break;
		  }
		}

		for (std::map<Value*, std::vector<Value*> >::iterator map_iter = struct_field_map.begin(); map_iter != struct_field_map.end(); map_iter++) {
		  std::cout << "deleting instruction: " << map_iter->first->getName().str() << std::endl;
		  dyn_cast<Instruction>(map_iter->first)->eraseFromParent();
		}
		struct_field_map.clear();

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
