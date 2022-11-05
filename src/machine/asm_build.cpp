#include "asm_build.hpp"
#include "../include/error.hpp"

std::unordered_map<std::string, int> name2regPhysNum {
  {"zero", 0},{"at", 1}, {"v0", 2}, {"v1", 3}, {"a0", 4}, {"a1", 5}, {"a2", 6},
  {"a3", 7},  {"t0", 8},  {"t1", 9},  {"t2", 10}, {"t3", 11}, {"t4", 12}, {"t5", 13},
  {"t6", 14}, {"t7", 15}, {"s0", 16}, {"s1", 17}, {"s2", 18}, {"s3", 19}, {"s4", 20},
  {"s5", 21}, {"s6", 22}, {"s7", 23}, {"t8", 24}, {"t9", 25}, {"k0", 26}, {"k1", 27},
  {"gp", 28}, {"sp", 29}, {"fp", 30}, {"ra", 31},
};

/* 保存物理寄存器名到对应AsmReg */
std::unordered_map<std::string, AsmReg*> name2PhysAsmReg;
/* 存放输入输出函数需要传给syscall的v0值 */
static std::unordered_map<std::string, int> ioFunc2v0 {
  {"putint", 1},
  {"putstr", 4},
  {"getint", 5},
};

/* 存放局部变量和函数参数相对sp指针的偏移（用局部变量对应的虚拟寄存器号索引），字节为单位 */
static std::unordered_map<int, AsmImm*> virtRegId2stackOffset; 
/* 暂存函数参数相对于sp指针的偏移（用函数参数对应的虚拟寄存器号索引），字节为单位 */
static std::unordered_map<int, AsmImm*> funcArgsId2stackOffset;
/* 当前函数栈大小，first是负数，second是正数，字节为单位 */
static std::pair<AsmImm*, AsmImm*> frameSize;

/* 当前处理的函数 */
static AsmFunction* nowFunction;
/* 当前待分配虚拟寄存器id */
static int nowVirtAsmId;

static AsmBasicBlock* nowblk;
/* ------------------------------------- tools ------------------------------------- */
/* 返回type类型对应的大小（多少个字）该函数只能用于int或int数组求值 */

void initName2PhysAsmReg()
{
  for (auto iter = name2regPhysNum.begin(); iter != name2regPhysNum.end(); iter++) {
    name2PhysAsmReg[iter->first] = new AsmReg(true, iter->second);
  }
}

/* 获取type对应的大小（字为单位）*/
int getTypeWordSize(Type* t)
{
  if (t->typeIdtfr ==TypeIdtfr::POINTER_TI || t->typeIdtfr == TypeIdtfr::INTEGER_TI) {
    return 1;
  }
  if (t->typeIdtfr == TypeIdtfr::ARRAY_TI) {
    ArrayType* at = (ArrayType*)t;
    return at->elemNums * getTypeWordSize(at->elemType);
  }
  panic("error");
  return -1;
}

int allocVirtAsmId()
{
  int tmp = nowVirtAsmId;
  nowVirtAsmId++;
  return tmp;
}

/* 分配虚拟寄存器 */
AsmReg* allocVirtAsmReg()
{
  return new AsmReg(false, allocVirtAsmId());
}

AsmLabel* allocAsmLabel(std::string name)
{
  return new AsmLabel(name);
}

AsmImm* allocAsmImm(int imm)
{
  return new AsmImm(imm);
}

void blockAddInst(AsmInstIdtfr idtfr, std::initializer_list<std::pair<AsmOperand*, RWP>> ops)
{
  AsmInst* inst = new AsmInst(idtfr, ops);
  nowblk->addInst(inst);
}

std::string getGlobalValueName(Value* v)
{
  if (v->valueIdtfr != ValueIdtfr::GLOBAL_VAR_VI) {
    panic("error");
  }
  return ((GlobalValue*)v)->name;
}

