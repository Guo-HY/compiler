#ifndef _REG_ALLOCATOR_H
#define _REG_ALLOCATOR_H 1
#include "asm_build.hpp"

void plainRegAllocator(AsmModule* module);
void allocVirtRegMem(AsmFunction* func, AsmInst* inst);
std::list<AsmInst*>::iterator
instRegVirt2phys(AsmFunction* func, AsmBasicBlock* blk, AsmInst* inst, std::list<AsmInst*>::iterator it);

#endif