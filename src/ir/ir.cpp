#include "ir.hpp"
#include "ir_build.hpp"
#include "ir_utils.hpp"
// std::string LabelValue::toString(bool withSign)
// {
//   std::string s;
//   if (withSign) {
//     s += "%";
//   }
//   s += "l";
//   s += std::to_string(this->id);
//   return s;
// }



std::string VoidType::toString()
{
  return "void";
}

std::string OtherType::toString()
{
  return "otherType";
}

std::string LabelType::toString()
{
  return "labelType";
}

std::string FunctionType::toString()
{
  return "funcType";
}

std::string IntegerType::toString()
{
  return "i" + std::to_string(this->bitWidth);
}

std::string PointerType::toString()
{
  return this->pointType->toString() + "*";
}

std::string ArrayType::toString()
{
  return "[" + std::to_string(this->elemNums) + " x " + this->elemType->toString() + "]";
}

std::string NumberConstant::toString()
{
  return std::to_string(this->value);
}

std::string StringConstant::toString()
{
  return "c\"" + this->str + "\"";
}

std::string LabelValue::toString()
{
  return "l" + std::to_string(this->id);
}

std::string VirtRegValue::toString()
{
  return "%reg" + std::to_string(this->id);
}

char binaryInstOp[][10] = {
  "add",
  "sub",
  "mul",
  "sdiv",
  "and",
  "or",
  "srem",
};

std::string BinaryInst::toString()
{
  std::string s;s += "\t";
  s += this->result->toString();
  s += " = ";
  s += binaryInstOp[this->binaryInstType];
  s += " ";
  s += this->result->type->toString();
  s += " ";
  s += this->op1->toString();
  s += ", ";
  s += this->op2->toString();
  s += "\n";
  return s;
}

std::string AllocaInst::toString()
{
  std::string s;s += "\t";
  s += this->result->toString();
  s += " = alloca ";
  s += this->allocType->toString();
  s += "\n";
  return s;
} 

std::string LoadInst::toString()
{ 
  std::string s;s += "\t";
  s += this->result->toString();
  s += " = load ";
  s += this->result->type->toString();
  s += ", ";
  s += this->pointer->type->toString();
  s += " ";
  s += this->pointer->toString();
  s += "\n";
  return s;
}

/* store <ty> <value>, <ty>* <pointer> */
std::string StoreInst::toString()
{
  std::string s;s += "\t";
  s += "store ";
  s += this->value->type->toString();
  s += " ";
  s += this->value->toString();
  s += ", ";
  s += this->pointer->type->toString();
  s += " ";
  s += this->pointer->toString();
  s += "\n";
  return s;
}
/* <result> = getelementptr <ty>, <ty>* <ptrval>, {<ty> <index>}* */
std::string GEPInst::toString()
{
  std::string s;s += "\t";
  s += this->result->toString();
  s += " = getelementptr ";
  s += this->elemType->toString();
  s += ", ";
  s += this->elemTypePointer->toString();
  s += " ";
  s += this->ptrval->toString();
  for (u_long i = 0; i < this->indexs.size(); i++) {
    s += ", ";
    s += this->indexs[i]->type->toString();
    s += " ";
    s += this->indexs[i]->toString();
  }
  s += "\n";
  return s;
}

/* `<result> = zext <ty> <value> to <ty2>` */
std::string ZextInst::toString()
{
  std::string s;s += "\t";
  s += this->result->toString();
  s += " = zext ";
  s += this->value->type->toString();
  s += " ";
  s += this->value->toString();
  s += " to ";
  s += this->result->type->toString();
  s += "\n";
  return s;
}
 
char icmpOp[][10] = {
  "none",
  "eq",
  "ne",
  "ugt",
  "uge",
  "ult",
  "ule",
  "sgt",
  "sge",
  "slt",
  "sle",
};

/* <result> = icmp <cond> <ty> <op1>, <op2> */ 
std::string IcmpInst::toString()
{
  std::string s;s += "\t";
  s += this->result->toString();
  s += " = icmp ";
  s += icmpOp[this->cond];
  s += " ";
  s += this->op1->type->toString();
  s += " ";
  s += this->op1->toString();
  s += ", ";
  s += this->op2->toString();
  s += "\n";
  return s;
}

/* `<result> = call [ret attrs] <ty> <fnptrval>(<function args>)` */
std::string CallInst::toString()
{
  std::string s;s += "\t";
  if (this->result != NULL) {
    s += this->result->toString();
    s += " = ";
  }
  s += "call ";
  s += this->returnType->toString();
  s += " @";
  s += this->name;
  s += "(";
  for (u_long i = 0; i < this->args.size(); i++) {
    s += this->args[i]->type->toString();
    s += " ";
    s += this->args[i]->toString();
    if (i < this->args.size() - 1) {
      s += ", ";
    }
  }
  s += ")\n";
  return s;
}