/* 判断一个虚拟寄存器是否为函数形参 */
bool isFuncFArg(Value* v)
{
   return isVirtRegValue(v) && 
    funcArgsId2stackOffset.count(((VirtRegValue*)v)->getId()) != 0;
}
/* 分配之后会改变的立即数（函数参数偏移）,需要传入函数参数虚拟寄存器 */
AsmImm* allocfuncArgOffset(Value* v)
{
  if (!isVirtRegValue(v)) {
    panic("error");
  }
  int id = ((VirtRegValue*)v)->getId();
  if (funcArgsId2stackOffset.count(id) == 0 ){
    panic("error");
  }
  return funcArgsId2stackOffset[id];
}

/* ---------------------------------------- *2asm ---------------------------------------- */

/* 将VirRegValue 或者 NumberConstantValue转为AsmReg对象 */
AsmReg* value2asmReg(Value* value)
{
  /* 如果是常量，需要用addiu转为寄存器类型，但应该尽量减少这样做的次数 */
  if (value->valueIdtfr == ValueIdtfr::NUMBER_CONSTANT_VI) {
    AsmImm* imm = allocAsmImm(((NumberConstant*)value)->value);
    AsmReg* rt = allocVirtAsmReg();
    AsmReg* rs = name2PhysAsmReg["zero"];
    blockAddInst(AsmInstIdtfr::ADDIU_AII, {WRR(rt,rs,imm)});
    return rt;
  }
  /* 如果是虚拟寄存器 */
  if (value->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    AsmReg* reg = new AsmReg(false, ((VirtRegValue*)value)->getId());
    return reg;
  }
  panic("error");
  return NULL;
}

/* 解析全局int数组或int初始化 */
void globalInitValue2asmData(GlobalInitValue* globalInitValue, std::vector<int>* datas)
{
  if (globalInitValue->isValue) {
    if (globalInitValue->value->valueIdtfr != ValueIdtfr::NUMBER_CONSTANT_VI) {
      panic("error");
    }
    datas->push_back(((NumberConstant*)(globalInitValue->value))->value);
    return;
  }
  for (u_long i = 0; i < globalInitValue->globalInitValues.size(); i++) {
    globalInitValue2asmData(globalInitValue->globalInitValues[i], datas);
  }
}

AsmGlobalData* globalValue2asm(GlobalValue* globalValue)
{
  AsmLabel* asmLabel = allocAsmLabel(globalValue->name);
  AsmGlobalData* asmGlobalData;
  if (globalValue->isconstString) {
    if (globalValue->globalInitValue == NULL || 
      globalValue->globalInitValue->value->valueIdtfr != ValueIdtfr::STRING_CONSTANT_VI) {
      panic("error");
    }
    asmGlobalData = new AsmStrData(asmLabel,((StringConstant*)(globalValue->globalInitValue->value))->str);
    return asmGlobalData;
  }
  int wordNum = getTypeWordSize(globalValue->type);
  AsmWordData* asmWordData = new AsmWordData(asmLabel, wordNum);
  if (globalValue->globalInitValue == NULL) {
    asmWordData->hasInitValue = false;
    return asmWordData;
  }
  asmWordData->hasInitValue = true;
  globalInitValue2asmData(globalValue->globalInitValue, &(asmWordData->wordsInitValue));
  return asmWordData;
}

AsmModule* module2asm(Module* lmodule)
{
  initName2PhysAsmReg();
  AsmModule* asmModule = new AsmModule();
  for (u_long i = 0; i < lmodule->globalVariables.size(); i++) {
     asmModule->addGlobalData(globalValue2asm(lmodule->globalVariables[i]));
  }
  for (u_long i = 0; i < lmodule->funcDef.size(); i++) {
    asmModule->addFunction(function2asm(lmodule->funcDef[i]));
  }
  return asmModule;
}

/* 计算函数调用需要传递的参数总大小（字为单位）*/
int funcCallArgsWordSize(CallInst* callInst)
{
  return callInst->args.size();/* 因为函数参数肯定都是4字节大小 */
}

