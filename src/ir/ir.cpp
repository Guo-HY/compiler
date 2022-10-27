#include "ir.hpp"
#include "ir_build.hpp"
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


std::string BasicBlock::toString()
{
  if(this->instructions.size() == 0) {
    return "";
  }
  std::string s;
  s += this->label->toString();
  s += ":\n\n";
  for (u_long i = 0; i < this->instructions.size(); i++) {
    s += this->instructions[i]->toString();
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