/* `ret <type> <value>` ,`ret void` */
std::string RetInst::toString()
{
  std::string s;
  s += "\t";
  s += "ret ";
  if (this->isVoid) {
    s += "void\n";
    return s;
  }
  s += this->returnType->toString();
  s += " ";
  s += this->value->toString();
  s += "\n";
  return s;
}

/* `br i1 <cond>, label <iftrue>, label <iffalse>``br label <dest>` */
std::string BrInst::toString()
{ 
  std::string s;
  s += "\t";
  if (this->isUnCond) {
    s += "br label %";
    s += this->dest->toString();
    s += "\n";
    return s;
  }
  s += "br i1 ";
  s += this->cond->toString();
  s += ", label %";
  s += this->iftrue->toString();
  s += ", label %";
  s += this->iffalse->toString();
  s += "\n";
  return s;
}

std::string PhiInst::toString()
{
  std::string s;
  s += "\t";
  s += this->result->toString();
  s += " = phi i32 ";
  for (u_long i = 0; i < this->vardefs.size(); i++) {
    std::pair<Value*, LabelValue*> it = this->vardefs[i];
    s += "[ ";
    s += it.first->toString();
    s += ", %";
    s += it.second->toString();
    s += " ]";
    if (i < this->vardefs.size() - 1) {
      s += ", ";
    }
  }
  s += "\n";
  return s;
}


std::string BasicBlock::toString()
{
  if(this->instructions.size() == 0) {
    return "";
  }
  std::string s;
  s += this->label->toString();
  s += ":\n\n";
  for (Instruction* inst : this->instructions) {
    s += inst->toString();
  }
  s += "\n";
  return s;
}

std::string FuncFParamValue::toString()
{
  std::string s;
  s += this->type->toString();
  s += " ";
  s += this->value->toString();
  return s;
}

std::string Function::toString()
{
  std::string s;
  s += "define ";
  s += this->returnType->toString();
  s += " @";
  s += this->funcName;
  s += "(";
  for (u_long i = 0; i < this->funcFParamValues.size(); i++) {
    s += this->funcFParamValues[i]->toString();
    if (i < this->funcFParamValues.size() - 1) {
      s += ", ";
    }
  }
  s += ") {\n";
  for (u_long i = 0; i < this->basicBlocks.size(); i++) {
    s += this->basicBlocks[i]->toString();
  }
  s += "}\n";
  return s;
}

std::string GlobalValue::globalDefToString()
{
  std::string s;
  s += "@";
  s += this->name;
  s += " = ";
  if (this->isConst) {
    s += "constant ";
  } else {
    s += "global ";
  }

  if (this->globalInitValue == NULL) {
    s += ptrType2Type(this->type)->toString();/* 注意全局变量类型是指针类型  */
    s += " ";
    s += "zeroinitializer\n";
    return s;
  }
  s += this->globalInitValue->toString();
  s += "\n";
  return s;
}

std::string GlobalValue::toString()
{
  std::string s;
  s += "@";
  s += this->name;
  return s;
}

std::string GlobalInitValue::toString()
{
  std::string s;
  s += this->type->toString();
  s += " ";
  if (this->isValue) {
    s += this->value->toString();
    return s;
  }
  s += "[ ";
  for (u_long i = 0; i < this->globalInitValues.size(); i++) {
    s += this->globalInitValues[i]->toString();
    if (i < this->globalInitValues.size() - 1) {
      s += ", ";
    }
  } 
  s += "]";
  return s;
}

std::string Module::toString()
{
  std::string s;
  for (u_long i = 0; i < this->funcDecl.size(); i++) {
    s += this->funcDecl[i];
    s += "\n";
  }
  for (u_long i = 0; i < this->globalVariables.size(); i++) {
    s += this->globalVariables[i]->globalDefToString();
  }
  for (u_long i = 0; i < this->funcDef.size(); i++) {
    s += this->funcDef[i]->toString();
  }
  return s;
}

void BinaryInst::updateUseValue(Value* oldv, Value* newv)
{
  if (op1 == oldv) {
    op1 = newv;
  }
  if (op2 == oldv) {
    op2 = newv;
  }
}

void AllocaInst::updateUseValue(Value* oldv, Value* newv)
{
}

void LoadInst::updateUseValue(Value* oldv, Value* newv)
{
}

void StoreInst::updateUseValue(Value* oldv, Value* newv)
{
  if (value == oldv) {
    value = newv;
  }
}