AsmFunction* function2asm(Function* function)
{
  virtRegId2stackOffset.clear();
  funcArgsId2stackOffset.clear();
  nowVirtAsmId = function->maxLlvmIrId + 1;
  Log("nowVirtAsmId = %d", nowVirtAsmId);
  nowFunction = new AsmFunction(allocAsmLabel(function->funcName));
  nowblk = new AsmBasicBlock(allocAsmLabel(nowFunction->funcName->label + "." + 
                      std::to_string(function->basicBlocks[0]->label->id)));

  /* 提前遍历一遍function并生成函数参数与局部变量相对于sp的偏移，以及栈大小,
    注意此时栈大小并没有考虑saved reg以及tmp变量 */
    /* 注意：由于栈大小之后可能会变化，因此函数参数所在位置相对sp的偏移可能会变化,这里没有考虑 */
    /* 调用函数时，自左向右自底向上压栈 */
  int stackWordSize = 1; /* 栈大小，初始化 ra 需要一个字 */ 
  int spOffsetBytes = 0; /* 相对栈指针的偏移量 */
  int maxFuncCallArgsWordSize = 0;  /* 最大函数参数栈空间字个数 */
  for (u_long i = 0; i < function->basicBlocks.size(); i++) {
    BasicBlock* basicBlock = function->basicBlocks[i];
    for (u_long j = 0; j < basicBlock->instructions.size(); j++) {
      Instruction* instruction = basicBlock->instructions[j];
      if (instruction->instType == InstIdtfr::CALL_II) {
        int tmp = funcCallArgsWordSize((CallInst*)instruction);
        maxFuncCallArgsWordSize = max(tmp, maxFuncCallArgsWordSize);
      } else if (instruction->instType == InstIdtfr::ALLOCA_II) {
        stackWordSize += getTypeWordSize(((AllocaInst*)instruction)->allocType);
      }
    }
  }
  maxFuncCallArgsWordSize = max(4, maxFuncCallArgsWordSize); /* 需要留足a0-a3的空间 */
  stackWordSize = stackWordSize + maxFuncCallArgsWordSize;
  nowFunction->stackWordSize = stackWordSize;
  nowFunction->funcCallArgsWordSize = maxFuncCallArgsWordSize;
  nowFunction->raWordOffset = maxFuncCallArgsWordSize;
  spOffsetBytes = maxFuncCallArgsWordSize * 4;

  frameSize.first = allocAsmImm((-stackWordSize) * 4);
  frameSize.second = allocAsmImm((stackWordSize) * 4);
  /* 开辟栈空间 */
  blockAddInst(AsmInstIdtfr::ADDIU_AII, {WRR(name2PhysAsmReg["sp"],name2PhysAsmReg["sp"],frameSize.first)});
  /* 将ra压栈，放在函数参数域上方 sw rt, imm(rs) */
  blockAddInst(AsmInstIdtfr::SW_AII, {RRR(name2PhysAsmReg["ra"],allocAsmImm(spOffsetBytes),name2PhysAsmReg["sp"])});
  spOffsetBytes += 4;
  /* 对局部变量在栈上分配空间 */
  for (u_long i = 0; i < function->basicBlocks.size(); i++) {
    BasicBlock* basicBlock = function->basicBlocks[i];
    for (u_long j = 0; j < basicBlock->instructions.size(); j++) {
      Instruction* instruction = basicBlock->instructions[j];
      if (instruction->instType == InstIdtfr::ALLOCA_II) {
        int virtRegId = ((AllocaInst*)instruction)->result->getId();
        virtRegId2stackOffset[virtRegId] = allocAsmImm(spOffsetBytes); /* 变量在栈中的起始地址相对栈指针的偏移 */
        spOffsetBytes += 4 * getTypeWordSize(((AllocaInst*)instruction)->allocType);
      }
    }
  }
  /* 计算函数参数的地址（相对栈指针的偏移） */
  int funcFParamOffsetBytes = stackWordSize * 4; /* 函数参数相对于栈指针的起始偏移 */
  for (u_long i = 0; i < function->funcFParamValues.size(); i++) {
    funcArgsId2stackOffset[function->funcFParamValues[i]->value->getId()] = allocAsmImm(funcFParamOffsetBytes);
    funcFParamOffsetBytes += 4;
  }

  for (u_long i = 0; i < function->basicBlocks.size(); i++) {
    BasicBlock* basicBlock = function->basicBlocks[i];
    for (u_long j = 0; j < basicBlock->instructions.size(); j++) {
      Instruction* instruction = basicBlock->instructions[j];
      switch (instruction->instType)
      {
      case InstIdtfr::ALLOCA_II:
        blockAddInst(AsmInstIdtfr::NOP_AII, {}); break;
      case InstIdtfr::BINARY_II:
        binaryInst2asm((BinaryInst*)instruction); break;
      case InstIdtfr::BR_II:
        brInst2asm((BrInst*)instruction); break;
      case InstIdtfr::CALL_II:
        callInst2asm((CallInst*)instruction); break;
      case InstIdtfr::GEP_II:
        gepInst2asm((GEPInst*)instruction); break;
      case InstIdtfr::ICMP_II:
        icmpInst2asm((IcmpInst*)instruction); break;
      case InstIdtfr::LOAD_II:
        loadInst2asm((LoadInst*)instruction); break;
      case InstIdtfr::RET_II:
        retInst2asm((RetInst*)instruction); break;
      case InstIdtfr::STORE_II:
        storeInst2asm((StoreInst*)instruction); break;
      case InstIdtfr::ZEXT_II:
        blockAddInst(AsmInstIdtfr::NOP_AII, {}); break;
      default:
        panic("error");
        break;
      }
    }
    nowFunction->addBasicBlock(nowblk);
    if (i < function->basicBlocks.size() - 1) {
      nowblk = new AsmBasicBlock(allocAsmLabel(nowFunction->funcName->label + "." + 
                    std::to_string(function->basicBlocks[i+1]->label->id)));
    }
  }
  nowFunction->frameSize = frameSize;
  nowFunction->funcArgsId2stackOffset = funcArgsId2stackOffset;
  nowFunction->virtRegId2stackOffset = virtRegId2stackOffset;
  return nowFunction;
}

