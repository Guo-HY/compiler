#include "ir_build.hpp"
#include "../include/error.hpp"
#include <unordered_set>
/* declare */
/* 全局符号表 */
extern SymbolTable* globalSymbolTable;
/* 当前符号表 */
extern SymbolTable* currentSymbolTable;

extern void symbolTableInit();

static int nowLlvmIrId;
static Function* nowFunction;
static std::unordered_map<int, VirtRegValue*> id2LocalVarAddr;
static std::unordered_map<std::string, GlobalValue*> name2globalValue;
static std::unordered_map<std::string, GlobalValue*> value2stringConst;
static std::vector<BasicBlock*> whileEndBlockStack;
static std::vector<BasicBlock*> whileCondBlockStack;
static std::unordered_set<int> nowFuncLabels;  /* 存放当前函数所有br指令使用的label编号 */

static int inFuncCallAnalysis;  /* 当前是否在函数调用解析中 */
/* --------------------------------------- tools ---------------------------------------  */

void updateLlvmIrId(int id)
{
  nowLlvmIrId = id;
}

/* 返回传入类型的指针类型 */
Type* type2PtrType(Type* t)
{
  return new PointerType(t);
}

/* 返回传入的指针类型所指向的类型 */
Type* ptrType2Type(Type* t)
{
  if (t->typeIdtfr != TypeIdtfr::POINTER_TI) {
    panic("error");
  }
  return ((PointerType*)t)->pointType;
}

/* 分配type类型的虚拟寄存器并将其插入当前id+1 */
VirtRegValue* allocVirtReg(Type* type)
{
  VirtRegValue* v = new VirtRegValue(nowLlvmIrId, type);
  nowLlvmIrId++;
  return v;
}

/* 分配一个标签并将id+1 */
LabelValue* allocLabel()
{
  LabelValue* l = new LabelValue(nowLlvmIrId);
  nowLlvmIrId++;
  return l;
}

/* 分配一个BasicBlock并插入当前函数 */
BasicBlock* allocBasicBlock()
{
  BasicBlock* b = new BasicBlock(allocLabel());
  nowFunction->basicBlocks.push_back(b);
  return b;
}

bool bblkHasTermiInst(BasicBlock* bblk)
{
  if (bblk == NULL) { panic("error"); }
  if (bblk->instructions.size() == 0) {
    return false;
  }

  Instruction* inst = bblk->instructions[bblk->instructions.size() - 1];
  if (inst == NULL) {
    panic("error");
  }
  if (inst->instType == InstIdtfr::RET_II || inst->instType == InstIdtfr::BR_II) {
    return true;
  }
  return false;
}

Type* bType2ir(BTypeNode* node)
{
  return new IntegerType(32);
}

/* 不可用于函数形参解析 */
Type* abstVarDef2Type(AbstVarDefNode* node)
{ 
  //Log("");
  if (node->arrayDimension == 0) {
    return bType2ir(node->bTypeNode);
  }
  if (node->arrayDimension == 1) {
    Log("value = %d",node->constExpNodes[0]->getConstValue() );
    return new ArrayType(node->constExpNodes[0]->getConstValue(), bType2ir(node->bTypeNode));
  } else if (node->arrayDimension == 2) {
    ArrayType* tmp = new ArrayType(node->constExpNodes[1]->getConstValue(), bType2ir(node->bTypeNode));
    return new ArrayType(node->constExpNodes[0]->getConstValue(), tmp);
  }
  panic("node->arrayDimension = %d", node->arrayDimension);
}

/* 对于数组返回数组第一维元素的指针类型 */
Type* funcFParam2Type(AbstVarDefNode* node)
{
  if (node->arrayDimension == 0) {
    return bType2ir(node->bTypeNode);
  }
  if (node->arrayDimension == 1) {
    return new PointerType(bType2ir(node->bTypeNode));
  }
  if (node->arrayDimension == 2) {
    return new PointerType(new ArrayType(node->constExpNodes[0]->getConstValue(), bType2ir(node->bTypeNode)));
  }
  panic("error");
}

int getIntegerTypeWidth(Type* type) 
{
  if (type->typeIdtfr != TypeIdtfr::INTEGER_TI) {
    panic("error");
  }
  return ((IntegerType*)type)->bitWidth;
}

void unifyOperandWidth(BasicBlock* nowBasicBlock, Value** op1, Value** op2)
{
  int op1Width = getIntegerTypeWidth((*op1)->type);
  int op2Width = getIntegerTypeWidth((*op2)->type);
  if (op1Width < op2Width) {
    *op1 = genZextInst(nowBasicBlock, op2Width, *op1);
  } else if(op2Width < op1Width) {
    *op2 = genZextInst(nowBasicBlock, op1Width, *op2);
  }
}

Type* getArrayElemType(Type* type)
{
  if (type->typeIdtfr != TypeIdtfr::ARRAY_TI) {
    panic("error");
  }
  return ((ArrayType*)type)->elemType;
}

int getArrayElemNum(Type* type)
{
  if (type->typeIdtfr != TypeIdtfr::ARRAY_TI) {
    panic("error");
  }
  return ((ArrayType*)type)->elemNums;
}

