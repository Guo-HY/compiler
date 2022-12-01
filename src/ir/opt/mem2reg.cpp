#include "opt.hpp"

/* 获取终结指令指向的基本块 */
std::vector<int> getTermInstPointed(Instruction* inst)
{
  std::vector<int> pointed;
  if (inst->instType == InstIdtfr::RET_II) {
    return pointed;
  }
  if (inst->instType == InstIdtfr::BR_II) {
    BrInst* brInst = (BrInst*)inst;
    if (brInst->isUnCond) {
      pointed.push_back(((LabelValue*)(brInst->dest))->getId());
    } else {
      pointed.push_back(((LabelValue*)(brInst->iftrue))->getId());
      pointed.push_back(((LabelValue*)(brInst->iffalse))->getId());
    }
    return pointed;
  }
  panic("error");
  return pointed;
}

/* 求CFG */
void calFunctionCfg(FunctionOptMsg* funcOptMsg)
{
  Function* func = funcOptMsg->func;
  for (int i = 0 ; i < func->basicBlocks.size(); i++) {
    BasicBlock* bblk = func->basicBlocks[i];
    funcOptMsg->id2bblk[bblk->getId()] = new BasicBlockOptMsg(bblk, funcOptMsg);
    if (i == 0) { /* 默认第0个块就是入口块 */
      funcOptMsg->entryBblkId = bblk->getId(); /* 设置entry基本块 */
    }
    std::vector<int> pointed = getTermInstPointed(bblk->instructions.back());
    funcOptMsg->addCfgEdge(bblk->getId(), pointed);
  }
}

/* 求每个基本块的严格支配 */
void calStrictlyDominated(FunctionOptMsg* funcMsg)
{
  std::set<int> survivor; /* 存放bfs遍历到的节点 */
  for (std::pair<int, BasicBlockOptMsg*> it0 : funcMsg->id2bblk) {
    int nowBblkId = it0.first;
    survivor.clear();
    funcMsg->bfsWithoutBblk(&survivor, nowBblkId);
    /* 遍历函数基本块，将不在survivor并且不是当前节点的节点插入当前节点的严格支配节点集合中,
       同时更新这些被支配节点的dominaterStrictly集合。
     */
    for (std::pair<int, BasicBlockOptMsg*> it1 : funcMsg->id2bblk) {
      if (survivor.count(it1.first) == 0 && it1.first != nowBblkId) {
        it0.second->strictlyDominated[it1.first] = it1.second;
        it1.second->dominaterStrictly[it0.first] = it0.second;
      }
    }
  }
}

/* 求每个基本块的直接支配者,入口块没有直接支配者 */
/* A的直接支配者是支配A的块集合中dom集最小的 */
void calIdom(FunctionOptMsg* funcMsg) 
{
  BasicBlockOptMsg* idom = NULL;
  int nowIdomDominatedSize = 0;
  for (std::pair<int, BasicBlockOptMsg*> it0 : funcMsg->id2bblk) {
    if (it0.first == funcMsg->entryBblkId) {
      continue;
    }
    idom = NULL;
    nowIdomDominatedSize = 2147483647;
    for (std::pair<int, BasicBlockOptMsg*> it1 : it0.second->dominaterStrictly) {
      BasicBlockOptMsg* dominater = it1.second;
      if (dominater->strictlyDominated.size() < nowIdomDominatedSize) {
        nowIdomDominatedSize = dominater->strictlyDominated.size();
        idom = dominater;
      }
    }
    it0.second->idom = idom;
    idom->idomds.push_back(it0.second);
  }
}


/* 求每个节点的支配边界 */
void calDF(FunctionOptMsg* funcMsg)
{
  for (std::pair<int, std::vector<int>*> links : funcMsg->cfgGraph) {
    int a = links.first;
    /* for (a,b) in CFG edges */
    for (int b : *(links.second)) {
      BasicBlockOptMsg* x = funcMsg->id2bblk[a];
      BasicBlockOptMsg* y = funcMsg->id2bblk[b];
      while (x->strictlyDominated.count(b) == 0) {
        x->DF[b] = y;
        if (x->idom == NULL) {
          break;
        }
        x = x->idom;
      }
    }
  }
}

/* 前序遍历支配树 */
void preorderTraversalDT(std::vector<BasicBlockOptMsg*>* bblkMsgs, BasicBlockOptMsg* nowBblk)
{
  bblkMsgs->push_back(nowBblk);
  for (BasicBlockOptMsg* it : nowBblk->idomds) {
    preorderTraversalDT(bblkMsgs, it);
  }
}

