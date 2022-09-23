#include "parser.hpp"
#include "../include/error.hpp"

extern char tokenName[][20];
extern char tokenValue[][20];

TokenInfo* Parser::popToken() 
{
  if (tokenInfoList[nowTokenListPtr]->tokenType == TokenType::END) {
    Log("error : pop END\n");
    return NULL;
  }
  TokenInfo* now = tokenInfoList[nowTokenListPtr];
  nowTokenListPtr++;
  return now;
}

TokenInfo* Parser::peekToken(int num)
{
  if (nowTokenListPtr + num >= tokenInfoList.size()) {
    Log("error : peek out of bounds\n");
  }
  return tokenInfoList[nowTokenListPtr + num];
}

CompUnitNode* Parser::compUnitAnalyse() 
{
  CompUnitNode* node = new CompUnitNode();
  /* deal Decl : CONSTTK | INTTK IDENFR !LPARENT */ 
  while ((peekToken(0)->tokenType == TokenType::CONSTTK) || 
        (peekToken(0)->tokenType == TokenType::INTTK && 
        peekToken(1)->tokenType == TokenType::IDENFR && 
        peekToken(2)->tokenType != TokenType::LPARENT)) {
          node->declNodes.push_back(declAnalyse());
        }
  /* deal FuncDef : VOIDTK | INTTK !MAINTK */
  while ((peekToken(0)->tokenType == TokenType::VOIDTK) ||
        (peekToken(0)->tokenType == TokenType::INTTK &&
        peekToken(1)->tokenType != TokenType::MAINTK)) {
          node->funcDefNodes.push_back(funcDefAnalyse());
        }

  node->mainFuncDefNode = mainFuncDefAnalyse();
  return node;
}

DeclNode* Parser::declAnalyse()
{
  DeclNode* node = new DeclNode();
  if (peekToken(0)->tokenType == TokenType::CONSTTK) {
    node->declNodeType = DeclNodeType::DECL_CONST;
    node->constDeclNode = constDeclAnalyse();
  } else {
    node->declNodeType = DeclNodeType::DECL_VAR;
    node->varDeclNode = varDeclAnalyse();
  }
  return node;
}

ConstDeclNode* Parser::constDeclAnalyse()
{
  ConstDeclNode* node = new ConstDeclNode();
  popToken(); /* eat CONSTTK */
  node->bTypeNode = bTypeAnalyse();
  node->constDefNodes.push_back(constDefAnalyse());
  while (peekToken(0)->tokenType == TokenType::COMMA) {
    popToken(); /* eat COMMA */
    node->constDefNodes.push_back(constDefAnalyse());
  }
  popToken(); /* eat SEMICN */
  return node;
}

BTypeNode* Parser::bTypeAnalyse()
{
  BTypeNode* node = new BTypeNode();
  popToken(); /* eat INTTK */
  return node;
}

ConstDefNode* Parser::constDefAnalyse()
{
  ConstDefNode* node = new ConstDefNode();
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */

  /* deal { '[' ConstExp ']' } */
  while (peekToken(0)->tokenType == TokenType::LBRACK) {
    popToken(); /* eat LBRACK */
    (node->arrayDimension)++;
    node->constExpNodes.push_back(constExpAnalyse());
    popToken(); /* eat RBRACK */
  }
  popToken(); /* eat ASSIGN */
  node->constInitValNode = constInitValAnalyse();
  return node;
}

ConstInitValNode* Parser::constInitValAnalyse()
{ 
  ConstInitValNode* node = new ConstInitValNode();
  if (peekToken(0)->tokenType == TokenType::LBRACE) {
    node->initArray = true;
    popToken(); /* eat LBRACE */
    if (peekToken(0)->tokenType == TokenType::RBRACE) {
      popToken(); /* eat RBRACE */
      return node;
    }
    node->constInitValNodes.push_back(constInitValAnalyse());
    while (peekToken(0)->tokenType == TokenType::COMMA) {
      popToken(); /* eat COMMA */
      node->constInitValNodes.push_back(constInitValAnalyse());
    }
    popToken(); /* eat RBRACE */
    return node;
  }
  node->initArray = false;
  node->constExpNode = constExpAnalyse();
  return node;
}

VarDeclNode* Parser::varDeclAnalyse()
{
  VarDeclNode* node = new VarDeclNode();
  node->bTypeNode = bTypeAnalyse();
  node->varDefNodes.push_back(varDefAnalyse());
  while (peekToken(0)->tokenType == TokenType::COMMA) {
    popToken(); /* eat COMMA */
    node->varDefNodes.push_back(varDefAnalyse());
  }
  popToken(); /* eat SEMICN */
  return node;
} 

