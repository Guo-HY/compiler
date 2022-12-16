#ifndef _ASM_BUILD_H
#define _ASM_BUILD_H 1

#include "asm.hpp"
#include "../ir/ir.hpp"
#include "../ir/ir_build.hpp"
#include "../ir/ir_utils.hpp"
#include <unordered_map>
#include <initializer_list>
#include <utility>

void initName2PhysAsmReg();
int getTypeWordSize(Type* t);
int allocVirtAsmId();
AsmReg* allocVirtAsmReg();
AsmLabel* allocAsmLabel(std::string name);
AsmImm* allocAsmImm(int imm);
void blockAddInst(AsmInstIdtfr idtfr, std::initializer_list<std::pair<AsmOperand*, RWP>> ops);
std::string getGlobalValueName(Value* v);
bool isFuncFArg(Value* v);
AsmImm* allocfuncArgOffset(Value* v);

AsmReg* value2asmReg(Value* value);
void globalInitValue2asmData(GlobalInitValue* globalInitValue, std::vector<int>* datas);
AsmGlobalData* globalValue2asm(GlobalValue* globalValue);
AsmModule* module2asm(Module* lmodule);
int funcCallArgsWordSize(CallInst* callInst);
AsmFunction* function2asm(Function* function);
void binaryInst2asm( BinaryInst* binaryInst);
AsmReg* loadInst2asm( LoadInst* loadInst);
void storeInst2asm(StoreInst* storeInst);
AsmReg* icmpInst2asm( IcmpInst* icmpInst);
void brInst2asm( BrInst* brInst);
void gepInst2asm( GEPInst* gepInst);
void callInst2asm( CallInst* callInst);
void retInst2asm( RetInst* retInst);
void zextInst2asm(ZextInst* inst);
void phiInst2asm(PhiInst* inst);

#endif