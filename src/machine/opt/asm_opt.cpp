#include "asm_opt.hpp"
extern std::unordered_map<std::string, int> name2regPhysNum;
extern std::unordered_map<std::string, AsmReg*> name2PhysAsmReg;



/* 这里假设跳转指令只有jr，bne，j,且jr没有地址 */
static void getBrInstPointed(AsmInst* inst, std::vector<int>* success)
{ 
  if (inst->asmInstIdtfr == AsmInstIdtfr::J_AII) {
    success->push_back(((AsmLabel*)(inst->ops[0].first))->bblkLabelId);
  } else if (inst->asmInstIdtfr == AsmInstIdtfr::BNE_AII) {
    success->push_back(((AsmLabel*)(inst->ops[2].first))->bblkLabelId);
  }
}

/* 求函数cfg */
static void calFunctionCfg(AsmFuncOptMsg* funcMsg)
{ 
  AsmFunction* func = funcMsg->asmFunction;
  for (u_long i = 0; i < func->basicBlocks.size(); i++) {
    AsmBasicBlock* bblk = func->basicBlocks[i];
    funcMsg->id2bblk[bblk->getBblkId()] = new AsmBlockOptMsg(bblk, func);
    if (i == 0) {
      funcMsg->entryBblkId = bblk->getBblkId();
    }
    /* 求bblk的后继，注意bblk的最后一条或两条指令可能是终结指令 */
    std::vector<int> success;
    success.clear();
    /* 直接遍历基本块 */
    for (AsmInst* inst : bblk->insts) {
      getBrInstPointed(inst, &success);
    }
    funcMsg->addCfgEdge(bblk->getBblkId(), success);  
  }
}


/* 目前只有BNE, J, JR */
static bool isTermInst(AsmInst* inst)
{
  if (inst->asmInstIdtfr == AsmInstIdtfr::BNE_AII || 
    inst->asmInstIdtfr == AsmInstIdtfr::J_AII ||
    inst->asmInstIdtfr == AsmInstIdtfr::JR_AII) {
      return true;
    }
  return false;
}

static AsmReg* blockAddImm2RegInst(AsmFuncOptMsg* funcMsg, AsmImm* imm, AsmLabel* label)
{
  int newRegId = funcMsg->asmFunction->maxVirtRegId;
  (funcMsg->asmFunction->maxVirtRegId)++;
  AsmReg* newReg = new AsmReg(false, newRegId);
  AsmInst* addInst = new AsmInst(AsmInstIdtfr::ADDIU_AII, 
      {WRR(newReg, name2PhysAsmReg["zero"], imm)});
  
  int bblkId = label->bblkLabelId;
  AsmBlockOptMsg* bblkMsg = funcMsg->id2bblk[bblkId];
  std::list<AsmInst*>::iterator it;
  for (it = bblkMsg->asmBasicBlock->insts.begin(); 
    it != bblkMsg->asmBasicBlock->insts.end(); it++) {
    if (isTermInst(*it)) {
      bblkMsg->asmBasicBlock->insts.insert(it, addInst);
      break;
    }
  }
  return newReg;
}

/* 将phi参数中的常量转变为前导块中的reg */
static void phiImm2Reg(AsmFuncOptMsg* funcMsg)
{
  AsmFunction* func = funcMsg->asmFunction;
  for (AsmBasicBlock* bblk : func->basicBlocks) {
    for (AsmInst* inst : bblk->insts) {
      if (inst->asmInstIdtfr == AsmInstIdtfr::PHI_AII) {
        AsmPhiInst* phiInst = (AsmPhiInst*)inst;
        for (u_long i = 0; i < phiInst->varDefs.size(); i++) {
          std::pair<AsmOperand*, AsmLabel*> it = phiInst->varDefs[i];
          if (it.first->asmOperandIdtfr == AsmOperandIdtfr::IMM_AOI) {
            AsmReg* newReg = blockAddImm2RegInst(funcMsg, (AsmImm*)it.first, it.second);
            phiInst->varDefs[i].first = newReg;
            phiInst->ops[i+1].first = newReg;
          }
        }
      }
    }
  }
}

