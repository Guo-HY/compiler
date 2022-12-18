#include "opt.hpp"

/* 判断一个数是否是2的幂次，若是返回幂次，否则返回-1 */
int is2pow(int n)
{
  if (n < 0) {
    return -1;
  }
  if (n & (n-1)) {
    return -1;
  }
  int pow = 0;
  while (n) {
    pow++;
    n >>= 1;
  }
  return pow - 1;
}

void funcDullMulDiv(Function* func)
{
  int pow;
  int value;
  for (BasicBlock* bblk : func->basicBlocks) {
    for (auto it = bblk->instructions.begin(); it != bblk->instructions.end(); it++) {
      if (isBinaryInst(*it)) {
        BinaryInst* inst = (BinaryInst*)(*it);
        if (inst->binaryInstType == BinaryInstIdtfr::MUL_BII) {
          if (isNumberConstant(inst->op1)) {
            value = ((NumberConstant*)(inst->op1))->value;
            if ((pow = is2pow(value)) != -1) {
              inst->binaryInstType = BinaryInstIdtfr::SHL_BII;
              inst->op1 = inst->op2;
              inst->op2 = new NumberConstant(pow, 32);
            }
          } else if (isNumberConstant(inst->op2)) {
            value = ((NumberConstant*)(inst->op2))->value;
            if ((pow = is2pow(value)) != -1) {
              inst->binaryInstType = BinaryInstIdtfr::SHL_BII;
              inst->op2 = new NumberConstant(pow, 32);
            }
          }
        } else if (inst->binaryInstType == BinaryInstIdtfr::SDIV_BII) {
          if (isNumberConstant(inst->op2)) {
            value = ((NumberConstant*)(inst->op2))->value;
            if ((pow = is2pow(value)) != -1) {
              inst->binaryInstType = BinaryInstIdtfr::ASHR_BII;
              inst->op2 = new NumberConstant(pow, 32);
            }
          }
        } else if (inst->binaryInstType == BinaryInstIdtfr::MOD_BII) {
          if (isNumberConstant(inst->op2)) {
            value = ((NumberConstant*)(inst->op2))->value;
            if ((pow = is2pow(value)) != -1) {
              /* 插入右移指令 */
              BinaryInst* rInst = new BinaryInst(BinaryInstIdtfr::ASHR_BII);
              rInst->op1 = inst->op1;
              rInst->op2 = new NumberConstant(pow, 32);
              rInst->result = new VirtRegValue(func->maxLlvmIrId, new IntegerType(32));
              func->maxLlvmIrId++;
              it = bblk->instructions.insert(it, rInst);
              it++;
              /* 插入左移指令 */
              BinaryInst* lInst = new BinaryInst(BinaryInstIdtfr::SHL_BII);
              lInst->op1 = rInst->result;
              lInst->op2 = rInst->op2;
              lInst->result = new VirtRegValue(func->maxLlvmIrId, new IntegerType(32));
              func->maxLlvmIrId++;
              it = bblk->instructions.insert(it, lInst);
              it++;
              /* 插入减法指令 */
              inst->binaryInstType = BinaryInstIdtfr::SUB_BII;
              inst->op2 = lInst->result;
              
            }
          }
        }
      }
    }
  }
}

void dullMulDiv(Module* module)
{
  for (Function* func : module->funcDef) {
    funcDullMulDiv(func);
  }
}