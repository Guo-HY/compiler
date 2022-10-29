#ifndef _IR_BUILD_H
#define _IR_BUILD_H 1
#include "ir.hpp"
#include "../frontend/parser/symbol_table.hpp"
#include "../frontend/parser/syntax_tree.hpp"

void updateLlvmIrId(int id);
Type* type2PtrType(Type* t);
Type* ptrType2Type(Type* t);
VirtRegValue* allocVirtReg(Type* type);
LabelValue* allocLabel();
BasicBlock* allocBasicBlock();
Type* bType2ir(BTypeNode* node);
Type* abstVarDef2Type(AbstVarDefNode* node);
Type* funcFParam2Type(AbstVarDefNode* node);


Value* genAllocaInst(BasicBlock* nowBasicBlock, Type* allocType);
void genStoreInst(BasicBlock* nowBasicBlock, Value* storeValue, Value* storePtr);
Value* genCallInst(BasicBlock* nowBasicBlock, std::string funcName, FuncRParamsNode* node);
Value* genBinaryInst(BasicBlock* nowBasicBlock, BinaryInstIdtfr opType, Value* op1, Value* op2);
Value* genLoadInst(BasicBlock* nowBasicBlock, Value* ptr);
Value* genIcmpInst(BasicBlock* nowBasicBlock, ICMPCASE op, Value* op1, Value* op2);
Value* genZextInst(BasicBlock* nowBasicBlock, int extWidth, Value* value);
void genBrInst(BasicBlock* nowBasicBlock, bool isUnCond, Value* ifTrue, Value* ifFalse, Value* cond);
void genRetInst(BasicBlock* nowBasicBlock,Type* type, Value* value, bool isVoid);
Value* genGEPInst(BasicBlock* nowblk,Value* ptrVal, Type* elemType, std::vector<Value*> indexs);

Module* compUnit2ir(CompUnitNode* node, std::vector<std::string> funcDecl);
GlobalInitValue* globalInitVal2ir(InitValNode* node) ;
GlobalInitValue* globalConstInitVal2ir(std::string* varName, ConstInitValNode* node);
GlobalValue* globalVarDef2ir(AbstVarDefNode* node);
Value* storeFuncParam(BasicBlock* nowBasicBlock, Type* type, VirtRegValue* value);
Function* funcDef2ir(FuncDefNode* node);
Value* exp2ir(BasicBlock* nowBasicBlock, ExpNode* node);
Value* number2ir(NumberNode* node);
Value* lVal2ir(BasicBlock* nowblk, LValNode* node);
Value* primaryExp2ir(BasicBlock* nowBasicBlock, PrimaryExpNode* node);
Value* number2ir(NumberNode* node);
Value* unaryExp2ir(BasicBlock* nowBasicBlock, UnaryExpNode* node);
Value* mulExp2ir(BasicBlock* nowBasicBlock, MulExpNode* node);
Value* addExp2ir(BasicBlock* nowBasicBlock, BinaryExpNode* node);
Value* relExp2ir(BasicBlock* nowBasicBlock, BinaryExpNode* node);
Value* eqExp2ir(BasicBlock* nowBasicBlock, BinaryExpNode* node);
Value* constExp2ir(BasicBlock* nowBasicBlock, ConstExpNode* node);
BasicBlock* block2ir(BasicBlock* nowBasicBlock, BlockNode* node, bool genNewSymbolTable);
void localConstInitVal2ir(BasicBlock* nowBasicBlock, ConstInitValNode* node, Value* addr);
void localInitVal2ir(BasicBlock* nowBasicBlock, InitValNode* node, Value* addr);
Value* localVarDef2ir(BasicBlock* nowBasicBlock, AbstVarDefNode* node);
BasicBlock* stmt2ir(BasicBlock* nowBasicBlock, StmtNode* node);
void printfStmt2ir(BasicBlock* nowblk, StmtNode* node);
BasicBlock* ifStmt2ir(BasicBlock* nowBasicBlock, StmtNode* node);
BasicBlock* whileStmt2ir(BasicBlock* nowBasicBlock, StmtNode* node);
void lOrExp2ir(BasicBlock* nowblk, BinaryExpNode* node, LabelValue* ifTrueLabel, LabelValue* ifFalseLabel);
void lAndExp2ir(BasicBlock* nowblk, BinaryExpNode* node, LabelValue* ifTrueLabel, LabelValue* ifFalseLabel);
BasicBlock* returnStmt2ir(BasicBlock* nowblk, StmtNode* node);
void constDecl2ir(BasicBlock* nowblk, ConstDeclNode* node);
void varDecl2ir(BasicBlock* nowblk, VarDeclNode* node);
void dealputstrCall(BasicBlock* nowblk, std::string s, int strLength);

#endif