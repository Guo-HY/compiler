#include "parser.hpp"
#include "../include/error.hpp"

extern char tokenName[][20];
extern char tokenValue[][20];
extern SymbolTable* currentSymbolTable;
extern SymbolTable* globalSymbolTable;
extern ErrorList errorList;
extern void symbolTableInit();
/* 当前循环层数 */
int cycleDepth = 0;
/* ------------------ tools function ------------------ */
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
  if (nowTokenListPtr + num >= (int)tokenInfoList.size() ||
      nowTokenListPtr + num < 0) {
    Log("error : peek out of bounds\n");
    return NULL;
  }
  return tokenInfoList[nowTokenListPtr + num];
}
/* ------------------ tools function ------------------ */

/* ------------------ error handle ------------------ */
bool Parser::tokenLackHandler(TokenType tokenType)
{
  if (peekToken(0)->tokenType != tokenType) {
    ErrorInfo* errorInfo = new ErrorInfo();
    errorInfo->line = peekToken(-1)->line;
    switch (tokenType)
    {
    case TokenType::SEMICN:
      errorInfo->errorType = 'i';
      break;
    case TokenType::RPARENT:
      errorInfo->errorType = 'j';
      break;
    case TokenType::RBRACK:
      errorInfo->errorType = 'k';
      break;
    default:
      errorInfo->errorType = '#';
      break;
    }
    errorList.addErrorInfo(errorInfo);
    return true;
  }
  popToken();
  return false;
}

bool Parser::formatErrorHandler(int pLine, int formatNum, TokenInfo* token)
{
  const char* str = token->str.c_str();
  int fNum = 0;
  int fLine = token->line;
  for (int i = 1; str[i] != '"'; i++) {
    if (str[i] == 32 || str[i] == 33 || (str[i] >= 40 && str[i] <= 126)) {
      if (str[i] == '\\' && str[i + 1] != 'n') {
        errorList.addErrorInfo(new ErrorInfo(fLine, 'a'));
        return true;
      }
    } else if (str[i] == '%' && str[i + 1] == 'd') {
      fNum++;
    } else {
      errorList.addErrorInfo(new ErrorInfo(fLine, 'a'));
      return true;
    }
  }
  if (fNum != formatNum) {
    errorList.addErrorInfo(new ErrorInfo(pLine, 'l'));
    return true;
  }
  return false;
}

bool Parser::errorMHandler(int line) 
{
  if (cycleDepth == 0) {
    errorList.addErrorInfo(new ErrorInfo(line, 'm'));
    return true;
  }
  return false;
}
/* ------------------ error handle ------------------ */

CompUnitNode* Parser::compUnitAnalyse() 
{
  symbolTableInit();
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
  node->constDefNodes.push_back(constDefAnalyse(node->bTypeNode));
  while (peekToken(0)->tokenType == TokenType::COMMA) {
    popToken(); /* eat COMMA */
    node->constDefNodes.push_back(constDefAnalyse(node->bTypeNode));
  }
  /* check if lack SEMICN ,if not lack then eat */
  tokenLackHandler(TokenType::SEMICN);
  return node;
}

BTypeNode* Parser::bTypeAnalyse()
{
  BTypeNode* node = new BTypeNode();
  popToken(); /* eat INTTK */
  return node;
}

ConstDefNode* Parser::constDefAnalyse(BTypeNode* bType)
{
  ConstDefNode* node = new ConstDefNode();
  node->isConst = true;
  node->bTypeNode = bType;
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */

  /* deal { '[' ConstExp ']' } */
  while (peekToken(0)->tokenType == TokenType::LBRACK) {
    popToken(); /* eat LBRACK */
    (node->arrayDimension)++;
    node->constExpNodes.push_back(constExpAnalyse());
    /* check if lack RBRACK if not lack then eat */
    tokenLackHandler(TokenType::RBRACK);
  }
  popToken(); /* eat ASSIGN */
  node->constInitValNode = constInitValAnalyse();

  /* insert symbol to currentSymbolTable */
  currentSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::ABSTVAR_SNT);

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
  node->varDefNodes.push_back(varDefAnalyse(node->bTypeNode));
  while (peekToken(0)->tokenType == TokenType::COMMA) {
    popToken(); /* eat COMMA */
    node->varDefNodes.push_back(varDefAnalyse(node->bTypeNode));
  }
  /* check if lack SEMICN ,if not lack then eat */
  tokenLackHandler(TokenType::SEMICN);
  return node;
} 