/* ---------------------------- gen*Inst 返回的是指令得出的值，指令直接存入nowBasicBlock ---------------------------- */

Value* genAllocaInst(BasicBlock* nowBasicBlock, Type* allocType)
{
  AllocaInst* inst = new AllocaInst(allocType, allocVirtReg(type2PtrType(allocType)));
  nowBasicBlock->instructions.push_back(inst);
  return inst->result;
}

void genStoreInst(BasicBlock* nowBasicBlock, Value* storeValue, Value* storePtr)
{
  StoreInst* inst = new StoreInst(storeValue, storePtr);
  nowBasicBlock->instructions.push_back(inst);
}

/* node可以为NULL */
Value* genCallInst(BasicBlock* nowBasicBlock, std::string funcName, FuncRParamsNode* node)
{
  inFuncCallAnalysis++;
  CallInst* inst = new CallInst();
  ObjectSymbolItem* item = globalSymbolTable->getFuncReturnType(&funcName);
  Log("item type = %d", item->symbolType);
  if (item->symbolType == SymbolType::INT_ST) {
    inst->returnType = new IntegerType(32);
  } else {
    inst->returnType = new VoidType();
  }
  delete item;
  
  inst->name = funcName;
  VirtRegValue* result = allocVirtReg(inst->returnType);
  if (inst->returnType->typeIdtfr != TypeIdtfr::VOID_TI) {
    inst->result = result;
  } else {
    inst->result = NULL;
  }
  
  if (node != NULL) {
    for (u_long i = 0; i < node->expNodes.size(); i++) {
      inst->args.push_back(exp2ir(nowBasicBlock, node->expNodes[i]));
    }
  }
  nowBasicBlock->instructions.push_back(inst);
  inFuncCallAnalysis--;
  return result;
}

Value* genBinaryInst(BasicBlock* nowBasicBlock, BinaryInstIdtfr opType, Value* op1, Value* op2)
{
  /* 操作前需要统一位宽 */
  unifyOperandWidth(nowBasicBlock, &op1, &op2);
  VirtRegValue* result = allocVirtReg(op1->type);
  BinaryInst* inst = new BinaryInst(opType);
  inst->op1 = op1;
  inst->op2 = op2;
  inst->result = result;
  nowBasicBlock->instructions.push_back(inst);
  return result;
}

/* 假设load 出值的类型就是指针所指的类型,ptr可能是存有地址的虚拟寄存器或者全局变量地址 */
Value* genLoadInst(BasicBlock* nowBasicBlock, Value* ptr)
{
  Type* ptrType;
  Type* valueType;
  ptrType = ptr->type;
  valueType = ptrType2Type(ptrType);
  LoadInst* inst = new LoadInst();
  inst->pointer = ptr;
  inst->result = allocVirtReg(valueType);
  nowBasicBlock->instructions.push_back(inst);
  return inst->result;
}

Value* genIcmpInst(BasicBlock* nowBasicBlock, ICMPCASE op, Value* op1, Value* op2)
{
  /* 操作前需要统一位宽 */
  unifyOperandWidth(nowBasicBlock, &op1, &op2);
  IcmpInst* inst = new IcmpInst();
  inst->cond = op;
  inst->op1 = op1;
  inst->op2 = op2;
  inst->result = allocVirtReg(new IntegerType(1));
  nowBasicBlock->instructions.push_back(inst);
  return inst->result;
}

Value* genZextInst(BasicBlock* nowBasicBlock, int extWidth, Value* value)
{
  VirtRegValue* result = allocVirtReg(new IntegerType(extWidth));
  ZextInst* inst = new ZextInst();
  inst->result = result;
  inst->value = value;
  nowBasicBlock->instructions.push_back(inst);
  return result;
}

void genBrInst(BasicBlock* nowBasicBlock, bool isUnCond, Value* ifTrue, Value* ifFalse, Value* cond)
{
  BrInst* inst = new BrInst();
  inst->cond = cond;
  inst->isUnCond = isUnCond;
  inst->iftrue = ifTrue;
  inst->iffalse = ifFalse;
  inst->dest = ifTrue;
  nowBasicBlock->instructions.push_back(inst);
  nowFuncLabels.emplace(((LabelValue*)ifTrue)->id);
  if (!isUnCond) {
    nowFuncLabels.emplace(((LabelValue*)ifFalse)->id);
  }
}

void genRetInst(BasicBlock* nowBasicBlock,Type* type, Value* value, bool isVoid)
{ 
  RetInst* inst = new RetInst();
  inst->isVoid = isVoid;
  inst->returnType = type;
  inst->value = value;
  nowBasicBlock->instructions.push_back(inst);
}

Value* genGEPInst(BasicBlock* nowblk,Value* ptrVal, Type* elemType, std::vector<Value*> indexs)
{
  
  GEPInst* inst = new GEPInst();
  Type* elemPtrType = type2PtrType(elemType);
  Type* resultPointType = elemType;
  int dims = indexs.size();
  for (int i = 1; i < dims; i++) {
    if (resultPointType->typeIdtfr != TypeIdtfr::ARRAY_TI) {
      panic("error");
    }
    resultPointType = ((ArrayType*)resultPointType)->elemType;
  }
  VirtRegValue* result = allocVirtReg(type2PtrType(resultPointType));
  inst->elemType = elemType;
  inst->elemTypePointer = elemPtrType;
  inst->indexs = indexs;

  inst->ptrval = ptrVal;
  inst->result = result;
  nowblk->instructions.push_back(inst);
  return result;
}

