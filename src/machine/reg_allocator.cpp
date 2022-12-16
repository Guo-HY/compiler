#include "reg_allocator.hpp"

extern std::unordered_map<std::string, int> name2regPhysNum;
extern std::unordered_map<std::string, AsmReg*> name2PhysAsmReg;


/* 分配寄存器仅使用k0,k1,v1 虚拟寄存器全部在内存中分配空间，
  运算完成后直接存入内存 */
int nowSpOffsetWord;
std::list<AsmReg*> freeRegPool; 
void plainRegAllocator(AsmModule* module)
{
  freeRegPool.push_back(name2PhysAsmReg["k0"]);
  freeRegPool.push_back(name2PhysAsmReg["k1"]);
  freeRegPool.push_back(name2PhysAsmReg["v1"]);

  for (u_long i = 0; i < module->functions.size(); i++) {
    AsmFunction* func = module->functions[i];
    nowSpOffsetWord = func->stackWordSize;
    for (u_long i = 0; i < func->basicBlocks.size(); i++) {
      AsmBasicBlock* blk = func->basicBlocks[i];
      std::list<AsmInst*>::iterator it = blk->insts.begin();
      while (it != blk->insts.end()) {
        AsmInst* inst = *it;
        if (inst->asmInstIdtfr == AsmInstIdtfr::PHI_AII) {
          /* TODO */
          it++;
          continue;
        }
        allocVirtRegMem(func, inst);
        it = instRegVirt2phys(func, blk, inst, it);
      }
    }
    int addWordSize = nowSpOffsetWord - func->stackWordSize;
    func->stackWordSize = nowSpOffsetWord;
    func->frameSize.first->immediate = nowSpOffsetWord * -4;
    func->frameSize.second->immediate = nowSpOffsetWord * 4;
    std::unordered_map<int, AsmImm*>::iterator iter;
    for (iter = func->funcArgsId2stackOffset.begin();
       iter != func->funcArgsId2stackOffset.end(); iter++) {
      iter->second->immediate += addWordSize * 4;
    }
  }
}

/* 给尚未在栈中分配空间的虚拟寄存器分配空间 */
void allocVirtRegMem(AsmFunction* func, AsmInst* inst)
{
  for (u_long i = 0; i < inst->ops.size(); i++) {
    if (inst->ops[i].first->asmOperandIdtfr == AsmOperandIdtfr::REG_AOI) {
      AsmReg* reg = (AsmReg*)(inst->ops[i].first);
      if (reg->isPhysReg) {
        continue;
      }
      if (func->virtRegId2stackOffset.count(reg->virtNumber) == 0) {
        func->virtRegId2stackOffset[reg->virtNumber] = allocAsmImm(nowSpOffsetWord * 4);
        nowSpOffsetWord++;
      }
    }
  }
}

/* 将指令中的虚拟寄存器替换为物理寄存器并生成load/store指令 */
/* 如果是读寄存器，则指令执行前load出来，如果是写寄存器，则指令执行完后store */
std::list<AsmInst*>::iterator
instRegVirt2phys(AsmFunction* func, AsmBasicBlock* blk, AsmInst* inst, std::list<AsmInst*>::iterator it)
{
  if (inst->asmInstIdtfr == AsmInstIdtfr::PHI_AII) {
    panic("error");
  }
  AsmInst* newInst;
  std::list<AsmReg*> usdRegPool; 
  AsmReg* writePhyReg = NULL;
  AsmImm* writeStoreAddr = NULL;
  for (u_long i = 0; i < inst->ops.size(); i++) {
    if (inst->ops[i].first->asmOperandIdtfr != AsmOperandIdtfr::REG_AOI) {
      continue;
    }
    AsmReg* virtReg = (AsmReg*)(inst->ops[i].first);
    RWP rwp = inst->ops[i].second;
    if (virtReg->isPhysReg) {
      continue;
    }
    if (freeRegPool.empty()) {
      panic("error");
    }
    AsmReg* physReg = freeRegPool.front();
    freeRegPool.pop_front();
    usdRegPool.push_front(physReg);
    inst->ops[i].first = physReg;
    if (rwp == READ_RWP) {
      AsmImm* spOffset = func->virtRegId2stackOffset[virtReg->virtNumber];
      newInst = new AsmInst(AsmInstIdtfr::LW_AII, {WRR(physReg, spOffset, name2PhysAsmReg["sp"])});
      it = blk->insts.insert(it, newInst);
      it++; /* 使之指向inst */
    }
    if (rwp == WRITE_RWP) {
      writePhyReg = physReg;
      writeStoreAddr = func->virtRegId2stackOffset[virtReg->virtNumber];
    }
  }
  it++; /* 指向inst的后一个 */
  if (writePhyReg != NULL) {
    newInst = new AsmInst(AsmInstIdtfr::SW_AII, {RRR(writePhyReg, writeStoreAddr, name2PhysAsmReg["sp"])});
    it = blk->insts.insert(it, newInst);
    it++;
  }
  for (auto iter = usdRegPool.begin(); iter != usdRegPool.end(); iter++) {
    freeRegPool.push_front(*iter);
  }
  return it;
}