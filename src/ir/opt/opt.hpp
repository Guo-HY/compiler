#ifndef _OPT_H
#define _OPT_H 1

#include <vector>
#include <unordered_map>
#include <utility>
#include <map>
#include <set>
#include <list>
#include <stdio.h>
#include "../../include/error.hpp"
#include "../ir.hpp"
#include "../ir_utils.hpp"

class FunctionOptMsg;
class BasicBlockOptMsg;

void rmblk(Module* module);
void mem2reg(Module* module);
void deadCodeElim(Module* module);
void globalConstantPropagation(Module* module);

inline void optir(Module* module)
{
  std::string s;
  FILE* fp = fopen("base.ll", "w");
  s = module->toString();
  fprintf(fp, "%s", s.c_str());
  fclose(fp);

  rmblk(module);
  Log("after rmblk");
  s = module->toString();
  fp = fopen("rmblk.ll", "w");
  fprintf(fp, "%s", s.c_str());
  fclose(fp);

  mem2reg(module);
  Log("after mem2reg");
  s = module->toString();
  fp = fopen("mem2reg.ll", "w");
  fprintf(fp, "%s", s.c_str());
  fclose(fp);
  
  globalConstantPropagation(module);
  Log("after globalConstantPropagation");
  s = module->toString();
  fp = fopen("gcp.ll", "w");
  fprintf(fp, "%s", s.c_str());
  fclose(fp);

  deadCodeElim(module);
  Log("after deadCodeElim");
  s = module->toString();
  fp = fopen("dce.ll", "w");
  fprintf(fp, "%s", s.c_str());
  fclose(fp);
}

class FunctionOptMsg {
  public:
  /* 对应函数 */
  Function* func;
  /* 入口基本块id */
  int entryBblkId;
  /* 邻接表存储cfg图，key：基本块编号， value：边指向的基本块编号数组 */
  std::unordered_map<int, std::vector<int>*> cfgGraph;
  /* key:基本块编号， value: 基本块指针 */
  std::unordered_map<int, BasicBlockOptMsg*> id2bblk;
  /* key:局部int变量地址虚拟寄存器号, value:定义该变量的块列表 */
  std::unordered_map<int, std::set<BasicBlockOptMsg*>*> varAddrRegId2defBblk;

  FunctionOptMsg(Function* f) : func(f), entryBblkId(-1) {}

  void addCfgEdge(int from, std::vector<int> to) {
    if (cfgGraph.count(from) == 0) {
      cfgGraph[from] = new std::vector<int>;
    }
    std::vector<int>* edges = cfgGraph[from];
    edges->insert(edges->end(), to.begin(), to.end());
  }
  /* mem2reg */
  void bfsWithoutBblk(std::set<int>* visited, int excludeBblkId);

};

class BasicBlockOptMsg {
  public:
  BasicBlock* bblk;
  FunctionOptMsg* func; /* 所属function */

  std::unordered_map<int, BasicBlockOptMsg*> strictlyDominated; /* 该基本块严格支配的基本块 */
  std::unordered_map<int, BasicBlockOptMsg*> dominaterStrictly; /* 严格支配该基本块的基本块 */
  BasicBlockOptMsg* idom; /* 该基本块的直接支配者 */
  std::vector<BasicBlockOptMsg*> idomds; /* 该基本块直接支配的节点 */
  std::unordered_map<int, BasicBlockOptMsg*> DF;    /* 该基本块支配边界 */
  std::vector<PhiInst*> phiInsts; /* 该基本块需要插入的phi指令 */
  /* 局部int变量的到达定义,key为变量的地址寄存器号，value为变量最新值 */
  std::unordered_map<int, Value*> reachdef; 

  BasicBlockOptMsg(BasicBlock* b, FunctionOptMsg* f) : bblk(b), func(f), idom(NULL) {}
  
  void addPhiInst(int vAddrId);

  Value* getReachDefValue(int vAddrId) {
    if (this->reachdef.count(vAddrId) != 0) {
      return this->reachdef[vAddrId];
    }
    if (this->idom == NULL) {
      /* 未找到定义代表在某一路径上变量未定义，此时直接返回0值 */
      return new NumberConstant(0, 32);
    }
    return this->idom->getReachDefValue(vAddrId);
  }

};



#endif