void binaryInst2asm( BinaryInst* binaryInst)
{
  AsmInstIdtfr idtfr, idtfr2;
  switch (binaryInst->binaryInstType)
  {
  case BinaryInstIdtfr::ADD_BII:
    idtfr = AsmInstIdtfr::ADDU_AII;   goto lb1;
  case BinaryInstIdtfr::SUB_BII:
    idtfr = AsmInstIdtfr::SUBU_AII;   goto lb1;
  case BinaryInstIdtfr::AND_BII:
    idtfr = AsmInstIdtfr::AND_AII;    goto lb1;
  case BinaryInstIdtfr::OR_BII:
    idtfr = AsmInstIdtfr::OR_AII;     goto lb1;
    lb1:
    blockAddInst(idtfr, {WRR(value2asmReg(binaryInst->result),
      value2asmReg(binaryInst->op1),value2asmReg(binaryInst->op2))});
    break;
  case BinaryInstIdtfr::MUL_BII:
    idtfr = AsmInstIdtfr::MULT_AII;
    idtfr2 = AsmInstIdtfr::MFLO_AII;  goto lb2;
  case BinaryInstIdtfr::SDIV_BII:
    idtfr = AsmInstIdtfr::DIV_AII;
    idtfr2 = AsmInstIdtfr::MFLO_AII;  goto lb2;
  case BinaryInstIdtfr::MOD_BII:
    idtfr = AsmInstIdtfr::DIV_AII;
    idtfr2 = AsmInstIdtfr::MFHI_AII;  goto lb2;
    lb2:  
    blockAddInst(idtfr, {RR(value2asmReg(binaryInst->op1),value2asmReg(binaryInst->op2))});
    blockAddInst(idtfr2, {W(value2asmReg(binaryInst->result))});
    break;
  default:
    panic("error");
    break;
  }
}

