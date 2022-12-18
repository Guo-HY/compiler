#include "opt.hpp"

/* 删除不会跳转到的基本块,需要迭代至不动点 */
void funcRmIsolateblk(Function* func)
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

std::unordered_map<int, std::set<LabelValue*>*> labelId2Values;

void insertLabelId2Values(LabelValue* l)
{
  if (labelId2Values.count(l->id) == 0) {
    labelId2Values[l->id] = new std::set<LabelValue*>;
  }
  labelId2Values[l->id]->insert(l);
  if (labelId2Values[l->id]->size() > 1) {
    panic("error");
  }
}

/* 若是空块则返回跳转到的LabelValue，否则返回NULL */
LabelValue* isEmptyBblk(BasicBlock* bblk)
{
  if (bblk->instructions.size() == 0) {
    panic("error");
  }
  if (bblk->instructions.size() > 1) {
    return NULL;
  }
  if (isBrInst(*(bblk->instructions.begin()))) {
    BrInst* inst = (BrInst*)*(bblk->instructions.begin());
    if (!inst->isUnCond) {
      panic("error");
    }
    return (LabelValue*)inst->dest;
  }
  if (isRetInst(*(bblk->instructions.begin()))) {
    return NULL;
  }
  panic("error");
  return NULL;
}

/* 删除空的（只有无条件跳转指令）的基本块 */
void funcRmEmptyBlock(Function* func)
{
  /* 首先建立labelId到label对象的映射
    这里看起来可以处理对象不一致的情况
   */
  labelId2Values.clear();
  for (BasicBlock* bblk : func->basicBlocks) {
    insertLabelId2Values(bblk->label);
    for (Instruction* inst : bblk->instructions) {
      if (isBrInst(inst)) {
        BrInst* brInst = ((BrInst*)inst);
        if (brInst->isUnCond) {
          insertLabelId2Values((LabelValue*)brInst->dest);
        } else {
          insertLabelId2Values((LabelValue*)brInst->iffalse);
          insertLabelId2Values((LabelValue*)brInst->iftrue);
        }
      }
    }
  }
  LabelValue* emptyBblkJumpTo = NULL;
  /* 删除空基本块 */
  bool change = true;
  while (change) {
    change = false;
    for (auto it = func->basicBlocks.begin() + 1; it != func->basicBlocks.end(); ) {
      if ((emptyBblkJumpTo = isEmptyBblk(*it)) != NULL && (*it)->getId()) {
        int jumpToId = emptyBblkJumpTo->id;
        int bblkId = (*it)->getId();
        /* 将所有跳转到bblkId替换为跳转到jumpToId */
        for (LabelValue* l : *labelId2Values[bblkId]) {
          l->id = jumpToId;
          (*labelId2Values[jumpToId]).insert(l);
        }
        labelId2Values.erase(bblkId);
        it = func->basicBlocks.erase(it);
        change = true;
      } else {
        it++;
      }
    }
  }
  /* 没有保持对象唯一性 */
}


void rmblk(Module* module)
{
  for (Function* func : module->funcDef) {
    funcRmIsolateblk(func);
    funcRmEmptyBlock(func);
  }


}