VarDefNode* Parser::varDefAnalyse()
{
  VarDefNode* node = new VarDefNode();
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */
  while (peekToken(0)->tokenType == TokenType::LBRACK) {
    popToken(); /* eat LBRACK */
    node->constExpNodes.push_back(constExpAnalyse());
    popToken(); /* eat RBRACK */
  } 
  if (peekToken(0)->tokenType == TokenType::ASSIGN) {
    popToken(); /* eat ASSIGN */
    node->hasInitVal = true;
    node->initValNode = initValAnalyse();
  }
  return node;
}

InitValNode* Parser::initValAnalyse()
{
  InitValNode* node = new InitValNode();
  if (peekToken(0)->tokenType == TokenType::LBRACE) {
    popToken(); /* eat LBRACE */
    node->initArray = true;
    if (peekToken(0)->tokenType == TokenType::RBRACE) {
      popToken(); /* eat RBRACE */
      return node;
    }
    node->initValNodes.push_back(initValAnalyse());
    while (peekToken(0)->tokenType == TokenType::COMMA) {
      popToken(); /* eat COMMA */
      node->initValNodes.push_back(initValAnalyse());
    }
    popToken(); /* eat RBRACE */
    return node;
  }
  node->initArray = false;
  node->expNode = expAnalyse();
  return node;
}

FuncDefNode* Parser::funcDefAnalyse()
{
  FuncDefNode* node = new FuncDefNode();
  node->funcTypeNode = funcTypeAnalyse();
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */
  popToken(); /* eat LPARENT */

  if (peekToken(0)->tokenType != TokenType::RPARENT) {
    node->funcFParamsNode = funcFParamsAnalyse();
    node->hasFuncFParams = true;
  }
  popToken(); /* eat RPARENT */
  node->blockNode = blockAnalyse();
  return node;
}

MainFuncDefNode* Parser::mainFuncDefAnalyse()
{
  MainFuncDefNode* node = new MainFuncDefNode();
  popToken(); /* eat INTTK */
  popToken(); /* eat MAINTK */
  popToken(); /* eat LPARENT */
  popToken(); /* eat RPARENT */
  node->blockNode = blockAnalyse();
  return node;
}

FuncTypeNode* Parser::funcTypeAnalyse()
{
  FuncTypeNode* node = new FuncTypeNode();
  node->funcType = peekToken(0);
  popToken(); /* eat VOIDTK or INTTK */
  return node;
}

FuncFParamsNode* Parser::funcFParamsAnalyse()
{
  FuncFParamsNode* node = new FuncFParamsNode();
  node->funcFParamNodes.push_back(FuncFParamAnalyse());
  while (peekToken(0)->tokenType == TokenType::COMMA) {
    popToken(); /* eat COMMA */
    node->funcFParamNodes.push_back(FuncFParamAnalyse());
  }
  return node;
}

FuncFParamNode* Parser::FuncFParamAnalyse()
{
  FuncFParamNode* node = new FuncFParamNode();
  node->bTypeNode = bTypeAnalyse();
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */
  if (peekToken(0)->tokenType == TokenType::LBRACK) {
    popToken(); /* eat LBRACK */
    popToken(); /* eat RBRACK */
    node->arrayDimension = 1;
    while (peekToken(0)->tokenType == TokenType::LBRACK) {
      popToken(); /* eat LBRACK */
      node->arrayDimension++;
      node->constExpNodes.push_back(constExpAnalyse());
      popToken(); /* eat RBRACK */
    }
  }
  return node;
}

BlockNode* Parser::blockAnalyse()
{
  BlockNode* node = new BlockNode();
  popToken(); /* eat LBRACE */
  while (peekToken(0)->tokenType != TokenType::RBRACE) {
    node->blockItemNodes.push_back(blockItemAnalyse());
  }
  popToken(); /* eat RBRACE */
  return node;
}

BlockItemNode* Parser::blockItemAnalyse()
{
  BlockItemNode* node = new BlockItemNode();
  if (peekToken(0)->tokenType == TokenType::CONSTTK || 
    peekToken(0)->tokenType == TokenType::INTTK) {
      node->blockItemType = BlockItemType::BLOCKITEM_DECL;
      node->declNode = declAnalyse();
      return node;
    }
  node->blockItemType = BlockItemType::BLOCKITEM_STMT;
  node->stmtNode = stmtAnalyse();
  return node;
}