VarDefNode* Parser::varDefAnalyse(BTypeNode* bType)
{
  VarDefNode* node = new VarDefNode();
  node->bTypeNode = bType;
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */
  while (peekToken(0)->tokenType == TokenType::LBRACK) {
    popToken(); /* eat LBRACK */
    (node->arrayDimension)++;
    node->constExpNodes.push_back(constExpAnalyse());
    /* check if lack RBRACK ,if not lack then eat */
    tokenLackHandler(TokenType::RBRACK);
  } 
  if (peekToken(0)->tokenType == TokenType::ASSIGN) {
    popToken(); /* eat ASSIGN */
    node->hasInitVal = true;
    node->initValNode = initValAnalyse();
  }

  /* insert symbol to currentSymbolTable */
  currentSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::ABSTVAR_SNT);

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
  node->expNode = expAnalyse(NULL, false, NULL);
  return node;
}

FuncDefNode* Parser::funcDefAnalyse()
{
  /* 在解析函数形参前需要新建符号表 */
  currentSymbolTable = currentSymbolTable->newSon();

  FuncDefNode* node = new FuncDefNode();
  node->funcTypeNode = funcTypeAnalyse();
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */
  popToken(); /* eat LPARENT */
  if (peekToken(0)->tokenType == TokenType::INTTK) {
    node->funcFParamsNode = funcFParamsAnalyse();
    node->hasFuncFParams = true;
  }
  /* check if lack RPARENT ,if not lack then eat */
  tokenLackHandler(TokenType::RPARENT);

  /* insert symbol to globalSymbolTable 需要在解析block前，解析形参后将函数插入符号表 */
  globalSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::FUNC_SNT);

  node->blockNode = blockAnalyse(false, &(node->ident->str), 
  (node->funcTypeNode->funcType->tokenType == INTTK));
  /* 回溯符号表 */
  currentSymbolTable = currentSymbolTable->findParent();
  return node;
}

MainFuncDefNode* Parser::mainFuncDefAnalyse()
{
  /* 在解析函数形参前需要新建符号表 */
  currentSymbolTable = currentSymbolTable->newSon();

  MainFuncDefNode* node = new MainFuncDefNode();
  node->funcTypeNode = funcTypeAnalyse();
  node->ident = peekToken(0);
  popToken(); /* eat MAINTK */
  popToken(); /* eat LPARENT */
  /* check if lack RPARENT ,if not lack then eat */
  tokenLackHandler(TokenType::RPARENT);

  /* insert symbol to globalSymbolTable 需要在解析block前，解析形参后将函数插入符号表 */
  globalSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::FUNC_SNT);
  
  node->blockNode = blockAnalyse(false, &(node->ident->str), true);
  /* 回溯符号表 */
  currentSymbolTable = currentSymbolTable->findParent();
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
    /* check if lack RBRACK ,if not lack then eat */
    tokenLackHandler(TokenType::RBRACK);
    node->arrayDimension = 1;
    while (peekToken(0)->tokenType == TokenType::LBRACK) {
      popToken(); /* eat LBRACK */
      node->arrayDimension++;
      node->constExpNodes.push_back(constExpAnalyse());
      /* check if lack RBRACK ,if not lack then eat */
      tokenLackHandler(TokenType::RBRACK);
    }
  }

  /* insert symbol to currentSymbolTable */
  currentSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::ABSTVAR_SNT);

  return node;
}