void calVarDefUse(FunctionOptMsg* funcMsg)
{
  /* 按照前序遍历支配树顺序 */
  std::vector<BasicBlockOptMsg*> bblkMsgs;
  preorderTraversalDT(&bblkMsgs, funcMsg->id2bblk[funcMsg->entryBblkId]);

  for (BasicBlockOptMsg* it0 : bblkMsgs) {
    BasicBlock* bblk = it0->bblk;
    for (Instruction* inst : bblk->instructions) {
      /* 如果是alloc指令并且是局部int变量 */
      if (isAllocaInst(inst) && 
        ((AllocaInst*)inst)->allocType->typeIdtfr == INTEGER_TI) {
        AllocaInst* allocInst = (AllocaInst*)inst;
        int addrRegId = ((VirtRegValue*)allocInst->result)->getId();
        if (funcMsg->varAddrRegId2defBblk.count(addrRegId) == 0) {
          funcMsg->varAddrRegId2defBblk[addrRegId] = new std::set<BasicBlockOptMsg*>;
        }
        /* 如果是store指令并且pointer是虚拟寄存器 */
      } else if (isStoreInst(inst) && 
        isVirtRegValue(((StoreInst*)inst)->pointer)) {
        int ptrRegId = ((VirtRegValue*)(((StoreInst*)inst)->pointer))->getId();
        /* 如果store的地址在varAddrRegId2defBblk有记录，就插入，
          这里需要保证对同一变量，alloc的处理一定在store之前
        */
        if (funcMsg->varAddrRegId2defBblk.count(ptrRegId) != 0) {
          funcMsg->varAddrRegId2defBblk[ptrRegId]->insert(it0);
        }
      }
    }
  }
}

void insertPhi(FunctionOptMsg* funcMsg)
{
  /* set of basic blocks where phi is added */
  std::set<BasicBlockOptMsg*> F;  
  std::list<BasicBlockOptMsg*> W;
  /* for every local int var */
  for (std::pair<int, std::set<BasicBlockOptMsg*>*> it0 : funcMsg->varAddrRegId2defBblk) {
    F.clear();
    W.clear();
    int vAddrRegId = it0.first;
    /* set if bblk that contain definitions of v */
    for (BasicBlockOptMsg* bblkMsg : *(it0.second)) {
      W.push_back(bblkMsg);
    }
    while (W.size() != 0) {
      BasicBlockOptMsg* X = W.front();
      W.pop_front();
      for (std::pair<int, BasicBlockOptMsg*> it1 : X->DF) {
        BasicBlockOptMsg* Y = it1.second;
        if (F.count(Y) == 0) {
          Y->addPhiInst(vAddrRegId);
          F.insert(Y);
          /* if Y not in Defs(v) */
          if (it0.second->count(Y) == 0) {
            W.push_back(Y);
          }
        }
      }
    }
  }
}

void updateInstUseValue(std::list<Instruction*>::iterator it, 
  std::list<Instruction*>* insts, Value* oldv, Value* newv)
{
  for ( ; it != insts->end(); it++) {
    (*it)->updateUseValue(oldv, newv);
  }
}