AsmReg* loadInst2asm( LoadInst* loadInst)
{
  /* lw rt, imm(rs) */
  /* 对于全局变量不需要先la reg label（MARS特性）*/
  AsmReg* base;
  AsmReg* rt = value2asmReg(loadInst->result);
  AsmImm* offset0 = allocAsmImm(0);
  AsmImm* imm;
  if (isGlobalValue(loadInst->pointer)) {
    /* lw rt, label 注意label没有括号 */
    blockAddInst(AsmInstIdtfr::LW_AII, {WR(rt,allocAsmLabel(getGlobalValueName(loadInst->pointer)))});
  } else if (isVirtRegValue(loadInst->pointer) && 
    virtRegId2stackOffset.count(((VirtRegValue*)loadInst->pointer)->getId()) != 0) {
    base = name2PhysAsmReg["sp"];
    imm = virtRegId2stackOffset[((VirtRegValue*)loadInst->pointer)->getId()];
    blockAddInst(AsmInstIdtfr::LW_AII, {WRR(rt,imm,base)});
  } else {
    base = value2asmReg(loadInst->pointer);
    blockAddInst(AsmInstIdtfr::LW_AII, {WRR(rt,offset0,base)});
  }
  return rt;
}

void storeInst2asm(StoreInst* storeInst)
{
  /* sw rt, imm(rs) */
  /* 对于全局变量不需要先la reg label（MARS特性）*/
  AsmReg* base;
  AsmReg* rt = value2asmReg(storeInst->value);
  AsmImm* offset0 = allocAsmImm(0);
  AsmImm* imm;
  /* 如果是将函数参数挪到内存的store指令，（先load再store，假定所有函数参数均内存传递） */
  /* lw rt, imm(base) */
  if (isFuncFArg(storeInst->value)) {
    blockAddInst(AsmInstIdtfr::LW_AII, 
    {WRR(rt,allocfuncArgOffset(storeInst->value), 
    name2PhysAsmReg["sp"])});
  }
  if (isGlobalValue(storeInst->pointer)) {
    blockAddInst(AsmInstIdtfr::SW_AII, {RR(rt,allocAsmLabel(getGlobalValueName(storeInst->pointer)))});
  } else if (isVirtRegValue(storeInst->pointer) && 
    virtRegId2stackOffset.count(((VirtRegValue*)storeInst->pointer)->getId()) != 0) {
    base = name2PhysAsmReg["sp"];
    imm = virtRegId2stackOffset[((VirtRegValue*)storeInst->pointer)->getId()];
    blockAddInst(AsmInstIdtfr::SW_AII, {RRR(rt,imm,base)});
  } else {
    base = value2asmReg(storeInst->pointer);
    blockAddInst(AsmInstIdtfr::SW_AII, {RRR(rt,offset0, base)});
  }
}