void GEPInst::updateUseValue(Value* oldv, Value* newv)
{
  if (ptrval == oldv) {
    ptrval = newv;
  }
  for (u_long i = 0; i < indexs.size(); i++) {
    if (indexs[i] == oldv) {
      indexs[i] = newv;
    }
  }
}

void ZextInst::updateUseValue(Value* oldv, Value* newv)
{
  if (value == oldv) {
    value = newv;
  }
}

void IcmpInst::updateUseValue(Value* oldv, Value* newv)
{
  if (op1 == oldv) {
    op1 = newv;
  }
  if (op2 == oldv) {
    op2 = newv;
  }
}

void CallInst::updateUseValue(Value* oldv, Value* newv)
{
  for (u_long i = 0; i < args.size(); i++) {
    if (args[i] == oldv) {
      args[i] = newv;
    }
  }
}

void RetInst::updateUseValue(Value* oldv, Value* newv)
{
  if (value == oldv) {
    value = newv;
  }
}

void BrInst::updateUseValue(Value* oldv, Value* newv)
{
  if (cond == oldv) {
    cond = newv;
  }
}

void PhiInst::updateUseValue(Value* oldv, Value* newv)
{
  for (u_long i = 0; i < vardefs.size(); i++) {
    if (vardefs[i].first == oldv) {
      vardefs[i].first = newv;
    }
  }
}

bool BinaryInst::isUseThisReg(int virtRegId)
{
  if (op1->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    int op1Id = ((VirtRegValue*)op1)->getId();
    if (op1Id == virtRegId) {
      return true;
    }
  }

  if (op2->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    int op2Id = ((VirtRegValue*)op2)->getId();
    if (op2Id == virtRegId) {
      return true;
    }
  }
  return false;
}

bool AllocaInst::isUseThisReg(int virtRegId)
{
  return false;
}

bool LoadInst::isUseThisReg(int virtRegId)
{
  if (pointer->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    int id = ((VirtRegValue*)pointer)->getId();
    if (id == virtRegId) {
      return true;
    }
  }
  return false;
}

bool StoreInst::isUseThisReg(int virtRegId)
{
  int id;
  if (pointer->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    id = ((VirtRegValue*)pointer)->getId();
    if (id == virtRegId) {
      return true;
    }
  }
  
  if (value->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    id = ((VirtRegValue*)value)->getId();
    if (id == virtRegId) {
      return true;
    }
  }
  return false;
}

bool GEPInst::isUseThisReg(int virtRegId)
{
  int id;
  if (ptrval->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    id = ((VirtRegValue*)ptrval)->getId();
    if (id == virtRegId) {
      return true;
    }
  }
  
  for (Value* v : indexs) {
    if (v->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
      id = ((VirtRegValue*)v)->getId();
      if (id == virtRegId) {
        return true;
      }
    }
  }
  return false;
}

bool ZextInst::isUseThisReg(int virtRegId)
{
  if (value->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    int id = ((VirtRegValue*)value)->getId();
    return id == virtRegId;
  }
  return false;
}

bool IcmpInst::isUseThisReg(int virtRegId)
{
  if (op1->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    int op1Id = ((VirtRegValue*)op1)->getId();
    if (op1Id == virtRegId) {
      return true;
    }
  }

  if (op2->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    int op2Id = ((VirtRegValue*)op2)->getId();
    if (op2Id == virtRegId) {
      return true;
    }
  }
  return false;
}

bool CallInst::isUseThisReg(int virtRegId)
{
  int id;
  for (Value* v : args) {
    if (v->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
      id = ((VirtRegValue*)v)->getId();
      if (id == virtRegId) {
        return true;
      }
    }
  }
  return false;
}

bool RetInst::isUseThisReg(int virtRegId)
{
  if (isVoid) {
    return false;
  }
  if (value->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    int id = ((VirtRegValue*)value)->getId();
    return id == virtRegId;
  }
  return false;
}

bool BrInst::isUseThisReg(int virtRegId)
{
  if (isUnCond) {
    return false;
  }
  if (cond->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
    int id = ((VirtRegValue*)cond)->getId();
    return id == virtRegId;
  }
  return false;
}

bool PhiInst::isUseThisReg(int virtRegId)
{
  for (std::pair<Value*, LabelValue*> def : vardefs) {
    if (def.first->valueIdtfr == ValueIdtfr::VIRTREG_VI) {
      int id = ((VirtRegValue*)def.first)->getId();
      if (id == virtRegId) {
        return true;
      }
    }
  }
  return false;
}

Value* BinaryInst::getDefReg()
{
  return result;
}

