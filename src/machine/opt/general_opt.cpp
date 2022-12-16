// #include "asm_opt.hpp"
// extern std::unordered_map<std::string, int> name2regPhysNum;
// extern std::unordered_map<std::string, AsmReg*> name2PhysAsmReg;


// /* 这里假设跳转指令只有jr，bne，j,且jr没有地址 */
// static void getBrInstPointed(AsmInst* inst, std::vector<int>* success)
// { 
//   if (inst->asmInstIdtfr == AsmInstIdtfr::J_AII) {
//     success->push_back(((AsmLabel*)(inst->ops[0].first))->bblkLabelId);
//   } else if (inst->asmInstIdtfr == AsmInstIdtfr::BNE_AII) {
//     success->push_back(((AsmLabel*)(inst->ops[2].first))->bblkLabelId);
//   }
// }

// /* 求函数cfg */
// static void calFunctionCfg(AsmFuncOptMsg* funcMsg)
// { 
//   AsmFunction* func = funcMsg->asmFunction;
//   for (u_long i = 0; i < func->basicBlocks.size(); i++) {
//     AsmBasicBlock* bblk = func->basicBlocks[i];
//     funcMsg->id2bblk[bblk->getBblkId()] = new AsmBlockOptMsg(bblk, func);
//     if (i == 0) {
//       funcMsg->entryBblkId = bblk->getBblkId();
//     }
//     /* 求bblk的后继，注意bblk的最后一条或两条指令可能是终结指令 */
//     std::vector<int> success;
//     /* 直接遍历基本块 */
//     for (AsmInst* inst : bblk->insts) {
//       getBrInstPointed(inst, &success);
//     }
//     funcMsg->addCfgEdge(bblk->getBblkId(), success);  
//   }
// }

// /* 目前只有BNE, J, JR */
// static bool isTermInst(AsmInst* inst)
// {
//   if (inst->asmInstIdtfr == AsmInstIdtfr::BNE_AII || 
//     inst->asmInstIdtfr == AsmInstIdtfr::J_AII ||
//     inst->asmInstIdtfr == AsmInstIdtfr::JR_AII) {
//       return true;
//     }
//   return false;
// }

// static AsmReg* blockAddImm2RegInst(AsmFuncOptMsg* funcMsg, 
//   AsmImm* imm, AsmLabel* label)
// {
//   int newRegId = funcMsg->asmFunction->maxVirtRegId;
//   (funcMsg->asmFunction->maxVirtRegId)++;
//   AsmReg* newReg = new AsmReg(false, newRegId);
//   AsmInst* addInst = new AsmInst(AsmInstIdtfr::ADDIU_AII, 
//       {WRR(newReg, name2PhysAsmReg["zero"], imm)});
  
//   int bblkId = label->bblkLabelId;
//   AsmBlockOptMsg* bblkMsg = funcMsg->id2bblk[bblkId];
//   std::list<AsmInst*>::iterator it;
//   for (it = bblkMsg->asmBasicBlock->insts.begin(); 
//     it != bblkMsg->asmBasicBlock->insts.end(); it++) {
//     if (isTermInst(*it)) {
//       bblkMsg->asmBasicBlock->insts.insert(it, addInst);
//       break;
//     }
//   }
//   return newReg;
// }

// /* 将phi参数中的常量转变为前导块中的reg */
// static void phiImm2Reg(AsmFuncOptMsg* funcMsg)
// {
//   AsmFunction* func = funcMsg->asmFunction;
//   for (AsmBasicBlock* bblk : func->basicBlocks) {
//     for (AsmInst* inst : bblk->insts) {
//       if (inst->asmInstIdtfr == AsmInstIdtfr::PHI_AII) {
//         AsmPhiInst* phiInst = (AsmPhiInst*)inst;
//         for (u_long i = 0; i < phiInst->varDefs.size(); i++) {
//           std::pair<AsmOperand*, AsmLabel*> it = phiInst->varDefs[i];
//           if (it.first->asmOperandIdtfr == AsmOperandIdtfr::IMM_AOI) {
//             AsmReg* newReg = blockAddImm2RegInst(funcMsg, (AsmImm*)it.first, it.second);
//             phiInst->varDefs[i].first = newReg;
//             phiInst->ops[i+1].first = newReg;
//           }
//         }
//       }
//     }
//   }
// }

// /* 求解函数中每个基本块的useVar和killVar */
// /* 只计算虚拟寄存器 */
// static void calUseKill(AsmFuncOptMsg* funcMsg)
// {
//   AsmFunction* func = funcMsg->asmFunction;
//   for (std::pair<int, AsmBlockOptMsg*> id2bblk : funcMsg->id2bblk) {
//     AsmBlockOptMsg* bblkMsg = id2bblk.second;
//     AsmBasicBlock* bblk = bblkMsg->asmBasicBlock;
//     for (AsmInst* inst : bblk->insts) {
//       for (std::pair<AsmOperand*, RWP> op : inst->ops) {
//         if (op.first->asmOperandIdtfr != AsmOperandIdtfr::REG_AOI) {
//           continue;
//         }
//         int regId;
//         AsmReg* reg = ((AsmReg*)op.first);
//         if (reg->isPhysReg) {
//           continue;
//         }
//         regId = reg->virtNumber;
//         if (op.second == RWP::READ_RWP && bblkMsg->killVar.count(regId) == 0) {
//           bblkMsg->useVar[regId] = reg;
//         }
//         if (op.second == RWP::WRITE_RWP) {
//           bblkMsg->killVar[regId] = reg;
//         }
//       }
//     }
//   }
// }

// static void calLiveOut(AsmFuncOptMsg* funcMsg)
// {
//   bool change = true;
//   AsmFunction* func = funcMsg->asmFunction;
//   while (change) {
//     change = false;
//     for (std::pair<int, AsmBlockOptMsg*> id2bblk : funcMsg->id2bblk) {
//       AsmBlockOptMsg* bblkMsg = id2bblk.second;
//       AsmBasicBlock* bblk = bblkMsg->asmBasicBlock;
//       /* 对当前基本块的每个后继 */
//       std::vector<int>* success = funcMsg->cfgGraph[bblk->blockLabel->bblkLabelId];
//       for (int successId : *success) {
//         AsmBlockOptMsg* successMsg = funcMsg->id2bblk[successId];
//         /* 添加use */
//         for (std::pair<int, AsmReg*> it : successMsg->useVar) {
//           if (bblkMsg->liveOut.count(it.first) == 0) {
//             bblkMsg->liveOut[it.first] = it.second;
//             change = true;
//           }
//         }
//         /* 添加liveout - kill */
//         for (std::pair<int, AsmReg*> it : successMsg->liveOut) {
//           if (successMsg->killVar.count(it.first) == 0 && 
//             bblkMsg->liveOut.count(it.first) == 0) {
//             bblkMsg->liveOut[it.first] = it.second;
//             change = true;
//           }
//         }
//       }
//     }
//   }
// }



// /* 找到全局活动范围 : 对所有虚拟寄存器进行 */
// static void calLR(AsmFuncOptMsg* funcMsg)
// {
//   AsmFunction* func = funcMsg->asmFunction;
//   for (AsmBasicBlock* bblk : func->basicBlocks) {
//     for (AsmInst* inst : bblk->insts) {
//       if (inst->asmInstIdtfr != AsmInstIdtfr::PHI_AII) {
//         for (std::pair<AsmOperand*, RWP>op : inst->ops) {
          
//         }
//       }
      
//     }
//   }



// }