/* 求解函数中每个基本块的useVar和killVar */
/* 只计算虚拟寄存器 */
static void calUseKill(AsmFuncOptMsg* funcMsg)
{
  AsmFunction* func = funcMsg->asmFunction;
  for (std::pair<int, AsmBlockOptMsg*> id2bblk : funcMsg->id2bblk) {
    AsmBlockOptMsg* bblkMsg = id2bblk.second;
    AsmBasicBlock* bblk = bblkMsg->asmBasicBlock;
    for (AsmInst* inst : bblk->insts) {
      for (std::pair<AsmOperand*, RWP> op : inst->ops) {
        if (op.first->asmOperandIdtfr != AsmOperandIdtfr::REG_AOI) {
          continue;
        }
        int regId;
        AsmReg* reg = ((AsmReg*)op.first);
        if (reg->isPhysReg) {
          continue;
        }
        regId = reg->virtNumber;
        if (op.second == RWP::READ_RWP && bblkMsg->killVar.count(regId) == 0) {
          bblkMsg->useVar[regId] = reg;
        }
        if (op.second == RWP::WRITE_RWP) {
          bblkMsg->killVar[regId] = reg;
        }
      }
    }
  }
}

static void calLiveOut(AsmFuncOptMsg* funcMsg)
{
  bool change = true;
  AsmFunction* func = funcMsg->asmFunction;
  while (change) {
    change = false;
    for (std::pair<int, AsmBlockOptMsg*> id2bblk : funcMsg->id2bblk) {
      AsmBlockOptMsg* bblkMsg = id2bblk.second;
      AsmBasicBlock* bblk = bblkMsg->asmBasicBlock;
      /* 对当前基本块的每个后继 */
      std::vector<int>* success = funcMsg->cfgGraph[bblk->getBblkId()];
      for (int successId : *success) {
        AsmBlockOptMsg* successMsg = funcMsg->id2bblk[successId];
        /* 添加liveout - use */
        for (std::pair<int, AsmReg*> it : successMsg->useVar) {
          if (bblkMsg->liveOut.count(it.first) == 0) {
            bblkMsg->liveOut[it.first] = it.second;
            change = true;
          }
        }
        /* 添加liveout - kill */
        for (std::pair<int, AsmReg*> it : successMsg->liveOut) {
          if (successMsg->killVar.count(it.first) == 0 && 
            bblkMsg->liveOut.count(it.first) == 0) {
            bblkMsg->liveOut[it.first] = it.second;
            change = true;
          }
        }
      }
    }
  }
}

/* 计算LR的准备工作，初始化virtId2reg与virtregId2LR（并查集） */
static void initLR(AsmFuncOptMsg* funcMsg)
{
  AsmFunction* func = funcMsg->asmFunction;
  for (AsmBasicBlock* bblk : func->basicBlocks) {
    for (AsmInst* inst : bblk->insts) {
      for (std::pair<AsmOperand*, RWP> op : inst->ops) {
        if (op.first->asmOperandIdtfr == AsmOperandIdtfr::REG_AOI) {
          AsmReg* reg = (AsmReg*)(op.first);
          if (!reg->isPhysReg) {
            funcMsg->virtId2reg[reg->virtNumber] = reg;
            funcMsg->virtregId2LR[reg->virtNumber] = reg->virtNumber;
          }
        }
      }
    }
  }
}

/* 对于phi要合并参数与结果的LR */
static void calLR(AsmFuncOptMsg* funcMsg)
{
  AsmFunction* func = funcMsg->asmFunction;
  for (AsmBasicBlock* bblk : func->basicBlocks) {
    for (AsmInst* inst : bblk->insts) {
      if (inst->asmInstIdtfr == AsmInstIdtfr::PHI_AII){
        AsmPhiInst* phiInst = (AsmPhiInst*)inst;
        int resultLR = funcMsg->findLR(phiInst->result->virtNumber);
        for (std::pair<AsmOperand*, AsmLabel*> varDef : phiInst->varDefs) {
          if (varDef.first->asmOperandIdtfr != AsmOperandIdtfr::REG_AOI
            || ((AsmReg*)varDef.first)->isPhysReg) {
            panic("error");
          }
          int paraLR = funcMsg->findLR(((AsmReg*)varDef.first)->virtNumber);
          funcMsg->unionLR(resultLR, paraLR);
        }
      }
    }
  }
  /* 最后建立LR2virtRegId */
  for (std::pair<int, AsmReg*> it : funcMsg->virtId2reg) {
    int virtRegId = it.first;
    int LR = funcMsg->findLR(virtRegId);
    if (funcMsg->LR2virtRegId.count(LR) == 0) {
      funcMsg->LR2virtRegId[LR] = new std::vector<int>;
    }
    funcMsg->LR2virtRegId[LR]->push_back(virtRegId);
  }

}

