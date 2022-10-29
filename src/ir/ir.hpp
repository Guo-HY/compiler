#ifndef _IR_H
#define _IR_H 1
#include <vector>
#include <string>
// #include "../frontend/parser/symbol_table.hpp"

class Type;
class VoidType;
class OtherType;
class LabelType;
class FunctionType;
class IntegerType;
class PointerType;
class ArrayType;
class Value;
class Module;
class Function;
class BasicBlock;
class NumberConstant;
class StringConstant;
class LabelValue;
class VirtRegValue;
class FuncFParamValue;
class GlobalValue;
class GlobalInitValue;
class Instruction;
class BinaryInst;
class AllocaInst;
class LoadInst;
class StoreInst;
class GEPInst;
class ZextInst;
class IcmpInst;
class CallInst;
class RetInst;
class BrInst;

enum TypeIdtfr {
  INCOMPLETE_TI,
  LABEL_TI,
  FUNCTION_TI,
  INTEGER_TI,
  POINTER_TI,
  ARRAY_TI,
  OTHER_TI,
  VOID_TI,
};

class Type {
  public:
  TypeIdtfr typeIdtfr;

  Type() : typeIdtfr(INCOMPLETE_TI) {}

  Type(TypeIdtfr t) : typeIdtfr(t) {}

  virtual std::string toString() = 0;

};

class VoidType : public Type {
  public:
  VoidType() : Type(TypeIdtfr::VOID_TI) {}
  std::string toString() override;
};

class OtherType : public Type {
  public:
  OtherType() : Type(TypeIdtfr::OTHER_TI) {}
  std::string toString() override;
};

class LabelType : public Type {
  public:
  LabelType() : Type(TypeIdtfr::LABEL_TI) {}
  std::string toString() override;
};

class FunctionType : public Type {
  public:

  FunctionType() : Type(FUNCTION_TI) {}

  std::string toString() override ;
};

class IntegerType : public Type {
  public:
  int bitWidth;

  IntegerType() : Type(INTEGER_TI), bitWidth(0) {}
  IntegerType(int w) : Type(INTEGER_TI), bitWidth(w) {}
  std::string toString() override ;

};

class PointerType : public Type {
  public:
  Type* pointType;/* 指向的类型 */

  PointerType() : Type(POINTER_TI), pointType(NULL) {}
  PointerType(Type* t) : Type(POINTER_TI), pointType(t) {}
  std::string toString() override ;
};

class ArrayType : public Type {
  public:
  int elemNums; /* 数组元素个数 */
  Type* elemType; /* 数组元素类型 */

  ArrayType() : Type(ARRAY_TI), elemNums(0), elemType(NULL) {}
  ArrayType(int e, Type* t) : Type(ARRAY_TI), elemNums(e), elemType(t) {}
  std::string toString() override ;
  std::vector<int> getDims() 
  {
    std::vector<int> result;
    result.push_back(elemNums);
    if (elemType->typeIdtfr != TypeIdtfr::ARRAY_TI) {
      return result;
    }
    std::vector<int> tmp = ((ArrayType*)elemType)->getDims();
    for (u_long i = 0; i < tmp.size(); i++) {
      result.push_back(tmp[i]);
    }
    return result;
  }
};

enum ValueIdtfr {
  NONE_VI,
  MODULE_VI,
  FUNCTION_VI,
  BASICBLOCK_VI,
  INSTRUCTION_VI,
  NUMBER_CONSTANT_VI,
  STRING_CONSTANT_VI,
  LABEL_VI,
  VIRTREG_VI,
  FUNCRPARAMS_VI,
  FUNCFPARAM_VI,
  GLOBAL_VAR_VI,
  GLOBALINIT_VI,
};

class Value {
  public:
  ValueIdtfr valueIdtfr;
  Type* type;

  Value() : valueIdtfr(NONE_VI), type(NULL) {}
  Value(ValueIdtfr v, Type* t) : valueIdtfr(v), type(t) {}
  virtual std::string toString() = 0;
};