/* ------------------------------------ Node2ir ------------------------------------  */


Module* compUnit2ir(CompUnitNode* node, std::vector<std::string> funcDecl)
{
  //Log("");
  symbolTableInit();
  name2globalValue.clear();
  Module* m = new Module();
  m->funcDecl = funcDecl;
  u_long i,j;
  for (i = 0; i < node->declNodes.size(); i++) {
    if (node->declNodes[i]->declNodeType == DeclNodeType::DECL_CONST) {
      ConstDeclNode* c = node->declNodes[i]->constDeclNode;
      for (j = 0; j < c->constDefNodes.size(); j++) {
        m->globalVariables.push_back(globalVarDef2ir(c->constDefNodes[j]));
      }
    } else {
      VarDeclNode* v = node->declNodes[i]->varDeclNode;
      for (j = 0; j < v->varDefNodes.size(); j++) {
        m->globalVariables.push_back(globalVarDef2ir(v->varDefNodes[j]));
      }
    }
  }
  for (i = 0; i < node->funcDefNodes.size(); i++) {
    m->funcDef.push_back(funcDef2ir(node->funcDefNodes[i]));
  }
  m->funcDef.push_back(funcDef2ir(node->mainFuncDefNode));

  /* 最后需要将常量字符串加入 */
  for (auto iter = value2stringConst.begin(); iter != value2stringConst.end(); iter++) {
    m->globalVariables.push_back(iter->second);
  }
  return m;
}


GlobalInitValue* globalInitVal2ir(InitValNode* node) 
{
  if (node->initArray == false) {
    return new GlobalInitValue(new IntegerType(32), new NumberConstant(node->expNode->getConstValue(), 32));
  }
  GlobalInitValue* globalInitValue = new GlobalInitValue();
  for (u_long i = 0; i < node->initValNodes.size(); i++) {
    globalInitValue->globalInitValues.push_back(globalInitVal2ir(node->initValNodes[i]));
  }
  globalInitValue->type = new ArrayType(globalInitValue->globalInitValues.size(), globalInitValue->globalInitValues[0]->type);
  return globalInitValue;
}

GlobalInitValue* globalConstInitVal2ir(std::string* varName, ConstInitValNode* node)
{
  if (node->initArray == false) {
    GlobalInitValue* globalInitValue = new GlobalInitValue(new IntegerType(32), new NumberConstant(node->constExpNode->getConstValue(), 32));
    Log("%s, addInitValue = %d, again %d",varName->c_str(), node->constExpNode->getConstValue(), node->constExpNode->getConstValue());
    currentSymbolTable->addInitValue(varName, node->constExpNode->getConstValue());
    return globalInitValue;
  }
  GlobalInitValue* globalInitValue = new GlobalInitValue();
  for (u_long i = 0; i < node->constInitValNodes.size(); i++) {
    globalInitValue->globalInitValues.push_back(globalConstInitVal2ir(varName, node->constInitValNodes[i]));
  }
  globalInitValue->type = new ArrayType(globalInitValue->globalInitValues.size(), globalInitValue->globalInitValues[0]->type);
  return globalInitValue;
}

/* 全局变量：类型 初始化值,注意全局变量的类型是指向全局变量所存类型的指针类型 */
GlobalValue* globalVarDef2ir(AbstVarDefNode* node)
{
  globalSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::ABSTVAR_SNT);
  Type* type = abstVarDef2Type(node);
  globalSymbolTable->addVarType(&(node->ident->str), type);
  GlobalValue* globalValue = new GlobalValue(type2PtrType(type));
  globalValue->name = node->ident->str;
  globalValue->isConst = node->isConst;
  name2globalValue[globalValue->name] = globalValue;
  if (node->isConst == false) {
    VarDefNode* var = (VarDefNode*)node;
    if (var->hasInitVal) {
      globalValue->globalInitValue = globalInitVal2ir(var->initValNode);
    }
  } else {
    ConstDefNode* constVar = (ConstDefNode*)node;
    globalValue->globalInitValue = globalConstInitVal2ir(&(node->ident->str), constVar->constInitValNode);
  }
  return globalValue;
}

Value* storeFuncParam(BasicBlock* nowBasicBlock, Type* type, VirtRegValue* value)
{
  Value* addr;
  addr = genAllocaInst(nowBasicBlock, type);
  genStoreInst(nowBasicBlock, value, addr);
  return addr;
}