/* 计算冲突图 */
static void calConflictGraph(AsmFuncOptMsg* funcMsg)
{
  AsmFunction* func = funcMsg->asmFunction;
  for (std::pair<int,AsmBlockOptMsg*> it : funcMsg->id2bblk) {
    AsmBasicBlock* bblk = it.second->asmBasicBlock;
    /* 建立基于LR的livenow */
    std::set<int> liveNow;
    for (std::pair<int, AsmReg*> iter : it.second->liveOut) {
      liveNow.insert(funcMsg->findLR(iter.first));
    }
    /* 反向遍历基本块 */
    for (auto iter = bblk->insts.rbegin(); iter != bblk->insts.rend(); iter++ ) {
      AsmInst* inst = *iter;
      /* find writeLR */
      int writeLR = -1;
      for (std::pair<AsmOperand*, RWP> op : inst->ops) {
        if (op.second == RWP::WRITE_RWP && !((AsmReg*)op.first)->isPhysReg) {
          writeLR = funcMsg->findLR(((AsmReg*)op.first)->virtNumber);
          break;
        }
      }
      if (writeLR != -1) {
        /* for each LRi in LiveNow, add (writeLR, LRi) to graph */
        for (int liveLR : liveNow) {
          funcMsg->addConflictEdge(writeLR, liveLR);
        }
        /* reomve writeLR from LiveNow */
        liveNow.erase(writeLR);
      }
      /* add readLR to LiveNow */
      for (std::pair<AsmOperand*, RWP> op : inst->ops) {
        if (op.second == RWP::READ_RWP && 
          op.first->asmOperandIdtfr == AsmOperandIdtfr::REG_AOI 
          && !((AsmReg*)op.first)->isPhysReg) {
          int readLR = funcMsg->findLR(((AsmReg*)op.first)->virtNumber);
          liveNow.insert(readLR);
        }
      }
    }
  }
  /* 最后将没有冲突的节点加入冲突图 */
  for (std::pair<int, AsmReg*> it : funcMsg->virtId2reg) {
    int LR = funcMsg->findLR(it.first);
    funcMsg->addConflictEdge(LR, LR);
  }
}

/* 参与全局寄存器分配的物理寄存器 */
std::vector<std::string> allocRegPool = {
  "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9", 
  "a1", "a2", "a3",
};

struct conflictGraphNode {
  int LR;
  std::set<int>* links;
  int degree; /* 节点的度 */
  std::string physRegName;
};

struct LRWithDegreeRule {
  bool operator()(struct conflictGraphNode* a, struct conflictGraphNode* b) const {
    if (a->degree == b->degree) {
      return a->LR < b->LR;
    }
    return a->degree < b->degree;
  }
};

static bool isNoConflictReg(std::string regName, std::set<int>* links, AsmFuncOptMsg* funcMsg)
{
  for (int LR : *links) {
    if (funcMsg->LR2physRegName[LR] == regName) {
      return false;
    }
  }
  return true;
}