StmtNode* Parser::stmtAnalyse()
{
  StmtNode* node = new StmtNode();
  TokenType t = peekToken(0)->tokenType;

  if (t == TokenType::PRINTFTK) {
    node->stmtType = StmtType::STMT_PRINTF;
    popToken(); /* eat PRINTFTK */
    popToken(); /* eat LPARENT */
    node->formatString = peekToken(0);
    popToken(); /* eat STRCON */
    while (peekToken(0)->tokenType == TokenType::COMMA) {
      popToken(); /* eat COMMA */
      node->expNodes.push_back(expAnalyse());
    }
    popToken(); /* eat RPARENT */
    popToken(); /* eat SEMICN */
  } else if (t == TokenType::RETURNTK) { 
    node->stmtType = StmtType::STMT_RETURN;
    popToken(); /* eat RETURNTK */
    if (peekToken(0)->tokenType != TokenType::SEMICN) {
      node->expNodes.push_back(expAnalyse());
    }
    popToken(); /* eat SEMICN */
  } else if (t == TokenType::BREAKTK) {
    node->stmtType = StmtType::STMT_BREAK;
    popToken(); /* eat BREAKTK */
    popToken(); /* eat SEMICON */
  } else if (t == TokenType::CONTINUETK) {
    node->stmtType = StmtType::STMT_CONTINUE;
    popToken(); /* eat CONTINUETK */
    popToken(); /* eat SEMICON */
  } else if (t == TokenType::WHILETK) {
    node->stmtType = StmtType::STMT_WHILE;
    popToken(); /* eat WHILETK */
    popToken(); /* eat LPARENT */
    node->condNode = condAnalyse();
    popToken(); /* eat RPARENT */
    node->stmtNodes.push_back(stmtAnalyse());
  } else if (t == TokenType::IFTK) {
    node->stmtType = StmtType::STMT_IF;
    popToken(); /* eat IFTK */
    popToken(); /* eat LPARENT */
    node->condNode = condAnalyse();
    popToken(); /* eat RPARENT */
    node->stmtNodes.push_back(stmtAnalyse());
    if (peekToken(0)->tokenType == TokenType::ELSETK) {
      popToken(); /* eat ELSETK */
      node->hasElse = true;
      node->stmtNodes.push_back(stmtAnalyse());
    }
  } else if (t == TokenType::LBRACE) {
    node->stmtType = StmtType::STMT_BLOCK;
    node->blockNode = blockAnalyse();
  } else if (isAssign()) {
    node->stmtType = StmtType::STMT_ASSIGN;
    node->lValNode = lValAnalyse();
    popToken(); /* eat ASSIGN */
    if (peekToken(0)->tokenType == TokenType::GETINTTK) {
      node->stmtType = StmtType::STMT_GETINT;
      popToken(); /* eat GETINTTK */
      popToken(); /* eat LPARENT */
      popToken(); /* eat RPARENT */
      popToken(); /* eat SEMICN */
      return node;
    }
    node->expNodes.push_back(expAnalyse());
    popToken(); /* eat SEMICN */
  } else {
    node->stmtType = StmtType::STMT_EXP;
    if (peekToken(0)->tokenType != TokenType::SEMICN) {
      node->expNodes.push_back(expAnalyse());
    }
    popToken(); /* eat SEMICN */
  }
  return node;
}

bool Parser::isAssign()
{
  for (int i = 0;;i++) {
    if (peekToken(i)->tokenType == TokenType::ASSIGN) {
      return true;
    }
    if (peekToken(i)->tokenType == TokenType::SEMICN) {
      return false;
    }
  }
}

ExpNode* Parser::expAnalyse()
{
  ExpNode* node = new ExpNode();
  node->addExpNode = addExpAnalyse();
  return node;
}

CondNode* Parser::condAnalyse() 
{
  CondNode* node = new CondNode();
  node->lOrExpNode = lOrExpAnalyse();
  return node;
}

LValNode* Parser::lValAnalyse()
{
  LValNode* node = new LValNode();
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */
  while (peekToken(0)->tokenType == TokenType::LBRACK) {
    popToken(); /* eat LBRACK */
    node->expNodes.push_back(expAnalyse());
    popToken(); /* eat RBRACK */
  }
  return node;
}

PrimaryExpNode* Parser::primaryExpAnalyse()
{   
  PrimaryExpNode* node = new PrimaryExpNode();
  TokenType t = peekToken(0)->tokenType;
  switch (t)
  {
  case TokenType::LPARENT:
    node->primaryExpType = PrimaryExpType::PRIMARY_EXP;
    popToken(); /* eat LPARENT */
    node->expNode = expAnalyse();
    popToken(); /* eat RPARENT */
    break;
  case TokenType::IDENFR:
    node->primaryExpType = PrimaryExpType::PRIMARY_LVAL;
    node->lValNode = lValAnalyse();
    break;
  case TokenType::INTCON:
    node->primaryExpType = PrimaryExpType::PRIMARY_NUMBER;
    node->numberNode = numberAnalyse();
    break;
  default:
    Log("error:tokenType=%s,tokenValue=%s,poi=%d\n", tokenName[t], tokenValue[t], nowTokenListPtr);
    break;
  }
  return node;
}