Function* funcDef2ir(FuncDefNode* node)
{
  //Log("");
  id2LocalVarAddr.clear();
  nowFuncLabels.clear();
  updateLlvmIrId(1);
  currentSymbolTable = currentSymbolTable->newSon();
  globalSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::FUNC_SNT);
  Function* function = new Function();
  nowFunction = function;
  BasicBlock* nowBasicBlock = allocBasicBlock();

  function->funcName = node->ident->str;
  if (node->funcTypeNode->funcType->tokenType == TokenType::INTTK) {
    function->returnType = new IntegerType(32);
  } else {
    function->returnType = new VoidType();
  }

  if (node->hasFuncFParams) {
    for (u_long i = 0; i < node->funcFParamsNode->funcFParamNodes.size(); i++) {
      FuncFParamNode* ffpn = node->funcFParamsNode->funcFParamNodes[i];
      Type* t = funcFParam2Type((AbstVarDefNode*)ffpn);
      VirtRegValue* v = allocVirtReg(t);
      VirtRegValue* addr = (VirtRegValue*)storeFuncParam(nowBasicBlock, t, v);
      currentSymbolTable->insertNodeWithLlvmIrId(&(ffpn->ident->str), (SyntaxNode*)ffpn, SyntaxNodeType::ABSTVAR_SNT, addr->getId());
      id2LocalVarAddr[addr->getId()] = addr;
      function->funcFParamValues.push_back(new FuncFParamValue(t, v));
    }
  }
  nowBasicBlock = block2ir(nowBasicBlock, node->blockNode, false);
  /* 删除没有语句的block,同时 处理一下 void函数没有return语句的情况 */
  std::vector<BasicBlock*> bbkstmp = function->basicBlocks;
  function->basicBlocks.clear();
  for (u_long i = 0; i < bbkstmp.size(); i++) {
    if (bbkstmp[i]->instructions.size() > 0) {
      function->basicBlocks.push_back(bbkstmp[i]);
    } else if (nowFuncLabels.count(bbkstmp[i]->label->id) != 0) {
      if (function->returnType->typeIdtfr != TypeIdtfr::VOID_TI) {
        panic("error");
      }
      genRetInst(bbkstmp[i], NULL, NULL, true);
      function->basicBlocks.push_back(bbkstmp[i]);
    }
  }
  if (function->basicBlocks.size() == 0) {
    allocBasicBlock();
  }
  /* TODO 似乎没有必要了 */
  /* 处理一下 void函数没有return语句的情况 */

  BasicBlock* lastBasicBlock = function->basicBlocks[function->basicBlocks.size() - 1];
  if (lastBasicBlock == NULL) {
    panic("error");
  }
  if (!bblkHasTermiInst(lastBasicBlock)) {
    genRetInst(lastBasicBlock, NULL, NULL, true);
  }

  currentSymbolTable = currentSymbolTable->findParent();
  function->maxLlvmIrId = nowLlvmIrId + 1;  /* 记录最大的id号，给后端用 */
  return function;
}

Value* exp2ir(BasicBlock* nowBasicBlock, ExpNode* node)
{
  return addExp2ir(nowBasicBlock, node->addExpNode);
}

Value* number2ir(NumberNode* node)
{
  return new NumberConstant(node->getConstValue(), 32);
}

/* 返回的是变量的值的地址 */
Value* lVal2ir(BasicBlock* nowblk, LValNode* node)
{
  Value* addr;
  bool isGlobal;
  int llvmIrId = currentSymbolTable->getLlvmIrId(&(node->ident->str));
  if (id2LocalVarAddr.count(llvmIrId) != 0) {
    addr = id2LocalVarAddr[llvmIrId];
    isGlobal = false;
  } else if (name2globalValue.count(node->ident->str) != 0) {
    addr = name2globalValue[node->ident->str];
    isGlobal = true;
  } else {
    panic("error");
  }
  if (((PointerType*)(addr->type))->pointType->typeIdtfr == TypeIdtfr::INTEGER_TI) {
    /* 如果不是数组类型，直接返回地址就可以 */
    return addr;
  }


  std::vector<Value*> indexs;
  indexs.push_back(new NumberConstant(0, 64));
  for (u_long i = 0; i < node->expNodes.size(); i++) {
    indexs.push_back(exp2ir(nowblk, node->expNodes[i]));
  }
  if (isGlobal) {
    return genGEPInst(nowblk, addr, ptrType2Type(addr->type), indexs);
  }
  if (((PointerType*)(addr->type))->pointType->typeIdtfr == TypeIdtfr::POINTER_TI) {
    /* 此时一定是传入的函数参数 */
    if (node->expNodes.size() == 0) {
      return addr;
    }
    addr = genLoadInst(nowblk, addr);
    if (node->expNodes.size() == 1) {
      return genGEPInst(nowblk, addr, ptrType2Type(addr->type), {indexs[1]});
    } else {
      return genGEPInst(nowblk, addr, ptrType2Type(addr->type), {indexs[1],indexs[2]});
    }
  }
  /* 此时是局部声明的变量 */
  if (node->expNodes.size() == 1) {
    return genGEPInst(nowblk, addr, ptrType2Type(addr->type), indexs);
  }
  return genGEPInst(nowblk, addr, ptrType2Type(addr->type), indexs);
  
}

