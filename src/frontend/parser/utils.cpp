#include "syntax_tree.hpp"
#include "symbol_table.hpp"

extern SymbolTable* currentSymbolTable;
extern SymbolTable* globalSymbolTable;

int ExpNode::getConstValue()
{
  if (this->alreadyComputeValue) {
    return this->value;
  }
  this->value = this->addExpNode->getConstValue();
  this->alreadyComputeValue = true;
  return this->value;
}

int ConstExpNode::getConstValue() 
{
  if (this->alreadyComputeValue) {
    return this->value;
  }
  this->value = this->addExpNode->getConstValue();
  this->alreadyComputeValue = true;
  return this->value;
}

int AddExpNode::getConstValue()
{
  int result = ((MulExpNode*)(this->operands[0]))->getConstValue();
  for (u_long i = 1; i < this->operands.size(); i++) {
    int tmp = ((MulExpNode*)(this->operands[i]))->getConstValue();
    switch (this->ops[i-1]->tokenType)
    {
    case TokenType::PLUS:
      result += tmp;
      break;
    case TokenType::MINU:
      result -= tmp;
      break;
    default:
      panic("error");
      break;
    }
  }
  return result;
}

int MulExpNode::getConstValue()
{
  int result = this->unaryExpNodes[0]->getConstValue();
  for (u_long i = 1; i < this->unaryExpNodes.size(); i++) {
    int tmp = this->unaryExpNodes[i]->getConstValue();
    switch (this->ops[i-1]->tokenType)
    {
    case TokenType::MULT:
      result *= tmp;
      break;
    case TokenType::DIV:
      result /= tmp;
      break;
    case TokenType::MOD:
      result %= tmp;
      break;
    default:
      panic("error");
      break;
    }
  }
  return result;
}

int UnaryExpNode::getConstValue()
{
  switch (this->unaryExpType)
  {
  case UnaryExpType::UNARY_PRIMARYEXP:
    return this->primaryExpNode->getConstValue();
    break;
  case UnaryExpType::UNARY_PREFIX:
    switch (this->unaryOpNode->unaryOp->tokenType)
    {
    case TokenType::PLUS:
      return this->unaryExpNode->getConstValue();
      break;
    case TokenType::MINU:
      return this->unaryExpNode->getConstValue() * -1;
    case TokenType::NOT:
      return !(this->unaryExpNode->getConstValue());
    default:
      panic("error");
      break;
    }
  default:
    panic("error");
    break;
  }
  panic("error");
}

int PrimaryExpNode::getConstValue()
{
  switch (this->primaryExpType)
  {
  case PrimaryExpType::PRIMARY_EXP:
    return this->expNode->getConstValue();
    break;
  case PrimaryExpType::PRIMARY_NUMBER:
    return this->numberNode->getConstValue();
  case PrimaryExpType::PRIMARY_LVAL:
    return this->lValNode->getConstValue();
  default:
    panic("error");
    break;
  }
}

int NumberNode::getConstValue() 
{
  return this->intConst->value;
}

int LValNode::getConstValue()
{
  std::vector<int> ptrs;
  if (this->expNodes.size() > 0) {
    for (u_long i = 0; i < this->expNodes.size(); i++) {
      ptrs.push_back(this->expNodes[i]->getConstValue());
    }
  } else {
    ptrs.push_back(0);
  }
  int value = currentSymbolTable->getInitValue(&(this->ident->str), ptrs);
  Log("value = %d", value);
  return value;
}