NumberNode* Parser::numberAnalyse()
{
  NumberNode* node = new NumberNode();
  node->intConst = peekToken(0);
  popToken(); /* eat INTCON */
  return node;
}

UnaryExpNode* Parser::unaryExpAnalyse()
{
  UnaryExpNode* node = new UnaryExpNode();
  if (peekToken(0)->tokenType == TokenType::PLUS || 
  peekToken(0)->tokenType == TokenType::MINU ||
  peekToken(0)->tokenType == TokenType::NOT) {
    node->unaryExpType = UnaryExpType::UNARY_PREFIX;
    node->unaryOpNode = unaryOpAnalyse();
    node->unaryExpNode = unaryExpAnalyse();
  } else if (peekToken(0)->tokenType == TokenType::IDENFR && 
  peekToken(1)->tokenType == TokenType::LPARENT) {
    node->unaryExpType = UnaryExpType::UNARY_FUNCCALL;
    node->ident = peekToken(0);
    popToken(); /* eat IDENFR */
    popToken(); /* eat LPARENT */
    if (peekToken(0)->tokenType != TokenType::RPARENT) {
      node->hasFuncRParams = true;
      node->funcRParamsNode = funcRParamsAnalyse();
    }
    popToken();/* eat RPARENT */
  } else {
    node->unaryExpType = UnaryExpType::UNARY_PRIMARYEXP;
    node->primaryExpNode = primaryExpAnalyse();
  }
  return node;
}

UnaryOpNode* Parser::unaryOpAnalyse()
{
  UnaryOpNode* node = new UnaryOpNode();
  node->unaryOp = peekToken(0);
  popToken(); /* eat PLUS | MINU | NOT */
  return node;
}

FuncRParamsNode* Parser::funcRParamsAnalyse()
{
  FuncRParamsNode* node = new FuncRParamsNode();
  node->expNodes.push_back(expAnalyse());
  while (peekToken(0)->tokenType == TokenType::COMMA) {
    popToken(); /* eat COMMA */
    node->expNodes.push_back(expAnalyse());
  }
  return node;
}

MulExpNode* Parser::mulExpAnalyse()
{
  MulExpNode* node = new MulExpNode();
  node->unaryExpNodes.push_back(unaryExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::MULT ||
  peekToken(0)->tokenType == TokenType::DIV ||
  peekToken(0)->tokenType == TokenType::MOD) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat MULT | DIV | MID */
    node->unaryExpNodes.push_back(unaryExpAnalyse());
  }
  return node;
}

AddExpNode* Parser::addExpAnalyse()
{
  AddExpNode* node = new AddExpNode();
  node->MulExpNodes.push_back(mulExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::PLUS ||
  peekToken(0)->tokenType == TokenType::MINU) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat PLUS | MINU */
    node->MulExpNodes.push_back(mulExpAnalyse());
  }
  return node;
}

RelExpNode* Parser::relExpAnalyse()
{
  RelExpNode* node = new RelExpNode();
  node->addExpNodes.push_back(addExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::LSS || 
  peekToken(0)->tokenType == TokenType::GRE ||
  peekToken(0)->tokenType == TokenType::LEQ ||
  peekToken(0)->tokenType == TokenType::GEQ) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat LSS | GRE | LEQ | GEQ */
    node->addExpNodes.push_back(addExpAnalyse());
  }
  return node;
}

EqExpNode* Parser::eqExpAnalyse()
{
  EqExpNode* node = new EqExpNode();
  node->relExpNodes.push_back(relExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::EQL ||
  peekToken(0)->tokenType == TokenType::NEQ) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat EQL | NEQ */
    node->relExpNodes.push_back(relExpAnalyse());
  }
  return node;
}

LAndExpNode* Parser::lAndExpAnalyse()
{
  LAndExpNode* node = new LAndExpNode();
  node->eqExpNodes.push_back(eqExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::AND) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat AND */
    node->eqExpNodes.push_back(eqExpAnalyse());
  }
  return node;
}

LOrExpNode* Parser::lOrExpAnalyse()
{
  LOrExpNode* node = new LOrExpNode();
  node->lAndExpNodes.push_back(lAndExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::OR) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat OR */
    node->lAndExpNodes.push_back(lAndExpAnalyse());
  }
  return node;
}

ConstExpNode* Parser::constExpAnalyse()
{
  ConstExpNode* node = new ConstExpNode();
  node->addExpNode = addExpAnalyse();
  return node;
}

void Parser::toString()
{
  if (root == NULL) {
    root = compUnitAnalyse();
  }
  std::string s = root->toString();
  printf("%s", s.c_str());
}

SyntaxNode* Parser::syntaxAnalyse()
{ 
  if (root != NULL) {
    return root;
  }
  root = compUnitAnalyse();
  return root;
}