Value* primaryExp2ir(BasicBlock* nowBasicBlock, PrimaryExpNode* node)
{
  Value* addr;
  switch (node->primaryExpType)
  {
  case PrimaryExpType::PRIMARY_EXP:
    return exp2ir(nowBasicBlock, node->expNode);
  case PrimaryExpType::PRIMARY_LVAL:
    addr = lVal2ir(nowBasicBlock, node->lValNode);
    /* 此处将值load出来返回，注意需要区分是否是funcCall传数组 */
    if (inFuncCallAnalysis && ((PointerType*)(addr->type))->pointType->typeIdtfr == TypeIdtfr::ARRAY_TI) {
      NumberConstant* idx = new NumberConstant(0, 64);
      return genGEPInst(nowBasicBlock, addr, ptrType2Type(addr->type), {idx, idx} );
    } else {
      return genLoadInst(nowBasicBlock, addr);
    }
  case PrimaryExpType::PRIMARY_NUMBER:
    return number2ir(node->numberNode);
  default:
    panic("error");
    break;
  }
}

Value* unaryExp2ir(BasicBlock* nowBasicBlock, UnaryExpNode* node)
{
  Value* value;
  Value* i1;
  int bitWidth;
  switch (node->unaryExpType)
  {
  case UnaryExpType::UNARY_PRIMARYEXP:
    return primaryExp2ir(nowBasicBlock, node->primaryExpNode);
  case UnaryExpType::UNARY_FUNCCALL:
    return genCallInst(nowBasicBlock, node->ident->str, node->funcRParamsNode);
  case UnaryExpType::UNARY_PREFIX:
    value = unaryExp2ir(nowBasicBlock, node->unaryExpNode);
    if (value->type->typeIdtfr != TypeIdtfr::INTEGER_TI) {
      panic("error");
    }
    bitWidth = ((IntegerType*)value->type)->bitWidth;
    if (node->unaryOpNode->unaryOp->tokenType == TokenType::MINU) {
      return genBinaryInst(nowBasicBlock, BinaryInstIdtfr::SUB_BII, new NumberConstant(0, bitWidth), value);/* 默认value一定是int32 */
    } else if(node->unaryOpNode->unaryOp->tokenType == TokenType::NOT) {
      i1 = genIcmpInst(nowBasicBlock, ICMPCASE::EQ_ICMPCASE, value, new NumberConstant(0, bitWidth));
      return i1;    /* 此处不用扩展 */
      // return genZextInst(nowBasicBlock, 32, i1); /* 将i1扩展为i32 */
    }
    return value;
  default:
    panic("error");
  }
  panic("error");
  return NULL;
}

Value* mulExp2ir(BasicBlock* nowBasicBlock, MulExpNode* node)
{
  Value* op1 = unaryExp2ir(nowBasicBlock, node->unaryExpNodes[0]);
  BinaryInstIdtfr opType;
  for (u_long i = 1; i < node->unaryExpNodes.size(); i++) {
    Value* op2 = unaryExp2ir(nowBasicBlock, node->unaryExpNodes[i]);
    if (i-1 >= node->ops.size()) { panic("error"); }
    switch (node->ops[i-1]->tokenType)
    {
    case TokenType::MULT:
      opType = BinaryInstIdtfr::MUL_BII; break;
    case TokenType::DIV:
      opType = BinaryInstIdtfr::SDIV_BII; break;
    case TokenType::MOD:
      opType = BinaryInstIdtfr::MOD_BII; break;
    default:
      panic("error");
      break;
    }
    op1 = genBinaryInst(nowBasicBlock, opType, op1, op2);
  }
  return op1;
}

Value* addExp2ir(BasicBlock* nowBasicBlock, BinaryExpNode* node)
{
  Value* op1 = mulExp2ir(nowBasicBlock, (MulExpNode*)(node->operands[0]));
  BinaryInstIdtfr opType;
  for (u_long i = 1; i < node->operands.size(); i++) {
    Value* op2 = mulExp2ir(nowBasicBlock, (MulExpNode*)(node->operands[i]));
    if (i-1 >= node->ops.size()) { panic("error"); }
    switch (node->ops[i-1]->tokenType)
    {
    case TokenType::PLUS:
      opType = BinaryInstIdtfr::ADD_BII; break;
    case TokenType::MINU:
      opType = BinaryInstIdtfr::SUB_BII; break;
    default:
      panic("error");
      break;
    }
    op1 = genBinaryInst(nowBasicBlock, opType, op1, op2);
  }
  return op1;
}

/* 返回值的类型为i1 */
Value* relExp2ir(BasicBlock* nowBasicBlock, BinaryExpNode* node)
{
  Value* op1 = addExp2ir(nowBasicBlock, node->operands[0]);
  ICMPCASE opType;
  for (u_long i = 1; i < node->operands.size(); i++) {
    Value* op2 = addExp2ir(nowBasicBlock, node->operands[i]);
    if (i-1 >= node->ops.size()) { panic("error"); }
    switch (node->ops[i-1]->tokenType)
    {
    case TokenType::LSS:
      opType = ICMPCASE::SLT_ICMPCASE; break;
    case TokenType::GRE:
      opType = ICMPCASE::SGT_ICMPCASE; break;
    case TokenType::LEQ:
      opType = ICMPCASE::SLE_ICMPCASE; break;
    case TokenType::GEQ:
      opType = ICMPCASE::SGE_ICMPCASE; break;
    default:
      panic("error");
      break;
    }
    op1 = genIcmpInst(nowBasicBlock, opType, op1, op2);
  }

  return op1;
}