/* 根据冲突图分配物理寄存器 */
static void allocReg(AsmFuncOptMsg* funcMsg)
{ 
  struct conflictGraphNode* node;
  std::unordered_map<int, struct conflictGraphNode*> LR2node;
  std::map<struct conflictGraphNode*, int, LRWithDegreeRule> LRWithDegree;
  for (std::pair<int, std::set<int>*> it : funcMsg->conflictGraph) {
    node = new struct conflictGraphNode;
    node->LR = it.first;
    node->links = it.second;
    node->degree = it.second->size();
    LRWithDegree[node] = 0;
    LR2node[node->LR] = node;
  }
  int validRegNum = allocRegPool.size();
  struct conflictGraphNode* nowNode;
  std::vector<struct conflictGraphNode*> stack;
  while (LRWithDegree.size() > 0) {
    auto it = LRWithDegree.begin();
    nowNode = it->first;
    LRWithDegree.erase(nowNode);
    /* 标记是否分配寄存器并插入队列 */
    if (nowNode->degree >= validRegNum) {
      funcMsg->LR2physRegName[nowNode->LR] = "not";
      funcMsg->spillVirtRegNum ++;
    } else {
      funcMsg->LR2physRegName[nowNode->LR] = "yes";
    }
    stack.push_back(nowNode);
    /* 减少相连节点的度 */
    struct conflictGraphNode* linkNode;
    for (int linkLR : *(nowNode->links)) {
      linkNode = LR2node[linkLR];
      if (LRWithDegree.count(linkNode)) {
        LRWithDegree.erase(linkNode);
        linkNode->degree--;
        LRWithDegree[linkNode] = 0;
      }
    }
  }

  /* 用queue分配物理寄存器 */
  bool flag;
  while (stack.size() > 0) {
    flag = false;
    nowNode = stack.back(); stack.pop_back();
    if (funcMsg->LR2physRegName[nowNode->LR] == "not") {
      flag = true;
      continue;
    }
    for (std::string regName : allocRegPool) {
      if (isNoConflictReg(regName, nowNode->links, funcMsg)) {
        funcMsg->LR2physRegName[nowNode->LR] = regName;
        flag = true;
        break;
      }
    }
    if (!flag) {
      panic("error");
    }
  }
}

/* 迭代给标记为not的虚拟寄存器尝试分配物理寄存器，直到不动点 */
static void iterAllocReg(AsmFuncOptMsg* funcMsg)
{
  /* 首先将所有标记为not的LR放在一个set里 */
  int beforeSpillNum = funcMsg->spillVirtRegNum;
  std::set<int> notLR;
  for (std::pair<int, std::string> it : funcMsg->LR2physRegName) {
    if (it.second == "not") {
      notLR.insert(it.first);
    }
  }
  /* 迭代直到不动点 */
  int iterNum = 0;
  bool change = true;
  while (change) {
    iterNum++;
    change = false;
    for (auto it = notLR.begin(); it != notLR.end(); it++) {
      int LR = *it;
      for (std::string regName : allocRegPool) {
        if (isNoConflictReg(regName, funcMsg->conflictGraph[LR], funcMsg)) {
          funcMsg->LR2physRegName[LR] = regName;
          change = true;
          it = notLR.erase(it);
          funcMsg->spillVirtRegNum--;
          break;
        }
      }
    }
  }
  Log("in iterAllocReg : beforeSpillNum = %d, afterSpillNum = %d, iterNum=%d",
    beforeSpillNum, funcMsg->spillVirtRegNum, iterNum);
}

/* 这里仅映射分配了物理寄存器寄存器的虚拟寄存器。
  被逐出的虚拟寄存器使用plainRegAllocator分配
  */
static void mapVirtReg2Phys(AsmFuncOptMsg* funcMsg)
{
  AsmFunction* func = funcMsg->asmFunction;
  for (AsmBasicBlock* bblk : func->basicBlocks) {
    std::list<AsmInst*>::iterator it = bblk->insts.begin();
    while (it != bblk->insts.end()) {
      AsmInst* inst = *it;
      for (u_long i = 0; i < inst->ops.size(); i++) {
        if (inst->ops[i].first->asmOperandIdtfr == AsmOperandIdtfr::REG_AOI) {
          AsmReg* reg = (AsmReg*)(inst->ops[i].first);
          if (reg->isPhysReg) { continue; }
          int LR = funcMsg->virtregId2LR[reg->virtNumber];
          std::string physRegName = funcMsg->LR2physRegName[LR];
          if (physRegName == "not") { continue; }
          int physRegId = name2regPhysNum[physRegName];
          reg->isPhysReg = true;
          reg->physNumber = physRegId;
        }
      }
      it++;
    }
  }
}

/* 在进入函数时保存所有函数中用到的分配寄存器($s & $t)
    退出函数时恢复这些寄存器
 */
