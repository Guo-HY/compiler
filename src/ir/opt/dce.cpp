#include "opt.hpp"


static std::unordered_map<int, Instruction*> regId2DefInst;

static void insertRegId2DefInst(int regId, Instruction* defInst) 
{
  if (regId2DefInst.count(regId) != 0) {
    panic("error");
  } 
  regId2DefInst[regId] = defInst;
}
/* 初始化dce，建立虚拟寄存器号到定义指令集合的映射 */
static void dceInit(Function* func)
{
  regId2DefInst.clear();
  for (BasicBlock* bblk : func->basicBlocks) {
    for (Instruction* inst : bblk->instructions) {
      int defRegId = inst->getDefRegId();
      if (defRegId == -1) {
        continue;
      }
      insertRegId2DefInst(defRegId, inst);
    }
  }
  /* TODO : 遍历指令，对所有store指令，追溯其store的内存地址寄存器 */
}

/* 找到关键操作，ret，br，call getint，call putstr，call putint， 
   call putch，store是关键指令。*/
static std::list<Instruction*> usefulInsts; /* 待检查指令队列 */
static std::set<Instruction*> doneUsefulInsts; /* 已经检查完毕的指令 */

static bool isCriticalInst(Instruction* inst)
{
  if (isRetInst(inst) || isBrInst(inst) || isStoreInst(inst)) {
    return true;
  }
  if (isCallInst(inst)) {
    return true;
    // CallInst* callInst = (CallInst*)inst;
    // if (callInst->name == "getint" || callInst->name == "putint" || 
    //     callInst->name == "putch" || callInst->name == "putstr") {
    //   return true;
    // }
  }
  return false;
}

static void findCriticalInst(Function* func)
{
  usefulInsts.clear();
  doneUsefulInsts.clear();
  for (BasicBlock* bblk : func->basicBlocks) {
    for (Instruction* inst : bblk->instructions) {
      if (isCriticalInst(inst)) {
        inst->isCriticalInst = true;
        usefulInsts.push_back(inst);
      }
    }
  }
}

/* 迭代usefulInsts集合找到所有useful指令 */
static void findUsefulInst(Function* func)
{
  std::set<int> instUseRegs;
  Instruction* inst;
  Instruction* defInst;
  while (usefulInsts.size() > 0) {
    inst = usefulInsts.front(); usefulInsts.pop_front();
    instUseRegs = inst->getUseRegIds();
    for (int id : instUseRegs) {
      if (regId2DefInst.count(id) == 0) {
        // panic("error : inst Type = %d, id=%d", inst->instType, id);
        continue;/* 如果没找到，就是函数参数 */
      }
      defInst = regId2DefInst[id];
      if (doneUsefulInsts.count(defInst) == 0) {
        defInst->isCriticalInst = true;
        usefulInsts.push_back(defInst);
      }
    }
    doneUsefulInsts.insert(inst);
  }
}

static int rmUselessInst(Function* func)
{
  int rmInstNum = 0;
  for (BasicBlock* bblk : func->basicBlocks) {
    for (auto it = bblk->instructions.begin(); it != bblk->instructions.end();) {
      if (!(*it)->isCriticalInst) {
        it = bblk->instructions.erase(it);
        rmInstNum++;
      } else {
        it++;
      }
    }
  }
  return rmInstNum;
}


static int funcDeadCodeElim(Function* func)
{
  int rmNum;
  dceInit(func);
  findCriticalInst(func);
  findUsefulInst(func);
  rmNum = rmUselessInst(func);
  return rmNum;
}

void deadCodeElim(Module* module)
{
  int totalRmNum = 0;
  for (Function* func : module->funcDef) {
    totalRmNum += funcDeadCodeElim(func);
  }
  Log("deadCodeElim : totalRmInstNum = %d", totalRmNum);
}