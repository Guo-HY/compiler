#include "syntax_tree.hpp"

extern char tokenName[][20];

char tokenValue[][20] = {
  "",
  "", 
  "",
  "main\n",
  "const\n",
  "int\n",
  "break\n",
  "continue\n",
  "if\n",
  "else\n",
  "!\n",
  "&&\n",
  "||\n",
  "while\n",
  "getint\n",
  "printf\n",
  "return\n",
  "+\n",
  "-\n",
  "void\n",
  "*\n",
  "/\n",
  "%\n",
  "<\n",
  "<=\n",
  ">\n",
  ">=\n",
  "==\n",
  "!=\n",
  "=\n",
  ";\n",
  ",\n",
  "(\n",
  ")\n",
  "[\n",
  "]\n",
  "{\n",
  "}\n",
};


std::string tokenToString(TokenType t) 
{
  std::string s;
  s += tokenName[t];
  s += " ";
  s += tokenValue[t];
  return s;
}

std::string CompUnitNode::toString()
{
  std::string s;
  int i;
  for (i = 0; i < declNodes.size(); i++) {
    s += declNodes[i]->toString();
  }
  for (i = 0; i < funcDefNodes.size(); i++) {
    s += funcDefNodes[i]->toString();
  }
  s += mainFuncDefNode->toString();
  s += "<CompUnit>\n";
  return s;
}

std::string DeclNode::toString()
{
  std::string s;
  if (declNodeType == DeclNodeType::DECL_CONST) {
    s += constDeclNode->toString();
  } else if (declNodeType == DeclNodeType::DECL_VAR) {
    s += varDeclNode->toString();
  } else {
    Log("error\n");
  }
  return s;
}

std::string ConstDeclNode::toString()
{
  std::string s;
  int i;
  s += tokenToString(TokenType::CONSTTK);
  s += bTypeNode->toString();
  if (constDefNodes.size() == 0) {
    Log("error\n");
  }
  // Assert(constDefNodes.size() > 0, "error");
  s += constDefNodes[0]->toString();
  for (i = 1; i < constDefNodes.size(); i++) {
    s += tokenToString(TokenType::COMMA);
    s += constDefNodes[i]->toString();
  }
  s += tokenToString(TokenType::SEMICN);
  s += "<ConstDecl>\n";
  return s;
}

std::string BTypeNode::toString()
{
  std::string s;
  s += tokenToString(TokenType::INTTK);
  return s;
}

std::string ConstDefNode::toString()
{
  std::string s;
  s += tokenToString(TokenType::IDENFR);
  s += ident->str;
  s += "\n";

  for (int i = 0; i < constExpNodes.size(); i++) {
    s += tokenToString(TokenType::LBRACK);
    s += constExpNodes[i]->toString();
    s += tokenToString(TokenType::RBRACK);
  }
  s += tokenToString(TokenType::ASSIGN);
  s += constInitValNode->toString();
  s += "<ConstDef>\n";
  return s;
}

std::string ConstInitValNode::toString()
{
  std::string s;
  if (initArray) {
    s += tokenToString(TokenType::LBRACE);
    for (int i = 0; i < constInitValNodes.size(); i++) {
      if (i > 0) {
        s += tokenToString(TokenType::COMMA);
      }
      s += constInitValNodes[i]->toString();
    }
    s += tokenToString(TokenType::RBRACE);
  } else {
    s += constExpNode->toString();
  }
  s += "<ConstInitVal>\n";
  return s;
}

std::string VarDeclNode::toString()
{
  std::string s;
  s += bTypeNode->toString();
  s += varDefNodes[0]->toString();
  for (int i = 1;i < varDefNodes.size(); i++) {
    s += tokenToString(TokenType::COMMA);
    s += varDefNodes[i]->toString();
  }
  s += tokenToString(TokenType::SEMICN);
  s += "<VarDecl>\n";
  return s;
}

std::string VarDefNode::toString()
{ 
  std::string s;
  s += tokenToString(TokenType::IDENFR);
  s += ident->str;
  s += "\n";
  for (int i = 0; i < constExpNodes.size(); i++) {
    s += tokenToString(TokenType::LBRACK);
    s += constExpNodes[i]->toString();
    s += tokenToString(TokenType::RBRACK);
  }
  if (hasInitVal) {
    s += tokenToString(TokenType::ASSIGN);
    s += initValNode->toString();
  }
  s += "<VarDef>\n";
  return s;
}

std::string InitValNode::toString()
{
  std::string s;
  if (initArray) {
    s += tokenToString(TokenType::LBRACE);
    for (int i = 0; i < initValNodes.size(); i++) {
      if (i > 0) {
        s += tokenToString(TokenType::COMMA);
      }
      s += initValNodes[i]->toString();
    }
    s += tokenToString(TokenType::RBRACE);
  } else {
    s += expNode->toString();
  }
  s += "<InitVal>\n";
  return s;
}