static void insertSaveRecoverRegInst(AsmFuncOptMsg* funcMsg)
{
  AsmFunction* func = funcMsg->asmFunction;
  int nowSpOffsetWord = func->stackWordSize;
  std::unordered_map<std::string, AsmImm*> saveRegName2stackOffset;
  for (std::pair<int, std::string> it : funcMsg->LR2physRegName) {
    if (it.second == "not") { 
      continue;
    }
    if (it.second == "yes") {
      panic("error");
    }
    if (saveRegName2stackOffset.count(it.second) == 0) {
      saveRegName2stackOffset[it.second] = new AsmImm(nowSpOffsetWord * 4);
      nowSpOffsetWord++;
    }
  }
  int addWordSize = nowSpOffsetWord - func->stackWordSize;
  func->stackWordSize = nowSpOffsetWord; /* 要更新栈大小 */
  func->frameSize.first->immediate = nowSpOffsetWord * -4;
  func->frameSize.second->immediate = nowSpOffsetWord * 4;
  std::unordered_map<int, AsmImm*>::iterator iter;
  for (iter = func->funcArgsId2stackOffset.begin();
    iter != func->funcArgsId2stackOffset.end(); iter++) {
    iter->second->immediate += addWordSize * 4;
  }

  auto it = func->basicBlocks[0]->insts.begin();
  it++; /* 需要指向addiu sp sp -stacksize 的下一条 */
  for (std::pair<std::string, AsmImm*> regs : saveRegName2stackOffset) {
    if (name2PhysAsmReg.count(regs.first) == 0) {
      panic("error : regs.first=%s#", regs.first.c_str());
    }
    it = func->basicBlocks[0]->insts.insert(it, new AsmInst(AsmInstIdtfr::SW_AII, 
    {RRR(name2PhysAsmReg[regs.first], regs.second, name2PhysAsmReg["sp"])}));
  }
  AsmInst* nowInst;
  for (AsmBasicBlock* bblk : func->basicBlocks) {
    auto it = bblk->insts.end(); /* end 指向最后一个元素的下一个位置 */
    it--;
    nowInst = *it;
    if (nowInst->asmInstIdtfr == AsmInstIdtfr::JR_AII) {
      it--; /* point to the 'add sp' */
      for (std::pair<std::string, AsmImm*> regs : saveRegName2stackOffset) {
        if (name2PhysAsmReg.count(regs.first) == 0) {
          panic("error");
        }
        it = bblk->insts.insert(it, new AsmInst(AsmInstIdtfr::LW_AII, 
        {WRR(name2PhysAsmReg[regs.first], regs.second, name2PhysAsmReg["sp"])}));
      }
    }
  }
}

/* 最后将没有分配物理寄存器的虚拟寄存器号统一转换为对应的LR
   这样才能消除PHI
 */
static void convertVirtRegId2LR(AsmFuncOptMsg* funcMsg)
{
  AsmFunction* func = funcMsg->asmFunction;
  for (AsmBasicBlock* bblk : func->basicBlocks) {
    for (AsmInst* inst : bblk->insts) {
      for (std::pair<AsmOperand*, RWP> op : inst->ops) {
        if (op.first->asmOperandIdtfr == AsmOperandIdtfr::REG_AOI) {
          AsmReg* reg = (AsmReg*)(op.first);
          if (!reg->isPhysReg) {
            reg->virtNumber = funcMsg->findLR(reg->virtNumber);
          }
        }
      }
    }
  }
}

void printAsmCfg(FILE* fp, AsmFuncOptMsg* funcMsg);
void printUseKillLiveOut(FILE* fp, AsmFuncOptMsg* funcMsg);
void printLR(FILE* fp, AsmFuncOptMsg* funcMsg);
void printConflictGraph(FILE* fp, AsmFuncOptMsg* funcMsg);
void printLR2physRegName(FILE* fp, AsmFuncOptMsg* funcMsg);