/* 变量重命名 */
void varRenaming(FunctionOptMsg* funcMsg)
{
  /* 按照前序遍历支配树的顺序遍历基本块,结果存在bblkMsgs中 */
  std::vector<BasicBlockOptMsg*> bblkMsgs;
  preorderTraversalDT(&bblkMsgs, funcMsg->id2bblk[funcMsg->entryBblkId]);
  bool flag = false;
  for (BasicBlockOptMsg* bblkMsg : bblkMsgs) {
    std::list<Instruction*>* insts = &(bblkMsg->bblk->instructions);
    for (std::list<Instruction*>::iterator it1 = insts->begin(); it1 != insts->end(); ) {
      flag = false;
      /* 局部int的alloc直接删除 */
      if (isAllocaInst(*it1) && 
        ((AllocaInst*)(*it1))->allocType->typeIdtfr == TypeIdtfr::INTEGER_TI ) {
        // Log("before delete alloca : alloc addr = %d", ((VirtRegValue*)(((AllocaInst*)(*it1))->result))->id);
        it1 = insts->erase(it1);
        flag = true;
        // Log("after delete alloca");
        /* 如果是局部int变量的load，更新所有指令中对该指令结果的使用并删除load */
      } else if (isLoadInst(*it1) && isVirtRegValue(((LoadInst*)*it1)->pointer)) {
        VirtRegValue* addrReg = (VirtRegValue*)(((LoadInst*)*it1)->pointer);
        if (funcMsg->varAddrRegId2defBblk.count(addrReg->getId()) != 0) {
          // Log("before delete load");
          /* load出来的一定是一个寄存器,但是变量的到达定义不一定是寄存器 */
          Log("before this");
          Value* reachdefValue = bblkMsg->getReachDefValue(addrReg->getId());
          /* 如果reachdefValue是寄存器，那么直接改编号就可以了 */
          if (isVirtRegValue(reachdefValue)) {
            /* 假设一个函数中编号相同的虚拟寄存器，对象也相同（同一块内存） */
            ((VirtRegValue*)(((LoadInst*)*it1)->result))->id = ((VirtRegValue*)reachdefValue)->getId(); /* 直接改id */
          } else if (isNumberConstant(reachdefValue)){
            /* 否则reachdefValue一定是NumberConstant, 此时需要替换所有使用指令中的引用 */
            updateInstUseValue(it1, insts, ((LoadInst*)*it1)->result, reachdefValue);
          } else {
            panic("error");
          }
          it1 = insts->erase(it1);
          flag = true;
          // Log("after delete load");
        }
      /* 如果是局部int变量的store，更新变量到达定义，并删除store */
      } else if (isStoreInst(*it1) && isVirtRegValue(((StoreInst*)*it1)->pointer)) {
        VirtRegValue* addrReg = (VirtRegValue*)(((StoreInst*)*it1)->pointer);
        if (funcMsg->varAddrRegId2defBblk.count(addrReg->getId()) != 0) {
          // Log(" before delete store");
          Value* storeValue = ((StoreInst*)*it1)->value;
          bblkMsg->reachdef[addrReg->getId()] = storeValue;
          it1 = insts->erase(it1);
          flag = true;
          // Log("after delete store");
        }
      }

      if (flag == false) {
        it1++;
      }
    }
    // Log("before update phi");
    /* 维护后继基本块的phi指令 */
    int nowBblkId = bblkMsg->bblk->getId();
    std::vector<int>* succeeds = funcMsg->cfgGraph[nowBblkId];
    for (int bblkId : *succeeds) {
      BasicBlockOptMsg* succeedBblk = funcMsg->id2bblk[bblkId];
      for (PhiInst* phiInst : succeedBblk->phiInsts) {
        Log("before this");
        Value* reachdefValue = bblkMsg->getReachDefValue(phiInst->varAddrRegId);
        if (reachdefValue != NULL) {
          phiInst->vardefs.push_back({reachdefValue, bblkMsg->bblk->label});
        }
      }
    }
    // Log("after update phi");
  }
}

void printCFGLog(FILE* fp, FunctionOptMsg* funcOptMsg);
void printStrictlyDominated(FILE* fp, FunctionOptMsg* funcOptMsg);
void printVarDefUse(FILE* fp, FunctionOptMsg* funcOptMsg);
void printIdom(FILE* fp, FunctionOptMsg* funcOptMsg);
void printDF(FILE* fp, FunctionOptMsg* funcOptMsg);
void printInsertPhi(FILE* fp, FunctionOptMsg* funcOptMsg);
/* 以函数为单位进行mem2reg */
void funcMem2reg(Function* func)
{
  // FILE* fp = fopen("mem2reg.log", "w");
  // fprintf(fp, "func name = %s\n", func->funcName.c_str());

  FunctionOptMsg* funcOptMsg = new FunctionOptMsg(func);
  /* 求CFG */
  calFunctionCfg(funcOptMsg);
  // printCFGLog(fp, funcOptMsg);
  // return;
  /* 求每个基本块的严格支配 */
  calStrictlyDominated(funcOptMsg);
  // printStrictlyDominated(fp, funcOptMsg);
  // return;
  /* 求每个基本块的直接支配者,入口块没有直接支配者 */
  calIdom(funcOptMsg);
  // printIdom(fp, funcOptMsg);
  // return;
  /* 求每个节点的支配边界 */
  calDF(funcOptMsg);
  // printDF(fp, funcOptMsg);
  // return;
  /* 计算变量的定义使用点 */
  calVarDefUse(funcOptMsg);
  // printVarDefUse(fp, funcOptMsg);
  // return;

  /* 插入phi节点 */
  insertPhi(funcOptMsg);
  // printInsertPhi(fp, funcOptMsg);
  // return;
  /* 变量重命名 */
  varRenaming(funcOptMsg);
  /* 将phi指令插入基本块中 */
  for (std::pair<int, BasicBlockOptMsg*> it0 : funcOptMsg->id2bblk) {
    std::list<Instruction*>* insts = &(it0.second->bblk->instructions);
    for (PhiInst* phiInst : it0.second->phiInsts) {
      insts->push_front(phiInst);
    }
  }
}

void mem2reg(Module* module)
{
  for (Function* func : module->funcDef) {
    funcMem2reg(func);
  }
}