/* 返回值的类型为i1 */
Value* eqExp2ir(BasicBlock* nowBasicBlock, BinaryExpNode* node)
{
  Value* op1 = relExp2ir(nowBasicBlock, node->operands[0]);
  ICMPCASE opType;
  for (u_long i = 1; i < node->operands.size(); i++) {
    Value* op2 = relExp2ir(nowBasicBlock, node->operands[i]);
    if (i-1 >= node->ops.size()) { panic("error"); }
    switch (node->ops[i-1]->tokenType)
    {
    case TokenType::EQL:
      opType = ICMPCASE::EQ_ICMPCASE; break;
    case TokenType::NEQ:
      opType = ICMPCASE::NE_ICMPCASE; break;
    default:
      panic("error");
      break;
    }
    op1 = genIcmpInst(nowBasicBlock, opType, op1, op2);
  }
  // if (node->operands.size() == 1) {
  //   op1 = genIcmpInst(nowBasicBlock, ICMPCASE::NE_ICMPCASE, op1, new NumberConstant(0, 32));
  // }
  return op1;
}

Value* constExp2ir(BasicBlock* nowBasicBlock, ConstExpNode* node)
{
  return addExp2ir(nowBasicBlock, node->addExpNode);
}



/* 涉及控制流的函数均返回自己处理的最后一个block,保证传给stmt的block是没有终结指令的 */

BasicBlock* block2ir(BasicBlock* nowBasicBlock, BlockNode* node, bool genNewSymbolTable)
{
  //Log("");
  if (genNewSymbolTable) {
    currentSymbolTable = currentSymbolTable->newSon();
  }
  
  for (u_long i = 0; i < node->blockItemNodes.size(); i++) {
    BlockItemNode* blockItemNode = node->blockItemNodes[i];
    if (blockItemNode->blockItemType == BlockItemType::BLOCKITEM_DECL) {
      if (blockItemNode->declNode->declNodeType == DeclNodeType::DECL_VAR) {
        varDecl2ir(nowBasicBlock, blockItemNode->declNode->varDeclNode);
      } else {
        constDecl2ir(nowBasicBlock, blockItemNode->declNode->constDeclNode);
      }
    } else {
      nowBasicBlock = stmt2ir(nowBasicBlock, blockItemNode->stmtNode);
    }
  }

  if (genNewSymbolTable) {
    currentSymbolTable = currentSymbolTable->findParent();
  }
  return nowBasicBlock;
}

void constDecl2ir(BasicBlock* nowblk, ConstDeclNode* node)
{
  for (u_long i = 0; i < node->constDefNodes.size(); i++) {
    localVarDef2ir(nowblk, node->constDefNodes[i]);
  }
}

void varDecl2ir(BasicBlock* nowblk, VarDeclNode* node)
{ 
  for (u_long i = 0; i < node->varDefNodes.size(); i++) {
    localVarDef2ir(nowblk, node->varDefNodes[i]);
  }
}

void localConstInitVal2ir(BasicBlock* nowBasicBlock, ConstInitValNode* node, Value* addr, std::string* varName)
{
  if (!node->initArray) {
    int value = node->constExpNode->getConstValue();
    currentSymbolTable->addInitValue(varName, value);
    Value* initValue = new NumberConstant(value, 32);
    genStoreInst(nowBasicBlock, initValue, addr);
  }
  Value* elemAddr;
  for (u_long i = 0; i < node->constInitValNodes.size(); i++) {
    elemAddr = genGEPInst(nowBasicBlock, addr, ptrType2Type(addr->type), {new NumberConstant(0, 64), new NumberConstant(i, 64)});
    localConstInitVal2ir(nowBasicBlock,node->constInitValNodes[i], elemAddr, varName);
  }
}

void localInitVal2ir(BasicBlock* nowBasicBlock, InitValNode* node, Value* addr)
{
  if (!node->initArray) {
    Value* initValue = exp2ir(nowBasicBlock, node->expNode);
    genStoreInst(nowBasicBlock, initValue, addr);
    return;
  }
  Value* elemAddr;
  for (u_long i = 0; i < node->initValNodes.size(); i++) {
    elemAddr = genGEPInst(nowBasicBlock, addr, ptrType2Type(addr->type), {new NumberConstant(0, 64), new NumberConstant(i, 64)});
    localInitVal2ir(nowBasicBlock,node->initValNodes[i], elemAddr);
  }
}

Value* localVarDef2ir(BasicBlock* nowBasicBlock, AbstVarDefNode* node)
{
  //Log("");
  Type* valueType = abstVarDef2Type(node);

  VirtRegValue* addr = (VirtRegValue*)genAllocaInst(nowBasicBlock, valueType);
  currentSymbolTable->insertNodeWithLlvmIrId(&(node->ident->str), node, SyntaxNodeType::ABSTVAR_SNT, addr->getId());
  currentSymbolTable->addVarType(&(node->ident->str), valueType);
  if (node->isConst) {
    localConstInitVal2ir(nowBasicBlock, ((ConstDefNode*)node)->constInitValNode, addr, &(node->ident->str));
  } else if (((VarDefNode*)node)->hasInitVal) {
    localInitVal2ir(nowBasicBlock, ((VarDefNode*)node)->initValNode, addr);
  }
  /* 插入当前函数变量虚拟寄存器表 */
  id2LocalVarAddr[addr->getId()] = addr; 
  return addr;
}

