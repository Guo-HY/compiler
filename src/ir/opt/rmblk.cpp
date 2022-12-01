#include "opt.hpp"
/* 删除不会跳转到的基本块,需要迭代至不动点 */
/* TODO : 删除多余的基本块 */

void funcRmblk(Function* func)
{
  int entryBblkId = func->basicBlocks[0]->getId();
  std::set<int> jumpedBblk;
  bool flag = false;
  do
  {
    flag = false;
    jumpedBblk.clear();
    for (BasicBlock* bblk : func->basicBlocks) {
      for (Instruction* inst : bblk->instructions) {
        if (isBrInst(inst)) {
          BrInst* brInst = (BrInst*)inst;
          if (brInst->isUnCond) {
            jumpedBblk.insert(((LabelValue*)brInst->dest)->id);
          } else {
            jumpedBblk.insert(((LabelValue*)brInst->iffalse)->id);
            jumpedBblk.insert(((LabelValue*)brInst->iftrue)->id);
          }
        }
      }
    }

    for (std::vector<BasicBlock*>::iterator it = func->basicBlocks.begin(); 
      it != func->basicBlocks.end(); ) {
      int bblkId = (*it)->getId();
      if (bblkId != entryBblkId && jumpedBblk.count(bblkId) == 0) {
        it = func->basicBlocks.erase(it);
        flag = true;
      } else {
        it++;
      }
    }
  } while (flag == true);
}


void rmblk(Module* module)
{
  for (Function* func : module->funcDef) {
    funcRmblk(func);
  }
}