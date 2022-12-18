#include "opt.hpp"

/* 虚拟寄存器id到所有使用这个寄存器的指令的映射 */
static std::unordered_map<int, std::set<Instruction*>*> regId2useInsts;
static std::unordered_map<int, Instruction*> regId2DefInst;

static void insertRegId2useInsts(int regId, Instruction* inst)
{
  if (regId2useInsts.count(regId) == 0) {
    regId2useInsts[regId] = new std::set<Instruction*>;
  }
  regId2useInsts[regId]->insert(inst);
}

static void insertRegId2DefInst(int regId, Instruction* defInst) 
{
  if (regId2DefInst.count(regId) != 0) {
    panic("error");
  } 
  regId2DefInst[regId] = defInst;
}

/* 计算regId2useInsts */
static void gcpInit(Function* func)
{
  regId2useInsts.clear();
  regId2DefInst.clear();
  std::set<int> useRegIds;
  for (BasicBlock* bblk : func->basicBlocks) {
    for (Instruction* inst : bblk->instructions) {
      useRegIds = inst->getUseRegIds();
      for (int id : useRegIds) {
        insertRegId2useInsts(id, inst);
      }
      int defRegId = inst->getDefRegId();
      if (defRegId == -1) {
        continue;
      }
      insertRegId2DefInst(defRegId, inst);
    }
  }
}

/* 尝试获取全局常量值，不成功返回NULL */
extern Type* ptrType2Type(Type* t);
static Value* tryConvertGlobalConst(LoadInst* inst)
{
  Value* addr = inst->pointer;
  /* 首先判断是否是全局int常量 */
  if (isGlobalValue(addr)) {
    GlobalValue* gint = ((GlobalValue*)addr);
    if (gint->isConst && !gint->isconstString) {
      if (!gint->globalInitValue->isValue) {
        panic("error");
      }
      return gint->globalInitValue->value;
    }
    return NULL;
  }
  /* 否则就是数组，需要先找到数组的gep指令 */
  Instruction* gInst = regId2DefInst[((VirtRegValue*)addr)->getId()];
  if (!isGEPInst(gInst)) {
    // panic("error");
    return NULL;
  }
  GEPInst* gepInst = (GEPInst*)gInst;
  addr = gepInst->ptrval;
  if (!isGlobalValue(addr)) {
    return NULL;
  }
  GlobalValue* garray = (GlobalValue*)addr;
  if (!(garray->isConst && !garray->isconstString)) {
    return NULL;
  }
  GlobalInitValue* initValue = garray->globalInitValue;
  for (u_long i = 1; i < gepInst->indexs.size(); i++) {
    Value* v = gepInst->indexs[i];
    if (!isNumberConstant(v)) {
      return NULL;
    }
    initValue = initValue->globalInitValues[((NumberConstant*)v)->value];
  }
  return initValue->value;
}