void FunctionOptMsg::bfsWithoutBblk(std::set<int>* visited, int excludeBblkId) {
    visited->clear();
    if (excludeBblkId == entryBblkId) {
      return;
    }
    std::list<int> queue;
    queue.push_back(entryBblkId);
    visited->insert(entryBblkId);

    while (queue.size() != 0) {
      int nowBblkId = queue.front();
      queue.pop_front();
      if (cfgGraph.count(nowBblkId) == 0) {
        panic("error");
      }
      std::vector<int>* pointedBblks = cfgGraph[nowBblkId];
      for (int pointedBblk : *pointedBblks) {
        if (pointedBblk != excludeBblkId && visited->count(pointedBblk) == 0) {
          queue.push_back(pointedBblk);
          visited->insert(pointedBblk);
        }
      }
    }
}

void BasicBlockOptMsg::addPhiInst(int vAddrId) {
    PhiInst* inst = new PhiInst();
    inst->varAddrRegId = vAddrId;
    inst->result = new VirtRegValue(func->func->maxLlvmIrId, new IntegerType(32));
    func->func->maxLlvmIrId = func->func->maxLlvmIrId + 1;
    this->phiInsts.push_back(inst);
    /* 同时初始化到达定义 */
    this->reachdef[vAddrId] = inst->result;
}

void printCFGLog(FILE* fp, FunctionOptMsg* funcOptMsg)
{
  fprintf(fp, "cfg ------------------------\n");
  for (std::pair<int, std::vector<int>*> it0 : funcOptMsg->cfgGraph) {
    for (int it1 : *(it0.second)) {
      fprintf(fp, "%d -> %d ", it0.first, it1);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "---------------------------\n");
}

void printStrictlyDominated(FILE* fp, FunctionOptMsg* funcOptMsg)
{
  fprintf(fp, "StrictlyDominated & dominaterStrictly ------------------------\n");
  for (std::pair<int, BasicBlockOptMsg*> it0 : funcOptMsg->id2bblk) {
    fprintf(fp, "block id = %d, StrictlyDominated = ", it0.first);
    for (std::pair<int, BasicBlockOptMsg*> it1 : it0.second->strictlyDominated) {
      fprintf(fp , " %d ",it1.first);
    }
    fprintf(fp, ", dominaterStrictly = ");
    for (std::pair<int, BasicBlockOptMsg*> it1 : it0.second->dominaterStrictly) {
      fprintf(fp , " %d ",it1.first);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "---------------------------\n");
}

void printVarDefUse(FILE* fp, FunctionOptMsg* funcOptMsg) 
{
  fprintf(fp, "varDefUse-----------------------\n");
  for (std::pair<int, std::set<BasicBlockOptMsg*>*> it0 : funcOptMsg->varAddrRegId2defBblk) {
    fprintf(fp, "var addr = %d , def block = ", it0.first);
    for (BasicBlockOptMsg* it1 : *(it0.second)) {
      fprintf(fp, "  %d ", it1->bblk->getId());
    }
    fprintf(fp, "\n");
  } 
  fprintf(fp, "---------------------------\n");
}

void printIdom(FILE* fp, FunctionOptMsg* funcOptMsg)
{
  fprintf(fp, "idom---------------------\n");
  for (std::pair<int, BasicBlockOptMsg*> it0 : funcOptMsg->id2bblk) {
    if (it0.second->idom == NULL) {
      fprintf(fp, "block id = %d, no idom\n",it0.second->bblk->getId());
      continue;
    }
    fprintf(fp, "block id = %d, idom = %d\n", it0.second->bblk->getId(), 
      it0.second->idom->bblk->getId());
  }
}

void printDF(FILE* fp, FunctionOptMsg* funcOptMsg)
{
  fprintf(fp, "DF ---------------------\n");
  for (std::pair<int, BasicBlockOptMsg*> it0 : funcOptMsg->id2bblk) {
    fprintf(fp, "block id = %d, DF = ", it0.second->bblk->getId());
    for (std::pair<int, BasicBlockOptMsg*> it1 : it0.second->DF) {
      fprintf(fp, " %d ", it1.first);
    }
    fprintf(fp, "\n");
  }
}

void printInsertPhi(FILE* fp, FunctionOptMsg* funcOptMsg)
{
  fprintf(fp, "insertPhi---------------------\n");
  for (std::pair<int, BasicBlockOptMsg*> it0 : funcOptMsg->id2bblk) {
    fprintf(fp, "block id = %d ,phi inst : ", it0.first);
    for (PhiInst* inst : it0.second->phiInsts) {
      fprintf(fp, " varAddr=%d, ", inst->varAddrRegId);
    }
    fprintf(fp, "\n");
  }
}