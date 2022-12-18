#include "opt.hpp"

/* 虚拟寄存器id到所有使用这个寄存器的指令的映射 */
static std::unordered_map<int, std::set<Instruction*>*> regId2useInsts;

static void insertRegId2useInsts(int regId, Instruction* inst)
{
  if (regId2useInsts.count(regId) == 0) {
    regId2useInsts[regId] = new std::set<Instruction*>;
  }
  regId2useInsts[regId]->insert(inst);
}

/* 计算regId2useInsts */
static void gcpInit(Function* func)
{
  regId2useInsts.clear();
  std::set<int> useRegIds;
  for (BasicBlock* bblk : func->basicBlocks) {
    for (Instruction* inst : bblk->instructions) {
      useRegIds = inst->getUseRegIds();
      for (int id : useRegIds) {
        insertRegId2useInsts(id, inst);
      }
    }
  }
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

void globalConstantPropagation(Module* module)
{
  int totalRmInst = 0;
  for (Function* func : module->funcDef) {
    totalRmInst += funcGCP(func);
  }
  Log("globalConstantPropagation : totalRmInstNum=%d", totalRmInst);
}