// class FuncDeclValue : public Value {
//   public:
//   std::string declare;
//   std::string toString() override ;
// };  

class Module : public Value {
  public:
  std::vector<GlobalValue*> globalVariables; /* GlobalValue */
  std::vector<std::string> funcDecl;
  std::vector<Function*> funcDef; 

  Module() : Value(MODULE_VI, new OtherType()) {}
  std::string toString() override ;
};

class Function : public Value {
  public:
  std::string funcName;
  Type* returnType;
  std::vector<FuncFParamValue*> funcFParamValues;  
  std::vector<BasicBlock*> basicBlocks;
  
  Function() : Value(FUNCTION_VI, new FunctionType()), returnType(NULL) {}
  std::string toString() override ;
};

class BasicBlock : public Value {
  public:
  LabelValue* label;
  std::vector<Instruction*> instructions;

  BasicBlock() : Value(BASICBLOCK_VI, new OtherType()), label(NULL) {}
  BasicBlock(LabelValue* lv) : Value(BASICBLOCK_VI, new OtherType()), label(lv) {}
  std::string toString() override ;
};


class NumberConstant : public Value {
  public:
  int value;

  NumberConstant(int v, int bitWidth) : Value(NUMBER_CONSTANT_VI, new IntegerType(bitWidth)), value(v) {}
  std::string toString() override ;
};

class StringConstant : public Value {
  public:
  std::string str;

  StringConstant(std::string s, Type* t) : Value(STRING_CONSTANT_VI ,t), str(s) {}
  std::string toString() override ;
};


class LabelValue : public Value {
  public:
  int id;
  LabelValue() : Value(LABEL_VI, new LabelType()), id(0) {}
  LabelValue(int _id) : Value(LABEL_VI, new LabelType()), id(_id) {}
  std::string toString() override;
};

class VirtRegValue : public Value {
  public:
  int id;
  // VirtRegValue() : Value(VIRTREG_VI, new Type(INCOMPLETE_TI)), id(0) {}
  VirtRegValue(int _id, Type* t) : Value(VIRTREG_VI, t), id(_id) {}

  int getId() { return this->id; }
  std::string toString() override ;
};

// class FuncRParamsValue : public Value {
//   public:
//   std::vector<Type*> types;
//   std::vector<Value*> values; /* 常数或虚拟寄存器 */
//   FuncRParamsValue() : Value(FUNCRPARAMS_VI, new OtherType()) {}
//   std::string toString() override ;
// };

class FuncFParamValue : public Value {
  public:
  VirtRegValue* value;  

  FuncFParamValue(Type* t, VirtRegValue* v) : Value(FUNCFPARAM_VI, t), value(v) {}
  std::string toString() override ;
};

class GlobalValue : public Value {
  public:
  std::string name;
  /* 全局变量的类型,需要为指针类型 */
  bool isConst;
  GlobalInitValue* globalInitValue; /* 可能为NULL */
  GlobalValue(Type* t) : Value(GLOBAL_VAR_VI, t), isConst(false), globalInitValue(NULL) {}
  std::string toString() override ;
  std::string globalDefToString();
};

/* 递归定义 */
class GlobalInitValue : public Value {
  public:
  bool isValue;
  std::vector<GlobalInitValue*> globalInitValues;
  Value* value; /* numberConstant or stringConstant */

  GlobalInitValue() : Value(GLOBALINIT_VI, new OtherType()),isValue(false), value(NULL) {}
  GlobalInitValue(Type* t, NumberConstant* n) : Value(GLOBALINIT_VI, t), isValue(true), value(n) {}
  GlobalInitValue(Type* t, StringConstant* s) : Value(GLOBALINIT_VI, t), isValue(true), value(s) {}
  std::string toString() override ;
};

enum InstIdtfr {
  NONE_II,
  BINARY_II,
  CALL_II,
  ALLOCA_II,
  LOAD_II,
  STORE_II,
  GEP_II,
  ICMP_II,
  ZEXT_II,
  BR_II,
  RET_II,
};

