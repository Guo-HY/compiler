#ifndef _ASM_OPT_H 
#define _ASM_OPT_H 1

#include "../asm.hpp"
#include "../../include/error.hpp"
#include <set>
#include <map>
#include <unordered_map>

class AsmFuncOptMsg;
class AsmBlockOptMsg;

void graphRegAllocator(AsmModule* module);


class AsmFuncOptMsg {
  public:
  AsmFunction* asmFunction;
  int entryBblkId;  /* 入口基本块编号 */
  /* key:基本块编号， value: 基本块指针 */
  std::unordered_map<int, AsmBlockOptMsg*> id2bblk;
  /* 邻接表存储cfg图，key：基本块编号， value：边指向的基本块编号数组 */
  std::unordered_map<int, std::vector<int>*> cfgGraph;
    /* 虚拟寄存器id到寄存器 */
  std::unordered_map<int, AsmReg*> virtId2reg;
  /* LR并查集，下标是虚拟寄存器号，值是LR编号或父节点，不可以直接通过id获取LR */
  std::unordered_map<int,int> virtregId2LR;
  /* 全局活动范围 ： 活动范围到虚拟寄存器号集合 */
  std::unordered_map<int, std::vector<int>*> LR2virtRegId;
  /* 活动范围的冲突图，存放的都是LR编号 */
  std::unordered_map<int, std::set<int>*> conflictGraph;
  /* LR到分配的物理寄存器名，"not"代表不分配物理寄存器 */
  std::unordered_map<int, std::string> LR2physRegName;
  int spillVirtRegNum;

  AsmFuncOptMsg(AsmFunction* f) : asmFunction(f), spillVirtRegNum(0) {}

  void addCfgEdge(int from, std::vector<int> to) {
    if (cfgGraph.count(from) == 0) {
      cfgGraph[from] = new std::vector<int>;
    }
    std::vector<int>* edges = cfgGraph[from];
    edges->insert(edges->end(), to.begin(), to.end());
  }

  int findLR(int virtRegId) {
    if (virtregId2LR.count(virtRegId) == 0) {
      panic("error");
    }
    if (virtregId2LR[virtRegId] == virtRegId) {
      return virtRegId;
    }
    virtregId2LR[virtRegId] = this->findLR(virtregId2LR[virtRegId]);
    return virtregId2LR[virtRegId];
  }

  void unionLR(int i, int j) {
    if (virtregId2LR.count(i) * virtregId2LR.count(j) == 0) {
      panic("error");
    }
    virtregId2LR[findLR(i)] = findLR(j);
  } 

  void addConflictEdge(int i, int j) {
    if (conflictGraph.count(i) == 0) {
      conflictGraph[i] = new std::set<int>;
    }
    if (conflictGraph.count(j) == 0) {
      conflictGraph[j] = new std::set<int>;
    }
    if (i == j) {
      return;
    }
    conflictGraph[i]->insert(j);
    conflictGraph[j]->insert(i);
  }

};


class AsmBlockOptMsg {
  public:
  AsmBasicBlock* asmBasicBlock;
  AsmFunction* asmFunction;
  /* 基本块中定义前使用的所有寄存器 */
  std::unordered_map<int, AsmReg*> useVar; 
  /* 基本块中定义的所有寄存器,也可能已经被使用过 */
  std::unordered_map<int, AsmReg*> killVar;
  std::unordered_map<int, AsmReg*> liveOut;
  /* 活动范围不超过基本块的寄存器 */
  std::unordered_map<int, AsmReg*> bblkRangeVar;


  AsmBlockOptMsg(AsmBasicBlock* b, AsmFunction* f) : asmBasicBlock(b), asmFunction(f) {}

};



#endif