BlockNode* Parser::blockAnalyse(bool newSymbolTable, std::string* funcName, bool needRE)
{
  bool hasRE = false;
  if (newSymbolTable) {
    currentSymbolTable = currentSymbolTable->newSon();
  }
  BlockNode* node = new BlockNode();
  popToken(); /* eat LBRACE */
  while (peekToken(0)->tokenType != TokenType::RBRACE) {
    node->blockItemNodes.push_back(blockItemAnalyse(funcName, &hasRE));
  }
  /* check error g */
  if (needRE && !hasRE) {
    errorList.addErrorInfo(new ErrorInfo(peekToken(0)->line, 'g'));
  }
  popToken(); /* eat RBRACE */

  if (newSymbolTable) {
    currentSymbolTable = currentSymbolTable->findParent();
  }
  return node;
}

BlockItemNode* Parser::blockItemAnalyse(std::string* funcName, bool* hasRE)
{
  BlockItemNode* node = new BlockItemNode();
  if (peekToken(0)->tokenType == TokenType::CONSTTK || 
    peekToken(0)->tokenType == TokenType::INTTK) {
      node->blockItemType = BlockItemType::BLOCKITEM_DECL;
      node->declNode = declAnalyse();
      *hasRE = false;
      return node;
    }
  node->blockItemType = BlockItemType::BLOCKITEM_STMT;
  node->stmtNode = stmtAnalyse(funcName, hasRE);
  return node;
}

bool Parser::isExpFirst() 
{
  TokenType type = peekToken(0)->tokenType;
  if (type == TokenType::LPARENT || type == TokenType::IDENFR ||
      type == TokenType::INTCON || type == TokenType::PLUS ||
      type == TokenType::MINU || type == TokenType::NOT) {
    return true;
  }
  return false;
}

