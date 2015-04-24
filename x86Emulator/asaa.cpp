//===- AddressSpaceAliasAnalysis.cpp - Address Space Alias Analysis Impl --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an alias analysis pass that will return NoAlias for
// pointers in different address spaces. This is useful for some languages, e.g.
// OpenCL, where address spaces are used to differentiate between different
// types of memory, and aliasing cannot possibly occur.
//
//===----------------------------------------------------------------------===//

// This file is borrowed and recycled from a patch from Justin Holewinski that
// never made it to the main repository.
// http://lists.cs.uiuc.edu/pipermail/llvm-commits/Week-of-Mon-20111010/129632.html

#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/IR/GlobalAlias.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>

#include "asaa.h"

using namespace llvm;

namespace
{
	struct AddressSpaceAliasAnalysis : public ImmutablePass, public AliasAnalysis
	{
		static char ID;
		AddressSpaceAliasAnalysis() : ImmutablePass(ID) {
		}
		
		virtual bool doInitialization(Module& m) override
		{
			InitializeAliasAnalysis(this, &m.getDataLayout());
			return true;
		}
		
		virtual void getAnalysisUsage(AnalysisUsage &AU) const override
		{
			AliasAnalysis::getAnalysisUsage(AU);
		}
		
		virtual AliasResult alias(const Location &LocA, const Location &LocB) override
		{
			const Value *V1 = LocA.Ptr;
			const Value *V2 = LocB.Ptr;
			
			const PointerType *PT1 = dyn_cast<const PointerType>(V1->getType());
			const PointerType *PT2 = dyn_cast<const PointerType>(V2->getType());
			
			// The logic here is very simple: pointers to two different address spaces
			// cannot alias.
			if (PT1 != nullptr && PT2 != nullptr)
			{
				if (PT1->getAddressSpace() != PT2->getAddressSpace())
				{
					return NoAlias;
				}
			}
			
			return AliasAnalysis::alias(LocA, LocB);
		}
		
		virtual void *getAdjustedAnalysisPointer(AnalysisID PI) override
		{
			if (PI == &AliasAnalysis::ID)
				return (AliasAnalysis*)this;
			return this;
		}
	};
}

// Register this pass...
char AddressSpaceAliasAnalysis::ID = 0;

static RegisterPass<AddressSpaceAliasAnalysis> aasa("asaa", "NoAlias for pointers in different address spaces", false, true);
static RegisterAnalysisGroup<AliasAnalysis> aag(aasa);

ImmutablePass* createAddressSpaceAliasAnalysisPass() {
	return new AddressSpaceAliasAnalysis();
}