BasicBlock* stmt2ir(BasicBlock* nowBasicBlock, StmtNode* node)
{
  Value *expValue, *lvalAddr, *value, *addr;
  CallInst* callInst;
  switch (node->stmtType)
  {
  case StmtType::STMT_ASSIGN:
    expValue = exp2ir(nowBasicBlock, node->expNodes[0]);
    lvalAddr = lVal2ir(nowBasicBlock, node->lValNode);
    genStoreInst(nowBasicBlock, expValue, lvalAddr);
    return nowBasicBlock;
  case StmtType::STMT_EXP:
    if (node->expNodes.size() > 0) {
      exp2ir(nowBasicBlock, node->expNodes[0]);
    }
    return nowBasicBlock;
  case StmtType::STMT_RETURN:
    return returnStmt2ir(nowBasicBlock, node);
  case StmtType::STMT_BLOCK:
    return block2ir(nowBasicBlock, node->blockNode, true);
  case StmtType::STMT_IF:
    return ifStmt2ir(nowBasicBlock, node);
  case StmtType::STMT_WHILE:
    return whileStmt2ir(nowBasicBlock, node);
  case StmtType::STMT_BREAK:
    /* 要跳到当前所在while的endblock */
    genBrInst(nowBasicBlock, true, whileEndBlockStack.at(whileEndBlockStack.size() - 1)->label , NULL, NULL);
    return allocBasicBlock();
  case StmtType::STMT_CONTINUE:
    genBrInst(nowBasicBlock, true, whileCondBlockStack.at(whileCondBlockStack.size() - 1)->label, NULL, NULL);
    return allocBasicBlock();
  case StmtType::STMT_GETINT:
    value = allocVirtReg(new IntegerType(32));
    callInst = new CallInst();
    callInst->name = "getint";
    callInst->result = value;
    callInst->returnType = new IntegerType(32);
    nowBasicBlock->instructions.push_back(callInst);
    addr = lVal2ir(nowBasicBlock, node->lValNode);
    genStoreInst(nowBasicBlock, value, addr);
    return nowBasicBlock;
  case StmtType::STMT_PRINTF:
    printfStmt2ir(nowBasicBlock, node);
    return nowBasicBlock;
  default:  
    panic("error");
    break;
  }

}

void dealputstrCall(BasicBlock* nowblk, std::string s, int strLength)
{
  s += "\\00";
  GlobalValue* strConst;
  if (value2stringConst.count(s) != 0) {
    strConst = value2stringConst[s];
  } else {
    Type* t = new ArrayType(strLength, new IntegerType(8));
    GlobalInitValue* initValue = new GlobalInitValue(t, new StringConstant(s, t));
    strConst = new GlobalValue(t);
    strConst->isConst = true;
    strConst->isconstString = true;
    strConst->name = ".const_str" + std::to_string(value2stringConst.size());
    strConst->globalInitValue = initValue;
    value2stringConst[s] = strConst;
  }
  NumberConstant* ptr = new NumberConstant(0, 64);
  Value* addr = genGEPInst(nowblk, strConst, strConst->type, {ptr,ptr});
  CallInst* inst = new CallInst();
  inst->returnType = new VoidType();
  inst->name = "putstr";
  inst->args.push_back(addr);
  nowblk->instructions.push_back(inst);
}

void printfStmt2ir(BasicBlock* nowblk, StmtNode* node)
{
  /* 需要先解析完printf里的exp，再解析printf的输出语句 */
  const char* str = node->formatString->str.c_str();
  std::string s;
  int expPtr = 0;
  int strLength = 0;
  std::vector<Value*> exps;
  for (u_long i = 0; i < node->expNodes.size(); i++) {
    exps.push_back(exp2ir(nowblk, node->expNodes[i]));
  }
  for (int i = 1; str[i] != '"'; i++) {
    if (str[i] == '%') {
      i++;
      if (s.length() != 0) {
        dealputstrCall(nowblk, s, strLength + 1); /* +1是因为有\00 */
      }
      CallInst* inst = new CallInst();
      inst->returnType = new VoidType();
      inst->name = "putint";
      inst->args.push_back(exps[expPtr]);
      expPtr++;
      nowblk->instructions.push_back(inst);
      s.clear();
      strLength = 0;
    } else if (str[i] == '\\') {
      s += "\\0A";
      i++;
      strLength++;
    } else {
      s += str[i];
      strLength++;
    }
  }
  if (strLength != 0) {
    dealputstrCall(nowblk, s, strLength + 1);
  }
}