StmtNode* Parser::stmtAnalyse(std::string* funcName, bool* hasRE)
{
  *hasRE = false;
  bool tmpRE;
  StmtNode* node = new StmtNode();
  TokenType t = peekToken(0)->tokenType;
  
  if (t == TokenType::PRINTFTK) {
    int fNum = 0;
    int pLine = peekToken(0)->line;
    node->stmtType = StmtType::STMT_PRINTF;
    popToken(); /* eat PRINTFTK */
    popToken(); /* eat LPARENT */
    node->formatString = peekToken(0);
    popToken(); /* eat STRCON */
    while (peekToken(0)->tokenType == TokenType::COMMA) {
      popToken(); /* eat COMMA */
      node->expNodes.push_back(expAnalyse(NULL, false, NULL));
      fNum++;
    }
    /* check error a & l */
    formatErrorHandler(pLine, fNum, node->formatString);
    /* check if lack RPARENT if not lack then eat */
    tokenLackHandler(TokenType::RPARENT);
    /* check if lack SEMICN if not lack then eat */
    tokenLackHandler(TokenType::SEMICN);
  } else if (t == TokenType::RETURNTK) { 
    node->stmtType = StmtType::STMT_RETURN;
    int rLine = peekToken(0)->line;
    popToken(); /* eat RETURNTK */
    if (isExpFirst()) {
      node->expNodes.push_back(expAnalyse(NULL, false, NULL));
      /* check error f */
      globalSymbolTable->funcReturnCheck(funcName, true, rLine);
      *hasRE = true;
    }
    /* check if lack SEMICN ,if not lack then eat */
    tokenLackHandler(TokenType::SEMICN);
  } else if (t == TokenType::BREAKTK) {
    errorMHandler(peekToken(0)->line);
    node->stmtType = StmtType::STMT_BREAK;
    popToken(); /* eat BREAKTK */
    /* check if lack SEMICN ,if not lack then eat */
    tokenLackHandler(TokenType::SEMICN);
  } else if (t == TokenType::CONTINUETK) {
    errorMHandler(peekToken(0)->line);
    node->stmtType = StmtType::STMT_CONTINUE;
    popToken(); /* eat CONTINUETK */
    /* check if lack SEMICN ,if not lack then eat */
    tokenLackHandler(TokenType::SEMICN);
  } else if (t == TokenType::WHILETK) {
    cycleDepth++;
    node->stmtType = StmtType::STMT_WHILE;
    popToken(); /* eat WHILETK */
    popToken(); /* eat LPARENT */
    node->condNode = condAnalyse();
    /* check if lack RPARENT ,if not lack then eat */
    tokenLackHandler(TokenType::RPARENT);
    node->stmtNodes.push_back(stmtAnalyse(funcName, &tmpRE));
    cycleDepth--;
  } else if (t == TokenType::IFTK) {
    node->stmtType = StmtType::STMT_IF;
    popToken(); /* eat IFTK */
    popToken(); /* eat LPARENT */
    node->condNode = condAnalyse();
    /* check if lack RPARENT ,if not lack then eat */
    tokenLackHandler(TokenType::RPARENT);
    node->stmtNodes.push_back(stmtAnalyse(funcName, &tmpRE));
    if (peekToken(0)->tokenType == TokenType::ELSETK) {
      popToken(); /* eat ELSETK */
      node->hasElse = true;
      node->stmtNodes.push_back(stmtAnalyse(funcName, &tmpRE));
    }
  } else if (t == TokenType::LBRACE) {
    node->stmtType = StmtType::STMT_BLOCK;
    node->blockNode = blockAnalyse(true, funcName, false);

    /* 此时需要解析LVal '=' Exp ';' 或者 [Exp] ';'
        如果当前token是IDENFR并且下一个token不是LPARENT，那么当前元素一定为LVal，
       并且无法判断是LVal '=' Exp ';'还是[Exp] ';'，此时先解析一个LVal，然后判断接下来的token是否为等号，
       如果是等号就解析LVal '=' Exp ';'，否则解析[Exp] ';'
        否则就一定是[Exp] ';'
    */
  } else if (peekToken(0)->tokenType == TokenType::IDENFR && 
             peekToken(1)->tokenType != TokenType::LPARENT) {
    LValNode* lValNodeTmp = lValAnalyse();
    if (peekToken(0)->tokenType == TokenType::ASSIGN) { /* LVal '=' Exp ';' */
      /* check error h */
      currentSymbolTable->constModifyCheck(lValNodeTmp);
      node->stmtType = StmtType::STMT_ASSIGN;
      node->lValNode = lValNodeTmp;
      popToken(); /* eat ASSIGN */
      if (peekToken(0)->tokenType == TokenType::GETINTTK) { /* LVal '=' 'getint' '(' ')' ';' */
        node->stmtType = StmtType::STMT_GETINT;
        popToken(); /* eat GETINTTK */
        popToken(); /* eat LPARENT */
        /* check if lack RPARENT ,if not lack then eat */
        tokenLackHandler(TokenType::RPARENT);
        /* check if lack SEMICN , if not lack then eat */
        tokenLackHandler(TokenType::SEMICN);
        return node;
      }
      node->expNodes.push_back(expAnalyse(NULL, false, NULL));
      /* check if lack SEMICN ,if not lack then eat */
      tokenLackHandler(TokenType::SEMICN);
    } else { /* [Exp] ';' */
      node->stmtType = StmtType::STMT_EXP;
      node->expNodes.push_back(expAnalyse(lValNodeTmp, false, NULL));// 此处传入已经分析好的lval
      /* check if lack SEMICN ,if not lack then eat */
      tokenLackHandler(TokenType::SEMICN);
    }
  } else {
    /* 此时一定为[Exp] ';' */
    node->stmtType = StmtType::STMT_EXP;
    if (isExpFirst()) {
      node->expNodes.push_back(expAnalyse(NULL, false, NULL));
    }
    /* check if lack SEMICN ,if not lack then eat */
    tokenLackHandler(TokenType::SEMICN);
  }

  return node;
}