static void funcGraphRegAllocator(AsmFuncOptMsg* funcMsg)
{
  #ifdef ENABLE_Log
  FILE* fp = fopen("asmopt.log", "a");
  #endif
  /* 求cfg，同时建立AsmBlockOptMsg，id2bblk */
  calFunctionCfg(funcMsg);
  #ifdef ENABLE_Log
  printAsmCfg(fp, funcMsg);
  #endif
  /* 将phi参数中的常量转变为前导块中的reg */
  /* 这里可能会插入冗余的add指令，但是不会导致正确性问题 */
  phiImm2Reg(funcMsg);
  /* 求解函数中每个基本块的useVar和killVar */ 
  calUseKill(funcMsg);
  /* 求解liveout */
  calLiveOut(funcMsg);
  #ifdef ENABLE_Log
  printUseKillLiveOut(fp, funcMsg);
  #endif
  /* 计算LR的准备工作 */
  initLR(funcMsg);
  /* 计算活动范围 */
  calLR(funcMsg);
  #ifdef ENABLE_Log
  printLR(fp, funcMsg);
  #endif
  /* 计算冲突图 */
  calConflictGraph(funcMsg);
  #ifdef ENABLE_Log
  printConflictGraph(fp, funcMsg);
  #endif
  /* 分配物理寄存器 */
  allocReg(funcMsg);
  iterAllocReg(funcMsg);
  /* 完成寄存器映射，插入lw，sw指令 */
  mapVirtReg2Phys(funcMsg);
  #ifdef ENABLE_Log
  printLR2physRegName(fp, funcMsg);
  fclose(fp);
  #endif
  /* 插入保存寄存器指令 */
  insertSaveRecoverRegInst(funcMsg);
  /* 最后将没有分配物理寄存器的虚拟寄存器号统一转换为对应的LR */
  convertVirtRegId2LR(funcMsg);
}

void graphRegAllocator(AsmModule* module)
{
  for (AsmFunction* func : module->functions) {
    funcGraphRegAllocator(new AsmFuncOptMsg(func));
  }
}

void printAsmCfg(FILE* fp, AsmFuncOptMsg* funcMsg)
{
  fprintf(fp, "in func name=%s\n", funcMsg->asmFunction->funcName->toString().c_str());
  fprintf(fp, "----------printAsmCfg-------------");
  for (std::pair<int, std::vector<int>*> it : funcMsg->cfgGraph) {
    fprintf(fp, "%d -> ", it.first);
    for (int link : *it.second) {
      fprintf(fp, "%d,  ", link);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
}

void printUseKillLiveOut(FILE* fp, AsmFuncOptMsg* funcMsg)
{
  fprintf(fp, "----------printUseKill-------------");
  for (std::pair<int, AsmBlockOptMsg*> it : funcMsg->id2bblk) {
    fprintf(fp, "\nbblk id = %d:\n", it.first);
    AsmBlockOptMsg* bblk = it.second;
    fprintf(fp, "killVar: ");
    for (std::pair<int, AsmReg*> it1 : bblk->killVar) {
      fprintf(fp, "%d,  ", it1.first);
    }
    fprintf(fp, "\nuseVar: ");
    for (std::pair<int, AsmReg*> it1 : bblk->useVar) {
      fprintf(fp, "%d,  ", it1.first);
    }
    fprintf(fp, "\nliveOut: ");
    for (std::pair<int, AsmReg*> it1 : bblk->liveOut) {
      fprintf(fp, "%d,  ", it1.first);
    }
  }
  fprintf(fp, "\n");
}

void printLR(FILE* fp, AsmFuncOptMsg* funcMsg)
{
  fprintf(fp, "----------printLR-------------");
  for (std::pair<int, std::vector<int>*> it : funcMsg->LR2virtRegId) {
    fprintf(fp, "LR = %d, virtRegId= ", it.first);
    for (int id : *it.second) {
      fprintf(fp, "%d,  ", id);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
}

void printConflictGraph(FILE* fp, AsmFuncOptMsg* funcMsg)
{
  fprintf(fp, "----------printConflictGraph-------------");
  for (std::pair<int, std::set<int>*> it : funcMsg->conflictGraph) {
    fprintf(fp, "%d -> ", it.first);
    for (int link : *it.second) {
      fprintf(fp, "%d,  ", link);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
}

void printLR2physRegName(FILE* fp, AsmFuncOptMsg* funcMsg)
{
  fprintf(fp, "----------printLR2physRegName-------------");
  fprintf(fp ,"now spillVirtRegNum = %d\n", funcMsg->spillVirtRegNum);
  for (std::pair<int, std::string>it : funcMsg->LR2physRegName) {
    fprintf(fp, " LR = %d, physReg=%s#\n", it.first, it.second.c_str());
  }
  fprintf(fp, "\n");
}