AsmReg* icmpInst2asm( IcmpInst* icmpInst)
{
  AsmReg* result = value2asmReg(icmpInst->result);
  AsmReg* op1 = value2asmReg(icmpInst->op1);
  AsmReg* op2 = value2asmReg(icmpInst->op2);
  AsmImm* imm1 = allocAsmImm(1);
  AsmReg* zero = name2PhysAsmReg["zero"];
  switch (icmpInst->cond)
  {
  case ICMPCASE::EQ_ICMPCASE:   
    blockAddInst(AsmInstIdtfr::XOR_AII, {WRR(result, op2, op1)});
    blockAddInst(AsmInstIdtfr::SLTIU_AII, {WRR(result, result, imm1)});
    break;
  case ICMPCASE::NE_ICMPCASE:
    blockAddInst(AsmInstIdtfr::XOR_AII, {WRR(result, op2, op1)});
    blockAddInst(AsmInstIdtfr::SLTU_AII, {WRR(result, zero, result)});
    break;
  case ICMPCASE::UGT_ICMPCASE:
    blockAddInst(AsmInstIdtfr::SLTU_AII, {WRR(result, op2, op1)});
    break;
  case ICMPCASE::UGE_ICMPCASE:
    blockAddInst(AsmInstIdtfr::SLTU_AII, {WRR(result, op1, op2)});
    blockAddInst(AsmInstIdtfr::SLTIU_AII, {WRR(result, result, imm1)});
    break;
  case ICMPCASE::ULT_ICMPCASE:
    blockAddInst(AsmInstIdtfr::SLTU_AII, {WRR(result, op1, op2)});
    break;
  case ICMPCASE::ULE_ICMPCASE:
    blockAddInst(AsmInstIdtfr::SLTU_AII, {WRR(result, op2, op1)});
    blockAddInst(AsmInstIdtfr::SLTIU_AII, {WRR(result, result , imm1)});
    break;
  case ICMPCASE::SGT_ICMPCASE:
    blockAddInst(AsmInstIdtfr::SLT_AII, {WRR(result, op2, op1)});
    break;
  case ICMPCASE::SGE_ICMPCASE:
    blockAddInst(AsmInstIdtfr::SLT_AII, {WRR(result, op1, op2)});
    blockAddInst(AsmInstIdtfr::SLTIU_AII, {WRR(result, result, imm1)});
    break;
  case ICMPCASE::SLT_ICMPCASE:
    blockAddInst(AsmInstIdtfr::SLT_AII, {WRR(result, op1, op2)});
    break;
  case ICMPCASE::SLE_ICMPCASE:
    blockAddInst(AsmInstIdtfr::SLT_AII, {WRR(result, op2, op1)});
    blockAddInst(AsmInstIdtfr::SLTIU_AII, {WRR(result, result, imm1)});
    break;
  default:
    panic("error");
    break;
  }
  return result;
}

void brInst2asm( BrInst* brInst)
{
  AsmLabel* trueLabel = allocAsmLabel(nowFunction->funcName->label + "." +
             std::to_string(((LabelValue*)(brInst->iftrue))->id));
  if (brInst->isUnCond) {
    blockAddInst(AsmInstIdtfr::J_AII, {R(trueLabel)});
    return;
  }
  AsmLabel* falseLabel = allocAsmLabel(nowFunction->funcName->label + "." +
              std::to_string(((LabelValue*)(brInst->iffalse))->id));
  AsmReg* cond = value2asmReg(brInst->cond);
  AsmReg* zero = name2PhysAsmReg["zero"];
  blockAddInst(AsmInstIdtfr::BNE_AII, {RRR(cond, zero, trueLabel)});
  blockAddInst(AsmInstIdtfr::J_AII, {R(falseLabel)});
}

void gepInst2asm( GEPInst* gepInst)
{
  /* 如果是ptrval全局变量，需要用la获取地址 */ 
  AsmReg* baseAddr; /* 基地址寄存器 */
  AsmReg* result = value2asmReg(gepInst->result); /* 最后得到的地址存在这个寄存器里 */
  /* 将baseAddr赋给result */
  /* 如果是全局变量需要先la到result中 */
  if (isGlobalValue(gepInst->ptrval)) {
    blockAddInst(AsmInstIdtfr::LA_AII, {WR(result, allocAsmLabel(getGlobalValueName(gepInst->ptrval)))});
  } else {
    /* 否则将baseAddr挪到result中 */
    baseAddr = value2asmReg(gepInst->ptrval);
    blockAddInst(AsmInstIdtfr::ADDU_AII, {WRR(result, baseAddr, name2PhysAsmReg["zero"])});
  }

  Type* nowType = gepInst->elemType;
  AsmReg* nowOffset;
  AsmReg* typeSizeReg = allocVirtAsmReg();
  for (u_long i = 0; i < gepInst->indexs.size(); i++) {
    /* 获取类型大小 */
    int typeSizeByte = getTypeWordSize(nowType) * 4;
    /* 将类型大小存入typeSizeReg寄存器 */
    blockAddInst(AsmInstIdtfr::ADDIU_AII, {WRR(typeSizeReg, name2PhysAsmReg["zero"], allocAsmImm(typeSizeByte))});
    /* 获取当前类型偏移的寄存器nowOffset */
    nowOffset = value2asmReg(gepInst->indexs[i]);
    /* 类型大小*类型偏移，结果存在typeSizeReg中 */
    blockAddInst(AsmInstIdtfr::MULT_AII, {RR(typeSizeReg, nowOffset)});
    blockAddInst(AsmInstIdtfr::MFLO_AII, {W(typeSizeReg)});
    /* 将result加上typeSizeReg */
    blockAddInst(AsmInstIdtfr::ADDU_AII, {WRR(result, result, typeSizeReg)});
    /* 将类型剥离一层 */
    if (i < gepInst->indexs.size() - 1) {
      nowType = getArrayElemType(nowType);
    }
  }
}