ExpNode* Parser::expAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams)
{
  ExpNode* node = new ExpNode();
  node->addExpNode = addExpAnalyse(lval, addToNFCP, funcCParams);
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
  /* check error c */
  currentSymbolTable->undefSymbolHandler(&(node->ident->str), node->ident->line);
  popToken(); /* eat IDENFR */
  while (peekToken(0)->tokenType == TokenType::LBRACK) {
    popToken(); /* eat LBRACK */
    node->expNodes.push_back(expAnalyse(NULL, false, NULL));
    /* check if lack RBRACK ,if not lack then eat */
    tokenLackHandler(TokenType::RBRACK);
  }
  return node;
}

PrimaryExpNode* Parser::primaryExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams)
{   
  PrimaryExpNode* node = new PrimaryExpNode();
  if (lval != NULL) {
    node->primaryExpType = PrimaryExpType::PRIMARY_LVAL;
    node->lValNode = lval;
    if (addToNFCP == true) {
      /* 向funcCParams中添加元素 */
      funcCParams->push_back(currentSymbolTable->getLValType(lval));
    }
    return node;
  }

  TokenType t = peekToken(0)->tokenType;
  switch (t)
  {
  case TokenType::LPARENT:
    node->primaryExpType = PrimaryExpType::PRIMARY_EXP;
    popToken(); /* eat LPARENT */
    node->expNode = expAnalyse(NULL, addToNFCP, funcCParams);
    /* check if lack RPARENT ,if not lack then eat */
    tokenLackHandler(TokenType::RPARENT);
    break;
  case TokenType::IDENFR:
    node->primaryExpType = PrimaryExpType::PRIMARY_LVAL;
    node->lValNode = lValAnalyse();
    if (addToNFCP == true) {
      /* 向funcCParams中添加元素 */
      funcCParams->push_back(currentSymbolTable->getLValType(node->lValNode));
    }
    break;
  case TokenType::INTCON:
    node->primaryExpType = PrimaryExpType::PRIMARY_NUMBER;
    node->numberNode = numberAnalyse();
    if (addToNFCP == true) {
      /* 向funcCParams中添加元素 */
      funcCParams->push_back(currentSymbolTable->getNumberType());
    }
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

UnaryExpNode* Parser::unaryExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams)
{
  bool ret;
  UnaryExpNode* node = new UnaryExpNode();
  if (lval != NULL) {
    node->unaryExpType = UnaryExpType::UNARY_PRIMARYEXP;
    node->primaryExpNode = primaryExpAnalyse(lval, addToNFCP, funcCParams);
    return node;
  }
  if (peekToken(0)->tokenType == TokenType::PLUS || 
      peekToken(0)->tokenType == TokenType::MINU ||
      peekToken(0)->tokenType == TokenType::NOT) 
  {
    node->unaryExpType = UnaryExpType::UNARY_PREFIX;
    node->unaryOpNode = unaryOpAnalyse();
    node->unaryExpNode = unaryExpAnalyse(NULL, addToNFCP, funcCParams);
  } 
  else if (peekToken(0)->tokenType == TokenType::IDENFR && 
           peekToken(1)->tokenType == TokenType::LPARENT) 
  {
    node->unaryExpType = UnaryExpType::UNARY_FUNCCALL;
    node->ident = peekToken(0);
    /* check error c ,需要在全局符号表中查找 */
    ret = globalSymbolTable->undefSymbolHandler(&(node->ident->str), node->ident->line);
    popToken(); /* eat IDENFR */
    popToken(); /* eat LPARENT */
    std::vector<ObjectSymbolItem*>* funcCParams = new std::vector<ObjectSymbolItem*>;
    if (peekToken(0)->tokenType != TokenType::RPARENT) {
      node->hasFuncRParams = true;
      node->funcRParamsNode = funcRParamsAnalyse(funcCParams);
    }
    if (ret == false) {
      /* check error d, e */
      globalSymbolTable->funcCallErrorHandler(&(node->ident->str), funcCParams, node->ident->line);
    }
    if (ret == false && addToNFCP == true) {
      /* 如果符号表中存在函数名，并且addToNFCP为真，就要在funcCParams中填入函数返回值类型 */
      funcCParams->push_back(globalSymbolTable->getFuncReturnType(&(node->ident->str)));
    }
    /* check if lack RPARENT ,if not lack then eat */
    tokenLackHandler(TokenType::RPARENT);
  } 
  else 
  {
    node->unaryExpType = UnaryExpType::UNARY_PRIMARYEXP;
    node->primaryExpNode = primaryExpAnalyse(NULL, addToNFCP, funcCParams);
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

FuncRParamsNode* Parser::funcRParamsAnalyse(std::vector<ObjectSymbolItem*>* funcCParams)
{
  FuncRParamsNode* node = new FuncRParamsNode();
  node->expNodes.push_back(expAnalyse(NULL, true, funcCParams));
  while (peekToken(0)->tokenType == TokenType::COMMA) {
    popToken(); /* eat COMMA */
    node->expNodes.push_back(expAnalyse(NULL, true, funcCParams));
  }
  return node;
}

MulExpNode* Parser::mulExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams)
{
  MulExpNode* node = new MulExpNode(BinaryExpType::MUL);
  node->unaryExpNodes.push_back(unaryExpAnalyse(lval, addToNFCP, funcCParams));
  while (peekToken(0)->tokenType == TokenType::MULT ||
  peekToken(0)->tokenType == TokenType::DIV ||
  peekToken(0)->tokenType == TokenType::MOD) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat MULT | DIV | MID */
    node->unaryExpNodes.push_back(unaryExpAnalyse(NULL, false, NULL));
  }
  return node;
}

AddExpNode* Parser::addExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams)
{
  AddExpNode* node = (AddExpNode*)new BinaryExpNode(BinaryExpType::ADD);
  node->operands.push_back(mulExpAnalyse(lval, addToNFCP, funcCParams));
  while (peekToken(0)->tokenType == TokenType::PLUS ||
  peekToken(0)->tokenType == TokenType::MINU) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat PLUS | MINU */
    node->operands.push_back(mulExpAnalyse(NULL, false, NULL));
  }
  return node;
}