/* 检查指令是否可以计算为常数，若可以则返回NumberConstant，否则返回NULL */
static Value* tryConvertInst2Const(Instruction* inst)
{
  if (isBinaryInst(inst)) {
    BinaryInst* bInst = (BinaryInst*)inst;
    if (isNumberConstant(bInst->op1) && isNumberConstant(bInst->op2)) {
      int value, bitWidth;
      int op1 = ((NumberConstant*)(bInst->op1))->value;
      int op2 = ((NumberConstant*)(bInst->op2))->value;
      bitWidth = 32; /* 默认位宽为32 */
      switch (bInst->binaryInstType)
      {
      case BinaryInstIdtfr::ADD_BII:
        value = op1 + op2; break;
      case BinaryInstIdtfr::SUB_BII:
        value = op1 - op2; break;
      case BinaryInstIdtfr::MUL_BII:
        value = op1 * op2; break;
      case BinaryInstIdtfr::SDIV_BII:
        value = op1 / op2; break;
      case BinaryInstIdtfr::AND_BII:
        value = op1 & op2; break;
      case BinaryInstIdtfr::OR_BII:
        value = op1 | op2; break;
      case BinaryInstIdtfr::MOD_BII:
        value = op1 % op2; break;
      default:
        break;
      }
      return new NumberConstant(value, bitWidth);
    } 
    /* 这里进行了一部分指令变换，相当于做了一些窥孔优化 */
    else if (isNumberConstant(bInst->op1)) {
      int op1 = ((NumberConstant*)(bInst->op1))->value;
      if (op1 == 0 && (bInst->binaryInstType == BinaryInstIdtfr::MUL_BII || 
          bInst->binaryInstType == BinaryInstIdtfr::SDIV_BII || 
          bInst->binaryInstType == BinaryInstIdtfr::AND_BII ||
          bInst->binaryInstType == BinaryInstIdtfr::MOD_BII )) {
        return bInst->op1;
      }
      if ((op1 == 0 && bInst->binaryInstType == BinaryInstIdtfr::ADD_BII) ||
          (op1 == 1 && bInst->binaryInstType == BinaryInstIdtfr::MUL_BII) ||
          (op1 == 0 && bInst->binaryInstType == BinaryInstIdtfr::OR_BII)) {
        return bInst->op2;
      }
    } 
    else if (isNumberConstant(bInst->op2)) {
      int op2 = ((NumberConstant*)(bInst->op2))->value;
      if (op2 == 0 && (bInst->binaryInstType == BinaryInstIdtfr::ADD_BII ||
          bInst->binaryInstType == BinaryInstIdtfr::SUB_BII  ||
          bInst->binaryInstType == BinaryInstIdtfr::OR_BII)) {
        return bInst->op1;
      }
      if (op2 == 0 && (bInst->binaryInstType == BinaryInstIdtfr::MUL_BII ||
          bInst->binaryInstType == BinaryInstIdtfr::AND_BII)) {
        return bInst->op2;
      }
      if (op2 == 1 && (bInst->binaryInstType == BinaryInstIdtfr::MUL_BII ||
            bInst->binaryInstType == BinaryInstIdtfr::SDIV_BII)) {
        return bInst->op1;
      }
      if (op2 == 1 && bInst->binaryInstType == BinaryInstIdtfr::MOD_BII) {
        return new NumberConstant(0, 32);
      }
    }
  } else if (isZextInst(inst)) {
    ZextInst* zInst = (ZextInst*)inst;
    if (isNumberConstant(zInst->value)) {
      return new NumberConstant(((NumberConstant*)(zInst->value))->value, 32);
    }
  } else if (isIcmpInst(inst)) {
    IcmpInst* iInst = (IcmpInst*)inst;
    if (isNumberConstant(iInst->op1) && isNumberConstant(iInst->op2)) {
      int op1 = ((NumberConstant*)(iInst->op1))->value;
      int op2 = ((NumberConstant*)(iInst->op2))->value;
      int bitWidth = 1;
      int value = -1;
      switch (iInst->cond)
      {
      case EQ_ICMPCASE:
        value = (op1 == op2); break;
      case NE_ICMPCASE:
        value = (op1 != op2); break;
      case UGT_ICMPCASE:
        value = (((unsigned int)op1) > ((unsigned int)op2)); break;
      case UGE_ICMPCASE:
        value = (((unsigned int)op1) >= ((unsigned int)op2)); break; 
      case ULT_ICMPCASE:
        value = (((unsigned int)op1) < ((unsigned int)op2)); break;
      case ULE_ICMPCASE:
        value = (((unsigned int)op1) <= ((unsigned int)op2)); break;
      case SGT_ICMPCASE:
        value = (op1 > op2); break;
      case SGE_ICMPCASE:
        value = (op1 >= op2); break;
      case SLT_ICMPCASE:
        value = (op1 < op2); break;
      case SLE_ICMPCASE:
        value = (op1 <= op2); break;
      default:
        panic("error");
        break;
      }
      return new NumberConstant(value, bitWidth);
    }
  } else if (isLoadInst(inst)) {
    return tryConvertGlobalConst((LoadInst*)inst);
  }
  return NULL;
}

static int funcGCP(Function* func)
{
  int rmInstNum = 0;
  gcpInit(func);
  bool change = true;
  Value* constant;
  while (change) {
    change = false;
    for (BasicBlock* bblk : func->basicBlocks) {
      for (auto it = bblk->instructions.begin(); it != bblk->instructions.end(); ) {
        if ((constant = tryConvertInst2Const(*it)) != NULL) {
          /* 更新其它指令中对该指令定义寄存器的使用 */
          int defRegId = (*it)->getDefRegId();
          Value* defReg = (*it)->getDefReg();
          /* 没找到使用的指令,就跳过 */
          if (regId2useInsts.count(defRegId) == 0) {
            it++;
            continue;
          }
          if (defRegId == -1 || defReg==NULL ) {
            panic("error : defRegId=%d, defReg = %p", defRegId, defReg);
          }
          for (Instruction* inst : *regId2useInsts[defRegId]) {
            inst->updateUseValue(defReg, constant);
          }
          /* 删除指令 */
          it = bblk->instructions.erase(it);
          change = true;
          rmInstNum++;
        } else {
          it++;
        }
      }
    }
  }
  return rmInstNum;
}

/* 发现条件为常值的br指令，并重写为无条件跳转指令 */
static int convertBr2Uncond(Function* func)
{
  int convertNum = 0;
  for (BasicBlock* bblk : func->basicBlocks) {
    Instruction* termInst = bblk->instructions.back();
    if (isBrInst(termInst)) {
      BrInst* brInst = (BrInst*)termInst;
      if (!brInst->isUnCond && isNumberConstant(brInst->cond)) {
        int cond = ((NumberConstant*)brInst->cond)->value;
        brInst->isUnCond = true;
        if (cond != 0) {
          brInst->dest = brInst->iftrue;
        } else {
          brInst->dest = brInst->iffalse;
          brInst->iftrue = brInst->iffalse; /* 默认iftrue也是dest */
        }
        convertNum++;
      }
    }
  }
  return convertNum;
}

void globalConstantPropagation(Module* module)
{
  int totalRmInst = 0;
  int totalConvertBrNum = 0;
  for (Function* func : module->funcDef) {
    totalRmInst += funcGCP(func);
    totalConvertBrNum += convertBr2Uncond(func);
  }
  Log("globalConstantPropagation : totalRmInstNum=%d, \
    totalConvertBrNum=%d", totalRmInst, totalConvertBrNum);
}