class Instruction : public Value {
  public:
  InstIdtfr instType;

  Instruction(InstIdtfr i) : Value(INSTRUCTION_VI, new OtherType()), instType(i) {}
  virtual std::string toString() = 0;
};

enum BinaryInstIdtfr {
  ADD_BII,
  SUB_BII,
  MUL_BII, 
  SDIV_BII,
  AND_BII,
  OR_BII,
  MOD_BII,
};

class BinaryInst : public Instruction {
  public:
  BinaryInstIdtfr binaryInstType;
  Value *op1, *op2, *result;
  BinaryInst(BinaryInstIdtfr b) : Instruction(BINARY_II), binaryInstType(b), op1(NULL), op2(NULL), result(NULL) {}
  std::string toString() override ;
};

class AllocaInst : public Instruction {
  public:
  Type* allocType;
  VirtRegValue* result;
  AllocaInst() : Instruction(ALLOCA_II), allocType(NULL), result(NULL) {}
  AllocaInst(Type* t, VirtRegValue* r) : Instruction(ALLOCA_II), allocType(t), result(r) {}
  std::string toString() override ;
};

class LoadInst : public Instruction {
  public:
  Value* pointer; /* 可能为寄存器或者全局变量名 */
  VirtRegValue* result;
  LoadInst() : Instruction(LOAD_II), pointer(NULL), result(NULL) {}
  std::string toString() override ;
};

class StoreInst : public Instruction {
  public:
  Value* value;
  Value* pointer;
  StoreInst() : Instruction(STORE_II), value(NULL), pointer(NULL) {}
  StoreInst(Value* v, Value* p) : Instruction(STORE_II), value(v), pointer(p) {}
  std::string toString() override ;
};

class GEPInst : public Instruction {
  public:
  Value* result;
  Type* elemType;
  Type* elemTypePointer;
  Value* ptrval;
  std::vector<Value*> indexs;

  GEPInst() : Instruction(GEP_II), result(NULL), elemType(NULL), elemTypePointer(NULL), ptrval(NULL) {}
  std::string toString() override ;
};

class ZextInst : public Instruction {
  public:
  Value *result, *value;
  ZextInst() : Instruction(ZEXT_II), result(NULL), value(NULL) {}
  std::string toString() override ;
};

enum ICMPCASE {
  NONE_ICMPCASE,
  EQ_ICMPCASE,
  NE_ICMPCASE,
  UGT_ICMPCASE,
  UGE_ICMPCASE,
  ULT_ICMPCASE,
  ULE_ICMPCASE,
  SGT_ICMPCASE,
  SGE_ICMPCASE,
  SLT_ICMPCASE,
  SLE_ICMPCASE,
};

class IcmpInst : public Instruction {
  public:
  ICMPCASE cond;
  Value *op1,*op2,*result;
  IcmpInst() : Instruction(ICMP_II), cond(NONE_ICMPCASE), op1(NULL), op2(NULL), result(NULL) {}
  std::string toString() override ;
};

class CallInst : public Instruction {
  public:
  Value* result; /* 可能为NULL */
  Type* returnType;
  std::string name;
  std::vector<Value*> args;
  CallInst() : Instruction(CALL_II), result(NULL), returnType(NULL) {}
  std::string toString() override ;
};

class RetInst : public Instruction {
  public:
  bool isVoid;
  Type* returnType; /* 可能为NULL */
  Value* value; /* 可能为NULL */
  RetInst() : Instruction(RET_II), isVoid(false), returnType(NULL), value(NULL) {}
  std::string toString() override ;
};

class BrInst : public Instruction {
  public:
  bool isUnCond;
  Value *iftrue, *iffalse, *dest, *cond;
  BrInst() : Instruction(BR_II), isUnCond(false), iftrue(NULL), iffalse(NULL), dest(NULL), cond(NULL) {}
  std::string toString() override ;
};



#endif