/* 返回解析完后的当前basicblock */
BasicBlock* ifStmt2ir(BasicBlock* nowBasicBlock, StmtNode* node)
{
  BasicBlock *ifTrueBlock, *ifFalseBlock, *endBlock;
  ifTrueBlock = new BasicBlock(allocLabel());
  ifFalseBlock = new BasicBlock(allocLabel());
  
  if (node->hasElse) {
    endBlock = new BasicBlock(allocLabel());
  } else {
    endBlock = ifFalseBlock;
  }
  lOrExp2ir(nowBasicBlock, node->condNode->lOrExpNode, ifTrueBlock->label, ifFalseBlock->label);
  nowFunction->basicBlocks.push_back(ifTrueBlock);

  nowBasicBlock = stmt2ir(ifTrueBlock, node->stmtNodes[0]);

  genBrInst(nowBasicBlock, true, endBlock->label, NULL, NULL); 

  if (node->hasElse) {
    nowFunction->basicBlocks.push_back(ifFalseBlock);
    nowBasicBlock = stmt2ir(ifFalseBlock, node->stmtNodes[1]);
    genBrInst(nowBasicBlock, true, endBlock->label, NULL, NULL);
  }
  nowFunction->basicBlocks.push_back(endBlock);
  return endBlock;
}

/* 传入的nowBasicBlock需要保证没有终结指令 */
BasicBlock* whileStmt2ir(BasicBlock* nowBasicBlock, StmtNode* node)
{
  BasicBlock* condBlock = new BasicBlock(allocLabel());
  BasicBlock* endBlock = new BasicBlock(allocLabel());
  BasicBlock* ifTrueBlock = new BasicBlock(allocLabel());
  whileCondBlockStack.push_back(condBlock);
  whileEndBlockStack.push_back(endBlock);
  genBrInst(nowBasicBlock, true, condBlock->label, NULL, NULL);
  lOrExp2ir(condBlock, node->condNode->lOrExpNode, ifTrueBlock->label, endBlock->label);
  nowFunction->basicBlocks.push_back(condBlock);
  nowFunction->basicBlocks.push_back(ifTrueBlock);
  nowBasicBlock = stmt2ir(ifTrueBlock, node->stmtNodes[0]);
  genBrInst(nowBasicBlock, true, condBlock->label, NULL, NULL);
  nowFunction->basicBlocks.push_back(endBlock);
  whileCondBlockStack.pop_back();
  whileEndBlockStack.pop_back();
  return endBlock;
}

/* 顺序判断，只要一个LAnd满足就跳到ifTrueLabel，否则新建块判断，当所有都不满足时跳到ifFalseLabel */
void lOrExp2ir(BasicBlock* nowblk, BinaryExpNode* node, LabelValue* ifTrueLabel, LabelValue* ifFalseLabel)
{
  LabelValue* lastIfFalseLabel = ifFalseLabel;
  BasicBlock* nextBlock = NULL;
  
  for (u_long i = 0; i < node->operands.size(); i++) {
    if (i < node->operands.size() - 1) {
      nextBlock = allocBasicBlock();
      ifFalseLabel = nextBlock->label;
    } else {
      ifFalseLabel = lastIfFalseLabel;
    }
    lAndExp2ir(nowblk, node->operands[i], ifTrueLabel, ifFalseLabel);
    nowblk = nextBlock;
  }

}

/* 顺序判断，只要一个EqExp不满足就跳到ifFalseLabel，否则新建块判断，当所有都满足时跳到ifTrueLabel */
void lAndExp2ir(BasicBlock* nowblk, BinaryExpNode* node, LabelValue* ifTrueLabel, LabelValue* ifFalseLabel)
{
  Value* cond = eqExp2ir(nowblk, node->operands[0]);
  LabelValue* lastIfTrueLabel = ifTrueLabel;
  BasicBlock* nextBlock;
  for (u_long i = 1; i < node->operands.size(); i++) {
    nextBlock = allocBasicBlock();
    ifTrueLabel = nextBlock->label;
    if (cond->type->typeIdtfr != TypeIdtfr::INTEGER_TI || 
        ((IntegerType*)(cond->type))->bitWidth != 1) {
      cond = genIcmpInst(nowblk, ICMPCASE::NE_ICMPCASE, cond, new NumberConstant(0, 32));
    }
    genBrInst(nowblk ,false, ifTrueLabel, ifFalseLabel, cond);
    nowblk = nextBlock;
    cond = eqExp2ir(nowblk, node->operands[i]);
  }
  if (cond->type->typeIdtfr != TypeIdtfr::INTEGER_TI || 
        ((IntegerType*)(cond->type))->bitWidth != 1) {
    cond = genIcmpInst(nowblk, ICMPCASE::NE_ICMPCASE, cond, new NumberConstant(0, 32));
  }
  genBrInst(nowblk ,false, lastIfTrueLabel, ifFalseLabel, cond);
}

/* 生成一个新的block返回 */
BasicBlock* returnStmt2ir(BasicBlock* nowblk, StmtNode* node)
{
  if (node->expNodes.size() == 0) {
    genRetInst(nowblk, NULL, NULL, true);
    return allocBasicBlock();
  }
  Value* v = exp2ir(nowblk, node->expNodes[0]);
  genRetInst(nowblk, v->type, v, false);
  return allocBasicBlock();
}