int BinaryInst::getDefRegId()
{
  return ((VirtRegValue*)result)->getId();
}

std::set<int> BinaryInst::getUseRegIds()
{
  std::set<int> r;
  if (isVirtRegValue(op1)) {
    r.insert(((VirtRegValue*)op1)->getId());
  }
  if (isVirtRegValue(op2)) {
    r.insert(((VirtRegValue*)op2)->getId());
  }
  return r;
}

Value* AllocaInst::getDefReg()
{
  return result;
}

int AllocaInst::getDefRegId()
{
  return ((VirtRegValue*)result)->getId();
}

std::set<int> AllocaInst::getUseRegIds()
{
  std::set<int> r;
  return r;
}

Value* LoadInst::getDefReg()
{
  return result;
}

int LoadInst::getDefRegId()
{
  return ((VirtRegValue*)result)->getId();
}

std::set<int> LoadInst::getUseRegIds()
{
  std::set<int> r;
  if (isVirtRegValue(pointer)) {
    r.insert(((VirtRegValue*)pointer)->getId());
  }
  return r;
}

Value* StoreInst::getDefReg()
{
  return NULL;
}

int StoreInst::getDefRegId()
{
  return -1;
}

std::set<int> StoreInst::getUseRegIds()
{
  std::set<int> r;
  if (isVirtRegValue(pointer)) {
    r.insert(((VirtRegValue*)pointer)->getId());
  }
  if (isVirtRegValue(value)) {
    r.insert(((VirtRegValue*)value)->getId());
  }
  return r;
}

Value* GEPInst::getDefReg()
{
  return result;
}

int GEPInst::getDefRegId()
{ 
  return ((VirtRegValue*)result)->getId();
}

std::set<int> GEPInst::getUseRegIds()
{
  std::set<int> r;
  if (isVirtRegValue(ptrval)) {
    r.insert(((VirtRegValue*)ptrval)->getId());
  }
  for (Value* v : indexs) {
    if (isVirtRegValue(v)) {
      r.insert(((VirtRegValue*)v)->getId());
    }
  }
  return r;
}
Value* ZextInst::getDefReg()
{
  return result;
}

int ZextInst::getDefRegId()
{
  return ((VirtRegValue*)result)->getId();
}

std::set<int> ZextInst::getUseRegIds()
{
  std::set<int> r;
  if (isVirtRegValue(value)) {
    r.insert(((VirtRegValue*)value)->getId());
  }
  return r;
}

Value* IcmpInst::getDefReg()
{
  return result;
}

int IcmpInst::getDefRegId()
{
  return ((VirtRegValue*)result)->getId();
}

std::set<int> IcmpInst::getUseRegIds()
{
  std::set<int> r;
  if (isVirtRegValue(op1)) {
    r.insert(((VirtRegValue*)op1)->getId());
  } 
  if (isVirtRegValue(op2)) {
    r.insert(((VirtRegValue*)op2)->getId());
  }
  return r;
}

Value* CallInst::getDefReg()
{
  return result;
}

int CallInst::getDefRegId()
{
  if (result == NULL) {
    return -1;
  }
  return ((VirtRegValue*)result)->getId();
}

std::set<int> CallInst::getUseRegIds()
{
  std::set<int> r;
  for (Value* v : args) {
    if (isVirtRegValue(v)) {
      r.insert(((VirtRegValue*)v)->getId());
    }
  }
  return r;
}

Value* RetInst::getDefReg()
{
  return NULL;
}

int RetInst::getDefRegId()
{
  return -1;
}

std::set<int> RetInst::getUseRegIds()
{
  std::set<int> r;
  if (!isVoid) {
    if (isVirtRegValue(value)) {
      r.insert(((VirtRegValue*)value)->getId());
    }
  }
  return r;
}

Value* BrInst::getDefReg()
{
  return NULL;
}

int BrInst::getDefRegId()
{
  return -1;
}

std::set<int> BrInst::getUseRegIds()
{
  std::set<int> r;
  if (!isUnCond) {
    if (isVirtRegValue(cond)) {
      r.insert(((VirtRegValue*)cond)->getId());
    }
  }
  return r;
}

Value* PhiInst::getDefReg()
{
  return result;
}

int PhiInst::getDefRegId()
{
  return ((VirtRegValue*)result)->getId();
}

std::set<int> PhiInst::getUseRegIds()
{
  std::set<int> r;
  for (std::pair<Value*, LabelValue*> it : vardefs) {
    if (isVirtRegValue(it.first)) {
      r.insert(((VirtRegValue*)it.first)->getId());
    }
  }
  return r;
}