RelExpNode* Parser::relExpAnalyse()
{
  RelExpNode* node = (RelExpNode*)new BinaryExpNode(BinaryExpType::REL);
  node->operands.push_back(addExpAnalyse(NULL, false, NULL));
  while (peekToken(0)->tokenType == TokenType::LSS || 
  peekToken(0)->tokenType == TokenType::GRE ||
  peekToken(0)->tokenType == TokenType::LEQ ||
  peekToken(0)->tokenType == TokenType::GEQ) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat LSS | GRE | LEQ | GEQ */
    node->operands.push_back(addExpAnalyse(NULL, false, NULL));
  }
  return node;
}

EqExpNode* Parser::eqExpAnalyse()
{
  EqExpNode* node = (EqExpNode*)new BinaryExpNode(BinaryExpType::EQ);
  node->operands.push_back(relExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::EQL ||
  peekToken(0)->tokenType == TokenType::NEQ) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat EQL | NEQ */
    node->operands.push_back(relExpAnalyse());
  }
  return node;
}

LAndExpNode* Parser::lAndExpAnalyse()
{
  LAndExpNode* node = (LAndExpNode*)new BinaryExpNode(BinaryExpType::LAND);
  node->operands.push_back(eqExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::AND) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat AND */
    node->operands.push_back(eqExpAnalyse());
  }
  return node;
}

LOrExpNode* Parser::lOrExpAnalyse()
{
  LOrExpNode* node = (LOrExpNode*)new BinaryExpNode(BinaryExpType::LOR);
  node->operands.push_back(lAndExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::OR) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat OR */
    node->operands.push_back(lAndExpAnalyse());
  }
  return node;
}

ConstExpNode* Parser::constExpAnalyse()
{
  ConstExpNode* node = new ConstExpNode();
  node->addExpNode = addExpAnalyse(NULL, false, NULL);
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