void callInst2asm( CallInst* callInst)
{
  /* 处理io函数 */
  if (ioFunc2v0.count(callInst->name) != 0) {
    blockAddInst(AsmInstIdtfr::ADDIU_AII, {WRR(name2PhysAsmReg["v0"], 
                  name2PhysAsmReg["zero"], allocAsmImm(ioFunc2v0[callInst->name]))});
    if (callInst->name == "putint") { /* 默认a0没有用到 */
      blockAddInst(AsmInstIdtfr::ADDU_AII, {WRR(name2PhysAsmReg["a0"], 
          name2PhysAsmReg["zero"], value2asmReg(callInst->args[0]))});
    } else if (callInst->name == "putstr") {
      blockAddInst(AsmInstIdtfr::ADDU_AII, {WRR(name2PhysAsmReg["a0"], 
      name2PhysAsmReg["zero"], value2asmReg(callInst->args[0]))});
      // blockAddInst(AsmInstIdtfr::LA_AII, {WR(name2PhysAsmReg["a0"], 
      //   allocAsmLabel(getGlobalValueName(callInst->args[0])))});
    }
    blockAddInst(AsmInstIdtfr::SYSCALL_AII, {});
    if (callInst->name == "getint") {
      blockAddInst(AsmInstIdtfr::ADDU_AII, {WRR(value2asmReg(callInst->result),
          name2PhysAsmReg["zero"], name2PhysAsmReg["v0"])});
    }
    return;
  }
  /* 参数压栈 */
  int argStackOffsetBytes = 0;
  for (u_long i = 0; i < callInst->args.size(); i++) {
    /* sw rt, imm(base) */
    blockAddInst(AsmInstIdtfr::SW_AII, {RRR(value2asmReg(callInst->args[i]), 
      allocAsmImm(argStackOffsetBytes), name2PhysAsmReg["sp"])});
    argStackOffsetBytes += 4; /* 函数参数一定都是4字节大小 */
  }
  blockAddInst(AsmInstIdtfr::JAL_AII, {R(allocAsmLabel(callInst->name))});
  if (callInst->result != NULL) {
    blockAddInst(AsmInstIdtfr::ADD_AII, {WRR(value2asmReg(callInst->result), 
      name2PhysAsmReg["zero"], name2PhysAsmReg["v0"])});
  }
}

void retInst2asm( RetInst* retInst)
{ 
  int raOffsetBytes = nowFunction->raWordOffset * 4;
  /* lw rt, imm(base) */
  blockAddInst(AsmInstIdtfr::LW_AII, {WRR(name2PhysAsmReg["ra"],
    allocAsmImm(raOffsetBytes), name2PhysAsmReg["sp"])});
  if (!retInst->isVoid) {
    blockAddInst(AsmInstIdtfr::ADDU_AII, {WRR(name2PhysAsmReg["v0"], 
      name2PhysAsmReg["zero"], value2asmReg(retInst->value))});
  }
  /* 消除栈帧 */
  blockAddInst(AsmInstIdtfr::ADDIU_AII, {WRR(name2PhysAsmReg["sp"], 
    name2PhysAsmReg["sp"], frameSize.second)});
  blockAddInst(AsmInstIdtfr::JR_AII, {R(name2PhysAsmReg["ra"])});
}