std::string FuncDefNode::toString()
{
  std::string s;
  s += funcTypeNode->toString();
  s += tokenToString(TokenType::IDENFR);
  s += ident->str;
  s += "\n";
  s += tokenToString(TokenType::LPARENT);
  if (hasFuncFParams) {
    s += funcFParamsNode->toString();
  }
  s += tokenToString(TokenType::RPARENT);
  s += blockNode->toString();
  s += "<FuncDef>\n";
  return s;
}

std::string MainFuncDefNode::toString()
{
  std::string s;  
  s += tokenToString(TokenType::INTTK);
  s += tokenToString(TokenType::MAINTK);
  s += tokenToString(TokenType::LPARENT);
  s += tokenToString(TokenType::RPARENT);
  s += blockNode->toString();
  s += "<MainFuncDef>\n";
  return s;
} 

std::string FuncTypeNode::toString()
{
  std::string s;
  s += tokenToString(funcType->tokenType);
  s += "<FuncType>\n";
  return s;
}

std::string FuncFParamsNode::toString()
{
  std::string s;
  s += funcFParamNodes[0]->toString();
  for (int i = 1; i < funcFParamNodes.size(); i++) {
    s += tokenToString(TokenType::COMMA);
    s += funcFParamNodes[i]->toString();
  }
  s += "<FuncFParams>\n";
  return s;
}

std::string FuncFParamNode::toString()
{
  std::string s;
  s += bTypeNode->toString();
  s += tokenToString(TokenType::IDENFR);
  s += ident->str;
  s += "\n";
  if (arrayDimension > 0) {
    s += tokenToString(TokenType::LBRACK);
    s += tokenToString(TokenType::RBRACK);
  }
  for (int i = 0; i < constExpNodes.size(); i++) {
    s += tokenToString(TokenType::LBRACK);
    s += constExpNodes[i]->toString();
    s += tokenToString(TokenType::RBRACK);
  }
  s+= "<FuncFParam>\n";
  return s;
}

std::string BlockNode::toString()
{
  std::string s;
  s += tokenToString(TokenType::LBRACE);
  for (int i = 0; i < blockItemNodes.size(); i++) {
    s += blockItemNodes[i]->toString();
  }
  s += tokenToString(TokenType::RBRACE);
  s += "<Block>\n";
  return s;
}

std::string BlockItemNode::toString()
{
  std::string s;
  if (blockItemType == BlockItemType::BLOCKITEM_DECL) {
    s += declNode->toString();
  } else if (blockItemType == BlockItemType::BLOCKITEM_STMT) {
    s += stmtNode->toString();
  }
  return s;
}

std::string StmtNode::toString()
{
  std::string s;
  switch (stmtType)
  {
  case STMT_PRINTF:
    s += tokenToString(TokenType::PRINTFTK);
    s += tokenToString(TokenType::LPARENT);
    s += tokenToString(TokenType::STRCON);
    s += formatString->str;
    s += "\n";
    for (int i = 0; i < expNodes.size(); i++) {
      s += tokenToString(TokenType::COMMA);
      s += expNodes[i]->toString();
    }
    s += tokenToString(TokenType::RPARENT);
    s += tokenToString(TokenType::SEMICN);
    break;
  case STMT_RETURN:
    s += tokenToString(TokenType::RETURNTK);
    if (expNodes.size() > 0) {
      s += expNodes[0]->toString();
    }
    s += tokenToString(TokenType::SEMICN);
    break;
  case STMT_BREAK:
    s += tokenToString(TokenType::BREAKTK);
    s += tokenToString(TokenType::SEMICN);
    break;
  case STMT_CONTINUE:
    s += tokenToString(TokenType::CONTINUETK);
    s += tokenToString(TokenType::SEMICN);
    break;
  case STMT_WHILE:
    s += tokenToString(TokenType::WHILETK);
    s += tokenToString(TokenType::LPARENT);
    s += condNode->toString();
    s += tokenToString(TokenType::RPARENT);
    s += stmtNodes[0]->toString();
    break;
  case STMT_IF:
    s += tokenToString(TokenType::IFTK);
    s += tokenToString(TokenType::LPARENT);
    s += condNode->toString();
    s += tokenToString(TokenType::RPARENT);
    s += stmtNodes[0]->toString();
    if (hasElse) {
      s += tokenToString(TokenType::ELSETK);
      s += stmtNodes[1]->toString();
    }
    break;
  case STMT_BLOCK:
    s += blockNode->toString();
    break;
  case STMT_ASSIGN:
    s += lValNode->toString();
    s += tokenToString(TokenType::ASSIGN);
    s += expNodes[0]->toString();
    s += tokenToString(TokenType::SEMICN);
    break;
  case STMT_GETINT:
    s += lValNode->toString();
    s += tokenToString(TokenType::ASSIGN);
    s += tokenToString(TokenType::GETINTTK);
    s += tokenToString(TokenType::LPARENT);
    s += tokenToString(TokenType::RPARENT);
    s += tokenToString(TokenType::SEMICN);
    break;
  case STMT_EXP:
    if (expNodes.size() > 0) {
      s += expNodes[0]->toString();
    }
    s += tokenToString(TokenType::SEMICN);
    break;
  default:
    Log("error\n");
    break;
  }
  s += "<Stmt>\n";
  return s;
}

std::string ExpNode::toString()
{
  std::string s;
  s += addExpNode->toString();
  s += "<Exp>\n";
  return s;
}

std::string CondNode::toString()
{
  std::string s;
  s += lOrExpNode->toString();
  s += "<Cond>\n";
  return s;
}

std::string LValNode::toString()
{
  std::string s;
  s += tokenToString(TokenType::IDENFR);
  s += ident->str;
  s += "\n";
  for (int i = 0; i < expNodes.size(); i++) {
    s += tokenToString(TokenType::LBRACK);
    s += expNodes[i]->toString();
    s += tokenToString(TokenType::RBRACK);
  }
  s += "<LVal>\n";
  return s;
}

std::string PrimaryExpNode::toString()
{
  std::string s;
  switch (primaryExpType)
  {
  case PRIMARY_EXP:
    s += tokenToString(TokenType::LPARENT);
    s += expNode->toString();
    s += tokenToString(TokenType::RPARENT);
    break;
  case PRIMARY_LVAL:
    s += lValNode->toString();
    break;
  case PRIMARY_NUMBER:
    s += numberNode->toString();
    break;
  default:
    Log("error\n");
    break;
  }
  s += "<PrimaryExp>\n";
  return s;
}

std::string NumberNode::toString()
{
  std::string s;
  s += tokenToString(TokenType::INTCON);
  s += intConst->str;
  s += "\n";
  s += "<Number>\n";
  return s;
}

std::string UnaryExpNode::toString()
{
  std::string s;
  switch (unaryExpType)
  {
  case UNARY_PREFIX:
    s += unaryOpNode->toString();
    s += unaryExpNode->toString();
    break;
  case UNARY_FUNCCALL:
    s += tokenToString(TokenType::IDENFR);
    s += ident->str;
    s += "\n";
    s += tokenToString(TokenType::LPARENT);
    if (hasFuncRParams) {
      s += funcRParamsNode->toString();
    }
    s += tokenToString(TokenType::RPARENT); 
    break;
  case UNARY_PRIMARYEXP:
    s += primaryExpNode->toString();
    break;
  default:
    Log("error\n");
    break;
  }
  s += "<UnaryExp>\n";
  return s;
}

std::string UnaryOpNode::toString()
{
  std::string s;
  s += tokenToString(unaryOp->tokenType);
  s += "<UnaryOp>\n";
  return s;
}

std::string FuncRParamsNode::toString()
{
  std::string s;
  s += expNodes[0]->toString();
  for (int i = 1; i < expNodes.size(); i++) {
    s += tokenToString(TokenType::COMMA);
    s += expNodes[i]->toString();
  }
  s += "<FuncRParams>\n";
  return s;
} 

std::string MulExpNode::toString()
{
  std::string s;
  for (int i = 0; i < unaryExpNodes.size(); i++) {
    s += unaryExpNodes[i]->toString();
    s += "<MulExp>\n";
    if (i < ops.size()) {
      s += tokenToString(ops[i]->tokenType);
    }
  }
  return s;
}

std::string AddExpNode::toString()
{
  std::string s;
  for (int i = 0; i < MulExpNodes.size(); i++) {
    s += MulExpNodes[i]->toString();
    s += "<AddExp>\n";
    if (i < ops.size()) {
      s += tokenToString(ops[i]->tokenType);
    }
  }
  return s;
}

std::string RelExpNode::toString()
{
  std::string s;
  for (int i = 0; i < addExpNodes.size(); i++) {
    s += addExpNodes[i]->toString();
    s += "<RelExp>\n";
    if (i < ops.size()) {
      s += tokenToString(ops[i]->tokenType);
    }
  }
  return s;
}

std::string EqExpNode::toString()
{
  std::string s;
  for (int i = 0; i < relExpNodes.size(); i++) {
    s += relExpNodes[i]->toString();
    s += "<EqExp>\n";
    if (i < ops.size()) {
      s += tokenToString(ops[i]->tokenType);
    }
  }
  return s;
}

std::string LAndExpNode::toString()
{
  std::string s;
  for (int i = 0; i < eqExpNodes.size(); i++) {
    s += eqExpNodes[i]->toString();
    s += "<LAndExp>\n";
    if (i < ops.size()) {
      s += tokenToString(ops[i]->tokenType);
    }
  }
  return s;
}

std::string LOrExpNode::toString()
{
  std::string s;
  for (int i = 0; i < lAndExpNodes.size(); i++) {
    s += lAndExpNodes[i]->toString();
    s += "<LOrExp>\n";
    if (i < ops.size()) {
      s += tokenToString(ops[i]->tokenType);
    }
  }
  return s;
}

std::string ConstExpNode::toString()
{
  std::string s;
  s += addExpNode->toString();
  s += "<ConstExp>\n";
  return s;
}